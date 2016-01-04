#pragma once

#include <utility>

// TODO replace by std::optional once it's been released

namespace Ralph {
namespace Common {

template <typename T>
class Optional
{
	T m_value;
	const bool m_valid = false;

public:
	constexpr Optional(T &&v) : m_value(std::forward<T>(v)), m_valid(true) {}
	constexpr explicit Optional() : m_valid(false) {}

	constexpr operator bool() const { return m_valid; }

	constexpr const T *operator->() const { return &m_value; }
	constexpr T *operator->() { return &m_value; }

	constexpr const T &operator*() const & { return m_value; }
	constexpr T &operator*() & { return m_value; }

	constexpr const T &&operator*() const && { return m_value; }
	constexpr T &&operator*() && { return m_value; }

	constexpr const T value() const { return m_value; }
};

}
}
