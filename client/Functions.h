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

	void verifyProject();
	void newProject(const Common::CommandLine::Result &result);

	void updateSources(const Common::CommandLine::Result &result);
	void addSource(const Common::CommandLine::Result &result);
	void removeSource(const Common::CommandLine::Result &result);

protected:
	Future<PackageDatabase *> createDB();

	PackageDatabase *m_db = nullptr;
	QString m_dir;
};

}
}
