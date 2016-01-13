#include "PackageDatabase.h"

#include <QDataStream>

#include "Functional.h"
#include "Exception.h"
#include "Json.h"
#include "PackageSource.h"
#include "PackageGroup.h"

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
		db->load();
		notifier.await(db->build());
		return db;
	});
}

bool PackageDatabase::isReadonly() const
{
	return !QFileInfo(m_dir.absolutePath()).isWritable();
}

void PackageDatabase::load()
{
	using namespace Json;
	QMutexLocker locker(&m_mutex);
	if (m_dir.exists("db.json")) {
		const QJsonObject root = ensureObject(ensureDocument(m_dir.absoluteFilePath("db.json")));

		m_sources = Functional::collection(ensureIsArrayOf<QJsonObject>(root, "sources"))
				.map([this](const QJsonObject &o) { return PackageSource::fromJson(o, this); })
				.tap([this](PackageSource *source) { source->setBasePath(m_dir.absoluteFilePath("sources/" + source->name())); });

		m_groups = Functional::map(ensureIsArrayOf<QJsonObject>(root, "groups", QVector<QJsonObject>()), [this](const QJsonObject &obj)
		{
			return std::make_shared<PackageGroup>(ensureString(obj, "name"), m_dir.absoluteFilePath(ensureString(obj, "dir")));
		});
	}
}
void PackageDatabase::save()
{
	using namespace Json;
	QMutexLocker locker(&m_mutex);
	QJsonObject obj;
	obj.insert("sources", Json::toJsonArray(m_sources));
	obj.insert("groups", Json::toJsonArray(Functional::map(m_groups, [this](const std::shared_ptr<PackageGroup> &group)
	{
		return QJsonObject({
							   qMakePair(QStringLiteral("name"), group->name()),
							   qMakePair(QStringLiteral("dir"), m_dir.relativeFilePath(group->dir().absolutePath()))
						   });
	})));
	write(obj, m_dir.absoluteFilePath("db.json"));
}

Future<void> PackageDatabase::build()
{
	return async([this](Notifier notifier)
	{
		QMutexLocker locker(&m_mutex);

		// step 1: read the cache and check if we actually changed
		{
			QFile f(m_dir.absoluteFilePath("cache.dat"));
			if (f.open(QFile::ReadOnly)) {
				QDataStream str(&f);
				QHash<QString, QDateTime> cacheSources;
				str >> cacheSources;

				QHash<QString, QDateTime> currentSources;
				for (const PackageSource *src : m_sources) {
					currentSources.insert(src->name(), src->lastUpdated());
				}

				if (cacheSources == currentSources) {
					// TODO read the packages from the cache
					return;
				}
			}
		}

		// step 2: read all packages from all sources
		{
			m_packageMapping.clear();
			m_packages = Functional::collection(m_sources)
					.map([this, notifier](const PackageSource *src)
			{
				notifier.status("Reading packages for '%1'..." % src->name());
				return notifier.await(src->packages());
			})
					.flatten()
					.tap([this](const Package *pkg) { m_packageMapping.insert(pkg->name().toLower(), pkg); });
		}

		// step 3: write the cache
		{
			// TODO write packages into the cache.dat file
			// wait until it's actually needed though
		}
	});
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
QVector<const Package *> PackageDatabase::findPackages(const QString &name, const VersionRequirement *version) const
{
	QMutexLocker locker(&m_mutex);
	QVector<const Package *> out;
	out.append(Functional::filter2<QVector<const Package *>>(m_packageMapping.values(name.toLower()), [version](const Package *pkg) { return !version || version->accepts(pkg->version()); }));
	for (const PackageDatabase *db : m_inherits) {
		out.append(db->findPackages(name, version));
	}
	return out;
}

QVector<QString> PackageDatabase::packageNames() const
{
	QMutexLocker locker(&m_mutex);
	return m_packageMapping.keys().toVector();
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

		save();
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

		save();
		notifier.await(build());
	});
}

std::shared_ptr<PackageGroup> PackageDatabase::group(const QString &name)
{
	if (name.isNull()) {
		return group("default");
	}

	for (const std::shared_ptr<PackageGroup> &candidate : m_groups) {
		if (candidate->name() == name) {
			return candidate;
		}
	}

	std::shared_ptr<PackageGroup> newGroup = std::make_shared<PackageGroup>(name, m_dir.absoluteFilePath("groups/%1" % name.toLower().replace(QRegExp("[^a-zA-Z0-9-_]"), "")));
	m_groups.append(newGroup);
	save();
	return newGroup;
}

}
}
