#ifndef SEGMENT_H
#define SEGMENT_H
#include "geometry.h"
namespace SGIS{

class Points;
class Polyline;

class SGIS_EXPORT Segment:
        public Geometry,
        public Geometry3D
{
public:
    Segment();
    Segment(double lnx1,double lny1,double lnx2,double lny2,double lnz1=0,double lnz2=0);
    Segment(const Segment&other);
    virtual~Segment();
    Segment&operator=(const Segment&other);
//IGeometry:
    AutoPtr<Envelope>GetExtent();
    GeometryType GetType();
    AutoPtr<Geometry>Clone();
    void CopyFromOther(Segment*other);
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
//Segment:
    double&Lx1();
    double&Ly1();
    double&Lx2();
    double&Ly2();
    void GetCoord(double*lnx1,double*lny1,double*lnx2,double*lny2);
    void PutCoord(double lnx1,double lny1,double lnx2,double lny2);
    AutoPtr<Point>FromPoint();
    AutoPtr<Point>ToPoint();
    double Angle();
    double GetLength();
    double GetLength3D();
    double DistanceToSegment(Segment*seg);
    bool IntersectsSegment(Segment*seg);
    bool IntersectsEnvelope(Envelope*env);
    bool IsPointOnSegment(double x,double y,double toler=VertexTolerance);
    vector<AutoPtr<Point>>IntersectSegment(Segment*seg);
    vector<AutoPtr<Point>>IntersectEnvelope(Envelope*env);
    void Reverse();
    AutoPtr<Polyline>ConvertToPolyline();
protected:
    bool EnvelopeIntersect(AutoPtr<Point>p1,AutoPtr<Point>p2,AutoPtr<Point>q);
    bool computeCollinearIntersection(AutoPtr<Point>p1,AutoPtr<Point>p2,AutoPtr<Point>q1,AutoPtr<Point>q2,vector<AutoPtr<Point>>&intersectPoints);
    void normalizeToEnvCentre(AutoPtr<Point>n00,AutoPtr<Point>n01,AutoPtr<Point>n10,AutoPtr<Point>n11,AutoPtr<Point>normPt);
    bool safeHCoordinateIntersection(AutoPtr<Point>p1,AutoPtr<Point>p2,AutoPtr<Point>q1,AutoPtr<Point>q2,AutoPtr<Point>ret);
    AutoPtr<Point>nearestEndpoint(AutoPtr<Point>p1,AutoPtr<Point>p2,AutoPtr<Point>q1,AutoPtr<Point>q2);
    void intersection(AutoPtr<Point>p1,AutoPtr<Point>p2,AutoPtr<Point>q1,AutoPtr<Point>q2,vector<AutoPtr<Point>>&intersectPoints);
    bool IsIntersect(Points*points,bool IsRing);
    int GetRingIntersectState(Points*points);
    int GetTouchState(Points*points);
    int GetCrossState(Points*points);
    bool IsInPoints(Points*points);
    int GetIntersectState(Points*points);
protected:
    double lnx1;
    double lny1;
    double lnz1;
    double lnx2;
    double lny2;
    double lnz2;
};

}
#endif // SEGMENT_H
