#include "ActionContext.h"

namespace Ralph {
namespace ClientLib {

BaseContextItem::~BaseContextItem() {}

InstallContextItem::InstallContextItem(const QDir &target, const QDir &build)
	: targetDir(target), buildDir(build) {}

InstallContextItem::~InstallContextItem() {}

}
}

