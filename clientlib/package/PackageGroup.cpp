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

#include "PackageGroup.h"

#include <QTemporaryDir>

#include "ActionContext.h"
#include "Json.h"
#include "Package.h"
#include "PackageMirror.h"
#include "Functional.h"
#include "FileSystem.h"

namespace Ralph {
using namespace Common;

namespace ClientLib {

PackageGroup::PackageGroup(const QString &name, const QDir &dir)
	: m_name(name), m_dir(dir)
{
	FS::ensureExists(dir);
}

Future<void> PackageGroup::install(const Package *pkg)
{
	return async([this, pkg](Notifier notifier)
	{
		readSettings();
		if (isInstalled(pkg)) {
			notifier.status("%1 is already installed!" % pkg->name());
			return;
		}

		notifier.status("Installing %1..." % pkg->name());

		QTemporaryDir buildDir;

		ActionContext ctxt;
		ctxt.emplace<InstallContextItem>(targetDir(pkg), buildDir.path());
		notifier.await(pkg->mirrors().first()->install(ctxt));
		m_installed.append(InstalledPackage{pkg, 0});
		writeSettings();
	});
}
Future<void> PackageGroup::remove(const Package *pkg)
{
	return async([this, pkg](Notifier notifier)
	{
		readSettings();
		if (!isInstalled(pkg)) {
			notifier.status("%1 is not installed!" % pkg->name());
			return;
		}

		notifier.status("Removing %1..." % pkg->name());

		FS::remove(baseDir(pkg));

		m_installed.erase(findInstalled(pkg));
		writeSettings();
	});
}

bool PackageGroup::isInstalled(const Package *pkg)
{
	return findInstalled(pkg) != m_installed.end();
}

QDir PackageGroup::targetDir(const Package *pkg) const
{
	return baseDir(pkg).absoluteFilePath("install");
}
QDir PackageGroup::baseDir(const Package *pkg) const
{
	return m_dir.absoluteFilePath("%1-%2" % pkg->name() % pkg->version().toString());
}

void PackageGroup::readSettings()
{
	if (!FS::exists(m_dir.absoluteFilePath("meta.json"))) {
		return;
	}

	const QJsonObject root = Json::ensureObject(Json::ensureDocument(m_dir.absoluteFilePath("meta.json")));

	m_installed = Functional::map(Json::ensureIsArrayOf<QJsonObject>(root, "packages"), [](const QJsonObject &obj)
	{
		return InstalledPackage{
			Package::fromJson(QJsonDocument(Json::ensureObject(obj, "pkg"))),
					Json::ensureInteger(obj, "mirrorIndex")
		};
	});
}
void PackageGroup::writeSettings() const
{
	QJsonObject root;
	root.insert("packages", Json::toJsonArray(Functional::map(m_installed, [](const InstalledPackage &pkg)
	{
		QJsonObject obj;
		obj.insert("pkg", pkg.pkg->toJson());
		obj.insert("mirrorIndex", pkg.mirrorIndex);
		return obj;
	})));
	Json::write(root, m_dir.absoluteFilePath("meta.json"));
}

QVector<PackageGroup::InstalledPackage>::Iterator PackageGroup::findInstalled(const Package *pkg)
{
	return std::find_if(m_installed.begin(), m_installed.end(), [pkg](const InstalledPackage &pack)
	{
		return pack.pkg->name() == pkg->name() && pack.pkg->version() == pkg->version();
	});
}

}
}
