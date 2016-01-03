#pragma once

#include <QObject>
#include <QDir>
#include <QUrl>
#include <functional>

#include "Functional.h"
#include "GitException.h"
#include "Task.h"
#include "Optional.h"

template <typename Class>
class GitResource
{
	using FreeFunc = void(*)(Class *);
	FreeFunc m_freeFunc;

	Class *m_resource = nullptr;

public:
	explicit GitResource(FreeFunc &&free, Class *resource)
		: m_freeFunc(free), m_resource(resource)
	{
	}
	explicit GitResource() {}
	explicit GitResource(const GitResource &) = delete;
	explicit GitResource(GitResource &&) = default;
	GitResource &operator=(const GitResource &) = delete;
	GitResource &operator=(GitResource &&) = default;
	~GitResource()
	{
		m_freeFunc(m_resource);
	}

	inline Class *get() const { return m_resource; }
	inline Class *operator->() const { return m_resource; }
	inline operator Class *() const { return m_resource; }
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

	explicit GitCredentialResponse(struct git_cred *cred);

public:
	struct git_cred *result() const { return m_cred; }

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
	virtual ~GitCredentialQuery() {}

	Types allowedTypes() const { return m_types; }

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

	static Task<GitRepo *>::Ptr init(const QDir &dir, QObject *parent = nullptr);
	static Task<GitRepo *>::Ptr open(const QDir &dir, QObject *parent = nullptr);
	static Task<GitRepo *>::Ptr clone(const QDir &dir, const QUrl &url, QObject *parent = nullptr);

	QDir dir() const { return m_dir; }

	Task<void>::Ptr fetch() const;
	Task<void>::Ptr checkout(const QString &id) const;
	Task<void>::Ptr pull(const QString &id) const;

	template <typename Func>
	static void setCredentialsCallback(Func &&func)
	{
		static_assert(std::is_same<GitCredentialResponse, typename Ralph::Common::Functional::FunctionTraits<Func>::ReturnType>::value,
					  "error: return type has to be a GitCredentialResponse");
		m_credentialsFunc = std::forward<Func>(func);
	}

private:
	QDir m_dir;
	GitResource<struct git_repository> m_repo;

	static std::function<GitCredentialResponse(const GitCredentialQuery &)> m_credentialsFunc;
	static int credentialsCallback(struct git_cred **out, const char *url,
								   const char *usernameFromUrl, const unsigned int allowedTypes,
								   void *payload);
};
