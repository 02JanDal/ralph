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
class ActionContext;

class Requirement
{
public:
	using Ptr = std::shared_ptr<Requirement>;

	virtual ~Requirement();

	virtual QJsonObject toJson() const = 0;

	static QVector<Requirement::Ptr> fromJson(const QJsonArray &array);
	static QJsonArray toJson(const QVector<Requirement::Ptr> &requirements);

	virtual bool isSatisfied(const ActionContext &ctxt) const = 0;
};

class AndRequirement : public Requirement
{
public:
	explicit AndRequirement(QVector<Requirement::Ptr> &&children) : m_children(std::forward<decltype(children)>(children)) {}

	QJsonObject toJson() const override;

	bool isSatisfied(const ActionContext &ctxt) const override;

private:
	QVector<Requirement::Ptr> m_children;
};
class OrRequirement : public Requirement
{
public:
	explicit OrRequirement(QVector<Requirement::Ptr> &&children) : m_children(std::forward<decltype(children)>(children)) {}

	QJsonObject toJson() const override;

	bool isSatisfied(const ActionContext &ctxt) const override;

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

	QJsonObject toJson() const override;

	bool isSatisfied(const ActionContext &) const override { return m_os == m_currentOs; }

private:
	const Os m_os;

	static const Os m_currentOs;
	static Os fromString(const QString &str);
};

class BuildTypeRequirement : public Requirement
{
public:
	explicit BuildTypeRequirement(const QString &configuration);

	QJsonObject toJson() const override;
	bool isSatisfied(const ActionContext &ctxt) const override;

private:
	QString m_configuration;
};

}
}
