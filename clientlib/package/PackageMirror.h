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

	static PackageMirror fromJson(const QJsonObject &obj);

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
