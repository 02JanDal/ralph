#include "Archive.h"

#include <QMimeDatabase>
#include <QDir>
#include <QStack>

#include <KArchive/KArchive>
#include <KArchive/KTar>
#include <KArchive/K7Zip>
#include <KArchive/KZip>
#include <KArchive/KAr>

namespace Ralph {
namespace ClientLib {
namespace Archive {



Future<void> extract(const QString &filename, const QDir &destination)
{
	return async([filename, destination](Notifier notifier)
	{
		const QMimeType mimetype = QMimeDatabase().mimeTypeForFile(filename);

		KArchive *ark = nullptr;
		if (mimetype.inherits("application/x-compressed-tar") || mimetype.inherits("application/x-bzip-compressed-tar") ||
				mimetype.inherits("application/x-lzma-compressed-tar") || mimetype.inherits("application-xz-compressed-tar") ||
				mimetype.inherits("application/x-tar")) {
			ark = new KTar(filename);
		} else if (mimetype.inherits("application/zip")) {
			ark = new KZip(filename);
		} else if (mimetype.inherits("application/x-7z-compressed")) {
			ark = new K7Zip(filename);
		} else if (mimetype.inherits("application/x-archive")) {
			ark = new KAr(filename);
		}

		if (!ark->open(QIODevice::ReadOnly)) {
			throw ArchiveException("Unable to open archive");
		}

		// the following code is taken from KArchiveDirectory::copyTo

		QVector<const KArchiveFile *> fileList;
		QHash<qint64, QString> fileToDir;
		std::size_t totalSize = 0;

		// placeholders for iterated items
		QStack<const KArchiveDirectory *> dirStack;
		QStack<QString> dirNameStack;

		dirStack.push(ark->directory());               // init stack at current directory
		dirNameStack.push(destination.absolutePath()); // ... with given path
		do {
			const KArchiveDirectory *curDir = dirStack.pop();
			const QString curDirName = dirNameStack.pop();
			if (!destination.mkpath(curDirName)) {
				throw ArchiveException("Unable to create directory");
			}

			const QStringList dirEntries = curDir->entries();
			for (const QString &dirEntry : dirEntries) {
				const KArchiveEntry *curEntry = curDir->entry(dirEntry);
				if (!curEntry->symLinkTarget().isEmpty()) {
					QString linkName = curDirName + QLatin1Char('/') + curEntry->name();
					// To create a valid link on Windows, linkName must have a .lnk file extension.
#ifdef Q_OS_WIN
					if (!linkName.endsWith(QLatin1String(".lnk"))) {
						linkName += QLatin1String(".lnk");
					}
#endif
					QFile symLinkTarget(curEntry->symLinkTarget());
					if (!symLinkTarget.link(linkName)) {
						throw ArchiveException("Unable to extract symlink: " + symLinkTarget.errorString());
					}
				} else {
					if (curEntry->isFile()) {
						const KArchiveFile *curFile = dynamic_cast<const KArchiveFile *>(curEntry);
						if (curFile) {
							fileList.append(curFile);
							fileToDir.insert(curFile->position(), curDirName);
							totalSize += std::size_t(curFile->size());
						}
					} else if (curEntry->isDirectory()) {
						const KArchiveDirectory *ad = dynamic_cast<const KArchiveDirectory *>(curEntry);
						if (ad) {
							dirStack.push(ad);
							dirNameStack.push(curDirName + QLatin1Char('/') + curEntry->name());
						}
					}
				}
			}
		} while (!dirStack.isEmpty());

		std::sort(fileList.begin(), fileList.end(), [](const KArchiveFile *a, const KArchiveFile *b) { return a->position() < b->position(); });    // sort on d->pos, so we have a linear access

		notifier.progress(0, totalSize);

		std::size_t currentSize = 0;

		for (const KArchiveFile *f : fileList) {
			qint64 pos = f->position();
			if (!f->copyTo(fileToDir[pos])) {
				throw ArchiveException("Unable to extract file");
			}
			currentSize += std::size_t(f->size());
			notifier.progress(currentSize, totalSize);
		}
	});
}

}
}
}
