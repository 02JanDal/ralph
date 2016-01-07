#pragma once

#include "Task.h"
#include "Exception.h"

QT_BEGIN_NAMESPACE
class QDir;
QT_END_NAMESPACE

namespace Ralph {
namespace ClientLib {
namespace Archive {

DECLARE_EXCEPTION(Archive);

Task<void>::Ptr extract(const QString &filename, const QDir &destination);

}
}
}
