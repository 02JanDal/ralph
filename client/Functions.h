#pragma once

#include <QString>

#include "PackageDatabase.h"

namespace Ralph {
using namespace ClientLib;

namespace Client {

class State
{
public:
	int updatePackageDB();
	int installForProject();
	int removePackage(const QString &package, const QString &version);
	int installPackage(const QString &package, const QString &version);
	int checkPackage(const QString &package, const QString &version);

	void setDir(const QString &dir);

private:
	Task<PackageDatabase *>::Ptr createDB();

	PackageDatabase *m_db = nullptr;
	QString m_dir;
};

}
}
