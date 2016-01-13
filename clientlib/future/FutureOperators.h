#pragma once

#include "Future.h"
#include "task/Task.h"

namespace Ralph {
namespace ClientLib {

template <typename A, typename B>
inline auto operator +(Future<A> a, Future<B> b)
{
	return async(std::launch::deferred, [&a, &b]() { return a.result() + b.result(); });
}
template <typename A, typename B>
inline auto operator -(Future<A> a, Future<B> b)
{
	return async(std::launch::deferred, [&a, &b]() { return a.result() - b.result(); });
}
template <typename A, typename B>
inline auto operator *(Future<A> a, Future<B> b)
{
	return async(std::launch::deferred, [&a, &b]() { return a.result() * b.result(); });
}
template <typename A, typename B>
inline auto operator /(Future<A> a, Future<B> b)
{
	return async(std::launch::deferred, [&a, &b]() { return a.result() / b.result(); });
}
template <typename A, typename B>
inline auto operator %(Future<A> a, Future<B> b)
{
	return async(std::launch::deferred, [&a, &b]() { return a.result() % b.result(); });
}

template <typename A, typename B>
inline auto operator &&(Future<A> a, Future<B> b)
{
	return async(std::launch::deferred, [&a, &b]() { return a.result() && b.result(); });
}
template <typename A, typename B>
inline auto operator ||(Future<A> a, Future<B> b)
{
	return async(std::launch::deferred, [&a, &b]() { return a.result() || b.result(); });
}

}
}
