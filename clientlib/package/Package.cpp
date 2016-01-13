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

#include "Package.h"

#include "Json.h"
#include "Functional.h"
#include "PackageSource.h"
#include "PackageMirror.h"
#include "Requirement.h"

namespace Ralph {
using namespace Common;

namespace ClientLib {

PackageDependency::PackageDependency(QObject *parent)
	: QObject(parent) {}
PackageDependency::PackageDependency(const QString &package, QObject *parent)
	: QObject(parent), m_package(package) {}

void PackageDependency::setPackage(const QString &package)
{
	if (package != m_package) {
		m_package = package;
		emit packageChanged(m_package);
	}
}
void PackageDependency::setVersion(VersionRequirement *version)
{
	if (version != m_version) {
		m_version = version;
		emit versionChanged(m_version);
	}
}
void PackageDependency::setRequirements(const RequirementPtr &requirements)
{
	if (requirements != m_requirements) {
		m_requirements = requirements;
		emit requirementsChanged(m_requirements);
	}
}
void PackageDependency::setOptional(const bool optional)
{
	if (optional != m_optional) {
		m_optional = optional;
		emit optionalChanged(m_optional);
	}
}
void PackageDependency::setSource(PackageSource *source)
{
	if (source != m_source) {
		m_source = source;
		emit sourceChanged(m_source);
	}
}

Package::Package(QObject *parent)
	: QObject(parent)
{
}

void Package::setName(const QString &name)
{
	if (name != m_name) {
		m_name = name;
		emit nameChanged(m_name);
	}
}
void Package::setVersion(const Version &version)
{
	if (version != m_version) {
		m_version = version;
		emit versionChanged(m_version);
	}
}
void Package::setDependencies(const QVector<PackageDependency *> &dependencies)
{
	if (dependencies != m_dependencies) {
		m_dependencies = dependencies;
		emit dependenciesChanged(m_dependencies);
	}
}
void Package::setMirrors(QVector<std::shared_ptr<PackageMirror>> mirrors)
{
	if (m_mirrors == mirrors) {
		return;
	}

	m_mirrors = mirrors;
	emit mirrorsChanged(mirrors);
}

QJsonObject Package::toJson() const
{
	QJsonObject obj;
	obj.insert("name", name());
	obj.insert("version", version().toString());
	if (!mirrors().isEmpty()) {
		obj.insert("mirrors", Json::toJsonArray(mirrors()));
	}
	if (!dependencies().isEmpty()) {
		obj.insert("dependencies", Json::toJsonArray(Functional::map(dependencies(), [](const PackageDependency *dep)
		{
			QJsonObject depObj;
			depObj.insert("name", dep->package());
			if (dep->version()->isValid()) {
				depObj.insert("version", dep->version()->toString());
			}
			depObj.insert("optional", dep->isOptional());
			depObj.insert("requirements", dep->requirements()->toJson());
			if (dep->source()) {
				depObj.insert("source", dep->source()->toJson());
			}
			return depObj;
		})));
	}
	return obj;
}

const Package *Package::fromJson(const QJsonDocument &doc, Package *package)
{
	using namespace Json;

	if (!package) {
		package = new Package();
	}

	try {
		const QJsonObject root = ensureObject(doc);

		package->setName(ensureString(root, "name"));
		package->setVersion(Version::fromString(ensureString(root, "version")));
		package->setMirrors(Functional::map(ensureIsArrayOf<QJsonObject>(root, "mirrors", QVector<QJsonObject>()), &PackageMirror::fromJson));
		package->setDependencies(Functional::map(ensureIsArrayOf<QJsonObject>(root, "dependencies", QVector<QJsonObject>()), [package](const QJsonObject &obj)
		{
			PackageDependency *dep = new PackageDependency(package);
			dep->setPackage(ensureString(obj, "name"));

			if (obj.contains("version")) {
				dep->setVersion(VersionRequirement::fromString(ensureString(obj, "version"), dep));
			}

			dep->setOptional(ensureBoolean(obj, QStringLiteral("optional"), false));
			dep->setRequirements(std::make_shared<AndRequirement>(AndRequirement::fromJson(ensureArray(obj, "requirements", QJsonArray()))));

			if (obj.contains("source") && obj.value("source").isObject()) {
				dep->setSource(PackageSource::fromJson(obj.value("source"), dep));
			} else if (obj.contains("source") && obj.value("source").isString()) {
				dep->setSource(PackageSource::fromString(obj.value("source").toString(), dep));
			}

			return dep;
		}));

		return package;
	} catch (...) {
		delete package;
		throw;
	}
}

}
}
