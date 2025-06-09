#ifndef RASTERTOFEATURE_H
#define RASTERTOFEATURE_H
#include "samodel.h"
#include "featureclasstarget.h"

namespace SGIS{

class RRing
{
public:
    RRing(int deri,int firstx,int firsty);
    virtual~RRing();
    void RemoveAll();
    void AddPoint(Point2D<double>pt,int state);
    int GetCount();
    Point2D<double> Item(int index);
    int GetState(int index);
    int GetDeri();
    int GetFirstX();
    int GetFirstY();
    void SimplifyRing(double fuzzy,double minfuzzy);
    int GetPolyIndex();
    void SetPolyIndex(int index);
    DOUBLE*GetRawData();
    int GetRingValue();
    void SetRingValue(int value);
    bool IsPointIn(double x,double y);
    bool IsRingIn(RRing*otherRing);
    double GetArea();
protected:
    void ReparePoints();
private:
    void SimplifyRingBetweenBorder(int findex,int tindex,double fuzzy,BYTE*states,double minfuzzy);
    void SimplifyRing(int findex,int tindex,double fuzzy,BYTE*states);
    vector<Point2D<double>>pts;
    vector<unsigned char>States;
    int Deri;
    int FirstX,FirstY;
    int PolyIndex;
    int numofPerBlock;
    int currentPin;
    int blockNum;
    int ringValue;
};
class RPolygon
{
public:
    RPolygon();
    virtual~RPolygon();
    int GetCount();
    RRing*GetRing(int index);
    void AddRing(RRing*ring);
    double GetArea();
    void SimplifyPolygon(double fuzzy,double minfuzzy);
private:
    vector<RRing*>rings;
};

class SGIS_EXPORT RasterToPolygon :public SAModel
{
public:
    RasterToPolygon();
    virtual~RasterToPolygon();
    AutoPtr<FeatureClass>Execute(RasterBand*pBand,FeatureClassTarget*target,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
protected:
    bool InitialBorder(RasterBand*pRasterBand,CallBack*callBack,Semaphora*signal);
    bool PursueRing(int x,int y);
    void GetNextPoint(int Deri,int&Posi,int&x,int&y,int ri);
    bool RegisterRings(CallBack*callBack,Semaphora*signal);
    bool RegisterPolygons(CallBack*callBack,Semaphora*signal);
    AutoPtr<Polygon>get_Polygon(LONG index);
    int GetStateOfBorderPixel(int x,int y);
protected:
    FLOAT NoData;
    BYTE*b0;
    BYTE*b1;
    INT*data;
    LONG Cols,Rows;
    vector<RPolygon*>polys;
    vector<RRing*>rings;
    double Left,Top,Right,Bottom;
    double xCellSize;
    double yCellSize;
    float FuzzyRatio;
};

};
#endif // RASTERTOFEATURE_H
