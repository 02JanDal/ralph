#pragma once

#include <algorithm>

#include "Base.h"
#include "ContainerTraits.h"
#include "FunctionTraits.h"

namespace Ralph
{
namespace Common
{
namespace Functional
{
template <typename OutputContainer, typename InputContainer, typename Func>
auto filter2(const InputContainer &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	static_assert(std::is_same<typename FunctionTraits<Func>::ReturnType, bool>::value, "error: func needs to return a boolean value");
	static_assert(ContainerTraits<InputContainer>::arity >= FunctionTraits<Func>::arity, "error: func needs an arity equal to or less than the input container");
	OutputContainer output;
	std::copy_if(std::begin(input), std::end(input), ContainerTraits<OutputContainer>::InsertionIterator(output), std::forward<Func>(func));
	return output;
}
template <typename OutputContainer, typename InputContainer, typename Func>
auto filter2(const InputContainer &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 2>* = nullptr)
{
	static_assert(ContainerTraits<InputContainer>::arity >= FunctionTraits<Func>::arity, "error: func needs an arity equal to or less than the input container");

	// special check for this overload
	static_assert(ContainerTraits<InputContainer>::Associative::value, "error: this version requires the container to be associative");

	using FuncRet = typename FunctionTraits<Func>::ReturnType;
	static_assert(ContainerTraitsDetails::HasMember_first<FuncRet>::value, "error: this version requires the callable to return a pair");
	static_assert(ContainerTraitsDetails::HasMember_second<FuncRet>::value, "error: this version requires the callable to return a pair");

	using PairType = typename ContainerTraits<InputContainer>::IteratorTraits::value_type;
	return filter2<OutputContainer, InputContainer>(input, [func](const PairType &pair) { return func(pair.first, pair.second); });
}

template <typename Container, typename Func>
auto filter(const Container &input, Func &&func,
		 std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	return filter2<Container, Container, Func>(input, std::forward<Func>(func));
}
template <typename Container, typename Func>
auto filter(const Container &input, Func &&func,
		 std::enable_if_t<FunctionTraits<Func>::arity == 2>* = nullptr)
{
	return filter2<Container, Container, Func>(input, std::forward<Func>(func));
}

template <typename OutputContainer, typename InputContainer, typename Func>
auto filterByValue(const InputContainer &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using OutputPairType = typename ContainerTraits<OutputContainer>::IteratorTraits::value_type;
	using InputPairType = typename ContainerTraits<InputContainer>::IteratorTraits::value_type;
	return filter2<OutputContainer, InputContainer>(input, [func](const InputPairType &pair)
	{
		return OutputPairType{pair.first, func(pair.second)};
	});
}
template <typename Container, typename Func>
auto filterByValue(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	return filterByValue<Container, Container, Func>(input, func);
}

template <typename OutputContainer, typename InputContainer, typename Func>
auto filterByKey(const InputContainer &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using OutputPairType = typename ContainerTraits<OutputContainer>::IteratorTraits::value_type;
	using InputPairType = typename ContainerTraits<InputContainer>::IteratorTraits::value_type;
	return filter2<OutputContainer, InputContainer>(input, [func](const InputPairType &pair)
	{
		return OutputPairType{func(pair.first), pair.second};
	});
}
template <typename Container, typename Func>
auto filterByKey(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	return filterByKey<Container, Container, Func>(input, func);
}
}
}
}
