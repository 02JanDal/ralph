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

#include "Future.h"

namespace Ralph {
namespace ClientLib {
namespace Private {

BaseFutureData::~BaseFutureData() {}
BaseFuture::~BaseFuture() {}

void BaseFuture::addWatcher(BaseFutureWatcher *watcher)
{
	d->watchers.insert(watcher);
}
void BaseFuture::removeWatcher(BaseFutureWatcher *watcher)
{
	d->watchers.erase(watcher);
}
void BaseFuture::start()
{
	d->start();
}
void BaseFuture::waitForFinished()
{
	d->start();
	d->future.wait();
}

void BaseFutureData::start()
{
	std::unique_lock<std::mutex> lock(mutex);
	if (state != BaseFutureData::Scheduled) {
		return;
	}
	state = BaseFutureData::Running;
	// reporting will be done from the other thread
	startupMutex.unlock();
}

}
}
}
