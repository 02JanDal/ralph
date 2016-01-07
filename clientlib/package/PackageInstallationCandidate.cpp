#include "PackageInstallationCandidate.h"

#include "Json.h"
#include "FileSystem.h"
#include "Requirement.h"
#include "Package.h"
#include "task/Network.h"
#include "task/Archive.h"

namespace Ralph {
namespace ClientLib {

PackageInstallationCandidate::PackageInstallationCandidate(QObject *parent)
 : QObject(parent)
{
}

QJsonObject PackageInstallationCandidate::toJson() const
{
	QJsonObject obj;
	obj.insert("requirements", m_requirement->toJson());
	return obj;
}
PackageInstallationCandidate *PackageInstallationCandidate::fromJson(const QJsonObject &obj, QObject *parent)
{
	using namespace Json;

	PackageInstallationCandidate *candidate;
	const QString typeString = ensureString(obj, "type");
	if (typeString == "binary") {
		BinaryPackageInstallationCandidate *binary = new BinaryPackageInstallationCandidate(parent);
		binary->setUrl(ensureUrl(obj, "url"));
		candidate = binary;
	} else if (typeString == "source") {
		SourcePackageInstallationCandidate *source = new SourcePackageInstallationCandidate(parent);
		candidate = source;
	} else {
		throw Exception("Unknown package installation candidate type: " + typeString);
	}
	candidate->setRequirement(std::make_unique<AndRequirement>(AndRequirement::fromJson(ensureArray(obj, "requirements", QJsonArray()))));
	return candidate;
}

void PackageInstallationCandidate::setRequirement(const RequirementPtr &requirement)
{
	if (m_requirement == requirement) {
		return;
	}

	m_requirement = requirement;
	emit requirementChanged(requirement);
}

BinaryPackageInstallationCandidate::BinaryPackageInstallationCandidate(QObject *parent)
	: PackageInstallationCandidate(parent)
{
}

void BinaryPackageInstallationCandidate::setUrl(QUrl url)
{
	if (m_url == url) {
		return;
	}

	m_url = url;
	emit urlChanged(url);
}

Task<void>::Ptr BinaryPackageInstallationCandidate::install(const Package *package, const QDir &cacheDir, const QDir &directory) const
{
	return createTask([this, package, cacheDir, directory](Notifier notifier)
	{
		const QString dlDestination = cacheDir.absoluteFilePath("downloads/%1/%2/%3").arg(package->name(), package->version().toString(), QFileInfo(m_url.path()).fileName());
		FS::ensureExists(QFileInfo(dlDestination).dir());
		notifier.await(Network::download(m_url, dlDestination));
		notifier.await(Archive::extract(dlDestination, directory));
	});
}

QJsonObject BinaryPackageInstallationCandidate::toJson() const
{
	QJsonObject obj = PackageInstallationCandidate::toJson();
	obj.insert("url", Json::toJson(m_url));
	return obj;
}

SourcePackageInstallationCandidate::SourcePackageInstallationCandidate(QObject *parent)
	: PackageInstallationCandidate(parent)
{
}

Task<void>::Ptr SourcePackageInstallationCandidate::install(const Package *package, const QDir &cacheDir, const QDir &directory) const
{
	Q_UNUSED(package)
	Q_UNUSED(cacheDir)
	Q_UNUSED(directory)
	return createTask([package, cacheDir, directory](Notifier notifier)
	{
		Q_UNUSED(notifier)
	});
}
QJsonObject SourcePackageInstallationCandidate::toJson() const
{
	return PackageInstallationCandidate::toJson();
}

}
}
