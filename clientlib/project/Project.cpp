#include "Project.h"

namespace Ralph {
namespace ClientLib {

Project::Project(QObject *parent)
	: Package(parent)
{
}

const Project *Project::fromJson(const QJsonDocument &doc)
{
	Project *project = new Project;
	Package::fromJson(doc, project);
	return project;
}

}
}
