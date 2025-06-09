#ifndef GEOMETRYOPERATOR_H
#define GEOMETRYOPERATOR_H
#include "topobuilder.h"
#include "geometry.h"
#include "Point.h"
#include "segment.h"
#include "envelope.h"
#include "ellipse.h"
#include "points.h"
#include "polyline.h"
#include "polygon.h"
#include "multipolygon.h"
#include "geometrycollection.h"
namespace SGIS{

struct EnvelopeCorner {
    double frompos;
    double topos;
    EnvelopeCorner() {
        frompos = topos = -1;
    };
    EnvelopeCorner(double frompos,double topos) {
        this->frompos = frompos;
        this->topos = topos;
    };
    EnvelopeCorner(const EnvelopeCorner&other){
        frompos = other.frompos;
        topos = other.topos;
    };
    EnvelopeCorner& operator =(const EnvelopeCorner& other)
    {
        frompos = other.frompos;
        topos = other.topos;
        return *this;
    };
};

class EnvelopeCorners {
public:
    EnvelopeCorners(double xmin, double ymin, double xmax, double ymax);
    virtual~EnvelopeCorners();
    void BeginCorners();
    void EndCorners();
    int LeaveEnvelope(double bv);
    int EnterEnvelope(double bv);
    TopoPoint*GetPoint(int nIndex);
    void SetPoint(int nIndex,TopoPoint*pt);
    void AddCorners(TopoLines*tLns);
    int GetPointCount();
protected:
    void AddCorner(double fromv, double tov);
protected:
    OrderArray<double>pArray;
    vector<TopoPoint*>topopts;
    vector<EnvelopeCorner>corners;
    double formperPos;
    double firstPos;
    bool firstEnter;
    vector<int>states;
    double xmin;
    double ymin;
    double xmax;
    double ymax;
};

class SGIS_EXPORT EnvelopeClipper {
public:
    EnvelopeClipper(double xmin,double ymin,double xmax,double ymax);
    virtual~EnvelopeClipper();

    AutoPtr<Point>ClipPoint(Point*ppt);
    AutoPtr<Points>ClipPoints(Points*ppts);
    AutoPtr<Envelope>ClipEnvelope(Envelope*env);
    AutoPtr<Geometry>ClipEllipse(Ellipse*elli);
    AutoPtr<Segment> ClipSegment(Segment*seg);
    AutoPtr<Polyline>ClipPolyline(Polyline*pln);
    AutoPtr<Geometry>ClipPolygon(Polygon*poly);
    AutoPtr<Geometry>ClipMultiPolygon(MultiPolygon*poly);
    AutoPtr<Geometry>ClipGeometryCollection(GeometryCollection*geoCol);
    AutoPtr<Geometry>ClipGeometry(Geometry*geo);
protected:
    int getNbPos(double x, double y);
    double GetBorderValue(double x, double y);
    int IntersectBorder(double x1, double y1, double x2, double y2, int borderCode, double&ix1, double&iy1, double&ix2, double&iy2);
    int IntersectBorder(double x1, double y1, double x2, double y2, double&ix1, double&iy1, double&ix2, double&iy2);
    vector<AutoPtr<Path>>ClipPath(Path*pPath);
    vector<AutoPtr<Ring>>ClipRing(Ring*pPath,bool clockwise);
protected:
    DOUBLE xmin;
    DOUBLE ymin;
    DOUBLE xmax;
    DOUBLE ymax;
    int neighborCodes[9];
};

class SGIS_EXPORT GeometryOperator
{
public:
    GeometryOperator(AutoPtr<Geometry>geo=nullptr);
    virtual~GeometryOperator();
    AutoPtr<Geometry>GetGeometry();
    void SetGeometry(AutoPtr<Geometry>geo);
    double Distance(Geometry*other);
    AutoPtr<Geometry>Buffer(DOUBLE dist,LONG nQuadSegs);
    AutoPtr<Geometry>Intersection(Geometry*other);
    AutoPtr<Geometry>Union(Geometry*other);
    AutoPtr<Geometry>Difference(Geometry*other);
    AutoPtr<Geometry>SymmetricDifference(Geometry*other);
protected:
    AutoPtr<Geometry>pGeo;
};
}
#endif // GEOMETRYOPERATOR_H
