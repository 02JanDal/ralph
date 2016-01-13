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

#include <QObject>
#include <QFuture>
#include <QDir>

#include "Package.h"
#include "Version.h"
#include "task/Task.h"

namespace Ralph {
namespace ClientLib {
class PackageGroup;

class PackageDatabase : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool readonly READ isReadonly CONSTANT)

	explicit PackageDatabase(const QDir &dir, const QVector<PackageDatabase *> &inherits, QObject *parent = nullptr);

public:
	static Future<PackageDatabase *> get(const QDir &dir, const QVector<PackageDatabase *> inherits = {});

	bool isReadonly() const;

	void load();
	Future<void> build();

	const Package *getPackage(const QString &name, const Version &version) const;
	QVector<const Package *> findPackages(const QString &name, const VersionRequirement *version = nullptr) const;

	QVector<QString> packageNames() const;

	PackageSource *source(const QString &name) const;
	QVector<PackageSource *> sources() const { return m_sources; }
	Future<void> registerPackageSource(PackageSource *source);
	Future<void> unregisterPackageSource(const QString &name);

	QVector<PackageDatabase *> inheritedDatabases() const { return m_inherits; }

	std::shared_ptr<PackageGroup> group(const QString &name = QString());
	QVector<std::shared_ptr<PackageGroup>> groups() const { return m_groups; }

private: // internal
	void save();

private: // static/on creation
	const QDir m_dir;
	const QVector<PackageDatabase *> m_inherits;

private: // settings, semi-static
	QVector<PackageSource *> m_sources;
	QVector<std::shared_ptr<PackageGroup>> m_groups;

private: // packages, semi-static
	mutable QMutex m_mutex;
	QVector<const Package *> m_packages;
	QMultiHash<QString, const Package *> m_packageMapping;
};

}
}
