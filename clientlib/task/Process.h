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
