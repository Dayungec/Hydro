#ifndef GEOMETRY_H
#define GEOMETRY_H
#include "Base/base_globle.h"
#include "Base/autoptr.h"
#include "Base/jsonio.h"
#include "Base/classfactory.h"
#include  "Base/bufferio.h"

#define VertexTolerance 0.000001f

namespace SGIS{

enum GeometryType{
    gtPoint,
    gtPoints,
    gtSegment,
    gtEnvelope,
    gtEllipse,
    gtPolyline,
    gtPolygon,
    gtMultiPolygon,
    gtCollection
};


class Point;
class Vector;
class Envelope;
class CoordinateTransformation;
class Polyline;
class CurvePolyline;
class Polygon;
class CurvePolygon;

class SGIS_EXPORT Geometry:
     public Object,
     public BufferWritable,
     public JsonWritable
{
public:
    Geometry();
    virtual~Geometry();
    virtual AutoPtr<Envelope>GetExtent()=0;
    virtual GeometryType GetType()=0;
    virtual AutoPtr<Geometry>Clone()=0;
    virtual bool Project(CoordinateTransformation*pTrans)=0;
    virtual void AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY)=0;
    virtual void Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr=1)=0;
    virtual void Move(DOUBLE difx,DOUBLE dify,DOUBLE difz=0)=0;
    virtual bool Equals(Geometry*other)=0;
    virtual bool Intersects(Geometry*other)=0;
    virtual bool Disjoint(Geometry*other)=0;
    virtual bool Touches(Geometry*other)=0;
    virtual bool Crosses(Geometry*other)=0;
    virtual bool Within(Geometry*other)=0;
    virtual bool Contains(Geometry*other)=0;
    virtual bool Overlaps(Geometry*other)=0;
protected:
    static AutoPtr<Polyline>ConvertToPolyline(Polyline*polyline);
    static AutoPtr<Polygon> ConvertToPolygon(Polygon*polygon);
};

class SGIS_EXPORT Geometry3D{
public:
    Geometry3D();
    virtual~Geometry3D();
    virtual AutoPtr<Envelope>GetExtent3D()=0;
    virtual void Rotate(DOUBLE RotateAngle,Vector*axis)=0;
    virtual bool Read3D(BufferReader*br)=0;
    virtual void Write3D(BufferWriter*bw)=0;
    virtual bool ReadFromJson3D(JsonReader*jr,JsonNode node)=0;
    virtual string ToJson3D()=0;
protected:
    void innerRotate(double sina,double cosa,DOUBLE x,double y,DOUBLE z,DOUBLE&X,DOUBLE&Y,DOUBLE&Z);
};

}

#endif // GEOMETRY_H
