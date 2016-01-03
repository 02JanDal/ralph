#include "GitRepo.h"

#include <QUrl>

#include <git2.h>

void initGit()
{
	static bool haveInitialized = false;
	if (!haveInitialized) {
		git_libgit2_init();
	}
}

std::function<GitCredentialResponse(const GitCredentialQuery &)> GitRepo::m_credentialsFunc;

GitRepo::GitRepo(const QDir &dir, QObject *parent)
	: QObject(parent), m_dir(dir)
{
	initGit();
}

Task<GitRepo *>::Ptr GitRepo::init(const QDir &dir, QObject *parent)
{
	return createTask([dir, parent](Notifier)
	{
		initGit();
		if (!dir.mkpath(dir.absolutePath())) {
			throw Exception("Unable to create directory to init");
		}

		std::unique_ptr<GitRepo> repo = std::make_unique<GitRepo>(dir, parent);

		git_repository_init_options opts = GIT_REPOSITORY_INIT_OPTIONS_INIT;
		opts.flags |= GIT_REPOSITORY_INIT_MKPATH;
		repo->m_repo = GitResource<git_repository>::create(&git_repository_init_ext, &git_repository_free,
														   dir.absolutePath().toLocal8Bit(), &opts);

		return repo.release();
	});
}

Task<GitRepo *>::Ptr GitRepo::open(const QDir &dir, QObject *parent)
{
	return createTask([dir, parent](Notifier)
	{
		initGit();

		std::unique_ptr<GitRepo> repo = std::make_unique<GitRepo>(dir);

		repo->m_repo = GitResource<git_repository>::create(&git_repository_open_ext, &git_repository_free,
														   dir.absolutePath().toLocal8Bit(), GIT_REPOSITORY_OPEN_NO_SEARCH, nullptr);

		return repo.release();
	});
}

struct GitNotifierPayload
{
	Notifier notifier;
	enum { Initial, Fetching, CheckingOut } state = Initial;
};

void gitCheckoutNotifier(const char *, const size_t current, const size_t total, void *payload)
{
	GitNotifierPayload *pl = static_cast<GitNotifierPayload *>(payload);
	if (pl->state != GitNotifierPayload::CheckingOut) {
		pl->notifier.status("Checking out...");
		pl->state = GitNotifierPayload::CheckingOut;
	}
	pl->notifier.progressTotal(int(total));
	pl->notifier.progressCurrent(int(current));
}
int gitFetchNotifier(const git_transfer_progress *stats, void *payload)
{
	GitNotifierPayload *pl = static_cast<GitNotifierPayload *>(payload);
	if (pl->state != GitNotifierPayload::Fetching) {
		pl->notifier.status("Fetching...");
		pl->state = GitNotifierPayload::Fetching;
	}
	pl->notifier.progressTotal(int(stats->total_objects));
	pl->notifier.progressCurrent(int(stats->received_objects));

	return 0;
}

Task<GitRepo *>::Ptr GitRepo::clone(const QDir &dir, const QUrl &url, QObject *parent)
{
	return createTask([dir, url, parent](Notifier notifier)
	{
		initGit();

		std::unique_ptr<GitRepo> repo = std::make_unique<GitRepo>(dir);

		GitNotifierPayload payload{notifier};

		git_clone_options opts = GIT_CLONE_OPTIONS_INIT;
		opts.checkout_opts.checkout_strategy = GIT_CHECKOUT_FORCE | GIT_CHECKOUT_USE_THEIRS;
		opts.checkout_opts.progress_cb = &gitCheckoutNotifier;
		opts.checkout_opts.progress_payload = &payload;
		opts.fetch_opts.callbacks.transfer_progress = &gitFetchNotifier;
		opts.fetch_opts.callbacks.payload = &payload;
		opts.fetch_opts.callbacks.credentials = &credentialsCallback;

		repo->m_repo = GitResource<git_repository>::create(&git_clone, &git_repository_free,
														   url.toString().toLocal8Bit(), dir.absolutePath().toLocal8Bit(), &opts);

		return repo.release();
	});
}

Task<void>::Ptr GitRepo::fetch() const
{
	return createTask([this](Notifier notifier)
	{
		auto remote = GitResource<git_remote>::create(&git_remote_lookup, &git_remote_free, m_repo, "origin");

		GitNotifierPayload payload{notifier};

		git_fetch_options opts = GIT_FETCH_OPTIONS_INIT;
		opts.callbacks.transfer_progress = &gitFetchNotifier;
		opts.callbacks.payload = &payload;
		opts.callbacks.credentials = &credentialsCallback;

		GitException::checkAndThrow(git_remote_fetch(remote, nullptr, &opts, nullptr));
	});
}

Task<void>::Ptr GitRepo::checkout(const QString &id) const
{
	return createTask([this, id](Notifier notifier)
	{
		auto treeish = GitResource<git_object>::create(&git_revparse_single, &git_object_free, m_repo, id.toLocal8Bit().constData());

		git_checkout_options opts = GIT_CHECKOUT_OPTIONS_INIT;
		opts.checkout_strategy = GIT_CHECKOUT_FORCE | GIT_CHECKOUT_USE_THEIRS;
		opts.progress_cb = &gitCheckoutNotifier;
		GitNotifierPayload payload{notifier};
		opts.progress_payload = &payload;

		GitException::checkAndThrow(git_checkout_tree(m_repo, treeish, &opts));
	});
}

Task<void>::Ptr GitRepo::pull(const QString &id) const
{
	return createTask([this, id](Notifier notifier)
	{
		notifier.await(fetch());
		notifier.await(checkout(id));
	});
}

GitCredentialResponse::GitCredentialResponse(git_cred *cred)
	: m_cred(cred) {}
GitCredentialResponse GitCredentialResponse::createForUsername(const QString &username)
{
	git_cred *cred;
	if (git_cred_username_new(&cred, username.toLocal8Bit().constData()) < 0) {
		return createError();
	} else {
		return GitCredentialResponse(cred);
	}
}
GitCredentialResponse GitCredentialResponse::createForUsernamePassword(const QString &username, const QString &password)
{
	git_cred *cred;
	if (git_cred_userpass_plaintext_new(&cred,
										username.toLocal8Bit().constData(),
										password.toLocal8Bit().constData()) < 0) {
		return createError();
	} else {
		return GitCredentialResponse(cred);
	}
}
GitCredentialResponse GitCredentialResponse::createForSSHKey(const QString &username, const QString &pubkeyPath, const QString &privkeyPath, const QString &passthrase)
{
	git_cred *cred;
	if (git_cred_ssh_key_new(&cred,
							 username.toLocal8Bit().constData(),
							 pubkeyPath.toLocal8Bit().constData(),
							 privkeyPath.toLocal8Bit().constData(),
							 passthrase.toLocal8Bit().constData()) < 0) {
		return createError();
	} else {
		return GitCredentialResponse(cred);
	}
}
GitCredentialResponse GitCredentialResponse::createForDefault()
{
	git_cred *cred;
	if (git_cred_default_new(&cred) < 0) {
		return createError();
	} else {
		return GitCredentialResponse(cred);
	}
}
GitCredentialResponse GitCredentialResponse::createInvalid()
{
	return GitCredentialResponse(nullptr);
}
GitCredentialResponse GitCredentialResponse::createError()
{
	return GitCredentialResponse(static_cast<git_cred *>(1));
}

int GitRepo::credentialsCallback(git_cred **out, const char *url, const char *usernameFromUrl, const unsigned int allowedTypes, void *payload)
{
	GitCredentialQuery::Types types;
	if (allowedTypes & GIT_CREDTYPE_DEFAULT) {
		types |= GitCredentialQuery::Default;
	}
	if (allowedTypes & GIT_CREDTYPE_USERNAME) {
		types |= GitCredentialQuery::Username;
	}
	if (allowedTypes & GIT_CREDTYPE_USERPASS_PLAINTEXT) {
		types |= GitCredentialQuery::UsernamePassword;
	}
	if (allowedTypes & GIT_CREDTYPE_SSH_CUSTOM) {
		types |= GitCredentialQuery::SSHCustom;
	}
	if (allowedTypes & GIT_CREDTYPE_SSH_INTERACTIVE) {
		types |= GitCredentialQuery::SSHInteractive;
	}
	if (allowedTypes & GIT_CREDTYPE_SSH_KEY) {
		types |= GitCredentialQuery::SSHKey;
	}
	const GitCredentialQuery query = GitCredentialQuery(types, QString::fromLocal8Bit(url), QString::fromLocal8Bit(usernameFromUrl));
	const GitCredentialResponse response = m_credentialsFunc(query);
	if (!response.result()) {
		return 1;
	} else if (response.result() == (git_cred *)1) {
		return GIT_EUSER;
	} else {
		*out = response.result();
		return 0;
	}
}

GitCredentialQuery::GitCredentialQuery(const GitCredentialQuery::Types types, const QUrl &url, const QString &usernameDefault)
	: m_types(types), m_url(url), m_usernameFromUrl(usernameDefault) {}
