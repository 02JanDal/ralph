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

#include <QString>

#include <ostream>

inline std::ostream &operator<<(std::ostream &str, const QString &string)
{
	return str << string.toLocal8Bit().constData();
}

namespace Ralph {
namespace Common {
namespace Term {

enum Color
{
	Grey, Red, Green, Yellow, Blue, Magenta, Cyan, White
};
enum Style
{
	Bold, Dark, Underline, Blink, Reverse, Concealed
};
enum MoveType
{
	Up,
	Down,
	Left,
	Right,
	LineDown,
	LineUp
};

QString wrap(const QString &text, const int maxWidth, const int indent = 0);
QString table(const QVector<QVector<QString>> &rows, const QVector<int> &columnSizeRatios, const int maxWidth, const int indent = 0);

QString move(const MoveType type, const int n = 1);
QString save();
QString restore();
QString style(const Style style, const QString &in = QString());
QString fg(const Color color, const QString &in = QString());
QString bg(const Color color, const QString &in = QString());
QString reset();
bool isTty();

int currentWidth();

QString readPassword();

}
}
}
