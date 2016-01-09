#pragma once

#include <QString>
#include <QVector>
#include <memory>

QT_BEGIN_NAMESPACE
class QJsonArray;
class QJsonObject;
class QJsonValue;
QT_END_NAMESPACE

namespace Ralph {
namespace ClientLib {

class Requirement
{
public:
	using Ptr = std::shared_ptr<Requirement>;

	virtual ~Requirement();

	virtual QJsonValue toJson() const = 0;

	static QVector<Requirement::Ptr> fromJson(const QJsonArray &array);
	static QJsonArray toJson(const QVector<Requirement::Ptr> &requirements);

	virtual bool isSatisfied() const = 0;
};

class AndRequirement : public Requirement
{
public:
	explicit AndRequirement(QVector<Requirement::Ptr> &&children) : m_children(std::forward<decltype(children)>(children)) {}

	QJsonValue toJson() const override;

	bool isSatisfied() const override;

private:
	QVector<Requirement::Ptr> m_children;
};
class OrRequirement : public Requirement
{
public:
	explicit OrRequirement(QVector<Requirement::Ptr> &&children) : m_children(std::forward<decltype(children)>(children)) {}

	QJsonValue toJson() const override;

	bool isSatisfied() const override;

private:
	QVector<Requirement::Ptr> m_children;
};

class OsRequirement : public Requirement
{
public:
	enum Os
	{
		Windows,
		Linux,
		OSX,

		Invalid
	};

	explicit OsRequirement(const Os os) : m_os(os) {}
	explicit OsRequirement(const QString &string);

	QJsonValue toJson() const override;

	bool isSatisfied() const override { return m_os == m_currentOs; }

private:
	const Os m_os;

	static const Os m_currentOs;
	static Os fromString(const QString &str);
};

class ConfigurationRequirement : public Requirement
{
public:
	explicit ConfigurationRequirement(const QString &configuration);

	QJsonValue toJson() const override;
	bool isSatisfied() const override { return false; }

private:
	QString m_configuration;
};

}
}
