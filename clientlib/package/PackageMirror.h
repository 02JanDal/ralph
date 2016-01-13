#pragma once

#include <QObject>
#include <QUrl>

#include "task/Task.h"

QT_BEGIN_NAMESPACE
class QDir;
QT_END_NAMESPACE

namespace Ralph {
namespace ClientLib {
class Package;
class InstallationStep;
class ActionContext;
using RequirementPtr = std::shared_ptr<class Requirement>;

class PackageMirror
{
public:
	explicit PackageMirror();

	QJsonObject toJson() const;

	static std::shared_ptr<PackageMirror> fromJson(const QJsonObject &obj);

	RequirementPtr requirement() const { return m_requirement; }
	void setRequirement(const RequirementPtr &requirement) { m_requirement = requirement; }

	QVector<std::shared_ptr<InstallationStep>> steps() const { return m_steps; }
	void setSteps(const QVector<std::shared_ptr<InstallationStep>> &steps) { m_steps = steps; }

	Future<void> install(const ActionContext &ctxt) const;

private:
	RequirementPtr m_requirement;
	QVector<std::shared_ptr<InstallationStep>> m_steps;
};

}
}
