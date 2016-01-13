/* Copyright 2016 Jan Dalheimer <jan@dalheimer.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Functions.h"

#include <QFutureWatcher>
#include <QException>
#include <QStandardPaths>

#include <iostream>

#include "future/FutureWatcher.h"
#include "project/ProjectGenerator.h"
#include "project/Project.h"
#include "package/PackageSource.h"
#include "package/PackageGroup.h"
#include "task/Network.h"
#include "git/GitRepo.h"
#include "TermUtil.h"
#include "FileSystem.h"
#include "Json.h"
#include "CommandLineParser.h"
#include "config.h"

namespace Ralph {
using namespace Common;

namespace Client {

namespace {
template <typename T>
T awaitTerminal(const Future<T> &future)
{
	const int maxWidth = Term::currentWidth() == 0 ? 120 : Term::currentWidth();

	FutureWatcher<T> watcher(future);
	FutureWatcher<T>::connect(&watcher, &FutureWatcher<T>::status, [maxWidth](const QString &str)
	{
		std::cout << Term::wrap(str, maxWidth - 6) << std::endl;
	});
	FutureWatcher<T>::connect(&watcher, &FutureWatcher<T>::progress, [maxWidth](const std::size_t current, const std::size_t total)
	{
		if (Term::isTty()) {
			const int percent = int(std::floor(100 * qreal(current) / qreal(total)));
			std::cout << Term::save() << Term::move(Term::Up) << Term::move(Term::Right, maxWidth - 6) << "[";
			if (percent < 10) {
				std::cout << "  ";
			} else if (percent < 100) {
				std::cout << " ";
			}
			std::cout << percent << "%]" << Term::restore() << std::flush;
		}
	});
	return await(future);
}

QString databasePath(const QString &type)
{
	if (type == "user") {
		return QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#ifdef Q_OS_LINUX
	} else if (type == "system") {
		return "/var/ralph";
#endif
	} else {
		return QString();
	}
}

Future<PackageDatabase *> createDatabase(const QString &type)
{
	const QString path = databasePath(type);
	return PackageDatabase::get(path);
}

PackageSource *sourceFromUrl(const QString &url)
{
	const QUrl parsed = QUrl::fromUserInput(url);
	if (!parsed.isValid()) {
		throw Exception("The given URL '%1' is not a valid URL" % url);
	}

	GitRepoPackageSource *src = new GitRepoPackageSource;
	src->setUrl(parsed);
	return src;
}
Term::Color lastUpdatedColor(const PackageSource *source)
{
	const qint64 secsSinceLastUpdate = source->lastUpdated().secsTo(QDateTime::currentDateTimeUtc());
	if (secsSinceLastUpdate < (3600 * 24 * 1)) {
		return Term::Green;
	} else if (secsSinceLastUpdate < (3600 * 24 * 7)) {
		return Term::Yellow;
	} else {
		return Term::Red;
	}
}

const Package *queryPackage(const PackageDatabase *db, const QString &query)
{
	const int splitIndex = query.indexOf('@');
	const QString name = query.mid(0, splitIndex);
	const VersionRequirement *version = splitIndex == -1 ? nullptr : VersionRequirement::fromString(query.mid(splitIndex + 1));

	QVector<const Package *> candidates = db->findPackages(name, version);
	std::sort(candidates.begin(), candidates.end(), [](const Package *a, const Package *b) { return a->version() < b->version(); });

	if (candidates.isEmpty()) {
		const bool haveOtherVersions = !db->findPackages(name).isEmpty();
		if (haveOtherVersions) {
			throw Exception("No package found for %1, but other versions are available" % query);
		} else {
			throw Exception("No package found for %1" % query);
		}
	}

	return candidates.first();
}
}

State::State()
{
	Network::init();
	Git::GitRepo::setCredentialsCallback([](const Git::GitCredentialQuery &query) -> Git::GitCredentialResponse
	{
		if (query.allowedTypes() & Git::GitCredentialQuery::UsernamePassword) {
			std::string username;
			std::cout << "Username and password for %1 required:\n" % query.url().toString()
					  << "Username [%1]: " % query.usernameFromUrl();
			std::getline(std::cin, username);
			std::cout << "Password []: ";
			const QString password = Term::readPassword();
			return Git::GitCredentialResponse::createForUsernamePassword(
						username.empty() ? query.usernameFromUrl() : QString::fromStdString(username),
						password);
		} else {
			return Git::GitCredentialResponse::createInvalid();
		}
	});
}

void State::removePackage(const CommandLine::Result &result)
{
	PackageDatabase *db = awaitTerminal(createDB());
	const QString group = result.value("group");

	Functional::collection(result.argumentMulti("packages"))
			.map([db](const QString &query) { return queryPackage(db, query); })
			.each([db, group](const Package *pkg) { awaitTerminal(db->group(group)->remove(pkg)); });
}
void State::installPackage(const CommandLine::Result &result)
{
	PackageDatabase *db = awaitTerminal(createDB());
	const QString group = result.value("group");

	Functional::collection(result.argumentMulti("packages"))
			.map([db](const QString &query) { return queryPackage(db, query); })
			.each([db, group](const Package *pkg) { awaitTerminal(db->group(group)->install(pkg)); });
}
void State::checkPackage(const CommandLine::Result &result)
{
	PackageDatabase *db = awaitTerminal(createDB());
	const QString group = result.value("group");

	Functional::collection(result.argumentMulti("packages"))
			.map([db](const QString &query) { return queryPackage(db, query); })
			.each([db, group](const Package *pkg) { if (!db->group(group)->isInstalled(pkg)) { throw Exception("%1 is not installed" % pkg->name()); } });
}
void State::searchPackages(const CommandLine::Result &result)
{
	const QRegExp query{result.argument("query"), Qt::CaseInsensitive, QRegExp::WildcardUnix};
	PackageDatabase *db = awaitTerminal(createDB());
	Functional::collection(db->packageNames())
			.filter([query](const QString &str) { return query.isEmpty() || str.contains(query); })
			.each([query](const QString &str) { std::cout << str << '\n'; });
}

void State::setDir(const QString &dir)
{
	m_dir = dir;
}

void State::verifyProject()
{
	const Project *project = Project::fromJson(Json::ensureDocument(QDir(m_dir).absoluteFilePath("ralph.json")));
	std::cout << "The project " << Common::Term::style(Common::Term::Bold, project->name()) << " in " << m_dir << " is valid!\n";
}
void State::newProject(const CommandLine::Result &result)
{
	ProjectGenerator generator;
	generator.setName(result.argument("name"));
	generator.setBuildSystem(result.value("build-system"));
	generator.setVCS(result.value("version-control-system"));
	generator.setDirectory(m_dir);
	Project *project = awaitTerminal(generator.generate());
	std::cout << "The project " << project->name().toLocal8Bit().constData() << " was created successfully!\n";
}

void State::updateSources(const CommandLine::Result &result)
{
	using namespace Term;

	PackageDatabase *db = awaitTerminal(createDatabase(result.value("database")));
	if (!db) {
		throw Exception("Database does not exists and unable to create it");
	}

	const QVector<PackageSource *> sources = result.hasArgument("names") ?
				Functional::map(result.argumentMulti("names"), [db](const QString &name) { return db->source(name); })
			  : db->sources();

	for (PackageSource *source : sources) {
		std::cout << "Updating " << source->typeString() << " source " << fg(Cyan, source->name()) << "...\n";
		awaitTerminal(source->update());
	}
}
void State::addSource(const CommandLine::Result &result)
{
	PackageDatabase *db = awaitTerminal(createDatabase(result.value("database")));
	if (!db) {
		throw Exception("Database does not exists and unable to create it");
	}

	PackageSource *source = sourceFromUrl(result.argument("url"));
	source->setName(result.argument("name"));
	source->setLastUpdated();
	awaitTerminal(db->registerPackageSource(source));
	std::cout << "New source " << source->name() << " successfully registered. You may want to run 'ralph sources update %1' now.\n" % source->name();
}
void State::removeSource(const CommandLine::Result &result)
{
	PackageDatabase *db = awaitTerminal(createDatabase(result.value("database")));
	if (!db) {
		throw Exception("Database does not exists and unable to create it");
	}

	awaitTerminal(db->unregisterPackageSource(result.argument("name")));
	std::cout << "Source " << result.argument("name") << " was successfully removed.\n";
}
void State::listSources(const CommandLine::Result &result)
{
	using namespace Term;

	auto output = [](const QString &databaseType, bool force = false)
	{
		PackageDatabase *db = awaitTerminal(createDatabase(databaseType));
		if (!db) {
			if (force) {
				throw Exception("Database does not exists and unable to create it");
			} else {
				return;
			}
		}

		std::cout << style(Bold, "Package sources in the %1 database:\n" % databaseType);
		for (const PackageSource *source : db->sources()) {
			std::cout << " * " << source->name() << " (type: %1, last updated: %2)\n" % source->typeString() % fg(lastUpdatedColor(source), source->lastUpdated().toString());
		}
		if (db->sources().isEmpty()) {
			std::cout << "    Empty.\n    Use 'ralph sources add <name> <url>' to add a source!\n";
		}
	};

	output(result.value("database"), true);

	if (result.value("database") == "project") {
		std::cout << '\n';
		output("user");
		std::cout << '\n';
		output("system");
	}
	if (result.value("database") == "user") {
		std::cout << '\n';
		output("system");
	}
}
void State::showSource(const CommandLine::Result &result)
{
	using namespace Term;

	PackageDatabase *db = awaitTerminal(createDatabase(result.value("database")));
	PackageSource *src = db->source(result.argument("name"));
	std::cout << style(Bold, "Name: ") << src->name() << '\n'
			  << style(Bold, "Last updated: ") << fg(lastUpdatedColor(src), src->lastUpdated().toString()) << '\n'
			  << style(Bold, "Type: ") << src->typeString() << '\n';
}

void State::info()
{
	const QString systemPath = databasePath("system");
	if (!systemPath.isEmpty()) {
		std::cout << "Available database location: system at " << systemPath << '\n';
	}

	const QString userPath = databasePath("user");
	if (!userPath.isEmpty()) {
		std::cout << "Available database location: user at " << userPath << '\n';
	}
}

Future<PackageDatabase *> State::createDB()
{
	return async([this](Notifier notifier) -> PackageDatabase *
	{
		if (!m_db) {
			const QString userDir = databasePath("user");
			const QString systemDir = databasePath("system");

			PackageDatabase *system = systemDir.isNull() ? nullptr : notifier.await(PackageDatabase::get(databasePath("system")));
			PackageDatabase *user = userDir.isEmpty() ? nullptr : notifier.await(PackageDatabase::get(userDir, {system}));

#ifdef Ralph_DEFAULT_REPO
			if (system && !system->isReadonly() && system->sources().isEmpty()) {
				await()
			} else if (user && !user->isReadonly() && user->sources().isEmpty()) {

			}
#endif

			// system -> user -> local, if no user db is available, but a system db is, it's system -> local
			PackageDatabase *global = (system && !user) ? system : user;

			if (system) {
				std::cout << "Using database: system\n";
			}
			if (user) {
				std::cout << "Using database: user\n";
			}

			if (!m_dir.isNull()) {
				m_db = notifier.await(PackageDatabase::get(m_dir, {global}));
				std::cout << "Using database: project\n";
			} else {
				m_db = global;
			}

			if (!m_db) {
				std::cerr << "Error: Unable to load any package database\n";
				exit(1);
			}
		}
		return m_db;
	});
}

}
}
