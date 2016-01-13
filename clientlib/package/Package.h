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

#pragma once

#include <QObject>
#include <QVector>
#include <memory>

#include "Version.h"

class QJsonDocument;

namespace Ralph {
namespace ClientLib {
class PackageSource;
class PackageMirror;
using RequirementPtr = std::shared_ptr<class Requirement>;

class PackageDependency : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString package READ package WRITE setPackage NOTIFY packageChanged)
	Q_PROPERTY(VersionRequirement *version READ version WRITE setVersion NOTIFY versionChanged)
	Q_PROPERTY(RequirementPtr requirements READ requirements WRITE setRequirements NOTIFY requirementsChanged)
	Q_PROPERTY(bool optional READ isOptional WRITE setOptional NOTIFY optionalChanged)
	Q_PROPERTY(PackageSource *source READ source WRITE setSource NOTIFY sourceChanged)

public:
	explicit PackageDependency(QObject *parent = nullptr);
	explicit PackageDependency(const QString &package, QObject *parent = nullptr);

	QString package() const { return m_package; }
	void setPackage(const QString &package);

	VersionRequirement *version() const { return m_version; }
	void setVersion(VersionRequirement *version);

	RequirementPtr requirements() const { return m_requirements; }
	void setRequirements(const RequirementPtr &requirements);

	bool isOptional() const { return m_optional; }
	void setOptional(const bool optional);

	PackageSource *source() const { return m_source; }
	void setSource(PackageSource *source);

signals:
	void packageChanged(const QString &package);
	void versionChanged(VersionRequirement *version);
	void requirementsChanged(const RequirementPtr &os);
	void optionalChanged(const bool optional);
	void sourceChanged(const PackageSource *source);

private:
	QString m_package;
	VersionRequirement *m_version;
	RequirementPtr m_requirements;
	QVector<QString> m_configurations;
	bool m_optional;
	PackageSource *m_source;
};

class Package : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(Version version READ version WRITE setVersion NOTIFY versionChanged)
	Q_PROPERTY(QVector<PackageDependency *> dependencies READ dependencies WRITE setDependencies NOTIFY dependenciesChanged)
	Q_PROPERTY(QVector<std::shared_ptr<PackageMirror>> mirrors READ mirrors WRITE setMirrors NOTIFY mirrorsChanged)

public:
	explicit Package(QObject *parent = nullptr);

	QString name() const { return m_name; }
	void setName(const QString &name);

	Version version() const { return m_version; }
	void setVersion(const Version &version);

	QVector<PackageDependency *> dependencies() const { return m_dependencies; }
	void setDependencies(const QVector<PackageDependency *> &dependencies);

	virtual QJsonObject toJson() const;
	static const Package *fromJson(const QJsonDocument &doc, Package *package = nullptr);

	QVector<std::shared_ptr<PackageMirror>> mirrors() const { return m_mirrors; }
	void setMirrors(QVector<std::shared_ptr<PackageMirror>> mirrors);

signals:
	void nameChanged(const QString &name);
	void versionChanged(const Version &version);
	void dependenciesChanged(const QVector<PackageDependency *> &dependencies);
	void mirrorsChanged(QVector<std::shared_ptr<PackageMirror>> mirrors);

private:
	QString m_name;
	Version m_version;
	QVector<PackageDependency *> m_dependencies;
	QVector<std::shared_ptr<PackageMirror>> m_mirrors;
};

}
}
