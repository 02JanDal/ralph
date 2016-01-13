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
