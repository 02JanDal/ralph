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

#include "Exception.h"

Exception::Exception(const QString &message) : std::exception(), m_message(message) {}

Exception::Exception(const Exception &other)
	: std::exception(), m_message(other.cause())
{
}

Exception::~Exception() noexcept {}

const char *Exception::what() const noexcept
{
	return m_message.toLatin1().constData();
}
