#include "GitException.h"

#include <git2/errors.h>

GitException::GitException(const char *error, const int id)
	: Exception(QString::fromLocal8Bit(error)), m_error(id)
{
}

void GitException::checkAndThrow(const int error)
{
	if (error < 0) {
		throw GitException(giterr_last()->message, error);
	}
}
