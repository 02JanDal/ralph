#include "WrappedException.h"

namespace Ralph {
namespace ClientLib {

QString WrappedException::message(const std::exception_ptr &exception)
{
	if (exception) {
		try {
			std::rethrow_exception(exception);
		} catch (Exception &e) {
			return e.cause();
		} catch (std::exception &e) {
			return QString(e.what());
		} catch (...) {
			return QString();
		}
	} else {
		return QString();
	}
}

WrappedException::WrappedException(const std::exception_ptr &exception)
	: Exception(message(exception)), m_exception(exception) {}

WrappedException::WrappedException() : Exception("") {}

WrappedException::~WrappedException() {}

void WrappedException::rethrow()
{
	if (m_exception) {
		std::rethrow_exception(m_exception);\
	}
}

}
}
