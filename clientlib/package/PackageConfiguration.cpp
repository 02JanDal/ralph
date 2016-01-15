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

#include "PackageConfiguration.h"

#include <QRegularExpression>

#include "Json.h"

namespace Ralph {
namespace ClientLib {

PackageConfiguration::PackageConfiguration() {}

QString PackageConfiguration::key(const PackageConfiguration::PredefinedKeys key)
{
	switch (key) {
	case BuildType: return "build.type";
	}
}

PackageConfiguration PackageConfiguration::fromItems(const QVector<QString> &items)
{
	static QRegularExpression regex("(?<key>[^=:])+[+:](?<value>.*)");

	PackageConfiguration config;

	for (const QString &item : items) {
		const QRegularExpressionMatch match = regex.match(item);
		config.set(match.captured("key"), match.captured("value"));
	}

	return config;
}

PackageConfiguration::BuildTypes PackageConfiguration::buildType() const
{
	const QString str = get<QString>(BuildType).toLower();
	if (str == "debug") {
		return Debug;
	} else if (str == "release") {
		return Release;
	} else {
		return InvalidBuildType;
	}
}
void PackageConfiguration::setBuildType(const PackageConfiguration::BuildTypes &type)
{
	switch (type) {
	case Debug: set(BuildType, "debug"); break;
	case Release: set(BuildType, "release"); break;
	case InvalidBuildType: set(BuildType, QString()); break;
	}
}

QJsonObject PackageConfiguration::toJson() const
{
	QJsonObject obj;

	for (auto it = m_values.constBegin(); it != m_values.constEnd(); ++it) {
		obj.insert(it.key(), QJsonValue::fromVariant(it.value()));
	}

	return obj;
}
PackageConfiguration PackageConfiguration::fromJson(const QJsonObject &obj)
{
	PackageConfiguration config;

	for (auto it = obj.constBegin(); it != obj.constEnd(); ++it) {
		config.m_values.insert(it.key(), it.value().toVariant());
	}

	return config;
}

}
}
