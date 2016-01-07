#pragma once

#include <QString>
#include <exception>

class Exception : public std::exception
{
public:
	Exception(const QString &message);
	Exception(const Exception &other);
	virtual ~Exception() noexcept {}
	const char *what() const noexcept;
	QString cause() const { return m_message; }

private:
	QString m_message;
};

#define DECLARE_EXCEPTION(name)                 \
	class name##Exception : public ::Exception  \
	{                                           \
	public:                                     \
		using ::Exception::Exception;           \
	}
