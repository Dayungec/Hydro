#ifndef LABELCOMPUTER_H
#define LABELCOMPUTER_H

#include "Base/base_globle.h"
#include "geometry.h"
#include "geometry2d.h"
#include "math.h"
#include "polygon.h"
#include "envelope.h"

namespace SGIS{

class SGIS_EXPORT LabelPointComputer
{
public:
    LabelPointComputer();
    virtual~LabelPointComputer();
    vector<Point2D<double>>GetLabelPoint(Polygon*pRefGeo);
    void SetLeftTop(DOUBLE left,DOUBLE top);
    void SetCellSize(DOUBLE xcellSize,double ycellsize);
    void SetCols(int cols);
    void SetRows(int rows);
protected:
    BYTE*Convert(Polygon*pRefGeo);
    void ConvertPolygon(Polygon*poly,BYTE*pArray);
    void ConvertRing(Points*pnts,BYTE*pArray);
protected:
    long RegionNumber(int regionx,int regiony);
    int RegionX(double X);
    int RegionY(double Y);
    int RealRegionX(double X);
    int RealRegionY(double Y);
protected:
    DOUBLE Left;
    DOUBLE Top;
    int Rows;
    int Cols;
    Rect2D<double> FullExtent;
    double XCellSize;
    double YCellSize;
    DOUBLE DifFuzzy;
};

}
#endif // LABELCOMPUTER_H
