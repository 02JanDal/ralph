#include "Package.h"

#include "Json.h"
#include "Functional.h"
#include "PackageSource.h"
#include "PackageInstallationCandidate.h"
#include "Requirement.h"

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
void PackageDependency::setRequirements(const RequirementPtr &requirements)
{
	if (requirements != m_requirements) {
		m_requirements = requirements;
		emit requirementsChanged(m_requirements);
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
void Package::setInstallationCandidates(QVector<PackageInstallationCandidate *> installationCandidates)
{
	if (m_installationCandidates == installationCandidates) {
		return;
	}

	m_installationCandidates = installationCandidates;
	emit installationCandidatesChanged(installationCandidates);
}

QJsonObject Package::toJson() const
{
	QJsonObject obj;
	obj.insert("name", name());
	obj.insert("version", version().toString());
	if (!installationCandidates().isEmpty()) {
		obj.insert("installation", Json::toJsonArray(installationCandidates()));
	}
	if (!dependencies().isEmpty()) {
		obj.insert("dependencies", Json::toJsonArray(Functional::map(dependencies(), [](const PackageDependency *dep)
		{
			QJsonObject depObj;
			depObj.insert("name", dep->package());
			if (dep->version()->isValid()) {
				depObj.insert("version", dep->version()->toString());
			}
			depObj.insert("optional", dep->isOptional());
			depObj.insert("requirements", dep->requirements()->toJson());
			if (dep->source()) {
				depObj.insert("source", dep->source()->toJson());
			}
			return depObj;
		})));
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
		package->setInstallationCandidates(Functional::map(ensureIsArrayOf<QJsonObject>(root, "installation", QVector<QJsonObject>()), [package](const QJsonObject &obj)
		{
			return PackageInstallationCandidate::fromJson(obj, package);
		}));
		package->setDependencies(Functional::map(ensureIsArrayOf<QJsonObject>(root, "dependencies", QVector<QJsonObject>()), [package](const QJsonObject &obj)
		{
			PackageDependency *dep = new PackageDependency(package);
			dep->setPackage(ensureString(obj, "name"));

			if (obj.contains("version")) {
				dep->setVersion(VersionRequirement::fromString(ensureString(obj, "version"), dep));
			}

			dep->setOptional(ensureBoolean(obj, QStringLiteral("optional"), false));
			dep->setRequirements(std::make_shared<AndRequirement>(AndRequirement::fromJson(ensureArray(obj, "requirements", QJsonArray()))));

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
