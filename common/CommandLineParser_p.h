#pragma once

#include <QString>
#include <QDir>

#include "Functional.h"

namespace Ralph {
namespace Common {
namespace CommandLine {
class Result;
class Option;
namespace detail {

template <typename T> struct Type {};
template <std::size_t V> struct Value {};

QString valueOf(const Option &option, const Result &result);

static inline QDir convert(const QString &in, Type<QDir>) { return in; }
static inline QString convert(const QString &in, Type<QString>) { return in; }
static inline double convert(const QString &in, Type<double>) { return in.toDouble(); }
static inline float convert(const QString &in, Type<float>) { return in.toFloat(); }
static inline int convert(const QString &in, Type<int>) { return in.toInt(); }
static inline unsigned int convert(const QString &in, Type<unsigned int>) { return in.toUInt(); }
static inline long convert(const QString &in, Type<long>) { return in.toLong(); }
static inline long long convert(const QString &in, Type<long long>) { return in.toLongLong(); }
static inline unsigned long convert(const QString &in, Type<unsigned long>) { return in.toULong(); }
static inline unsigned long long convert(const QString &in, Type<unsigned long long>) { return in.toULongLong(); }

template <typename Func>
using FirstArgumentType = typename Functional::FunctionTraits<Func>::template Argument<0>::Type;

template <typename Func>
static std::function<void(const Result &)> callback2(Func &&func, const Option &, Type<Result>) { return func; }
template <typename Func, typename T>
static std::function<void(const Result &)> callback2(Func &&func, const Option &option, Type<T>)
{
	return [func, &option](const Result &r) { func(detail::convert(valueOf(option, r), Type<T>())); };
}
template <typename Func>
static std::function<void(const Result &)> callback1(Func &&func, const Option &, Value<0>) { return [func](const Result &) { func(); }; }
template <typename Func>
static std::function<void(const Result &)> callback1(Func &&func, const Option &option, Value<1>)
{
	using T = typename Functional::FunctionTraits<Func>::template Argument<0>::Type;
	return callback2(func, option, Type<std::remove_cv_t<std::remove_reference_t<T>>>());
}
}
}
}
}
