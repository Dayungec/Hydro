#ifndef TOPOBUILDER_H
#define TOPOBUILDER_H
#include "Base/BaseArray.h"
#include "geometry.h"
#include "Point.h"
#include "points.h"
#include "envelope.h"
#include "ellipse.h"
#include "polygon.h"
#include "multipolygon.h"
#include "Base/CallBack.h"

namespace SGIS{

class TopoPoint;
class TopoLine;
class LineRegionElement;
class LineRegion;
class LineRegions;
class TopoPoints;
class TopoLines;
class TopoRing;
class TopoRings;
class TopoBuilder;

class SGIS_EXPORT TopoPoint
{
public:
    TopoPoint();
    TopoPoint(double x,double y);
    virtual~TopoPoint();
    void AddLine(TopoLine*line);
    void RemoveLine(TopoLine*line);
    bool IsSamePoint(TopoPoint*other,double fuzzy);
    bool IsSamePoint(Point2D<double> dpt,double fuzzy);
    bool HasMultipleAttributes();
    bool FindLine(TopoPoint*other);
public:
    double X;
    double Y;
    TopoPoint*father;
    TopoPoint*child;
    vector<TopoLine*>lines;
};
class SGIS_EXPORT TopoLine
{
public:
    TopoLine();
    TopoLine(int attribute);
    TopoLine(TopoPoint*fN,TopoPoint*tN);
    TopoLine(TopoPoint*fN,TopoPoint*tN,int attribute);
    virtual~TopoLine();
    int GetAttributesCount();
    int GetAttribute(int nIndex);
    void AddAttributes(int at);
    bool FindAttributes(int at);
    int GetMinPositiveAttribute();
    int GetMaxNegtiveAttribute();
    int GetMaxAbsAttribute();
    void CombineAttributes(TopoLine*otherLine,bool negative=false);
    void CopyAttributes(TopoLine*otherLine);
    void CombineFromPoint(TopoPoint*point);
    void CombineToPoint(TopoPoint*point);
    void SetFromNode(TopoPoint*point);
    void SetToNode(TopoPoint*point);
    AutoPtr<Envelope>ComputeEnvelope();
    AutoPtr<Polyline>CreatePolyline();
public:
    TopoPoint*fromNode;
    TopoPoint*toNode;
    vector<Point2D<double>>vertices;
    TopoLine*father;
    TopoLine*child;
    int state;
    vector<LineRegionElement*>regionEls;
    TopoRing*leftRing;
    TopoRing*rightRing;
protected:
    vector<int>attributes;
};
class SGIS_EXPORT LineRegionElement
{
public:
    LineRegionElement(LineRegion*region,TopoLine*tLine);
    virtual~LineRegionElement();
public:
    TopoLine*topoLine;
    LineRegionElement*father;
    LineRegionElement*child;
    LineRegion*lineRegion;
};
class SGIS_EXPORT LineRegion
{
public:
    LineRegion();
    virtual~LineRegion();
    void RemoveAll();
    LineRegionElement*AddLine(TopoLine*tLine);
    void RemoveLine(LineRegionElement*el);
    int GetCount();
public:
    LineRegionElement*rootElement;
    LineRegionElement*tailElement;
    int count;
};
class SGIS_EXPORT LineRegions
{
public:
    LineRegions(Envelope*fullExtent);
    LineRegions(Envelope*fullExtent,int xSize,int ySize);
    virtual~LineRegions();
    int GetRegionXNum();
    int GetRegionYNum();
    LineRegion*GetRegion(int nIndex);
    int GetRegionX(double x);
    int GetRegionY(double y);
    int GetRealRegionX(double x);
    int GetRealRegionY(double y);
    bool IsSameRegion(TopoPoint*firstPoint,TopoPoint*secondPoint);
    void AppendToArray(AutoArray<Point2D<int>>&pArray,int ix,int iy);
    void GetLineRegions(TopoLine*tLine,AutoArray<Point2D<int>>&pArray);
    void RegisterLine(TopoLine*tLine);
    void UpdateLine(TopoLine*tLine);
    void RegisterContour(TopoLine*tLine);
    void UnregisterLine(TopoLine*tLine);
    void Clear();
protected:
    void InitialCells();
protected:
    vector<LineRegion*>regions;
    double left;
    double top;
    double right;
    double bottom;
    double difx;
    double dify;
    int xNum;
    int yNum;
};
class SGIS_EXPORT TopoPoints
{
public:
    TopoPoints();
    virtual~TopoPoints();
    int GetPointCount();
    AutoPtr<Envelope>ComputeEnvelope();
    TopoPoint*AddPoint(double x,double y);
    void AddPoint(TopoPoint*point);
    void RemovePoint(TopoPoint*point);
    void Clear();
public:
    TopoPoint*rootPoint;
    TopoPoint*tailPoint;
    int pointCount;
};
class SGIS_EXPORT TopoLines
{
public:
    TopoLines();
    virtual~TopoLines();
    void BuildSpatialIndex(Envelope*fullExtent,int regionNum);
    void BuildContourSpatialIndex(Envelope*fullExtent,int regionNum);
    void AddLine(TopoLine*line);
    void RemoveLine(TopoLine*line);
    void Clear();
    int GetLineCount();
public:
    TopoLine*rootLine;
    TopoLine*tailLine;
    LineRegions*regions;
    int lineCount;
};
class SGIS_EXPORT TopoRing
{
public:
    TopoRing();
    virtual~TopoRing();
    AutoPtr<Envelope>GetEnvelope();
    void AddFirstLine(TopoPoint*fromPoint,TopoLine*tLine);
    void AddLine(TopoLine*tLine);
    void AddInnerRing(TopoRing*ring);
    int GetInnerRingCount();
    TopoRing*GetInnerRing(int nIndex);
    int GetLineCount();
    TopoLine*GetLine(int nIndex);
    void CloseRing();
    bool GetIsClockWise();
    double GetArea();
    AutoPtr<Polygon>CreatePolygon(bool isCurve);
    void RegisterClockwiseLines();
    void UnregisterClockwiseLines();
    void RegisterCounterClockwiseLines(TopoRing*outerRing);
    void UnregisterCounterClockwiseLines();
    void CopyLines(TopoRing*other);
    bool IsPointIn(double x,double y);
    bool IsPointIn(TopoPoint*point);
    bool IsRingIn(TopoRing*other);
    TopoRing*FindOuterRing();
    void FindNeighborRings(vector<TopoRing*>&neighbors);//查找与之共享边的相邻ring
    bool FindAttribute(int at);
    bool FindNeighborAttribute(int at);
    int GetMinPositiveAttribute();
    int GetMaxNegtiveAttribute();
protected:
    void ComputeEnvelope();
protected:
    Ring* CreateRing();
    vector<TopoRing*>innerRings;
    vector<TopoLine*>lines;
    AutoPtr<Envelope>extent;
    double area;
public:
    TopoRing*father;
    TopoRing*child;
    TopoPoint*fromPoint;
    TopoRing*attachRing;//用于在空间索引网格中判断是否已经搜索过的指针
};
//思路：在构建线拓扑基础上，构建环拓扑并建立空间索引；形成环链；遍历所有环，如果环为顺时针，则将线的左多边形或右多边形写入；对于逆时针环，寻求空间范围大于等于该环的所有顺时针环并进行IsRingIn判别，如果有多个满足，找到最小的那一个，同时将线的左或右还更新。对于环，注意的是需要有一个数组表达内环。这样在形成多边形时速度快一些。
class SGIS_EXPORT RingRegion
{
public:
    RingRegion();
    virtual~RingRegion();
    void AddRing(TopoRing*ring);
    TopoRing*GetItem(int nIndex);
    int GetCount();
    void RemoveAll();
protected:
    vector<TopoRing*>rings;
};
class SGIS_EXPORT RingRegions
{
public:
    RingRegions(Envelope*fullExtent);
    RingRegions(Envelope*fullExtent,int xSize,int ySize);
    virtual~RingRegions();
    int GetRegionXNum();
    int GetRegionYNum();
    int GetRegionX(double x);
    int GetRegionY(double y);
    RingRegion*GetRegion(int nIndex);
    void RegisterRing(TopoRing*ring);
    void GetRegionEnvelope(Envelope*env,int&il,int&it,int&ir,int&ib);
    void Clear();
protected:
    void InitialCells();
protected:
    vector<RingRegion*>regions;
    double left;
    double top;
    double right;
    double bottom;
    double difx;
    double dify;
    int xNum;
    int yNum;
};
class SGIS_EXPORT TopoRings
{
public:
    TopoRings();
    virtual~TopoRings();
    void BuildSpatialIndex(Envelope*fullExtent,int regionNum);
    void AddRing(TopoRing*ring);
    void RemoveRing(TopoRing*ring);
    void DetachRing(TopoRing*ring);
    void Clear();
    int GetRingCount();
    TopoRing*FindOuterRing(TopoRing*innerRing);
protected:
    RingRegions*rRegions;
    int ringCount;
public:
    TopoRing*rootRing;
    TopoRing*tailRing;
};
class SGIS_EXPORT TopoBuilder
{
public:
    TopoBuilder();
    TopoBuilder(double fuzy);
    virtual~TopoBuilder();
    TopoLines*GetLines();
    TopoPoints*GetPoints();
    TopoRings*GetRings();
    void ConnectPoint(TopoPoint*topoPt1,TopoPoint*topoPt2,int attribute);
    void ConnectPointEx(TopoPoint*topoPt1,TopoPoint*topoPt2,vector<Point2D<double>>&vertices,int attribute);
    void SetFullExtent(Envelope*env);
    TopoPoint* AppendPoint(Point*ppt);
    TopoLine*AppendSegment(Segment*seg,int attribute);
    void AppendPoints(Points*ppts);
    void AppendRing(Points*pts);
    void AppendPath(Points*pts);
    void AppendRing(Points*pts,int attribute);
    void AppendPath(Points*pts,int attribute);
    void AppendPolyline(Polyline*polyline,int attribute);
    void AppendPolygon(Polygon*poly,int attribute);
    void AppendEnvelope(Envelope*env,int attribute);
    void AppendEllipse(Ellipse*env,int attribute);
    void AppendMultiPolygon(MultiPolygon*mpoly,int attribute);
    void AppendGeometry(Geometry*geo,int attribute);
    void BuidLineTopology();
    void BuidLineTopologyNoIntersect();
    void BuildContours(CallBack*callBack);
    void BuildPolygonTopology();
    void RemoveNoAttributeLines();
    void CreatePolygons(vector<AutoPtr<Polygon>>&polys);
    void CombineBuilder(TopoBuilder&tb);
    void Clear();
protected:
    void IntersectLines(int regionNum=50);
    void BuildLineTopo();
    void RemovePhesudos();
    void RemoveIsolateLine();
    void IntersectLine(TopoLine*tLine);
    void PursuitRings();
    void BuildPolygons();
    bool BuildContour(TopoLine*tLine1,TopoLine*tLine2);
    void BuildContour(TopoLine*tLine);
protected:
    bool RemovePhesudo(TopoPoint*topoPoint);
    double TriangleAngle(Point2D<double>&fromPoint,Point2D<double>&centerPoint,Point2D<double>&toPoint);
    TopoLine*GetNextLine(TopoPoint*centerPoint,TopoLine*fromLine);
    bool IntersectLine(TopoLine*tLine1,TopoLine*tLine2,bool&delLine);
    bool IntersectLineOfSameNode(TopoLine*tLine1,TopoLine*tLine2,bool&delLine);
    bool RemoveZeroLine(TopoLine*tLine);
    void PursuitLine(TopoLine*tLine,TopoPoint*fromPoint);
    int GetBestRegionNum();
protected:
    TopoPoints*tPoints;
    TopoLines*tLines;
    TopoRings*oRings;
    TopoRings*iRings;
    double fuzzy;
    AutoPtr<Envelope>fullExtent;
    AutoArray<Point2D<double>>pArray;
};

class SGIS_EXPORT PolylineOper{
public:
    PolylineOper();
    virtual~PolylineOper();
    static AutoPtr<Polyline>Build(Polyline*geo,double toler=0.00001);
    static AutoPtr<Polyline>Union(Polyline*poly1,Polyline*poly2,double toler=0.00001);
};

class SGIS_EXPORT PolygonSplitter
{
public:
    PolygonSplitter();
    virtual~PolygonSplitter();
    static vector<AutoPtr<Polygon>> SplitPolygon(Polygon*polygon,Points*splitLine);
    static vector<AutoPtr<Polygon>> SplitMultiPolygon(MultiPolygon*mpolygon,Points*splitLine);
protected:
    static bool IsRingValid(TopoRing*tRing);
};


class SGIS_EXPORT PolygonOper
{
public:
    PolygonOper();
    virtual~PolygonOper();
    static AutoPtr<Geometry> Build(Geometry*poly);
    static AutoPtr<Geometry> ReparePolygon(Polygon*poly);
    static AutoPtr<Geometry> ReparePolygon(MultiPolygon*mpoly);
    static AutoPtr<Geometry> Intersect(Geometry*polygon,Geometry*otherPoly);
    static AutoPtr<Geometry> Union(Geometry*polygon,Geometry*otherPoly);
    static AutoPtr<Geometry> Difference(Geometry*polygon,Geometry*otherPoly);
    static AutoPtr<Geometry> SymmetricDifference(Geometry*polygon,Geometry*otherPoly);
protected:
    static AutoPtr<Geometry>ConvertPolygon(Geometry*geo);
    static void RegisterPolygon(TopoBuilder*tb,Polygon*polygon,int attribute);
    static void RegisterPolygon(TopoBuilder*tb,MultiPolygon*mpolygon,int attribute);
    static void RegisterGeometry(TopoBuilder*tb,Geometry*geo,int attribute);
};


}

#endif // TOPOBUILDER_H
