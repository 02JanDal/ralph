#pragma once

#include "package/Package.h"

namespace Ralph {
namespace ClientLib {

class Project : public Package
{
	Q_OBJECT
public:
	explicit Project(QObject *parent = nullptr);
};

}
}
