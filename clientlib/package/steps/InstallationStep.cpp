#include "InstallationStep.h"

#include "GitInstallationSteps.h"
#include "CMakeInstallationSteps.h"
#include "Exception.h"
#include "Json.h"

namespace Ralph {
namespace ClientLib {

InstallationStep::InstallationStep()
{
}

InstallationStep::~InstallationStep()
{
}

void InstallationStep::fromJsonObject(const QJsonObject &object)
{
	Q_UNUSED(object)
}
QJsonValue InstallationStep::toJson() const
{
	return type();
}

std::unique_ptr<InstallationStep> InstallationStep::create(const QString &type, const QJsonObject &obj)
{
	std::unique_ptr<InstallationStep> step;
	if (type == "git-clone") {
		step = std::make_unique<GitCloneStep>();
	} else if (type == "git-submodule-setup") {
		step = std::make_unique<GitSubmoduleSetupStep>();
	} else if (type == "cmake-config") {
		step = std::make_unique<CMakeConfigStep>();
	} else if (type == "cmake-build") {
		step = std::make_unique<CMakeBuildStep>();
	}
	Q_ASSERT_X(step->type() == type, "InstallationStep::create", "error: InstallationStep::type implementation returned wrong value");
	step->fromJsonObject(obj);
	return step;
}

}
}

