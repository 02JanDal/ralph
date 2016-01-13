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
#include "Exception.h"

namespace Ralph {
namespace ClientLib {
namespace Network {

class NetworkException : public Exception
{
public:
	using Exception::Exception;
	NetworkException(const NetworkException &e) = default;
	virtual ~NetworkException();

	static void throwIfError(int code, const char *errorbuffer = nullptr);
};

void init();
Future<void> download(const QUrl &url, const QString &destination);
Future<QByteArray> get(const QUrl &url);

}
}
}
