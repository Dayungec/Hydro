#ifndef SAENVIRONMENT_H
#define SAENVIRONMENT_H
#include "Base/autoptr.h"
#include "Base/semaphora.h"
#include "Geometry/Point.h"
#include "Geometry/spatialreference.h"
#include "Datasources/dataset.h"

namespace SGIS{

class SGIS_EXPORT SAEnvironment
{
public:
    SAEnvironment();
    SAEnvironment(RasterDesp desp,SpatialReference*psp);
    virtual~SAEnvironment();
    double&Left();
    double&Top();
    double&XCellSize();
    double&YCellSize();
    int&Cols();
    int&Rows();
    RasterDesp GetRasterDesp();
    AutoPtr<SpatialReference>GetSpatialReference();
    void SetSpatialReference(AutoPtr<SpatialReference>psp);
    AutoPtr<Point>GetLeftTop();
    double GetRight();
    double GetBottom();
    AutoPtr<SAEnvironment>Clone(bool withMask=true);
    AutoPtr<RasterBand>GetMaskBand();
    bool CreateMaskBand(CallBack*callBack,Semaphora*signal=nullptr);
    AutoPtr<Envelope>GetExtent();
    void CreateFromRasterBand(RasterBand*pBand,bool autoMask=false);
    void CreateFromFeatureClass(FeatureClass*pfc,double cellSize);
    void CreateFromGeometry(Geometry*geo,SpatialReference*psp,double cellSize,bool autoMask=true);
    void CreateMaskBandByGeometry(Geometry*geo,SpatialReference*psp);
    bool CreateMaskBandByRasterBand(RasterBand*pBand,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    bool CreateMaskBandByFeatureClass(FeatureClass*pfc,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
protected:
    int cols;
    int rows;
    double xCellSize;
    double yCellSize;
    double left;
    double top;
    AutoPtr<SpatialReference>psp;
    AutoPtr<RasterBand>maskBand;
};

class SGIS_EXPORT SAModel {
public:
   SAModel();
   virtual~SAModel();
   AutoPtr<SAEnvironment>GetEnvironment();
   void SetEnvironment(AutoPtr<SAEnvironment>envi);
protected:
   bool CheckValid();
   BYTE*GetMaskData(CallBack*callBack,Semaphora*siganl,bool&bSuc);
   bool DetectExitSignal(CallBack*callBack,Semaphora*signal);
protected:
   AutoPtr<SAEnvironment>pEnvi;
    DateTime detectTime;
};

}
#endif // SAENVIRONMENT_H
