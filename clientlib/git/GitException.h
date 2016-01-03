#pragma once

#include "Exception.h"

class GitException : public Exception
{
	int m_error = -1;
public:
	explicit GitException(const char *error, const int id);

	int error() const { return m_error; }

	static void checkAndThrow(const int error);
};
