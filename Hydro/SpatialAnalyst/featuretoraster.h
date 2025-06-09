#ifndef FEATURETORASTER_H
#define FEATURETORASTER_H
#include "samodel.h"
#include "Geometry/Point.h"
#include "Geometry/geometry2d.h"
#include "rastertarget.h"

namespace SGIS{
class SGIS_EXPORT VectorToGrid{
public:
    VectorToGrid();
    virtual~VectorToGrid();
    RasterDesp GetRasterDesp();
    void SetRasterDesp(RasterDesp desp);
    bool Convert(Geometry*pRefGeo,Envelope*CurrentBoundry,BYTE*data,bool reset=true);
protected:
    void ConvertPoint(Point*pt,BYTE*data);
    void ConvertPoints(Points*pts,BYTE*data);
    void ConvertSegment(DOUBLE X1,DOUBLE Y1,DOUBLE X2,DOUBLE Y2,BYTE*data);
    void ConvertSegment(Segment*seg,BYTE*data);
    void ConvertPolyline(Points*ply,BYTE*data);
    void ConvertPolyline(Polyline*ply,BYTE*data);
    void ConvertRing(Points*pnts,BYTE*data);
    void ConvertPolygon(Polygon*poly,BYTE*data);
    void ConvertMultiPolygon(MultiPolygon*poly,BYTE*data);
    void ConvertGeometryCollection(GeometryCollection*poly,BYTE*data);
    void ConvertGeometry(Geometry*geo,BYTE*data);
protected:
    long RegionNumber(int regionx,int regiony);
    int RegionX(double X);
    int RegionY(double Y);
    int RealRegionX(double X);
    int RealRegionY(double Y);
protected:
    RasterDesp rasterDesp;
    Rect2D<double>FullExtent;
    Rect2D<int>ResultBoundry;
    DOUBLE DifFuzzy;
    BYTE* ptArray=nullptr;
    LONG formerSize=0;
};

class SGIS_EXPORT FeatureToRaster :public SAModel
{
public:
    FeatureToRaster();
    virtual~FeatureToRaster();
    FLOAT&OutputNoData();
    AutoPtr<RasterBand>Execute(FeatureClass*pfc,string NumericField,RasterTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    AutoPtr<RasterBand>ConvertMask(FeatureClass*pfc,Semaphora*signal=nullptr);
    AutoPtr<RasterBand>ConvertGeometry(Geometry*geo,SpatialReference*psp,RasterTarget*target);
    bool ConvertGeometry(Geometry*geo,SpatialReference*psp,RasterBand*band);
protected:
    FLOAT nodata;
    Rect2D<double>FullExtent;
};

}
#endif // FEATURETORASTER_H
