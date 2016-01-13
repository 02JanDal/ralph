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

#include <QUrl>

#include "InstallationStep.h"

namespace Ralph {
namespace ClientLib {

class GitCloneStep : public InstallationStep
{
public:
	explicit GitCloneStep();
	explicit GitCloneStep(const QUrl &url);

	QString type() const override { return "git-clone"; }
	QJsonValue toJson() const override;
	void fromJsonObject(const QJsonObject &object) override;

	Future<void> perform(const ActionContext &ctxt) override;

private:
	QUrl m_url;
};

class GitSubmoduleSetupStep : public InstallationStep
{
public:
	explicit GitSubmoduleSetupStep();

	QString type() const override { return "git-submodule-setup"; }

	Future<void> perform(const ActionContext &ctxt) override;
};

}
}
