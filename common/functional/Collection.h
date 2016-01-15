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

#include "ContainerTraits.h"
#include "Map.h"
#include "Filter.h"
#include "Each.h"
#include "Tap.h"
#include "Functions.h"

namespace Ralph {
namespace Common {
namespace Functional {
namespace detail {
template <typename T>
T defaultInitialValue(std::false_type) { return T{}; }
template <typename T>
T defaultInitialValue(std::true_type) { return T(0); }
}

template <typename Cont>
class CollectionImpl
{
public:
	using Traits = ContainerTraits<Cont>;
	static_assert(Traits::IsContainer::value, "error: Cont is not a container");

	using TOne = typename Traits::template ValueType<0>::Type;

	using Type = typename Traits::Type;

	const Type m_collection;

public:
	CollectionImpl(const Type &collection) : m_collection(collection) {}

	template <typename Func>
	inline auto map(Func &&func) const { return CollectionImpl<decltype(Functional::map(m_collection, std::forward<Func>(func)))>(Functional::map(m_collection, std::forward<Func>(func))); }

	template <typename Func>
	inline auto filter(Func &&func) const { return CollectionImpl<decltype(Functional::filter(m_collection, std::forward<Func>(func)))>(Functional::filter(m_collection, std::forward<Func>(func))); }

	template <typename Func>
	inline auto tap(Func &&func) const { return CollectionImpl<decltype(Functional::tap(m_collection, std::forward<Func>(func)))>(Functional::tap(m_collection, std::forward<Func>(func))); }

	template <typename Func>
	inline void each(Func &&func) const { Functional::each(m_collection, std::forward<Func>(func)); }

	template <typename Func, typename FuncTraits = FunctionTraits<Func>, typename T = typename FuncTraits::ReturnType>
	inline T reduce(Func &&func) const
	{
		static_assert(FuncTraits::arity == 2, "");
		static_assert(std::is_convertible<T, typename FuncTraits::template Argument<0>::Type>::value, "");
		static_assert(std::is_convertible<typename Traits::template ValueType<0>::Type, typename FuncTraits::template Argument<1>::Type>::value, "");
		static_assert(std::is_constructible<T>::value, "");
		return std::accumulate(std::begin(m_collection), std::end(m_collection), detail::defaultInitialValue<T>(typename std::is_integral<T>::type{}), std::forward<Func>(func));
	}

	template <typename NewCont>
	inline const CollectionImpl<NewCont> type() const
	{
		return CollectionImpl<NewCont>(Functional::map2<NewCont>(m_collection, [](const typename Traits::InsertType &item) { return item; }));
	}

	inline auto mapSize() const { return map(&TOne::size); }

	inline auto flatten() const
	{
		return CollectionImpl<TOne>(reduce([](TOne a, const TOne &b)
		{
			std::copy(std::begin(b), std::end(b), ContainerTraits<TOne>::InsertionIterator(a));
			return a;
		}));
	}

	template <typename Func>
	inline auto sort(Func &&func)
	{
		Type col = m_collection;
		std::sort(std::begin(col), std::end(col), std::forward<Func>(func));
		return CollectionImpl(col);
	}
	inline auto sort()
	{
		Type col = m_collection;
		std::sort(std::begin(col), std::end(col));
		return CollectionImpl(col);
	}

	inline TOne max() const
	{
		return reduce(&Max<TOne>);
	}
	inline TOne min() const
	{
		return reduce(&Min<TOne>);
	}
	inline TOne sum() const
	{
		return reduce(&Add<TOne>);
	}

	template <typename Glue>
	inline TOne join(const Glue &glue) const
	{
		if (m_collection.size() == 0) {
			return TOne{};
		} else if (m_collection.size() == 1) {
			return *std::begin(m_collection);
		} else {
			TOne result;
			for (int i = 0; i < (m_collection.size()-1); ++i) {
				result += m_collection[i] + glue;
			}
			result += m_collection[m_collection.size()-1];
			return result;
		}
	}

	inline const Type get() const { return m_collection; }

	inline operator Type() const { return m_collection; }
};

template <typename Cont>
auto collection(const Cont &container)
{
	return CollectionImpl<Cont>(container);
}

}
}
}

#undef CoSpMe
#undef CoSpNuMe
