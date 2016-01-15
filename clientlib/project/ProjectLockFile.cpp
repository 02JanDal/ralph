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

#include "ProjectLockFile.h"

#include "Version.h"
#include "Json.h"
#include "package/Package.h"
#include "package/PackageGroup.h"
#include "Project.h"

namespace Ralph {
namespace ClientLib {

ProjectLockFile::ProjectLockFile(const Project *project)
	: m_project(project)
{
	read();
}

void ProjectLockFile::setPackage(const Package *pkg, const PackageGroup *group)
{
	m_versions[pkg->name()] = pkg->version().toString();
	m_groups[pkg->name()] = group->name();
	write();
}
Version ProjectLockFile::getVersion(const QString &name) const
{
	return Version::fromString(m_versions[name]);
}
QString ProjectLockFile::getGroup(const QString &name) const
{
	return m_groups[name];
}
bool ProjectLockFile::contains(const QString &name) const
{
	return m_versions.contains(name);
}

void ProjectLockFile::write() const
{
	QJsonObject obj;
	obj.insert("versions", Json::toJsonObject(m_versions));
	obj.insert("groups", Json::toJsonObject(m_groups));
	Json::write(obj, filename());
}
void ProjectLockFile::read()
{
	const QJsonObject obj = Json::ensureObject(Json::ensureDocument(filename()));
	m_versions = Json::ensureIsHashOf<QString>(obj, "versions");
	m_groups = Json::ensureIsHashOf<QString>(obj, "groups");
}

QString ProjectLockFile::filename() const
{
	return m_project->dir().absoluteFilePath(".ralph.json.lock");
}

}
}
