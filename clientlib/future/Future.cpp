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
