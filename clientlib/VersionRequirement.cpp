#include "VersionRequirement.h"

namespace Ralph {
namespace ClientLib {

VersionRequirement::VersionRequirement(QObject *parent)
	: QObject(parent)
{
}

void VersionRequirement::setVersion(const Version &version)
{
	if (version != m_version) {
		m_version = version;
		emit versionChanged(m_version);
	}
}
void VersionRequirement::setType(const VersionRequirement::Type type)
{
	if (type != m_type) {
		m_type = type;
		emit typeChanged(m_type);
	}
}

bool VersionRequirement::accepts(const Version &version) const
{
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
bool Version::isValid() const
{
	return false;
}

Version Version::fromString(const QString &string)
{
	Q_UNUSED(string)
	return Version();
}

char Version::compareWith(const Version &other) const
{
	Q_UNUSED(other)
	return 0;
}

}
}
