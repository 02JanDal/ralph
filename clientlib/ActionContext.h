#pragma once

#include <QDir>
#include <unordered_map>
#include <memory>

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

}
}
