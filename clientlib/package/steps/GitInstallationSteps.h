#pragma once

#include <QUrl>

#include "InstallationStep.h"

namespace Ralph {
namespace ClientLib {

class GitCloneStep : public InstallationStep
{
public:
	explicit GitCloneStep();
	explicit GitCloneStep(const QUrl &url);

	QString type() const override { return "git-clone"; }
	QJsonValue toJson() const override;
	void fromJsonObject(const QJsonObject &object) override;

	Future<void> perform(const ActionContext &ctxt) override;

private:
	QUrl m_url;
};

class GitSubmoduleSetupStep : public InstallationStep
{
public:
	explicit GitSubmoduleSetupStep();

	QString type() const override { return "git-submodule-setup"; }

	Future<void> perform(const ActionContext &ctxt) override;
};

}
}
