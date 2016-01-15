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

#include <QHash>

class QJsonObject;

namespace Ralph {
namespace ClientLib {
class Project;
class Version;
class Package;
class PackageGroup;

class ProjectLockFile
{
public:
	explicit ProjectLockFile(const Project *project);

	void setPackage(const Package *pkg, const PackageGroup *group);
	Version getVersion(const QString &name) const;
	QString getGroup(const QString &name) const;
	bool contains(const QString &name) const;

	void write() const;
	void read();

private:
	const Project *m_project;
	QHash<QString, QString> m_versions;
	QHash<QString, QString> m_groups;

	QString filename() const;
};

}
}
