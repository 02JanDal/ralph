#include "Task.h"

namespace Ralph {
namespace ClientLib {

Notifier::Notifier(BaseStatusObject *status, QFutureInterfaceBase *future)
	: m_status(status), m_future(future) {}

void Notifier::status(const QString &status) const
{
	if (m_delegateTo) {
		m_delegateTo->status(status);
	}
	m_status->status(status);
}

void Notifier::progressCurrent(const int current) const
{
	if (m_delegateTo) {
		m_delegateTo->progressCurrent(current);
	}
	m_future->setProgressValue(current);
}

void Notifier::progressTotal(const int total) const
{
	if (m_delegateTo) {
		m_delegateTo->progressTotal(total);
	}
	m_future->setProgressRange(0, total);
}

}
}
