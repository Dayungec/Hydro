#ifndef SEGMENTINDEXES_H
#define SEGMENTINDEXES_H

#include "Base/base_globle.h"
#include "geometry.h"
#include "geometry2d.h"
#include "envelope.h"
#include "ellipse.h"
#include "segment.h"
#include "points.h"
#include "polyline.h"
#include "polygon.h"
#include "multipolygon.h"

namespace SGIS{

struct SegmentIndex
{
    int polyIndex;
    int partIndex;
    int pointIndex;
    int pointCount;
    Segment2D<double> formerseg;
    Segment2D<double> seg;
    int state;
    SegmentIndex()
    {
        polyIndex=partIndex=pointIndex-1;
        pointCount=0;
        state=0;
    }
    SegmentIndex(int polyindex,int partindex,int pointindex,int pCount,Segment2D<double>&segment)
    {
        polyIndex=polyindex;
        partIndex=partindex;
        pointIndex=pointindex;
        pointCount=pCount;
        seg=segment;
        state=0;
    }
    SegmentIndex(const SegmentIndex&other)
    {
        polyIndex=other.polyIndex;
        partIndex=other.partIndex;
        pointIndex=other.pointIndex;
        pointCount=other.pointCount;
        seg=other.seg;
        state=other.state;
        formerseg=other.formerseg;
    };

};

class SegmentRegion
{
public:
    SegmentRegion();
    virtual~SegmentRegion();
    void AddSegment(SegmentIndex seg);
    int GetSize();
public:
    vector<SegmentIndex>segments;
};

class SegmentRegions
{
public:
    SegmentRegions();
    SegmentRegions(int xSize,int ySize);
    virtual~SegmentRegions();
    int GetRegionXNum();
    int GetRegionYNum();
    SegmentRegion*GetRegion(int nIndex);
    void RegisterSegment(SegmentIndex index);
    void InitialCells(Envelope*fullExtent);
    int GetRegionX(double x);
    int GetRegionY(double y);
    int GetRealRegionX(double x);
    int GetRealRegionY(double y);
    void GetLineRegions(double x1,double y1,double x2,double y2,AutoArray<Point2D<int>>&pArray);
    void GetLineRegions(Path*path,AutoArray<Point2D<int>>&pArray);
protected:
    void AppendToArray(AutoArray<Point2D<int>>&pArray,int ix,int iy);
    void GetLineRegions(SegmentIndex&index,AutoArray<Point2D<int>>&pArray);
    void innerGetLineRegions(double x1,double y1,double x2,double y2,AutoArray<Point2D<int>>&pArray);
protected:
    vector<SegmentRegion*>regions;
    double left;
    double top;
    double right;
    double bottom;
    double difx;
    double dify;
    int xNum;
    int yNum;
};

class SegmentIndexes
{
public:
    SegmentIndexes(AutoPtr<Geometry>geo,int cellNum);
    virtual~SegmentIndexes();
    void BuildSpatialIndex();
    bool Intersect(SegmentIndexes*other);
    bool IntersectGeometry(AutoPtr<Geometry>pGeo);
    int GetSegmentIntersectState(Segment2D<double>&seg,int label,Segment2D<double>&intersectSeg);//0--不相交；1--一个交点；2--2个交点；3--相交；4--对于1个交点，如果交点位于本线段的起点;
    int GetSegmentTouchState(Segment2D<double>&seg,int label,Segment2D<double>&intersectSeg);//0--不相交；1--一个交点；2--2个交点；3--相交；4--对于1个交点，如果交点位于本线段的起点;对于等于2的情况，由于已经不满足线段的Touch条件，直接返回
    int GetRingSegmentIntersectState(Segment2D<double>&seg,int label);
    int GetRingSegmentIntersectOuterState(Segment2D<double>&seg,int label);
    int GetRingSegmentIntersectInnerState(Segment2D<double>&seg,int label);
    int GetRingSegmentIntersectState(Path*pPath);
    int GetRingSegmentIntersectState(Ring*pPath);
    int GetRingSegmentIntersectOuterState(Path*pPath);
    int GetRingSegmentIntersectOuterState(Ring*pPath);
    int GetRingSegmentIntersectInnerState(Path*pPath);
    int GetRingSegmentIntersectInnerState(Ring*pPath);
    int GetPolylineIntersectState(Polyline*polyline);//0--不相交；1--一个交点；2--2个交点；3--相交；
    int GetPolylineTouchState(Polyline*polyline);//0--不相交；1--一个交点；2--2个交点；3--相交；
    int GetPolylineCrossState(Polyline*polyline);//0--不相交；1--一个交点；2--2个交点；3--相交；
    int IsPathTouchRing(Path*pPath);//0---不支持Touch；1--支持Touch；2:不相交;3--不能确定
    int IsRingTouchRing(Ring*pRing);//0---不支持Touch；1--支持Touch；2:不相交;3--不能确定
    int IsPathCrossRing(Path*pPath);//0---不支持Touch；1--支持Touch；2:不相交;3--不能确定
    int IsRingCrossRing(Ring*pRing);//0---不支持Touch；1--支持Touch；2:不相交;3--不能确定
public:
    bool IntersectSegment(Segment2D<double>&seg,int label);
    bool IntersectPoint(Point*point);
    bool IntersectPath(Path*pPath);
    bool IntersectRing(Ring*pRing);
    bool IntersectPolyline(Polyline*polyline);
    bool IntersectPolygon(Polygon*polygon);
    bool IntersectMultiPolygon(MultiPolygon*polygon);
    void ResetSegmentStates();
protected:
    void BuildPolyline(Polyline*poly);
    void BuildPolygon(int polyIndex,Polygon*poly);
    void BuildMultiPolygon(MultiPolygon*poly);
    void BuildGeometry();
    void Clear();
    void SetSize(int size);
    AutoPtr<Geometry>ConvertGeometry(AutoPtr<Geometry>geo);
    AutoPtr<Polyline>ConvertToPolyline(AutoPtr<Polyline>geo);
    AutoPtr<Polygon> ConvertToPolygon(AutoPtr<Polygon>geo);
protected:
    void RegisterPoints(Points*points,int polyIndex,int partIndex,bool IsRing);
    bool IsCurrentGeometryPolygon();
protected:
    AutoPtr<Geometry>pGeo;
    vector<SegmentIndex>indexes;
    SegmentRegions*regions;
    int cellNum;
};

}
#endif // SEGMENTINDEXES_H
