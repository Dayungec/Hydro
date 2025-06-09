#include "segment.h"
#include "envelope.h"
#include "points.h"
#include "math.h"
#include "spatialreference.h"
#include "Base/classfactory.h"
namespace SGIS{

REGISTER(Segment)

Segment::Segment()
{
   this->lnx1=0;
   this->lny1=0;
   this->lnz1=0;
   this->lnx2=0;
   this->lny2=0;
   this->lnz2=0;
}
Segment::Segment(double lnx1,double lny1,double lnx2,double lny2,double lnz1,double lnz2){
    this->lnx1=lnx1;
    this->lny1=lny1;
    this->lnz1=lnz1;
    this->lnx2=lnx2;
    this->lny2=lny2;
    this->lnz2=lnz2;
}

Segment::Segment(const Segment&other){
    this->lnx1=other.lnx1;
    this->lny1=other.lny1;
    this->lnz1=other.lnz1;
    this->lnx2=other.lnx2;
    this->lny2=other.lny2;
    this->lnz2=other.lnz2;
}

Segment::~Segment(){

}

Segment&Segment::operator=(const Segment&other){
    this->lnx1=other.lnx1;
    this->lny1=other.lny1;
    this->lnz1=other.lnz1;
    this->lnx2=other.lnx2;
    this->lny2=other.lny2;
    this->lnz2=other.lnz2;
    return *this;
}
//IGeometry:
AutoPtr<Envelope>Segment::GetExtent(){
    AutoPtr<Envelope>env(new Envelope(lnx1,lny1,lnx2,lny2));
    env->Normalize();
    return env;
}
GeometryType Segment::GetType(){
    return gtSegment;
}
AutoPtr<Geometry>Segment::Clone(){
    return new Segment(lnx1,lny1,lnx2,lny2,lnz1,lnz2);
}

void Segment::CopyFromOther(Segment*other){
    lnx1=other->lnx1;
    lny1=other->lny1;
    lnx2=other->lnx2;
    lny2=other->lny2;
    lnz1=other->lnz1;
    lnz2=other->lnz2;
}

bool Segment::Equals(Geometry*other){
    if(GetType()!=other->GetType()) return false;
    Segment*gs=(Segment*)other;
    return ((lnx1==gs->lnx1)&&(lny1==gs->lny1)&&(lnx2==gs->lnx2)&&(lny2==gs->lny2));
}

bool Segment::IsIntersect(Points*points,bool IsRing)
{
    LONG ptCount=points->GetSize();
    if(ptCount<2) return false;
    Segment2D<double>seg(lnx1,lny1,lnx2,lny2);
    Segment2D<double>seg2;
    points->GetItem(0,seg2.lnx1,seg2.lny1);
    int state;
    Point2D<double>intersectpt;
    double fuzzy=VertexTolerance;
    if(IsRing)
    {
        for(int k=1;k<=ptCount;k++)
        {
            if(k<ptCount)
                points->GetItem(k,seg2.lnx2,seg2.lny2);
            else
                points->GetItem(0,seg2.lnx2,seg2.lny2);
            if((fabs(seg2.lnx1-seg2.lnx2)<=fuzzy)&&(fabs(seg2.lny1-seg2.lny2)<=fuzzy)) continue;
            if(seg.IntersectofSegment(seg2,intersectpt)) return true;
            seg2.lnx1=seg2.lnx2;
            seg2.lny1=seg2.lny2;
        }
    }
    else
    {
        for(int k=1;k<ptCount;k++)
        {
            points->GetItem(k,seg2.lnx2,seg2.lny2);
            if((fabs(seg2.lnx1-seg2.lnx2)<=fuzzy)&&(fabs(seg2.lny1-seg2.lny2)<=fuzzy)) continue;
            if(seg.IntersectofSegment(seg2,intersectpt)) return true;
            seg2.lnx1=seg2.lnx2;
            seg2.lny1=seg2.lny2;
        }
    }
    return true;
}

bool Segment::Intersects(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return IsPointOnSegment(ppt->X(),ppt->Y(),VertexTolerance);
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        Segment2D<double> seg1(lnx1,lny1,lnx2,lny2);
        Segment2D<double> seg2;
        pSeg->GetCoord(&seg2.lnx1,&seg2.lny1,&seg2.lnx2,&seg2.lny2);
        Point2D<double> intersectpt;
        return seg1.IntersectofSegment(seg2,intersectpt);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        DOUBLE xmin,ymin,xmax,ymax;
        pEnv->GetCoord(&xmin,&ymax,&xmax,&ymin);
        if((lnx1>=xmin)&&(lnx1<=xmax)&&(lny1>=ymin)&&(lny1<=ymax)) return true;
        if((lnx2>=xmin)&&(lnx2<=xmax)&&(lny2>=ymin)&&(lny2<=ymax)) return true;
        Segment2D<double> seg(lnx1,lny1,lnx2,lny2);
        Point2D<double> intersectpt;
        Segment2D<double> seg2;
        seg2.lnx1=xmin;seg2.lny1=ymax;seg2.lnx2=xmax;seg2.lny2=ymax;
        if(seg.IntersectofSegment(seg2,intersectpt)) return true;
        seg2.lnx1=xmax;seg2.lny1=ymax;seg2.lnx2=xmax;seg2.lny2=ymin;
        if(seg.IntersectofSegment(seg2,intersectpt)) return true;
        seg2.lnx1=xmax;seg2.lny1=ymin;seg2.lnx2=xmin;seg2.lny2=ymin;
        if(seg.IntersectofSegment(seg2,intersectpt)) return true;
        seg2.lnx1=xmin;seg2.lny1=ymin;seg2.lnx2=xmin;seg2.lny2=ymax;
        if(seg.IntersectofSegment(seg2,intersectpt)) return true;
        return false;
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        DOUBLE W,H;
        W=pElli->Width();
        H=pElli->Height();
        if((lnx1/W)*(lnx1/W)+(lny1/H)*(lny1/H)<=1+VertexTolerance) return true;
        if((lnx2/W)*(lnx2/W)+(lny2/H)*(lny2/H)<=1+VertexTolerance) return true;
        AutoPtr<Polygon>poly;
        poly=pElli->ConvertToPolygon(true);
        AutoPtr<Ring>pRing;
        pRing=poly->GetItem(0);
        AutoPtr<Points>points=pRing;
        return IsIntersect(points.get(),true);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=ppts->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn) return false;
        LONG ptCount=ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            if(IsPointOnSegment(ox,oy)) return true;
        }
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polyline->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polyline>pply;
        pply=Geometry::ConvertToPolyline(polyline.get());
        LONG partCount=pply->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath;
            pPath=pply->GetItem(k);
            AutoPtr<Points>points=pPath;
            if(IsIntersect(points.get(),false)) return true;
        }
        return false;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polygon->GetExtent();
        AutoPtr<Envelope>segEnv=GetExtent();
        bool IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>ppoly;
        ppoly=ConvertToPolygon(polygon.get());
        AutoPtr<Point>ppt(new Point());
        ppt->PutCoord(lnx1,lny1);
        IsIn=ppoly->IsPointIn(ppt.get());
        if(IsIn) return true;
        ppt->PutCoord(lnx2,lny2);
        IsIn=ppoly->IsPointIn(ppt.get());
        if(IsIn) return true;
        LONG partCount;
        partCount=ppoly->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Ring>pPath;
            pPath=ppoly->GetItem(k);
            AutoPtr<Points>points=pPath;
            if(IsIntersect(points.get(),true)) return true;
        }
        return false;
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polygon->GetExtent();
        AutoPtr<Envelope>segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount=mpolygon->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            poly=mpolygon->GetItem(k);
            IsIn=Intersects(poly.get());
            if(IsIn)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=NULL)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo=pCol->GetItem(k);
            IsInter=Intersects(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}
bool Segment::Disjoint(Geometry*other){
    return (!Intersects(other));
}

int Segment::GetRingIntersectState(Points*points)
{
    LONG ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg(lnx1,lny1,lnx2,lny2);
    Segment2D<double> seg2;
    points->GetItem(0,seg2.lnx1,seg2.lny1);
    Point2D<double>formerPoint;
    int state=0;
    for(int k=ptCount-1;k>=0;k--)
    {
        points->GetItem(k,seg2.lnx2,seg2.lny2);
        if((fabs(seg2.lnx1-seg2.lnx2)<=VertexTolerance)&&(fabs(seg2.lny1-seg2.lny2)<=VertexTolerance)) continue;
        break;
    }
    formerPoint.X=seg2.lnx2;
    formerPoint.Y=seg2.lny2;
    for(int k=1;k<=ptCount;k++)
    {
        if(k<ptCount)
            points->GetItem(k,seg2.lnx2,seg2.lny2);
        else
            points->GetItem(0,seg2.lnx2,seg2.lny2);
        if((fabs(seg2.lnx1-seg2.lnx2)<=VertexTolerance)&&(fabs(seg2.lny1-seg2.lny2)<=VertexTolerance)) continue;
        int s=seg.GetRingSegmentIntersectState(seg2,formerPoint);
        state=(state|s);
        if(state&4) return state;
        formerPoint.X=seg2.lnx1;
        formerPoint.Y=seg2.lny1;
        seg2.lnx1=seg2.lnx2;
        seg2.lny1=seg2.lny2;
    }
    return state;
}

int Segment::GetTouchState(Points*points)
{
    LONG ptCount;
    ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg(lnx1,lny1,lnx2,lny2);
    Segment2D<double> seg2;
    points->GetItem(0,seg2.lnx1,seg2.lny1);
    int state;
    int formers=0;
    int s=0;
    double fuzzy=VertexTolerance;
    Point2D<double>formerPoint(seg2.lnx1,seg2.lny1);
    for(int k=1;k<ptCount;k++)
    {
        points->GetItem(k,seg2.lnx2,seg2.lny2);
        if((fabs(seg2.lnx1-seg2.lnx2)<=fuzzy)&&(fabs(seg2.lny1-seg2.lny2)<=fuzzy)) continue;
        s=seg.GetSegmentIntersectState(seg2);
        if((s==5)||(s==6))
        {
            DOUBLE fx,fy;
            fx=formerPoint.X;
            fy=formerPoint.Y;
            float f1=(seg.lny2-seg.lny1)*fx+(seg.lnx1-seg.lnx2)*fy+seg.lnx2*seg.lny1-seg.lnx1*seg.lny2;
            float f2=(seg.lny2-seg.lny1)*seg2.lnx2+(seg.lnx1-seg.lnx2)*seg2.lny2+seg.lnx2*seg.lny1-seg.lnx1*seg.lny2;
            if(f1*f2<0) return 3;
            s=1;
        }
        if(s==1)
        {
            if(state==0) state=1;
        }
        else if(s==2)
        {
            state=2;
        }
        else if(s==3)
        {
            return 3;
        }
        formerPoint.X=seg2.lnx1;
        formerPoint.Y=seg2.lny1;
        seg2.lnx1=seg2.lnx2;
        seg2.lny1=seg2.lny2;
        formers=s;
    }
    return state;
}
int Segment::GetCrossState(Points*points)
{
    LONG ptCount;
    ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg(lnx1,lny1,lnx2,lny2);
    Segment2D<double> seg2;
    points->GetItem(0,seg2.lnx1,seg2.lny1);
    int state;
    int formers=0;
    int s=0;
    double fuzzy=VertexTolerance;
    Point2D<double>formerPoint(seg2.lnx1,seg2.lny1);
    for(int k=1;k<ptCount;k++)
    {
        points->GetItem(k,seg2.lnx2,seg2.lny2);
        if((fabs(seg2.lnx1-seg2.lnx2)<=fuzzy)&&(fabs(seg2.lny1-seg2.lny2)<=fuzzy)) continue;
        s=seg.GetSegmentIntersectState(seg2);
        if((s==5)||(s==6))
        {
            DOUBLE fx,fy;
            fx=formerPoint.X;
            fy=formerPoint.Y;
            float f1=(seg.lny2-seg.lny1)*fx+(seg.lnx1-seg.lnx2)*fy+seg.lnx2*seg.lny1-seg.lnx1*seg.lny2;
            float f2=(seg.lny2-seg.lny1)*seg2.lnx2+(seg.lnx1-seg.lnx2)*seg2.lny2+seg.lnx2*seg.lny1-seg.lnx1*seg.lny2;
            if(f1*f2<0)
                s=3;
            else
                s=1;
        }
        if(s==1)
        {
            if(state==0) state=1;
        }
        else if(s==2)
        {
            return 2;
        }
        else if(s==3)
        {
            state=3;
        }
        formerPoint.X=seg2.lnx1;
        formerPoint.Y=seg2.lny1;
        seg2.lnx1=seg2.lnx2;
        seg2.lny1=seg2.lny2;
        formers=s;
    }
    return state;
}

bool Segment::Touches(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return IsPointOnSegment(ppt->X(),ppt->Y(),VertexTolerance);
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        Segment2D<double> seg1(lnx1,lny1,lnx2,lny2);
        Segment2D<double> seg2;
        pSeg->GetCoord(&seg2.lnx1,&seg2.lny1,&seg2.lnx2,&seg2.lny2);
        Point2D<double> intersectpt;
        int state=seg1.GetSegmentIntersectState(seg2);
        return ((state==1)||(state>=4));
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        bool IsF,IsT;
        DOUBLE xmin,ymin,xmax,ymax;
        pEnv->GetCoord(&xmin,&ymax,&xmax,&ymin);
        IsF=((lnx1>=xmin)&&(lnx1<=xmax)&&(lny1>=ymin)&&(lny1<=ymax));
        IsT=((lnx2>=xmin)&&(lnx2<=xmax)&&(lny2>=ymin)&&(lny2<=ymax));
        Segment2D<double> seg(lnx1,lny1,lnx2,lny2);
        Segment2D<double> seg2;
        seg2.lnx1=xmin;seg2.lny1=ymax;seg2.lnx2=xmax;seg2.lny2=ymax;
        int state=seg.GetSegmentIntersectState(seg2);
        if(state>=4) state=1;
        if(((1==state)&&(IsF^IsT))||(state==2))
        {
            return true;
        }
        seg2.lnx1=xmax;seg2.lny1=ymax;seg2.lnx2=xmax;seg2.lny2=ymin;
        state=seg.GetSegmentIntersectState(seg2);
        if(state>=4) state=1;
        if(((1==state)&&(IsF^IsT))||(state==2))
        {
            return true;
        }
        seg2.lnx1=xmax;seg2.lny1=ymin;seg2.lnx2=xmin;seg2.lny2=ymin;
        state=seg.GetSegmentIntersectState(seg2);
        if(state>=4) state=1;
        if(((1==state)&&(IsF^IsT))||(state==2))
        {
            return true;
        }
        seg2.lnx1=xmin;seg2.lny1=ymin;seg2.lnx2=xmin;seg2.lny2=ymax;
        state=seg.GetSegmentIntersectState(seg2);
        if(state>=4) state=1;
        if(((1==state)&&(IsF^IsT))||(state==2))
        {
            return true;
        }
        return false;
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        AutoPtr<Polygon>poly;
        poly=pElli->ConvertToPolygon(true);
        AutoPtr<Ring>pRing;
        pRing=poly->GetItem(0);
        AutoPtr<Points>points=pRing;
        int state=GetRingIntersectState(points.get());
        if(state&1)
            return false;
        else
            return (state&2);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=ppts->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG ptCount;
        ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            if(IsPointOnSegment(ox,oy))
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polyline->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        AutoPtr<Polyline>pply;
        pply=Geometry::ConvertToPolyline(polyline.get());
        LONG partCount;
        partCount=pply->GetSize();
        bool HasSuc=false;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath;
            pPath=pply->GetItem(k);
            AutoPtr<Points>points=pPath;
            int state=GetTouchState(points.get());
            if(state==1)
                HasSuc=true;
            else if(state>1)
            {
                return false;
            }
        }
        return HasSuc;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        AutoPtr<Polygon>ppoly;
        ppoly=ConvertToPolygon(polygon.get());
        LONG partCount;
        partCount=ppoly->GetSize();
        bool HasBorder=false;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Ring>pRing;
            pRing=ppoly->GetItem(k);
            AutoPtr<Points>points=pRing;
            int state=GetRingIntersectState(points.get());
            if(state&1)
            {
                return false;
            }
            if(state&2) HasBorder=true;
        }
        return HasBorder;
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount;
        partCount=mpolygon->GetSize();
        bool IsInter;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            poly=mpolygon->GetItem(k);
            IsInter=Touches(poly.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=NULL)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Touches(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}
bool Segment::Crosses(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        Segment2D<double> seg1(lnx1,lny1,lnx2,lny2);
        Segment2D<double> seg2;
        pSeg->GetCoord(&seg2.lnx1,&seg2.lny1,&seg2.lnx2,&seg2.lny2);
        int state=seg1.GetSegmentIntersectState(seg2);
        return (state==3);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        bool IsF,IsT;
        DOUBLE xmin,ymin,xmax,ymax;
        pEnv->GetCoord(&xmin,&ymax,&xmax,&ymin);
        IsF=((lnx1>=xmin)&&(lnx1<=xmax)&&(lny1>=ymin)&&(lny1<=ymax));
        IsT=((lnx2>=xmin)&&(lnx2<=xmax)&&(lny2>=ymin)&&(lny2<=ymax));
        if(IsF&&IsT)
        {
            return false;
        }
        Segment2D<double> seg(lnx1,lny1,lnx2,lny2);
        Segment2D<double> seg2;
        seg2.lnx1=xmin;seg2.lny1=ymax;seg2.lnx2=xmax;seg2.lny2=ymax;
        int state=seg.GetSegmentIntersectState(seg2);
        if(state==3)
        {
            return true;
        }
        seg2.lnx1=xmax;seg2.lny1=ymax;seg2.lnx2=xmax;seg2.lny2=ymin;
        state=seg.GetSegmentIntersectState(seg2);
        if(state==3)
        {
            return true;
        }
        seg2.lnx1=xmax;seg2.lny1=ymin;seg2.lnx2=xmin;seg2.lny2=ymin;
        state=seg.GetSegmentIntersectState(seg2);
        if(state==3)
        {
            return true;
        }
        seg2.lnx1=xmin;seg2.lny1=ymin;seg2.lnx2=xmin;seg2.lny2=ymax;
        state=seg.GetSegmentIntersectState(seg2);
        if(state==3)
        {
            return true;
        }
        return false;
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        AutoPtr<Polygon>poly;
        poly=pElli->ConvertToPolygon(true);
        AutoPtr<Ring>pRing;
        pRing=poly->GetItem(0);
        AutoPtr<Points>points=pRing;
        int state=GetRingIntersectState(points.get());
        return (state==7);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polyline->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        AutoPtr<Polyline>pply;
        pply=Geometry::ConvertToPolyline(polyline.get());
        LONG partCount;
        partCount=pply->GetSize();
        bool HasSuc=false;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath;
            pPath=pply->GetItem(k);
            AutoPtr<Points>points=pPath;
            int state=GetCrossState(points.get());
            if(state==3)
                HasSuc=true;
            else if(state==2)
            {
                return false;
            }
        }
        return HasSuc;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        AutoPtr<Polygon>ppoly;
        ppoly=ConvertToPolygon(polygon.get());
        LONG partCount;
        partCount=ppoly->GetSize();
        int state=0;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Ring>pRing;
            pRing=ppoly->GetItem(k);
            AutoPtr<Points>points=pRing;
            int s=GetRingIntersectState(points.get());
            if(s==7)
            {
                return true;
            }
            state=(state|s);
        }
        return (state==7);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount;
        partCount=mpolygon->GetSize();
        bool IsInter;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            poly=mpolygon->GetItem(k);
            IsInter=Crosses(poly.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=NULL)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Crosses(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}
bool Segment::IsInPoints(Points*points)
{
    LONG ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg(lnx1,lny1,lnx2,lny2);
    Segment2D<double> seg2;
    points->GetItem(0,seg2.lnx1,seg2.lny1);
    int posf=-1;
    int post=-1;
    for(int k=1;k<ptCount;k++)
    {
        points->GetItem(k,seg2.lnx2,seg2.lny2);
        if((lnx1>=min(seg2.lnx1,seg2.lnx2)-VertexTolerance)&&(lnx1<=max(seg2.lnx1,seg2.lnx2)+VertexTolerance)&&(lny1>=min(seg2.lny1,seg2.lny2)-VertexTolerance)&&(lny1<=max(seg2.lny1,seg2.lny2)+VertexTolerance))
        {
            if(seg2.IsPointOnSegment(lnx1,lny1,VertexTolerance)) posf=k-1;
        }
        if((lnx2>=min(seg2.lnx1,seg2.lnx2)-VertexTolerance)&&(lnx2<=max(seg2.lnx1,seg2.lnx2)+VertexTolerance)&&(lny2>=min(seg2.lny1,seg2.lny2)-VertexTolerance)&&(lny2<=max(seg2.lny1,seg2.lny2)+VertexTolerance))
        {
            if(seg2.IsPointOnSegment(lnx2,lny2,VertexTolerance)) post=k-1;
        }
        seg2.lnx1=seg2.lnx2;
        seg2.lny1=seg2.lny2;
    }
    if((posf==-1)||(post==-1)) return false;
    if(posf>post)
    {
        int temp=posf;
        posf=post;
        post=posf;
    }
    for(int k=posf+1;k<=post;k++)
    {
        points->GetItem(k,seg2.lnx2,seg2.lny2);
        if(!seg.IsPointOnSegment(seg2.lnx2,seg2.lny2,VertexTolerance)) return false;
    }
    return true;
}
bool Segment::Within(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        Segment2D<double> seg1(lnx1,lny1,lnx2,lny2);
        Segment2D<double> seg2;
        pSeg->GetCoord(&seg2.lnx1,&seg2.lny1,&seg2.lnx2,&seg2.lny2);
        return seg2.ContainSegment(seg1);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        bool IsF,IsT;
        DOUBLE xmin,ymin,xmax,ymax;
        pEnv->GetCoord(&xmin,&ymax,&xmax,&ymin);
        IsF=((lnx1>=xmin)&&(lnx1<=xmax)&&(lny1>=ymin)&&(lny1<=ymax));
        IsT=((lnx2>=xmin)&&(lnx2<=xmax)&&(lny2>=ymin)&&(lny2<=ymax));
        return (IsF&&IsT);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        DOUBLE W,H;
        W=pElli->Width();
        H=pElli->Height();
        bool IsF;
        bool IsT;
        IsF=((lnx1/W)*(lnx1/W)+(lny1/H)*(lny1/H)<=1+VertexTolerance);
        IsT=((lnx2/W)*(lnx2/W)+(lny2/H)*(lny2/H)<=1+VertexTolerance);
        return (IsF&&IsT);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polyline->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Contains(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        AutoPtr<Polyline>pply;
        pply=Geometry::ConvertToPolyline(polyline.get());
        LONG partCount;
        partCount=pply->GetSize();
        bool HasSuc=false;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath;
            pPath=pply->GetItem(k);
            AutoPtr<Points>points=pPath;
            if(IsInPoints(points.get()))
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn=pEnv->Contains(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        AutoPtr<Polygon>ppoly;
        ppoly=ConvertToPolygon(polygon.get());
        LONG partCount;
        partCount=ppoly->GetSize();
        int state=0;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Ring>pRing;
            pRing=ppoly->GetItem(k);
            AutoPtr<Points>points=pRing;
            int s=GetRingIntersectState(points.get());
            if(s==0)
            {
                AutoPtr<Point>ppt(new Point());
                ppt->PutCoord(lnx1,lny1);
                bool IsPointIn=pRing->IsPointIn(ppt.get());
                if(IsPointIn)
                    s=((k==0)?1:4);
                else
                    s=((k==0)?4:1);
            }
            if(s&4)
            {
                return false;
            }
        }
        return true;
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn=pEnv->Contains(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount;
        partCount=mpolygon->GetSize();
        bool IsInter;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            poly=mpolygon->GetItem(k);
            IsInter=Within(poly.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=NULL)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Within(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}
bool Segment::Contains(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return IsPointOnSegment(ppt->X(),ppt->Y(),VertexTolerance);
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        Segment2D<double> seg1(lnx1,lny1,lnx2,lny2);
        Segment2D<double> seg2;
        pSeg->GetCoord(&seg2.lnx1,&seg2.lny1,&seg2.lnx2,&seg2.lny2);
        return seg1.ContainSegment(seg2);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return false;
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return false;
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=ppts->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn=pEnv->Within(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG ptCount;
        ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            if(!IsPointOnSegment(ox,oy))
            {
                return false;
            }
        }
        return true;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polyline->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn=pEnv->Within(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        AutoPtr<Polyline>pply;
        pply=Geometry::ConvertToPolyline(polyline.get());
        LONG partCount;
        partCount=pply->GetSize();
        bool HasSuc=false;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath;
            pPath=pply->GetItem(k);
            AutoPtr<Points>points=pPath;
            LONG ptCount;
            ptCount=points->GetSize();
            bool IsPtOn;
            for(int p=0;p<ptCount;p++)
            {
                DOUBLE X,Y;
                points->GetItem(p,X,Y);
                IsPtOn=IsPointOnSegment(X,Y);
                if(!IsPtOn)
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        return false;
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=NULL)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Contains(pGeo.get());
            if(!IsInter)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}
int Segment::GetIntersectState(Points*points)
{
    LONG ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg(lnx1,lny1,lnx2,lny2);
    Segment2D<double> seg2;
    points->GetItem(0,seg2.lnx1,seg2.lny1);
    int state;
    int formers=0;
    int s=0;
    double fuzzy=VertexTolerance;
    Point2D<double> formerPoint(seg2.lnx1,seg2.lny1);
    for(int k=1;k<ptCount;k++)
    {
        points->GetItem(k,seg2.lnx2,seg2.lny2);
        if((fabs(seg2.lnx1-seg2.lnx2)<=fuzzy)&&(fabs(seg2.lny1-seg2.lny2)<=fuzzy)) continue;
        s=seg.GetSegmentIntersectState(seg2);
        if((s==5)||(s==6))
        {
            float f1=Triangle2D<double>::TriangleArea(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,formerPoint.X,formerPoint.Y);
            float f2=Triangle2D<double>::TriangleArea(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,seg2.lnx2,seg2.lny2);
            if(f1*f2<0) return 3;
            s=1;
        }
        if(s==1)
        {
            state=1;
        }
        else if(s==2)
        {
            if(state==0) state=2;
        }
        else if(s==3)
        {
            return 3;
        }
        formerPoint.X=seg2.lnx1;
        formerPoint.Y=seg2.lny1;
        seg2.lnx1=seg2.lnx2;
        seg2.lny1=seg2.lny2;
        formers=s;
    }
    return state;
}
bool Segment::Overlaps(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        Segment2D<double> seg1(lnx1,lny1,lnx2,lny2);
        Segment2D<double> seg2;
        pSeg->GetCoord(&seg2.lnx1,&seg2.lny1,&seg2.lnx2,&seg2.lny2);
        Point2D<double> intersectpt;
        return seg1.IntersectofSegment(seg2,intersectpt);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return false;
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return false;
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polyline->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        AutoPtr<Polyline>pply;
        pply=Geometry::ConvertToPolyline(polyline.get());
        LONG partCount;
        partCount=pply->GetSize();
        bool HasSuc=false;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath;
            pPath=pply->GetItem(k);
            AutoPtr<Points>points=pPath;
            int state=GetIntersectState(points.get());
            if(state>0)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        return false;
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=NULL)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Overlaps(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}

void Segment::AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY){
    DOUBLE X,Y;
    AnchorPoint->GetCoord(&X,&Y);
    double difx,dify;
    difx=DiffX+X;
    dify=DiffY+Y;
    if(RotateAngle!=0)
    {
        double px,py;
        double cosv=cos(RotateAngle);
        double sinv=sin(RotateAngle);
        px=lnx1-X;
        py=lny1-Y;
        lnx1=(px*cosv-py*sinv)*ZoomXRatio+difx;
        lny1=(px*sinv+py*cosv)*ZoomYRatio+dify;
        px=lnx2-X;
        py=lny2-Y;
        lnx2=(px*cosv-py*sinv)*ZoomXRatio+difx;
        lny2=(px*sinv+py*cosv)*ZoomYRatio+dify;
    }
    else
    {
        lnx1=(lnx1-X)*ZoomXRatio+difx;
        lny1=(lny1-Y)*ZoomYRatio+dify;
        lnx2=(lnx2-X)*ZoomXRatio+difx;
        lny2=(lny2-Y)*ZoomYRatio+dify;
    }
}
bool Segment::Project(CoordinateTransformation*pTrans){
   pTrans->TransformXY(&lnx1,&lny1);
   pTrans->TransformXY(&lnx2,&lny2);
   return true;
}

void Segment::Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr){
    lnx1*=xr;
    lny1*=yr;
    lnx2*=xr;
    lny2*=yr;
    lnz1*=zr;
    lnz2*=zr;
}
void Segment::Move(DOUBLE difx,DOUBLE dify,DOUBLE difz){
    lnx1+=difx;
    lny1+=dify;
    lnx2+=difx;
    lny2+=dify;
    lnz1+=difz;
    lnz2+=difz;
}
//BufferWritable:
bool Segment::Read(BufferReader*br){
    lnx1=br->Read<double>();
    lny1=br->Read<double>();
    lnx2=br->Read<double>();
    lny2=br->Read<double>();
    return true;
}
void Segment::Write(BufferWriter*bw){
     bw->Write<double>(lnx1);
     bw->Write<double>(lny1);
     bw->Write<double>(lnx2);
     bw->Write<double>(lny2);
}
string Segment::GetTypeName(){
    return "Segment";
}
//JsonWritable:
bool Segment::ReadFromJson(JsonReader*jr,JsonNode node){
    if(node.size()<2) return false;
    JsonNode childNode=node[0];
    if(childNode.size()<2) return false;
    JsonNode childNode2=node[1];
    if(childNode2.size()<2) return false;
    lnx1=childNode[0].asDouble();
    lny1=childNode[1].asDouble();
    lnx2=childNode2[0].asDouble();
    lny2=childNode2[1].asDouble();
    return true;
}

string Segment::ToJson(){
    JsonArrayWriter writer;
    writer.Add<double>(lnx1);
    writer.Add<double>(lny1);
    writer.Add<double>(lnx2);
    writer.Add<double>(lny2);
    return writer.GetJsonString();
}
//Geometry3D:
AutoPtr<Envelope>Segment::GetExtent3D(){
    AutoPtr<Envelope>env(new Envelope(lnx1,lny1,lnx2,lny2,lnz1,lnz2));
    env->Normalize();
    return env;
}

void Segment::Rotate(DOUBLE RotateAngle,Vector*axis){
    double rx,ry,rz;
    axis->GetCoord(&rx,&ry,&rz);
    double L=sqrt(rx*rx+ry*ry+rz*rz);
    RotateAngle=RotateAngle/180*PI;
    FLOAT cosa=cos(RotateAngle);
    FLOAT sina=sin(RotateAngle);
    if(L!=0)
    {
        rx=rx/L;
        ry=ry/L;
        rz=rz/L;
    }
    innerRotate(sina,cosa,rx,ry,rx,lnx1,lny1,lnz1);
    innerRotate(sina,cosa,rx,ry,rx,lnx2,lny2,lnz2);
}
bool Segment::Read3D(BufferReader*br){
    lnx1=br->Read<double>();
    lny1=br->Read<double>();
    lnx2=br->Read<double>();
    lny2=br->Read<double>();
    lnz1=br->Read<double>();
    lnz2=br->Read<double>();
    return true;
}
void Segment::Write3D(BufferWriter*bw){
    bw->Write<double>(lnx1);
    bw->Write<double>(lny1);
    bw->Write<double>(lnx2);
    bw->Write<double>(lny2);
    bw->Write<double>(lnz1);
    bw->Write<double>(lnz2);
}
bool Segment::ReadFromJson3D(JsonReader*jr,JsonNode node){
    if(node.size()<2) return false;
    JsonNode childNode=node[0];
    if(childNode.size()<2) return false;
    JsonNode childNode2=node[1];
    if(childNode2.size()<2) return false;
    lnx1=childNode[0].asDouble();
    lny1=childNode[1].asDouble();
    if(childNode.size()>2)
        lnz1=childNode[2].asDouble();
    else
        lnz1=0;
    lnx2=childNode2[0].asDouble();
    lny2=childNode2[1].asDouble();
    if(childNode2.size()>2)
        lnz2=childNode2[2].asDouble();
    else
        lnz2=0;
    return true;
}
string Segment::ToJson3D(){
   return "[[" + to_string(lnx1) + "," + to_string(lny1)+ "," + to_string(lnz1)  + "],[" + to_string(lnx2) + "," + to_string(lny2)+ "," + to_string(lnz2)  + "]]";
}

double&Segment::Lx1(){
    return lnx1;
}
double&Segment::Ly1(){
    return lny1;
}
double&Segment::Lx2(){
    return lnx2;
}
double&Segment::Ly2(){
    return lny2;
}
void Segment::GetCoord(double*lnx1,double*lny1,double*lnx2,double*lny2){
    *lnx1=this->lnx1;
    *lny1=this->lny1;
    *lnx2=this->lnx2;
    *lny2=this->lny2;
}
void Segment::PutCoord(double lnx1,double lny1,double lnx2,double lny2){
    this->lnx1=lnx1;
    this->lny1=lny1;
    this->lnx2=lnx2;
    this->lny2=lny2;
}

AutoPtr<Point>Segment::FromPoint(){
    return AutoPtr<Point>(new Point(lnx1,lny1));
}
AutoPtr<Point>Segment::ToPoint(){
    return AutoPtr<Point>(new Point(lnx2,lny2));
}

double Segment::Angle(){
    if (lnx1 == lnx2) {
        if (lny1 > lny2)
            return PI * 3 / 2;
        if (lny1 == lny2)
            return 0;
        return PI / 2;
    }
    else {
        double temp = atan((lny2 - lny1) / (lnx2 - lnx1));
        if (lnx2 < lnx1)
            temp = temp + PI;
        if (temp < 0)
            temp = temp + PI * 2;
        return temp;
    }
}

double Segment::GetLength(){
    return sqrt((lnx1-lnx2)*(lnx1-lnx2)+(lny1-lny2)*(lny1-lny2));
}

double Segment::GetLength3D(){
    return sqrt((lnx1-lnx2)*(lnx1-lnx2)+(lny1-lny2)*(lny1-lny2)+(lnz1-lnz2)*(lnz1-lnz2));
}

double Segment::DistanceToSegment(Segment*seg){
    Segment* seg1=this;
    Segment* seg2=seg;
    if((seg1->lnx1==seg1->lnx2)&&(seg1->lny1==seg1->lny2)){
        AutoPtr<Point>pt(new Point(seg1->lnx1,seg1->lny1));
        double dist= pt->DistanceToSegment(seg2);
        return dist;
    }
    if((seg2->lnx1==seg2->lnx2)&&(seg2->lny1==seg2->lny2)){
        AutoPtr<Point>pt(new Point(seg2->lnx1,seg2->lny1));
        double dist=pt->DistanceToSegment(seg1);
        return dist;
    }
    double r_top=(seg1->lny1-seg2->lny1)*(seg2->lnx2-seg2->lnx1)-(seg1->lnx1-seg2->lnx1)*(seg2->lny2-seg2->lny1);
    double r_bot=(seg1->lnx2-seg1->lnx1)*(seg2->lny2-seg2->lny1)-(seg1->lny2-seg1->lny1)*(seg2->lnx2-seg2->lnx1);
    double s_top=(seg1->lny1-seg2->lny1)*(seg1->lnx2-seg1->lnx1)-(seg1->lnx1-seg2->lnx1)*(seg1->lny2-seg1->lny1);
    double s_bot=(seg1->lnx2-seg1->lnx1)*(seg2->lny2-seg2->lny1)-(seg1->lny2-seg1->lny1)*(seg2->lnx2-seg2->lnx1);
    double s=s_top/s_bot;
    double r=r_top/r_bot;
    if ((r_bot==0)||(s_bot==0)||((r<0)||( r>1)||(s<0)||(s>1))) {
        AutoPtr<Point>pt=seg1->FromPoint();
        double dist=pt->DistanceToSegment(seg2);
        pt->PutCoord(seg1->lnx2,seg1->lny2);
        dist=min(dist,pt->DistanceToSegment(seg2));
        pt->PutCoord(seg2->lnx1,seg2->lny1);
        dist=min(dist,pt->DistanceToSegment(seg1));
        pt->PutCoord(seg2->lnx2,seg2->lny2);
        dist=min(dist,pt->DistanceToSegment(seg1));
        return dist;
    }
    return 0.0;
}

bool Segment::IntersectsSegment(Segment*seg){
    AutoPtr<Envelope>env=seg->GetExtent();
    if(!this->GetExtent()->IntersectsEnvelope(env.get())) return false;
    AutoPtr<Point>point(new Point(seg->lnx1,seg->lny1));
    int Pq1=RobustDeterminant::OrientationIndex(this,point.get());
    point->PutCoord(seg->lnx2,seg->lny2);
    int Pq2=RobustDeterminant::OrientationIndex(this,point.get());
    if ((Pq1>0 && Pq2>0) || (Pq1<0 && Pq2<0))  return false;
    point->PutCoord(lnx1,lny1);
    int Qp1=RobustDeterminant::OrientationIndex(seg,point.get());
    point->PutCoord(lnx2,lny2);
    int Qp2=RobustDeterminant::OrientationIndex(seg,point.get());
    if ((Qp1>0 && Qp2>0)||(Qp1<0 && Qp2<0)) return false;
    bool collinear=(Pq1==0 && Pq2==0 && Qp1==0 && Qp2==0);
    if (collinear) {
        point->PutCoord(lnx1,lny1);
        if(point->IntersectsSegment(seg)) return true;
        point->PutCoord(lnx2,lny2);
        if(point->IntersectsSegment(seg)) return true;
        point->PutCoord(seg->lnx1,seg->lny1);
        if(point->IntersectsSegment(this)) return true;
        point->PutCoord(seg->lnx2,seg->lny2);
        if(point->IntersectsSegment(this)) return true;
        return false;
    }
    return true;
}

bool Segment::IntersectsEnvelope(Envelope*env){
    AutoPtr<Envelope>ext=this->GetExtent();
    if(!ext->IntersectsEnvelope(env)) return false;
    AutoPtr<Point>pt(new Point(lnx1,lny1));
    if(env->IsPointIn(pt.get())) return true;
    pt->PutCoord(lnx2,lny2);
    if(env->IsPointIn(pt.get())) return true;
    double left,top,right,bottom;
    env->GetCoord(&left,&top,&right,&bottom);
    AutoPtr<Segment>seg(new Segment(left,top,right,top));
    if(seg->IntersectsSegment(this)) return true;
    seg->PutCoord(right,top,right,bottom);
    if(seg->IntersectsSegment(this)) return true;
    seg->PutCoord(right,bottom,left,bottom);
    if(seg->IntersectsSegment(this)) return true;
    seg->PutCoord(left,bottom,left,top);
    if(seg->IntersectsSegment(this)) return true;
    return false;
}

bool Segment::IsPointOnSegment(double x,double y,double toler){
    if(toler>0){
        AutoPtr<Point>ppt(new Point(x,y));
        return (ppt->DistanceToSegment(this)<=toler);
    }
    double minx=lnx1;if(lnx2<minx) minx=lnx2;
    double maxx=lnx1;if(lnx2>minx) maxx=lnx2;
    double miny=lny1;if(lny2<miny) miny=lny2;
    double maxy=lny1;if(lny2>miny) maxy=lny2;
    if((x>=minx)&&(x<=maxx)&&(y<=maxy)&&(y<=maxy)){

        if((RobustDeterminant::OrientationIndex(lnx1,lny1,lnx2,lny2,x,y)==0)&&(RobustDeterminant::ReverseOrientationIndex(lnx1,lny1,lnx2,lny2,x,y)==0)){
            return true;
        }
    }
    return false;
}

bool Segment::EnvelopeIntersect(AutoPtr<Point>p1,AutoPtr<Point>p2,AutoPtr<Point>q){
    double x1,y1,x2,y2;
    p1->GetCoord(&x1,&y1);
    p2->GetCoord(&x2,&y2);
    double minx=x1;if(x2<minx) minx=x2;
    double maxx=x1;if(x2>maxx) maxx=x2;
    double miny=y1;if(y2<miny) miny=y2;
    double maxy=y1;if(y2>maxy) maxy=y2;
    double x,y;
    q->GetCoord(&x,&y);
    return ((x>=minx)&&(x<=maxx)&&(y>=miny)&&(y<=maxy));
}

bool Segment::computeCollinearIntersection(AutoPtr<Point>p1,AutoPtr<Point>p2,AutoPtr<Point>q1,AutoPtr<Point>q2,vector<AutoPtr<Point>>&intersectPoints){
    bool p1q1p2=EnvelopeIntersect(p1,p2,q1);
    bool p1q2p2=EnvelopeIntersect(p1,p2,q2);
    bool q1p1q2=EnvelopeIntersect(q1,q2,p1);
    bool q1p2q2=EnvelopeIntersect(q1,q2,p2);
    if (p1q1p2 && p1q2p2) {
        intersectPoints.push_back(q1);
        if(!q1->Equals(q2.get())) intersectPoints.push_back(q2);
        return true;
    }
    if (q1p1q2 && q1p2q2) {
        intersectPoints.push_back(p1);
        if(!p1->Equals(p2.get())) intersectPoints.push_back(p2);
        return true;
    }
    if (p1q1p2 && q1p1q2) {
        intersectPoints.push_back(q1);
        if(!q1->Equals(p1.get())) intersectPoints.push_back(p1);
        return true;
    }
    if (p1q1p2 && q1p2q2) {
        intersectPoints.push_back(q1);
        if(!q1->Equals(p2.get())) intersectPoints.push_back(p2);
        return true;
    }
    if (p1q2p2 && q1p1q2) {
        intersectPoints.push_back(q2);
        if(!q2->Equals(p1.get())) intersectPoints.push_back(p1);
        return true;
    }
    if (p1q2p2 && q1p2q2) {
        intersectPoints.push_back(q2);
        if(!q2->Equals(p2.get())) intersectPoints.push_back(p2);
        return true;
    }
    return false;
}

void Segment::normalizeToEnvCentre(AutoPtr<Point>n00,AutoPtr<Point>n01,AutoPtr<Point>n10,AutoPtr<Point>n11,AutoPtr<Point>normPt){
    double n00x,n00y;
    n00->GetCoord(&n00x,&n00y);
    double n01x,n01y;
    n01->GetCoord(&n01x,&n01y);
    double n10x,n10y;
    n10->GetCoord(&n10x,&n10y);
    double n11x,n11y;
    n11->GetCoord(&n11x,&n11y);
    double minX0 = n00x < n01x ? n00x : n01x;
    double minY0 = n00y < n01y ? n00y : n01y;
    double maxX0 = n00x > n01x ? n00x : n01x;
    double maxY0 = n00y > n01y ? n00y : n01y;

    double minX1 = n10x < n11x ? n10x : n11x;
    double minY1 = n10y < n11y ? n10y : n11y;
    double maxX1 = n10x > n11x ? n10x : n11x;
    double maxY1 = n10y > n11y ? n10y : n11y;

    double intMinX = minX0 > minX1 ? minX0 : minX1;
    double intMaxX = maxX0 < maxX1 ? maxX0 : maxX1;
    double intMinY = minY0 > minY1 ? minY0 : minY1;
    double intMaxY = maxY0 < maxY1 ? maxY0 : maxY1;

    double intMidX = (intMinX + intMaxX) / 2.0;
    double intMidY = (intMinY + intMaxY) / 2.0;
    normPt->PutCoord(intMidX,intMidY);
    n00->PutCoord(n00x-intMidX,n00y-intMidY);
    n01->PutCoord(n01x-intMidX,n01y-intMidY);
    n10->PutCoord(n10x-intMidX,n10y-intMidY);
    n11->PutCoord(n11x-intMidX,n11y-intMidY);
}

bool Segment::safeHCoordinateIntersection(AutoPtr<Point>p1,AutoPtr<Point>p2,AutoPtr<Point>q1,AutoPtr<Point>q2,AutoPtr<Point>ret){
    double p1x,p1y;
    p1->GetCoord(&p1x,&p1y);
    double p2x,p2y;
    p2->GetCoord(&p2x,&p2y);
    double q1x,q1y;
    q1->GetCoord(&q1x,&q1y);
    double q2x,q2y;
    q2->GetCoord(&q2x,&q2y);
    double px = p1y - p2y;
    double py = p2x - p1x;
    double pw = p1x * p2y - p2x * p1y;
    double qx = q1y - q2y;
    double qy = q2x - q1x;
    double qw = q1x * q2y - q2x * q1y;
    double w = px * qy - qx * py;
    if(w==0) return false;
    double x = py * qw - qy * pw;
    double y = qx * pw - px * qw;
    double xInt = x/w;
    double yInt = y/w;
    ret->PutCoord(xInt,yInt);
    return true;
}

AutoPtr<Point>Segment::nearestEndpoint(AutoPtr<Point>p1,AutoPtr<Point>p2,AutoPtr<Point>q1,AutoPtr<Point>q2){
    AutoPtr<Segment>seg(new Segment(q1->X(),q1->Y(),q1->X(),q1->Y()));
    AutoPtr<Point>ret=p1;
    double dist=p1->DistanceToSegment(seg.get());
    if(dist>p2->DistanceToSegment(seg.get())) ret=p2;
    seg->PutCoord(p1->X(),p1->Y(),p1->X(),p1->Y());
    if(dist>q1->DistanceToSegment(seg.get())) ret=q1;
    if(dist>q2->DistanceToSegment(seg.get())) ret=q2;
    return ret;
}

void Segment::intersection(AutoPtr<Point>p1,AutoPtr<Point>p2,AutoPtr<Point>q1,AutoPtr<Point>q2,vector<AutoPtr<Point>>&intersectPoints){
    AutoPtr<Point>normPt(new Point());
    normalizeToEnvCentre(p1,p2,q1,q2, normPt);
    AutoPtr<Point> intPtOut(new Point());
    if(safeHCoordinateIntersection(p1,p2,q1,q2,intPtOut)) {
        (*intPtOut)+=(*normPt);
        intersectPoints.push_back(intPtOut);
    }
    else
    {
        AutoPtr<Point>nearpt=nearestEndpoint(p1,p2,q1,q2);
        intPtOut->CopyFromOther(nearpt.get());
        intersectPoints.push_back(intPtOut);
    }
}

vector<AutoPtr<Point>>Segment::IntersectSegment(Segment*seg){
    vector<AutoPtr<Point>>intersectPoints;
    AutoPtr<Envelope>env=seg->GetExtent();
    if(!this->GetExtent()->IntersectsEnvelope(env.get())) return intersectPoints;
    double segx1,segy1,segx2,segy2;
    seg->GetCoord(&segx1,&segy1,&segx2,&segy2);
    AutoPtr<Point>point(new Point(segx1,segy1));
    int Pq1=RobustDeterminant::OrientationIndex(this,point.get());
    point->PutCoord(segx2,segy2);
    int Pq2=RobustDeterminant::OrientationIndex(this,point.get());
    if ((Pq1>0 && Pq2>0) || (Pq1<0 && Pq2<0))  return intersectPoints;
    point->PutCoord(lnx1,lny1);
    int Qp1=RobustDeterminant::OrientationIndex(seg,point.get());
    point->PutCoord(lnx2,lny2);
    int Qp2=RobustDeterminant::OrientationIndex(seg,point.get());
    if ((Qp1>0 && Qp2>0)||(Qp1<0 && Qp2<0)) return intersectPoints;
    bool collinear=(Pq1==0 && Pq2==0 && Qp1==0 && Qp2==0);
    if (collinear) {
        AutoPtr<Point>p1(new Point(lnx1,lny1));
        AutoPtr<Point>p2(new Point(lnx2,lny2));
        AutoPtr<Point>q1(new Point(segx1,segy1));
        AutoPtr<Point>q2(new Point(segx2,segy2));
        if(computeCollinearIntersection(p1,p2,q1,q2,intersectPoints)) return intersectPoints;
        intersectPoints.clear();
        return intersectPoints;
    }
    if (Pq1==0 || Pq2==0 || Qp1==0 || Qp2==0) {
        if(((lnx1==segx1)&&(lny1==segy1))||((lnx1==segx2)&&(lny1==segy2))){
            intersectPoints.push_back(AutoPtr<Point>(new Point(lnx1,lny1)));
        }
        else if(((lnx2==segx1)&&(lny2==segy1))||((lnx2==segx2)&&(lny2==segy2))){
            intersectPoints.push_back(AutoPtr<Point>(new Point(lnx2,lny2)));
        }
        else if(((segx1==lnx1)&&(segy1==lny1))||((segx1==lnx2)&&(segy1==lny2))){
            intersectPoints.push_back(AutoPtr<Point>(new Point(segx1,segy1)));
        }
        else if(((segx2==lnx1)&&(segy2==lny1))||((segx2==lnx2)&&(segy2==lny2))){
            intersectPoints.push_back(AutoPtr<Point>(new Point(segx2,segy2)));
        }
        else if (Pq1==0) {
            intersectPoints.push_back(AutoPtr<Point>(new Point(segx1,segy1)));
        }
        else if (Pq2==0) {
            intersectPoints.push_back(AutoPtr<Point>(new Point(segx2,segy2)));
        }
        else if (Qp1==0) {
            intersectPoints.push_back(AutoPtr<Point>(new Point(lnx1,lny1)));
        }
        else if (Qp2==0) {
            intersectPoints.push_back(AutoPtr<Point>(new Point(lnx2,lny2)));
        }
    } else {
        AutoPtr<Point> p1(new Point(lnx1,lny1));
        AutoPtr<Point> p2 (new Point(lnx2,lny2));
        AutoPtr<Point> q1 (new Point(segx1,segy1));
        AutoPtr<Point> q2  (new Point(segx2,segy2));
        intersection(p1, p2, q1, q2, intersectPoints);
        return intersectPoints;
    }
    return intersectPoints;
}

vector<AutoPtr<Point>>Segment::IntersectEnvelope(Envelope*env){
    AutoPtr<Envelope>rect(new Envelope(lnx1,lny1,lnx2,lny2));
    double envtop,envbottom,envleft,envright;
    envtop=env->Top();
    envbottom=env->Bottom();
    if(lnx1>lnx2){
        rect->Left()=lnx2;
        rect->Right()=lnx1;
    }
    if(envtop<envbottom){
        if(lny1>lny2){
           rect->Top()=lny2;
           rect->Bottom()=lny1;
        }
        if(!env->IntersectsEnvelopeInClient(rect.get())) return vector<AutoPtr<Point>>();
    }
    else{
        if(lny2>lny1){
           rect->Top()=lny2;
           rect->Bottom()=lny1;
        }
        if(!env->IntersectsEnvelope(rect.get())) return vector<AutoPtr<Point>>();
    }
    env->GetCoord(&envleft,&envtop,&envright,&envbottom);
    AutoPtr<Segment>otherSeg(new Segment(envleft,envtop,envright,envtop));

    vector<AutoPtr<Point>>intersectPoints=IntersectSegment(otherSeg.get());
    if(intersectPoints.size()>=2) return intersectPoints;
    otherSeg->Lx1()=env->Right();
    otherSeg->Ly1()=env->Bottom();
    vector<AutoPtr<Point>>pts=IntersectSegment(otherSeg.get());
    if(pts.size()>=2) return pts;
    for(int k=0;k<pts.size();k++){
        if((pts[k]->X()==env->Right())&&(pts[k]->Y()==env->Top())) continue;
        intersectPoints.push_back(pts[k]);
        if(intersectPoints.size()>=2) return intersectPoints;
    }
    otherSeg->Lx2()=env->Left();
    otherSeg->Ly2()=env->Bottom();
    pts=IntersectSegment(otherSeg.get());
    if(pts.size()>=2) return pts;
    for(int k=0;k<pts.size();k++){
        if((pts[k]->X()==env->Right())&&(pts[k]->Y()==env->Bottom())) continue;
        intersectPoints.push_back(pts[k]);
        if(intersectPoints.size()>=2) return intersectPoints;
    }
    otherSeg->Lx1()=env->Left();
    otherSeg->Ly1()=env->Top();
    pts=IntersectSegment(otherSeg.get());
    if(pts.size()>=2) return pts;
    for(int k=0;k<pts.size();k++){
        if((pts[k]->X()==env->Left()&&(pts[k]->Y()==env->Bottom()))) continue;
        intersectPoints.push_back(pts[k]);
        if(intersectPoints.size()>=2) return intersectPoints;
    }
    return intersectPoints;
}

void Segment::Reverse(){
    double temp=lnx1;
    lnx1=lnx2;
    lnx2=temp;
    temp=lny1;
    lny1=lny2;
    lny2=temp;
}

AutoPtr<Polyline>Segment::ConvertToPolyline(){
    AutoPtr<Points>pts(new Points());
    pts->Add(lnx1,lny1);
    pts->Add(lnx2,lny2);
    AutoPtr<Points>pPath=pts;
    AutoPtr<Polyline>poly(new Polyline());
    poly->Add(pPath);
    return poly;
}

}
