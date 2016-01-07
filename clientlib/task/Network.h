#pragma once

#include "Task.h"
#include "Exception.h"

namespace Ralph {
namespace ClientLib {
namespace Network {

class NetworkException : public Exception
{
public:
	using Exception::Exception;

	static void throwIfError(int code, const char *errorbuffer = nullptr);
};

void init();
Task<void>::Ptr download(const QUrl &url, const QString &destination);
Task<QByteArray>::Ptr get(const QUrl &url);

}
}
}
