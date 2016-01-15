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

#include <functional>

namespace Ralph {
namespace Common {
namespace Functional {
template <typename...> using void_t = void;

#define DEFINE_HAS_MEMBER_HELPER(name, member)			\
	template <typename T, typename... A>		\
	using HasMemberFunctionHelper_##name = void_t<decltype(std::declval<T>().member(std::declval<A>()...))>

#define DEFINE_HAS_MEMBER(name, member)																				\
	template <typename Class, typename... Args>																		\
	class HasMemberFunction_##name##_Impl																			\
	{																												\
		template <typename T, typename... A>																		\
		using HasMemberFunctionHelper_##name =																		\
					::Ralph::Common::Functional::void_t<decltype(std::declval<T>().member(std::declval<A>()...))>;	\
		template <typename C> static std::true_type test(HasMemberFunctionHelper_##name<C, Args...>*);				\
		template <typename> static std::false_type test(...);														\
	public:																											\
		using result = decltype(test<Class>(nullptr));																\
	};																												\
	template <typename Class, typename... Args>																		\
	using HasMemberFunction_##name = typename HasMemberFunction_##name##_Impl<Class, Args...>::result;				\
	template <typename Class>																						\
	class HasMember_##name##_Impl																					\
	{																												\
		template <typename C> static std::true_type test(::Ralph::Common::Functional::void_t<decltype(&C::member)>*);\
		template <typename> static std::false_type test(...);														\
	public:																											\
		using result = decltype(test<Class>(nullptr));																\
	};																												\
	template <typename Class, typename...>																			\
	using HasMember_##name = typename HasMember_##name##_Impl<Class>::result

namespace StaticIfDetail
{
struct Identity
{
	template <typename T>
	T operator()(T &&t) const { return std::forward<T>(t); }
};
template <bool Condition>
struct Statement
{
	template <typename F> void then(const F &f) { f(Identity()); }
	template <typename F> void else_(const F&) {}
};
template <>
struct Statement<false>
{
	template <typename F> void then(const F &) {}
	template <typename F> void else_(const F&f) { f(Identity()); }
};
}
template <bool Condition, typename F>
StaticIfDetail::Statement<Condition> static_if(F const &f)
{
	StaticIfDetail::Statement<Condition> if_;
	if_.then(f);
	return if_;
}

}
}
}
