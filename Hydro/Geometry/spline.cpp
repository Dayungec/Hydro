#include "spline.h"
#include "polyline.h"
namespace SGIS {

Curve::Curve(){

}
Curve::Curve(double ax,double ay,double bx,double by,double cx,double cy,int ndiv)
{
    Ax = ax;
    Ay = ay;
    Bx = bx;
    By = by;
    Cx = cx;
    Cy = cy;
    Ndiv = ndiv;
}
Curve::Curve(double ax,double ay,double bx,double by,double cx,double cy,double fuzzy)
{
    Ax = ax;
    Ay = ay;
    Bx = bx;
    By = by;
    Cx = cx;
    Cy = cy;
    if(fuzzy<=0)
        Ndiv=1;
    else
        Ndiv = (int)(max(fabs(Ax),fabs(Ay))/fuzzy);
}

Curve::~Curve()
{
}

void Curve::PutCurveByDiv(double ax,double ay,double bx,double by,double cx,double cy,int ndiv)
{
    Ax = ax;
    Ay = ay;
    Bx = bx;
    By = by;
    Cx = cx;
    Cy = cy;
    Ndiv = ndiv;
}

void Curve::PutCurve(double ax,double ay,double bx,double by,double cx,double cy,double fuzzy)
{
    Ax = ax;
    Ay = ay;
    Bx = bx;
    By = by;
    Cx = cx;
    Cy = cy;
    if(fuzzy<=0)
        Ndiv=1;
    else
        Ndiv = (int)(max(fabs(Ax),fabs(Ay))/fuzzy);
}

int Curve::GetCount()
{
    if(Ndiv==0) return 1;
    return Ndiv;
    //return Ndiv+1;
}
void Curve::GetCurve(double x,double y, Point2D<double> points[], int& PointCount)
{
    double X,Y;
    double  t,f,g,h;
    if (Ndiv==0)
        Ndiv=1;

    X = x;
    Y=   y;
    points[PointCount].X = X;
    points[PointCount].Y = Y;
    //PointCount++;

    for(int i=1; i<=Ndiv ; i++)
    {
        t = 1.0f / (double)Ndiv * (double)i;
        f = t*t*(3.0f-2.0f*t);
        g = t*(t-1.0f)*(t-1.0f);
        h = t*t*(t-1.0f);
        X = (x + Ax*f + Bx*g + Cx*h);
        Y = (y + Ay*f + By*g + Cy*h);
        points[PointCount].X = X;
        points[PointCount].Y = Y;
        PointCount++;
    }
}
void Curve::GetCurveByDiv(double x,double y, Point2D<double> points[], int& PointCount)
{
    double X,Y;
    double  t,f,g,h;
    if (Ndiv==0)
        Ndiv=1;

    X = x;
    Y=  y;
    points[PointCount].X = X;
    points[PointCount].Y = Y;
    PointCount++;

    for(int i=1; i<Ndiv ; i++)
    {
        t = 1.0f / (double)Ndiv * (double)i;
        f = t*t*(3.0f-2.0f*t);
        g = t*(t-1.0f)*(t-1.0f);
        h = t*t*(t-1.0f);
        X = (x + Ax*f + Bx*g + Cx*h);
        Y = (y + Ay*f + By*g + Cy*h);
        points[PointCount].X = X;
        points[PointCount].Y = Y;
        PointCount++;
    }
}


Spline::Spline(Point2D<double> pt[], int np)
{
    NP = np;
    Px = new double[NP];
    Py = new double[NP];
    np=0;
    DOUBLE fX,fY,tX,tY;
    for(int i=0;i<NP ;i++)
    {
        tX=pt[i].X;
        tY=pt[i].Y;
        if(i>0)
        {
           if((fabs(tX-fX)<=0.000001)&&(fabs(tY-fY)<=0.000001)) continue;
        }
        Px[np] = tX;
        Py[np] = tY;
        fX=tX;
        fY=tY;
        np++;
    }
    NP=np;
    Ax = new double[NP];
    Ay = new double[NP];
    Bx = new double[NP];
    By = new double[NP];
    Cx = new double[NP];
    Cy = new double[NP];
    k = new double[NP];
    Mat[0] = new double[NP];
    Mat[1] = new double[NP];
    Mat[2] = new double[NP];
    for(int i=0;i<NP ;i++)
    {
        Px[i] = (double)pt[i].X;
        Py[i] = (double)pt[i].Y;
    }
}

Spline::Spline(double px[] , double py[] , int np)
{
    NP = np;
    Px = new double[NP];
    Py = new double[NP];
    np=0;
    DOUBLE fX,fY,tX,tY;
    for(int i=0;i<NP ;i++)
    {
        tX=px[i];
        tY=py[i];
        if(i>0)
        {
           if((fabs(tX-fX)<=0.000001)&&(fabs(tY-fY)<=0.000001)) continue;
        }
        Px[np] = tX;
        Py[np] = tY;
        fX=tX;
        fY=tY;
        np++;
    }
    NP = np;
    Ax = new double[NP];
    Ay = new double[NP];
    Bx = new double[NP];
    By = new double[NP];
    Cx = new double[NP];
    Cy = new double[NP];
    k = new double[NP];
    Mat[0] = new double[NP];
    Mat[1] = new double[NP];
    Mat[2] = new double[NP];
    for(int i=0;i<NP ;i++)
    {
        Px[i] = px[i];
        Py[i] = py[i];
    }
}
Spline::Spline(double pxy[] , int np)
{
    NP = np;
    Px = new double[NP];
    Py = new double[NP];
    int nIndex=0;
    np=0;
    DOUBLE fX,fY,tX,tY;
    for(int i=0;i<NP ;i++)
    {
        tX=pxy[nIndex++];
        tY=pxy[nIndex++];
        if(i>0)
        {
           if((fabs(tX-fX)<=0.000001)&&(fabs(tY-fY)<=0.000001)) continue;
        }
        Px[np] = tX;
        Py[np] = tY;
        fX=tX;
        fY=tY;
        np++;
    }
    NP = np;
    Ax = new double[NP];
    Ay = new double[NP];
    Bx = new double[NP];
    By = new double[NP];
    Cx = new double[NP];
    Cy = new double[NP];
    k = new double[NP];
    Mat[0] = new double[NP];
    Mat[1] = new double[NP];
    Mat[2] = new double[NP];
}
Spline::~Spline()
{
    delete[] Px;
    delete[] Py;
    delete[] Ax;
    delete[] Ay;
    delete[] Bx;
    delete[] By;
    delete[] Cx;
    delete[] Cy;
    delete[] k;
    delete[] Mat[0];
    delete[] Mat[1];
    delete[] Mat[2];
}
void Spline::Generate()
{
    double AMag , AMagOld;
    // vector A
    for(int i= 0 ; i<=NP-2 ; i++ )
    {
        Ax[i] = Px[i+1] - Px[i];
        Ay[i] = Py[i+1] - Py[i];
    }
    // k
    AMagOld = (double)sqrt(Ax[0]*Ax[0] + Ay[0]*Ay[0]);
    for(int i=0 ; i<=NP-3 ; i++)
    {
        AMag = (double)sqrt(Ax[i+1]*Ax[i+1] + Ay[i+1]*Ay[i+1]);
        k[i] = AMagOld / AMag;
        AMagOld = AMag;
    }
    k[NP-2] = 1.0f;
    // Matrix
    for(int i=1; i<=NP-2;i++)
    {
        Mat[0][i] = 1.0f;
        Mat[1][i] = 2.0f*k[i-1]*(1.0f + k[i-1]);
        Mat[2][i] = k[i-1]*k[i-1]*k[i];
    }
    Mat[1][0] = 2.0f;
    Mat[2][0] = k[0];
    Mat[0][NP-1] = 1.0f;
    Mat[1][NP-1] = 2.0f*k[NP-2];

    //
    for(int i=1; i<=NP-2;i++)
    {
        Bx[i] = 3.0f*(Ax[i-1] + k[i-1]*k[i-1]*Ax[i]);
        By[i] = 3.0f*(Ay[i-1] + k[i-1]*k[i-1]*Ay[i]);
    }
    Bx[0] = 3.0f*Ax[0];
    By[0] = 3.0f*Ay[0];
    Bx[NP-1] = 3.0f*Ax[NP-2];
    By[NP-1] = 3.0f*Ay[NP-2];

    //
    MatrixSolve(Bx);
    MatrixSolve(By);

    for(int i=0 ; i<=NP-2 ; i++ )
    {
        Cx[i] = k[i]*Bx[i+1];
        Cy[i] = k[i]*By[i+1];
    }
}
void Spline::GenerateClosed()
{
    double AMag , AMagOld;
    // vector A
    for(int i= 0 ; i<=NP-1 ; i++ )
    {
        if(i<NP-1)
        {
            Ax[i] = Px[i+1] - Px[i];
            Ay[i] = Py[i+1] - Py[i];
        }
        else
        {
            Ax[i] = Px[0] - Px[i];
            Ay[i] = Py[0] - Py[i];
        }
    }
    // k
    AMagOld = (double)sqrt(Ax[0]*Ax[0] + Ay[0]*Ay[0]);
    for(int i=0 ; i<=NP-1 ; i++)
    {
        if(i<NP-1)
        {
            AMag = (double)sqrt(Ax[i+1]*Ax[i+1] + Ay[i+1]*Ay[i+1]);
            k[i] = AMagOld / AMag;
            AMagOld = AMag;
        }
        else
        {
            AMag = (double)sqrt(Ax[0]*Ax[0] + Ay[0]*Ay[0]);
            k[i] = AMagOld / AMag;
            AMagOld = AMag;
        }
    }
    //k[NP-1] = 1.0f;
    // Matrix
    for(int i=1; i<=NP-1;i++)
    {
        Mat[0][i] = 1.0f;
        Mat[1][i] = 2.0f*k[i-1]*(1.0f + k[i-1]);
        Mat[2][i] = k[i-1]*k[i-1]*k[i];
    }
    Mat[1][0] = 2.0f;
    Mat[2][0] = k[0];
    Mat[0][NP-1] = 1.0f;
    Mat[1][NP-1] = 2.0f*k[NP-2];
    //
    for(int i=1; i<=NP-1;i++)
    {
        Bx[i] = 3.0f*(Ax[i-1] + k[i-1]*k[i-1]*Ax[i]);
        By[i] = 3.0f*(Ay[i-1] + k[i-1]*k[i-1]*Ay[i]);
    }
    Bx[0] = 3.0f*Ax[0];
    By[0] = 3.0f*Ay[0];
    Bx[NP-1] = 3.0f*Ax[NP-2];
    By[NP-1] = 3.0f*Ay[NP-2];
    //
    MatrixSolve(Bx);
    MatrixSolve(By);
    for(int i=0 ; i<=NP-1 ; i++ )
    {
        if(i<NP-1)
        {
            Cx[i] = k[i]*Bx[i+1];
            Cy[i] = k[i]*By[i+1];
        }
        else
        {
            Cx[i] = k[i]*Bx[0];
            Cy[i] = k[i]*By[0];
        }
    }
}
void Spline::MatrixSolve(double B[])
{
    double* Work = new double[NP];
    double* WorkB = new double[NP];
    for(int i=0;i<=NP-1;i++)
    {
        Work[i] = B[i] / Mat[1][i];
        WorkB[i] = Work[i];
    }

    for(int j=0 ; j<10 ; j++)
    { ///  need convergence judge
        Work[0] = (B[0] - Mat[2][0]*WorkB[1])/Mat[1][0];
        for(int i=1; i<NP-1 ; i++ )
        {
            Work[i] = (B[i]-Mat[0][i]*WorkB[i-1]-Mat[2][i]*WorkB[i+1])
                        /Mat[1][i];
        }
        Work[NP-1] = (B[NP-1] - Mat[0][NP-1]*WorkB[NP-2])/Mat[1][NP-1];

        for(int i=0 ; i<=NP-1 ; i++ )
        {
            WorkB[i] = Work[i];
        }
    }
    for(int i=0 ; i<=NP-1 ; i++ )
    {
        B[i] = Work[i];
    }
    delete[] Work;
    delete[] WorkB;
}
int Spline::GetCurveCount(double fuzzy)
{
    Curve c;
    int count = 0;
    for(int i=0; i<NP-1 ; i++)
    {
        c.PutCurve(Ax[i],Ay[i],Bx[i],By[i],Cx[i],Cy[i],fuzzy);
        count += c.GetCount();
    }
    return count;
}
int Spline::GetClosedCurveCount(double fuzzy)
{
    Curve c;
    int count = 0;
    for(int i=0; i<NP ; i++)
    {
        c.PutCurve(Ax[i],Ay[i],Bx[i],By[i],Cx[i],Cy[i],fuzzy);
        count += c.GetCount();
    }
    return count;
}
void Spline::GetCurve(Point2D<double> points[], double fuzzy,int& PointCount)
{
    Curve c;
    for(int i=0; i<NP-1 ; i++)
    {
        c.PutCurve(Ax[i],Ay[i],Bx[i],By[i],Cx[i],Cy[i],fuzzy);
        c.GetCurve(Px[i],Py[i], points, PointCount);
    }
}
void Spline::GetCurveByPointNum(Point2D<double> points[], int insertNum,int& PointCount)
{
    Curve c;
    for(int i=0; i<NP-1 ; i++)
    {
        c.PutCurveByDiv(Ax[i],Ay[i],Bx[i],By[i],Cx[i],Cy[i],insertNum+1);
        c.GetCurveByDiv(Px[i],Py[i], points, PointCount);
    }
}
void Spline::GetClosedCurve(Point2D<double> points[], double fuzzy, int& PointCount)
{
    Curve c;
    for(int i=0; i<NP; i++)
    {
        c.PutCurve(Ax[i],Ay[i],Bx[i],By[i],Cx[i],Cy[i],fuzzy);
        c.GetCurve(Px[i],Py[i], points, PointCount);
    }
}

void Spline::GetClosedTempCurve(Point2D<double> points[], double fuzzy, int& PointCount){
    Curve c;
    for(int i=0; i<NP-1; i++)
    {
        c.PutCurve(Ax[i],Ay[i],Bx[i],By[i],Cx[i],Cy[i],fuzzy);
        c.GetCurve(Px[i],Py[i], points, PointCount);
    }
}

void Spline::GetClosedCurveByPointNum(Point2D<double> points[], int insertNum, int& PointCount)
{
    Curve c;
    for(int i=0; i<NP; i++)
    {
        c.PutCurveByDiv(Ax[i],Ay[i],Bx[i],By[i],Cx[i],Cy[i],insertNum+1);
        c.GetCurveByDiv(Px[i],Py[i], points, PointCount);
    }
}


SplineFunction::SplineFunction()
{

}

SplineFunction::~SplineFunction()
{

}

bool SplineFunction::Add(double x, double y)
{
    LONG pos=xs.Find(x);
    if (pos >= 0) return false;
    pos=xs.Add(x);
    ys.insert(std::begin(ys) + pos, y);
    return true;
}

int SplineFunction::GetSize() {
    return ys.size();
}

double SplineFunction::GetX(int nIndex)
{
    return xs[nIndex];
}
double SplineFunction::GetY(int nIndex)
{
    return ys[nIndex];
}

bool SplineFunction::CreateSpline(bool hQuality)
{
    ixs.Clear();
    iys.clear();
    LONG count = ys.size();
    if (count < 3) return false;
    AutoPtr<CurvePolyline>curve(new CurvePolyline());
    AutoPtr<Points>points(new Points());
    points->SetSize(count);
    DOUBLE*XYs=points->GetXYs();
    LONG nIndex = 0;
    double x;
    for (int k = 0; k < count; k++){
        XYs[nIndex] = xs[k];
        XYs[nIndex + 1] = ys[k];
        nIndex += 2;
    }
    AutoPtr<Path>path(points);
    curve->Add(path);
    AutoPtr<Polyline>poly=curve->ConvertToDefaultPolyline(hQuality);
    AutoPtr<Path>tPath=poly->GetItem(0);
    AutoPtr<Points>tpts(tPath);
    LONG ptsCount=tpts->GetSize();
    for (int k = 0; k < ptsCount; k++) {
        double x, y;
        tpts->GetItem(k, x, y);
        int npos=ixs.Add(x);
        if (npos >= 0) iys.insert(std::begin(iys) + npos,y);
    }
    return true;
}

double SplineFunction::GetSplineY(double x, bool sampleBorder)
{
    if (ixs.GetSize()==0 ){
        int Count = ys.size();
        if (Count == 0) return 0;
        if (Count == 1) return ys[0];
        if (Count == 2){
            double fx, tx,fy,ty;
            fx = xs[0];
            tx = xs[1];
            fy = ys[0];
            ty = ys[1];
            if (sampleBorder) return (ty - fy) / (tx - fx) * (x - fx) + fy;
            if (x <= fx) return fy;
            if (x >= tx) return ty;
            return (ty - fy) / (tx - fx) * (x - fx) + fy;
        }
        return 0;
    }
    else
    {
        LONG Count = xs.GetSize();
        int fromIndex = Count - 2;
        int toIndex = Count - 1;
        if (ys.size() < 3) return 0;
        double xv = 0, yv = 0;
        if (!sampleBorder){
            xv = xs[0]; yv =ys[0];
            if (x <= xv) return yv;
            xv = xs[Count - 1]; yv = ys[Count - 1];
            if (x >= xv) return yv;
        }
        int fpos = ixs.FindOrder(x);
        if ((fpos >0)&&(fpos < ixs.GetSize() - 1)) {
            fromIndex = fpos-1;
            toIndex = fpos;
        }
        else if (fpos==0) {
            fromIndex = 0;
            toIndex = 1;
        }
        else {
            fromIndex = ixs.GetSize() - 2;
            toIndex = ixs.GetSize() - 1;
        }
        double fx = 0, fy = 0;
        double tx = 0, ty = 0;
        fx = ixs[fromIndex]; fy = iys[fromIndex];
        tx = ixs[toIndex]; ty = iys[toIndex];
        return (ty - fy) / (tx - fx) * (x - fx) + fy;
    }
}
}

