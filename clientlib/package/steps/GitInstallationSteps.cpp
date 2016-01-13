/* Copyright 2016 Jan Dalheimer <jan@dalheimer.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "GitInstallationSteps.h"

#include "Json.h"
#include "git/GitRepo.h"

namespace Ralph {
namespace ClientLib {

GitCloneStep::GitCloneStep() {}
GitCloneStep::GitCloneStep(const QUrl &url) : m_url(url) {}

QJsonValue GitCloneStep::toJson() const
{
	return QJsonObject({
						   qMakePair(QStringLiteral("type"), type()),
						   qMakePair(QStringLiteral("url"), Json::toJson(m_url))
					   });
}

void GitCloneStep::fromJsonObject(const QJsonObject &object)
{
	m_url = Json::ensureUrl(object, "url");
	m_url.setFragment(Json::ensureString(object, "identifier", m_url.fragment()));
}

Future<void> GitCloneStep::perform(const ActionContext &ctxt)
{
	return async([this, ctxt](Notifier notifier)
	{
		QUrl url = m_url;
		url.setFragment(QString());
		Git::GitRepo *repo = notifier.await(Git::GitRepo::clone(ctxt.get<InstallContextItem>().buildDir, url));
		if (!url.fragment().isEmpty()) {
			notifier.await(repo->checkout(url.fragment()));
		}
	});
}

GitSubmoduleSetupStep::GitSubmoduleSetupStep() {}

Future<void> GitSubmoduleSetupStep::perform(const ActionContext &ctxt)
{
	return async([ctxt](Notifier notifier)
	{
		Git::GitRepo *repo = notifier.await(Git::GitRepo::open(ctxt.get<InstallContextItem>().buildDir));
		notifier.await(repo->submodulesUpdate());
	});
}

}
}

