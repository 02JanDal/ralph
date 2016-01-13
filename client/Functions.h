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

#include "package/PackageDatabase.h"

namespace Ralph {
using namespace ClientLib;

namespace Common {
namespace CommandLine {
class Result;
}
}

namespace Client {

class State
{
public:
	explicit State();

	void setDir(const QString &dir);

	void removePackage(const Common::CommandLine::Result &result);
	void installPackage(const Common::CommandLine::Result &result);
	void checkPackage(const Common::CommandLine::Result &result);
	void searchPackages(const Common::CommandLine::Result &result);

	void verifyProject();
	void newProject(const Common::CommandLine::Result &result);

	void updateSources(const Common::CommandLine::Result &result);
	void addSource(const Common::CommandLine::Result &result);
	void removeSource(const Common::CommandLine::Result &result);
	void listSources(const Common::CommandLine::Result &result);
	void showSource(const Common::CommandLine::Result &result);

	void info();

protected:
	Future<PackageDatabase *> createDB();

	PackageDatabase *m_db = nullptr;
	QString m_dir;
};

}
}
