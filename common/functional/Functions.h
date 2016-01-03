#pragma once

#include "Base.h"

namespace Ralph
{
namespace Common
{
namespace Functional
{
constexpr bool IsNull(const void *ptr)
{
	return !!ptr;
}
}
}
}
