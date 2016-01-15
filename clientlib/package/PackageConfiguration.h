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

#include <QHash>
#include <QVariant>

namespace Ralph {
namespace ClientLib {

class PackageConfiguration
{
public:
	explicit PackageConfiguration();

	enum PredefinedKeys
	{
		BuildType
	};
	static QString key(const PredefinedKeys key);

	void set(const PredefinedKeys k, const QVariant &value) { set(key(k), value); }
	void set(const QString &key, const QVariant &value) { m_values[key] = value; }
	QVariant get(const PredefinedKeys k) const { return get(key(k)); }
	QVariant get(const QString &key) const { return m_values.value(key); }

	template <typename T>
	T get(const QString &key) const { return get(key).value<T>(); }
	template <typename T>
	T get(const PredefinedKeys k) const { return get(k).value<T>(); }

	bool isEmpty() const { return m_values.isEmpty(); }

	static PackageConfiguration fromItems(const QVector<QString> &items);

public: // convenience helpers
	enum BuildTypes
	{
		Debug,
		Release,
		InvalidBuildType
	};
	BuildTypes buildType() const;
	void setBuildType(const BuildTypes &type);

public: // serialization
	QJsonObject toJson() const;
	static PackageConfiguration fromJson(const QJsonObject &obj);

private:
	QHash<QString, QVariant> m_values;
};

}
}

Q_DECLARE_METATYPE(Ralph::ClientLib::PackageConfiguration::BuildTypes)
