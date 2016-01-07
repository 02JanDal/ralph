#pragma once

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDateTime>
#include <QUrl>
#include <QDir>
#include <QUuid>
#include <QVariant>
#include <QVector>

#include "Exception.h"
#include "Functional.h"

namespace Json
{
DECLARE_EXCEPTION(Json);

enum Requirement
{
	Required
};

void write(const QJsonDocument &doc, const QString &filename);
void write(const QJsonObject &object, const QString &filename);
void write(const QJsonArray &array, const QString &filename);
QByteArray toBinary(const QJsonObject &obj);
QByteArray toBinary(const QJsonArray &array);
QByteArray toText(const QJsonObject &obj);
QByteArray toText(const QJsonArray &array);

QJsonDocument ensureDocument(const QByteArray &data);
QJsonDocument ensureDocument(const QString &filename);
QJsonObject ensureObject(const QJsonDocument &doc, const QString &what = "Document");
QJsonArray ensureArray(const QJsonDocument &doc, const QString &what = "Document");

namespace detail {
template <typename T> QJsonValue toJsonHelper(const T *t, std::true_type) { return t->toJson(); }
template <typename T> QJsonValue toJsonHelper(const T &t, std::false_type) { return QJsonValue(t); }
}

template <typename T> QJsonValue toJson(const T &t)
{
	return detail::toJsonHelper(t, typename std::is_pointer<T>::type());
}
//template <typename T> QJsonValue toJson(const T &value) { return value->toJson(); }
template <> QJsonValue toJson<QUrl>(const QUrl &url);
template <> QJsonValue toJson<QByteArray>(const QByteArray &data);
template <> QJsonValue toJson<QDateTime>(const QDateTime &datetime);
template <> QJsonValue toJson<QDir>(const QDir &dir);
template <> QJsonValue toJson<QUuid>(const QUuid &uuid);
template <> QJsonValue toJson<QVariant>(const QVariant &variant);

template<typename T>
QJsonArray toJsonArray(const QVector<T> &container)
{
	QJsonArray array;
	for (const T &item : container) {
		array.append(toJson<T>(item));
	}
	return array;
}

template <typename T>
T ensureIsType(const QJsonValue &value, const Requirement = Required, const QString &what = QString());

template <> double ensureIsType<double>(const QJsonValue &value, const Requirement, const QString &what);
template <> bool ensureIsType<bool>(const QJsonValue &value, const Requirement, const QString &what);
template <> int ensureIsType<int>(const QJsonValue &value, const Requirement, const QString &what);
template <> QJsonObject ensureIsType<QJsonObject>(const QJsonValue &value, const Requirement, const QString &what);
template <> QJsonArray ensureIsType<QJsonArray>(const QJsonValue &value, const Requirement, const QString &what);
template <> QJsonValue ensureIsType<QJsonValue>(const QJsonValue &value, const Requirement, const QString &what);
template <> QByteArray ensureIsType<QByteArray>(const QJsonValue &value, const Requirement, const QString &what);
template <> QDateTime ensureIsType<QDateTime>(const QJsonValue &value, const Requirement, const QString &what);
template <> QVariant ensureIsType<QVariant>(const QJsonValue &value, const Requirement, const QString &what);
template <> QString ensureIsType<QString>(const QJsonValue &value, const Requirement, const QString &what);
template <> QUuid ensureIsType<QUuid>(const QJsonValue &value, const Requirement, const QString &what);
template <> QDir ensureIsType<QDir>(const QJsonValue &value, const Requirement, const QString &what);
template <> QUrl ensureIsType<QUrl>(const QJsonValue &value, const Requirement, const QString &what);

// the following functions are higher level functions, that make use of the above functions for
// type conversion
template <typename T>
T ensureIsType(const QJsonValue &value, const T default_, const QString &what = "Value")
{
	if (value.isUndefined())
	{
		return default_;
	}
	return ensureIsType<T>(value, Required, what);
}
template <typename T>
T ensureIsType(const QJsonObject &parent, const QString &key,
			   const Requirement requirement = Required,
			   const QString &what = "__placeholder__")
{
	const QString localWhat = QString(what).replace("__placeholder__", '\'' + key + '\'');
	if (!parent.contains(key))
	{
		throw JsonException(localWhat + "s parent does not contain " + localWhat);
	}
	return ensureIsType<T>(parent.value(key), requirement, localWhat);
}
template <typename T>
T ensureIsType(const QJsonObject &parent, const QString &key, const T default_,
			   const QString &what = "__placeholder__")
{
	const QString localWhat = QString(what).replace("__placeholder__", '\'' + key + '\'');
	if (!parent.contains(key))
	{
		return default_;
	}
	return ensureIsType<T>(parent.value(key), default_, localWhat);
}

template <typename T>
QVector<T> ensureIsArrayOf(const QJsonDocument &doc)
{
	const QJsonArray array = ensureArray(doc);
	QVector<T> out;
	for (const QJsonValue val : array)
	{
		out.append(ensureIsType<T>(val, Required, "Document"));
	}
	return out;
}
template <typename T>
QVector<T> ensureIsArrayOf(const QJsonValue &value, const Requirement = Required,
						 const QString &what = "Value")
{
	const QJsonArray array = ensureIsType<QJsonArray>(value, Required, what);
	QVector<T> out;
	for (const QJsonValue val : array)
	{
		out.append(ensureIsType<T>(val, Required, what));
	}
	return out;
}
template <typename T>
QVector<T> ensureIsArrayOf(const QJsonValue &value, const QVector<T> default_,
						 const QString &what = "Value")
{
	if (value.isUndefined())
	{
		return default_;
	}
	return ensureIsArrayOf<T>(value, Required, what);
}
template <typename T>
QVector<T> ensureIsArrayOf(const QJsonObject &parent, const QString &key,
						 const Requirement requirement = Required,
						 const QString &what = "__placeholder__")
{
	const QString localWhat = QString(what).replace("__placeholder__", '\'' + key + '\'');
	if (!parent.contains(key))
	{
		throw JsonException(localWhat + "s parent does not contain " + localWhat);
	}
	return ensureIsArrayOf<T>(parent.value(key), requirement, localWhat);
}
template <typename T>
QVector<T> ensureIsArrayOf(const QJsonObject &parent, const QString &key,
						 const QVector<T> &default_, const QString &what = "__placeholder__")
{
	const QString localWhat = QString(what).replace("__placeholder__", '\'' + key + '\'');
	if (!parent.contains(key))
	{
		return default_;
	}
	return ensureIsArrayOf<T>(parent.value(key), default_, localWhat);
}

// this macro part could be replaced by variadic functions that just pass on their arguments, but that wouldn't work well with IDE helpers
#define JSON_HELPERFUNCTIONS(NAME, TYPE) \
	inline TYPE ensure##NAME(const QJsonValue &value, const Requirement requirement = Required, const QString &what = "Value") \
{ return ensureIsType<TYPE>(value, requirement, what); } \
	inline TYPE ensure##NAME(const QJsonValue &value, const TYPE default_, const QString &what = "Value") \
{ return ensureIsType<TYPE>(value, default_, what); } \
	inline TYPE ensure##NAME(const QJsonObject &parent, const QString &key, const Requirement requirement = Required, const QString &what = "__placeholder__") \
{ return ensureIsType<TYPE>(parent, key, requirement, what); } \
	inline TYPE ensure##NAME(const QJsonObject &parent, const QString &key, const TYPE default_, const QString &what = "__placeholder") \
{ return ensureIsType<TYPE>(parent, key, default_, what); }

JSON_HELPERFUNCTIONS(Array, QJsonArray)
JSON_HELPERFUNCTIONS(Object, QJsonObject)
JSON_HELPERFUNCTIONS(String, QString)
JSON_HELPERFUNCTIONS(Boolean, bool)
JSON_HELPERFUNCTIONS(Double, double)
JSON_HELPERFUNCTIONS(Integer, int)
JSON_HELPERFUNCTIONS(DateTime, QDateTime)
JSON_HELPERFUNCTIONS(Url, QUrl)
JSON_HELPERFUNCTIONS(ByteArray, QByteArray)
JSON_HELPERFUNCTIONS(Dir, QDir)
JSON_HELPERFUNCTIONS(Uuid, QUuid)
JSON_HELPERFUNCTIONS(Variant, QVariant)

#undef JSON_HELPERFUNCTIONS

}
