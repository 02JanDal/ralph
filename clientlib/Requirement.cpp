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

#include "Requirement.h"

#include "Json.h"
#include "Functional.h"
#include "ActionContext.h"

namespace Ralph {
namespace ClientLib {

Requirement::~Requirement() {}

QVector<Requirement::Ptr> Requirement::fromJson(const QJsonArray &array)
{
	using namespace Json;

	return Common::Functional::map(ensureIsArrayOf<QJsonObject>(array), [](const QJsonObject &obj) -> Requirement::Ptr
	{
		const QString type = ensureString(obj, "type", obj.keys().first());
		if (type == "os") {
			return std::make_shared<OsRequirement>(ensureString(obj, "os"));
		} else if (type == "buildType") {
			return std::make_shared<BuildTypeRequirement>(ensureString(obj, "config"));
		} else if (type == "and") {
			return std::make_shared<AndRequirement>(fromJson(ensureArray(obj, "and")));
		} else if (type == "or") {
			return std::make_shared<OrRequirement>(fromJson(ensureArray(obj, "and")));
		} else {
			throw Exception("Unknown requirement type: %1" % type);
		}
	});
}

QJsonArray Requirement::toJson(const QVector<Ptr> &requirements)
{
	QJsonArray array;
	for (const Requirement::Ptr &req : requirements) {
		array.append(req->toJson());
	}
	return array;
}

const OsRequirement::Os OsRequirement::m_currentOs =
		#if defined(Q_OS_LINUX)
		OsRequirement::Linux
		#elif defined(Q_OS_OSX)
		OsRequirement::OSX
		#elif defined(Q_OS_WIN)
		OsRequirement::Windows
		#endif
		;
OsRequirement::OsRequirement(const QString &string)
	: m_os(fromString(string)) {}
QJsonObject OsRequirement::toJson() const
{
	QJsonValue value;
	switch (m_os)
	{
	case Linux: value = "linux"; break;
	case OSX: value = "osx"; break;
	case Windows: value = "windows"; break;
	case Invalid: value = QJsonValue(); break;
	}
	return QJsonObject({qMakePair(QStringLiteral("os"), value)});
}
OsRequirement::Os OsRequirement::fromString(const QString &str)
{
	if (str == "linux") {
		return Linux;
	} else if (str == "osx") {
		return OSX;
	} else if (str == "windows") {
		return Windows;
	} else {
		return Invalid;
	}
}

QJsonObject AndRequirement::toJson() const
{
	return QJsonObject({qMakePair(QStringLiteral("os"), Requirement::toJson(m_children))});
}
bool AndRequirement::isSatisfied(const ActionContext &ctxt) const
{
	return m_children.empty() ||
			std::all_of(std::begin(m_children), std::end(m_children),
						[ctxt](const Requirement::Ptr &ptr) { return ptr->isSatisfied(ctxt); });
}

QJsonObject OrRequirement::toJson() const
{
	return QJsonObject({qMakePair(QStringLiteral("os"), Requirement::toJson(m_children))});
}
bool OrRequirement::isSatisfied(const ActionContext &ctxt) const
{
	return m_children.empty() ||
			std::any_of(std::begin(m_children), std::end(m_children),
						[ctxt](const Requirement::Ptr &ptr) { return ptr->isSatisfied(ctxt); });
}

BuildTypeRequirement::BuildTypeRequirement(const QString &configuration)
	: m_configuration(QString(configuration).toLower()) {}

QJsonObject BuildTypeRequirement::toJson() const
{
	return QJsonObject({qMakePair(QStringLiteral("os"), m_configuration)});
}

bool BuildTypeRequirement::isSatisfied(const ActionContext &ctxt) const
{
	const PackageConfiguration::BuildTypes type = ctxt.get<ConfigurationContextItem>().config.buildType();
	if (m_configuration == "debug" && type == PackageConfiguration::Debug) {
		return true;
	} else if (m_configuration == "release" && type == PackageConfiguration::Release) {
		return true;
	} else {
		return false;
	}
}

}
}
