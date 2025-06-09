#ifndef POINT_H
#define POINT_H
#include "geometry.h"
#include "Base/linearmath.h"

namespace SGIS{

enum AxisType{
    atX=0,
    atY=1,
    atZ=2
};

class Point;
class Segment;
class Points;

class RobustDeterminant{
public:
    RobustDeterminant();
    virtual~RobustDeterminant();
    static int SignOfDet2x2(double x1,double y1,double x2,double y2);
    static int OrientationIndex(double lnx1,double lny1,double lnx2,double lny2,double x,double y);
    static int ReverseOrientationIndex(double lnx1,double lny1,double lnx2,double lny2,double x,double y);
    static int OrientationIndex(const Segment&seg,const Point&point);
    static int ReverseOrientationIndex(const Segment&seg,const Point&point);
    static int OrientationIndex(Segment*seg,Point*point);
    static int ReverseOrientationIndex(Segment*seg,Point*point);
};

class SGIS_EXPORT Vector{
public:
    Vector(){};
    virtual~Vector(){};
    virtual void GetCoord(double*x,double*y,double*z)=0;
    virtual void PutCoord(double x,double y,double z)=0;
    virtual double GetModule()=0;
    virtual double Dot(Vector*other)=0;
    virtual AutoPtr<Vector>Cross(Vector*other)=0;
    virtual void ScaleByAnchor(Point*anchor,double ratio)=0;
    virtual AutoPtr<Vector>Multiply(Matrix4x4*matrix)=0;
};

class SGIS_EXPORT Point:
        public Geometry,
        public Geometry3D,
        public Vector
{
public:
    Point();
    Point(double x,double y,double z=0);
    Point(const Point&other);
    virtual~Point();
//IGeometry:
    AutoPtr<Envelope>GetExtent();
    GeometryType GetType();
    bool Equals(Geometry*other);
    bool Intersects(Geometry*other);
    bool Disjoint(Geometry*other);
    bool Touches(Geometry*other);
    bool Crosses(Geometry*other);
    bool Within(Geometry*other);
    bool Contains(Geometry*other);
    bool Overlaps(Geometry*other);
    AutoPtr<Geometry>Clone();
    void AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY);
    bool Project(CoordinateTransformation*pTrans);
    void Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr=1);
    void Move(DOUBLE difx,DOUBLE dify,DOUBLE difz=0);
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
//Point:
    double&X();
    double&Y();
    double&Z();
    Point operator + (const Point&other);
    Point operator - (const Point&other);
    Point operator * (const Point&other);
    Point operator / (const Point&other);
    Point&operator += (const Point&other);
    Point&operator -= (const Point&other);
    Point&operator *= (const Point&other);
    Point&operator /= (const Point&other);
    void GetCoord(double*x,double*y);
    void PutCoord(double x,double y);
    double DistanceToPoint(const Point&other);
    double CaptureSegment(Segment*seg,Point&capturedPoint);
    double DistanceToSegment(Segment*seg);
    bool IntersectsSegment(Segment*seg);
    void RotatePointByAnchor(Point*anchor,AxisType aType,DOUBLE planeangle,DOUBLE axisangle);
    void RotatePoint(Vector*axis,double angle);
    void CopyFromOther(Point*other);
    double DistanceTo(Geometry*geo);
//Vector:
    void GetCoord(double*x,double*y,double*z);
    void PutCoord(double x,double y,double z);
    double GetModule();
    double Dot(Vector*other);
    AutoPtr<Vector>Cross(Vector*other);
    void ScaleByAnchor(Point*anchor,double ratio);
    AutoPtr<Vector>Multiply(Matrix4x4*matrix);
protected:
    double distanceToPath(Points*path);
    double distanceToRing(Points*ring);
protected:
    double x;
    double y;
    double z;
};

}

#endif // POINT_H
