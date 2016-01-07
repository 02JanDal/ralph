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
	static Task<PackageDatabase *>::Ptr get(const QDir &dir, const QVector<PackageDatabase *> inherits = {});

	bool isReadonly() const;

	Task<void>::Ptr update();
	Task<void>::Ptr read();
	Task<void>::Ptr build();

	const Package *getPackage(const QString &name, const Version &version) const;
	QVector<const Package *> findPackages(const QString &name, VersionRequirement * const version) const;

	QVector<PackageSource *> sources() const { return m_sources; }
	Task<void>::Ptr registerPackageSource(PackageSource *source);

	QVector<PackageDatabase *> inheritedDatabases() const { return m_inherits; }

private: // internal
	Task<void>::Ptr save();

private: // static/on creation
	const QDir m_dir;
	const QVector<PackageDatabase *> m_inherits;

private: // settings, semi-static
	QVector<PackageSource *> m_sources;

private: // packages, semi-static
	QMutex m_mutex;
	QVector<const Package *> m_packages;
	QMultiHash<QString, const Package *> m_packageMapping;
};

}
}
