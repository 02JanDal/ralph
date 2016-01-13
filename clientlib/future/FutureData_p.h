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
#include <QPointer>
#include <QSemaphore>

#include <future>
#include <mutex>
#include <memory>
#include <unordered_set>

#include "Functional.h"
#include "WrappedException.h"

namespace Ralph {
namespace ClientLib {
namespace Private {
class BaseFutureWatcher;
}

namespace Private {
class BasePromise;
}

namespace Private {
class BaseFutureData
{
public:
	virtual ~BaseFutureData();
	enum { Scheduled, Running, Finished, Exception, Canceled } state = Scheduled;

	std::future<void> future;
	std::mutex mutex;

	void start();
	std::mutex startupMutex;

	std::size_t progressCurrent = 0;
	std::size_t progressTotal = 0;
	std::shared_ptr<WrappedException> exception;
	QString status;

	std::shared_ptr<Private::BasePromise> delegateTo;
	std::unordered_set<std::shared_ptr<void>> tasks;

	std::unordered_set<BaseFutureWatcher *> watchers;
};

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wweak-vtables")
template <typename T>
class FutureData : public BaseFutureData
{
public:
	T result;
};
template <>
class FutureData<void> : public BaseFutureData
{
};
QT_WARNING_POP

}
}
}
