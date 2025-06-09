#ifndef GEOMETRYBORDERCAPTURE_H
#define GEOMETRYBORDERCAPTURE_H
#include "Base/autoptr.h"
#include "Point.h"
#include "points.h"
#include "polyline.h"
#include "polygon.h"
#include "multipolygon.h"
#include "geometry2d.h"
namespace SGIS{

enum GeometryBorderCaptureType
{
    gbctNone	= 0,
    gbctVertex	= 0x1,
    gbctSegment	= 0x2
};

struct GeometryBorderCaptureData
{
    enum GeometryBorderCaptureType CaptureType;
    LONG FeatureID;
    LONG PolygonIndex;
    LONG PartIndex;
    LONG PointIndex;
    DOUBLE CapturedX;
    DOUBLE CapturedY;
    GeometryBorderCaptureData(){
        CaptureType=GeometryBorderCaptureType::gbctNone;
        FeatureID=-1;
        PolygonIndex=-1;
        PartIndex=-1;
        PointIndex=-1;
        CapturedX=0;
        CapturedY=0;
    };
    GeometryBorderCaptureData(const GeometryBorderCaptureData&other){
        CaptureType=other.CaptureType;
        FeatureID=other.FeatureID;
        PolygonIndex=other.PolygonIndex;
        PartIndex=other.PartIndex;
        PointIndex=other.PointIndex;
        CapturedX=other.CapturedX;
        CapturedY=other.CapturedY;
    };
    GeometryBorderCaptureData&operator=(const GeometryBorderCaptureData&other){
        CaptureType=other.CaptureType;
        FeatureID=other.FeatureID;
        PolygonIndex=other.PolygonIndex;
        PartIndex=other.PartIndex;
        PointIndex=other.PointIndex;
        CapturedX=other.CapturedX;
        CapturedY=other.CapturedY;
        return *this;
    };
};

struct GeometryCapture
{
    int polyIndex;
    int partIndex;
    int pointIndex;
    GeometryCapture()
    {
        polyIndex=-1;
        partIndex=-1;
        pointIndex=-1;
    };
    GeometryCapture(int polyi,int parti,int pointi)
    {
        polyIndex=polyi;
        partIndex=parti;
        pointIndex=pointi;
    };
    GeometryCapture(const GeometryCapture&other)
    {
        polyIndex=other.polyIndex;
        partIndex=other.partIndex;
        pointIndex=other.pointIndex;
    };
    GeometryCapture(const GeometryBorderCaptureData&other)
    {
        polyIndex=other.PolygonIndex;
        partIndex=other.PartIndex;
        pointIndex=other.PointIndex;
    };
    GeometryCapture&operator=(const GeometryCapture&other)
    {
        polyIndex=other.polyIndex;
        partIndex=other.partIndex;
        pointIndex=other.pointIndex;
        return *this;
    };
    GeometryCapture&operator=(const GeometryBorderCaptureData&other)
    {
        polyIndex=other.PolygonIndex;
        partIndex=other.PartIndex;
        pointIndex=other.PointIndex;
        return *this;
    };
};
struct GeometryIntersectPoint
{
    int featureIndex;
    int polyIndex;
    int partIndex;
    int pointIndex;
    int reserved;
    Point2D<double> dpt;
    GeometryIntersectPoint()
    {
        polyIndex=-1;
        partIndex=-1;
        pointIndex=-1;
        reserved=-1;
        featureIndex=-1;
    };
    GeometryIntersectPoint(int polyi,int parti,int pointi,Point2D<double> pt)
    {
        polyIndex=polyi;
        partIndex=parti;
        pointIndex=pointi;
        dpt=pt;
    };
    GeometryIntersectPoint(const GeometryIntersectPoint&other)
    {
        polyIndex=other.polyIndex;
        partIndex=other.partIndex;
        pointIndex=other.pointIndex;
        dpt=other.dpt;
        reserved=other.reserved;
        featureIndex=other.featureIndex;
    };
    GeometryIntersectPoint&operator=(const GeometryIntersectPoint&other)
    {
        polyIndex=other.polyIndex;
        partIndex=other.partIndex;
        pointIndex=other.pointIndex;
        dpt=other.dpt;
        reserved=other.reserved;
        featureIndex=other.featureIndex;
        return *this;
    };
};
class SGIS_EXPORT GeometryIntersectPoints
{
public:
    GeometryIntersectPoints();
    virtual~GeometryIntersectPoints();
    int GetSize();
    GeometryIntersectPoint GetItem(int nIndex);
    void AddPoint(GeometryIntersectPoint point);
    void SetPoint(int nIndex,GeometryIntersectPoint point);
    void Clear();
    void Remove(int nIndex);
    bool ContainFeatureIndex(int featureIndex);
    void ReparePoins();
    void CopyFromOther(GeometryIntersectPoints&other);
protected:
    vector<GeometryIntersectPoint>points;
};

class SGIS_EXPORT GeometryTopo
{
public:
    GeometryTopo(AutoPtr<Geometry>geo);
    virtual~GeometryTopo();
    virtual Point2D<double> GetVertex(GeometryCapture gc)=0;
    virtual Segment2D<double> GetSegment(GeometryCapture gc)=0;
    virtual AutoPtr<Points> GetPoints(GeometryCapture gc)=0;
    virtual GeometryCapture CaptureVertex(DOUBLE x,DOUBLE y,DOUBLE tolerance)=0;
    virtual GeometryCapture CaptureVertex(DOUBLE x,DOUBLE y)=0;
    virtual void MoveVertex(GeometryCapture gc,DOUBLE x,DOUBLE y)=0;
    virtual GeometryCapture CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt)=0;
    virtual void InsertVertex(GeometryCapture gc,DOUBLE x,DOUBLE y)=0;
    virtual void IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points)=0;
    virtual bool CanDeleteVertex(GeometryCapture gc);
    virtual bool DeleteVertex(GeometryCapture gc);
    void IntersectLine(Points*pts,GeometryIntersectPoints&points);
    void IntersectClosedLine(Points*pts,GeometryIntersectPoints&points);
    static AutoPtr<GeometryTopo>CreateGeometryTopo(AutoPtr<Geometry>geo);
protected:
    AutoPtr<Geometry>geometry;
};
class SGIS_EXPORT PointsTopo :public GeometryTopo
{
public:
    PointsTopo(AutoPtr<Geometry>geo)
        :GeometryTopo(geo)
    {
    };
    virtual~PointsTopo()
    {
    };
    virtual Point2D<double> GetVertex(GeometryCapture gc);
    virtual Segment2D<double> GetSegment(GeometryCapture gc);
    virtual GeometryCapture CaptureVertex(DOUBLE x,DOUBLE y,DOUBLE tolerance);
    virtual GeometryCapture CaptureVertex(DOUBLE x,DOUBLE y);
    virtual GeometryCapture CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt);
    virtual void MoveVertex(GeometryCapture gc,DOUBLE x,DOUBLE y);
    void InsertVertex(GeometryCapture gc,DOUBLE x,DOUBLE y);
    AutoPtr<Points>GetPoints(GeometryCapture gc);
    virtual void IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points);
};

class SGIS_EXPORT PathTopo :public PointsTopo
{
public:
    PathTopo(AutoPtr<Geometry>geo)
        :PointsTopo(geo)
    {
    }
    virtual~PathTopo()
    {
    };
    Point2D<double> GetPoint(double percentage);
    double GetPercentage(GeometryIntersectPoint gc);
    bool CanDeleteVertex(GeometryCapture gc);
    bool DeleteVertex(GeometryCapture gc);
};

class SGIS_EXPORT RingTopo :public PointsTopo
{
public:
    RingTopo(AutoPtr<Geometry>geo)
        :PointsTopo(geo)
    {
    }
    virtual~RingTopo()
    {
    };
    GeometryCapture CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt);
    void MoveVertex(GeometryCapture gc,DOUBLE x,DOUBLE y);
    void IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points);
    void IntersectPath(Path*path,GeometryIntersectPoints&points);
    Point2D<double> GetPoint(double percentage);
    double GetPercentage(GeometryIntersectPoint gc);
    bool CanDeleteVertex(GeometryCapture gc);
    bool DeleteVertex(GeometryCapture gc);
};

class SGIS_EXPORT PolylineTopo: public GeometryTopo
{
public:
    PolylineTopo(AutoPtr<Geometry>geo)
        :GeometryTopo(geo)
    {

    };
    virtual~PolylineTopo()
    {

    };
    Point2D<double> GetVertex(GeometryCapture gc);
    Segment2D<double> GetSegment(GeometryCapture gc);
    GeometryCapture CaptureVertex(DOUBLE x,DOUBLE y,DOUBLE tolerance);
    GeometryCapture CaptureVertex(DOUBLE x,DOUBLE y);
    GeometryCapture CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt);
    void MoveVertex(GeometryCapture pc,DOUBLE x,DOUBLE y);
    void InsertVertex(GeometryCapture gc,DOUBLE x,DOUBLE y);
    AutoPtr<Points>GetPoints(GeometryCapture gc);
    void IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points);
    bool CanDeleteVertex(GeometryCapture gc);
    bool DeleteVertex(GeometryCapture gc);
};
class SGIS_EXPORT PolygonTopo :public GeometryTopo
{
public:
    PolygonTopo(AutoPtr<Geometry>geo)
        :GeometryTopo(geo)
    {

    };
    virtual~PolygonTopo()
    {

    };
    Point2D<double> GetVertex(GeometryCapture gc);
    Segment2D<double> GetSegment(GeometryCapture gc);
    GeometryCapture CaptureVertex(DOUBLE x,DOUBLE y,DOUBLE tolerance);
    GeometryCapture CaptureVertex(DOUBLE x,DOUBLE y);
    GeometryCapture CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt);
    void MoveVertex(GeometryCapture pc,DOUBLE x,DOUBLE y);
    void InsertVertex(GeometryCapture gc,DOUBLE x,DOUBLE y);
    AutoPtr<Points>GetPoints(GeometryCapture gc);
    void IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points);
    bool CanDeleteVertex(GeometryCapture gc);
    bool DeleteVertex(GeometryCapture gc);
};

class SGIS_EXPORT MultiPolygonTopo :public GeometryTopo
{
public:
    MultiPolygonTopo(AutoPtr<Geometry>geo)
        :GeometryTopo(geo)
    {

    };
    virtual~MultiPolygonTopo()
    {

    };
    Point2D<double> GetVertex(GeometryCapture gc);
    Segment2D<double> GetSegment(GeometryCapture gc);
    GeometryCapture CaptureVertex(DOUBLE x,DOUBLE y,DOUBLE tolerance);
    GeometryCapture CaptureVertex(DOUBLE x,DOUBLE y);
    GeometryCapture CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt);
    void MoveVertex(GeometryCapture pc,DOUBLE x,DOUBLE y);
    void InsertVertex(GeometryCapture gc,DOUBLE x,DOUBLE y);
    AutoPtr<Points>GetPoints(GeometryCapture gc);
    void IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points);
    bool CanDeleteVertex(GeometryCapture gc);
    bool DeleteVertex(GeometryCapture gc);
};

class SGIS_EXPORT GeometryBorderCapture{
public:
    GeometryBorderCapture();
    virtual~GeometryBorderCapture();
    AutoPtr<Point>GetPosition();
    void SetPosition(AutoPtr<Point>pos);
    void SetPosition(double x,double y);
    GeometryBorderCaptureData CaptureVertex(Geometry*geo);
    GeometryBorderCaptureData CaptureVertex(Geometry*geo,double toler);
    GeometryBorderCaptureData CaptureSegment(Geometry*geo,double toler);
    GeometryBorderCaptureData CaptureBorder(Geometry*geo,double toler);
    void MoveVertex(Geometry*geo,GeometryBorderCaptureData capdata,double x,double y);
    void InsertVertex(Geometry*geo,GeometryBorderCaptureData capdata,double x,double y);
    bool CanDeleteVertex(Geometry*geo,GeometryBorderCaptureData capdata);
    bool DeleteVertex(Geometry*geo,GeometryBorderCaptureData capdata);
protected:
    DOUBLE X;
    DOUBLE Y;
};

}
#endif // GEOMETRYBORDERCAPTURE_H
