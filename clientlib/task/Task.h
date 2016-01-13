#pragma once

#include <QFutureInterface>
#include <QFuture>
#include <QThreadPool>

#include "Functional.h"
#include "Exception.h"
#include "future/Future.h"

namespace Ralph {
namespace ClientLib {
template <typename> class Task;
namespace Private {
template <typename T>
inline void runAndReportResult(Task<T> *task);
template <>
inline void runAndReportResult<void>(Task<void> *task);
}

template <typename T>
class Task : public std::enable_shared_from_this<Task<T>>
{
	std::launch m_policy;
public:
	explicit Task(std::launch policy) : m_policy(policy) {}
	virtual ~Task() {}

	Future<T> start()
	{
		m_promise.prime(std::async(m_policy, [this]()
		{
			try {
				m_promise.reportStarted();
				Private::runAndReportResult(this);
				m_promise.reportFinished();
			} catch (...) {
				m_promise.reportException(std::current_exception());
			}
		}));
		return future();
	}

	Future<T> future() const { return m_promise.future(); }

protected:
	friend void Private::runAndReportResult<T>(Task<T> *);
	virtual T run() = 0;

	void reportStatus(const QString &status) { m_promise.reportStatus(status); }
	void reportProgress(const std::size_t current, const std::size_t total) { m_promise.reportProgress(current, total); }
	void keepAlive(const std::shared_ptr<Task<T>> &task) { m_promise.addTask(task); }

private:
	friend class Notifier;
	Promise<T> m_promise;
};

class Notifier
{
	mutable Private::BasePromise m_promise;
public:
	template <typename T>
	explicit Notifier(Task<T> *task)
		: m_promise(task->m_promise) {}

	void status(const QString &status) const { m_promise.reportStatus(status); }
	void progress(const std::size_t current, const std::size_t total) const { m_promise.reportProgress(current, total); }

	template <typename T>
	inline T await(const Future<T> &future) const
	{
		return m_promise.await(future);
	}
};
template <>
inline void Notifier::await<void>(const Future<void> &future) const
{
	m_promise.await(future);
}

namespace Private {
template <typename T>
inline void runAndReportResult(Task<T> *task)
{
	task->m_promise.reportResult(task->run());
}
template <>
inline void runAndReportResult<void>(Task<void> *task)
{
	task->run();
}

using Ralph::Common::Functional::FunctionTraits;

template <typename Func>
std::enable_if_t<FunctionTraits<Func>::arity == 0, void>
callWithoutReturn(Func &&func, Notifier &&)
{
	func();
}
template <typename Func>
std::enable_if_t<FunctionTraits<Func>::arity == 1, void>
callWithoutReturn(Func &&func, Notifier &&notifier)
{
	func(std::forward<Notifier>(notifier));
}
template <typename Func, typename T>
std::enable_if_t<FunctionTraits<Func>::arity == 0, T>
callWithReturn(Func &&func, Notifier &&)
{
	return func();
}
template <typename Func, typename T>
std::enable_if_t<FunctionTraits<Func>::arity == 1, T>
callWithReturn(Func &&func, Notifier &&notifier)
{
	return func(std::forward<Notifier>(notifier));
}

template <typename T, typename Func>
class LambdaTask : public Task<T>
{
	Func m_func;
public:
	explicit LambdaTask(std::launch policy, Func &&func) : Task<T>(policy), m_func(std::forward<Func>(func)) {}
	static std::shared_ptr<LambdaTask<T, Func>> make(std::launch policy, Func &&func)
	{
		std::shared_ptr<LambdaTask<T, Func>> ptr = std::make_shared<LambdaTask<T, Func>>(policy, std::forward<Func>(func));
		ptr->keepAlive(ptr);
		return ptr;
	}

	T run()
	{
		return callWithReturn<Func, T>(std::move(m_func), Notifier(this));
	}
};
template <typename Func>
class LambdaTask<void, Func> : public Task<void>
{
	Func m_func;
public:
	explicit LambdaTask(std::launch policy, Func &&func) : Task<void>(policy), m_func(std::forward<Func>(func)) {}
	static std::shared_ptr<LambdaTask<void, Func>> make(std::launch policy, Func &&func)
	{
		std::shared_ptr<LambdaTask<void, Func>> ptr = std::make_shared<LambdaTask<void, Func>>(policy, std::forward<Func>(func));
		ptr->keepAlive(ptr);
		return ptr;
	}

	void run()
	{
		callWithoutReturn<Func>(std::move(m_func), Notifier(this));
	}
};
}

template <typename Func, typename Type = typename Common::Functional::FunctionTraits<Func>::ReturnType>
Future<Type> async(Func &&func)
{
	return Private::LambdaTask<Type, Func>::make(std::launch::deferred, std::forward<Func>(func))->start();
}
template <typename Func, typename Type = typename Common::Functional::FunctionTraits<Func>::ReturnType>
Future<Type> async(std::launch policy, Func &&func)
{
	return Private::LambdaTask<Type, Func>::make(policy, std::forward<Func>(func))->start();
}

}
}
