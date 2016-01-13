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

#include <algorithm>

#include "Base.h"
#include "ContainerTraits.h"
#include "FunctionTraits.h"

namespace Ralph {
namespace Common {
namespace Functional {

template <typename Container, typename Func>
void each(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	static_assert(ContainerTraits<Container>::arity >= FunctionTraits<Func>::arity, "error: func needs an arity equal to or less than the input container");
	std::for_each(std::begin(input), std::end(input), std::forward<Func>(func));
}
template <typename Container, typename Func>
void each(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 2>* = nullptr)
{
	static_assert(ContainerTraits<Container>::arity >= FunctionTraits<Func>::arity, "error: func needs an arity equal to or less than the input container");

	// special check for this overload
	static_assert(ContainerTraits<Container>::Associative::value, "error: this version requires the container to be associative");

	using PairType = typename ContainerTraits<Container>::IteratorTraits::value_type;
	each<Container>(input, [func](const PairType &pair) { return func(pair.first, pair.second); });
}

template <typename Container, typename Func>
void eachValue(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using PairType = typename ContainerTraits<Container>::IteratorTraits::value_type;
	each<Container>(input, [func](const PairType &pair) { func(pair.second); });
}

template <typename Container, typename Func>
void eachKey(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using PairType = typename ContainerTraits<Container>::IteratorTraits::value_type;
	each<Container>(input, [func](const PairType &pair) { func(pair.first); });
}
}
}
}
