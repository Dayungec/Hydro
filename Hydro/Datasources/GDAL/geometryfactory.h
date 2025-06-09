#ifndef GEOMETRYFACTORY_H
#define GEOMETRYFACTORY_H
#include "../GeometryBase/geometry.h"
#include "../GeometryBase/Point.h"
#include "../GeometryBase/segment.h"
#include "../GeometryBase/envelope.h"
#include "../GeometryBase/ellipse.h"
#include "../GeometryBase/points.h"
#include "../GeometryBase/polyline.h"
#include "../GeometryBase/polygon.h"
#include "../GeometryBase/multipolygon.h"
#include "../GeometryBase/geometrycollection.h"

#include "gdal_priv.h"
#include "ogrsf_frmts.h"

namespace SGIS{

class GDALGeometryFactory
{
public:
    static OGRGeometry* ConvertGeometry(Geometry*pDef);
protected:
    static OGRGeometry* ConvertPoint(DOUBLE X,DOUBLE Y,DOUBLE Z=0);
    static OGRGeometry* ConvertPolyline(DOUBLE*xy,int*counts,LONG partscount);
    static OGRGeometry* ConvertPolygon(DOUBLE*xy,int*counts,LONG partscount);
    static OGRGeometry* ConvertMultiPoint(DOUBLE*xy,LONG pointscount);
protected:
    static OGRPoint*ConvertPoint(Point*pDef);
    static OGRMultiPoint*ConvertPoints(Points*pDef);
    static OGRMultiLineString*ConvertPolyline(Polyline*pts);
    static OGRPolygon*ConvertPolygon(Polygon*pDef);
    static OGRMultiPolygon*ConvertMultiPolygon(MultiPolygon*pDef);
    static OGRGeometryCollection*ConvertGeometryCollection(GeometryCollection*pDef);
};

class GeometryFactory
{
public:
    static AutoPtr<Geometry> CreateGeometry(OGRGeometry*pGeo);
protected:
    static AutoPtr<Geometry> CreatePoint(DOUBLE X,DOUBLE Y,DOUBLE Z=0);
    static AutoPtr<Geometry> CreatePolyline(DOUBLE*xy,int*counts,LONG partscount);
    static AutoPtr<Geometry> CreatePolygon(DOUBLE*xy,int*counts,LONG partscount);
    static AutoPtr<Geometry> CreateMultiPoint(DOUBLE*xy,LONG pointscount);
protected:
    static AutoPtr<Point>CreatePoint(OGRPoint*def);
    static AutoPtr<Points>CreatePoints(OGRMultiPoint*def);
    static AutoPtr<Polyline>CreatePolyline(OGRLineString*def);
    static AutoPtr<Polyline>CreatePolyline(OGRMultiLineString*def);
    static AutoPtr<Polygon>CreatePolygon(OGRPolygon*def);
    static AutoPtr<MultiPolygon>CreateMultiPolygon(OGRMultiPolygon*def);
    static AutoPtr<GeometryCollection>CreateCollection(OGRGeometryCollection*def);
};


}
#endif // GEOMETRYFACTORY_H
