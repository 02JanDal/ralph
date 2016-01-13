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

#pragma once

#include "Exception.h"

class QDir;
class QFileInfo;
class QIODevice;
class QString;
class QByteArray;

namespace FS
{
DECLARE_EXCEPTION(FileSystem);

void ensureExists(const QDir &dir);
void remove(const QString &filename);
void remove(const QDir &dir);
void remove(const QFileInfo &info);
void copy(const QString &from, const QString &to);
void copy(const QString &from, const QDir &to);
void move(const QString &from, const QString &to);
void write(const QString &filename, const QByteArray &data);
void touch(const QString &filename);
bool exists(const QString &filename);
bool exists(const QDir &directory);
QByteArray read(const QString &filename);

void chunkedTransfer(QIODevice *from, QIODevice *to);

void removeEmptyRecursive(const QDir &dir);
void mergeDirectoryInto(const QDir &source, const QDir &destination);
}
