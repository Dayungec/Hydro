#include "labelcomputer.h"
#include "math.h"
namespace SGIS{

LabelPointComputer::LabelPointComputer()
{

}
LabelPointComputer::~LabelPointComputer()
{

}

long LabelPointComputer::RegionNumber(int regionx,int regiony)
{
    return (long)regiony*Cols+regionx;
}
int LabelPointComputer::RegionX(double X)
{
    int x=(X-FullExtent.Left)/XCellSize;
    if(x<0)
       x=0;
    else if(x>=Cols)
      x=Cols-1;
    return x;
}
int LabelPointComputer::RegionY(double Y)
{
    int y=(FullExtent.Top-Y)/YCellSize;
    if(y<0)
      y=0;
    else if(y>=Rows)
      y=Rows-1;
    return y;
}
int LabelPointComputer::RealRegionX(double X)
{
    return (X-FullExtent.Left)/XCellSize;
}
int LabelPointComputer::RealRegionY(double Y)
{
    return (FullExtent.Top-Y)/YCellSize;
}
void LabelPointComputer::SetLeftTop(DOUBLE left,DOUBLE top)
{
     Left=left;
     Top=top;
}
void LabelPointComputer::SetCellSize(DOUBLE xcellSize,double ycellSize)
{
     XCellSize=xcellSize;
     YCellSize=ycellSize;
     DifFuzzy=min(XCellSize,YCellSize)/1000;
}
void LabelPointComputer::SetCols(int cols)
{
     Cols=cols;
}
void LabelPointComputer::SetRows(int rows)
{
    Rows=rows;
}
void LabelPointComputer::ConvertRing(Points*pnts,BYTE*pArray)
{
    double XMin,YMin,XMax,YMax;
    AutoPtr<Envelope>ext=pnts->GetExtent();
    ext->GetCoord(&XMin,&YMax,&XMax,&YMin);
    int RegionX1=RealRegionX(XMin);
    int RegionY1=RealRegionY(YMax);
    int RegionX2=RealRegionX(XMax);
    int RegionY2=RealRegionY(YMin);
    long RegionN;
    int V;
    int fromx1,fromx2,fromy1,fromy2;
    fromx1=max(RegionX1,0);
    fromx2=min(RegionX2,Cols-1);
    fromy1=max(RegionY1,0);
    fromy2=min(RegionY2,Rows-1);
    if((fromx2==fromx1)&&(fromy2==fromy1))
    {
        for(int i=fromy1;i<=fromy2;i++)
        {
            for(int j=fromx1;j<=fromx2;j++)
            {
                RegionN=RegionNumber(j,i);
                V=pArray[RegionN];
                if((V%4)==2)
                    pArray[RegionN]=0;
                else
                    pArray[RegionN]=2;
            }
        }
        return;
    }
    double X1,Y1,X2,Y2;
    int rx1,rx2,ry1,ry2,j,minr;
    LONG ptCount=pnts->GetSize();
    if(ptCount<3)
    {
        return;
    }
    pnts->GetItem(0,X1,Y1);
    rx1=RealRegionX(X1);ry1=RealRegionY(Y1);
    double InterX;
    double centery;
    double lx=FullExtent.Left+XCellSize*RegionX1;
    double rx=FullExtent.Left+XCellSize*(RegionX2+1);
    int leftr;
    int FormerY;
    int FormerDeri=0;
    bool IsIntersect;
    for(long k=1;k<ptCount;k++)
    {
        if(k<ptCount-1)
            pnts->GetItem(k,X2,Y2);
        else
            pnts->GetItem(0,X2,Y2);
        rx2=RealRegionX(X2);ry2=RealRegionY(Y2);
        if(Y1>Y2)
        {
           if((FormerDeri==1)&&(FormerY>=ry1)) ry1=FormerY+1;
           double r=(X2-X1)/(Y2-Y1);
           for(int p=ry1;p<=ry2;p++)
           {
               centery=FullExtent.Top-YCellSize/2-p*YCellSize;
               IsIntersect=false;
               if((centery>=Y2-DifFuzzy)&&(centery<=Y1+DifFuzzy))
               {
                   FormerY=p;
                   FormerDeri=1;
                   IsIntersect=true;
                   InterX=r*(centery-Y1)+X1;
               }
               if((IsIntersect)&&(p>=0)&&(p<=Rows-1))
               {
                   leftr=RealRegionX(InterX);
                   if(InterX>FullExtent.Left+XCellSize/2+XCellSize*leftr)
                   {
                      leftr++;
                   }
                   if(leftr>0)
                   {
                     RegionN=RegionNumber(max(RegionX1,0),p);
                     minr=min(leftr,Cols);
                     for(j=max(RegionX1,0);j<minr;j++)
                     {
                       V=pArray[RegionN]-1;
                       if(V<0) V+=4;
                       pArray[RegionN]=V;
                       RegionN++;
                     }
                   }
                   if(leftr<0) leftr=0;
                   if(leftr<=Cols-1)
                   {
                     RegionN=RegionNumber(leftr,p);
                     minr=min(RegionX2,Cols-1);
                     for(j=leftr;j<=minr;j++)
                     {
                       V=pArray[RegionN]+1;
                       if(V>3) V-=4;
                       pArray[RegionN]=V;
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
               centery=FullExtent.Top-YCellSize/2-p*YCellSize;
               IsIntersect=false;
               if((centery>=Y1-DifFuzzy)&&(centery<=Y2+DifFuzzy))
               {
                   FormerY=p;
                   FormerDeri=-1;
                   IsIntersect=true;
                   InterX=r*(centery-Y1)+X1;
               }
               if((IsIntersect)&&(p>=0)&&(p<=Rows-1))
               {
                   leftr=RealRegionX(InterX);
                   if(InterX>FullExtent.Left+XCellSize/2+XCellSize*leftr)
                   {
                      leftr++;
                   }
                   if(leftr>0)
                   {
                     RegionN=RegionNumber(max(RegionX1,0),p);
                     minr=min(leftr,Cols);
                     for(j=max(RegionX1,0);j<minr;j++)
                     {
                       V=pArray[RegionN]+1;
                       if(V>3) V-=4;
                       pArray[RegionN]=V;
                       RegionN++;
                     }
                   }
                   if(leftr<0) leftr=0;
                   if(leftr<=Cols-1)
                   {
                     RegionN=RegionNumber(leftr,p);
                     minr=min(RegionX2,Cols-1);
                     for(j=leftr;j<=minr;j++)
                     {
                       V=pArray[RegionN]-1;
                       if(V<0) V+=4;
                       pArray[RegionN]=V;
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
void LabelPointComputer::ConvertPolygon(Polygon*poly,BYTE*pArray)
{
    int PtsCount=poly->GetSize();
    AutoPtr<Ring>pRing(new Points());
    Points*pts;
    for(long k=0;k<PtsCount;k++)
    {
        pRing=poly->GetItem(k);
        pts=(Points*)pRing.get();
        ConvertRing(pts,pArray);
    }
}
BYTE*LabelPointComputer::Convert(Polygon*pRefGeo)
{
    FullExtent.Left=Left;
    FullExtent.Top=Top;
    FullExtent.Right=XCellSize*Cols+Left;
    FullExtent.Bottom=Top-YCellSize*Rows;
    AutoPtr<Envelope>ext=pRefGeo->GetExtent();
    DOUBLE XMin,YMin,XMax,YMax;
    ext->GetCoord(&XMin,&YMax,&XMax,&YMin);
    Rect2D<double>rect(XMin,YMax,XMax,YMin);
    if(!FullExtent.IntersectRect(rect)) return nullptr;
    BYTE*pArray=new BYTE[Cols*Rows];
    memset(pArray,0,Cols*Rows);
    ConvertPolygon(pRefGeo,pArray);
    return pArray;
}
vector<Point2D<double>>LabelPointComputer::GetLabelPoint(Polygon*pRefGeo)
{
    BYTE*pArray=Convert(pRefGeo);
    vector<Point2D<double>>dpts;
    if(pArray==NULL) return dpts;
    LONG Pos=0;
    int V;
    int x;
    int y;
    bool IsBorder;
    for(int i=0;i<Rows;i++)
    {
        for(int j=0;j<Cols;j++)
        {
            V=pArray[Pos]%4;
            if(V!=2)
            {
                pArray[Pos]=0;
                Pos++;
                continue;
            }
            IsBorder=false;
            if((i==0)||(i==Rows-1)||(j==0)||(j==Cols-1))
                IsBorder=true;
            else
            {
                for(int k=0;k<8;k++)
                {
                    x=j+aroundx8[k];
                    y=i+aroundy8[k];
                    if((pArray[x+y*Cols]%4)!=2)
                    {
                        IsBorder=true;
                        break;
                    }
                }
            }
            pArray[Pos]=(IsBorder?2:30);
            Pos++;
        }
    }
    bool NeedContinue=true;
    int fV,tV;
    fV=2;
    tV=3;
    while(NeedContinue)
    {
        NeedContinue=false;
        Pos=-1;
        for(int i=0;i<Rows;i++)
        {
            for(int j=0;j<Cols;j++)
            {
                Pos++;
                V=pArray[Pos];
                if(V!=fV) continue;
                for(int k=0;k<8;k++)
                {
                    x=j+aroundx8[k];
                    y=i+aroundy8[k];
                    if((x<0)||(x>=Cols)||(y<0)||(y>=Rows)) continue;
                    if(30==pArray[x+y*Cols])
                    {
                        pArray[x+y*Cols]=tV;
                        NeedContinue=true;
                    }
                }
            }
        }
        if(!NeedContinue) break;
        fV++;
        tV++;
    }
    float semiCols=Cols/2;
    float semiRows=Rows/2;
    float minxy=-1,xy;
    int xindex[5];
    int yindex[5];
    for(int k=0;k<5;k++)
    {
        xindex[k]=yindex[k]=-1;
    }
    int targetV=fV;
    int minPos=2;
    if(minPos<targetV) minPos=targetV;
    Pos=-1;
    for(int i=0;i<Rows;i++)
    {
        for(int j=0;j<Cols;j++)
        {
            Pos++;
            if(fV==pArray[Pos])
            {
                xy=min(fabs(j-semiCols),fabs(i-semiRows));
                if(minxy==-1)
                {
                    minxy=xy;
                    xindex[0]=j;
                    yindex[0]=i;
                }
                else if(xy<minxy)
                {
                    minxy=xy;
                    xindex[0]=j;
                    yindex[0]=i;
                }
            }
        }
    }
    if(minxy==-1)
    {
        delete []pArray;
        return dpts;
    }
    int xi,yi;
    int kp=1;
    int fp=0;
    float minspace=max(Cols,Rows)/8;
    while(true)
    {
        Pos=-1;
        float maxdist=0;
        xi=-1;
        yi=-1;
        for(int i=0;i<Rows;i++)
        {
            for(int j=0;j<Cols;j++)
            {
                Pos++;
                if(fV!=pArray[Pos]) continue;
                bool HasAdded=false;
                for(int p=fp;p<kp;p++)
                {
                    if((i==yindex[p])&&(j==xindex[p]))
                    {
                        HasAdded=true;
                        break;
                    }
                }
                if(HasAdded) continue;
                float mindist=30;
                float submaxdist=0;
                for(int p=0;p<kp;p++)
                {
                    float dist=sqrt((float)(i-yindex[p])*(i-yindex[p])+(float)(j-xindex[p])*(j-xindex[p]));
                    mindist=min(mindist,dist);
                    if(mindist<=minspace) break;
                    if(dist>submaxdist) submaxdist=dist;
                }
                if(mindist<=minspace) continue;
                if(maxdist==0)
                {
                    xi=j;
                    yi=i;
                    maxdist=submaxdist;
                }
                else if(submaxdist>maxdist)
                {
                    xi=j;
                    yi=i;
                    maxdist=submaxdist;
                }
            }
        }
        if(xi==-1)
        {
            fV--;
            if(fV<minPos) break;
            fp=kp;
        }
        else
        {
            xindex[kp]=xi;
            yindex[kp]=yi;
            kp++;
            if(kp>=5) break;
        }
    }
    int validPos=0;
    for(int i=0;i<5;i++)
    {
        if(xindex[i]>=0)
            validPos=i;
        else
            break;
    }
    float xy2=xy;
    for(int i=0;i<validPos-1;i++)
    {
        for(int j=i;j<validPos-1;j++)
        {
            xy=min(fabs(xindex[j]-semiCols),fabs(yindex[j]-semiRows));
            xy2=min(fabs(xindex[j+1]-semiCols),fabs(yindex[j+1]-semiRows));
            if(xy>xy2)
            {
                int temp=xindex[j];xindex[j]=xindex[j+1];xindex[j+1]=temp;
                temp=yindex[j];yindex[j]=yindex[j+1];yindex[j+1]=temp;
            }
        }
    }
    for(int p=0;p<5;p++)
    {
        if(xindex[p]==-1)
        {
            delete []pArray;
            return dpts;
        }
        dpts.push_back(Point2D<double>(xindex[p]*XCellSize+Left+XCellSize/2,Top-yindex[p]*YCellSize-YCellSize/2));
    }
    delete []pArray;
    return dpts;
}


}
