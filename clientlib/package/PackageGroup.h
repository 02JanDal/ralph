#pragma once

#include <QString>
#include <QDir>

#include "task/Task.h"

namespace Ralph {
namespace ClientLib {
class Package;

class PackageGroup
{
public:
	explicit PackageGroup(const QString &name, const QDir &dir);

	QString name() const { return m_name; }
	QDir dir() const { return m_dir; }

	Future<void> install(const Package *pkg);
	Future<void> remove(const Package *pkg);

	bool isInstalled(const Package *pkg);

	QDir targetDir(const Package *pkg) const;
	QDir baseDir(const Package *pkg) const;

private: // static
	QString m_name;
	QDir m_dir;

private: // from storage
	void readSettings();
	void writeSettings() const;

	struct InstalledPackage
	{
		const Package *pkg;
		int mirrorIndex;
	};
	QVector<InstalledPackage> m_installed;

	QVector<InstalledPackage>::Iterator findInstalled(const Package *pkg);
};

}
}
