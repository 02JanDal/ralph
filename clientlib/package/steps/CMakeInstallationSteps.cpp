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

#include "CMakeInstallationSteps.h"

#include "Json.h"
#include "task/Process.h"
#include "FileSystem.h"

namespace Ralph {
namespace ClientLib {

CMakeConfigStep::CMakeConfigStep() {}

QJsonValue CMakeConfigStep::toJson() const
{
	return type();
}
void CMakeConfigStep::fromJsonObject(const QJsonObject &obj)
{
	Q_UNUSED(obj)
}
Future<void> CMakeConfigStep::perform(const ActionContext &ctxt)
{
	return async([this, ctxt](Notifier notifier)
	{
		FS::ensureExists(ctxt.get<InstallContextItem>().buildDir.absoluteFilePath("build"));

		Process proc;
		proc.setExecutable("cmake");
		proc.setArguments(QVector<QString>() << "-DCMAKE_INSTALL_PREFIX=" + ctxt.get<InstallContextItem>().targetDir.absolutePath() << "..");
		proc.setWorkingDirectory(ctxt.get<InstallContextItem>().buildDir.absoluteFilePath("build"));
		notifier.await(proc.run());
	});
}

CMakeBuildStep::CMakeBuildStep() {}

QJsonValue CMakeBuildStep::toJson() const
{
	return QJsonObject({
						   qMakePair(QStringLiteral("type"), type()),
						   qMakePair(QStringLiteral("targets"), Json::toJsonArray(m_targets))
					   });
}
void CMakeBuildStep::fromJsonObject(const QJsonObject &obj)
{
	m_targets = Json::ensureIsArrayOf<QString>(obj, "targets", QVector<QString>() << "install");
}
Future<void> CMakeBuildStep::perform(const ActionContext &ctxt)
{
	return async([this, ctxt](Notifier notifier)
	{
		Process proc;
		proc.setExecutable("cmake");
		proc.setWorkingDirectory(ctxt.get<InstallContextItem>().buildDir.absoluteFilePath("build"));
		for (const QString &target : m_targets) {
			notifier.status("Building target %1..." % target);
			proc.setArguments(QVector<QString>() << "--build" << "." << "--target" << target);
			notifier.await(proc.run());
		}
	});
}

}
}

