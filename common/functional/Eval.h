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

#include "Base.h"
#include "FunctionTraits.h"

namespace Ralph {
namespace Common {
namespace Functional {

// functions, functors, lambdas, etc.
template<
	typename F, typename... Args,
	typename = typename std::enable_if<!std::is_member_function_pointer<F>::value>::type,
	typename = typename std::enable_if<!std::is_member_object_pointer<F>::value>::type
	>
auto eval(F&& f, Args&&... args)
{
	return f(std::forward<Args>(args)...);
}

// const member function
template<typename R, typename C, typename... Args>
auto eval(R(C::*f)(Args...) const, const C& c, Args&&... args)
{
	return (c.*f)(std::forward<Args>(args)...);
}

template<typename R, typename C, typename... Args>
auto eval(R(C::*f)(Args...) const, C& c, Args&&... args)
{
	return (c.*f)(std::forward<Args>(args)...);
}

// non-const member function
template<typename R, typename C, typename... Args>
auto eval(R(C::*f)(Args...), C& c, Args&&... args)
{
	return (c.*f)(std::forward<Args>(args)...);
}

// member object
template<class R, class C>
auto eval(R(C::*m), const C& c)
{
	return c.*m;
}

template<class R, class C>
auto eval(R(C::*m), C& c)
{
	return c.*m;
}

}
}
}
