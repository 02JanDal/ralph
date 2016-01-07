#include "TermUtil.h"

#include <QStringList>
#include <QRegularExpression>
#include <QVector>

#include "Functional.h"

#ifdef Q_OS_UNIX
# include <sys/ioctl.h>
# include <cstdio>
# include <unistd.h>
#else
# include <io.h>
# include <windows.h>
# warning Windows support has not yet been implemented. No colors will be shown.
#endif

namespace Ralph {
namespace Common {
using Functional::collection;
namespace Term {
namespace detail {
#ifdef Q_OS_UNIX
static QString getStyleCode(const Style style)
{
	switch (style) {
	case Term::Bold: return "\033[1m";
	case Term::Dark: return "\033[2m";
	case Term::Underline: return "\033[4m";
	case Term::Blink: return "\033[5m";
	case Term::Reverse: return "\033[7m";
	case Term::Concealed: return "\033[8m";
	}
}
static QString getStyleEndCode(const Style style)
{
	switch (style) {
	case Term::Bold: return "\033[22m";
	case Term::Dark: return "\033[22m";
	case Term::Underline: return "\033[24m";
	case Term::Blink: return "\033[25m";
	case Term::Reverse: return "\033[27m";
	case Term::Concealed: return "\033[38m";
	}
}
static QString getFGColorCode(const Color color)
{
	switch (color) {
	case Term::Grey: return "\033[30m";
	case Term::Red: return "\033[31m";
	case Term::Green: return "\033[32m";
	case Term::Yellow: return "\033[33m";
	case Term::Blue: return "\033[34m";
	case Term::Magenta: return "\033[35m";
	case Term::Cyan: return "\033[36m";
	case Term::White: return "\033[37m";
	}
}
static QString getBGColorCode(const Color color)
{
	switch (color) {
	case Term::Grey: return "\033[40m";
	case Term::Red: return "\033[41m";
	case Term::Green: return "\033[42m";
	case Term::Yellow: return "\033[43m";
	case Term::Blue: return "\033[44m";
	case Term::Magenta: return "\033[45m";
	case Term::Cyan: return "\033[46m";
	case Term::White: return "\033[47m";
	}
}
#endif
}

QString style(const Style style, const QString &in)
{
	if (!isTty()) {
		return in;
	}
#ifdef Q_OS_WIN
	return in;
#else
	if (in.isEmpty()) {
		return detail::getStyleCode(style);
	} else {
		return detail::getStyleCode(style) + in + detail::getStyleEndCode(style);
	}
#endif
}
QString fg(const Color color, const QString &in)
{
	if (!isTty()) {
		return in;
	}
#ifdef Q_OS_WIN
	return in;
#else
	if (in.isEmpty()) {
		return detail::getFGColorCode(color);
	} else {
		return detail::getFGColorCode(color) + in + "\033[39m";
	}
#endif
}
QString bg(const Color color, const QString &in)
{
	if (!isTty()) {
		return in;
	}
#ifdef Q_OS_WIN
	return in;
#else
	if (in.isEmpty()) {
		return detail::getBGColorCode(color);
	} else {
		return detail::getBGColorCode(color) + in + "\033[39m";
	}
#endif
}
QString reset()
{
	if (!isTty())
	{
		return QString();
	}
#ifdef Q_OS_WIN
	return "";
#else
	return "\033[00m";
#endif
}

bool isTty()
{
#ifdef Q_OS_WIN
	return ::_isatty(_fileno(stdout));
#else
	return ::isatty(fileno(stdout));
#endif
}

int currentWidth()
{
#ifdef Q_OS_UNIX
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	return w.ws_col;
#else
	return 120;
#endif
}

QString wrap(const QString &text, const int maxWidth, const int indent)
{
	static QRegularExpression breakExpression("[^a-zA-Z]");

	Q_ASSERT(maxWidth > indent);
	const int realWidth = maxWidth - indent;

	// simplest case
	if (text.size() <= realWidth) {
		return text;
	}

	QStringList rows;
	int index = 0;
	while (index < text.size()) {
		QString row = text.mid(index);

		int breakIndex = row.lastIndexOf('\n', realWidth);
		bool breakAtSpace = true;
		if (breakIndex == -1) {
			breakIndex = row.lastIndexOf(' ', realWidth);
		}
		if (breakIndex == -1) {
			breakIndex = row.lastIndexOf(breakExpression, realWidth);
			if (breakIndex >= 0) {
				breakAtSpace = row.at(breakIndex).isSpace();
			}
		}
		if (breakIndex <= 0) {
			breakIndex = realWidth;
		}
		row = row.mid(0, breakIndex);
		rows.append(row);

		index += breakAtSpace ? row.size() + 1 : row.size();
	}

	return rows.join('\n' + QString(indent, ' '));
}

static QVector<QVector<QString>> partitionRow(const QVector<QString> &columns, const QVector<int> &columnSizes)
{
	Q_ASSERT(columns.size() == columnSizes.size());

	QVector<QVector<QString>> wrapped;
	for (int i = 0; i < columns.size(); ++i) {
		// perform wrapping and split at new lines
		wrapped.append(wrap(columns.at(i), columnSizes.at(i)).split('\n').toVector());
	}

	// how many rows to we have at most in a single column?
	const int numRows = Functional::collection(wrapped).mapSize().max();

	// wrapped is a list of columns, but we need a list of rows
	// this is just a "normal" row-major => column-major conversion
	QVector<QVector<QString>> result;
	result.resize(numRows);
	for (int i = 0; i < numRows; ++i) {
		result[i].resize(wrapped.size());
		for (int j = 0; j < wrapped.size(); ++j) {
			// if one cell does not have any more rows we use an empty string
			result[i][j] = (i >= wrapped[j].size()) ? QString() : wrapped.at(j).at(i);
		}
	}

	return result;
}
QString table(const QVector<QVector<QString> > &rows, const QVector<int> &columnSizeRatios, const int maxWidth, const int indent)
{
	Q_ASSERT(maxWidth > indent);
	const int realWidth = maxWidth - indent;

	QVector<int> columnSizes(columnSizeRatios.size());
	for (const QVector<QString> &row : rows) {
		Q_ASSERT(row.size() == columnSizeRatios.size());
		for (int i = 0; i < row.size(); ++i) {
			columnSizes[i] = std::max(columnSizes.at(i), row.at(i).size());
		}
	}

	const int requiredPadding = columnSizes.size() - 1;
	const int totalRequiredColumnSizes = collection(columnSizes).sum() + requiredPadding;
	if (totalRequiredColumnSizes > realWidth) {
		const int totalRatio = collection(columnSizeRatios).sum();
		columnSizes = collection(columnSizeRatios)
				.map([realWidth, totalRatio, requiredPadding](const int ratio) { return std::min(1, int(std::floor(qreal(ratio) / qreal(totalRatio) * qreal(realWidth-requiredPadding)))); });

		// due to the flooring we might end up with some left over space. give it to the last column
		const int newWidth = collection(columnSizes).sum() + requiredPadding;
		columnSizes.last() += realWidth - newWidth;
	}

	return collection(rows)
			.map([columnSizes, indent](const QVector<QString> &row)
	{
		return collection(partitionRow(row, columnSizes))
				.map([columnSizes](const QVector<QString> &partitionedRow)
		{
			QStringList res;
			for (int i = 0; i < partitionedRow.size(); ++i) {
				const int padding = columnSizes.at(i) - partitionedRow.at(i).size();
				res.append(partitionedRow.at(i) + QString(padding, ' '));
			}
			return res.join(' ');
		})
				.join("\n" + QString(indent, ' '));
	})
			.join("\n" + QString(indent, ' '));
}

}
}
}
