#pragma once

#include "ContainerTraits.h"
#include "Map.h"
#include "Filter.h"
#include "Each.h"
#include "Tap.h"

namespace Ralph {
namespace Common {
namespace Functional {

template <typename Cont>
class CollectionImpl
{
public:
	using Traits = ContainerTraits<Cont>;
	static_assert(Traits::IsContainer::value, "error: Cont is not a container");

	using Type = typename Traits::Type;

	const Type m_collection;

public:
	CollectionImpl(const Type &collection) : m_collection(collection) {}

	template <typename Func>
	inline const CollectionImpl map(Func &&func) const { return CollectionImpl(Functional::map(m_collection, std::forward<Func>(func))); }

	template <typename Func>
	inline const CollectionImpl filter(Func &&func) const { return CollectionImpl(Functional::filter(m_collection, std::forward<Func>(func))); }

	template <typename Func>
	inline const CollectionImpl tap(Func &&func) const { return CollectionImpl(Functional::tap(m_collection, std::forward<Func>(func))); }

	template <typename Func>
	inline void each(Func &&func) const { Functional::each(m_collection, std::forward<Func>(func)); }

	template <typename NewCont>
	inline const CollectionImpl<NewCont> type() const
	{
		return CollectionImpl<NewCont>(Functional::map2<NewCont>(m_collection, [](const typename Traits::InsertType &item) { return item; }));
	}

	inline const Type get() const { return m_collection; }

	inline operator Type() const { return m_collection; }
};

template <typename Cont>
auto collection(Cont &&container)
{
	return CollectionImpl<decltype(container)>(std::forward<Cont>(container));
}
}
}
}
