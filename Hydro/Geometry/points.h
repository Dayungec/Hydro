#ifndef POINTS_H
#define POINTS_H
#include "geometry.h"
#include "Point.h"
#include "spline.h"

namespace SGIS{

class SGIS_EXPORT Path{
public:
    Path(){};
    virtual~Path(){};
    virtual bool Smooth(double toler)=0;
    virtual bool SmoothByPtNum(int ptNum)=0;
    virtual void Simplify(double toler)=0;
    virtual void SimplifyByPtNum(int ptNum)=0;
    virtual double GetLength()=0;
    virtual bool CanCaptureBorder(Point*ppt,double toler)=0;
    virtual AutoPtr<Point>CaptureBorder(Point*ppt,double toler)=0;
    virtual AutoPtr<Point>ReturnPoint(double length)=0;
    virtual AutoPtr<Points>ReturnPoints(const vector<double>&lengths)=0;
    virtual AutoPtr<Points>ReturnPoints(double fromlen,double interval)=0;
};

class SGIS_EXPORT Path3D{
public:
    Path3D(){};
    virtual~Path3D(){};
    virtual double GetLength3D()=0;

};

class SGIS_EXPORT Ring{
public:
    Ring(){};
    virtual~Ring(){};
    virtual double GetPerimeter()=0;
    virtual double GetArea()=0;
    virtual double GetDiameter(int&node1, int&node2)=0;
    virtual bool SmoothRing(double toler)=0;
    virtual bool SmoothRingByPtNum(int ptNum)=0;
    virtual void SimplifyRing(double toler)=0;
    virtual void SimplifyRingByPtNum(int ptNum)=0;
    virtual bool IsPointIn(Point*pt)=0;
    virtual bool IsRingIn(Ring*other)=0;
    virtual AutoPtr<Point>GetGeometryCenter()=0;
    virtual AutoPtr<Point>GetGavityCenter()=0;
    virtual bool CanCaptureRingBorder(Point*ppt,double toler)=0;
    virtual AutoPtr<Point>CaptureRingBorder(Point*ppt,double toler)=0;
    virtual AutoPtr<Point>ReturnRingPoint(double length)=0;
    virtual AutoPtr<Points>ReturnRingPoints(const vector<double>&lengths)=0;

};

class SGIS_EXPORT Ring3D{
public:
    Ring3D(){};
    virtual~Ring3D(){};
    virtual double GetPerimeter3D()=0;
};

class SGIS_EXPORT Points:
        public Geometry,
        public Geometry3D,
        public Path,
        public Path3D,
        public Ring,
        public Ring3D
{
public:
    Points();
    virtual~Points();
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
//IPath:
    double GetLength();
    bool Smooth(double toler);
    bool SmoothByPtNum(int ptNum);
    void Simplify(double toler);
    void SimplifyByPtNum(int ptNum);
    bool CanCaptureBorder(Point*ppt,double toler);
    AutoPtr<Point>CaptureBorder(Point*ppt,double toler);
    AutoPtr<Point>ReturnPoint(double length);
    AutoPtr<Points>ReturnPoints(const vector<double>&lengths);
    AutoPtr<Points>ReturnPoints(double fromlen,double interval);
//IPath3D:
    double GetLength3D();
//IRing:
    double GetPerimeter();
    double GetArea();
    double GetDiameter(int&node1, int&node2);
    double GetDiameter();
    bool SmoothRing(double toler);
    bool SmoothRingByPtNum(int ptNum);
    void SimplifyRing(double toler);
    void SimplifyRingByPtNum(int ptNum);
    bool CanCaptureRingBorder(Point*ppt,double toler);
    AutoPtr<Point>CaptureRingBorder(Point*ppt,double toler);
    bool IsPointIn(Point*pt);
    bool IsRingIn(Ring*other);
    AutoPtr<Point>GetGeometryCenter();
    AutoPtr<Point>GetGavityCenter();
    AutoPtr<Point>ReturnRingPoint(double length);
    AutoPtr<Points>ReturnRingPoints(const vector<double>&lengths);
    AutoPtr<Points>ReturnRingPoints(double fromlen,double interval);
//IRing3D:
    double GetPerimeter3D();
//Points:
    int GetSize();
    void Add(double x,double y,double z=0);
    void SetSize(int size);
    void Insert(int nIndex,double x,double y,double z=0);
    void Remove(int nIndex);
    void Clear();
    void GetItem(int nIndex,double&x,double&y);
    void GetItem(int nIndex,double&x,double&y,double&z);
    void SetItem(int nIndex,double x,double y);
    void SetItem(int nIndex,double x,double y,double z);
    void PutCoordByPtr(int ptCount,DOUBLE*data);
    AutoPtr<Point>operator [](int nIndex);
    void ComputeEnvelope();
    double*GetXYs();
    double*GetZs();
    void Reverse();
protected:
    vector<DOUBLE>XYs;
    vector<DOUBLE>Zs;
    double XMin;
    double XMax;
    double YMin;
    double YMax;
    double ZMin;
    double ZMax;
    bool IsExtentValid;
};

}
#endif // POINTS_H
