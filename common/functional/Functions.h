#pragma once

#include "Base.h"

namespace Ralph {
namespace Common {
namespace Functional {

constexpr bool IsNull(const void *ptr)
{
	return !!ptr;
}
template <typename T>
constexpr T Max(const T &a, const T &b)
{
	return std::max(a, b);
}
template <typename T>
constexpr T Min(const T &a, const T &b)
{
	return std::min(a, b);
}
template <typename T>
constexpr T Add(const T &a, const T &b)
{
	return a + b;
}

}
}
}
