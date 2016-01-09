#include "Functions.h"

#include <QFutureWatcher>
#include <QException>
#include <QStandardPaths>

#include <iostream>

#include "task/FutureWatcher.h"
#include "project/ProjectGenerator.h"
#include "project/Project.h"
#include "task/Network.h"
#include "TermUtil.h"
#include "FileSystem.h"
#include "Json.h"
#include "CommandLineParser.h"
#include "config.h"

namespace Ralph {
namespace Client {

template <typename T>
T awaitTerminal(const Future<T> &future)
{
	FutureWatcher<T> watcher(future);
	FutureWatcher<T>::connect(&watcher, &FutureWatcher<T>::status, [](const QString &str) {
		std::cout << str << '\n';
	});
	return await(future);
}

State::State()
{
	Network::init();
}

void State::removePackage(const Common::CommandLine::Result &result)
{
	Q_UNUSED(result)
}
void State::installPackage(const Common::CommandLine::Result &result)
{
	Q_UNUSED(result)
}
void State::checkPackage(const Common::CommandLine::Result &result)
{
	Q_UNUSED(result)
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
void State::newProject(const Common::CommandLine::Result &result)
{
	ProjectGenerator generator;
	generator.setName(result.argument("name"));
	generator.setBuildSystem(result.value("build-system"));
	generator.setVCS(result.value("version-control-system"));
	generator.setDirectory(m_dir);
	Project *project = awaitTerminal(generator.generate());
	std::cout << "The project " << project->name().toLocal8Bit().constData() << " was created successfully!\n";
}

void State::updateSources(const Common::CommandLine::Result &result)
{
	Q_UNUSED(result)
	std::cout << "Updating package database(s). This might take a while...\n";
	Future<void> task = awaitTerminal(createDB())->update();
	awaitTerminal(task);
	std::cout << "Finished successfully\n";
}

void State::addSource(const Common::CommandLine::Result &result)
{
	Q_UNUSED(result)
}

void State::removeSource(const Common::CommandLine::Result &result)
{
	Q_UNUSED(result)
}

Future<PackageDatabase *> State::createDB()
{
	return async([this](Notifier notifier) -> PackageDatabase *
	{
		if (!m_db) {
			const QString userDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
#if defined(Q_OS_LINUX)
			const QString systemDir = "/var/ralph";
#else
			const QString systemDir;
#endif
			PackageDatabase *system = systemDir.isNull() ? nullptr : notifier.await(PackageDatabase::get(systemDir));
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
