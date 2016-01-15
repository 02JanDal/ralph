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

#include <QFuture>
#include <QDir>

#include "task/Task.h"
#include "PackageGroup.h"
#include "Version.h"

namespace Ralph {
namespace ClientLib {
class PackageSource;
class Package;

class PackageDatabase
{
	explicit PackageDatabase(const QDir &dir, const QVector<PackageDatabase *> &inherits);

public:
	static Future<PackageDatabase *> get(const QDir &dir, const QVector<PackageDatabase *> inherits = {});
	static Future<PackageDatabase *> create(const QString &dir);
	static QString databasePath(const QString &type);

	bool isReadonly() const;

	void load();
	Future<void> build();

	const Package *getPackage(const QString &name, const Version &version) const;
	QVector<const Package *> findPackages(const QString &name, const VersionRequirement &version = VersionRequirement()) const;

	QVector<QString> packageNames() const;

	PackageSource *source(const QString &name) const;
	QVector<PackageSource *> sources() const { return m_sources; }
	Future<void> registerPackageSource(PackageSource *source);
	Future<void> unregisterPackageSource(const QString &name);

	QVector<PackageDatabase *> inheritedDatabases() const { return m_inherits; }

	PackageGroup group(const QString &name = QString());
	QVector<PackageGroup> groups() const { return m_groups; }

private: // internal
	void save();

private: // static/on creation
	const QDir m_dir;
	const QVector<PackageDatabase *> m_inherits;

private: // settings, semi-static
	QVector<PackageSource *> m_sources;
	QVector<PackageGroup> m_groups;

private: // packages, semi-static
	mutable QMutex m_mutex;
	QVector<const Package *> m_packages;
	QMultiHash<QString, const Package *> m_packageMapping;
};

}
}
