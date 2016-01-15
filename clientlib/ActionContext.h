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

#include <QDir>
#include <unordered_map>
#include <memory>

#include "package/PackageConfiguration.h"

namespace Ralph {
namespace ClientLib {

class BaseContextItem
{
public:
	virtual ~BaseContextItem();
};

class ActionContext
{
	std::unordered_map<std::size_t, std::shared_ptr<BaseContextItem>> m_items;

	template <typename T>
	static constexpr std::size_t key() { return typeid(T).hash_code(); }

public:
	template <typename T>
	const T &get() const { return *std::dynamic_pointer_cast<T>(m_items.at(key<T>())); }

	template <typename T>
	void insert(const T &t) { m_items[key<T>()] = std::make_shared<T>(t); }

	template <typename T, typename... Args>
	void emplace(const Args &... args) { m_items[key<T>()] = std::make_shared<T>(args...); }
};

class InstallContextItem : public BaseContextItem
{
public:
	explicit InstallContextItem(const QDir &target, const QDir &build);
	virtual ~InstallContextItem();

	QDir targetDir;
	QDir buildDir;
};

class ConfigurationContextItem : public BaseContextItem
{
public:
	explicit ConfigurationContextItem(const PackageConfiguration &conf);
	virtual ~ConfigurationContextItem();

	const PackageConfiguration config;
};

}
}
