#pragma once

#include <QFutureInterface>
#include <QFuture>
#include <QThreadPool>
#include <memory>

#include "Functional.h"

namespace Ralph {
namespace ClientLib {

class WrappedException : public QException
{
	const char *m_msg;
public:
	explicit WrappedException(const char *msg)
		: m_msg(strdup(msg)) {}

	const char *what() const noexcept override { return m_msg; }

	void raise() const override { throw *this; }
	QException *clone() const override { return new WrappedException(*this); }
};

class BaseStatusObject : public QObject
{
	Q_OBJECT
public:
	explicit BaseStatusObject(QObject *parent = nullptr) : QObject(parent) {}

signals:
	void status(const QString &str);
};

template <typename T>
class Task : public BaseStatusObject, public QFutureInterface<T>, public QRunnable
{
	class Notifier *m_notifier;
public:
	using Ptr = std::shared_ptr<Task<T>>;

	QFuture<T> start()
	{
		return start(QThreadPool::globalInstance());
	}
	QFuture<T> start(QThreadPool *pool)
	{
		this->setThreadPool(pool);
		this->setRunnable(this);
		this->reportStarted();
		QFuture<T> future = this->future();
		pool->start(this, 0);
		return future;
	}

	virtual void run() override
	{
		if (this->isCanceled()) {
			this->reportFinished();
			return;
		}
		try {
			Ralph::Common::Functional::static_if<std::is_same<void, T>::value>([this](auto f) {
				f(this)->runImpl();
			}).else_([this](auto f) {
				f(this)->reportResult(f(this)->runImpl(), -1);
			});
		} catch (QException &e) {
			this->reportException(e);
		} catch (std::exception &e) {
			this->reportException(WrappedException(e.what()));
		} catch (...) {
			this->reportException(QUnhandledException());
		}

		this->reportFinished();
	}

	Notifier *notifier() const { return m_notifier; }

protected:
	explicit Task();
	virtual T runImpl() = 0;
};
template <typename T>
using TaskPtr = typename Task<T>::Ptr;

class Notifier
{
	BaseStatusObject *m_status;
	QFutureInterfaceBase *m_future;
	Notifier *m_delegateTo = nullptr;
public:
	explicit Notifier(BaseStatusObject *status, QFutureInterfaceBase *future);

	void status(const QString &status) const;
	void progressCurrent(const int current) const;
	void progressTotal(const int total) const;

	template <typename T>
	inline T await(const std::shared_ptr<Task<T>> &task)
	{
		task->notifier()->m_delegateTo = this;
		QFuture<T> future = task->future();
		future.waitForFinished();
		task->notifier()->m_delegateTo = nullptr;
		return future.result();
	}
};
template <>
inline void Notifier::await(const TaskPtr<void> &task)
{
	task->notifier()->m_delegateTo = this;
	task->future().waitForFinished();
	task->notifier()->m_delegateTo = nullptr;
}

template <typename T>
Task<T>::Task()
{
	m_notifier = new Notifier(this, this);
}

namespace details
{
using namespace Ralph::Common::Functional;

template <typename Func, typename T>
std::enable_if_t<FunctionTraits<Func>::arity == 0 && std::is_same<typename FunctionTraits<Func>::ReturnType, void>::value>
call(Func &&func, Notifier &&)
{
	func();
}
template <typename Func, typename T>
std::enable_if_t<FunctionTraits<Func>::arity == 0 && !std::is_same<typename FunctionTraits<Func>::ReturnType, void>::value>
call(Func &&func, Notifier &&)
{
	return func();
}
template <typename Func, typename T>
std::enable_if_t<FunctionTraits<Func>::arity == 1 && std::is_same<typename FunctionTraits<Func>::ReturnType, void>::value,
	typename FunctionTraits<Func>::ReturnType>
call(Func &&func, Notifier &&notifier)
{
	func(std::forward<Notifier>(notifier));
}
template <typename Func, typename T>
std::enable_if_t<FunctionTraits<Func>::arity == 1 && !std::is_same<typename FunctionTraits<Func>::ReturnType, void>::value,
	typename FunctionTraits<Func>::ReturnType>
call(Func &&func, Notifier &&notifier)
{
	return func(std::forward<Notifier>(notifier));
}
}

template <typename Func, typename T>
class LambdaTask : public Task<T>
{
	Func m_func;

public:
	explicit LambdaTask(Func &&func)
		: Task<T>(), m_func(std::forward<Func>(func)) {}

	T runImpl() override
	{
		return details::call<Func, T>(std::forward<Func>(m_func), std::forward<Notifier>(*this->notifier()));
		//return m_func(*this->notifier());
	}
};
template <typename Func>
class LambdaTask<void, Func> : public Task<void>
{
	Func m_func;
public:
	explicit LambdaTask(Func &&func)
		: Task<void>(), m_func(std::forward<Func>(func)) {}

	void runImpl() override
	{
		m_func(*notifier());
	}
};

template <typename Func>
auto createTask(Func &&func)
{
	using Type = typename Ralph::Common::Functional::FunctionTraits<Func>::ReturnType;
	return std::static_pointer_cast<Task<Type>>(std::make_shared<LambdaTask<Func, Type>>(std::forward<Func>(func)));
}

template <typename T>
inline T await(const std::shared_ptr<Task<T>> &task)
{
	QFuture<T> future = task->future();
	future.waitForFinished();
	return future.result();
}
template <>
inline void await(const TaskPtr<void> &task)
{
	task->future().waitForFinished();
}

template <typename A, typename B>
auto operator+(const std::shared_ptr<Task<A>> &a, const std::shared_ptr<Task<B>> &b)
{
	return createTask([a, b](Notifier notifier) { return notifier.await(a) + notifier.await(b); });
}

}
}
