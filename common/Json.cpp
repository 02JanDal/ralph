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

#include "Json.h"

#include <QFile>
#include <QSaveFile>

#include <cmath>

#include "FileSystem.h"

void Json::write(const QJsonDocument &doc, const QString &filename)
{
	FS::write(filename, doc.toJson());
}
void Json::write(const QJsonObject &object, const QString &filename)
{
	write(QJsonDocument(object), filename);
}
void Json::write(const QJsonArray &array, const QString &filename)
{
	write(QJsonDocument(array), filename);
}

QByteArray Json::toBinary(const QJsonObject &obj)
{
	return QJsonDocument(obj).toBinaryData();
}
QByteArray Json::toBinary(const QJsonArray &array)
{
	return QJsonDocument(array).toBinaryData();
}
QByteArray Json::toText(const QJsonObject &obj)
{
	return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}
QByteArray Json::toText(const QJsonArray &array)
{
	return QJsonDocument(array).toJson(QJsonDocument::Compact);
}

static bool isBinaryJson(const QByteArray &data)
{
	decltype(QJsonDocument::BinaryFormatTag) tag = QJsonDocument::BinaryFormatTag;
	return memcmp(data.constData(), &tag, sizeof(QJsonDocument::BinaryFormatTag)) == 0;
}
QJsonDocument Json::ensureDocument(const QByteArray &data)
{
	if (isBinaryJson(data))
	{
		QJsonDocument doc = QJsonDocument::fromBinaryData(data);
		if (doc.isNull())
		{
			throw JsonException("Invalid JSON (binary JSON detected)");
		}
		return doc;
	}
	else
	{
		QJsonParseError error;
		QJsonDocument doc = QJsonDocument::fromJson(data, &error);
		if (error.error != QJsonParseError::NoError)
		{
			throw JsonException("Error parsing JSON: " + error.errorString());
		}
		return doc;
	}
}
QJsonDocument Json::ensureDocument(const QString &filename)
{
	return Json::ensureDocument(FS::read(filename));
}
QJsonObject Json::ensureObject(const QJsonDocument &doc, const QString &what)
{
	if (!doc.isObject())
	{
		throw JsonException(what + " is not an object");
	}
	return doc.object();
}
QJsonArray Json::ensureArray(const QJsonDocument &doc, const QString &what)
{
	if (!doc.isArray())
	{
		throw JsonException(what + " is not an array");
	}
	return doc.array();
}

template<>
QJsonValue Json::toJson<QUrl>(const QUrl &url)
{
	return QJsonValue(url.toString(QUrl::FullyEncoded));
}
template<>
QJsonValue Json::toJson<QByteArray>(const QByteArray &data)
{
	return QJsonValue(QString::fromLatin1(data.toHex()));
}
template<>
QJsonValue Json::toJson<QDateTime>(const QDateTime &datetime)
{
	return QJsonValue(datetime.toString(Qt::ISODate));
}
template<>
QJsonValue Json::toJson<QDir>(const QDir &dir)
{
	return QDir::current().relativeFilePath(dir.absolutePath());
}
template<>
QJsonValue Json::toJson<QUuid>(const QUuid &uuid)
{
	return uuid.toString();
}
template<>
QJsonValue Json::toJson<QVariant>(const QVariant &variant)
{
	return QJsonValue::fromVariant(variant);
}

namespace Json
{
template<> QByteArray ensureIsType<QByteArray>(const QJsonValue &value, const Requirement, const QString &what)
{
	const QString string = ensureIsType<QString>(value, Required, what);
	// ensure that the string can be safely cast to Latin1
	if (string != QString::fromLatin1(string.toLatin1()))
	{
		throw JsonException(what + " is not encodable as Latin1");
	}
	return QByteArray::fromHex(string.toLatin1());
}

template<> QJsonArray ensureIsType<QJsonArray>(const QJsonValue &value, const Requirement, const QString &what)
{
	if (!value.isArray())
	{
		throw JsonException(what + " is not an array");
	}
	return value.toArray();
}


template<> QString ensureIsType<QString>(const QJsonValue &value, const Requirement, const QString &what)
{
	if (!value.isString())
	{
		throw JsonException(what + " is not a string");
	}
	return value.toString();
}

template<> bool ensureIsType<bool>(const QJsonValue &value, const Requirement, const QString &what)
{
	if (!value.isBool())
	{
		throw JsonException(what + " is not a bool");
	}
	return value.toBool();
}

template<> double ensureIsType<double>(const QJsonValue &value, const Requirement, const QString &what)
{
	if (!value.isDouble())
	{
		throw JsonException(what + " is not a double");
	}
	return value.toDouble();
}

template<> int ensureIsType<int>(const QJsonValue &value, const Requirement, const QString &what)
{
	const double doubl = ensureIsType<double>(value, Required, what);
	if (fmod(doubl, 1) != 0)
	{
		throw JsonException(what + " is not an integer");
	}
	return int(doubl);
}

template<> QDateTime ensureIsType<QDateTime>(const QJsonValue &value, const Requirement, const QString &what)
{
	const QString string = ensureIsType<QString>(value, Required, what);
	const QDateTime datetime = QDateTime::fromString(string, Qt::ISODate);
	if (!datetime.isValid())
	{
		throw JsonException(what + " is not a ISO formatted date/time value");
	}
	return datetime;
}

template<> QUrl ensureIsType<QUrl>(const QJsonValue &value, const Requirement, const QString &what)
{
	const QString string = ensureIsType<QString>(value, Required, what);
	if (string.isEmpty())
	{
		return QUrl();
	}
	const QUrl url = QUrl(string, QUrl::StrictMode);
	if (!url.isValid())
	{
		throw JsonException(what + " is not a correctly formatted URL");
	}
	return url;
}

template<> QDir ensureIsType<QDir>(const QJsonValue &value, const Requirement, const QString &what)
{
	const QString string = ensureIsType<QString>(value, Required, what);
	// FIXME: does not handle invalid characters!
	return QDir::current().absoluteFilePath(string);
}

template<> QUuid ensureIsType<QUuid>(const QJsonValue &value, const Requirement, const QString &what)
{
	const QString string = ensureIsType<QString>(value, Required, what);
	const QUuid uuid = QUuid(string);
	if (uuid.toString() != string) // converts back => valid
	{
		throw JsonException(what + " is not a valid UUID");
	}
	return uuid;
}

template<> QJsonObject ensureIsType<QJsonObject>(const QJsonValue &value, const Requirement, const QString &what)
{
	if (!value.isObject())
	{
		throw JsonException(what + " is not an object");
	}
	return value.toObject();
}

template<> QVariant ensureIsType<QVariant>(const QJsonValue &value, const Requirement, const QString &what)
{
	if (value.isNull() || value.isUndefined())
	{
		throw JsonException(what + " is null or undefined");
	}
	return value.toVariant();
}

template<> QJsonValue ensureIsType<QJsonValue>(const QJsonValue &value, const Requirement, const QString &what)
{
	if (value.isNull() || value.isUndefined())
	{
		throw JsonException(what + " is null or undefined");
	}
	return value;
}
}
