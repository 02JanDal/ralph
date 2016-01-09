#pragma once

#include <QString>

template <typename T>
inline QString operator%(const QString &string, T &&value) { return string.arg(value); }
inline QString operator%(const char *string, QString &&value) { return QString(string).arg(value); }
