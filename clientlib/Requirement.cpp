#include "Requirement.h"

#include "Json.h"
#include "Functional.h"

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
		} else if (type == "configuration") {
			return std::make_shared<ConfigurationRequirement>(ensureString(obj, "config"));
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
QJsonValue OsRequirement::toJson() const
{
	switch (m_os)
	{
	case Linux: return "linux";
	case OSX: return "osx";
	case Windows: return "windows";
	case Invalid: return QJsonValue();
	}
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

QJsonValue AndRequirement::toJson() const
{
	return Requirement::toJson(m_children);
}
bool AndRequirement::isSatisfied() const
{
	return m_children.empty() ||
			std::all_of(std::begin(m_children), std::end(m_children),
						[](const Requirement::Ptr &ptr) { return ptr->isSatisfied(); });
}

QJsonValue OrRequirement::toJson() const
{
	return Requirement::toJson(m_children);
}
bool OrRequirement::isSatisfied() const
{
	return m_children.empty() ||
			std::any_of(std::begin(m_children), std::end(m_children),
						[](const Requirement::Ptr &ptr) { return ptr->isSatisfied(); });
}

ConfigurationRequirement::ConfigurationRequirement(const QString &configuration)
	: m_configuration(configuration) {}

QJsonValue ConfigurationRequirement::toJson() const
{
	return m_configuration;
}

}
}
