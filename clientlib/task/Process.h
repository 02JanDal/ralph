#pragma once

#include "Task.h"

class QProcess;

namespace Ralph {
namespace ClientLib {

class Process
{
public:
	explicit Process(const QString &executable = QString());

	QString executable() const { return m_executable; }
	void setExecutable(const QString &exe) { m_executable = exe; }

	QVector<QString> arguments() const { return m_arguments; }
	void setArguments(const QVector<QString> &arguments) { m_arguments = arguments; }

	QString workingDirectory() const { return m_workingDir; }
	void setWorkingDirectory(const QString &dir) { m_workingDir = dir; }

	Future<void> run() const;
	Future<QByteArray> runCaptureOutput() const;

private:
	QString m_executable;
	QVector<QString> m_arguments;
	QString m_workingDir;

	std::unique_ptr<QProcess> prime() const;
};

}
}
