#include "PackageMirror.h"

#include "Json.h"
#include "FileSystem.h"
#include "Requirement.h"
#include "Package.h"
#include "Functional.h"
#include "steps/InstallationStep.h"

namespace Ralph {
namespace ClientLib {

PackageMirror::PackageMirror() {}

QJsonObject PackageMirror::toJson() const
{
	QJsonObject obj;
	obj.insert("requirements", m_requirement->toJson());
	obj.insert("steps", Json::toJsonArray(m_steps));
	return obj;
}
std::shared_ptr<PackageMirror> PackageMirror::fromJson(const QJsonObject &obj)
{
	using namespace Json;

	std::shared_ptr<PackageMirror> candidate = std::make_shared<PackageMirror>();

	if (obj.contains("git")) {
		const QJsonObject gitObj = QJsonObject({qMakePair(QStringLiteral("url"), obj.value("git"))});
		candidate->m_steps.append(std::shared_ptr<InstallationStep>(InstallationStep::create("git-clone", gitObj)));
	}

	candidate->m_steps.append(Common::Functional::map(ensureIsArrayOf<QJsonValue>(obj, "steps", QVector<QJsonValue>()), [](const QJsonValue &val)
	{
		if (val.isString()) {
			return std::shared_ptr<InstallationStep>(InstallationStep::create(val.toString(), QJsonObject()));
		} else {
			const QJsonObject object = ensureObject(val);
			return std::shared_ptr<InstallationStep>(InstallationStep::create(ensureString(object, "type"), object));
		}
	}));

	candidate->setRequirement(std::make_unique<AndRequirement>(AndRequirement::fromJson(ensureArray(obj, "requirements", QJsonArray()))));
	return candidate;
}

Future<void> PackageMirror::install(const ActionContext &ctxt) const
{
	return async([this, ctxt](Notifier notifier)
	{
		for (const auto &step : m_steps) {
			notifier.status("Running step '%1'..." % step->type());
			notifier.await(step->perform(ctxt));
		}
	});
}

}
}
