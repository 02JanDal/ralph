#pragma once

#include <QObject>
#include <QUrl>

#include "task/Task.h"

QT_BEGIN_NAMESPACE
class QDir;
QT_END_NAMESPACE

namespace Ralph {
namespace ClientLib {
class Package;
using RequirementPtr = std::shared_ptr<class Requirement>;

class PackageInstallationCandidate : public QObject
{
	Q_OBJECT
	Q_PROPERTY(RequirementPtr requirement READ requirement WRITE setRequirement NOTIFY requirementChanged)

public:
	explicit PackageInstallationCandidate(QObject *parent = nullptr);
	virtual ~PackageInstallationCandidate() {}

	virtual Future<void> install(const Package *package, const QDir &cacheDir, const QDir &directory) const = 0;
	virtual QJsonObject toJson() const;

	static PackageInstallationCandidate *fromJson(const QJsonObject &obj, QObject *parent = nullptr);

	RequirementPtr requirement() const { return m_requirement; }
	void setRequirement(const RequirementPtr &requirement);

signals:
	void requirementChanged(RequirementPtr requirement);

private:
	RequirementPtr m_requirement;
};

class BinaryPackageInstallationCandidate : public PackageInstallationCandidate
{
	Q_OBJECT
	Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)

public:
	explicit BinaryPackageInstallationCandidate(QObject *parent = nullptr);

	QUrl url() const { return m_url; }
	void setUrl(QUrl url);

	Future<void> install(const Package *package, const QDir &cacheDir, const QDir &directory) const override;
	QJsonObject toJson() const override;

signals:
	void urlChanged(QUrl url);

private:
	QUrl m_url;
};

class SourcePackageInstallationCandidate : public PackageInstallationCandidate
{
	Q_OBJECT

public:
	explicit SourcePackageInstallationCandidate(QObject *parent = nullptr);

	Future<void> install(const Package *package, const QDir &cacheDir, const QDir &directory) const override;
	QJsonObject toJson() const override;
};

}
}
