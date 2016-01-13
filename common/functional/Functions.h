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
