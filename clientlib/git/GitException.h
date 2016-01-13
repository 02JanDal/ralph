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

#include "Exception.h"

namespace Ralph {
namespace ClientLib {
namespace Git {

class GitException : public Exception
{
	int m_error = -1;
public:
	explicit GitException(const char *error, const int id);
	GitException(const GitException &) = default;
	virtual ~GitException();

	int error() const { return m_error; }

	static void checkAndThrow(const int error);
};

}
}
}
