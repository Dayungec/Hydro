#ifndef POLYLINE_H
#define POLYLINE_H
#include "geometry.h"
#include "Point.h"
#include "points.h"

namespace SGIS{

class SGIS_EXPORT Polyline:
       public Geometry,
       public Geometry3D
{
public:
    Polyline();
    virtual~Polyline();
//IGeometry:
    AutoPtr<Envelope>GetExtent();
    GeometryType GetType();
    virtual AutoPtr<Geometry>Clone();
    bool Equals(Geometry*other);
    bool Intersects(Geometry*other);
    bool Disjoint(Geometry*other);
    bool Touches(Geometry*other);
    bool Crosses(Geometry*other);
    bool Within(Geometry*other);
    bool Contains(Geometry*other);
    bool Overlaps(Geometry*other);
    void AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY);
    bool Project(CoordinateTransformation*pTrans);
    void Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr=1);
    void Move(DOUBLE difx,DOUBLE dify,DOUBLE difz=0);
    AutoPtr<Point>GetCenter();
    AutoPtr<Point>GetFromPoint();
    AutoPtr<Point>GetToPoint();
//BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    virtual string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
//Geometry3D:
    AutoPtr<Envelope>GetExtent3D();
    void Rotate(DOUBLE RotateAngle,Vector*axis);
    bool Read3D(BufferReader*br);
    void Write3D(BufferWriter*bw);
    bool ReadFromJson3D(JsonReader*jr,JsonNode node);
    string ToJson3D();
//Polyline:
    int GetSize();
    AutoPtr<Path>operator[](int nIndex);
    AutoPtr<Path>GetItem(int nIndex);
    void SetItem(int nIndex,AutoPtr<Path>path);
    void Add(AutoPtr<Path>item);
    void Insert(int nIndex,AutoPtr<Path>item);
    void Clear();
    void Remove(int nIndex);
    virtual double GetLength();
    double GetDiameter();
    virtual bool CanCaptureBorder(Point*ppt,double toler);
    virtual AutoPtr<Point> CaptureBorder(Point*ppt,double toler);
protected:
    bool IsInPath(Points*outerPoints,Points*innerPoints);
    bool IsPathIn(Polyline*polyline,Points*innerPoints);
protected:
    vector<AutoPtr<Path>>parts;
};


class SGIS_EXPORT CurvePolyline:
        public Polyline
{
public:
    CurvePolyline();
    virtual~CurvePolyline();
    AutoPtr<Geometry>Clone();
    double GetLength();
    AutoPtr<Polyline>ConvertToPolyline(double toler);
    AutoPtr<Polyline>ConvertToDefaultPolyline(bool hQuality=false);
    bool CanCaptureBorder(Point*ppt,double toler);
    AutoPtr<Point> CaptureBorder(Point*ppt,double toler);
    string GetTypeName();
};

}
#endif // POLYLINE_H
