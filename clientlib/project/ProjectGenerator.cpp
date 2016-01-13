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

#include "ProjectGenerator.h"

#include "Project.h"
#include "FileSystem.h"
#include "git/GitRepo.h"
#include "Formatting.h"

namespace Ralph {
namespace ClientLib {
ProjectGenerator::ProjectGenerator() {}

Future<Project *> ProjectGenerator::generate() const
{
	return async([this](Notifier notifier)
	{
		Project *project = new Project();
		const QDir dir = QDir::current().absoluteFilePath(m_directory.isEmpty() ? m_name : m_directory);
		if (dir.exists()) {
			throw Exception("Directory %1 already exists" % dir.absolutePath());
		}
		if (!dir.mkpath(dir.absolutePath())) {
			throw Exception("Unable to create project directory %1" % dir.absolutePath());
		}
		try {
			if (m_vcs == "git") {
				Git::GitRepo *repo = notifier.await(Git::GitRepo::init(dir, project));
				notifier.status("Git repository has been initialized in %1" % repo->dir().absolutePath());
				copyTemplate("gitignore", dir.absoluteFilePath(".gitignore"));
			}
			if (m_bs == "cmake") {
				copyTemplate("CMakeLists.txt", dir.absoluteFilePath("CMakeLists.txt"));
			}
			copyTemplate("ralph.json", dir.absoluteFilePath("ralph.json"));
			copyTemplate("main.cpp", dir.absoluteFilePath("main.cpp"));
			if (m_bs == "cmake") {
				FS::ensureExists(dir.absoluteFilePath("build"));
				notifier.status("Now run 'cmake .. && cmake --build .' in %1" % dir.absoluteFilePath("build"));
			}
		} catch (...) {
			delete project;
			notifier.status("Error detected, rolling back");
			if (!dir.rmdir(dir.absolutePath())) {
				notifier.status("Rollback failed. Manual cleanup required.");
			}
			throw;
		}
		return project;
	});
}

void ProjectGenerator::copyTemplate(const QString &name, const QString &destination) const
{
	const QByteArray data = FS::read(":/resources/" + name + ".template")
			.replace("@PROJECT_NAME@", m_name.toUtf8());
	FS::write(destination, data);
}

}
}
