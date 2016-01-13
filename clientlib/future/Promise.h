#pragma once

#include <future>

#include "FutureData_p.h"

namespace Ralph {
namespace ClientLib {

template <typename> class Future;

namespace Private {
class BasePromise
{
public:
	explicit BasePromise(const std::shared_ptr<Private::BaseFutureData> &other);
	BasePromise(const BasePromise &) = default;
	virtual ~BasePromise();

	template <typename T>
	void addTask(const std::shared_ptr<T> &task) { d->tasks.insert(task); }

	void prime(std::future<void> &&future);
	void reportStarted();
	void reportFinished();
	void reportCanceled();
	void reportProgress(const std::size_t current, const std::size_t total);
	void reportStatus(const QString &message);
	void reportException(const std::exception_ptr &exception);
	// in Future.h
	template <typename OtherT>
	OtherT await(const Future<OtherT> &other);

protected:
	template <typename T> std::shared_ptr<Private::FutureData<T>> d_func() const { return std::static_pointer_cast<Private::FutureData<T>>(d); }

	std::shared_ptr<Private::BaseFutureData> d;

	template <typename Func, typename... Args>
	void report(Func &&func, Args&&... args);
};
}

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wweak-vtables")
template <typename T>
class Promise : public Private::BasePromise
{
public:
	explicit Promise() : Private::BasePromise(std::make_shared<Private::FutureData<T>>()) {}
	Promise(const Promise<T> &other) : Private::BasePromise(other.d) {}

	void reportResult(T &&result)
	{
		std::unique_lock<std::mutex> lock(Private::BasePromise::d->mutex);
		d_func<T>()->result = std::forward<T>(result);
	}

	// in Future.h
	inline Future<T> future() const;
};
template <>
class Promise<void> : public Private::BasePromise
{
public:
	explicit Promise() : Private::BasePromise(std::make_shared<Private::FutureData<void>>()) {}
	Promise(const Promise<void> &other) : Private::BasePromise(other.d) {}

	// in Future.h
	inline Future<void> future() const;
};
QT_WARNING_POP

}
}
