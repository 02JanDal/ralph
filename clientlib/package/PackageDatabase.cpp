#include "PackageDatabase.h"

#include "Functional.h"
#include "Exception.h"
#include "Json.h"
#include "PackageSource.h"

namespace Ralph {
using namespace Common;

namespace ClientLib {

PackageDatabase::PackageDatabase(const QDir &dir, const QVector<PackageDatabase *> &inherits, QObject *parent)
	: QObject(parent), m_dir(dir), m_inherits(inherits), m_mutex(QMutex::Recursive)
{
}

Future<PackageDatabase *> PackageDatabase::get(const QDir &dir, const QVector<PackageDatabase *> inherits)
{
	return async([dir, inherits](Notifier notifier) -> PackageDatabase *
	{
		if (!dir.exists()) {
			if (!dir.mkpath(dir.absolutePath())) {
				return nullptr;
			}
		}

		PackageDatabase *db = new PackageDatabase(dir, Functional::filter(inherits, Functional::IsNull));
		Functional::each(db->inheritedDatabases(), [db](QObject *obj) { obj->setParent(db); });
		notifier.await(db->load());
		notifier.await(db->build());
		return db;
	});
}

bool PackageDatabase::isReadonly() const
{
	return !QFileInfo(m_dir.absolutePath()).isWritable();
}

Future<void> PackageDatabase::load()
{
	return async([this](Notifier notifier)
	{
		Q_UNUSED(notifier)
		using namespace Json;
		QMutexLocker locker(&m_mutex);
		if (m_dir.exists("db.json")) {
			const QJsonObject obj = ensureObject(ensureDocument(m_dir.absoluteFilePath("db.json")));
			m_sources = Functional::collection(ensureIsArrayOf<QJsonObject>(obj, "sources"))
					.map([this](const QJsonObject &o) { return PackageSource::fromJson(o, this); })
					.tap([this](PackageSource *source) { source->setBasePath(m_dir.absoluteFilePath("sources/" + source->name())); });
		}
	});
}
Future<void> PackageDatabase::save()
{
	return async([this]()
	{
		using namespace Json;
		QMutexLocker locker(&m_mutex);
		QJsonObject obj;
		obj.insert("sources", Json::toJsonArray(m_sources));
		write(obj, m_dir.absoluteFilePath("db.json"));
	});
}

Future<void> PackageDatabase::build()
{
	return async([this]() {});
}

const Package *PackageDatabase::getPackage(const QString &name, const Version &version) const
{
	QMutexLocker locker(&m_mutex);
	for (const Package *pkg : m_packageMapping.values(name)) {
		if (pkg->version() == version) {
			return pkg;
		}
	}
	for (const PackageDatabase *db : m_inherits) {
		const Package *pkg = db->getPackage(name, version);
		if (pkg) {
			return pkg;
		}
	}
	return nullptr;
}
QVector<const Package *> PackageDatabase::findPackages(const QString &name, VersionRequirement * const version) const
{
	QMutexLocker locker(&m_mutex);
	QVector<const Package *> out;
	out.append(Functional::filter2<QVector<const Package *>>(m_packageMapping.values(name), [version](const Package *pkg) { return version->accepts(pkg->version()); }));
	for (const PackageDatabase *db : m_inherits) {
		out.append(db->findPackages(name, version));
	}
	return out;
}

PackageSource *PackageDatabase::source(const QString &name) const
{
	QMutexLocker locker(&m_mutex);
	auto it = std::find_if(m_sources.begin(), m_sources.end(), [name](const PackageSource *src) { return src->name() == name; });
	if (it == m_sources.end()) {
		throw Exception("No source with that name found");
	}
	return *it;
}
Future<void> PackageDatabase::registerPackageSource(PackageSource *source)
{
	return async([this, source](Notifier notifier)
	{
		auto it = std::find_if(m_sources.begin(), m_sources.end(), [source](const PackageSource *src) { return src->name() == source->name(); });
		if (it != m_sources.end()) {
			throw Exception("A source with the name '%1' already exists in the current database" % source->name());
		}

		QMutexLocker locker(&m_mutex);
		m_sources.append(source);
		source->setBasePath(m_dir.absoluteFilePath("sources/" + source->name()));
		source->setParent(this);

		notifier.await(save());
		notifier.await(build());
	});
}
Future<void> PackageDatabase::unregisterPackageSource(const QString &name)
{
	return async([this, name](Notifier notifier)
	{
		QMutexLocker locker(&m_mutex);
		auto it = std::find_if(m_sources.begin(), m_sources.end(), [name](const PackageSource *src) { return src->name() == name; });
		if (it == m_sources.end()) {
			throw Exception("No source with that name found");
		}
		PackageSource *source = *it;
		m_sources.erase(it);

		if (source->basePath().exists() && source->basePath().rmdir(source->basePath().absolutePath())) {
			notifier.status("Cleanup failed, manual cleanup of '%1' required" % source->basePath().absolutePath());
		}

		notifier.await(save());
		notifier.await(build());
	});
}

}
}
