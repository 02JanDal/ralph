#include "Exception.h"

Exception::Exception(const QString &message) : std::exception(), m_message(message) {}

Exception::Exception(const Exception &other)
	: std::exception(), m_message(other.cause())
{
}

Exception::~Exception() noexcept {}

const char *Exception::what() const noexcept
{
	return m_message.toLatin1().constData();
}
