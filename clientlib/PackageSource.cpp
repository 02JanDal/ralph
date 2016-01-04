#include "PackageSource.h"

#include "Json.h"
#include "Project.h"
#include "Functional.h"

namespace Ralph {
using namespace Common;

namespace ClientLib {

PackageSource::PackageSource(const SourceType type, QObject *parent)
	: QObject(parent), m_type(type) {}

void PackageSource::setName(const QString &name)
{
	if (name != m_name) {
		m_name = name;
		emit nameChanged(m_name);
	}
}

PackageSource *PackageSource::fromJson(const QJsonValue &value, QObject *parent)
{
	using namespace Json;

	if (value.isObject()) {
		const QJsonObject obj = ensureObject(value);
		const QString type = ensureString(obj, "type");
		if (type == "git") {
			GitSinglePackageSource *source = new GitSinglePackageSource(parent);
			source->setUrl(ensureUrl(obj, "url"));
			source->setIdentifier(ensureString(obj, "identifier", QString()));
			source->setPath(ensureString(obj, "path", QString()));
			return source;
		} else if (type == "github") {
			GitHubSinglePackageSource *source = new GitHubSinglePackageSource(parent);
			source->setRepo(ensureString(obj, "repo"));
			source->setIdentifier(ensureString(obj, "identifier", QString()));
			source->setPath(ensureString(obj, "path", QString()));
			return source;
		} else if (type == "gitrepo") {
			GitRepoPackageSource *source = new GitRepoPackageSource(parent);
			source->setUrl(ensureUrl(obj, "url"));
			source->setIdentifier(ensureString(obj, "identifier", QString()));
			return source;
		} else {
			throw Exception("Invalid source type: '" + type + "'. Known types: 'git', 'github', 'gitrepo'.");
		}
	} else {
		const QString string = ensureString(value);
		const QStringList parts = string.split(':', QString::KeepEmptyParts);
		if (parts.isEmpty()) {
			throw Exception("Invalid source specifier");
		}
		const QString type = parts.first();
		if (type == "git") {
			if (parts.size() < 2) {
				throw Exception("Invalid source specifier for type 'git'. Expected format: git:<url>[:<identifier>]");
			}
			GitSinglePackageSource *source = new GitSinglePackageSource(parent);
			source->setUrl(QUrl(parts.at(1)));
			if (parts.size() > 2) {
				source->setIdentifier(parts.at(2));
			}
			return source;
		} else if (type == "github") {
			if (parts.size() < 2) {
				throw Exception("Invalid source specifier for type 'github'. Expected format: github:<repo>[:<identifier>]");
			}
			GitHubSinglePackageSource *source = new GitHubSinglePackageSource(parent);
			source->setRepo(parts.at(1));
			if (parts.size() > 2) {
				source->setIdentifier(parts.at(2));
			}
			return source;
		} else {
			throw Exception("Invalid source specifier: Unknown type '" + type + "'. Known types: 'git', 'github'.");
		}
	}
}

BaseGitPackageSource::BaseGitPackageSource(const SourceType type, QObject *parent)
	: PackageSource(type, parent) {}
void BaseGitPackageSource::setUrl(const QUrl &url)
{
	if (url != m_url) {
		m_url = url;
		emit urlChanged(m_url);
	}
}
void BaseGitPackageSource::setIdentifier(const QString &identifier)
{
	if (identifier != m_identifier) {
		m_identifier = identifier;
		emit identifierChanged(m_identifier);
	}
}

QString BaseGitPackageSource::toString() const
{
	if (identifier().isNull()) {
		return typeString() + ':' + url().toString();
	} else {
		return QString("%1:%2:%3").arg(typeString(), url().toString(), identifier());
	}
}
QJsonObject BaseGitPackageSource::toJson() const
{
	QJsonObject obj;
	obj.insert("url", Json::toJson(url()));
	if (!identifier().isEmpty()) {
		obj.insert("identifier", identifier());
	}
	return obj;
}

GitSinglePackageSource::GitSinglePackageSource(const PackageSource::SourceType type, QObject *parent)
	: BaseGitPackageSource(type, parent) {}

GitSinglePackageSource::GitSinglePackageSource(QObject *parent)
	: BaseGitPackageSource(GitSingle, parent) {}

void GitSinglePackageSource::setPath(const QString &path)
{
	if (path != m_path) {
		m_path = path;
		emit pathChanged(m_path);
	}
}

QVector<const Package *> GitSinglePackageSource::packages() const
{
	const Package *proj = Project::fromJson(Json::ensureDocument(basePath().absoluteFilePath(path())));
	return {proj};
}

QJsonObject GitSinglePackageSource::toJson() const
{
	QJsonObject obj = BaseGitPackageSource::toJson();
	if (!path().isEmpty()) {
		obj.insert("path", path());
	}
	return obj;
}

GitHubSinglePackageSource::GitHubSinglePackageSource(QObject *parent)
	: GitSinglePackageSource(GitHubSingle, parent) {}
void GitHubSinglePackageSource::setRepo(const QString &repo)
{
	if (repo != m_repo) {
		m_repo = repo;
		emit repoChanged(m_repo);

		setUrl(QUrl("https://github.com/" + m_repo + ".git"));
	}
}

QString GitHubSinglePackageSource::toString() const
{
	if (identifier().isNull()) {
		return typeString() + ':' + repo();
	} else {
		return typeString() + ':' + repo() + ':' + identifier();
	}
}
QJsonObject GitHubSinglePackageSource::toJson() const
{
	QJsonObject obj;
	obj.insert("repo", repo());
	if (!identifier().isEmpty()) {
		obj.insert("identifier", identifier());
	}
	if (!path().isEmpty()) {
		obj.insert("path", path());
	}
	return obj;
}

GitRepoPackageSource::GitRepoPackageSource(QObject *parent)
	: BaseGitPackageSource(GitRepo, parent) {}

QVector<const Package *> GitRepoPackageSource::packages() const
{
	const auto files = basePath().entryInfoList(QStringList() << "*.json", QDir::Files | QDir::NoSymLinks | QDir::Readable);
	return Functional::map2<QVector<const Package *>>(files, [](const QFileInfo &file)
	{
		return Package::fromJson(Json::ensureDocument(file.absoluteFilePath()));
	});
}

}
}
