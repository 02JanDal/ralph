#include "Functions.h"

#include <QFutureWatcher>
#include <QException>
#include <QStandardPaths>

#include <iostream>

#include "PackageDatabase.h"
#include "config.h"

int State::updatePackageDB()
{
	std::cout << "Updating package database(s). This might take a while...\n";
	try {
		Task<void>::Ptr task = await(createDB())->update();
		task->waitForFinished();
		std::cout << "Finished successfully\n";
		return 0;
	} catch (QException &e) {
		std::cerr << "Failure: " << e.what() << '\n';
		return 1;
	}
}

int State::installForProject()
{
	return 1;
}

int State::removePackage(const QString &package, const QString &version)
{
	return 1;
}

int State::installPackage(const QString &package, const QString &version)
{
	return 1;
}

int State::checkPackage(const QString &package, const QString &version)
{
	return 1;
}

void State::setDir(const QString &dir)
{
	m_dir = dir;
}

Task<PackageDatabase *>::Ptr State::createDB()
{
	return createTask([this](Notifier notifier) -> PackageDatabase *
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
				m_db = await(PackageDatabase::get(m_dir, {global}));
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
