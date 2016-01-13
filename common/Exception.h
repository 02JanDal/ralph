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
#include <exception>

#include "Formatting.h"

class Exception : public std::exception
{
public:
	Exception(const QString &message);
	Exception(const Exception &other);
	virtual ~Exception() noexcept;
	const char *what() const noexcept;
	QString cause() const { return m_message; }

private:
	QString m_message;
};

#define DECLARE_EXCEPTION(name)                 \
	QT_WARNING_PUSH                             \
	QT_WARNING_DISABLE_GCC("-Wweak-vtables")    \
	class name##Exception : public ::Exception  \
	{                                           \
	public:                                     \
		using ::Exception::Exception;           \
	}                                           \
	QT_WARNING_POP                              \
