#pragma once

#include <QObject>
#include <QDir>
#include <QUrl>
#include <functional>

#include "task/Task.h"
#include "Functional.h"
#include "GitException.h"
#include "Optional.h"

struct git_repository;
struct git_cred;

namespace Ralph {
namespace ClientLib {
namespace Git {

template <typename Class>
class GitResource
{
	using FreeFunc = void(*)(Class *);
	FreeFunc m_freeFunc;

	std::shared_ptr<Class> m_resource;

public:
	explicit GitResource(FreeFunc &&free, Class *resource)
		: m_resource(std::shared_ptr<Class>(resource, free))
	{
	}
	explicit GitResource() {}

	inline Class *get() const { return m_resource.get(); }
	inline Class *operator->() const { return m_resource.get(); }
	inline operator Class *() const { return m_resource.get(); }
	inline operator bool() const { return m_resource; }

	template <typename InitFunc, typename... Args>
	static GitResource<Class> create(InitFunc &&init, FreeFunc &&free, Args &&... args)
	{
		Class *resource;
		GitException::checkAndThrow(init(&resource, args...));
		return GitResource<Class>(std::forward<FreeFunc>(free), resource);
	}
};

class GitCredentialResponse
{
	struct git_cred *m_cred;
	bool m_isError = false;

	explicit GitCredentialResponse(struct git_cred *cred);
	explicit GitCredentialResponse() : m_isError(true) {}

public:
	struct git_cred *result() const { return m_cred; }
	bool isError() const { return m_isError; }

	static GitCredentialResponse createForUsername(const QString &username);
	static GitCredentialResponse createForUsernamePassword(const QString &username, const QString &password);
	static GitCredentialResponse createForSSHKey(const QString &username, const QString &pubkeyPath, const QString &privkeyPath, const QString &passthrase);
	static GitCredentialResponse createForDefault();
	static GitCredentialResponse createInvalid();
	static GitCredentialResponse createError();
};

class GitCredentialQuery
{
	Q_GADGET
public:
	enum Type
	{
		Username,
		UsernamePassword,
		SSHKey,
		SSHCustom,
		SSHInteractive,
		Default
	};
	Q_DECLARE_FLAGS(Types, Type)
	Q_FLAG(Types)

	explicit GitCredentialQuery(const Types types, const QUrl &url, const QString &usernameDefault);

	Types allowedTypes() const { return m_types; }

	QUrl url() const { return m_url; }
	QString usernameFromUrl() const { return m_usernameFromUrl; }

private:
	Types m_types;
	QUrl m_url;
	QString m_usernameFromUrl;
};

class GitRepo : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QDir dir READ dir CONSTANT)

public:
	explicit GitRepo(const QDir &dir, QObject *parent = nullptr);

	static Future<GitRepo *> init(const QDir &dir, QObject *parent = nullptr);
	static Future<GitRepo *> open(const QDir &dir, QObject *parent = nullptr);
	static Future<GitRepo *> clone(const QDir &dir, const QUrl &url, QObject *parent = nullptr);

	QDir dir() const { return m_dir; }

	Future<void> fetch() const;
	Future<void> checkout(const QString &id) const;
	Future<void> pull(const QString &id) const;
	Future<void> submodulesUpdate(const bool init = true) const;

	template <typename Func>
	static void setCredentialsCallback(Func &&func)
	{
		static_assert(std::is_same<GitCredentialResponse, typename Ralph::Common::Functional::FunctionTraits<Func>::ReturnType>::value,
					  "error: return type has to be a GitCredentialResponse");
		m_credentialsFunc = std::forward<Func>(func);
	}

	static int credentialsCallback(git_cred **out, const char *url,
								   const char *usernameFromUrl, const unsigned int allowedTypes,
								   void *payload);

private:
	QDir m_dir;
	GitResource<git_repository> m_repo;

	static std::function<GitCredentialResponse(const GitCredentialQuery &)> m_credentialsFunc;
};

}
}
}
