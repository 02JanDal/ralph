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
