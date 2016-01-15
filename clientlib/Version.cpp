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

#include "Version.h"

#include "Functional.h"
#include "Exception.h"

namespace Ralph {
using namespace Common;
namespace ClientLib {

VersionRequirement::VersionRequirement() {}

QString VersionRequirement::toString() const
{
	QString result;
	if (!m_allowedTypeString.isEmpty()) {
		result += m_allowedTypeString + '@';
	}
	switch (m_type) {
	case Ralph::ClientLib::VersionRequirement::Less: result += '<'; break;
	case Ralph::ClientLib::VersionRequirement::LessEqual: result += "<="; break;
	case Ralph::ClientLib::VersionRequirement::Greater: result += '>'; break;
	case Ralph::ClientLib::VersionRequirement::GreaterEqual: result += ">="; break;
	case Ralph::ClientLib::VersionRequirement::Equal: result += "=="; break;
	case Ralph::ClientLib::VersionRequirement::NonEqual: result += "!="; break;
	}
	return result + m_version.toString();
}

VersionRequirement VersionRequirement::fromString(const QString &string)
{
	VersionRequirement version;

	if (string.contains('@')) {
		version.m_allowedTypeString = string.left(string.indexOf('@'));
		version.setAllowedType(Version::typeFromString(version.m_allowedTypeString));
	}

	// if string contains an @ the index will be before the at (+1 results in after the symbol), if it doesn't
	// indexOf returns -1, and -1+1 = 0, thus we take from the start of the string
	const QString str = string.mid(string.indexOf('@')+1);

	if (str.startsWith(">=")) {
		version.setVersion(Version::fromString(str.mid(2)));
		version.setType(GreaterEqual);
	} else if (str.startsWith('>')) {
		version.setVersion(Version::fromString(str.mid(1)));
		version.setType(Greater);
	} else if (str.startsWith("<=")) {
		version.setVersion(Version::fromString(str.mid(2)));
		version.setType(LessEqual);
	} else if (str.startsWith('<')) {
		version.setVersion(Version::fromString(str.mid(1)));
		version.setType(Less);
	} else if (str.startsWith("!=")) {
		version.setVersion(Version::fromString(str.mid(2)));
		version.setType(NonEqual);
	} else if (str.startsWith("==")) {
		version.setVersion(Version::fromString(str.mid(2)));
		version.setType(Equal);
	} else {
		version.setVersion(Version::fromString(str));
		version.setType(Equal);
	}
	if (!version.version().isValid()) {
		throw Exception("Unable to parse version in version requirement: '%1'" % string);
	}
	return version;
}

bool VersionRequirement::accepts(const Version &version) const
{
	if (version.type() < m_allowedType) {
		return false;
	}

	switch (m_type) {
	case VersionRequirement::Less: return version < m_version;
	case VersionRequirement::LessEqual: return version <= m_version;
	case VersionRequirement::Greater: return version > m_version;
	case VersionRequirement::GreaterEqual: return version >= m_version;
	case VersionRequirement::Equal: return version == m_version;
	case VersionRequirement::NonEqual: return version != m_version;
	}
}


Version::Version()
{
}

QString Version::toString() const
{
	QString result;
	if (!m_typeString.isEmpty()) {
		result = m_typeString + '@';
	}
	return Functional::collection(m_sections)
			.map([](const QVector<Section> &sections)
	{
		return Functional::collection(sections)
				.map([](const Section &section) { return section.first.isNull() ? QString::number(section.second) : section.first; })
				.join('.');
	}).join('-');
}

Version Version::fromString(const QString &string)
{
	Version result;

	if (string.contains('@')) {
		result.m_typeString = string.left(string.indexOf('@')).toLower();
		result.m_type = typeFromString(result.m_typeString);
	}

	result.m_sections = Functional::map(string.mid(string.indexOf('@')+1).split('-').toVector(), [](const QString &section)
	{
		return Functional::map(section.split('.').toVector(), [](const QString &subsection)
		{
			bool ok = false;
			const int integer = subsection.toInt(&ok);
			return ok ? qMakePair(QString(), integer) : qMakePair(subsection, 0);
		});
	});

	result.m_isValid = true;

	return result;
}

Version::Type Version::typeFromString(const QString &string)
{
	if (string == "alpha") {
		return Alpha;
	} else if (string == "beta") {
		return Beta;
	} else if (string == "rc") {
		return ReleaseCandidate;
	} else if (string == "stable") {
		return Stable;
	} else {
		return Custom;
	}
}

static char compareIntegers(const int a, const int b) {
	if (a == b) {
		return 0;
	} else if (a < b) {
		return -1;
	} else /* if (a > b) */ {
		return 1;
	}
}

char Version::compareWith(const Version &other) const
{
	static const QVector<Section> defaultSection = QVector<Section>();

	for (int i = 0; i < std::max(m_sections.size(), other.m_sections.size()); ++i) {
		const QVector<Section> a = m_sections.size() > i ? m_sections.at(i) : defaultSection;
		const QVector<Section> b = other.m_sections.size() > i ? other.m_sections.at(i) : defaultSection;

		const char value = compareSections(a, b);
		if (value != 0) {
			return value;
		}
	}
	return 0;
}

/* Some notes on this algorithm:
 *
 * #1 Empty sections use the default value of 0 (1.0 and 1.0.0.0 are equal)
 * #2 String sections are worth "less" than integer sections (1.0-a is less than 1.0, since 1.0 is equivalent with 1.0-0)
 */
char Version::compareSections(const QVector<Version::Section> &a, const QVector<Version::Section> &b)
{
	static const Section defaultSubsection = qMakePair(QString(), 0); // #1

	for (int i = 0; i < std::max(a.size(), b.size()); ++i) {
		const Section secA = a.size() > i ? a.at(i) : defaultSubsection; // #1
		const Section secB = b.size() > i ? b.at(i) : defaultSubsection; // #1

		if (secA.first.isNull() && secB.first.isNull()) { // both are integers
			const char value = compareIntegers(secA.second, secB.second);
			if (value != 0) {
				return value;
			}
		} else if (secA.first.isNull()) { // only secB is integer
			return -1; // #2
		} else if (secB.first.isNull()) { // only secA is integer
			return 1; // #2
		} else { // both are strings
			const char value = static_cast<const char>(secA.first.compare(secB.first));
			if (value != 0) {
				return value;
			}
		}
	}

	return 0;
}

}
}
