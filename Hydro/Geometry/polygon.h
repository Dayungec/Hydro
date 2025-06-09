#ifndef POLYGON_H
#define POLYGON_H
#include "geometry.h"
#include "Point.h"
#include "points.h"
namespace SGIS{

class GeometryCollection;

class SGIS_EXPORT Polygon:
        public Geometry,
        public Geometry3D
{
public:
    Polygon();
    virtual~Polygon();
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
    AutoPtr<Ring>operator[](int nIndex);
    AutoPtr<Ring>GetItem(int nIndex);
    void SetItem(int nIndex,AutoPtr<Ring>ring);
    void Add(AutoPtr<Ring>item);
    void Insert(int nIndex,AutoPtr<Ring>item);
    void Clear();
    void Remove(int nIndex);
    bool IsPointIn(Point*point);
    virtual double GetPerimeter();
    virtual double GetArea();
    double GetDiameter();
    virtual bool CanCaptureBorder(Point*ppt,double toler);
    virtual AutoPtr<Point> CaptureBorder(Point*ppt,double toler);
    AutoPtr<Points>GetLabelPoint(Envelope*clipEnv,LONG maxGridNum);
    virtual AutoPtr<GeometryCollection>Split(Path*path);
    AutoPtr<Geometry>Rebuild();
protected:
    vector<AutoPtr<Ring>>parts;
};

class SGIS_EXPORT CurvePolygon:
        public Polygon
{
public:
    CurvePolygon();
    virtual~CurvePolygon();
    AutoPtr<Geometry>Clone();
    double GetPerimeter();
    double GetArea();
    AutoPtr<Polygon>ConvertToPolygon(double toler);
    AutoPtr<Polygon>ConvertToDefaultPolygon(bool hQuality=false);
    bool CanCaptureBorder(Point*ppt,double toler);
    AutoPtr<Point> CaptureBorder(Point*ppt,double toler);
    AutoPtr<GeometryCollection>Split(Path*path);
    string GetTypeName();
};

}
#endif // POLYGON_H
