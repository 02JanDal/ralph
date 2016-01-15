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

#include "FileSystem.h"

#include <QDir>
#include <QSaveFile>
#include <QFileInfo>

void FS::ensureExists(const QDir &dir)
{
	if (!QDir().mkpath(dir.absolutePath()))
	{
		throw FileSystemException("Unable to create directory " + dir.dirName() + " (" +
								  dir.absolutePath() + ")");
	}
}

void FS::remove(const QString &filename)
{
	QFile file(filename);
	if (!file.remove())
	{
		throw FileSystemException("Unable to remove " + filename + ": " + file.errorString());
	}
}
void FS::remove(const QDir &dir)
{
	if (!QDir(dir).removeRecursively())
	{
		throw FileSystemException("Unable to remove " + dir.dirName());
	}
}
void FS::remove(const QFileInfo &info)
{
	if (info.isDir())
	{
		remove(QDir(info.absoluteFilePath()));
	}
	else if (info.isFile())
	{
		remove(info.absoluteFilePath());
	}
}

void FS::write(const QString &filename, const QByteArray &data)
{
	ensureExists(QFileInfo(filename).dir());
	QSaveFile file(filename);
	if (!file.open(QSaveFile::WriteOnly))
	{
		throw FileSystemException("Couldn't open " + filename + " for writing: " +
								  file.errorString());
	}
	if (data.size() != file.write(data))
	{
		throw FileSystemException("Error writing data to " + filename + ": " +
								  file.errorString());
	}
	if (!file.commit())
	{
		throw FileSystemException("Error while committing data to " + filename + ": " +
								  file.errorString());
	}
}
void FS::touch(const QString &filename)
{
	ensureExists(QFileInfo(filename).dir());
	QFile file(filename);
	if (!file.open(QFile::WriteOnly | QFile::Append))
	{
		throw FileSystemException("Couldn't open " + filename + " for writing: " +
								  file.errorString());
	}
	file.close();
}

QByteArray FS::read(const QString &filename)
{
	QFile file(filename);
	if (!file.open(QFile::ReadOnly))
	{
		throw FileSystemException("Unable to open " + filename + " for reading: " +
								  file.errorString());
	}
	const qint64 size = file.size();
	QByteArray data(int(size), 0);
	const qint64 ret = file.read(data.data(), size);
	if (ret == -1 || ret != size)
	{
		throw FileSystemException("Error reading data from " + filename + ": " +
								  file.errorString());
	}
	return data;
}

void FS::copy(const QString &from, const QString &to)
{
	FS::ensureExists(QFileInfo(to).dir());
	if (exists(to))
	{
		remove(to);
	}
	QFile file(from);
	if (!file.copy(to))
	{
		throw FileSystemException("Error copying file to " + to + ": " + file.errorString());
	}
}
void FS::copy(const QString &from, const QDir &to)
{
	copy(from, to.absoluteFilePath(QFileInfo(from).fileName()));
}

void FS::move(const QString &from, const QString &to)
{
	copy(from, to);
	remove(from);
}

bool FS::exists(const QString &filename)
{
	return QDir().exists(filename);
}
bool FS::exists(const QDir &directory)
{
	return directory.exists();
}

void FS::removeEmptyRecursive(const QDir &dir)
{
	for (const QString &child : dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden))
	{
		removeEmptyRecursive(QDir(dir.absoluteFilePath(child)));
	}
	if (dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot | QDir::Files | QDir::Hidden |
					  QDir::System).isEmpty())
	{
		remove(dir);
	}
}
void FS::mergeDirectoryInto(const QDir &source, const QDir &destination)
{
	// ensure the directory itself exists
	if (!FS::exists(destination))
	{
		FS::ensureExists(destination.absolutePath());
	}
	// and then move all the contents
	for (const QFileInfo &entry :
		 source.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot))
	{
		if (entry.isFile())
		{
			copy(entry.absoluteFilePath(), destination.absoluteFilePath(entry.fileName()));
		}
		else
		{
			mergeDirectoryInto(QDir(entry.absoluteFilePath()),
							   destination.absoluteFilePath(entry.fileName()));
		}
	}
}

void FS::chunkedTransfer(QIODevice *from, QIODevice *to)
{
	Q_ASSERT(from->isReadable() && to->isWritable());

	static constexpr qint64 bufferSize = 1024 * 1024;
	qint64 remaining = from->bytesAvailable();
	char buffer[bufferSize];
	while (remaining > 0)
	{
		const qint64 currentChunkSize = std::min(bufferSize, remaining);
		const qint64 actualSize = from->read(buffer, currentChunkSize);
		Q_ASSERT(currentChunkSize == actualSize);
		if (currentChunkSize != to->write(buffer, currentChunkSize))
		{
			throw FileSystemException(QString("Error during chunked transfer: %1").arg(to->errorString()));
		}
		remaining -= currentChunkSize;
	}
}
