#pragma once

#include <QObject>
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

class VersionRequirement : public QObject
{
	Q_OBJECT
	Q_PROPERTY(Version::Type allowedType READ allowedType WRITE setAllowedType NOTIFY allowedTypeChanged)
	Q_PROPERTY(Version version READ version WRITE setVersion NOTIFY versionChanged)
	Q_PROPERTY(Type type READ type WRITE setType NOTIFY typeChanged)

public:
	explicit VersionRequirement(QObject *parent = nullptr);

	enum Type
	{
		Less, LessEqual,
		Greater, GreaterEqual,
		Equal, NonEqual
	};
	Q_ENUM(Type)

	Version version() const { return m_version; }
	void setVersion(const Version &version);

	Type type() const { return m_type; }
	void setType(const Type type);

	bool accepts(const Version &version) const;

	Version::Type allowedType() const { return m_allowedType; }
	void setAllowedType(const Version::Type allowedType);

	bool isValid() const { return m_version.isValid(); }

	QString toString() const;
	static VersionRequirement *fromString(const QString &string, QObject *parent = nullptr);

signals:
	void versionChanged(const Version &version);
	void typeChanged(const Type type);
	void allowedTypeChanged(Version::Type allowedType);

private:
	Version m_version;
	Type m_type;
	QString m_allowedTypeString;
	Version::Type m_allowedType;
};

}
}
