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

#include <QVector>
#include <QHash>
#include <memory>

#include "Version.h"
#include "PackageMirror.h"
#include "PackageDependency.h"

class QJsonDocument;
class QJsonObject;

namespace Ralph {
namespace ClientLib {

class Package
{
public:
	explicit Package();
	virtual ~Package();

public: // properties
	QString name() const { return m_name; }
	void setName(const QString &name) { m_name = name; }

	Version version() const { return m_version; }
	void setVersion(const Version &version) { m_version = version; }

	QVector<PackageDependency> dependencies() const { return m_dependencies; }
	void setDependencies(const QVector<PackageDependency> &dependencies) { m_dependencies = dependencies; }

	QVector<PackageMirror> mirrors() const { return m_mirrors; }
	void setMirrors(QVector<PackageMirror> mirrors) { m_mirrors = mirrors; }

	QHash<QString, QString> paths() const { return m_paths; }
	void setPaths(const QHash<QString, QString> &paths) { m_paths = paths; }

public: //serialization
	QJsonObject toJson() const;
	static const Package *fromJson(const QJsonDocument &doc, Package *package = nullptr);

private:
	QString m_name;
	Version m_version;
	QVector<PackageDependency> m_dependencies;
	QVector<PackageMirror> m_mirrors;
	QHash<QString, QString> m_paths;
};

}
}
