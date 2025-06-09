#ifndef GEOMETRY2D_H
#define GEOMETRY2D_H
#include "Base/base_globle.h"
#include "Base/autoptr.h"
#include "geometry.h"
#include "math.h"
#include "envelope.h"
namespace SGIS{

template<typename T>
struct Point2D;
template<typename T>
struct Segment2D;
template<typename T>
struct Triangle2D;

template<typename T>
struct Point2D{
    T X;
    T Y;
    Point2D(){
      X=Y=0;
    };
    Point2D(T X,T Y){
        this->X=X;
        this->Y=Y;
    };
    Point2D(const Point2D<T>&other){
        this->X=other.X;
        this->Y=other.Y;
    };
    Point2D(Point&ppt){
        double x,y;
        ppt.GetCoord(&x,&y);
        X=x;
        Y=y;
    };
    void PutCoord(T x,T y){
        X=x;
        Y=y;
    };
    void ToPoint(Point&pt){
        pt.PutCoord(X,Y);
    };
    Point2D<T>&operator =(const Point2D<T>&other){
        this->X=other.X;
        this->Y=other.Y;
        return *this;
    };
    Point2D<T>&operator =(Point*other){
        this->X=other->X();
        this->Y=other->Y();
        return *this;
    };
    Point2D operator + (const Point2D&other){
       return Point2D(X+other.X,Y+other.Y);
    };
    Point2D operator + (T v){
       return Point2D(X+v,Y+v);
    };
    Point2D operator - (const Point2D&other){
       return Point2D(X-other.X,Y-other.Y);
    };
    Point2D operator * (const Point2D&other){
       return Point2D(X*other.X,Y*other.Y);
    };
    Point2D operator * (T v){
       return Point2D(X*v,Y*v);
    };
    Point2D operator - (T v){
       return Point2D(X-v,Y-v);
    };
    Point2D operator / (const Point2D&other){
       return Point2D(X/other.X,Y/other.Y);
    };
    Point2D operator / (T v){
       return Point2D(X/v,Y/v);
    };
    Point2D&operator += (const Point2D&other){
       X+=other.X;
       Y+=other.Y;
       return *this;
    };
    Point2D&operator -= (const Point2D&other){
       X-=other.X;
       Y-=other.Y;
       return *this;
    };
    Point2D&operator *= (const Point2D&other){
       X*=other.X;
       Y*=other.Y;
       return *this;
    };
    Point2D&operator /= (const Point2D&other){
       X/=other.X;
       Y/=other.Y;
       return *this;
    };
    Point2D Abs(){
       return Point2D(fabs(X),fabs(Y));
    };

    double DistanceTo(Point2D<T>other){
        return sqrt((X-other.X)*(X-other.X)+(Y-other.Y)*(Y-other.Y));
    };
    double DistanceTo(T x,T y){
        return sqrt((X-x)*(X-x)+(Y-y)*(Y-y));
    };
};

template<typename T>
struct SGIS_EXPORT Segment2D{
    T lnx1;
    T lny1;
    T lnx2;
    T lny2;
    T Fuzzy;
    Segment2D(){
        lnx1=lny1=lnx2=lny2=0;
    };
    Segment2D(T x1,T y1,T x2,T y2){
        lnx1=x1;
        lny1=y1;
        lnx2=x2;
        lny2=y2;
    };
    Segment2D(const Segment2D&seg){
        lnx1=seg.lnx1;
        lny1=seg.lny1;
        lnx2=seg.lnx2;
        lny2=seg.lny2;
    };
    double Angle(){
        if(lnx1==lnx2)
        {
            if(lny1>lny2)
                return PI*3/2;
            if(lny1==lny2)
                return 0;
            return PI/2;
        }
        else
        {
            double temp=atan((lny2-lny1)/(lnx2-lnx1));
            if(lnx2<lnx1)
                temp=temp+PI;
            if(temp<0)
                temp=temp+DPI;
            return temp;
        }
    };
    bool IsPointOnSegment(T X,T Y,T toler){
        if((lnx1==lnx2)&&(lny1==lny2))
        {
            if((fabs(X-lnx1)<=toler)&&(fabs(Y-lny1)<=toler)) return true;
            return false;
        }
        if(lnx1==lnx2)
        {
            if((fabs(X-lnx1)<=toler)&&(Y>=min(lny1,lny2))&&(Y<=max(lny1,lny2))) return true;
            return false;
        }
        if(lny1==lny2)
        {
            if((fabs(Y-lny1)<=toler)&&(X>=min(lnx1, lnx2))&&(X<=max(lnx1, lnx2))) return true;
            return false;
        }
        if((X>=min(lnx1, lnx2)-toler)&&(X<=max(lnx1, lnx2)+toler))
        {
            double k;
            k=(double)(lny2-lny1)/(lnx2-lnx1);
            double iY=k*(X-lnx1)+lny1;
            if((fabs(iY-Y)<=toler)&&(iY>=min(lny1, lny2))&&(iY<=max(lny1, lny2))) return true;
            return false;
        }
        return false;
    };
    double GetLength(){
        return sqrt((lnx1-lnx2)*(lnx1-lnx2)+(lny1-lny2)*(lny1-lny2));
    };
    bool IsPointOnSegment(T X,T Y){
        return IsPointOnSegment(X,Y,VertexTolerance);
    };
    double PtMinDistToSegment(T ptx,T pty){
        double dist1,dist2;
        if((lnx1==lnx2)&&(lny1==lny2)) return sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
        if(lnx1==lnx2)
        {
            if((pty>=min(lny1,lny2))&&(pty<=max(lny1,lny2)))
                return fabs(ptx-lnx1);
            dist1=sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
            dist2=sqrt(pow(ptx-lnx2,2)+pow(pty-lny2,2));
            return min(dist1,dist2);
        }
        if(lny1==lny2)
        {
            if((ptx>=min(lnx1,lnx2))&&(ptx<=max(lnx1,lnx2))) return fabs(pty-lny1);
            dist1=sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
            dist2=sqrt(pow(ptx-lnx2,2)+pow(pty-lny2,2));
            return min(dist1,dist2);
        }
        double k=(double)(lny2-lny1)/(lnx2-lnx1);
        double tempx=(k*lnx1+(double)ptx/k-lny1+pty)/(k+1/k);
        if((tempx>=min(lnx1,lnx2))&&(tempx<=max(lnx1,lnx2))) return sqrt(pow(ptx-tempx,2)+pow(k*(tempx-lnx1)+lny1-pty,2));
        dist1=sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
        dist2=sqrt(pow(ptx-lnx2,2)+pow(pty-lny2,2));
        return min(dist1,dist2);
    };

    double PtMinDistToSegment(T ptx,T pty,T&intersectx,T&intersecty){
        double dist1,dist2;
        if((lnx1==lnx2)&&(lny1==lny2))
        {
            intersectx=lnx1;intersecty=lny1;
            return sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
        }
        if(lnx1==lnx2)
        {
            if((pty>=min(lny1,lny2))&&(pty<=max(lny1,lny2)))
            {
                intersectx=lnx1;intersecty=pty;
                return fabs(ptx-lnx1);
            }
            dist1=sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
            dist2=sqrt(pow(ptx-lnx2,2)+pow(pty-lny2,2));
            if(dist1<=dist2)
            {
                intersectx=lnx1;intersecty=lny1;
                return dist1;
            }
            else
            {
                intersectx=lnx2;intersecty=lny2;
                return dist2;
            }
        }
        if(lny1==lny2)
        {
            if((ptx>=min(lnx1,lnx2))&&(ptx<=max(lnx1,lnx2)))
            {
                intersectx=ptx;intersecty=lny1;
                return fabs(pty-lny1);
            }
            dist1=sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
            dist2=sqrt(pow(ptx-lnx2,2)+pow(pty-lny2,2));
            if(dist1<=dist2)
            {
                intersectx=lnx1;intersecty=lny1;
                return dist1;
            }
            else
            {
                intersectx=lnx2;intersecty=lny2;
                return dist2;
            }
        }
        double k=(double)(lny2-lny1)/(lnx2-lnx1);
        double tempx=(k*lnx1+(double)ptx/k-lny1+pty)/(k+1/k);
        if((tempx>=min(lnx1,lnx2))&&(tempx<=max(lnx1,lnx2)))
        {
            double tempy=k*(tempx-lnx1)+lny1;
            intersectx=tempx;intersecty=tempy;
            return sqrt(pow(ptx-tempx,2)+pow(pty-tempy,2));
        }
        dist1=sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
        dist2=sqrt(pow(ptx-lnx2,2)+pow(pty-lny2,2));
        if(dist1<=dist2)
        {
            intersectx=lnx1;intersecty=lny1;
            return dist1;
        }
        intersectx=lnx2;intersecty=lny2;
        return dist2;
    };
    double PtMinDistToSegment(Point2D<T>&dpt){
        return PtMinDistToSegment(dpt.X,dpt.Y);
    };
    bool IntersectofSegment(T otherlnx1, T otherlny1, T otherlnx2,T otherlny2,T&intersectx,T&intersecty){
        if((otherlnx1==otherlnx2)&&(otherlny1==otherlny2 )&&(lnx1==lnx2 )&&(lny1==lny2))
        {
            if((otherlnx1==lnx1)&&(otherlny1==lny1))
            {
                intersectx=otherlnx1;
                intersecty=otherlny1;
                return true;
            }
            return false;
        }
        if((otherlnx1==otherlnx2)&&(otherlny1==otherlny2))
        {
            if(IsPointOnSegment(otherlnx1,otherlny1))
            {
                intersectx=otherlnx1;
                intersecty=otherlny1;
                return true;
            }
            return false;
        }
        if((lnx1==lnx2)&&(lny1==lny2))
        {
            Segment2D<T>otherseg(otherlnx1,otherlny1,otherlnx2,otherlny1);
            if(otherseg.IsPointOnSegment(lnx1,lny1))
            {
                intersectx=lnx1;
                intersecty=lny1;
                return true;
            }
            return false;
        }
        if((otherlnx1==otherlnx2)&&(lnx1==lnx2))
        {
            if(otherlnx1==lnx1)
            {
                double y1,y2,y3,y4;
                y1=min(lny1,lny2);
                y2=max(lny1,lny2);
                y3=min(otherlny1,otherlny2);
                y4=max(otherlny1,otherlny2);
                if((y1>=y3)&&(y1<=y4))
                {
                    intersectx=otherlnx1;
                    intersecty=y1;
                    return true;
                }
                else
                {
                    if((y3>=y1)&&(y3<=y2))
                    {
                        intersectx=otherlnx1;
                        intersecty=y3;
                        return true;
                    }
                }
            }
            return false;
        }
        if(otherlnx1==otherlnx2)
        {
            if((otherlnx1>=min(lnx1,lnx2))&&(otherlnx1<=max(lnx1, lnx2)))
            {
                double k,tempy;
                k=(double)(lny2-lny1)/(lnx2-lnx1);
                tempy=k*(otherlnx1-lnx1)+lny1;
                if((tempy>=min(otherlny1,otherlny2))&&(tempy<=max(otherlny1, otherlny2)))
                {
                    intersectx=otherlnx1;
                    intersecty=tempy;
                    return true;
                }
            }
            return false;
        }
        if(lnx1==lnx2)
        {
            if((lnx1>=min(otherlnx1,otherlnx2))&&(lnx1<=max(otherlnx1,otherlnx2)))
            {
                double k,tempy;
                k=(double)(otherlny2-otherlny1)/(otherlnx2-otherlnx1);
                tempy=k*(lnx1-otherlnx1)+otherlny1;
                if((tempy>=min(lny1,lny2))&&(tempy<=max(lny1,lny2)))
                {
                    intersectx=lnx1;
                    intersecty=tempy;
                    return true;
                }
            }
            return false;
        }
        double k1,k2,tempx,tempy;
        k1=(double)(lny2-lny1)/(lnx2-lnx1);
        k2=(double)(otherlny2-otherlny1)/(otherlnx2-otherlnx1);
        double x1,x2,x3,x4;
        x1=min(lnx1,lnx2);
        x2=max(lnx1,lnx2);
        x3=min(otherlnx1,otherlnx2);
        x4=max(otherlnx1,otherlnx2);
        if(k1==k2)
        {
            Segment2D<T> otherseg(otherlnx1,otherlny1,otherlnx2,otherlny1);
            if(otherseg.IsPointOnSegment(lnx1,lny1))
            {
                intersectx=lnx1;
                intersecty=lny1;
                return true;
            }
            if(otherseg.IsPointOnSegment(lnx2,lny2))
            {
                intersectx=lnx2;
                intersecty=lny2;
                return true;
            }
            if(IsPointOnSegment(otherlnx1,otherlny1))
            {
                intersectx=otherlnx1;
                intersecty=otherlny1;
                return true;
            }
            if(IsPointOnSegment(otherlnx2,otherlny2))
            {
                intersectx=otherlnx2;
                intersecty=otherlny2;
                return true;
            }
            return false;
        }
        else
        {
            if(lnx1==otherlnx1)
            {
                if(lny1==otherlny1)
                {
                    intersectx=lnx1;
                    intersecty=lny1;
                    return true;
                }
            }
            if(lnx1==otherlnx2)
            {
                if(lny1==otherlny2)
                {
                    intersectx=lnx1;
                    intersecty=lny1;
                    return true;
                }
            }
            if(lnx2==otherlnx1)
            {
                if(lny2==otherlny1)
                {
                    intersectx=lnx2;
                    intersecty=lny2;
                    return true;
                }
            }
            if(lnx2==otherlnx2)
            {
                if(lny2==otherlny2)
                {
                    intersectx=lnx2;
                    intersecty=lny2;
                    return true;
                }
            }
            tempx=(k1*lnx1-k2*otherlnx1+otherlny1-lny1)/(k1 - k2);
            if((tempx>=x1)&&(tempx<=x2)&&(tempx>=x3)&&(tempx<=x4))
            {
                tempy=k1*(tempx-lnx1)+lny1;
                double y1=min(lny1,lny2);
                double y2=max(lny1,lny2);
                double y3=min(otherlny1,otherlny2);
                double y4=max(otherlny1,otherlny2);
                if((tempy>=y1)&&(tempy<=y2)&&(tempy>=y3)&&(tempy<=y4))
                {
                    intersectx=tempx;
                    intersecty=tempy;
                    return true;
                }
            }
        }
        return false;
    };

    bool IntersectofSegment(Segment2D<T>&otherseg,Point2D<T>&intersectpt) {//判断点两条线段是否相交,如果相交,交点以intersectpt返回
         return IntersectofSegment(otherseg.lnx1,otherseg.lny1,otherseg.lnx2,otherseg.lny2,intersectpt.X,intersectpt.Y);
    };

    bool ContainSegment(Segment2D<T>&seg){
        T xmin,ymin,xmax,ymax;
        T xmin2,ymin2,xmax2,ymax2;
        xmin=lnx1;xmax=lnx2;
        ymin=lny1;ymax=lny2;
        if(xmin>xmax){T temp=xmin;xmin=xmax;xmax=temp;}
        if(ymin>ymax){T temp=ymin;ymin=ymax;ymax=temp;}
        xmin2=seg.lnx1;xmax2=seg.lnx2;
        ymin2=seg.lny1;ymax2=seg.lny2;
        if(xmin2>xmax2){T temp=xmin2;xmin2=xmax2;xmax2=temp;}
        if(ymin2>ymax2){T temp=ymin2;ymin2=ymax2;ymax2=temp;}
        if((xmin2>xmax+VertexTolerance)||(xmax2<xmin-VertexTolerance)||(ymin2>ymax+VertexTolerance)||(ymax2<ymin-VertexTolerance)) return false;
        bool IsTSegFromPtIn=IsPointOnSegment(seg.lnx1,seg.lny1,VertexTolerance);
        bool IsTSegToPtIn=IsPointOnSegment(seg.lnx2,seg.lny2,VertexTolerance);
        return ((IsTSegFromPtIn)&&(IsTSegToPtIn));
    };

    int GetSegmentIntersectState(Segment2D<T>&seg){
        //0--不相交；1--1个交点；2--2个交点；3--相交；4--对于1个交点，如果交点位于本线段的起点;5--对于1个交点，如果交点位于seg的起点;6--对于1个交点，如果交点位于seg的起点且本线段的起点
        T xmin,ymin,xmax,ymax;
        T xmin2,ymin2,xmax2,ymax2;
        xmin=lnx1;xmax=lnx2;
        ymin=lny1;ymax=lny2;
        if(xmin>xmax){T temp=xmin;xmin=xmax;xmax=temp;}
        if(ymin>ymax){T temp=ymin;ymin=ymax;ymax=temp;}
        xmin2=seg.lnx1;xmax2=seg.lnx2;
        ymin2=seg.lny1;ymax2=seg.lny2;
        if(xmin2>xmax2){T temp=xmin2;xmin2=xmax2;xmax2=temp;}
        if(ymin2>ymax2){T temp=ymin2;ymin2=ymax2;ymax2=temp;}
        if((xmin2>xmax+VertexTolerance)||(xmax2<xmin-VertexTolerance)||(ymin2>ymax+VertexTolerance)||(ymax2<ymin-VertexTolerance)) return 0;
        bool IsFSegFromPtIn=seg.IsPointOnSegment(lnx1,lny1);
        bool IsFSegToPtIn=seg.IsPointOnSegment(lnx2,lny2);
        bool IsTSegFromPtIn=IsPointOnSegment(seg.lnx1,seg.lny1);
        bool IsTSegToPtIn=IsPointOnSegment(seg.lnx2,seg.lny2);
        if((IsFSegFromPtIn)&&(IsFSegToPtIn))
            return 2;
        else if((IsTSegFromPtIn)&&(IsTSegToPtIn))
            return 2;
        else if((IsFSegFromPtIn)&&(IsTSegFromPtIn))
        {
            if((fabs(lnx1-seg.lnx1)<=VertexTolerance)&&(fabs(lny1-seg.lny1)<=VertexTolerance)) return 6;
            return 2;
        }
        else if((IsFSegFromPtIn)&&(IsTSegToPtIn))
        {
            if((fabs(lnx1-seg.lnx2)<=VertexTolerance)&&(fabs(lny1-seg.lny2)<=VertexTolerance)) return 4;
            return 2;
        }
        else if((IsFSegToPtIn)&&(IsTSegFromPtIn))
        {
            if((fabs(lnx2-seg.lnx1)<=VertexTolerance)&&(fabs(lny2-seg.lny1)<=VertexTolerance)) return 5;
            return 2;
        }
        else if((IsFSegToPtIn)&&(IsTSegToPtIn))
        {
            if((fabs(lnx2-seg.lnx2)<=VertexTolerance)&&(fabs(lny2-seg.lny2)<=VertexTolerance)) return 1;
            return 2;
        }
        else if(IsFSegFromPtIn)
        {
            return 4;
        }
        else if(IsFSegToPtIn)
        {
            return 1;
        }
        else if(IsTSegFromPtIn)
        {
            return 5;
        }
        else if(IsTSegToPtIn)
        {
            return 1;
        }
        Point2D<T> intersectPoint;
        if(!IntersectofSegment(seg,intersectPoint)) return 0;//没有交点
        return 1;
    };
    int GetRingSegmentIntersectState(Segment2D<T>&seg,Point2D<T>&fromPoint){
       //0-不相交；对于面：1-7:面内；边缘、面外的掩膜；对于线：1-7:右侧；边缘、左侧的掩膜
        T xmin,ymin,xmax,ymax;
        T xmin2,ymin2,xmax2,ymax2;
        xmin=lnx1;xmax=lnx2;
        ymin=lny1;ymax=lny2;
        if(xmin>xmax){T temp=xmin;xmin=xmax;xmax=temp;}
        if(ymin>ymax){T temp=ymin;ymin=ymax;ymax=temp;}
        xmin2=seg.lnx1;xmax2=seg.lnx2;
        ymin2=seg.lny1;ymax2=seg.lny2;
        if(xmin2>xmax2){T temp=xmin2;xmin2=xmax2;xmax2=temp;}
        if(ymin2>ymax2){T temp=ymin2;ymin2=ymax2;ymax2=temp;}
        if((xmin2>xmax+Fuzzy)||(xmax2<xmin-Fuzzy)||(ymin2>ymax+Fuzzy)||(ymax2<ymin-Fuzzy)) return 0;
        bool IsFSegFromPtIn=seg.IsPointOnSegment(lnx1,lny1,VertexTolerance);
        bool IsFSegToPtIn=seg.IsPointOnSegment(lnx2,lny2,VertexTolerance);
        bool IsTSegFromPtIn=IsPointOnSegment(seg.lnx1,seg.lny1,VertexTolerance);
        bool IsTSegToPtIn=IsPointOnSegment(seg.lnx2,seg.lny2,VertexTolerance);
        if((IsFSegFromPtIn)&&(IsFSegToPtIn)) return 2;
        Point2D<T> toPoint,intersectPoint,toPoint2;
        toPoint2.X=seg.lnx2;
        toPoint2.Y=seg.lny2;
        if((IsTSegFromPtIn)&&(IsTSegToPtIn))
        {
            intersectPoint.X=seg.lnx1;
            intersectPoint.Y=seg.lny1;
            double dist1=sqrt((lnx1-seg.lnx1)*(lnx1-seg.lnx1)+(lny1-seg.lny1)*(lny1-seg.lny1));
            double dist2=sqrt((lnx1-seg.lnx2)*(lnx1-seg.lnx2)+(lny1-seg.lny2)*(lny1-seg.lny2));
            if(dist1<=dist2)
            {
                toPoint.X=lnx1;toPoint.Y=lny1;
            }
            else
            {
                toPoint.X=lnx2;toPoint.Y=lny2;
            }
            if((fabs(toPoint.X-seg.lnx1)<=VertexTolerance)&&(fabs(toPoint.Y-seg.lny1)<=VertexTolerance)) return 2;//此时无法判断面内和面外
            if((fabs(fromPoint.X-seg.lnx1)<=VertexTolerance)&&(fabs(fromPoint.Y-seg.lny1)<=VertexTolerance)) return 2;
        }
        else if((IsFSegFromPtIn)&&(IsTSegFromPtIn))
        {
            if((fabs(fromPoint.X-seg.lnx1)<=VertexTolerance)&&(fabs(fromPoint.Y-seg.lny1)<=VertexTolerance)) return 2;
            if(IsPointOnSegment(fromPoint.X,fromPoint.Y,VertexTolerance)) return 2;
            Segment2D<T> dseg(fromPoint.X,fromPoint.Y,lnx1,lny1);
            if(dseg.IsPointOnSegment(lnx2,lny2)) return 2;
            double angle1=Triangle2D<T>::TriangleAngle(fromPoint,Point2D<T>(seg.lnx1,seg.lny1),Point2D<T>(lnx2,lny2));
            double angle2=Triangle2D<T>::TriangleAngle(fromPoint,Point2D<T>(seg.lnx1,seg.lny1),Point2D<T>(seg.lnx2,seg.lny2));
            if(fabs(angle1-angle2)<=VertexTolerance) return 2;
            if(angle1<angle2) return 3;
            return 6;
        }
        else if((IsFSegFromPtIn)&&(IsTSegToPtIn))
        {
            return 2;//此时仅能判断边缘有
        }
        else if((IsFSegToPtIn)&&(IsTSegFromPtIn))
        {
            if((fabs(fromPoint.X-seg.lnx1)<=VertexTolerance)&&(fabs(fromPoint.Y-seg.lny1)<=VertexTolerance)) return 2;
            if(IsPointOnSegment(fromPoint.X,fromPoint.Y)) return 2;
            Segment2D<T>  dseg(fromPoint.X,fromPoint.Y,lnx2,lny2);
            if(dseg.IsPointOnSegment(lnx1,lny1)) return 2;
            double angle1=Triangle2D<T>::TriangleAngle(fromPoint,Point2D<T>(seg.lnx1,seg.lny1),Point2D<T>(lnx1,lny1));
            double angle2=Triangle2D<T>::TriangleAngle(fromPoint,Point2D<T>(seg.lnx1,seg.lny1),Point2D<T>(seg.lnx2,seg.lny2));
            if(fabs(angle1-angle2)<=VertexTolerance) return 2;//此时仅能判断边缘有
            if(angle1<angle2) return 3;
            return 6;
        }
        else if((IsFSegToPtIn)&&(IsTSegToPtIn))
        {
            return 2;//此时仅能判断边缘有
        }
        else if(IsFSegFromPtIn)
        {
            toPoint.X=lnx2;toPoint.Y=lny2;
            intersectPoint.X=lnx1;intersectPoint.Y=lny1;
            fromPoint.X=seg.lnx1;
            fromPoint.Y=seg.lny1;
        }
        else if(IsFSegToPtIn)
        {
            toPoint.X=lnx1;toPoint.Y=lny1;
            intersectPoint.X=lnx2;intersectPoint.Y=lny2;
            fromPoint.X=seg.lnx1;
            fromPoint.Y=seg.lny1;
        }
        else if(IsTSegFromPtIn)
        {
            if((fabs(fromPoint.X-seg.lnx1)<=VertexTolerance)&&(fabs(fromPoint.Y-seg.lny1)<=VertexTolerance)) return 2;
            float f1=Triangle2D<T>::TriangleArea(lnx1,lny1,lnx2,lny2,fromPoint.X,fromPoint.Y);
            if(fabs(f1)>VertexTolerance) return 7;//之前的点与当前线段不在同一条直线上，直接判断为面内、边缘、面外均有
            //否则判断fromPoint、DBPoint(lnx1,lny1)、DBPoint(lnx2,lny2)三点在seg的左右关系
            f1=Triangle2D<T>::TriangleArea(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,fromPoint.X,fromPoint.Y);
            float f2=Triangle2D<T>::TriangleArea(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,lnx1,lny1);
            if(f1*f2<0)
            {
                toPoint.X=lnx1;toPoint.Y=lny1;
                //说明fromPoint和DBPoint(lnx1,lny1)在seg的异侧
            }
            else
            {
                toPoint.X=lnx2;toPoint.Y=lny2;
            }
            intersectPoint.X=seg.lnx1;intersectPoint.Y=seg.lny1;
        }
        else if(IsTSegToPtIn)
        {
            return 2;//此时仅能判断边缘有
        }
        else
        {
            if(!IntersectofSegment(seg,intersectPoint)) return 0;//没有交点
            return 7;//直接判断为面内、边缘、面外均有
        }
        double angle1=Triangle2D<T>::TriangleAngle(fromPoint,intersectPoint,toPoint);
        double angle2=Triangle2D<T>::TriangleAngle(fromPoint,intersectPoint,toPoint2);
        if(fabs(angle1-angle2)<=VertexTolerance) return 2;//此时仅能判断边缘有
        if(angle1<angle2) return 3;
        return 6;
    }
};

template<typename T>
struct SGIS_EXPORT Line2D{
    T lnx1;
    T lny1;
    T lnx2;
    T lny2;
    T Fuzzy;
    Line2D(){
        lnx1=lny1=lnx2=lny2=0;
    };
    Line2D(T x1,T y1,T x2,T y2){
        lnx1=x1;
        lny1=y1;
        lnx2=x2;
        lny2=y2;
    };
    Line2D(const Line2D&seg){
        lnx1=seg.lnx1;
        lny1=seg.lny1;
        lnx2=seg.lnx2;
        lny2=seg.lny2;
    };
    bool GetIntersection(const Line2D<T>&other,Point2D<T>&interPt){
        //a1=lnx1  b1=lny1  a2=lnx2 b2=lny2  c1=other.lnx1  d1=other.lny1  c2=other.lnx2  d2=other.lny2
        double d=((lny2-lny1)*(other.lnx2-other.lnx1)-(other.lny2-other.lny1)*(lnx2-lnx1));
        if(d==0) return false;
        interPt.X=((lnx2-lnx1)*(other.lnx2-other.lnx1)*(other.lny2-lny2)+(lny2-lny1)*(other.lnx2-other.lnx1)*lnx2-(other.lny2-other.lny1)*(lnx2-lnx1)*other.lnx2)/d;
        interPt.Y=(lny2-lny1)/(lnx2-lnx1)*(interPt.X-lnx2)+lny2;
        return true;
    };
};

template<typename T>
struct Triangle2D
{
public:
    static double TriangleAngle(const Point2D<T>&fromPoint,const Point2D<T>&centerPoint,const Point2D<T>&toPoint)
    {
        Segment2D<T> seg1(centerPoint.X,centerPoint.Y,fromPoint.X,fromPoint.Y);
        double angle1=seg1.Angle();
        Segment2D<T>seg2(centerPoint.X,centerPoint.Y,toPoint.X,toPoint.Y);
        double angle2=seg2.Angle();
        angle2-=angle1;
        if(angle2<0) angle2=DPI+angle2;
        return angle2;
    };
    static double TriangleArea(const Point2D<T>&fromPoint,const Point2D<T>&centerPoint,const Point2D<T>&toPoint)
    {
        return ((centerPoint.X-fromPoint.X)*(centerPoint.Y+fromPoint.Y)+(toPoint.X-centerPoint.X)*(toPoint.Y+centerPoint.Y)+(fromPoint.X-toPoint.X)*(fromPoint.Y+toPoint.Y))/2.0;
    };
    static double TriangleArea(double x1,double y1,double x2,double y2,double x3,double y3)
    {
        return ((x2-x1)*(y1+y2)+(x3-x2)*(y3+y2)+(x1-x3)*(y1+y3))/2.0;
    };
    static double TriangleState(const Point2D<T>&fromPoint,const Point2D<T>&centerPoint,const Point2D<T>&toPoint)
    {
        double a=((centerPoint.X-fromPoint.X)*(centerPoint.Y+fromPoint.Y)+(toPoint.X-centerPoint.X)*(toPoint.Y+centerPoint.Y)+(fromPoint.X-toPoint.X)*(fromPoint.Y+toPoint.Y));
        if(a>0) return 1;
        if(a<0) return -1;
        return 0;
    };
    static double TriangleState(double x1,double y1,double x2,double y2,double x3,double y3)
    {
        double a((x2-x1)*(y1+y2)+(x3-x2)*(y3+y2)+(x1-x3)*(y1+y3));
        if(a>0) return 1;
        if(a<0) return -1;
        return 0;
    };
};

template<typename T>
struct Rect2D
{
    T Left;
    T Bottom;
    T Right;
    T Top;
    Rect2D()
    {
        Left=Top=Right=Bottom=0;
    };
    Rect2D(T l,T t,T r,T b)
    {
        Left=l;
        Top=t;
        Right=r;
        Bottom=b;
    };
    Rect2D(const Rect2D<T>&rect)
    {
        Left=rect.Left;
        Top=rect.Top;
        Right=rect.Right;
        Bottom=rect.Bottom;
    };
    Rect2D(Envelope&env)
    {
        double l,t,r,b;
        env.GetCoord(&l,&t,&r,&b);
        Left=l;
        Top=t;
        Right=r;
        Bottom=b;
    };
    T Width()
    {
        return Right-Left;
    };
    T Height()
    {
        return Bottom-Top;
    };
    Rect2D<T>&operator =(const Rect2D<T>&other){
        this->Left=other.Left;
        this->Top=other.Top;
        this->Right=other.Right;
        this->Bottom=other.Bottom;
        return *this;
    };
    Rect2D<T>&operator =(Envelope&env){
        double l,t,r,b;
        env.GetCoord(&l,&t,&r,&b);
        Left=l;
        Top=t;
        Right=r;
        Bottom=b;
        return *this;
    };
    void ToEnvelope(Envelope&env){
        env.PutCoord(Left,Top,Right,Bottom);
    };
    Rect2D<T> operator + (const Rect2D<T>&Other)
    {
        Rect2D<T> rect;
        rect.Left=min(Left,Other.Left);
        rect.Right=max(Right,Other.Right);
        Rect2D<T> other=Other;
        other.NormalizeRect();
        if(Bottom<=Top)
        {
            rect.Top=max(Top,other.Top);
            rect.Bottom=min(Bottom,other.Bottom);
        }
        else
        {
            rect.Top=min(Top,other.Bottom);
            rect.Bottom=max(Bottom,other.Top);
        }
        return rect;
    };
    Rect2D<T>& operator += (const Rect2D<T>&Other)
    {
        Left=min(Left,Other.Left);
        Right=max(Right,Other.Right);
        Rect2D<T> other=Other;
        other.NormalizeRect();
        if(Bottom<=Top)
        {
            Top=max(Top,other.Top);
            Bottom=min(Bottom,other.Bottom);
        }
        else
        {
            Top=min(Top,other.Bottom);
            Bottom=max(Bottom,other.Top);
        }
        return *this;
    };
    Rect2D<T> Intersect(Rect2D<T> other)
    {
        bool IsNormal=(Top>=Bottom);
        Rect2D<T> pr(Left,Top,Right,Bottom);
        pr.NormalizeRect();
        Rect2D<T> rpr=pr;
        other.NormalizeRect();
        rpr.Left=max(pr.Left,other.Left);
        rpr.Top=min(pr.Top,other.Top);
        rpr.Right=min(pr.Right,other.Right);
        rpr.Bottom=max(pr.Bottom,other.Bottom);
        if(!IsNormal)
        {
            double t=rpr.Top;
            rpr.Top=rpr.Bottom;
            rpr.Bottom=t;
        }
        return rpr;
    };
    void InflateRect(T l,T t,T r,T b,bool IsRatio=false)
    {
        if(IsRatio)
        {
            l=l*(Right-Left);
            r=r*(Right-Left);
            t=t*fabs(Top-Bottom);
            b=b*fabs(Top-Bottom);
        }
        Left-=l;
        Right+=r;
        if(Top>Bottom)
        {
          Top+=t;
          Bottom-=b;
        }
        else
        {
          Top-=t;
          Bottom+=b;
        }
    };
    void NormalizeRect()
    {
        if(Left>Right)
        {
            double temp=Left;
            Left=Right;
            Right=temp;
        }
        if(Bottom>Top)
        {
            double temp=Bottom;
            Bottom=Top;
            Top=temp;
        }
    };
    void NormalizeClientRect()
    {
        if(Left>Right)
        {
            double temp=Left;
            Left=Right;
            Right=temp;
        }
        if(Bottom<Top)
        {
            double temp=Bottom;
            Bottom=Top;
            Top=temp;
        }
    };
    bool PtInRect(double X,double Y)
    {
        return ((X>=Left)&&(X<=Right)&&(Y>=min(Bottom,Top))&&(Y<=max(Bottom,Top)));
    };
    bool PtInRect(Point2D<T>&pt)
    {
        return ((pt.X>=Left)&&(pt.X<=Right)&&(pt.Y>=min(Bottom,Top))&&(pt.Y<=max(Bottom,Top)));
    };
    bool IntersectRect(const Rect2D<T>&other)
    {
        if((Left>other.Right)||(Right<other.Left)||(Top<other.Bottom)||(Bottom>other.Top)) return false;
        return true;
    };
    bool IntersectRectInClient(const Rect2D<T>&other)
    {
        if((Left>other.Right)||(Right<other.Left)||(Bottom<other.Top)||(Top>other.Bottom)) return false;
        return true;
    };
    bool WithInRect(const Rect2D<T>env){
        return ((Left>=env.Left)&&(Right<=env.Right)&&(Top<=env.Top)&&(Bottom>=env.Bottom));
    };
    bool WithInRectInClient(const Rect2D<T>env){
        return ((Left>=env.Left)&&(Right<=env.Right)&&(Top>=env.Top)&&(Bottom<=env.Bottom));
    };
    bool IntersectRect(Envelope*env)
    {
        double ol,ot,ori,ob;
        env->GetCoord(&ol,&ot,&ori,&ob);
        if((Left>ori)||(Right<ol)||(Top<ob)||(Bottom>ot)) return false;
        return true;
    };
    bool IntersectRectInClient(Envelope*env)
    {
        double ol,ot,ori,ob;
        env->GetCoord(&ol,&ot,&ori,&ob);
        if((Left>ori)||(Right<ol)||(Bottom<ot)||(Top>ob)) return false;
        return true;
    };
    bool IsPointIn(const Point2D<T>&dpt)
    {
        if((dpt.X<Left)||(dpt.X>Right)) return false;
        if((dpt.Y<min(Top,Bottom))||(dpt.Y>max(Top,Bottom))) return false;
        return true;
    };
    bool IsPointIn(double X,double Y)
    {
        if((X<Left)||(X>Right)) return false;
        if((Y<min(Top,Bottom))||(Y>max(Top,Bottom))) return false;
        return true;
    };
    bool IsRectIn(Rect2D<T>&other)
    {
        other.NormalizeRect();
        Rect2D<T> rt(Left,Top,Right,Bottom);
        rt.NormalizeRect();
        return ((other.Left>=rt.Left)&&(other.Right<=rt.Right)&&(other.Bottom>=rt.Bottom)&&(other.Top<=rt.Top));
    };
    bool Equals(const Rect2D<T>&other)
    {
        return ((other.Left==Left)&&(other.Right==Right)&&(other.Bottom==Bottom)&&(other.Top==Top));
    };
};

template<typename T>
struct Box{
    T Width;
    T Height;
    Box(){
        Width=Height=0;
    };
    Box(T w,T h){
        Width=w;
        Height=h;
    };
    Box(const Box&other){
        Width=other.Width;
        Height=other.Height;
    };
    Box&operator=(const Box&other){
        Width=other.Width;
        Height=other.Height;
        return *this;
    };
};

}
#endif // GEOMETRY2D_H
