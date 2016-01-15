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

#include <QPair>
#include <QVector>

namespace Ralph {
namespace ClientLib {

class Version
{
	using Section = QPair<QString, int>;

public:
	enum Type
	{
		Custom,

		Alpha,
		Beta,
		ReleaseCandidate,
		Stable
	};

	explicit Version();

	bool isValid() const { return m_isValid; }

	QString typeString() const { return m_typeString; }
	Type type() const { return m_type; }

	inline bool operator<(const Version &other) const { return compareWith(other) == -1; }
	inline bool operator<=(const Version &other) const { return compareWith(other) != 1; }
	inline bool operator>(const Version &other) const { return compareWith(other) == 1; }
	inline bool operator>=(const Version &other) const { return compareWith(other) != -1; }
	inline bool operator==(const Version &other) const { return compareWith(other) == 0; }
	inline bool operator!=(const Version &other) const { return compareWith(other) != 0; }

	QString toString() const;

	static Version fromString(const QString &string);
	static Type typeFromString(const QString &string);

private:
	char compareWith(const Version &other) const;
	static char compareSections(const QVector<Section> &a, const QVector<Section> &b);

private:
	bool m_isValid = false;

	QString m_typeString;
	Type m_type = Custom;
	QVector<QVector<Section>> m_sections;
};

class VersionRequirement
{
public:
	explicit VersionRequirement();

	enum Type
	{
		Less, LessEqual,
		Greater, GreaterEqual,
		Equal, NonEqual
	};

	Version version() const { return m_version; }
	void setVersion(const Version &version) { m_version = version; }

	Type type() const { return m_type; }
	void setType(const Type type) { m_type = type; }

	bool accepts(const Version &version) const;

	Version::Type allowedType() const { return m_allowedType; }
	void setAllowedType(const Version::Type allowedType) { m_allowedType = allowedType; }

	bool isValid() const { return m_version.isValid(); }

	QString toString() const;
	static VersionRequirement fromString(const QString &string);

private:
	Version m_version;
	Type m_type;
	QString m_allowedTypeString;
	Version::Type m_allowedType;
};

}
}
