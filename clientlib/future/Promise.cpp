#include "Promise.h"

#include "FutureWatcher.h"

namespace Ralph {
namespace ClientLib {
namespace Private {

template <typename Func, typename... Args>
void BasePromise::report(Func &&func, Args&&... args)
{
	for (BaseFutureWatcher *watcher : d->watchers) {
		emit (watcher->*func)(args...);
	}
}

BasePromise::BasePromise(const std::shared_ptr<BaseFutureData> &other) : d(other)
{
	d->startupMutex.lock();
}

BasePromise::~BasePromise() {}

void BasePromise::prime(std::future<void> &&future)
{
	std::unique_lock<std::mutex> lock(d->mutex);
	Q_ASSERT(!d->future.valid());
	d->future = std::forward<std::future<void>>(future);
}
void BasePromise::reportStarted()
{
	// setting the state is done from BaseFutureData::start
	d->startupMutex.lock();
	report(&BaseFutureWatcher::started);
}
void BasePromise::reportFinished()
{
	{
		std::unique_lock<std::mutex> lock(d->mutex);
		d->state = Private::BaseFutureData::Finished;
	}
	report(&BaseFutureWatcher::finished);
}
void BasePromise::reportCanceled()
{
	{
		std::unique_lock<std::mutex> lock(d->mutex);
		d->state = Private::BaseFutureData::Canceled;
	}
	report(&BaseFutureWatcher::canceled);
}
void BasePromise::reportProgress(const std::size_t current, const std::size_t total)
{
	{
		std::unique_lock<std::mutex> lock(d->mutex);
		d->progressCurrent = current;
		d->progressTotal = total;
	}
	report(&BaseFutureWatcher::progress, current, total);

	if (d->delegateTo) {
		d->delegateTo->reportProgress(current, total);
	}
}
void BasePromise::reportStatus(const QString &message)
{
	{
		std::unique_lock<std::mutex> lock(d->mutex);
		d->status = message;
	}
	report(&BaseFutureWatcher::status, message);

	if (d->delegateTo) {
		d->delegateTo->reportStatus(message);
	}
}
void BasePromise::reportException(const std::exception_ptr &exception)
{
	{
		std::unique_lock<std::mutex> lock(d->mutex);
		d->exception = std::make_shared<WrappedException>(exception);
		d->state = Private::BaseFutureData::Exception;
	}
	report(&BaseFutureWatcher::exception);

	if (d->delegateTo) {
		d->delegateTo->reportException(exception);
	}
}

}
}
}
