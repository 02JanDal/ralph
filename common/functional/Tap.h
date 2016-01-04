#pragma once

#include <algorithm>

#include "Base.h"
#include "ContainerTraits.h"
#include "FunctionTraits.h"

namespace Ralph {
namespace Common {
namespace Functional {

template <typename Container, typename Func>
Container tap(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	static_assert(ContainerTraits<Container>::arity >= FunctionTraits<Func>::arity, "error: func needs an arity equal to or less than the input container");
	std::for_each(std::begin(input), std::end(input), std::forward<Func>(func));
	return input;
}
template <typename Container, typename Func>
Container tap(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 2>* = nullptr)
{
	static_assert(ContainerTraits<Container>::arity >= FunctionTraits<Func>::arity, "error: func needs an arity equal to or less than the input container");

	// special check for this overload
	static_assert(ContainerTraits<Container>::Associative::value, "error: this version requires the container to be associative");

	using PairType = typename ContainerTraits<Container>::IteratorTraits::value_type;
	tap<Container>(input, [func](const PairType &pair) { return func(pair.first, pair.second); });

	return input;
}

template <typename Container, typename Func>
Container tapValue(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using PairType = typename ContainerTraits<Container>::IteratorTraits::value_type;
	tap<Container>(input, [func](const PairType &pair) { func(pair.second); });
	return input;
}

template <typename Container, typename Func>
Container tapKey(const Container &input, Func &&func, std::enable_if_t<FunctionTraits<Func>::arity == 1>* = nullptr)
{
	using PairType = typename ContainerTraits<Container>::IteratorTraits::value_type;
	tap<Container>(input, [func](const PairType &pair) { func(pair.first); });\
	return input;
}
}
}
}
