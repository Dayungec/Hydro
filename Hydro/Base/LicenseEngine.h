#ifndef LICENSEENGINE_H
#define LICENSEENGINE_H
#include "base_globle.h"

namespace SGIS{

class SGIS_EXPORT LicenseEngine
{
public:
    LicenseEngine();
    virtual~LicenseEngine();
    bool StartUsing(string assets_path="");
    static string&AssetsPath();
};

}

#endif // LICENSEENGINE_H
