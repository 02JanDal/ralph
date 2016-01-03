#pragma once

#include <QObject>

class Version
{
public:
	explicit Version();

	bool isValid() const;

	inline bool operator<(const Version &other) const { return compareWith(other) == -1; }
	inline bool operator<=(const Version &other) const { return compareWith(other) != 1; }
	inline bool operator>(const Version &other) const { return compareWith(other) == 1; }
	inline bool operator>=(const Version &other) const { return compareWith(other) != -1; }
	inline bool operator==(const Version &other) const { return compareWith(other) == 0; }
	inline bool operator!=(const Version &other) const { return compareWith(other) != 0; }

	static Version fromString(const QString &string);

private:
	char compareWith(const Version &other) const;
};

class VersionRequirement : public QObject
{
	Q_OBJECT
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

signals:
	void versionChanged(const Version &version);
	void typeChanged(const Type type);

private:
	Version m_version;
	Type m_type;
};
