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
	NetworkException(const NetworkException &e) = default;
	virtual ~NetworkException();

	static void throwIfError(int code, const char *errorbuffer = nullptr);
};

void init();
Future<void> download(const QUrl &url, const QString &destination);
Future<QByteArray> get(const QUrl &url);

}
}
}
