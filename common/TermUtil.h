#pragma once

#include <QString>

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

QString wrap(const QString &text, const int maxWidth, const int indent = 0);
QString table(const QVector<QVector<QString>> &rows, const QVector<int> &columnSizeRatios, const int maxWidth, const int indent = 0);

QString style(const Style style, const QString &in = QString());
QString fg(const Color color, const QString &in = QString());
QString bg(const Color color, const QString &in = QString());
QString reset();
bool isTty();

int currentWidth();

}
}
}
