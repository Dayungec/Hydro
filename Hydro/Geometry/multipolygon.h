#ifndef MULTIPOLYGON_H
#define MULTIPOLYGON_H
#include "geometry.h"
#include "Point.h"
#include "points.h"
#include "polygon.h"

namespace SGIS{

class SGIS_EXPORT MultiPolygon:
        public Geometry,
        public Geometry3D
{
public:
    MultiPolygon();
    virtual~MultiPolygon();
//IGeometry:
    AutoPtr<Envelope>GetExtent();
    GeometryType GetType();
    AutoPtr<Geometry>Clone();
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
    double GetDiameter();
    AutoPtr<Point>GetCenter();
//BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
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
    AutoPtr<Polygon>operator[](int nIndex);
    AutoPtr<Polygon>GetItem(int nIndex);
    void SetItem(int nIndex,AutoPtr<Polygon>poly);
    void Add(AutoPtr<Polygon>item);
    void Insert(int nIndex,AutoPtr<Polygon>item);
    void Clear();
    void Remove(int nIndex);
    bool IsPointIn(Point*point);
    double GetPerimeter();
    double GetArea();
    bool CanCaptureBorder(Point*ppt,double toler);
    AutoPtr<Point> CaptureBorder(Point*ppt,double toler);
    AutoPtr<GeometryCollection>Split(Path*path);
    AutoPtr<Geometry>Rebuild();
protected:
    vector<AutoPtr<Polygon>>parts;
};
}
#endif // MULTIPOLYGON_H
