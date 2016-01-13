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

#include "WrappedException.h"

namespace Ralph {
namespace ClientLib {

QString WrappedException::message(const std::exception_ptr &exception)
{
	if (exception) {
		try {
			std::rethrow_exception(exception);
		} catch (Exception &e) {
			return e.cause();
		} catch (std::exception &e) {
			return QString(e.what());
		} catch (...) {
			return QString();
		}
	} else {
		return QString();
	}
}

WrappedException::WrappedException(const std::exception_ptr &exception)
	: Exception(message(exception)), m_exception(exception) {}

WrappedException::WrappedException() : Exception("") {}

WrappedException::~WrappedException() {}

void WrappedException::rethrow()
{
	if (m_exception) {
		std::rethrow_exception(m_exception);\
	}
}

}
}
