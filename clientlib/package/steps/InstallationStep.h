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

#pragma once

#include <QString>
#include <QDir>
#include <memory>

#include "task/Task.h"
#include "ActionContext.h"

QT_BEGIN_NAMESPACE
class QJsonObject;
QT_END_NAMESPACE

namespace Ralph {
namespace ClientLib {

class InstallationStep
{
public:
	explicit InstallationStep();
	virtual ~InstallationStep();

	virtual QString type() const = 0;
	virtual void fromJsonObject(const QJsonObject &object);
	virtual QJsonValue toJson() const;

	virtual Future<void> perform(const ActionContext &ctxt) = 0;

	static std::unique_ptr<InstallationStep> create(const QString &type, const QJsonObject &obj);
};

}
}
