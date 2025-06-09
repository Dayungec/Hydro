#include "rastertofeature.h"
#include "Base/classfactory.h"
#include "Datasources/rasterbandtool.h"
#include "Datasources/tabledesc.h"
//#include "MapDisplay/featurelegend.h"
//#include "GISControls/viewcontrol.h"
#include <QString>

namespace SGIS{

REGISTER(RasterToPolygon)

RRing::RRing(int deri,int firstx,int firsty)
{
    Deri=deri;
    FirstX=firstx;
    FirstY=firsty;
    PolyIndex=0;
    numofPerBlock=256;
    currentPin=0;
    blockNum=1;
    pts.resize(numofPerBlock);
    States.resize(numofPerBlock);
}
RRing::~RRing()
{

}
DOUBLE*RRing::GetRawData()
{
    return (DOUBLE*)pts.data();
}
int RRing::GetRingValue()
{
    return ringValue;
}
void RRing::SetRingValue(int value)
{
    ringValue=value;
}
void RRing::ReparePoints()
{
    int pointCount=pts.size();
    blockNum=pointCount/numofPerBlock;
    if(blockNum*numofPerBlock<pointCount) blockNum++;
    pts.resize(numofPerBlock*blockNum);
    States.resize(numofPerBlock*blockNum);
    currentPin=pointCount;
}
void RRing::RemoveAll()
{
    if(blockNum>1)
    {
       blockNum=1;
       pts.resize(numofPerBlock);
       States.resize(numofPerBlock);
    }
    currentPin=0;
}
void RRing::AddPoint(Point2D<double> pt,int state)
{
    if(currentPin>=numofPerBlock*blockNum)
    {
        blockNum++;
        pts.resize(numofPerBlock*blockNum);
        States.resize(numofPerBlock*blockNum);
    }
    pts[currentPin]=pt;
    States[currentPin]=state;
    currentPin++;
}
int RRing::GetCount()
{
    return currentPin;
}
Point2D<double> RRing::Item(int index)
{
    return pts[index];
}
int RRing::GetState(int index)
{
    return States[index];
}
int RRing::GetDeri()
{
    return Deri;
}
int RRing::GetFirstX()
{
   return FirstX;
}
int RRing::GetFirstY()
{
   return FirstY;
}
void RRing::SimplifyRing(int findex,int tindex,double fuzzy,BYTE*states)
{
    Point2D<double> pt;
    if(tindex-findex<=1)
    {
        return;
    }
    double MaxDist,tempDist;
    int index=-1;
    Point2D<double> fpt,tpt;
    fpt=pts[findex];
    tpt=pts[tindex];
    for(int k=findex+1;k<=tindex-1;k++)
    {
        pt=pts[k];
        Segment2D<double>seg(fpt.X,fpt.Y,tpt.X,tpt.Y);
        tempDist=seg.PtMinDistToSegment(pt.X,pt.Y);
        if(index==-1)
        {
            MaxDist=tempDist;
            index=k;
        }
        else if(tempDist>MaxDist)
        {
            MaxDist=tempDist;
            index=k;
        }
    }
    if(MaxDist<fuzzy)
    {
        for(int k=findex+1;k<tindex;k++) states[k]=false;
    }
    else
    {
        int FIndex,TIndex;
        FIndex=findex;
        for(int k=findex+1;k<=tindex-1;k++)
        {
           pt=pts[k];
           Segment2D<double>seg(fpt.X,fpt.Y,tpt.X,tpt.Y);
           tempDist=seg.PtMinDistToSegment(pt.X,pt.Y);
           if(tempDist==MaxDist)
           {
               TIndex=k;
               SimplifyRing(FIndex,TIndex,fuzzy,states);
               FIndex=TIndex;
           }
        }
        SimplifyRing(FIndex,tindex,fuzzy,states);
    }
}
void RRing::SimplifyRingBetweenBorder(int findex,int tindex,double fuzzy,BYTE*states,double minfuzzy)
{
    if(tindex-findex+1<=5) return;
    int IntersecPosi=-1;
    Point2D<double> pt;
    int k;
    IntersecPosi=-1;
    int FromPosi=findex;
    int ToPosi;
    for(k=findex+1;k<=tindex;k++)
    {
        if(States[k]!=0)
        {
           ToPosi=k;
           SimplifyRing(FromPosi,ToPosi,fuzzy,states);
           FromPosi=ToPosi;
        }
        else if(k==tindex)
        {
           ToPosi=k;
           SimplifyRing(FromPosi,ToPosi,fuzzy,states);
        }
    }
    int RemoveCount=0;
    Point2D<double> fpt,tpt;
    fpt=pts[findex];
    tpt=pts[tindex];
    for(k=findex+1;k<tindex;k++)
    {
        pt=pts[k];
        if(!states[k]) RemoveCount++;
    }
    while(tindex-findex+1-RemoveCount<=3)
    {
       int index=-1;
       double MaxDist=-1,tempDist;
       for(k=findex+1;k<tindex;k++)
       {
          if(states[k]) continue;
          pt=pts[k];
          Segment2D<double> seg(fpt.X,fpt.Y,tpt.X,tpt.Y);
          tempDist=seg.PtMinDistToSegment(pt.X,pt.Y);
          if(index==-1)
          {
            MaxDist=tempDist;
            index=k;
          }
          else if(tempDist>MaxDist)
          {
              MaxDist=tempDist;
              index=k;
          }
       }
       if(MaxDist==-1) break;
       for(k=findex+1;k<tindex;k++)
       {
           if(states[k]) continue;
           pt=pts[k];
           Segment2D<double> seg(fpt.X,fpt.Y,tpt.X,tpt.Y);
           tempDist=seg.PtMinDistToSegment(pt.X,pt.Y);
           if(tempDist==MaxDist)
           {
               states[k]=1;
               RemoveCount--;
           }
       }
    }
}
void RRing::SimplifyRing(double fuzzy,double minfuzzy)
{
    long size=currentPin;
    if(size<=5) return;
    pts.resize(currentPin);
    States.resize(currentPin);
    vector<BYTE>states;
    states.resize(size);
    for(int k=0;k<size;k++) states[k]=1;
    int IntersecPosi=-1;
    Point2D<double> pt;
    int k;
    int Num=0;
    for(k=0;k<size;k++)
    {
        if(States[k]==2)
        {
            if(Num==0) IntersecPosi=k;
            Num++;
            if(Num>=2) break;
        }
    }
    if((Num>=2)&&(IntersecPosi!=-1))
    {
       if(IntersecPosi!=0)
       {
          pts.erase(std::begin(pts)+size-1);
          States.erase(std::begin(States)+size-1);
          int formerSize=pts.size();
          pts.resize(pts.size()+IntersecPosi);
          States.resize(States.size()+IntersecPosi);
          for(int j=0;j<IntersecPosi;j++)
          {
             pts[j+formerSize]=pts[j];
             States[j+formerSize]=States[j];
          }

          pts.erase(std::begin(pts), std::begin(pts) + IntersecPosi);
          States.erase(std::begin(States), std::begin(States) + IntersecPosi);
          pts.push_back(pts[0]);
          States.push_back(States[0]);
       }
       int FromPosi=0;
       int ToPosi;
       for(k=1;k<size;k++)
       {
          if(States[k]==2)
          {
             ToPosi=k;
             SimplifyRingBetweenBorder(FromPosi,ToPosi,fuzzy,states.data(),minfuzzy);
             FromPosi=ToPosi;
          }
          else if(k==size-1)
          {
             ToPosi=k;
             SimplifyRingBetweenBorder(FromPosi,ToPosi,fuzzy,states.data(),minfuzzy);
          }
       }
       vector<Point2D<double>>temppts;
       vector<unsigned char>tempstates;
       temppts.resize(size);
       tempstates.resize(size);
       temppts[0]=pts[0];
       tempstates[0]=States[0];
       int fromPin=1;
       for(int k=1;k<size-1;k++)
       {
           if(!states[k]) continue;
           temppts[fromPin]=pts[k];
           tempstates[fromPin]=States[k];
           fromPin++;
       }
       temppts[fromPin]=pts[size-1];
       tempstates[fromPin]=States[size-1];
       fromPin++;
       pts.resize(fromPin);
       for(int k=0;k<fromPin;k++)
       {
           pts[k]=temppts[k];
           States[k]=tempstates[k];
       }
       ReparePoints();
       return;
    }
    IntersecPosi=-1;
    for(k=0;k<size;k++)
    {
        pt=pts[k];
        if(States[k]!=0)
        {
            IntersecPosi=k;
            break;
        }
    }
    if((IntersecPosi!=-1)&&(IntersecPosi!=0))
    {
        pts.erase(std::begin(pts) + size - 1);
        States.erase(std::begin(States) + size - 1);
        int formerSize=pts.size();
        pts.resize(pts.size()+IntersecPosi);
        States.resize(States.size()+IntersecPosi);
        for(int j=0;j<IntersecPosi;j++)
        {
            pts[j + formerSize] = pts[j];
            States[j + formerSize] = States[j];
        }
        pts.erase(std::begin(pts), std::begin(pts) + IntersecPosi);
        States.erase(std::begin(States), std::begin(States) + IntersecPosi);
        pts.push_back(pts[0]);
        States.push_back(States[0]);
    }
    int FromPosi=0;
    int ToPosi;
    for(k=1;k<size;k++)
    {
        if(States[k]!=0)
        {
           ToPosi=k;
           SimplifyRing(FromPosi,ToPosi,fuzzy,states.data());
           FromPosi=ToPosi;
        }
        else if(k==size-1)
        {
           ToPosi=k;
           SimplifyRing(FromPosi,ToPosi,fuzzy,states.data());
        }
    }
    int RemoveCount=0;
    Point2D<double> fpt,tpt;
    fpt=pts[0];
    tpt=pts[size-1];
    for(k=size-2;k>=1;k--)
    {
        pt=pts[k];
        if(!states[k]) RemoveCount++;
    }
    while(size-RemoveCount<=3)
    {
       int index=-1;
       double MaxDist=-1,tempDist;
       for(k=size-2;k>=1;k--)
       {
          if(states[k]) continue;
          pt=pts[k];
          Segment2D<double>seg(fpt.X,fpt.Y,tpt.X,tpt.Y);
          tempDist=seg.PtMinDistToSegment(pt.X,pt.Y);
          if(index==-1)
          {
            MaxDist=tempDist;
            index=k;
          }
          else if(tempDist>MaxDist)
          {
              MaxDist=tempDist;
              index=k;
          }
       }
       if(MaxDist==-1) break;
       for(k=1;k<size-1;k++)
       {
           if(states[k]) continue;
           pt=pts[k];
           Segment2D<double> seg(fpt.X,fpt.Y,tpt.X,tpt.Y);
           tempDist=seg.PtMinDistToSegment(pt.X,pt.Y);
           if(tempDist==MaxDist)
           {
               states[k]=1;
               RemoveCount--;
           }
       }
    }
    vector<Point2D<double>>temppts;
    vector<unsigned char>tempstates;
    temppts.resize(size);
    tempstates.resize(size);
    temppts[0]=pts[0];
    tempstates[0]=States[0];
    int fromPin=1;
    for(int k=1;k<size-1;k++)
    {
        if(!states[k]) continue;
        temppts[fromPin]=pts[k];
        tempstates[fromPin]=States[k];
        fromPin++;
    }
    temppts[fromPin]=pts[size-1];
    tempstates[fromPin]=States[size-1];
    fromPin++;
    pts.resize(fromPin);
    for(int k=0;k<fromPin;k++)
    {
        pts[k]=temppts[k];
        States[k]=tempstates[k];
    }
    ReparePoints();
}
void RRing::SetPolyIndex(int index)
{
    PolyIndex=index;
}
int RRing::GetPolyIndex()
{
    return PolyIndex;
}
bool RRing::IsPointIn(double x,double y)
{
    int Size=pts.size();
    if(Size<3)
    {
        return false;
    }
    double af,at,a=0;
    double pi=3.14159265;
    double dpi=6.28318530;
    DOUBLE X1,Y1,X2,Y2;
    X1=pts[0].X;
    Y1=pts[0].Y;
    if(x==X1)
    {
        if(Y1>y)
             af=pi/2;
        else if(y==Y1)
             af=0;
        else
             af=pi*3/2;
    }
    else
    {
       af=atan((Y1-y)/(X1-x));
       if(X1<x) af=af+pi;
    }
    double dt;
    for(int k=1;k<=Size;k++)
    {
        if(k<Size)
        {
            X2=pts[k].X;
            Y2=pts[k].Y;
        }
        else
        {
            X2=pts[0].X;
            Y2=pts[0].Y;
        }
        if(x==X2)
        {
            if(Y2>y)
                 at=pi/2;
            else if(y==Y2)
                 at=0;
            else
                 at=pi*3/2;
        }
        else
        {
           at=atan((Y2-y)/(X2-x));
           if(X2<x) at=at+pi;
        }
        dt=at-af;
        if(dt<0) dt=dpi+dt;
        if(dt>pi) dt=-(dpi-dt);
        a=a+dt;
        af=at;
        X1=X2;
        Y1=Y2;
    }
    if(fabs(a)>=0.000001)
    {
        return (fabs(fmod(a,6.283185))<0.000001);
    }
    return false;
}
bool RRing::IsRingIn(RRing*otherRing)
{
    if(otherRing->GetCount()<3) return false;
    Point2D<double> dpt=otherRing->Item(0);
    return IsPointIn(dpt.X,dpt.Y);
}
double RRing::GetArea()
{
    int Size=pts.size();
    if(Size<3)
    {
        return 0;
    }
    DOUBLE area=0;
    DOUBLE x1,y1,x2,y2;
    x1=pts[0].X;
    y1=pts[0].Y;
    for(int k=0;k<Size;k++)
    {
        if(k<Size-1)
        {
            x2=pts[k].X;
            y2=pts[k].Y;
        }
        else
        {
            x2=pts[0].X;
            y2=pts[0].Y;
        }
        area+=(x2-x1)*(y2+y1);
        x1=x2;
        y1=y2;
    }
    return area/2;
}
RPolygon::RPolygon()
{

}
RPolygon::~RPolygon()
{
    for(int k=rings.size()-1;k>=0;k--)
        delete rings[k];
    rings.clear();
}
int RPolygon::GetCount()
{
    return rings.size();
}
RRing*RPolygon::GetRing(int index)
{
    return rings[index];
}
void RPolygon::AddRing(RRing*ring)
{
    rings.push_back(ring);
}
double RPolygon::GetArea() {
    double area = 0;
    for (int k = 0; k < rings.size(); k++) {
        area += rings[k]->GetArea();
    }
    return area;
}
void RPolygon::SimplifyPolygon(double fuzzy,double minfuzzy)
{
    RRing*ring;
    for(int k=rings.size()-1;k>=0;k--)
    {
        ring=rings[k];
        ring->SimplifyRing(fuzzy,minfuzzy);
    }
}

RasterToPolygon::RasterToPolygon()
{
    data=nullptr;
    b0=nullptr;
    b1=nullptr;
    FuzzyRatio=0.71;
    Rows=Cols=0;
    NoData=-32768;
}
RasterToPolygon::~RasterToPolygon()
{
    if(data!=nullptr) delete[]data;
    data=nullptr;
    if(b0!=nullptr) delete b0;
    if(b1!=nullptr) delete b1;
    b0=nullptr;
    b1=nullptr;
    for(int k=rings.size()-1;k>=0;k--)
    {
        if(rings[k]->GetPolyIndex()==0) delete rings[k];
    }
    rings.clear();
    for(int k=polys.size()-1;k>=0;k--) delete polys[k];
    polys.clear();
}

int RasterToPolygon::GetStateOfBorderPixel(int x,int y)
{
    OrderArray<int> pArray;
    int nx,ny;
    int FVs[4];
    nx=x-1;ny=y-1;
    float fV;
    if((nx<0)||(ny<0))
        FVs[0]=this->NoData;
    else
        FVs[0]=data[ny*Cols+nx];
    nx=x;ny=y-1;
    if((nx>=Cols)||(ny<0))
        FVs[1]=this->NoData;
    else
        FVs[1]=data[ny*Cols+nx];
    nx=x;ny=y;
    if((nx>=Cols)||(ny>=Rows))
        FVs[2]=this->NoData;
    else
        FVs[2]=data[ny*Cols+nx];
    nx=x-1;ny=y;
    if((nx>=Cols)||(ny>=Rows))
        FVs[3]=this->NoData;
    else
        FVs[3]=data[ny*Cols+nx];
    if(FVs[0]==FVs[2])
    {
        if((FVs[1]!=FVs[0])&&(FVs[3]!=FVs[0])) return 2;
    }
    if(FVs[1]==FVs[3])
    {
        if((FVs[0]!=FVs[1])&&(FVs[2]!=FVs[3])) return 2;
    }
    for(int k=0;k<4;k++) pArray.Add(FVs[k]);
    if(pArray.GetSize()>2) return 2;
    return 0;
}

bool RasterToPolygon::InitialBorder(RasterBand*pRasterBand,CallBack*callBack,Semaphora*signal)
{
    if(data!=nullptr) delete[]data;
    data=nullptr;
    if(b0!=nullptr) delete b0;
    if(b1!=nullptr) delete b1;
    b0=nullptr;
    b1=nullptr;
    for(int k=rings.size()-1;k>=0;k--)
    {
        if(rings[k]->GetPolyIndex()==0) delete rings[k];
    }
    rings.clear();
    for(int k=polys.size()-1;k>=0;k--) delete polys[k];
    polys.clear();
    DOUBLE NoData=pRasterBand->GetNodata();
    AutoPtr<SpatialReference>disp=pRasterBand->GetSpatialReference();
    bool bSuc;
    BYTE*mArray=this->GetMaskData(callBack,signal,bSuc);
    if(!bSuc) return false;
    AutoPtr<SpatialReference>pspto=pEnvi->GetSpatialReference();
    data=new int[Cols*Rows];
    if(data==nullptr) return false;
    Left=pEnvi->Left();
    Top=pEnvi->Top();
    Right=pEnvi->GetRight();
    Bottom=pEnvi->GetBottom();
    xCellSize=pEnvi->XCellSize();
    yCellSize=pEnvi->YCellSize();
    LONG DifRows=128;
    AutoPtr<Point>BlockLeftTop=new Point();
    FLOAT*psa=nullptr;
    int FormerDif=0;
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(pRasterBand));
    CallBackHelper::BeginProgress(callBack,"initial data");
    for(int i=0;i<Rows;i+=DifRows)
    {
        int dif=DifRows;
        if(i+dif-1>=Rows) dif=Rows-i;
        BlockLeftTop->PutCoord(Left,Top-i*yCellSize);
        if(FormerDif!=dif)
        {
            if(psa!=nullptr) delete[]psa;
            psa=new FLOAT[Cols*dif];
        }
        pTool->GetBlockDataByCoord(BlockLeftTop.get(),xCellSize,yCellSize,Cols,dif,psa,pspto.get(),NoData,false);
        int tm=i+dif;
        for(int m=i;m<tm;m++){
            LONG pos=m*Cols;
            LONG npos=(m-i)*Cols;
            for(int j=0;j<Cols;j++){
                if(mArray!=nullptr){
                    if(mArray[pos]==0)
                        data[pos]=this->NoData;
                    else{
                        if((FLOAT)psa[npos]==(FLOAT)NoData)
                            data[pos]=this->NoData;
                        else
                            data[pos]=psa[npos];
                    }
                }
                else{
                    if((FLOAT)psa[npos]==(FLOAT)NoData)
                        data[pos]=this->NoData;
                    else
                        data[pos]=psa[npos];
                }
                pos++;
                npos++;
            }
        }
        if(this->DetectExitSignal(callBack,signal)){
            if(psa!=nullptr) delete []psa;
            return false;
        }
    }
    if(psa!=nullptr) delete []psa;
    b0=new BYTE[(Rows+1)*(Cols+1)];
    b1=new BYTE[(Rows+1)*(Cols+1)];
    long AllNum=0;
    int nx,ny;
    int V1,V2;
    for(int i=0;i<=Rows;i++)
    {
        for(int j=0;j<=Cols;j++)
        {
            nx=j;ny=i-1;
            if((nx>=Cols)||(ny<0))
               V1=this->NoData;
            else
            {
                V1=data[ny*Cols+nx];
            }
            nx=j;ny=i;
            if((nx>=Cols)||(ny>=Rows))
              V2=this->NoData;
            else
            {
                V2=data[ny*Cols+nx];
            }
            if(V1==V2)
               b0[AllNum]=0;
            else
               b0[AllNum]=1;
            nx=j-1;ny=i;
            if((nx<0)||(ny>=Rows))
               V1=this->NoData;
            else
            {
                V1=data[ny*Cols+nx];
            }
            if(V1==V2)
               b1[AllNum]=0;
            else
               b1[AllNum]=1;
            AllNum++;
        }
        if(this->DetectExitSignal(callBack,signal)){
            return false;
        }
    }
    return true;
}
bool RasterToPolygon::PursueRing(int x,int y)
{
    long Posi=y*(Cols+1)+x;
    int State;
    State=b0[Posi];
    Point2D<double>pt;
    int nx,ny;
    int posi;
    int ri=-1;
    LONG V;
    LONG lV;
    float fV;
    bool HasPursued=false;
    if((State==1)||(State==3))
    {
       if((x<Cols)&&(y<Rows))
       {
          lV=data[y*Cols+x];
          if(lV!=NoData)
         {
           RRing*ring=new RRing(0,x,y);
           rings.push_back(ring);
           pt.X=Left+x*xCellSize;
           pt.Y=Top-y*yCellSize;
           V=data[y*Cols+x];
           ring->SetRingValue(lV);
           ring->AddPoint(pt,GetStateOfBorderPixel(x,y));
           if(State==1)
             b0[Posi]=2;
           else if(State==3)
             b0[Posi]=0;
           ri=rings.size();
           nx=x;ny=y;
           posi=0;
           while(true)
           {
             GetNextPoint(0,posi,nx,ny,ri);
             pt.X=Left+nx*xCellSize;
             pt.Y=Top-ny*yCellSize;
             ring->AddPoint(pt,GetStateOfBorderPixel(nx,ny));
             if((nx==x)&&(ny==y))
             {
                 break;
             }
           }
           HasPursued=true;
         }
       }
    }
    State=b1[Posi];
    if((State==1)||(State==3))
    {
       if((x>0)&&(y<Rows))
       {
         lV=data[y*Cols+x-1];
         if(lV!=NoData)
         {
           RRing*ring=new RRing(1,x,y);
           V=data[y*Cols+x-1];
           ring->SetRingValue(V);
           rings.push_back(ring);
           pt.X=Left+x*xCellSize;
           pt.Y=Top-y*yCellSize;
           ring->AddPoint(pt,GetStateOfBorderPixel(x,y));
           if(State==1)
             b1[Posi]=2;
           else if(State==3)
             b1[Posi]=0;
           nx=x;ny=y;
           posi=1;
           while(true)
           {
             GetNextPoint(1,posi,nx,ny,ri);
             pt.X=Left+nx*xCellSize;
             pt.Y=Top-ny*yCellSize;
             ring->AddPoint(pt,GetStateOfBorderPixel(nx,ny));
             if((nx==x)&&(ny==y)) break;
           }
           HasPursued=true;
         }
       }
    }
    return HasPursued;
}
bool RasterToPolygon::RegisterRings(CallBack*callBack,Semaphora*signal)
{
    CallBackHelper::BeginProgress(callBack,"pursuit polygons");
    for(int i=0;i<Rows;i++)
    {
        for(int j=0;j<Cols;j++)
        {
            if(PursueRing(j,i))
            {
                if(rings.size()>=65535) break;
            }
            if(this->DetectExitSignal(callBack,signal)){
                return false;
            }
        }
        CallBackHelper::SendProgress(callBack,i,Rows);
    }
    return true;
}
void RasterToPolygon::GetNextPoint(int Deri,int&Posi,int&x,int&y,int ri)
{
    switch(Posi)
    {
    case 0:
        x++;
        break;
    case 1:
        y++;
        break;
    case 2:
        x--;
        break;
    case 3:
        y--;
        break;
    }
    Posi=Posi+2;
    if(Posi>=4) Posi-=4;
    int NextP;
    int State;
    long Pos;
    if(Deri==0)
    {
       for(int p=Posi-1;p>Posi-4;p--)
       {
           NextP=p;
           if(NextP<0) NextP+=4;
           if(NextP==0)
           {
              if((x>Cols)||(y>Rows)) continue;
              Pos=y*(Cols+1)+x;
              State=b0[Pos];
              if(State==1)
              {
                  b0[Pos]=2;
                  break;
              }
              else if(State==3)
              {
                  b0[Pos]=0;
                  break;
              }
           }
           else if(NextP==1)
           {
              if((y>Rows)||(x>Cols)) continue;
              Pos=y*(Cols+1)+x;
              State=b1[Pos];
              if(State==1)
              {
                  b1[Pos]=2;
                  break;
              }
              else if(State==3)
              {
                  b1[Pos]=0;
                  break;
              }
           }
           else if(NextP==2)
           {
              if((x<=0)||(y>Rows)) continue;
              Pos=y*(Cols+1)+x-1;
              State=b0[Pos];
              if(State==1)
              {
                  b0[Pos]=3;
                  break;
              }
              else if(State==2)
              {
                  b0[Pos]=0;
                  break;
              }
           }
           else if(NextP==3)
           {
              if((x>Cols)||(y<=0)) continue;
              Pos=(y-1)*(Cols+1)+x;
              State=b1[Pos];
              if(State==1)
              {
                  b1[Pos]=3;
                  break;
              }
              else if(State==2)
              {
                  b1[Pos]=0;
                  break;
              }
           }
       }
       Posi=NextP;
    }
    else
    {
       for(int p=Posi-1;p>Posi-4;p--)
       {
           NextP=p;
           if(NextP<0) NextP+=4;
           if(NextP==0)
           {
              if((x>Cols)||(y>Rows)) continue;
              Pos=y*(Cols+1)+x;
              State=b0[Pos];
              if(State==1)
              {
                  b0[Pos]=2;
                  break;
              }
              else if(State==3)
              {
                  b0[Pos]=0;
                  break;
              }
           }
           else if(NextP==1)
           {
              if((x>Cols)||(y>Rows)) continue;
              Pos=y*(Cols+1)+x;
              State=b1[Pos];
              if(State==1)
              {
                  b1[Pos]=2;
                  break;
              }
              else if(State==3)
              {
                  b1[Pos]=0;
                  break;
              }
           }
           else if(NextP==2)
           {
              if((x<=0)||(y>Rows)) continue;
              Pos=y*(Cols+1)+x-1;
              State=b0[Pos];
              if(State==1)
              {
                  b0[Pos]=3;
                  break;
              }
              else if(State==2)
              {
                  b0[Pos]=0;
                  break;
              }
           }
           else if(NextP==3)
           {
              if((x>Cols)||(y<=0)) continue;
              Pos=(y-1)*(Cols+1)+x;
              State=b1[Pos];
              if(State==1)
              {
                  b1[Pos]=3;
                  break;
              }
              else if(State==2)
              {
                  b1[Pos]=0;
                  break;
              }
           }
       }
       Posi=NextP;
    }
}

bool RasterToPolygon::RegisterPolygons(CallBack*callBack,Semaphora*signal)
{
    CallBackHelper::SendMessage(callBack,"register polygons");
    int Size=rings.size();
    for(int k=0;k<Size;k++)
    {
        RRing*ring=rings[k];
        if(ring->GetDeri()==0)
        {
            RPolygon*poly=new RPolygon;
            poly->AddRing(ring);
            polys.push_back(poly);
            ring->SetPolyIndex(polys.size());
        }
    }
    for(int k=0;k<Size;k++)
    {
        RRing*ring=rings[k];
        int rV=ring->GetRingValue();
        if(ring->GetDeri()==1)
        {
            double minarea=0;
            int minindex=-1;
            for(int j=0;j<Size;j++)
            {
                RRing*oring=rings[j];
                if(oring->GetDeri()!=0) continue;
                if(oring->GetRingValue()!=rV) continue;
                if(oring->IsRingIn(ring))
                {
                    double a=oring->GetArea();
                    if(minindex==-1)
                    {
                        minarea=a;
                        minindex=j;
                    }
                    else if(a<minarea)
                    {
                        minarea=a;
                        minindex=j;
                    }
                }
            }
            if(minindex>=0)
            {
                RRing*oring=rings[minindex];
                int rIndex=oring->GetPolyIndex()-1;
                RPolygon*poly=polys[rIndex];
                poly->AddRing(ring);
                ring->SetPolyIndex(rIndex+1);
            }
        }
        if(this->DetectExitSignal(callBack,signal)){
            return false;
        }
    }
    Size=polys.size();
    double Fuzzy=sqrt((xCellSize*xCellSize+yCellSize*yCellSize)/2)*FuzzyRatio;
    double cellsize=xCellSize/10;
    if(yCellSize/10<cellsize) cellsize=yCellSize/10;
    if(Fuzzy<cellsize) Fuzzy=cellsize;
    for(int k=0;k<Size;k++)
    {
        RPolygon*poly=polys[k];
        poly->SimplifyPolygon(Fuzzy,cellsize);
        if(this->DetectExitSignal(callBack,signal)){
            return false;
        }
    }
    return true;
}

AutoPtr<Polygon>RasterToPolygon::get_Polygon(LONG index)
{
    RPolygon*poly=polys[index];
    AutoPtr<Polygon>pPoly(new Polygon());
    int Count=poly->GetCount();
    for(int k=0;k<Count;k++)
    {
        RRing*ring=poly->GetRing(k);
        AutoPtr<Points>pts(new Points());
        int rCount=ring->GetCount();
        DOUBLE*rawData=ring->GetRawData();
        pts->PutCoordByPtr(rCount,rawData);
        AutoPtr<Ring>pRing(pts);
        pPoly->Add(pRing);
    }
    return pPoly;
}

AutoPtr<FeatureClass>RasterToPolygon::Execute(RasterBand*pRasterBand,FeatureClassTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    Rows=pEnvi->Rows();
    Cols=pEnvi->Cols();
    if(InitialBorder(pRasterBand,callBack,signal))
    {
        if(!RegisterRings(callBack,signal)) return nullptr;
        if(!RegisterPolygons(callBack,signal)) return nullptr;
    }
    else
        return nullptr;
    CallBackHelper::SendMessage(callBack,"保存矢量");
    AutoPtr<TableDescCreator>pCreator(new TableDescCreator());
    pCreator->AddField(FieldDesp("ID",vftInteger,6,0));
    pCreator->AddField(FieldDesp("GRIDCODE",vftInteger,10,0));
    AutoPtr<TableDesc>pDesc=pCreator->CreateTableDesc();
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FeatureClass>pfc=target->CreateFeatureClass(psp.get(),vstPolygon,pDesc.get());
    if(pfc==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"create feature class failed");
        return nullptr;
    }
    long m;
    RPolygon*poly;
    long PolyCount=polys.size();
    Point2D<double> pt;
    RRing*ring;
    LONG V;
    vector<Variant>bVs;
    bVs.resize(2);
    AutoPtr<FeatureClassEdit>pEdit(pfc);
    for(m=0;m<PolyCount;m++)
    {
        poly=polys[m];
        AutoPtr<Polygon>pPoly=get_Polygon(m);
        ring=poly->GetRing(0);
        V=data[Cols*ring->GetFirstY()+ring->GetFirstX()];
        bVs[0]=Variant(to_string(m+1));
        bVs[1] = Variant(to_string(V));
        pEdit->AddFeatureEx(pPoly,bVs);
        if(this->DetectExitSignal(callBack,signal)) return nullptr;
        CallBackHelper::SendProgress(callBack,(float)m/PolyCount*100);
    }
    CallBackHelper::EndProgress(callBack);
    pfc->FlushCache();
    return pfc;
}

}
