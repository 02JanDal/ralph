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

#include <QTest>
#include <QSignalSpy>

#include "future/Future.h"
#include "future/FutureWatcher.h"
#include "future/Promise.h"
#include "future/FutureOperators.h"
#include "task/Task.h"

using namespace Ralph::ClientLib;
using namespace std::literals;

class Future_Test : public QObject
{
	Q_OBJECT
public:
	virtual ~Future_Test();

private slots:
	void futuresAndPromisesHangTogether()
	{
		Promise<int> promise;
		Future<int> future1 = promise.future();
		Future<int> future2 = promise.future();
		Future<int> future3 = future2;

		promise.prime(std::async(std::launch::deferred, [&promise]()
		{
			promise.reportStarted();
			promise.reportResult(42);
			promise.reportFinished();
		}));

		QCOMPARE(future1.result(), 42);
		QCOMPARE(future2.result(), 42);
		QCOMPARE(future3.result(), 42);
	}
	void futureCanDowncastToVoid()
	{
		Promise<int> promise;
		Future<int> future1 = promise.future();
		Future<void> future2 = promise.future();
		Future<void> future3 = future1;
	}
	void futureWatcher()
	{
		std::mutex mutex;
		mutex.lock();
		Future<int> future = async(std::launch::async, [&mutex](Notifier notifier)
		{
			notifier.status("asdf");
			notifier.progress(1, 100);
			mutex.lock();
			mutex.unlock();
			notifier.progress(99, 100);
			notifier.status("fdsa");
			return 42;
		});

		FutureWatcher<int> watcher(future);
		QSignalSpy started(&watcher, SIGNAL(started()));
		QSignalSpy finished(&watcher, SIGNAL(finished()));
		QSignalSpy canceled(&watcher, SIGNAL(canceled()));
		QSignalSpy exception(&watcher, SIGNAL(exception()));
		QSignalSpy progress(&watcher, SIGNAL(progress(std::size_t, std::size_t)));
		QSignalSpy status(&watcher, SIGNAL(status(QString)));

		future.start();
		std::this_thread::sleep_for(1ms);
		QCOMPARE(started.size(), 1);
		QVERIFY(started.first().isEmpty());
		QVERIFY(finished.isEmpty());
		QVERIFY(canceled.isEmpty());
		QVERIFY(exception.isEmpty());
		QCOMPARE(progress.size(), 1);
		QCOMPARE(progress.first(), QList<QVariant>() << 1 << 100);
		QCOMPARE(status.size(), 1);
		QCOMPARE(status.first(), QList<QVariant>() << "asdf");

		mutex.unlock();
		future.result();

		QCOMPARE(started.size(), 1); // stays the same
		QVERIFY(started.first().isEmpty()); // stays the same
		QCOMPARE(finished.size(), 1); // incremented
		QVERIFY(finished.first().isEmpty());
		QVERIFY(canceled.isEmpty()); // stays the same
		QVERIFY(exception.isEmpty()); // stays the same
		QCOMPARE(progress.size(), 2);
		QCOMPARE(progress.at(0), QList<QVariant>() << 1 << 100);
		QCOMPARE(progress.at(1), QList<QVariant>() << 99 << 100);
		QCOMPARE(status.size(), 2);
		QCOMPARE(status.at(0), QList<QVariant>() << "asdf");
		QCOMPARE(status.at(1), QList<QVariant>() << "fdsa");
	}
	void futureOperators()
	{
		QCOMPARE((async([]() { return 42; }) + async([]() { return 2; })).result(), 44);
		QCOMPARE((async([]() { return 42; }) - async([]() { return 2; })).result(), 40);
		QCOMPARE((async([]() { return 42; }) * async([]() { return 2; })).result(), 84);
		QCOMPARE((async([]() { return 42; }) / async([]() { return 2; })).result(), 21);
		QCOMPARE((async([]() { return 42; }) % async([]() { return 2; })).result(), 0);

		QCOMPARE((async([]() { return true; }) && async([]() { return false; })).result(), false);
		QCOMPARE((async([]() { return true; }) || async([]() { return false; })).result(), true);
	}
};

Future_Test::~Future_Test() {}

QTEST_GUILESS_MAIN(Future_Test)

#include "Future_Test.moc"
