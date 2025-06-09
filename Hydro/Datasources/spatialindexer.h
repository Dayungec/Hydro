#ifndef SPATIALINDEXER_H
#define SPATIALINDEXER_H
#include "Base/autoptr.h"
#include "Base/BaseArray.h"
#include "Geometry/geometry2d.h"
#include "dataset.h"

namespace SGIS{

class SegmentRegionsComputer;
class SpatialIndexer;
class LocationSpatialIndexer;
class PointSpatialIndexer;

class SGIS_EXPORT SegmentRegionsComputer
{
public:
    SegmentRegionsComputer();
    virtual~SegmentRegionsComputer();
    void Initialize(double left,double top,double cellSizeX,double cellSizeY,int xNum,int yNum);
    int GetRegionX(double x);
    int GetRegionY(double y);
    double GetCellSizeX();
    double GetCellSizeY();
    Point2D<double>GetLeftTop();
    int GetRealRegionX(double x);
    int GetRealRegionY(double y);
    void GetSegmentRegions(const Segment2D<double>&seg,AutoArray<Point2D<int>>&pArray);
    void GetSegmentRegions(Path*path,AutoArray<Point2D<int>>&pArray);
protected:
    void AppendToArray(AutoArray<Point2D<int>>&pArray,int ix,int iy);
    void innerGetSegmentRegions(const Segment2D<double>&seg,AutoArray<Point2D<int>>&pArray);
protected:
    double left;
    double top;
    double cellSizeX;
    double cellSizeY;
    int xNum;
    int yNum;
};

class SGIS_EXPORT DistanceArray {
public:
    DistanceArray();
    virtual~DistanceArray();
    virtual void Add(int index, double dist)=0;
    int GetSize();
    virtual double GetDistance(int nIndex)=0;
    int GetFeatureIndex(int nIndex);
    virtual void Trim(int count);
    virtual void TrimToOther(int count, DistanceArray*other);
    virtual void Clear();
    void CopyFrom(DistanceArray*other);
    bool IsSameAs(DistanceArray*other);
    virtual void GetFeatureAndDistance(int nIndex,int&featureIndex,double&dist)=0;
    virtual float GetIDWValue(int nearCount,int power,vector<double>&values)=0;
protected:
    vector<int>indices;
};

class SGIS_EXPORT OrderDistanceArray:public DistanceArray{
public:
    OrderDistanceArray();
    virtual~OrderDistanceArray();
    virtual void Add(int index, double dist);
    double GetDistance(int nIndex);
    virtual void Trim(int count);
    void TrimToOther(int count, DistanceArray*other);
    virtual void Clear();
    void GetFeatureAndDistance(int nIndex,int&featureIndex,double&dist);
    float GetIDWValue(int nearCount,int power,vector<double>&values);
protected:
    OrderArray<double>dists;
    friend class MirrorDistanceArray;
    friend class IndexDistanceArray;
};

class SGIS_EXPORT IndexDistanceArray :public OrderDistanceArray{
public:
    IndexDistanceArray();
    virtual~IndexDistanceArray();
    void Add(int index,double dist);
    void Trim(int count);
    void TrimToOther(int count, DistanceArray*other);
    void Clear();
    bool HasFeature(int featureIndex);
protected:
    OrderArray<int>iis;
};

class SGIS_EXPORT MirrorDistanceArray :public DistanceArray {
public:
    MirrorDistanceArray();
    virtual~MirrorDistanceArray();
    void FromDistanceArray(LocationSpatialIndexer*sind,OrderDistanceArray*distArray,double centerX,double centerY,int ptNum);
    void MoveCenter(LocationSpatialIndexer*sind,double centerX,double centerY);
    void Add(int index, double dist);
    double GetDistance(int nIndex);
    void GetFeatureAndDistance(int nIndex,int&featureIndex,double&dist);
    float GetIDWValue(int nearCount,int power,vector<double>&values);
    void Clear();
    void Trim(int count);
    void TrimToOther(int count, DistanceArray*other);
protected:
    Point2D<double>refPos;
    vector<double>dists;
    double freeDist;
    int ptNum;
    OrderDistanceArray tempArray;
    friend class OrderDistanceArray;
    friend class IndexDistanceArray;
};

class SGIS_EXPORT SpatialCell{
public:
    SpatialCell();
    virtual~SpatialCell();
    void Add(int findex);
    int GetSize();
    int GetFeatureIndex(int nIndex);
    int operator[](int nIndex);
    void Clear();
protected:
    OrderArray<int>findices;
};

class SGIS_EXPORT SpatialIndexer
{
public:
    SpatialIndexer(int maxWidthOrHeight=50);
    virtual~SpatialIndexer();
    void Clear();
    void GetFeatures(Envelope*env,OrderArray<int>&pArray);
    void BuildSpatialIndices(AutoPtr<FeatureClass>pfc,AutoPtr<SpatialReference>psp=nullptr);
    void BuildSpatialIndicesEx(AutoPtr<FeatureClass>pfc,double mintoler,AutoPtr<SpatialReference>psp=nullptr);
    void SearchNearstFeaturesByRaius(double x,double y,double toler,IndexDistanceArray&results);
    void SearchNearstFeaturesByNum(double x,double y,int Num,IndexDistanceArray&results);
    void SearchNearstFeaturesByRaius(Point*dpt,double toler,IndexDistanceArray&results);
    void SearchNearstFeaturesByNum(Point*dpt,int Num,IndexDistanceArray&results);
    int GetRegionX(double x);
    int GetRegionY(double y);
    int GetRealRegionX(double x);
    int GetRealRegionY(double y);
protected:
    void InitialCells(Envelope*fullExtent);
    void RegisterPoint(Point*pt,int fIndex);
    void RegisterPath(Path*path,int fIndex);
    void RegisterEnvelope(Envelope*env,int fIndex);
    void RegisterPolyline(Polyline*poly,int fIndex);
    void RegisterGeometry(Geometry*geo,int fIndex);
protected:
    vector<SpatialCell*>regions;
    double left;
    double top;
    double right;
    double bottom;
    double cellSize;
    int xNum;
    int yNum;
    SegmentRegionsComputer sgc;
    AutoPtr<FeatureClass>pfc;
    int maxWidthOrHeight;
    AutoPtr<SpatialReference>psp;
    bool canTransform=false;
    AutoPtr<CoordinateTransformation>pTrans;
};

class SGIS_EXPORT SpatialLocations{
public:
    SpatialLocations(){};
    virtual~SpatialLocations(){};
    virtual Point2D<double>GetLocation(int nIndex)=0;
    virtual void GetLocation(int nIndex,Point2D<double>&loc)=0;
    virtual int GetSize()=0;
    virtual AutoPtr<Envelope>GetExtent()=0;
};

class SGIS_EXPORT SpatialPoints :public SpatialLocations{
public:
    SpatialPoints();
    virtual~SpatialPoints();
    Point2D<double>GetLocation(int nIndex);
    void GetLocation(int nIndex,Point2D<double>&loc);
    int GetSize();
    AutoPtr<Envelope>GetExtent();
    void Add(Point2D<double>pos);
    void Clear();
protected:
    AutoPtr<Points>dpts;
};

class SGIS_EXPORT LocationSpatialIndexer
{
public:
    LocationSpatialIndexer(int maxWidthOrHeight=50);
    virtual~LocationSpatialIndexer();
    void Clear();
    bool HasBuildSpatialIndices();
    bool BuildSpatialIndices(SpatialLocations*dpts);
    AutoPtr<DistanceArray>SearchNearstFeaturesByRaius(double x,double y,double toler);
    AutoPtr<DistanceArray>SearchNearstFeaturesByNum(double x,double y,int Num);
    AutoPtr<DistanceArray>SearchNearstFeaturesByRaius(Point*dpt,double toler);
    AutoPtr<DistanceArray>SearchNearstFeaturesByNum(Point*dpt,int Num);
    AutoPtr<DistanceArray>SearchNearstFeaturesByNumEx(double x,double y,int Num,int M);
    int GetRegionX(double x);
    int GetRegionY(double y);
    int GetRealRegionX(double x);
    int GetRealRegionY(double y);
    AutoPtr<Envelope>GetFullExtent();
    Point2D<double> GetFeaturePoint(int fIndex);
    void GetFeaturePoint(int fIndex,Point2D<double>&loc);
protected:
    void InitialCells(Envelope*fullExtent);
    void RegisterPoint(Point*pt,int fIndex);
    void RegisterPoint(DOUBLE X,double Y,int fIndex);
protected:
    vector<SpatialCell*>regions;
    double left;
    double top;
    double right;
    double bottom;
    double cellSize;
    int xNum;
    int yNum;
    AutoPtr<FeatureClass>pfc;
    int maxWidthOrHeight;
    AutoPtr<SpatialReference>psp;
    AutoPtr<OrderDistanceArray>distArray;
    AutoPtr<OrderDistanceArray>bufferdistArray;
    AutoPtr<MirrorDistanceArray>buffermirrorArray;
    AutoPtr<MirrorDistanceArray>mdistArray;
    Point2D<double>formerPos;
    SpatialLocations*dpts;
    int formerPtNum;
    bool onlyRef;
    double kToler;
};

class SGIS_EXPORT PointSpatialIndexer :public LocationSpatialIndexer
{
public:
    PointSpatialIndexer(int maxWidthOrHeight=50);
    virtual~PointSpatialIndexer();
    bool BuildSpatialIndices(AutoPtr<FeatureClass>pfc,AutoPtr<SpatialReference>psp=nullptr);
    bool BuildSpatialIndices(AutoPtr<Points>pts);
};

class SGIS_EXPORT ManualPointSpatialIndexer :public LocationSpatialIndexer{
public:
    ManualPointSpatialIndexer(int widthOrHeight=50);
    virtual~ManualPointSpatialIndexer();
    void Create(AutoPtr<Envelope>env);
    bool Add(AutoPtr<Point>dpt);
    bool CanSearchNearFeature(double x,double y,double distance);
protected:
    int nIndex;
};

}
#endif // SPATIALINDEXER_H
