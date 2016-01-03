#pragma once

#include <QObject>
#include <QUrl>

class QJsonValue;
class QJsonObject;

class PackageSource : public QObject
{
	Q_OBJECT
	Q_PROPERTY(SourceType type READ type CONSTANT)
	Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)

public:
	enum SourceType
	{
		GitHubSingle,
		GitSingle,
		GitRepo
	};
	Q_ENUM(SourceType)

	explicit PackageSource(const SourceType type, QObject *parent = nullptr);

	SourceType type() const { return m_type; }
	virtual QString typeString() const = 0;

	QString name() const { return m_name; }
	void setName(const QString &name);

	static PackageSource *fromJson(const QJsonValue &value, QObject *parent = nullptr);
	virtual QString toString() const { return QString(); }
	virtual QJsonObject toJson() const = 0;

signals:
	void nameChanged(const QString &name);

private:
	const SourceType m_type;
	QString m_name;
};

class BaseGitPackageSource : public PackageSource
{
	Q_OBJECT
	Q_PROPERTY(QUrl url READ url WRITE setUrl NOTIFY urlChanged)
	Q_PROPERTY(QString identifier READ identifier WRITE setIdentifier NOTIFY identifierChanged)

protected:
	explicit BaseGitPackageSource(const SourceType type, QObject *parent = nullptr);

public:
	QUrl url() const { return m_url; }
	void setUrl(const QUrl &url);

	QString identifier() const { return m_identifier; }
	void setIdentifier(const QString &identifier);

	QString toString() const override;
	QJsonObject toJson() const override;

signals:
	void urlChanged(const QUrl &url);
	void identifierChanged(const QString &identifier);

private:
	QUrl m_url;
	QString m_identifier;
};

class GitSinglePackageSource : public BaseGitPackageSource
{
	Q_OBJECT
public:
	explicit GitSinglePackageSource(QObject *parent = nullptr);

	QString typeString() const override { return "git"; }
};

class GitHubSinglePackageSource : public BaseGitPackageSource
{
	Q_OBJECT
	Q_PROPERTY(QString repo READ repo WRITE setRepo NOTIFY repoChanged)

public:
	explicit GitHubSinglePackageSource(QObject *parent = nullptr);

	QString repo() const { return m_repo; }
	void setRepo(const QString &repo);

	QString typeString() const override { return "github"; }

	QString toString() const override;
	QJsonObject toJson() const override;

signals:
	void repoChanged(const QString &repo);

private:
	QString m_repo;

	// prevent setting the raw url
	using BaseGitPackageSource::setUrl;
};

class GitRepoPackageSource : public BaseGitPackageSource
{
	Q_OBJECT

public:
	explicit GitRepoPackageSource(QObject *parent = nullptr);

	QString typeString() const override { return "gitrepo"; }
};
