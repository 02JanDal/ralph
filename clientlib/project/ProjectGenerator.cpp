#include "ProjectGenerator.h"

#include "Project.h"

namespace Ralph {
namespace ClientLib {
ProjectGenerator::ProjectGenerator() {}

Task<Project *>::Ptr ProjectGenerator::generate() const
{
	return createTask([this](Notifier notifier)
	{
		Q_UNUSED(notifier)
		return new Project();
	});
}
}
}
