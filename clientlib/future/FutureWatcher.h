#pragma once

#include <QObject>

#include "Future.h"

Q_DECLARE_METATYPE(std::size_t)

namespace Ralph {
namespace ClientLib {
namespace Private {
class BaseFutureWatcher : public QObject
{
	Q_OBJECT
public:
	explicit BaseFutureWatcher();
	virtual ~BaseFutureWatcher();

signals:
	void started();
	void finished();
	void canceled();
	void exception();
	void progress(const std::size_t current, const std::size_t total);
	void status(const QString &message);
};
}

template <typename T>
class FutureWatcher : public Private::BaseFutureWatcher
{
public:
	explicit FutureWatcher(const Future<T> &future)
		: m_future(future)
	{
		m_future.addWatcher(this);
	}
	~FutureWatcher()
	{
		m_future.removeWatcher(this);
	}

	Future<T> future() const { return m_future; }

private:
	Future<T> m_future;
};

}
}
