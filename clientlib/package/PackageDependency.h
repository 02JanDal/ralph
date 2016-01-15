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

#include <memory>

#include "Version.h"
#include "PackageConfiguration.h"

class QJsonObject;

namespace Ralph {
namespace ClientLib {
class PackageSource;
using RequirementPtr = std::shared_ptr<class Requirement>;

class PackageDependency
{
public:
	explicit PackageDependency(const QString &package = QString());

	QString package() const { return m_package; }
	void setPackage(const QString &package) { m_package = package; }

	VersionRequirement version() const { return m_version; }
	void setVersion(const VersionRequirement &version) { m_version = version; }

	RequirementPtr requirements() const { return m_requirements; }
	void setRequirements(const RequirementPtr &requirements) { m_requirements = requirements; }

	bool isOptional() const { return m_optional; }
	void setOptional(const bool optional) { m_optional = optional; }

	PackageSource *source() const { return m_source; }
	void setSource(PackageSource *source) { m_source = source; }

	PackageConfiguration config() const { return m_config; }
	void setConfig(const PackageConfiguration &config) { m_config = config; }

	QJsonObject toJson() const;
	static PackageDependency fromJson(const QJsonObject &obj);

private:
	QString m_package;
	VersionRequirement m_version;
	RequirementPtr m_requirements;
	bool m_optional;
	PackageSource *m_source;
	PackageConfiguration m_config;
};

}
}
