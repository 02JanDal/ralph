#include "Package.h"

#include "Json.h"
#include "Functional.h"
#include "PackageSource.h"

namespace Ralph {
using namespace Common;

namespace ClientLib {

PackageDependency::PackageDependency(QObject *parent)
	: QObject(parent) {}
PackageDependency::PackageDependency(const QString &package, QObject *parent)
	: QObject(parent), m_package(package) {}

void PackageDependency::setPackage(const QString &package)
{
	if (package != m_package) {
		m_package = package;
		emit packageChanged(m_package);
	}
}
void PackageDependency::setVersion(VersionRequirement *version)
{
	if (version != m_version) {
		m_version = version;
		emit versionChanged(m_version);
	}
}
void PackageDependency::setConfigurations(const QVector<QString> &configurations)
{
	if (configurations != m_configurations) {
		m_configurations = configurations;
		emit configurationsChanged(m_configurations);
	}
}
void PackageDependency::setOperatingSystems(const QVector<QString> &os)
{
	if (os != m_os) {
		m_os = os;
		emit operatingSystemsChanged(m_os);
	}
}
void PackageDependency::setOptional(const bool optional)
{
	if (optional != m_optional) {
		m_optional = optional;
		emit optionalChanged(m_optional);
	}
}
void PackageDependency::setSource(PackageSource *source)
{
	if (source != m_source) {
		m_source = source;
		emit sourceChanged(m_source);
	}
}

Package::Package(QObject *parent)
	: QObject(parent)
{
}

void Package::setName(const QString &name)
{
	if (name != m_name) {
		m_name = name;
		emit nameChanged(m_name);
	}
}
void Package::setVersion(const Version &version)
{
	if (version != m_version) {
		m_version = version;
		emit versionChanged(m_version);
	}
}
void Package::setDependencies(const QVector<PackageDependency *> &dependencies)
{
	if (dependencies != m_dependencies) {
		m_dependencies = dependencies;
		emit dependenciesChanged(m_dependencies);
	}
}

const Package *Package::fromJson(const QJsonDocument &doc)
{
	using namespace Json;
	Package *package = new Package();
	try {
		const QJsonObject root = ensureObject(doc);

		package->setName(ensureString(root, "name"));
		package->setVersion(Version::fromString(ensureString(root, "version")));
		package->setDependencies(Functional::map(ensureIsArrayOf<QJsonObject>(root, "dependencies"), [package](const QJsonObject &obj)
		{
			PackageDependency *dep = new PackageDependency(package);
			dep->setPackage(ensureString(obj, "name"));

			if (obj.contains("version")) {
				const QString v = ensureString(obj, "version");
				VersionRequirement *version = new VersionRequirement(dep);
				if (v.startsWith(">=")) {
					version->setVersion(Version::fromString(v.mid(2)));
					version->setType(VersionRequirement::GreaterEqual);
				} else if (v.startsWith(">")) {
					version->setVersion(Version::fromString(v.mid(1)));
					version->setType(VersionRequirement::Greater);
				} else if (v.startsWith("<=")) {
					version->setVersion(Version::fromString(v.mid(2)));
					version->setType(VersionRequirement::LessEqual);
				} else if (v.startsWith("<")) {
					version->setVersion(Version::fromString(v.mid(1)));
					version->setType(VersionRequirement::Less);
				} else if (v.startsWith("==")) {
					version->setVersion(Version::fromString(v.mid(2)));
					version->setType(VersionRequirement::Equal);
				} else if (v.startsWith("!=")) {
					version->setVersion(Version::fromString(v.mid(2)));
					version->setType(VersionRequirement::NonEqual);
				}
				dep->setVersion(version);
			}

			dep->setOptional(ensureBoolean(obj, QStringLiteral("optional"), false));
			dep->setOperatingSystems(ensureIsArrayOf<QString>(obj, "os", QVector<QString>()));
			dep->setConfigurations(ensureIsArrayOf<QString>(obj, "config", QVector<QString>()));

			if (obj.contains("source")) {
				dep->setSource(PackageSource::fromJson(obj.value("source"), dep));
			}

			return dep;
		}));

		return package;
	} catch (...) {
		delete package;
		throw;
	}
}

}
}
