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

#include <iterator>

#include "Base.h"
#include "FunctionTraits.h"

namespace Ralph {
namespace Common {
namespace Functional {

namespace ContainerTraitsDetails {
DEFINE_HAS_MEMBER(push_back, push_back);
DEFINE_HAS_MEMBER(push_front, push_front);
DEFINE_HAS_MEMBER(insert, insert);
DEFINE_HAS_MEMBER(first, first);
DEFINE_HAS_MEMBER(second, second);

template <bool Condition, typename T = void>
using enif_t = std::enable_if_t<Condition, T>;

template <typename Container>
using InsertionArgumentType = typename std::iterator_traits<typename Container::iterator>::value_type;

template <typename Type>
static constexpr bool has_push_back = HasMemberFunction_push_back<Type, InsertionArgumentType<Type>>::value;
template <typename Type>
static constexpr bool has_push_front = HasMemberFunction_push_front<Type, InsertionArgumentType<Type>>::value;
template <typename Type>
static constexpr bool has_insert = HasMemberFunction_insert<Type, InsertionArgumentType<Type>>::value;

template <typename Type>
static auto InsertionIterator(Type &container, enif_t<has_push_back<Type>>* = nullptr)
{
	return std::back_inserter(container);
}
template <typename Type>
static auto InsertionIterator(Type &container, enif_t<has_push_front<Type> && !has_push_back<Type>>* = nullptr)
{
	return std::front_inserter(container);
}
template <typename Type>
static auto InsertionIterator(Type &container, enif_t<HasMemberFunction_insert<Type, InsertionArgumentType<Type>>::value>* = nullptr)
{
	return std::inserter(container, std::end(container));
}

template <template<typename...> class Cont, typename... Args>
class ContainerTraitsImpl
{
	template <bool Condition, typename T = void>
	using enif_t = std::enable_if_t<Condition, T>;

public:
	using IsContainer = std::true_type;
	using Type = Cont<Args...>;
	using Iterator = typename Type::iterator;
	using IteratorTraits = std::iterator_traits<Iterator>;
	template <typename... OtherArgs>
	using TypeOtherArgs = Cont<OtherArgs...>;

private:
	using ContainerIteratorValueType = typename IteratorTraits::value_type;
public:
	using Associative = std::conditional_t<
			ContainerTraitsDetails::HasMember_first<ContainerIteratorValueType>::value &&
			ContainerTraitsDetails::HasMember_second<ContainerIteratorValueType>::value,
			std::true_type, std::false_type>;

	template <typename... A>
	using ContainerType = Cont<A...>;

	static constexpr std::size_t arity = sizeof...(Args);
	template <std::size_t N>
	struct ValueType
	{
		static_assert(N < arity, "error: invalid value type index");
		using Type = typename std::tuple_element<N, std::tuple<Args...>>::type;
	};
	using InsertType = typename IteratorTraits::value_type;

private:
	static constexpr bool Has_push_back =
			ContainerTraitsDetails::HasMemberFunction_push_back<Type,ContainerTraitsDetails::InsertionArgumentType<Type>>::value;
	static constexpr bool Has_push_front =
			ContainerTraitsDetails::HasMemberFunction_push_front<Type, ContainerTraitsDetails::InsertionArgumentType<Type>>::value;
	static constexpr bool Has_insert =
			ContainerTraitsDetails::HasMemberFunction_insert<Type, ContainerTraitsDetails::InsertionArgumentType<Type>>::value;
public:
	template <typename T>
	static inline void add_element(Type &container, T &&element)
	{
		static_assert(Has_push_back || Has_push_front || Has_insert, "error: container does not support any known insertion method");
		static_if<Has_push_back>([&](auto f) {
			f(container).push_back(element);
		}).else_([&](auto) {
			static_if<Has_push_front>([&](auto g) {
				g(container).push_front(element);
			}).else_([&](auto h) {
				h(container).insert(element);
			});
		});
	}
	template <typename T1, typename T2>
	static inline void add_element(Type &container, T1 &&e1, T2 &&e2)
	{
		static_assert(Associative::value, "error: this overload only works with associative containers");
		add_element(container, std::pair<T1, T2>(std::forward<T1>(e1), std::forward<T2>(e2)));
	}

	template <typename T>
	static inline auto size(const Type &container) -> decltype(container.size())
	{
		return container.size();
	}

	static inline auto InsertionIterator(Type &container)
	{
		return ContainerTraitsDetails::InsertionIterator<Type>(container);
	}
};
}

template <typename Cont>
class ContainerTraits
{
public:
	using IsContainer = std::false_type;
};

// std::pair and clones almost look like containers, but aren't
template <template<typename...> class Cont, typename... Args>
class ContainerTraits<Cont<Args...>>
		: public std::conditional_t<ContainerTraitsDetails::HasMember_second<Cont<Args...>>::value,
									ContainerTraits<void>,
									ContainerTraitsDetails::ContainerTraitsImpl<Cont, Args...>>
{
};

template <typename Cont>
class ContainerTraits<Cont&> : public ContainerTraits<Cont> {};
template <typename Cont>
class ContainerTraits<Cont&&> : public ContainerTraits<Cont> {};

}
}
}
