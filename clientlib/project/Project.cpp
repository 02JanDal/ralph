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

#include "Project.h"

#include "Json.h"

namespace Ralph {
namespace ClientLib {

Project::Project(const QDir &dir)
	: Package(), m_dir(dir)
{
}

Project::~Project() {}

const Project *Project::fromJson(const QJsonDocument &doc, const QDir &dir)
{
	Project *project = new Project{dir};
	Package::fromJson(doc, project);
	return project;
}

const Project *Project::load(const QDir &dir)
{
	return fromJson(Json::ensureDocument(dir.absoluteFilePath("ralph.json")), dir);
}

}
}
