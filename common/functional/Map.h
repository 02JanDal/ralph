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
#include "Eval.h"

namespace Ralph {
namespace Common {
namespace Functional {

template <typename OutputContainer, typename InputContainer, typename Func>
auto map2(const InputContainer &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	static_assert(ContainerTraits<InputContainer>::IsContainer::value, "error: the input is not a container");
	static_assert(ContainerTraits<OutputContainer>::IsContainer::value, "error: the output is not a container");
	static_assert(ContainerTraits<InputContainer>::arity >= FunctionTraits<Func>::arity, "error: func needs an arity equal to or less than the input container");
	OutputContainer output;
	std::transform(std::begin(input), std::end(input), ContainerTraits<OutputContainer>::InsertionIterator(output), std::forward<Func>(func));
	return output;
}
template <typename OutputContainer, typename InputContainer, typename Func>
auto map2(const InputContainer &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 2>* = nullptr)
{
	static_assert(ContainerTraits<InputContainer>::IsContainer::value, "error: the input is not a container");
	static_assert(ContainerTraits<OutputContainer>::IsContainer::value, "error: the output is not a container");
	static_assert(ContainerTraits<InputContainer>::arity >= FunctionTraits<Func>::arity, "error: func needs an arity equal to or less than the input container");

	// special check for this overload
	static_assert(ContainerTraits<InputContainer>::Associative::value, "error: this version requires the container to be associative");

	using FuncRet = typename FunctionTraits<Func>::ReturnType;
	static_assert(ContainerTraitsDetails::HasMember_first<FuncRet>::value, "error: this version requires the callable to return a pair");
	static_assert(ContainerTraitsDetails::HasMember_second<FuncRet>::value, "error: this version requires the callable to return a pair");

	using PairType = typename ContainerTraits<InputContainer>::IteratorTraits::value_type;
	return map2<OutputContainer, InputContainer>(input, [func](const PairType &pair) { return func(pair.first, pair.second); });
}

template <typename Container, typename Func>
auto map(const Container &input, Func &&func,
		 std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using OutputContainer = typename ContainerTraits<Container>::template ContainerType<typename FunctionTraits<Func>::ReturnType>;
	return map2<OutputContainer, Container, Func>(input, std::forward<Func>(func));
}
template <typename Container, typename Func>
auto map(const Container &input, Func &&func,
		 std::enable_if_t<FunctionTraits<Func>::arity == 2>* = nullptr)
{
	using FuncRet = typename FunctionTraits<Func>::ReturnType;
	using OutputContainer = typename ContainerTraits<Container>::template ContainerType<typename FuncRet::first_type, typename FuncRet::second_type>;
	return map2<OutputContainer, Container, Func>(input, std::forward<Func>(func));
}
template <typename Container, typename Func>
auto map(const Container &input, Func &&func,
		 std::enable_if_t<FunctionTraits<Func>::arity == 0>* = nullptr)
{
	using Type = typename ContainerTraits<Container>::InsertType;
	return map<Container>(input, [&func](const Type &val) { return eval(std::forward<Func>(func), val); });
}

template <typename OutputContainer, typename InputContainer, typename Func>
auto mapValue(const InputContainer &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using OutputPairType = typename ContainerTraits<OutputContainer>::IteratorTraits::value_type;
	using InputPairType = typename ContainerTraits<InputContainer>::IteratorTraits::value_type;
	return map2<OutputContainer, InputContainer>(input, [func](const InputPairType &pair)
	{
		return OutputPairType{pair.first, func(pair.second)};
	});
}
template <typename Container, typename Func>
auto mapValue(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using OutputContainer = typename ContainerTraits<Container>::template ContainerType<
		typename ContainerTraits<Container>::template ValueType<0>::Type,
		typename FunctionTraits<Func>::ReturnType>;
	return mapValue<OutputContainer, Container, Func>(input, func);
}

template <typename OutputContainer, typename InputContainer, typename Func>
auto mapKey(const InputContainer &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using OutputPairType = typename ContainerTraits<OutputContainer>::IteratorTraits::value_type;
	using InputPairType = typename ContainerTraits<InputContainer>::IteratorTraits::value_type;
	return map2<OutputContainer, InputContainer>(input, [func](const InputPairType &pair)
	{
		return OutputPairType{func(pair.first), pair.second};
	});
}
template <typename Container, typename Func>
auto mapKey(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using OutputContainer = typename ContainerTraits<Container>::template ContainerType<
		typename FunctionTraits<Func>::ReturnType,
		typename ContainerTraits<Container>::template ValueType<1>::Type>;
	return mapKey<OutputContainer, Container, Func>(input, func);
}

}
}
}
