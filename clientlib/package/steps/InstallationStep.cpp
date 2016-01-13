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

