#pragma once

#include <QObject>
#include <QFuture>
#include <QDir>

#include "Package.h"
#include "Version.h"
#include "task/Task.h"

namespace Ralph {
namespace ClientLib {

class PackageDatabase : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool readonly READ isReadonly CONSTANT)

	explicit PackageDatabase(const QDir &dir, const QVector<PackageDatabase *> &inherits, QObject *parent = nullptr);

public:
	static Future<PackageDatabase *> get(const QDir &dir, const QVector<PackageDatabase *> inherits = {});

	bool isReadonly() const;

	Future<void> load();
	Future<void> build();

	const Package *getPackage(const QString &name, const Version &version) const;
	QVector<const Package *> findPackages(const QString &name, VersionRequirement * const version) const;

	PackageSource *source(const QString &name) const;
	QVector<PackageSource *> sources() const { return m_sources; }
	Future<void> registerPackageSource(PackageSource *source);
	Future<void> unregisterPackageSource(const QString &name);

	QVector<PackageDatabase *> inheritedDatabases() const { return m_inherits; }

private: // internal
	Future<void> save();

private: // static/on creation
	const QDir m_dir;
	const QVector<PackageDatabase *> m_inherits;

private: // settings, semi-static
	QVector<PackageSource *> m_sources;

private: // packages, semi-static
	mutable QMutex m_mutex;
	QVector<const Package *> m_packages;
	QMultiHash<QString, const Package *> m_packageMapping;
};

}
}
