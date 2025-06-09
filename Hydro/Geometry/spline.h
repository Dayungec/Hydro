#ifndef SPLINE_H
#define SPLINE_H
#include "geometry2d.h"
#include "Point.h"
#include "math.h"

namespace SGIS {

class Curve
{
public:
    Curve();
    Curve(double ax,double ay,double bx,double by,double cx,double cy,int ndiv);
    Curve(double ax,double ay,double bx,double by,double cx,double cy,double fuzzy);
    virtual~Curve();
    void PutCurveByDiv(double ax,double ay,double bx,double by,double cx,double cy,int ndiv);
    void PutCurve(double ax,double ay,double bx,double by,double cx,double cy,double fuzzy);
    int GetCount();
    void GetCurve(double x,double y, Point2D<double> points[], int& PointCount);
    void GetCurveByDiv(double x,double y, Point2D<double> points[], int& PointCount);
protected:
    double  Ax,Ay;
    double  Bx,By;
    double  Cx,Cy;
    int    Ndiv;
};

class SGIS_EXPORT Spline
{
public:
    // constructor
    Spline(Point2D<double> pt[], int np);
    Spline(double px[] , double py[] , int np);
    Spline(double pxy[] , int np);
    virtual~Spline();
    void Generate();
    void GenerateClosed();
    void MatrixSolve(double B[]);
    int GetCurveCount(double fuzzy);
    int GetClosedCurveCount(double fuzzy);
    void GetCurve(Point2D<double> points[], double fuzzy,int& PointCount);
    void GetCurveByPointNum(Point2D<double> points[], int insertNum,int& PointCount);
    void GetClosedCurve(Point2D<double> points[], double fuzzy, int& PointCount);
    void GetClosedTempCurve(Point2D<double> points[], double fuzzy, int& PointCount);
    void GetClosedCurveByPointNum(Point2D<double> points[], int insertNum, int& PointCount);
protected:
    double* Px;
    double* Py;
    double* Ax;
    double* Ay;
    double* Bx;
    double* By;
    double* Cx;
    double* Cy;
    double*  k;
    double*  Mat[3];
    int  NP;
};

class SGIS_EXPORT SplineFunction
{
public:
    SplineFunction();
    virtual~SplineFunction();
    bool Add(double x, double y);
    int GetSize();
    double GetX(int nIndex);
    double GetY(int nIndex);
    bool CreateSpline(bool hQuality = true);
    double GetSplineY(double x, bool sampleBorder = true);
protected:
    OrderArray<double>xs;
    vector<double> ys;
    OrderArray<double>ixs;
    vector<double>iys;
};
};


#endif // SPLINE_H
