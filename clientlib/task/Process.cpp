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

#include "Process.h"

#include <QProcess>
#include <QRegularExpression>

namespace Ralph {
namespace ClientLib {

Process::Process(const QString &executable)
	: m_executable(executable)
{
}

Future<void> Process::run() const
{
	return async([this](Notifier notifier)
	{
		std::unique_ptr<QProcess> proc = prime();
		QProcess *procPtr = proc.get();
		procPtr->setReadChannelMode(QProcess::MergedChannels);
		QObject::connect(procPtr, &QProcess::readyRead, [notifier, procPtr]()
		{
			const QStringList lines = QString::fromLocal8Bit(procPtr->readAll()).split('\n');
			for (const QString &line : lines) {
				if (line.isEmpty()) {
					continue;
				}
				notifier.status(line);
			}
		});
		procPtr->start(QProcess::ReadOnly);
		procPtr->waitForStarted();
		procPtr->waitForFinished(-1);

		if (procPtr->error() != QProcess::UnknownError) {
			if (procPtr->exitStatus() == QProcess::CrashExit) {
				throw Exception("%1 failed with exit code %2" % m_executable % procPtr->exitCode());
			} else {
				throw Exception("%1 failed: %2" % m_executable % procPtr->errorString());
			}
		}
	});
}
Future<QByteArray> Process::runCaptureOutput() const
{
	return async([this](Notifier notifier)
	{
		std::unique_ptr<QProcess> proc = prime();
		QProcess *procPtr = proc.get();
		QObject::connect(procPtr, &QProcess::readyReadStandardError, [notifier, procPtr]()
		{
			const QStringList lines = QString::fromLocal8Bit(procPtr->readAllStandardError()).split('\n');
			for (const QString &line : lines) {
				notifier.status(line);
			}
		});
		procPtr->start(QProcess::ReadOnly);
		procPtr->waitForStarted();
		procPtr->waitForFinished(-1);

		if (procPtr->error() != QProcess::UnknownError) {
			if (procPtr->exitStatus() == QProcess::CrashExit) {
				throw Exception("%1 failed with exit code %2" % m_executable % procPtr->exitCode());
			} else {
				throw Exception("%1 failed: %2" % m_executable % procPtr->errorString());
			}
		}

		return procPtr->readAllStandardOutput();
	});
}

std::unique_ptr<QProcess> Process::prime() const
{
	std::unique_ptr<QProcess> proc = std::make_unique<QProcess>();
	proc->setProgram(m_executable);
	proc->setArguments(m_arguments.toList());
	if (!m_workingDir.isNull()) {
		proc->setWorkingDirectory(m_workingDir);
	}
	return proc;
}



}
}
