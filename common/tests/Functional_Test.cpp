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

#include "Functional.h"

#include <functional>
#include <vector>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <cassert>
#include <iostream>

using namespace Ralph::Common::Functional;

template <typename A, typename B>
std::ostream &operator<<(std::ostream &str, const std::pair<A, B> &pair)
{
	return str << '{' << pair.first << "; " << pair.second << '}';
}
template <typename T, typename = std::enable_if_t<ContainerTraits<T>::IsContainer::value>>
std::ostream &operator<<(std::ostream &str, const T &container)
{
	for (const auto &element : container) {
		str << element << ' ';
	}
	return str;
}

int free_function(const std::string &, bool)
{
	return 42;
}

DEFINE_HAS_MEMBER(size, size);
DEFINE_HAS_MEMBER(asdf, asdf);
void test_HasMember()
{
	static_assert(HasMember_size<std::vector<int>>::value, "");
	static_assert(!HasMember_asdf<std::vector<int>>::value, "");
}
void test_FunctionTraits()
{
	using FreeFunction = FunctionTraits<decltype(free_function)>;

	static_assert(FreeFunction::arity == 2, "");
	static_assert(std::is_same<FreeFunction::ReturnType, int>::value, "");
	static_assert(std::is_same<FreeFunction::Argument<0>::Type, const std::string &>::value, "");
	static_assert(std::is_same<FreeFunction::Argument<1>::Type, bool>::value, "");

	using FreeFunctionPointer = FunctionTraits<decltype(&free_function)>;

	static_assert(FreeFunctionPointer::arity == 2, "");
	static_assert(std::is_same<FreeFunctionPointer::ReturnType, int>::value, "");
	static_assert(std::is_same<FreeFunctionPointer::Argument<0>::Type, const std::string &>::value, "");
	static_assert(std::is_same<FreeFunctionPointer::Argument<1>::Type, bool>::value, "");

	auto lambda = [](const std::string &, bool) -> int { return 42; };
	using Lambda = FunctionTraits<decltype(lambda)>;

	static_assert(Lambda::arity == 2, "");
	static_assert(std::is_same<Lambda::ReturnType, int>::value, "");
	static_assert(std::is_same<Lambda::Argument<0>::Type, const std::string &>::value, "");
	static_assert(std::is_same<Lambda::Argument<1>::Type, bool>::value, "");

	std::function<int(const std::string &, bool)> function;
	using Function = FunctionTraits<decltype(function)>;

	static_assert(Function::arity == 2, "");
	static_assert(std::is_same<Function::ReturnType, int>::value, "");
	static_assert(std::is_same<Function::Argument<0>::Type, const std::string &>::value, "");
	static_assert(std::is_same<Function::Argument<1>::Type, bool>::value, "");
}
void test_ContainerTraits()
{
	static_assert(ContainerTraits<int>::IsContainer::value == false, "");
	static_assert(ContainerTraits<std::vector<int>>::IsContainer::value == true, "");
	static_assert(ContainerTraits<std::vector<int>>::arity == 2, "");
	static_assert(std::is_same<ContainerTraits<std::vector<int>>::ValueType<0>::Type, int>::value, "");
	static_assert(std::is_same<ContainerTraits<std::vector<int>>::ContainerType<double>, std::vector<double>>::value, "");
	static_assert(ContainerTraits<std::vector<int>>::Associative::value == false, "");
	static_assert(ContainerTraits<std::map<int, bool>>::Associative::value == true, "");

	std::vector<int> vector = {1, 2, 3};
	ContainerTraits<std::vector<int>>::add_element(vector, 4);
	assert(vector == std::vector<int>({1, 2, 3, 4}));

	std::set<int> set = {1, 2, 3};
	ContainerTraits<std::set<int>>::add_element(set, 4);
	assert(set == std::set<int>({1, 2, 3, 4}));

	std::unordered_set<int> unordered_set = {1, 2, 3};
	ContainerTraits<std::unordered_set<int>>::add_element(unordered_set, 4);
	assert(unordered_set == std::unordered_set<int>({1, 2, 3, 4}));

	using map_t = std::map<int, std::string>;
	map_t map = {{1, "1"}, {2, "2"}, {3, "3"}};
	ContainerTraits<map_t>::add_element(map, 4, "4");
	assert(map == map_t({{1, "1"}, {2, "2"}, {3, "3"}, {4, "4"}}));
}
void test_map()
{
	using std::string;
	using std::vector;
	using std::unordered_map;

	vector<int> in1 = {1, 2, 3};

	// map, same container
	vector<string> out1 = map(in1, static_cast<string(*)(int)>(&std::to_string));
	assert(out1 == vector<string>({"1", "2", "3"}));

	// map, same container
	vector<int> out2 = map(in1, [](int i) { return i * 2; });
	assert(out2 == vector<int>({2, 4, 6}));

	// map(2), different container
	auto out3 = map2<std::set<int>>(in1, [](int i) { return i; });
	static_assert(std::is_same<decltype(out3), std::set<int>>::value, "");
	assert(out3 == std::set<int>({1, 2, 3}));

	auto lambda = [](int a, double b) { return std::make_pair(std::to_string(a), std::to_string(b)); };
	auto lambdaValue = [](double b) { return std::to_string(b); };
	auto lambdaKey = [](int a) { return std::to_string(a); };

	unordered_map<int, double> in2 = {std::make_pair(1, 0.5), std::make_pair(2, 1), std::make_pair(3, 1.5)};

	// map, same container, key/value-pair expansion
	unordered_map<string, string> out4 = map(in2, lambda);
	unordered_map<string, string> expectedOut4 = {std::make_pair("1", "0.500000"), std::make_pair("2", "1.000000"), std::make_pair("3", "1.500000")};
	assert(out4 == expectedOut4);

	// map(2), different container, key/value-pair expansion
	std::map<string, string> out5 = map2<std::map<string, string>>(in2, lambda);
	std::map<string, string> expectedOut5 = std::map<string, string>(expectedOut4.begin(), expectedOut4.end());
	assert(out5 == expectedOut5);

	// mapKey, same container
	unordered_map<string, double> out6 = mapKey(in2, lambdaKey);
	unordered_map<string, double> expectedOut6 = {std::make_pair("1", 0.5), std::make_pair("2", 1), std::make_pair("3", 1.5)};
	assert(out6 == expectedOut6);

	// mapKey, different container
	std::map<string, double> out7 = mapKey<std::map<string, double>>(in2, lambdaKey);
	std::map<string, double> expectedOut7 = std::map<string, double>(expectedOut6.begin(), expectedOut6.end());
	assert(out7 == expectedOut7);

	// mapValue, same container
	unordered_map<int, string> out8 = mapValue(in2, lambdaValue);
	unordered_map<int, string> expectedOut8 = {std::make_pair(1, "0.500000"), std::make_pair(2, "1.000000"), std::make_pair(3, "1.500000")};
	assert(out8 == expectedOut8);

	// mapValue, different container
	std::map<int, string> out9 = mapValue<std::map<int, string>>(in2, lambdaValue);
	std::map<int, string> expectedOut9 = std::map<int, string>(expectedOut8.begin(), expectedOut8.end());
	assert(out9 == expectedOut9);
}
void test_filter()
{
	using std::vector;

	vector<int> in1 = {1, 2, 3};

	// filter, same container, simple
	assert(filter(in1, [](int) { return false; }).empty());
	assert(filter(in1, [](int i) { return i == 1; }) == std::vector<int>({1}));

	// filter(2), different container, simple
	assert(filter2<std::set<int>>(in1, [](int i) { return i == 1; }) == std::set<int>({1}));
}
void test_Collection()
{
	std::vector<int> vector = {1, 2, 3};

	assert(collection(vector).map([](int i) { return i * 2; }).filter([](int i) { return i < 6; }).type<std::set<int>>().get() == std::set<int>({2, 4}));
}

int main()
{
	std::cout << std::boolalpha;

	test_HasMember();
	test_FunctionTraits();
	test_ContainerTraits();
	test_map();
	test_filter();
	test_Collection();

	return 0;
}
