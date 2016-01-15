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

#include <QUrl>
#include <QDir>
#include <QDateTime>

#include "task/Task.h"

QT_BEGIN_NAMESPACE
class QJsonValue;
class QJsonObject;
QT_END_NAMESPACE

namespace Ralph {
namespace ClientLib {
class Package;

class PackageSource
{
public:
	enum SourceType
	{
		GitHubSingle,
		GitSingle,
		GitRepo
	};

	explicit PackageSource(const SourceType type);
	virtual ~PackageSource();

	SourceType type() const { return m_type; }
	virtual QString typeString() const = 0;

	// properties
	QString name() const { return m_name; }
	void setName(const QString &name);

	QDateTime lastUpdated() const { return m_lastUpdated; }
	void setLastUpdated();

	// (de-)serialization
	static PackageSource *fromJson(const QJsonValue &value);
	static PackageSource *fromString(const QString &value);
	virtual QString toString() const { return QString(); }
	virtual QJsonObject toJson() const;

	// package access
	virtual Future<QVector<const Package *>> packages() const = 0;
	virtual Future<void> update() = 0;

	// internal
	QDir basePath() const { return m_basePath; }
	/// @internal For usage by PackageDatabase only
	void setBasePath(const QDir &dir) { m_basePath = dir; }

private:
	const SourceType m_type;
	QString m_name;
	QDir m_basePath;
	QDateTime m_lastUpdated;
};

class BaseGitPackageSource : public PackageSource
{
protected:
	explicit BaseGitPackageSource(const SourceType type);

public:
	QUrl url() const { return m_url; }
	void setUrl(const QUrl &url) { m_url = url; }

	QString identifier() const { return m_identifier; }
	void setIdentifier(const QString &identifier) { m_identifier = identifier; }

	QString toString() const override;
	QJsonObject toJson() const override;

signals:
	void urlChanged(const QUrl &url);
	void identifierChanged(const QString &identifier);

private:
	QUrl m_url;
	QString m_identifier = "master";
};

class GitSinglePackageSource : public BaseGitPackageSource
{
protected:
	explicit GitSinglePackageSource(const SourceType type);

public:
	explicit GitSinglePackageSource();

	QString typeString() const override { return "git"; }

	QString path() const { return m_path; }
	void setPath(const QString &path) { m_path = path; }

	Future<QVector<const Package *>> packages() const override;
	Future<void> update() override;

	QJsonObject toJson() const override;

private:
	QString m_path;
};

class GitHubSinglePackageSource : public GitSinglePackageSource
{
public:
	explicit GitHubSinglePackageSource();

	QString repo() const { return m_repo; }
	void setRepo(const QString &repo);

	QString typeString() const override { return "github"; }

	QString toString() const override;
	QJsonObject toJson() const override;

private:
	QString m_repo;

	// prevent setting the raw url
	using BaseGitPackageSource::setUrl;
};

class GitRepoPackageSource : public BaseGitPackageSource
{
public:
	explicit GitRepoPackageSource();

	QString typeString() const override { return "gitrepo"; }

	Future<QVector<const Package *>> packages() const override;
	Future<void> update() override;
};

}
}
