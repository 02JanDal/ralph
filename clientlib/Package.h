#pragma once

#include <QObject>
#include <QVector>

#include "VersionRequirement.h"

class QJsonDocument;
class PackageSource;

class PackageDependency : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString package READ package WRITE setPackage NOTIFY packageChanged)
	Q_PROPERTY(VersionRequirement *version READ version WRITE setVersion NOTIFY versionChanged)
	Q_PROPERTY(QVector<QString> operatingSystems READ operatingSystems WRITE setOperatingSystems NOTIFY operatingSystemsChanged)
	Q_PROPERTY(QVector<QString> configurations READ configurations WRITE setConfigurations NOTIFY configurationsChanged)
	Q_PROPERTY(bool optional READ isOptional WRITE setOptional NOTIFY optionalChanged)
	Q_PROPERTY(PackageSource *source READ source WRITE setSource NOTIFY sourceChanged)

public:
	explicit PackageDependency(QObject *parent = nullptr);
	explicit PackageDependency(const QString &package, QObject *parent = nullptr);

	QString package() const { return m_package; }
	void setPackage(const QString &package);

	VersionRequirement *version() const { return m_version; }
	void setVersion(VersionRequirement *version);

	QVector<QString> operatingSystems() const { return m_os; }
	void setOperatingSystems(const QVector<QString> &os);

	QVector<QString> configurations() const { return m_configurations; }
	void setConfigurations(const QVector<QString> &configurations);

	bool isOptional() const { return m_optional; }
	void setOptional(const bool optional);

	PackageSource *source() const { return m_source; }
	void setSource(PackageSource *source);

signals:
	void packageChanged(const QString &package);
	void versionChanged(VersionRequirement *version);
	void operatingSystemsChanged(const QVector<QString> &os);
	void configurationsChanged(const QVector<QString> &configurations);
	void optionalChanged(const bool optional);
	void sourceChanged(const PackageSource *source);

private:
	QString m_package;
	VersionRequirement *m_version;
	QVector<QString> m_os;
	QVector<QString> m_configurations;
	bool m_optional;
	PackageSource *m_source;
};

class Package : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY versionChanged)
	Q_PROPERTY(QVector<PackageDependency *> dependencies READ dependencies WRITE setDependencies NOTIFY dependenciesChanged)

public:
	explicit Package(QObject *parent = nullptr);

	QString name() const { return m_name; }
	void setName(const QString &name);

	QString version() const { return m_version; }
	void setVersion(const QString &version);

	QVector<PackageDependency *> dependencies() const { return m_dependencies; }
	void setDependencies(const QVector<PackageDependency *> &dependencies);

	static Package *fromJson(const QJsonDocument &doc);

signals:
	void nameChanged(const QString &name);
	void versionChanged(const QString &version);
	void dependenciesChanged(const QVector<PackageDependency *> &dependencies);

private:
	QString m_name;
	QString m_version;
	QVector<PackageDependency *> m_dependencies;
};
