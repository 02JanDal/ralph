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

#include <QString>

#include "task/Task.h"

namespace Ralph {
namespace ClientLib {
class Project;

class ProjectGenerator
{
public:
	explicit ProjectGenerator();

	QString name() const { return m_name; }
	void setName(const QString &name) { m_name = name; }
	QString vcs() const { return m_vcs; }
	void setVCS(const QString &vcs) { m_vcs = vcs; }
	QString buildSystem() const { return m_bs; }
	void setBuildSystem(const QString &bs) { m_bs = bs; }
	QString directory() const { return m_directory; }
	void setDirectory(const QString &directory) { m_directory = directory; }

	Future<Project *> generate() const;

private:
	QString m_name;
	QString m_vcs;
	QString m_bs;
	QString m_directory;

	void copyTemplate(const QString &name, const QString &destination) const;
};
}
}
