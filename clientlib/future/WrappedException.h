#pragma once

#include <exception>

#include "Exception.h"

namespace Ralph {
namespace ClientLib {

class WrappedException : public Exception
{
	const std::exception_ptr m_exception;
	static QString message(const std::exception_ptr &exception);

public:
	explicit WrappedException(const std::exception_ptr &exception);
	explicit WrappedException();
	virtual ~WrappedException();

	void rethrow();
};

}
}
