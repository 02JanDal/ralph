#pragma once

#include <QString>
#include <exception>

#include "Formatting.h"

class Exception : public std::exception
{
public:
	Exception(const QString &message);
	Exception(const Exception &other);
	virtual ~Exception() noexcept;
	const char *what() const noexcept;
	QString cause() const { return m_message; }

private:
	QString m_message;
};

#define DECLARE_EXCEPTION(name)                 \
	QT_WARNING_PUSH                             \
	QT_WARNING_DISABLE_GCC("-Wweak-vtables")    \
	class name##Exception : public ::Exception  \
	{                                           \
	public:                                     \
		using ::Exception::Exception;           \
	}                                           \
	QT_WARNING_POP                              \
