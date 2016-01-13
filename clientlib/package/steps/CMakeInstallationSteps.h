#pragma once

#include "InstallationStep.h"

namespace Ralph {
namespace ClientLib {

class CMakeConfigStep : public InstallationStep
{
public:
	explicit CMakeConfigStep();

	QString type() const override { return "cmake-config"; }
	QJsonValue toJson() const override;
	void fromJsonObject(const QJsonObject &obj) override;

	Future<void> perform(const ActionContext &ctxt) override;
};

class CMakeBuildStep : public InstallationStep
{
public:
	explicit CMakeBuildStep();

	QString type() const override { return "cmake-build"; }
	QJsonValue toJson() const override;
	void fromJsonObject(const QJsonObject &obj) override;

	Future<void> perform(const ActionContext &ctxt) override;

private:
	QVector<QString> m_targets;
};

}
}
