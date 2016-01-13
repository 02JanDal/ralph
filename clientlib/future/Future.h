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

#include <QObject>
#include <QPointer>
#include <unordered_set>
#include <exception>
#include <memory>
#include <future>

#include "Exception.h"
#include "Functional.h"
#include "FutureData_p.h"
#include "Promise.h"

namespace Ralph {
namespace ClientLib {

template <typename> class Future;
template <typename> class FutureWatcher;
template <typename> class Task;

namespace Private {
class BaseFutureWatcher;
}

namespace Private {
class BaseFuture
{
protected:
	explicit BaseFuture(const std::shared_ptr<Private::BaseFutureData> &other) : d(other) {}
public:
	virtual ~BaseFuture();
	void addWatcher(BaseFutureWatcher *watcher);
	void removeWatcher(BaseFutureWatcher *watcher);

	void start();

	void waitForFinished();

protected:
	friend class BasePromise;
	template <typename T> std::shared_ptr<Private::FutureData<T>> d_func() const { return std::static_pointer_cast<Private::FutureData<T>>(d); }

	std::shared_ptr<Private::BaseFutureData> d;
};
}

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wweak-vtables")
template <typename T>
class Future : public Private::BaseFuture
{
	friend class Future<void>;
public:
	Future(const Future<T> &other) : Private::BaseFuture(other.d) {}
	explicit Future(const std::shared_ptr<Private::FutureData<T>> &data) : Private::BaseFuture(data) {}

	T result()
	{
		start(); // we might not have been started yet
		waitForFinished();
		std::unique_lock<std::mutex> lock(d->mutex);
		if (d->state == Private::FutureData<T>::Exception && d->exception) {
			d->exception->rethrow();
		}
		return d_func<T>()->result;
	}
};
template <>
class Future<void> : public Private::BaseFuture
{
public:
	Future(const Future<void> &other) : Private::BaseFuture(other.d) {}
	template <typename T> Future(const Future<T> &other) : Private::BaseFuture(other.d) {}
	explicit Future(const std::shared_ptr<Private::FutureData<void>> &data) : Private::BaseFuture(data) {}

	void result()
	{
		start(); // we might not have been started yet
		waitForFinished();
		std::unique_lock<std::mutex> lock(d->mutex);
		if (d->state == Private::FutureData<void>::Exception && d->exception) {
			d->exception->rethrow();
		}
	}
};
QT_WARNING_POP

template <typename T>
inline Future<T> Promise<T>::future() const
{
	return Future<T>(d_func<T>());
}
inline Future<void> Promise<void>::future() const
{
	return Future<void>(d_func<void>());
}
template <typename OtherT>
OtherT Private::BasePromise::await(const Future<OtherT> &other)
{
	Future<OtherT> future{other};
	future.d->delegateTo = std::make_shared<BasePromise>(*this);
	future.waitForFinished();
	future.d->delegateTo.reset();
	return future.result();
}

template <typename T>
T await(Future<T> future)
{
	return future.result();
}

}
}
