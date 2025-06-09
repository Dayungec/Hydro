#ifndef RASTERTARGET_H
#define RASTERTARGET_H
#include "samodel.h"
#include "Geometry/Point.h"
#include "Geometry/geometry2d.h"
#include "Datasources/filerasterdataset.h"
#include "Datasources/memrasterdataset.h"

namespace SGIS{
class SGIS_EXPORT RasterTarget
{
public:
    RasterTarget();
    virtual~RasterTarget();
    virtual AutoPtr<RasterDataset>CreateRasterDataset(SAEnvironment*envi,RasterDataType dType,int bandNumber,double nodata)=0;
};

class SGIS_EXPORT FileRasterTarget :public RasterTarget{
public:
    FileRasterTarget();
    virtual~FileRasterTarget();
    string&PathName();
    RasterCreateFileType&FileType();
    AutoPtr<RasterDataset>CreateRasterDataset(SAEnvironment*envi,RasterDataType dType,int bandNumber,double nodata);
protected:
    string pathName;
    RasterCreateFileType fileType;
};

class SGIS_EXPORT MemRasterTarget :public RasterTarget{
public:
    MemRasterTarget();
    virtual~MemRasterTarget();
    string&Name();
    AutoPtr<RasterDataset>CreateRasterDataset(SAEnvironment*envi,RasterDataType dType,int bandNumber,double nodata);
protected:
    string name;
};

};
#endif // RASTERTARGET_H
