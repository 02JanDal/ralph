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

#include <tuple>

namespace Ralph {
namespace Common {
namespace Functional {

// https://functionalcpp.wordpress.com/2013/08/05/function-traits/
template <typename Func>
class FunctionTraits
{
	using CallType = FunctionTraits<decltype(&Func::operator())>;
public:
	using ReturnType = typename CallType::ReturnType;
	static constexpr std::size_t arity = CallType::arity;
	template <std::size_t N>
	struct Argument
	{
		static_assert(N < arity, "error: invalid parameter index");
		using Type = typename CallType::template Argument<N>::Type;
	};
};

// function pointer
template <typename Ret, typename... Args>
class FunctionTraits<Ret(*)(Args...)> : public FunctionTraits<Ret(Args...)> {};

template <typename Ret, typename... Args>
class FunctionTraits<Ret(Args...)>
{
public:
	using ReturnType = Ret;
	static constexpr std::size_t arity = sizeof...(Args);
	template <std::size_t N>
	struct Argument
	{
		static_assert(N < arity, "error: invalid parameter index");
		using Type = typename std::tuple_element<N, std::tuple<Args...>>::type;
	};
};

// member function pointer
template <typename Class, typename Ret, typename... Args>
class FunctionTraits<Ret(Class::*)(Args...)> : public FunctionTraits<Ret(Args...)> {};

// const member function pointer
template <typename Class, typename Ret, typename... Args>
class FunctionTraits<Ret(Class::*)(Args...) const> : public FunctionTraits<Ret(Args...)> {};

// member object pointer
template <typename Class, typename Ret>
class FunctionTraits<Ret(Class::*)> : public FunctionTraits<Ret()> {};

// clean references
template <typename Func>
class FunctionTraits<Func&> : public FunctionTraits<Func> {};
template <typename Func>
class FunctionTraits<Func&&> : public FunctionTraits<Func> {};

}
}
}
