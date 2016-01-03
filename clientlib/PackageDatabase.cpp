#include "PackageDatabase.h"

#include "Functional.h"
#include "Exception.h"
#include "Json.h"
#include "PackageSource.h"

using namespace Ralph::Common;

PackageDatabase::PackageDatabase(const QDir &dir, const QVector<PackageDatabase *> &inherits, QObject *parent)
	: QObject(parent), m_dir(dir), m_inherits(inherits)
{
}

Task<PackageDatabase *>::Ptr PackageDatabase::get(const QDir &dir, const QVector<PackageDatabase *> inherits)
{
	return createTask([dir, inherits](Notifier notifier)
	{
		if (!dir.exists()) {
			if (!dir.mkpath(dir.absolutePath())) {
				throw Exception("Unable to create directory for package database: " + dir.absolutePath());
			}
		}

		PackageDatabase *db = new PackageDatabase(dir, Functional::filter(inherits, Functional::IsNull));
		notifier.await(db->read());
		notifier.await(db->build());
		return db;
	});
}

bool PackageDatabase::isReadonly() const
{
	return !QFileInfo(m_dir.absolutePath()).isWritable();
}

Task<void>::Ptr PackageDatabase::update()
{
	return createTask([this](Notifier notifier)
	{
		Q_UNUSED(notifier)
	});
}

Task<void>::Ptr PackageDatabase::read()
{
	return createTask([this](Notifier notifier)
	{
		Q_UNUSED(notifier)
		using namespace Json;
		if (m_dir.exists("db.json")) {
			const QJsonObject obj = ensureObject(ensureDocument(m_dir.absoluteFilePath("db.json")));
			m_sources = Functional::map(ensureIsArrayOf<QJsonObject>(obj, "sources"),
										[this](const QJsonObject &o) { return PackageSource::fromJson(o, this); });
		}
	});
}

Task<void>::Ptr PackageDatabase::build()
{
	return {};
}

Task<void>::Ptr PackageDatabase::registerPackageSource(PackageSource *source)
{
	m_sources.append(source);
	return createTask([this](Notifier notifier)
	{
		notifier.await(save());
		notifier.await(build());
	});
}

Task<void>::Ptr PackageDatabase::save()
{
	return createTask([]() {});
}
