/* Copyright 2016 Jan Dalheimer <jan@dalheimer.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Network.h"

#include <QString>
#include <QUrl>
#include <QBuffer>
#include <QFile>
#include <QDir>
#include <QFileInfo>

#include <curl/curl.h>

namespace Ralph {
namespace ClientLib {
namespace Network {

QT_WARNING_PUSH
QT_WARNING_DISABLE_GCC("-Wdisabled-macro-expansion")

struct NetworkCallbackData
{
	Notifier notifier;
};

static int progressCallback(void *data, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow)
{
	NetworkCallbackData *ncd = static_cast<NetworkCallbackData *>(data);
	ncd->notifier.progress(std::size_t(dlnow + ulnow), std::size_t(dltotal + ultotal));
	return CURLE_OK;
}
static size_t writeCallback(void *contents, size_t size, size_t nmemb, void *data)
{
	QIODevice *device = static_cast<QIODevice *>(data);

	const size_t realsize = size * nmemb;
	device->write(static_cast<const char *>(contents), static_cast<qint64>(realsize));

	return realsize;
}

static void commonSetup(CURL *curl, const QUrl &url, const NetworkCallbackData &progressData, QIODevice *device, const char *errorbuffer)
{
	// error check function
	auto ec = [errorbuffer](CURLcode code) { NetworkException::throwIfError(code, errorbuffer); };

	ec(curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorbuffer));
	ec(curl_easy_setopt(curl, CURLOPT_USERAGENT, "ralph"));
	ec(curl_easy_setopt(curl, CURLOPT_URL, url.toString(QUrl::FullyEncoded).toUtf8().constData()));
	ec(curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L));
	ec(curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, &progressCallback));
	ec(curl_easy_setopt(curl, CURLOPT_XFERINFODATA, &progressData));
	ec(curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &writeCallback));
	ec(curl_easy_setopt(curl, CURLOPT_WRITEDATA, device));
}

Future<void> download(const QUrl &url, const QString &destination)
{
	const QFileInfo info(destination);
	const QString dest = info.exists() && info.isDir() ?
				QDir(destination).absoluteFilePath(QFileInfo(url.path()).fileName())
			  : destination;

	return async([url, dest](Notifier notifier)
	{
		NetworkCallbackData progressData{notifier};

		char errorbuf[CURL_ERROR_SIZE];
		errorbuf[0] = '\0';

		// error check function
		auto ec = [errorbuf](CURLcode code) { NetworkException::throwIfError(code, errorbuf); };

		CURL *curl = curl_easy_init();
		try {
			QFile file(dest);

			if (!file.open(QFile::WriteOnly | QFile::Truncate)) {
				throw NetworkException("Unable to open download destionation: " + file.errorString());
			}

			commonSetup(curl, url, progressData, &file, errorbuf);
			ec(curl_easy_perform(curl));
			curl_easy_cleanup(curl);
		} catch (...) {
			curl_easy_cleanup(curl);
			/*re-*/throw;
		}
	});
}
Future<QByteArray> get(const QUrl &url)
{
	return async([url](Notifier notifier)
	{
		NetworkCallbackData progressData{notifier};

		char errorbuf[CURL_ERROR_SIZE];
		errorbuf[0] = '\0';

		// error check function
		auto ec = [errorbuf](CURLcode code) { NetworkException::throwIfError(code, errorbuf); };

		CURL *curl = curl_easy_init();
		try {
			QBuffer buffer;

			commonSetup(curl, url, progressData, &buffer, errorbuf);
			ec(curl_easy_perform(curl));
			curl_easy_cleanup(curl);

			return buffer.data();
		} catch (...) {
			curl_easy_cleanup(curl);
			/*re-*/throw;
		}
	});
}

void init()
{
	curl_global_init(CURL_GLOBAL_DEFAULT);
}

NetworkException::~NetworkException() {}

void NetworkException::throwIfError(int code, const char *errorbuffer)
{
	if (code != CURLE_OK) {
		const QString error = errorbuffer ? QString::fromLocal8Bit(errorbuffer, int(qstrlen(errorbuffer))) : curl_easy_strerror(static_cast<CURLcode>(code));
		throw NetworkException("Network error: " + error);
	}
}

}
}
}
