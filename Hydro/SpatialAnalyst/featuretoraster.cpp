#include "featuretoraster.h"
#include "Base/classfactory.h"
#include "math.h"
#include "Base/StringHelper.h"
#include "Datasources/featureclasstool.h"
#include <QString>

namespace SGIS{

REGISTER(FeatureToRaster)

VectorToGrid::VectorToGrid(){

}

VectorToGrid::~VectorToGrid(){
    if(ptArray!=nullptr) delete[]ptArray;
}

RasterDesp VectorToGrid::GetRasterDesp(){
    return rasterDesp;
}

void VectorToGrid::SetRasterDesp(RasterDesp desp){
    rasterDesp=desp;
    DifFuzzy=desp.xCellSize/1000;
    if(DifFuzzy>desp.yCellSize/1000) DifFuzzy=desp.yCellSize/1000;
}

long VectorToGrid::RegionNumber(int regionx,int regiony){
    return regiony*rasterDesp.cols+regionx;
}
int VectorToGrid::RegionX(double X){
    int x=(X-FullExtent.Left)/rasterDesp.xCellSize;
    if(x<0)
       x=0;
    else if(x>=rasterDesp.cols)
       x=rasterDesp.cols-1;
    return x;
}
int VectorToGrid::RegionY(double Y){
    int y=(FullExtent.Top-Y)/rasterDesp.yCellSize;
    if(y<0)
        y=0;
    else if(y>=rasterDesp.rows)
        y=rasterDesp.rows-1;
    return y;
}
int VectorToGrid::RealRegionX(double X){
    return (X-FullExtent.Left)/rasterDesp.xCellSize;
}
int VectorToGrid::RealRegionY(double Y){
    return (FullExtent.Top-Y)/rasterDesp.yCellSize;
}

void VectorToGrid::ConvertPoint(Point*pt,BYTE*data){
    int rx=RealRegionX(pt->X());
    if((rx<0)||(rx>=rasterDesp.cols)) return;
    int ry=RealRegionY(pt->Y());
    if((ry<0)||(ry>=rasterDesp.rows)) return;
    data[rx+ry*rasterDesp.cols]=2;
}

void VectorToGrid::ConvertPoints(Points*pts,BYTE*data){
    int ptCount=pts->GetSize();
    for(int k=0;k<ptCount;k++){
        double X,Y;
        pts->GetItem(k,X,Y);
        int rx=RealRegionX(X);
        if((rx<0)||(rx>=rasterDesp.cols)) return;
        int ry=RealRegionY(Y);
        if((ry<0)||(ry>=rasterDesp.rows)) return;
        data[rx+ry*rasterDesp.cols]=2;
    }
}

void VectorToGrid::ConvertSegment(DOUBLE X1,DOUBLE Y1,DOUBLE X2,DOUBLE Y2,BYTE*data){
    if((X1==X2)&&(Y1==Y2))
    {
        int rx,ry;
        rx=RealRegionX(X1);
        if((rx<0)||(rx>=rasterDesp.cols)) return;
        ry=RealRegionY(Y1);
        if((ry<0)||(ry>=rasterDesp.rows)) return;
        long rn=RegionNumber(rx,ry);
        data[rn]=2;
    }
    else if(X1==X2)
    {
        int ry1=RealRegionY(Y1);
        int ry2=RealRegionY(Y2);
        if((ry1<0)&&(ry2<0)) return;
        if((ry1>=rasterDesp.rows)&&(ry2>=rasterDesp.rows)) return;
        int maxr;
        maxr=max(ry1,ry2);
        if(maxr>=rasterDesp.rows) maxr=rasterDesp.rows-1;
        int rx=RealRegionX(X1);
        if((rx<0)||(rx>=rasterDesp.cols)) return;
        long rn;
        int minr=min(ry1,ry2);
        if(minr<0) minr=0;
        for(int j=minr;j<=maxr;j++)
        {
            rn=RegionNumber(rx,j);
            data[rn]=2;
        }
    }
    else if(Y1==Y2)
    {
        int rx1=RealRegionX(X1);
        int rx2=RealRegionX(X2);
        if((rx1<0)&&(rx2<0)) return;
        if((rx1>=rasterDesp.cols)&&(rx2>=rasterDesp.cols)) return;
        int maxr;
        maxr=max(rx1,rx2);
        if(maxr>=rasterDesp.cols) maxr=rasterDesp.cols-1;
        int ry=RegionY(Y1);
        if((ry<0)||(ry>=rasterDesp.rows)) return;
        long rn;
        int minr=min(rx1,rx2);
        if(minr<0) minr=0;
        for(int j=minr;j<=maxr;j++)
        {
            rn=RegionNumber(j,ry);
            data[rn]=2;
        }
    }
    else
    {
        double k=(Y2-Y1)/(X2-X1);
        int rx1=RealRegionX(X1);
        int rx2=RealRegionX(X2);
        int ry1=RealRegionY(Y1);
        int ry2=RealRegionY(Y2);
        double Ini,Dif;
        int FormerY,NextY,FormerX,NextX;
        double IntersectY,IntersectX;
        long rn;
        if((k>=0)&&(k<=1))
        {
            FormerY=ry1;
            Dif=k*rasterDesp.xCellSize;
            if(X1<=X2)
            {
                Ini=k*(FullExtent.Left+rx1*rasterDesp.xCellSize)-k*X1+Y1;
                for(int p=rx1;p<=rx2;p++)
                {
                    Ini+=Dif;
                    IntersectY=Ini;
                    NextY=RealRegionY(IntersectY);
                    if(p<0)
                    {
                        FormerY=NextY;
                        continue;
                    }
                    else if(p>=rasterDesp.cols)
                        break;
                    for(int m=FormerY;m>=NextY;m--)
                    {
                        if(m>=rasterDesp.rows)
                        {
                            m=rasterDesp.rows-1;
                            if(m<NextY) break;
                        }
                        else if((m<0)||(m<ry2))
                            break;
                        rn=RegionNumber(p,m);
                        data[rn]=2;
                    }
                    FormerY=NextY;
                }
            }
            else
            {
                Ini=k*(FullExtent.Left+rx1*rasterDesp.xCellSize+rasterDesp.xCellSize)-k*X1+Y1;
                for(int p=rx1;p>=rx2;p--)
                {
                    Ini-=Dif;
                    IntersectY=Ini;
                    NextY=RealRegionY(IntersectY);
                    if(p>=rasterDesp.cols)
                    {
                        FormerY=NextY;
                        continue;
                    }
                    else if(p<0)
                        break;
                    for(int m=FormerY;m<=NextY;m++)
                    {
                        if(m<0)
                        {
                            m=0;
                            if(m>NextY) break;
                        }
                        else if((m>=rasterDesp.rows)||(m>ry2))
                            break;
                        rn=RegionNumber(p,m);
                        data[rn]=2;
                    }
                    FormerY=NextY;
                }
            }
        }
        else if((k<0)&&(k>=-1))
        {
            FormerY=ry1;
            Dif=-k*rasterDesp.xCellSize;
            if(X1>=X2)
            {
                Ini=k*(FullExtent.Left+rx1*rasterDesp.xCellSize+rasterDesp.xCellSize)-k*X1+Y1;
                for(int p=rx1;p>=rx2;p--)
                {
                    Ini+=Dif;
                    IntersectY=Ini;
                    NextY=RealRegionY(IntersectY);
                    if(p>=rasterDesp.cols)
                    {
                        FormerY=NextY;
                        continue;
                    }
                    else if(p<0)
                        break;
                    for(int m=FormerY;m>=NextY;m--)
                    {
                        if(m>=rasterDesp.rows)
                        {
                            m=rasterDesp.rows-1;
                            if(m<NextY) break;
                        }
                        else if((m<0)||(m<ry2))
                            break;
                        rn=RegionNumber(p,m);
                        data[rn]=2;
                    }
                    FormerY=NextY;
                }
            }
            else
            {
                Ini=k*(FullExtent.Left+rx1*rasterDesp.xCellSize)-k*X1+Y1;
                for(int p=rx1;p<=rx2;p++)
                {
                    Ini-=Dif;
                    IntersectY=Ini;
                    NextY=RealRegionY(IntersectY);
                    if(p<0)
                    {
                        FormerY=NextY;
                        continue;
                    }
                    else if(p>=rasterDesp.cols)
                        break;
                    for(int m=FormerY;m<=NextY;m++)
                    {
                        if(m<0)
                        {
                            m=0;
                            if(m>NextY) break;
                        }
                        else if((m>=rasterDesp.rows)||(m>ry2))
                            break;
                        rn=RegionNumber(p,m);
                        data[rn]=2;
                    }
                    FormerY=NextY;
                }
            }
        }
        else if(k>1)
        {
            k=1/k;
            FormerX=rx1;
            Dif=k*rasterDesp.yCellSize;
            if(Y1<=Y2)
            {
                Ini=k*(FullExtent.Top-ry1*rasterDesp.yCellSize-rasterDesp.yCellSize)-k*Y1+X1;
                for(int p=ry1;p>=ry2;p--)
                {
                    Ini+=Dif;
                    IntersectX=Ini;
                    NextX=RealRegionX(IntersectX);
                    if(p<0)
                    {
                        FormerX=NextX;
                        continue;
                    }
                    else if(p>=rasterDesp.rows)
                        break;
                    for(int m=FormerX;m<=NextX;m++)
                    {
                        if(m<0)
                        {
                            m=0;
                            if(m>NextX) break;
                        }
                        else if((m>=rasterDesp.cols)||(m>rx2))
                            break;
                        rn=RegionNumber(m,p);
                        data[rn]=2;
                    }
                    FormerX=NextX;
                }
            }
            else
            {
                Ini=k*(FullExtent.Top-ry1*rasterDesp.yCellSize)-k*Y1+X1;
                for(int p=ry1;p<=ry2;p++)
                {
                    Ini-=Dif;
                    IntersectX=Ini;
                    NextX=RealRegionX(IntersectX);
                    if(p>=rasterDesp.rows)
                    {
                        FormerX=NextX;
                        continue;
                    }
                    else if(p<0)
                        break;
                    for(int m=FormerX;m>=NextX;m--)
                    {
                        if(m>=rasterDesp.cols)
                        {
                            m=rasterDesp.cols-1;
                            if(m<NextX) break;
                        }
                        else if((m<0)||(m<rx2))
                            break;
                        rn=RegionNumber(m,p);
                        data[rn]=2;
                    }
                    FormerX=NextX;
                }
            }
        }
        else if(k<-1)
        {
            k=1/k;
            FormerX=rx1;
            Dif=-k*rasterDesp.yCellSize;
            if(Y1>=Y2)
            {
                Ini=k*(FullExtent.Top-ry1*rasterDesp.yCellSize)-k*Y1+X1;
                for(int p=ry1;p<=ry2;p++)
                {
                    Ini+=Dif;
                    IntersectX=Ini;
                    NextX=RealRegionX(IntersectX);
                    if(p>=rasterDesp.rows)
                    {
                        FormerX=NextX;
                        continue;
                    }
                    else if(p<0)
                        break;
                    for(int m=FormerX;m<=NextX;m++)
                    {
                        if(m<0)
                        {
                            m=0;
                            if(m>NextX) break;
                        }
                        else if((m>=rasterDesp.cols)||(m>rx2))
                            break;
                        rn=RegionNumber(m,p);
                        data[rn]=2;
                    }
                    FormerX=NextX;
                }
            }
            else
            {
                Ini=k*(FullExtent.Top-ry1*rasterDesp.yCellSize-rasterDesp.yCellSize)-k*Y1+X1;
                for(int p=ry1;p>=ry2;p--)
                {
                    Ini-=Dif;
                    IntersectX=Ini;
                    NextX=RealRegionX(IntersectX);
                    if(p<0)
                    {
                        FormerX=NextX;
                        continue;
                    }
                    else if(p>=rasterDesp.rows)
                        break;
                    for(int m=FormerX;m>=NextX;m--)
                    {
                        if(m>=rasterDesp.cols)
                        {
                            m=rasterDesp.cols-1;
                            if(m<NextX) break;
                        }
                        else if((m<0)||(m<rx2))
                            break;
                        rn=RegionNumber(m,p);
                        data[rn]=2;
                    }
                    FormerX=NextX;
                }
            }
        }
    }
}

void VectorToGrid::ConvertSegment(Segment*seg,BYTE*data){
    DOUBLE X1,Y1,X2,Y2;
    seg->GetCoord(&X1,&Y1,&X2,&Y2);
    ConvertSegment(X1,Y1,X2,Y2,data);
}

void VectorToGrid::ConvertPolyline(Points*ply,BYTE*data){
    int count=ply->GetSize();
    if(count<2) return;
    double x1,y1,x2,y2;
    ply->GetItem(0,x1,y1);
    for(int k=1;k<count;k++){
        ply->GetItem(k,x2,y2);
        if((x1<FullExtent.Left)&&(x2<FullExtent.Left))
        {
            x1=x2;y1=y2;
            continue;
        }
        if((x1>FullExtent.Right)&&(x2>FullExtent.Right))
        {
            x1=x2;y1=y2;
            continue;
        }
        if((y1<FullExtent.Bottom)&&(y2<FullExtent.Bottom))
        {
            x1=x2;y1=y2;
            continue;
        }
        if((y1>FullExtent.Top)&&(y2>FullExtent.Top))
        {
            x1=x2;y1=y2;
            continue;
        }
        ConvertSegment(x1,y1,x2,y2,data);
        x1=x2;
        y1=y2;
    }
}

void VectorToGrid::ConvertPolyline(Polyline*ply,BYTE*data){
    long ptsCount=ply->GetSize();
    AutoPtr<Points>pts;
    for(int k=0;k<ptsCount;k++)
    {
        pts=ply->GetItem(k);
        ConvertPolyline(pts.get(),data);
    }
}

void VectorToGrid::ConvertRing(Points*pnts,BYTE*data){
    LONG ptCount=pnts->GetSize();
    if(ptCount<3) return;
    double XMin,YMin,XMax,YMax;
    AutoPtr<Envelope>ext=pnts->GetExtent();
    ext->GetCoord(&XMin,&YMax,&XMax,&YMin);
    int RegionX1=RealRegionX(XMin);
    int RegionY1=RealRegionY(YMax);
    int RegionX2=RealRegionX(XMax);
    int RegionY2=RealRegionY(YMin);
    double X1,Y1,X2,Y2;
    long RegionN;
    int rx1,rx2,ry1,ry2,j,minr;

    if(ptCount<3) return;
    pnts->GetItem(0,X1,Y1);
    rx1=RealRegionX(X1);ry1=RealRegionY(Y1);
    double InterX;
    double centery;
    double lx=FullExtent.Left+rasterDesp.xCellSize*RegionX1;
    double rx=FullExtent.Left+rasterDesp.xCellSize*(RegionX2+1);
    int leftr;
    int FormerY;
    int FormerDeri=0;
    bool IsIntersect;
    int V;
    float fV;
    for(long k=1;k<=ptCount;k++)
    {
        if(k==ptCount)
            pnts->GetItem(0,X2,Y2);
        else
            pnts->GetItem(k,X2,Y2);
        rx2=RealRegionX(X2);ry2=RealRegionY(Y2);
        if(Y1>Y2)
        {
            if((FormerDeri==1)&&(FormerY>=ry1)) ry1=FormerY+1;
            double r=(X2-X1)/(Y2-Y1);
            for(int p=ry1;p<=ry2;p++)
            {
                centery=FullExtent.Top-rasterDesp.yCellSize/2-p*rasterDesp.yCellSize;
                if((centery>Y1)||(centery<Y2)) continue;
                IsIntersect=false;
                if((centery>=Y2-DifFuzzy)&&(centery<=Y1+DifFuzzy))
                {
                    FormerY=p;
                    FormerDeri=1;
                    IsIntersect=true;
                    InterX=r*(centery-Y1)+X1;
                }
                if((IsIntersect)&&(p>=0)&&(p<=rasterDesp.rows-1))
                {
                    leftr=RealRegionX(InterX);
                    if(InterX>FullExtent.Left+rasterDesp.xCellSize/2+rasterDesp.xCellSize*leftr)
                    {
                        leftr++;
                    }
                    if(leftr>0)
                    {
                        RegionN=RegionNumber(max(RegionX1,0),p);
                        minr=min(leftr,rasterDesp.cols);
                        for(j=max(RegionX1,0);j<minr;j++)
                        {
                            fV=data[RegionN];
                            V=(int)fV-1;
                            if(V<0) V+=4;
                            data[RegionN]=V;
                            RegionN++;
                        }
                    }
                    if(leftr<0) leftr=0;
                    if(leftr<=rasterDesp.cols-1)
                    {
                        RegionN=RegionNumber(leftr,p);
                        minr=min(RegionX2,rasterDesp.cols-1);
                        for(j=leftr;j<=minr;j++)
                        {
                            fV=data[RegionN];
                            V=(int)fV+1;
                            if(V>3) V-=4;
                            data[RegionN]=V;
                            RegionN++;
                        }
                    }
                }
            }
        }
        else if(Y1<Y2)
        {
            if((FormerDeri==-1)&&(FormerY<=ry1)) ry1=FormerY-1;
            double r=(X2-X1)/(Y2-Y1);
            for(int p=ry1;p>=ry2;p--)
            {
                centery=FullExtent.Top-rasterDesp.yCellSize/2-p*rasterDesp.yCellSize;
                if((centery<Y1)||(centery>Y2)) continue;
                IsIntersect=false;
                if((centery>=Y1-DifFuzzy)&&(centery<=Y2+DifFuzzy))
                {
                    FormerY=p;
                    FormerDeri=-1;
                    IsIntersect=true;
                    InterX=r*(centery-Y1)+X1;
                }
                if((IsIntersect)&&(p>=0)&&(p<=rasterDesp.rows-1))
                {
                    leftr=RealRegionX(InterX);
                    if(InterX>FullExtent.Left+rasterDesp.xCellSize/2+rasterDesp.xCellSize*leftr)
                    {
                        leftr++;
                    }
                    if(leftr>0)
                    {
                        RegionN=RegionNumber(max(RegionX1,0),p);
                        minr=min(leftr,rasterDesp.cols);
                        for(j=max(RegionX1,0);j<minr;j++)
                        {
                            fV=data[RegionN];
                            V=(int)fV+1;
                            if(V>3) V-=4;
                            data[RegionN]=V;
                            RegionN++;
                        }
                    }
                    if(leftr<0) leftr=0;
                    if(leftr<=rasterDesp.cols-1)
                    {
                        RegionN=RegionNumber(leftr,p);
                        minr=min(RegionX2,rasterDesp.cols-1);
                        for(j=leftr;j<=minr;j++)
                        {
                            fV=data[RegionN];
                            V=(int)fV-1;
                            if(V<0) V+=4;
                            data[RegionN]=V;
                            RegionN++;
                        }
                    }
                }
            }
        }
        X1=X2;Y1=Y2;
        rx1=rx2;ry1=ry2;
    }
}

void VectorToGrid::ConvertPolygon(Polygon*poly,BYTE*data){
    long PtsCount=poly->GetSize();
    AutoPtr<Points>pts;
    for(long k=0;k<PtsCount;k++)
    {
        pts=poly->GetItem(k);
        ConvertRing(pts.get(),data);
    }
    for(long k=0;k<PtsCount;k++)
    {
        pts=poly->GetItem(k);
        ConvertPolyline(pts.get(),data);
    }
}

void VectorToGrid::ConvertMultiPolygon(MultiPolygon*poly,BYTE*data){
    long PtsCount;
    PtsCount=poly->GetSize();
    AutoPtr<Polygon> pts;
    LONG Size=rasterDesp.cols*rasterDesp.rows;
    if(formerSize!=Size){
        if(ptArray!=nullptr) delete[]ptArray;
        ptArray=new BYTE[Size];
        formerSize=Size;
    }
    else{
        memset(ptArray,0,Size);
    }
    for(long k=0;k<PtsCount;k++)
    {
        pts=poly->GetItem(k);
        ConvertPolygon(pts.get(),ptArray);
        if(k==0)
            memcpy(data,ptArray,rasterDesp.cols*rasterDesp.rows);
        else
        {
            float fv;
            for(int p=Size-1;p>=0;p--)
            {
                fv=ptArray[p];
                if(2==(int)fv) data[p]=2;
            }
        }
        if(k<PtsCount-1) {
            for(int k=Size-1;k>=0;k--) ptArray[k]=0;
        }
    }
}
void VectorToGrid::ConvertGeometryCollection(GeometryCollection*poly,BYTE*data){
    long PtsCount=poly->GetSize();
    LONG Size=rasterDesp.cols*rasterDesp.rows;
    AutoPtr<Geometry>pts;
    if(formerSize!=Size){
        if(ptArray!=nullptr) delete[]ptArray;
        ptArray=new BYTE[Size];
        formerSize=Size;
    }
    else{
        memset(ptArray,0,Size);
    }
    for(long k=0;k<PtsCount;k++)
    {
        poly->GetItem(k);
        ConvertGeometry(pts.get(),ptArray);
        if(k==0)
            memcpy(data,ptArray,Size);
        else
        {
            float fv;
            for(int p=Size-1;p>=0;p--)
            {
                fv=ptArray[p];
                if(2==(int)fv)
                {
                    data[p]=2;
                }
            }
        }
        if(k<PtsCount-1) {
            for(int k=Size-1;k>=0;k--){
                ptArray[k]=0;
            }

        }
    }
}
void VectorToGrid::ConvertGeometry(Geometry*geo,BYTE*data){
    GeometryType gType=geo->GetType();
    switch(gType)
    {
    case gtPoint:
    {
        ConvertPoint((Point*)geo,data);
        break;
    }
    case gtPoints:
    {
        ConvertPoints((Points*)geo,data);
        break;
    }
    case gtSegment:
    {
        ConvertSegment((Segment*)geo,data);
        break;
    }
    case gtEnvelope:
    {
        AutoPtr<Polygon>poly=((Envelope*)geo)->ConvertToPolygon();
        ConvertPolygon(poly.get(),data);
        break;
    }
    case gtEllipse:
    {
        AutoPtr<Polygon>poly=((Ellipse*)geo)->ConvertToPolygon(true);
        ConvertPolygon(poly.get(),data);
        break;
    }
    case gtPolyline:
    {
        AutoPtr<CurvePolyline>cpoly=AutoPtr<Geometry>(geo,true);
        if(cpoly!=nullptr)
        {
            AutoPtr<Polyline>poly=cpoly->ConvertToDefaultPolyline(true);
            ConvertPolyline(poly.get(),data);
        }
        else
        {
            ConvertPolyline((Polyline*)geo,data);
        }
        break;
    }
    case gtPolygon:
    {
        AutoPtr<CurvePolygon>cpoly=AutoPtr<Geometry>(geo,true);
        if(cpoly!=NULL)
        {
            AutoPtr<Polygon>poly=cpoly->ConvertToDefaultPolygon(true);
            ConvertPolygon(poly.get(),data);
        }
        else
        {
            ConvertPolygon((Polygon*)geo,data);
        }
        break;
    }
    case gtMultiPolygon:
    {
        MultiPolygon* pDef=(MultiPolygon*)geo;
        LONG subCount=pDef->GetSize();
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Polygon>poly=pDef->GetItem(k);
            AutoPtr<CurvePolygon>cpoly=poly;
            if(cpoly!=nullptr)
            {
                AutoPtr<Polygon>npoly=cpoly->ConvertToDefaultPolygon(true);
                ConvertPolygon(npoly.get(),data);
            }
            else
            {
                ConvertPolygon(poly.get(),data);
            }
        }
        break;
    }
    case gtCollection:
    {
        GeometryCollection* pDef=(GeometryCollection*)geo;
        LONG subCount=pDef->GetSize();
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>poly=pDef->GetItem(k);
            ConvertGeometry(poly.get(),data);
        }
        break;
    }
    }
}

bool VectorToGrid::Convert(Geometry*pRefGeo,Envelope*CurrentBoundry,BYTE*data,bool reset){
    if((rasterDesp.cols==0)||(rasterDesp.rows==0)||(rasterDesp.xCellSize==0)||(rasterDesp.yCellSize==0)) return false;
    DOUBLE X,Y;
    X=rasterDesp.left;
    Y=rasterDesp.top;
    FullExtent.Left=X;
    FullExtent.Top=Y;
    FullExtent.Right=rasterDesp.xCellSize*rasterDesp.cols+X;
    FullExtent.Bottom=Y-rasterDesp.yCellSize*rasterDesp.rows;
    AutoPtr<Envelope>ext=pRefGeo->GetExtent();
    DOUBLE XMin,YMin,XMax,YMax;
    ext->GetCoord(&XMin,&YMax,&XMax,&YMin);
    if(!FullExtent.IntersectRect(Rect2D<double>(XMin,YMax,XMax,YMin))) return false;
    int rx1=RegionX(XMin);
    int ry1=RegionY(YMax);
    int rx2=RegionX(XMax);
    int ry2=RegionY(YMin);
    ResultBoundry=Rect2D<int>(rx1,ry1,rx2,ry2);
    if(reset) memset(data,0,rasterDesp.cols*rasterDesp.rows);
    ConvertGeometry(pRefGeo,data);
    CurrentBoundry->PutCoord(ResultBoundry.Left,ResultBoundry.Top,ResultBoundry.Right,ResultBoundry.Bottom);
    return true;
}

FeatureToRaster::FeatureToRaster()
{
   nodata=-32768;
}

FeatureToRaster::~FeatureToRaster()
{

}
FLOAT&FeatureToRaster::OutputNoData(){
    return nodata;
}

AutoPtr<RasterBand>FeatureToRaster::Execute(FeatureClass*pfc,string NumericField,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    DOUBLE X,Y;
    AutoPtr<Point>leftTop=pEnvi->GetLeftTop();
    leftTop->GetCoord(&X,&Y);
    FullExtent.Left=X;
    FullExtent.Top=Y;
    FullExtent.Right=pEnvi->XCellSize()*pEnvi->Cols()+X;
    FullExtent.Bottom=Y-pEnvi->YCellSize()*pEnvi->Rows();
    AutoPtr<TableDesc>pDesc=pfc->GetTableDesc();
    LONG FieldIndex=pDesc->FindField(NumericField);
    VectorFieldType fieldtype;
    bool IsFID=false;
    if(FieldIndex==-1)
    {
        string sField=NumericField;
        StringHelper::TrimToUpper(sField);
        if(sField!="FID")
        {
            CallBackHelper::SendMessage(callBack,"非法字段");
            return nullptr;
        }
        fieldtype=vftInteger;
        IsFID=true;
    }
    else
    {
        VectorFieldType fieldtype=pDesc->GetFieldType(FieldIndex);
        if((fieldtype!=vftInteger)&&(fieldtype!=vftReal))
        {
            CallBackHelper::SendMessage(callBack,"非法字段类型");
            return nullptr;
        }
    }
    AutoPtr<SpatialReference>psp=pfc->GetSpatialReference();
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation(psp,pEnvi->GetSpatialReference()));
    pTrans->BeginTransform();
    bool bSuc;
    BYTE*maskData=GetMaskData(callBack,signal,bSuc);
    if(!bSuc) return nullptr;
    RasterDataType dt=rdtByte;
    AutoPtr<SelectionSet>pSet=pfc->GetSelectionSet();
    LONG SelCount=pSet->GetSize();
    long Count=pfc->GetFeatureCount();
    if(IsFID)
    {
        if(Count<32768)
            dt=rdtInt16;
        else
            dt=rdtInt32;
    }
    else
    {
        if(fieldtype==vftInteger)
        {
            double min,max,mean,sta;
            AutoPtr<FeatureClassTool>pTool(new FeatureClassTool(pfc));
            pTool->ComputeStatistics(NumericField,&min,&max,&mean,&sta);
            if(min>=0)
            {
                if(max<65536)
                    dt=rdtUInt16;
                else
                    dt=rdtUInt32;
            }
            else
            {
                if((min>-32768)&&(max<32767))
                    dt=rdtInt16;
                else
                    dt=rdtInt32;
            }
        }
        else
            dt=rdtFloat32;
    }
    AutoPtr<RasterDataset>pDataset=target->CreateRasterDataset(pEnvi.get(),dt,1,nodata);
    if(pDataset==nullptr) return nullptr;
    AutoPtr<RasterBand>pTarget=pDataset->GetRasterBand(0);
    LONG cols=pEnvi->Cols();
    LONG rows=pEnvi->Rows();
    double xCellSize=pEnvi->XCellSize();
    double yCellSize=pEnvi->YCellSize();
    FLOAT*rowData=new FLOAT[cols];
    for(int k=0;k<cols;k++) rowData[k]=nodata;
    CallBackHelper::BeginProgress(callBack,"初始化栅格数据");
    for(int k=0;k<rows;k++)
    {
        pTarget->SaveBlockData(0,k,cols,1,rowData);
        if(this->DetectExitSignal(callBack,signal)){
            delete []rowData;
            return nullptr;
        }
    }
    double bv;
    AutoPtr<VectorToGrid>pConvert(new VectorToGrid());
    int rx1,ry1,rx2,ry2;
    Rect2D<double>gext;
    AutoPtr<MemDataArray>pArray(new MemDataArray());
    pArray->Initialize(cols*rows,rdtByte);
    BYTE*rdata=pArray->GetValues();
    AutoPtr<Envelope>CurrentBoundry(new Envelope());
    int gcols,grows;
    float fv;
    long Pos,Posi;
    CallBackHelper::BeginProgress(callBack,"栅格化");
    LONG FID;
    AutoPtr<Point>lt(new Point());
    AutoPtr<Envelope>faEnv(new Envelope());
    for(int k=0;k<Count;k++)
    {
        if(SelCount>0){
            FID=pfc->GetFeatureID(k);
            if(!pSet->FindFeatureID(FID)) continue;
        }
        if(!pfc->GetFeatureEnvelope(k,faEnv.get())) continue;
        faEnv->Project(pTrans.get());
        faEnv->GetCoord(&gext.Left,&gext.Top,&gext.Right,&gext.Bottom);
        if(!gext.IntersectRect(FullExtent)) continue;
        AutoPtr<Geometry>pGeo=pfc->GetFeature(k);
        if(pGeo==nullptr) continue;
        pGeo->Project(pTrans.get());
        AutoPtr<Envelope>rt=pGeo->GetExtent();
        rt->GetCoord(&gext.Left,&gext.Top,&gext.Right,&gext.Bottom);
        gext=FullExtent.Intersect(gext);
        rx1=(gext.Left-X)/xCellSize;
        if(rx1<0) rx1=0;
        ry1=(Y-gext.Top)/yCellSize;
        if(ry1<0) ry1=0;
        rx2=(gext.Right-X)/xCellSize;
        if(rx2>=cols) rx2=cols-1;
        ry2=(Y-gext.Bottom)/yCellSize;
        if(ry2>=rows) ry2=rows-1;
        lt->PutCoord(rx1*xCellSize+X,Y-ry1*yCellSize);
        gcols=rx2-rx1+1;
        grows=ry2-ry1+1;
        pConvert->SetRasterDesp(RasterDesp(lt->X(),lt->Y(),gcols,grows,xCellSize,yCellSize));
        if(!pConvert->Convert(pGeo.get(),CurrentBoundry.get(),rdata,true)) continue;
        Pos=0;
        if(IsFID)
        {
            FID=pfc->GetFeatureID(k);
            bv=FID;
        }
        else
            bv=pfc->GetFieldValueAsDouble(k,FieldIndex);
        for(int i=0;i<grows;i++)
        {

            pTarget->GetBlockData(rx1,i+ry1,gcols,1,gcols,1,rowData);
            if(maskData==nullptr){
                for(int j=0;j<gcols;j++)
                {
                    fv=rdata[Pos];
                    if(2==(int)fv) rowData[j]=bv;
                    Pos++;
                }
            }
            else{
                Posi=(i+ry1)*cols+rx1;
                for(int j=0;j<gcols;j++)
                {
                    fv=rdata[Pos];
                    if(2==(int)fv){
                       if(maskData[Posi]!=0) rowData[j]=bv;
                    }
                    Pos++;
                    Posi++;
                }
            }
            pTarget->SaveBlockData(rx1,i+ry1,gcols,1,rowData);
        }
        CallBackHelper::SendProgress(callBack,(float)k/Count*100);
        if(this->DetectExitSignal(callBack,signal)){
            delete []rowData;
            return nullptr;
        }
    }
    delete []rowData;
    CallBackHelper::EndProgress(callBack);
    return pTarget;
}

AutoPtr<RasterBand>FeatureToRaster::ConvertMask(FeatureClass*pfc,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    DOUBLE X,Y;
    AutoPtr<Point>leftTop=pEnvi->GetLeftTop();
    leftTop->GetCoord(&X,&Y);
    FullExtent.Left=X;
    FullExtent.Top=Y;
    FullExtent.Right=pEnvi->XCellSize()*pEnvi->Cols()+X;
    FullExtent.Bottom=Y-pEnvi->YCellSize()*pEnvi->Rows();
    AutoPtr<SpatialReference>psp=pfc->GetSpatialReference();
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation(psp,pEnvi->GetSpatialReference()));
    pTrans->BeginTransform();
    RasterDataType dt=rdtByte;
    AutoPtr<SelectionSet>pSet=pfc->GetSelectionSet();
    LONG SelCount=pSet->GetSize();
    long Count=pfc->GetFeatureCount();
    AutoPtr<MemRasterTarget>target(new MemRasterTarget());
    AutoPtr<RasterDataset>pDataset=target->CreateRasterDataset(pEnvi.get(),dt,1,0);
    if(pDataset==nullptr) return nullptr;
    AutoPtr<MemRasterBand>pTarget=pDataset->GetRasterBand(0);
    LONG cols=pEnvi->Cols();
    LONG rows=pEnvi->Rows();
    double xCellSize=pEnvi->XCellSize();
    double yCellSize=pEnvi->YCellSize();
    BYTE*rowData=pTarget->GetRawData()->GetValues();
    for(int k=cols*rows-1;k>=0;k--) rowData[k]=0;
    AutoPtr<VectorToGrid>pConvert(new VectorToGrid());
    int rx1,ry1,rx2,ry2;
    Rect2D<double>gext;
    AutoPtr<MemDataArray>pArray(new MemDataArray());
    pArray->Initialize(cols*rows,rdtByte);
    BYTE*rdata=pArray->GetValues();
    AutoPtr<Envelope>CurrentBoundry(new Envelope());
    int gcols,grows;
    float fv;
    long Pos,Posi;
    LONG FID;
    AutoPtr<Point>lt(new Point());
    AutoPtr<Envelope>faEnv(new Envelope());
    for(int k=0;k<Count;k++)
    {
        if(SelCount>0){
            FID=pfc->GetFeatureID(k);
            if(!pSet->FindFeatureID(FID)) continue;
        }
        if(!pfc->GetFeatureEnvelope(k,faEnv.get())) continue;
        faEnv->Project(pTrans.get());
        faEnv->GetCoord(&gext.Left,&gext.Top,&gext.Right,&gext.Bottom);
        if(!gext.IntersectRect(FullExtent)) continue;
        AutoPtr<Geometry>pGeo=pfc->GetFeature(k);
        if(pGeo==nullptr) continue;
        pGeo->Project(pTrans.get());
        AutoPtr<Envelope>rt=pGeo->GetExtent();
        rt->GetCoord(&gext.Left,&gext.Top,&gext.Right,&gext.Bottom);
        gext=FullExtent.Intersect(gext);
        rx1=(gext.Left-X)/xCellSize;
        if(rx1<0) rx1=0;
        ry1=(Y-gext.Top)/yCellSize;
        if(ry1<0) ry1=0;
        rx2=(gext.Right-X)/xCellSize;
        if(rx2>=cols) rx2=cols-1;
        ry2=(Y-gext.Bottom)/yCellSize;
        if(ry2>=rows) ry2=rows-1;
        lt->PutCoord(rx1*xCellSize+X,Y-ry1*yCellSize);
        gcols=rx2-rx1+1;
        grows=ry2-ry1+1;
        pConvert->SetRasterDesp(RasterDesp(lt->X(),lt->Y(),gcols,grows,xCellSize,yCellSize));
        if(!pConvert->Convert(pGeo.get(),CurrentBoundry.get(),rdata,true)) continue;
        Pos=0;
        for(int i=0;i<grows;i++)
        {
            Posi=(i+ry1)*cols+rx1;
            for(int j=0;j<gcols;j++)
            {
                fv=rdata[Pos];
                if(2==(int)fv) rowData[Posi]=1;
                Pos++;
                Posi++;
            }
        }
        if(this->DetectExitSignal(nullptr,signal)){
            delete []rowData;
            return nullptr;
        }
    }
    return pTarget;
}
AutoPtr<RasterBand>FeatureToRaster::ConvertGeometry(Geometry*geo,SpatialReference*psp,RasterTarget*target){
    if(!CheckValid()) return nullptr;
    DOUBLE X,Y;
    AutoPtr<Point>leftTop=pEnvi->GetLeftTop();
    leftTop->GetCoord(&X,&Y);
    FullExtent.Left=X;
    FullExtent.Top=Y;
    FullExtent.Right=pEnvi->XCellSize()*pEnvi->Cols()+X;
    FullExtent.Bottom=Y-pEnvi->YCellSize()*pEnvi->Rows();
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation(AutoPtr<SpatialReference>(psp,true),pEnvi->GetSpatialReference()));
    pTrans->BeginTransform();
    bool bSuc;
    BYTE*maskData=GetMaskData(nullptr,nullptr,bSuc);
    if(!bSuc) return nullptr;
    AutoPtr<RasterDataset>pDataset=target->CreateRasterDataset(pEnvi.get(),rdtByte,1,0);
    if(pDataset==nullptr) return nullptr;
    AutoPtr<RasterBand>pTarget=pDataset->GetRasterBand(0);
    LONG cols=pEnvi->Cols();
    LONG rows=pEnvi->Rows();
    double xCellSize=pEnvi->XCellSize();
    double yCellSize=pEnvi->YCellSize();
    FLOAT*rowData=new FLOAT[cols];
    for(int k=0;k<cols;k++) rowData[k]=nodata;
    for(int k=0;k<rows;k++)
    {
        pTarget->SaveBlockData(0,k,cols,1,rowData);
    }
    Rect2D<double>gext;
    AutoPtr<Point>lt(new Point());
    AutoPtr<Envelope>faEnv=geo->GetExtent();
    faEnv->Project(pTrans.get());
    faEnv->GetCoord(&gext.Left,&gext.Top,&gext.Right,&gext.Bottom);
    if(!gext.IntersectRect(FullExtent)){
        return pTarget;
    }
    AutoPtr<VectorToGrid>pConvert(new VectorToGrid());
    int rx1,ry1,rx2,ry2;

    AutoPtr<MemDataArray>pArray(new MemDataArray());
    pArray->Initialize(cols*rows,rdtByte);
    BYTE*rdata=pArray->GetValues();
    AutoPtr<Envelope>CurrentBoundry(new Envelope());
    int gcols,grows;
    float fv;
    long Pos,Posi;
    AutoPtr<Geometry>pGeo=geo->Clone();
    pGeo->Project(pTrans.get());
    AutoPtr<Envelope>rt=pGeo->GetExtent();
    rt->GetCoord(&gext.Left,&gext.Top,&gext.Right,&gext.Bottom);
    gext=FullExtent.Intersect(gext);
    rx1=(gext.Left-X)/xCellSize;
    if(rx1<0) rx1=0;
    ry1=(Y-gext.Top)/yCellSize;
    if(ry1<0) ry1=0;
    rx2=(gext.Right-X)/xCellSize;
    if(rx2>=cols) rx2=cols-1;
    ry2=(Y-gext.Bottom)/yCellSize;
    if(ry2>=rows) ry2=rows-1;
    lt->PutCoord(rx1*xCellSize+X,Y-ry1*yCellSize);
    gcols=rx2-rx1+1;
    grows=ry2-ry1+1;
    pConvert->SetRasterDesp(RasterDesp(lt->X(),lt->Y(),gcols,grows,xCellSize,yCellSize));
    if(!pConvert->Convert(pGeo.get(),CurrentBoundry.get(),rdata,true)){
        delete []rowData;
        return pTarget;
    }
    Pos=0;
    for(int i=0;i<grows;i++)
    {
        pTarget->GetBlockData(rx1,i+ry1,gcols,1,gcols,1,rowData);
        if(maskData==nullptr){
            for(int j=0;j<gcols;j++)
            {
                fv=rdata[Pos];
                if(2==(int)fv) rowData[j]=1;
                Pos++;
            }
        }
        else{
            Posi=(i+ry1)*cols+rx1;
            for(int j=0;j<gcols;j++)
            {
                fv=rdata[Pos];
                if(2==(int)fv){
                    if(maskData[Posi]!=0) rowData[j]=1;
                }
                Pos++;
                Posi++;
            }
        }
        pTarget->SaveBlockData(rx1,i+ry1,gcols,1,rowData);
    }
    delete []rowData;
    return pTarget;
}

bool FeatureToRaster::ConvertGeometry(Geometry*geo,SpatialReference*psp,RasterBand*band){
    if(!CheckValid()) return false;
    DOUBLE X,Y;
    AutoPtr<Point>leftTop=pEnvi->GetLeftTop();
    leftTop->GetCoord(&X,&Y);
    RasterDesp desp=band->GetRasterDesp();
    if((X!=desp.left)||(Y!=desp.top)) return false;
    if((band->GetCols()!=pEnvi->Cols())||(band->GetRows()!=pEnvi->Rows())) return false;
    if((desp.xCellSize!=pEnvi->XCellSize())||(desp.yCellSize!=pEnvi->YCellSize())) return false;
    FullExtent.Left=X;
    FullExtent.Top=Y;
    FullExtent.Right=pEnvi->XCellSize()*pEnvi->Cols()+X;
    FullExtent.Bottom=Y-pEnvi->YCellSize()*pEnvi->Rows();
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation(AutoPtr<SpatialReference>(psp,true),pEnvi->GetSpatialReference()));
    pTrans->BeginTransform();
    bool bSuc;
    BYTE*maskData=GetMaskData(nullptr,nullptr,bSuc);
    if(!bSuc) return false;
    LONG cols=pEnvi->Cols();
    LONG rows=pEnvi->Rows();
    double xCellSize=pEnvi->XCellSize();
    double yCellSize=pEnvi->YCellSize();
    AutoPtr<MemRasterBand>pTarget=AutoPtr<RasterBand>(band,true);
    bool memConvert=false;
    if(pTarget!=nullptr){
        if(pTarget->GetDataType()==rdtByte) memConvert=true;
    }
    if(memConvert){
        AutoPtr<MemDataArray>pArray=pTarget->GetRawData();
        BYTE*rdata=pArray->GetValues();
        for(int k=cols*rows-1;k>=0;k--) rdata[k]=nodata;
        Rect2D<double>gext;
        AutoPtr<Point>lt(new Point());
        AutoPtr<Envelope>faEnv=geo->GetExtent();
        faEnv->Project(pTrans.get());
        faEnv->GetCoord(&gext.Left,&gext.Top,&gext.Right,&gext.Bottom);
        if(!gext.IntersectRect(FullExtent)){
            return true;
        }
        AutoPtr<VectorToGrid>pConvert(new VectorToGrid());
        int rx1,ry1,rx2,ry2;
        AutoPtr<Envelope>CurrentBoundry(new Envelope());
        int gcols,grows;
        long Pos,Posi;
        AutoPtr<Geometry>pGeo=geo->Clone();
        pGeo->Project(pTrans.get());
        AutoPtr<Envelope>rt=pGeo->GetExtent();
        rt->GetCoord(&gext.Left,&gext.Top,&gext.Right,&gext.Bottom);
        gext=FullExtent.Intersect(gext);
        rx1=(gext.Left-X)/xCellSize;
        if(rx1<0) rx1=0;
        ry1=(Y-gext.Top)/yCellSize;
        if(ry1<0) ry1=0;
        rx2=(gext.Right-X)/xCellSize;
        if(rx2>=cols) rx2=cols-1;
        ry2=(Y-gext.Bottom)/yCellSize;
        if(ry2>=rows) ry2=rows-1;
        lt->PutCoord(rx1*xCellSize+X,Y-ry1*yCellSize);
        gcols=rx2-rx1+1;
        grows=ry2-ry1+1;
        pConvert->SetRasterDesp(RasterDesp(lt->X(),lt->Y(),gcols,grows,xCellSize,yCellSize));
        if(!pConvert->Convert(pGeo.get(),CurrentBoundry.get(),rdata,true)){
            return false;
        }
        if(maskData==nullptr){
            for(int i=ry2;i>=ry1;i--){
                Pos=(i-ry1)*gcols+(rx2-rx1);
                Posi=i*cols+rx2;
                for(int j=rx2;j>=rx1;j--){
                    if(rdata[Pos]==2)
                        rdata[Posi]=1;
                    else
                        rdata[Posi]=0;
                    Pos--;
                    Posi--;
                }
            }
        }
        else{
            for(int i=ry2;i>=ry1;i--){
                Pos=(i-ry1)*gcols+(rx2-rx1);
                Posi=i*cols+rx2;
                for(int j=rx2;j>=rx1;j--){
                    if((maskData[Posi]!=0)&&(rdata[Pos]==2))
                        rdata[Posi]=1;
                    else
                        rdata[Posi]=0;
                    Pos--;
                    Posi--;
                }
            }
        }
        return true;
    }
    else{
        FLOAT*rowData=new FLOAT[cols];
        for(int k=0;k<cols;k++) rowData[k]=nodata;
        for(int k=0;k<rows;k++)
        {
            band->SaveBlockData(0,k,cols,1,rowData);
        }
        Rect2D<double>gext;
        AutoPtr<Point>lt(new Point());
        AutoPtr<Envelope>faEnv=geo->GetExtent();
        faEnv->Project(pTrans.get());
        faEnv->GetCoord(&gext.Left,&gext.Top,&gext.Right,&gext.Bottom);
        if(!gext.IntersectRect(FullExtent)){
            return true;
        }
        AutoPtr<VectorToGrid>pConvert(new VectorToGrid());
        int rx1,ry1,rx2,ry2;
        AutoPtr<MemDataArray>pArray(new MemDataArray());
        pArray->Initialize(cols*rows,rdtByte);
        BYTE*rdata=pArray->GetValues();
        AutoPtr<Envelope>CurrentBoundry(new Envelope());
        int gcols,grows;
        float fv;
        long Pos,Posi;
        AutoPtr<Geometry>pGeo=geo->Clone();
        pGeo->Project(pTrans.get());
        AutoPtr<Envelope>rt=pGeo->GetExtent();
        rt->GetCoord(&gext.Left,&gext.Top,&gext.Right,&gext.Bottom);
        gext=FullExtent.Intersect(gext);
        rx1=(gext.Left-X)/xCellSize;
        if(rx1<0) rx1=0;
        ry1=(Y-gext.Top)/yCellSize;
        if(ry1<0) ry1=0;
        rx2=(gext.Right-X)/xCellSize;
        if(rx2>=cols) rx2=cols-1;
        ry2=(Y-gext.Bottom)/yCellSize;
        if(ry2>=rows) ry2=rows-1;
        lt->PutCoord(rx1*xCellSize+X,Y-ry1*yCellSize);
        gcols=rx2-rx1+1;
        grows=ry2-ry1+1;
        pConvert->SetRasterDesp(RasterDesp(lt->X(),lt->Y(),gcols,grows,xCellSize,yCellSize));
        if(!pConvert->Convert(pGeo.get(),CurrentBoundry.get(),rdata,true)){
            delete []rowData;
            return false;
        }
        Pos=0;
        for(int i=0;i<grows;i++)
        {
            band->GetBlockData(rx1,i+ry1,gcols,1,gcols,1,rowData);
            if(maskData==nullptr){
                for(int j=0;j<gcols;j++)
                {
                    fv=rdata[Pos];
                    if(2==(int)fv) rowData[j]=1;
                    Pos++;
                }
            }
            else{
                Posi=(i+ry1)*cols+rx1;
                for(int j=0;j<gcols;j++)
                {
                    fv=rdata[Pos];
                    if(2==(int)fv){
                        if(maskData[Posi]!=0) rowData[j]=1;
                    }
                    Pos++;
                    Posi++;
                }
            }
            band->SaveBlockData(rx1,i+ry1,gcols,1,rowData);
        }
        delete []rowData;
        return true;
    }
    return false;
}

}
