#pragma once

#include <iostream>

#include "FutureWatcher.h"
#include "TermUtil.h"

namespace Ralph {
namespace ClientLib {

template <typename T>
T awaitTerminal(const Future<T> &future)
{
	using namespace Common;
	const int maxWidth = Term::currentWidth() == 0 ? 120 : Term::currentWidth();

	FutureWatcher<T> watcher(future);
	FutureWatcher<T>::connect(&watcher, &FutureWatcher<T>::status, [maxWidth](const QString &str)
	{
		std::cout << Term::wrap(str, maxWidth - 6) << std::endl;
	});
	FutureWatcher<T>::connect(&watcher, &FutureWatcher<T>::progress, [maxWidth](const std::size_t current, const std::size_t total)
	{
		if (Term::isTty()) {
			const int percent = int(std::floor(100 * qreal(current) / qreal(total)));
			std::cout << Term::save() << Term::move(Term::Up) << Term::move(Term::Right, maxWidth - 6) << "[";
			if (percent < 10) {
				std::cout << "  ";
			} else if (percent < 100) {
				std::cout << " ";
			}
			std::cout << percent << "%]" << Term::restore() << std::flush;
		}
	});
	return await(future);
}

}
}
