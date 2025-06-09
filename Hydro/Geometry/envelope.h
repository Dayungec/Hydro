#ifndef ENVELOPE_H
#define ENVELOPE_H
#include "Base/autoptr.h"
#include "geometry.h"
#include "Point.h"

namespace SGIS{
class Points;
class Polygon;

class SGIS_EXPORT Envelope :
        public Geometry,
        public Geometry3D
{
public:
    Envelope();
    Envelope(double left,double top,double right,double bottom,double floor=0,double roof=0);
    Envelope(const Envelope&other);
    virtual~Envelope();
    Envelope&operator=(const Envelope&other);
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
    void ParseInt();
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
//Envelope:
    double&Left();
    double&Top();
    double&Right();
    double&Bottom();
    double&Floor();
    double&Roof();
    double Width();
    double Height();
    AutoPtr<Point>GetCenterPoint();
    void GetCoord(double*left,double*top,double*right,double*bottom);
    void PutCoord(double left,double top,double right,double bottom);
    void Normalize();
    void NormalizeClient();
    bool IntersectsEnvelope(Envelope*env);
    bool IntersectsEnvelopeInClient(Envelope*env);
    bool IsPointIn(Point*point);
    bool IsPointIn(double x,double y);
    bool IsPointInClient(Point*point);
    bool WithinEnvelope(Envelope*env);
    bool WithinEnvelopeInClient(Envelope*env);
    AutoPtr<Envelope>IntersectEnvelope(Envelope*env);
    AutoPtr<Envelope>IntersectEnvelopeInClient(Envelope*env);
    void Inflate(double sizex,double sizey,bool isratio=false);
    void InflateClient(double sizex,double sizey,bool isratio=false);
    void Inflate(double sl,double st,double sr,double sb,bool isratio=false);
    void InflateClient(double sl,double st,double sr,double sb,bool isratio=false);
    AutoPtr<Envelope>Union(Envelope*env);
    AutoPtr<Envelope>UnionClient(Envelope*env);
    void UnionOther(Envelope*env);
    void UnionOtherClient(Envelope*env);
    AutoPtr<Points>ReturnPoints();
    AutoPtr<Polygon>ConvertToPolygon();
    void CopyFromOther(Envelope*other);
    AutoPtr<Geometry>ClipGeometry(Geometry*geo);
protected:
    double left;
    double top;
    double right;
    double bottom;
    double floor;
    double roof;
};


}
#endif // ENVELOPE_H
