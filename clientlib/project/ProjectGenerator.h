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

	Task<Project *>::Ptr generate() const;
};
}
}
