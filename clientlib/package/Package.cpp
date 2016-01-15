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

#include "Package.h"

#include "Json.h"
#include "Functional.h"
#include "PackageMirror.h"
#include "PackageDependency.h"

namespace Ralph {
using namespace Common;

namespace ClientLib {

Package::Package() {}

Package::~Package() {}

QJsonObject Package::toJson() const
{
	QJsonObject obj;
	obj.insert("name", name());
	obj.insert("version", version().toString());
	if (!mirrors().isEmpty()) {
		obj.insert("mirrors", Json::toJsonArray(mirrors()));
	}
	if (!dependencies().isEmpty()) {
		obj.insert("dependencies", Json::toJsonArray(dependencies()));
	}
	if (!paths().isEmpty()) {
		obj.insert("paths", Json::toJsonObject(paths()));
	}
	return obj;
}

const Package *Package::fromJson(const QJsonDocument &doc, Package *package)
{
	using namespace Json;

	if (!package) {
		package = new Package();
	}

	try {
		const QJsonObject root = ensureObject(doc);

		package->setName(ensureString(root, "name"));
		package->setVersion(Version::fromString(ensureString(root, "version")));
		package->setMirrors(Functional::map(ensureIsArrayOf<QJsonObject>(root, "mirrors", QVector<QJsonObject>()), &PackageMirror::fromJson));
		package->setPaths(ensureIsHashOf<QString>(root, "paths", QHash<QString, QString>()));
		package->setDependencies(Functional::map(ensureIsArrayOf<QJsonObject>(root, "dependencies", QVector<QJsonObject>()), &PackageDependency::fromJson));

		return package;
	} catch (...) {
		delete package;
		throw;
	}
}

}
}
