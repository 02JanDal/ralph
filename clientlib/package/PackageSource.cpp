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

#include "PackageSource.h"

#include "Json.h"
#include "project/Project.h"
#include "Functional.h"
#include "task/Task.h"
#include "git/GitRepo.h"

namespace Ralph {
using namespace Common;

namespace ClientLib {

static bool isGitCommitish(const QString &str)
{
	for (const QChar &c : str.toLower()) {
		if (c < '0' || c > 'f') {
			return false;
		}
	}
	return str.size() >= 3;
}
static QString cleanGitIdentifier(const QString &id)
{
	if (isGitCommitish(id)) {
		return id;
	} else if (id.startsWith("origin/")) {
		return id;
	} else {
		return "origin/" + id;
	}
}

PackageSource::PackageSource(const SourceType type)
	: m_type(type) {}
PackageSource::~PackageSource() {}

void PackageSource::setName(const QString &name)
{
	Q_ASSERT_X(m_name.isNull(), "PackageSource::setName", "May not change the name of a package");
	m_name = name;
}

void PackageSource::setLastUpdated()
{
	m_lastUpdated = QDateTime::currentDateTimeUtc();
}

PackageSource *PackageSource::fromJson(const QJsonValue &value)
{
	using namespace Json;

	const QJsonObject obj = ensureObject(value);

	auto parseCommon = [obj](PackageSource *src)
	{
		src->setName(ensureString(obj, "name"));
		src->m_lastUpdated = ensureDateTime(obj, "lastUpdated");
	};

	const QString type = ensureString(obj, "type");
	if (type == "git") {
		GitSinglePackageSource *source = new GitSinglePackageSource();
		source->setUrl(ensureUrl(obj, "url"));
		source->setIdentifier(ensureString(obj, "identifier", QString("master")));
		source->setPath(ensureString(obj, "path", QString()));
		parseCommon(source);
		return source;
	} else if (type == "github") {
		GitHubSinglePackageSource *source = new GitHubSinglePackageSource();
		source->setRepo(ensureString(obj, "repo"));
		source->setIdentifier(ensureString(obj, "identifier", QString("master")));
		source->setPath(ensureString(obj, "path", QString()));
		parseCommon(source);
		return source;
	} else if (type == "gitrepo") {
		GitRepoPackageSource *source = new GitRepoPackageSource();
		source->setUrl(ensureUrl(obj, "url"));
		source->setIdentifier(ensureString(obj, "identifier", QString("master")));
		parseCommon(source);
		return source;
	} else {
		throw Exception("Invalid source type: '%1'. Known types: 'git', 'github', 'gitrepo'." % type);
	}
}
PackageSource *PackageSource::fromString(const QString &value)
{
	const QStringList parts = value.split(':', QString::KeepEmptyParts);
	if (parts.isEmpty()) {
		throw Exception("Invalid source specifier");
	}
	const QString type = parts.first();
	if (type == "git") {
		if (parts.size() < 2) {
			throw Exception("Invalid source specifier for type 'git'. Expected format: git:<url>[:<identifier>]");
		}
		GitSinglePackageSource *source = new GitSinglePackageSource();
		source->setUrl(QUrl(parts.at(1)));
		if (parts.size() > 2) {
			source->setIdentifier(parts.at(2));
		}
		return source;
	} else if (type == "github") {
		if (parts.size() < 2) {
			throw Exception("Invalid source specifier for type 'github'. Expected format: github:<repo>[:<identifier>]");
		}
		GitHubSinglePackageSource *source = new GitHubSinglePackageSource();
		source->setRepo(parts.at(1));
		if (parts.size() > 2) {
			source->setIdentifier(parts.at(2));
		}
		return source;
	} else {
		throw Exception("Invalid source specifier: Unknown type '%1'. Known types: 'git', 'github'." % type);
	}
}

QJsonObject PackageSource::toJson() const
{
	return QJsonObject({
						   {"name", name()},
						   {"lastUpdated", Json::toJson(lastUpdated())},
						   {"type", typeString()}
					   });
}

BaseGitPackageSource::BaseGitPackageSource(const SourceType type)
	: PackageSource(type) {}

QString BaseGitPackageSource::toString() const
{
	if (identifier().isNull()) {
		return typeString() + ':' + url().toString();
	} else {
		return QString("%1:%2:%3").arg(typeString(), url().toString(), identifier());
	}
}
QJsonObject BaseGitPackageSource::toJson() const
{
	QJsonObject obj = PackageSource::toJson();
	obj.insert("url", Json::toJson(url()));
	if (!identifier().isEmpty()) {
		obj.insert("identifier", identifier());
	}
	return obj;
}

GitSinglePackageSource::GitSinglePackageSource(const PackageSource::SourceType type)
	: BaseGitPackageSource(type) {}

GitSinglePackageSource::GitSinglePackageSource()
	: BaseGitPackageSource(GitSingle) {}

Future<QVector<const Package *> > GitSinglePackageSource::packages() const
{
	return async([this]()
	{
		const Package *proj = Project::load(basePath().absoluteFilePath(path()));
		return QVector<const Package *>{proj};
	});
}
Future<void> GitSinglePackageSource::update()
{
	return async([this](Notifier notifier)
	{
		if (!basePath().exists()) {
			Git::GitRepo *repo = notifier.await(Git::GitRepo::clone(basePath(), url()));
			notifier.await(repo->checkout(identifier()));
		} else {
			Git::GitRepo *repo = notifier.await(Git::GitRepo::open(basePath()));
			notifier.await(repo->pull(cleanGitIdentifier(identifier())));
		}
		setLastUpdated();
	});
}

QJsonObject GitSinglePackageSource::toJson() const
{
	QJsonObject obj = BaseGitPackageSource::toJson();
	if (!path().isEmpty()) {
		obj.insert("path", path());
	}
	return obj;
}

GitHubSinglePackageSource::GitHubSinglePackageSource()
	: GitSinglePackageSource(GitHubSingle) {}
void GitHubSinglePackageSource::setRepo(const QString &repo)
{
	m_repo = repo;
	setUrl(QUrl("https://github.com/" + m_repo + ".git"));
}

QString GitHubSinglePackageSource::toString() const
{
	if (identifier().isNull()) {
		return typeString() + ':' + repo();
	} else {
		return typeString() + ':' + repo() + ':' + identifier();
	}
}
QJsonObject GitHubSinglePackageSource::toJson() const
{
	QJsonObject obj = GitSinglePackageSource::toJson();
	obj.insert("repo", repo());
	if (!identifier().isEmpty()) {
		obj.insert("identifier", identifier());
	}
	if (!path().isEmpty()) {
		obj.insert("path", path());
	}
	return obj;
}

GitRepoPackageSource::GitRepoPackageSource()
	: BaseGitPackageSource(GitRepo) {}

Future<QVector<const Package *> > GitRepoPackageSource::packages() const
{
	return async([this]()
	{
		const auto files = basePath().entryInfoList(QStringList() << "*.json", QDir::Files | QDir::NoSymLinks | QDir::Readable);
		return Functional::map2<QVector<const Package *>>(files, [](const QFileInfo &file)
		{
			return Package::fromJson(Json::ensureDocument(file.absoluteFilePath()));
		});
	});
}
Future<void> GitRepoPackageSource::update()
{
	return async([this](Notifier notifier)
	{
		if (!basePath().exists()) {
			Git::GitRepo *repo = notifier.await(Git::GitRepo::clone(basePath(), url()));
			notifier.await(repo->checkout(identifier()));
		} else {
			Git::GitRepo *repo = notifier.await(Git::GitRepo::open(basePath()));
			notifier.await(repo->pull(cleanGitIdentifier(identifier())));
		}
		setLastUpdated();
	});
}

}
}
