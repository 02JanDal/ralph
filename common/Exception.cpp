#include "Exception.h"

#include <QLoggingCategory>

Exception::Exception(const QString &message) : std::exception(), m_message(message)
{
	qCritical() << "Exception:" << message;
}

Exception::Exception(const Exception &other)
	: std::exception(), m_message(other.cause())
{
}

const char *Exception::what() const noexcept
{
	return m_message.toLatin1().constData();
}
