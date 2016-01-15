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

#include "PackageDependency.h"

#include "Json.h"
#include "Functional.h"
#include "PackageSource.h"
#include "PackageConfiguration.h"
#include "Requirement.h"

namespace Ralph {
using namespace Common;

namespace ClientLib {

PackageDependency::PackageDependency(const QString &package)
	: m_package(package) {}

QJsonObject PackageDependency::toJson() const
{
	QJsonObject obj;
	obj.insert("name", package());
	if (version().isValid()) {
		obj.insert("version", version().toString());
	}
	if (!config().isEmpty()) {
		obj.insert("config", config().toJson());
	}
	obj.insert("optional", isOptional());
	obj.insert("requirements", requirements()->toJson());
	if (source()) {
		obj.insert("source", source()->toJson());
	}
	return obj;
}
PackageDependency PackageDependency::fromJson(const QJsonObject &obj)
{
	using namespace Json;

	PackageDependency dep;

	dep.setPackage(ensureString(obj, "name"));

	if (obj.contains("version")) {
		dep.setVersion(VersionRequirement::fromString(ensureString(obj, "version")));
	}

	dep.setOptional(ensureBoolean(obj, QStringLiteral("optional"), false));
	dep.setRequirements(std::make_shared<AndRequirement>(AndRequirement::fromJson(ensureArray(obj, "requirements", QJsonArray()))));
	dep.setConfig(PackageConfiguration::fromJson(ensureObject(obj, "config", QJsonObject())));

	if (obj.contains("source") && obj.value("source").isObject()) {
		dep.setSource(PackageSource::fromJson(obj.value("source")));
	} else if (obj.contains("source") && obj.value("source").isString()) {
		dep.setSource(PackageSource::fromString(obj.value("source").toString()));
	}

	return dep;
}

}
}
