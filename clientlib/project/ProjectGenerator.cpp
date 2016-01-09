#include "ProjectGenerator.h"

#include "Project.h"

namespace Ralph {
namespace ClientLib {
ProjectGenerator::ProjectGenerator() {}

Future<Project *> ProjectGenerator::generate() const
{
	return async([this](Notifier notifier)
	{
		Q_UNUSED(notifier)
		return new Project();
	});
}
}
}
