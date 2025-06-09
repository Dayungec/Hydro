#include "geometrybase.h"
#include "Point.h"
#include "envelope.h"
#include "points.h"
#include "ellipse.h"
#include "polyline.h"
#include "polygon.h"
#include "multipolygon.h"
#include "segment.h"
#include "math.h"
#include "Base/classfactory.h"
#include "spatialreference.h"
namespace SGIS{

int RobustDeterminant::SignOfDet2x2(double x1,double y1,double x2,double y2){
    // returns -1 if the determinant is negative,
    // returns  1 if the determinant is positive,
    // retunrs  0 if the determinant is null.
    int sign=1;
    double swap;
    int k;
    if ((x1==0.0) || (y2==0.0)) {
        if ((y1==0.0) || (x2==0.0)) {
            return 0;
        } else if (y1>0) {
            if (x2>0) {
                return -sign;
            } else {
                return sign;
            }
        } else {
            if (x2>0) {
                return sign;
            } else {
                return -sign;
            }
        }
    }
    if ((y1==0.0) || (x2==0.0)) {
        if (y2>0) {
            if (x1>0) {
                return sign;
            } else {
                return -sign;
            }
        } else {
            if (x1>0) {
                return -sign;
            } else {
                return sign;
            }
        }
    }

    /*
            *  making y coordinates positive and permuting the entries
            *  so that y2 is the biggest one
            */
    if (0.0<y1) {
        if (0.0<y2) {
            if (y1<=y2) {
                ;
            } else {
                sign=-sign;
                swap=x1;
                x1=x2;
                x2=swap;
                swap=y1;
                y1=y2;
                y2=swap;
            }
        } else {
            if (y1<=-y2) {
                sign=-sign;
                x2=-x2;
                y2=-y2;
            } else {
                swap=x1;
                x1=-x2;
                x2=swap;
                swap=y1;
                y1=-y2;
                y2=swap;
            }
        }
    } else {
        if (0.0<y2) {
            if (-y1<=y2) {
                sign=-sign;
                x1=-x1;
                y1=-y1;
            } else {
                swap=-x1;
                x1=x2;
                x2=swap;
                swap=-y1;
                y1=y2;
                y2=swap;
            }
        } else {
            if (y1>=y2) {
                x1=-x1;
                y1=-y1;
                x2=-x2;
                y2=-y2;
            } else {
                sign=-sign;
                swap=-x1;
                x1=-x2;
                x2=swap;
                swap=-y1;
                y1=-y2;
                y2=swap;
            }
        }
    }
    if (0.0<x1) {
        if (0.0<x2) {
            if (x1 <= x2) {
                ;
            } else {
                return sign;
            }
        } else {
            return sign;
        }
    } else {
        if (0.0<x2) {
            return -sign;
        } else {
            if (x1 >= x2) {
                sign=-sign;
                x1=-x1;
                x2=-x2;
            } else {
                return -sign;
            }
        }
    }
    while (true) {
        k=floor(x2/x1);
        x2=x2-k*x1;
        y2=y2-k*y1;
        if (y2<0.0) {
            return -sign;
        }
        if (y2>y1) {
            return sign;
        }
        if (x1>x2+x2) {
            if (y1<y2+y2) {
                return sign;
            }
        } else {
            if (y1>y2+y2) {
                return -sign;
            } else {
                x2=x1-x2;
                y2=y1-y2;
                sign=-sign;
            }
        }
        if (y2==0.0) {
            if (x2==0.0) {
                return 0;
            } else {
                return -sign;
            }
        }
        if (x2==0.0) {
            return sign;
        }
        k=floor(x1/x2);
        x1=x1-k*x2;
        y1=y1-k*y2;
        if (y1<0.0) {
            return sign;
        }
        if (y1>y2) {
            return -sign;
        }
        if (x2>x1+x1) {
            if (y2<y1+y1) {
                return -sign;
            }
        } else {
            if (y2>y1+y1) {
                return sign;
            } else {
                x1=x2-x1;
                y1=y2-y1;
                sign=-sign;
            }
        }
        if (y1==0.0) {
            if (x1==0.0) {
                return 0;
            } else {
                return sign;
            }
        }
        if (x1==0.0) {
            return -sign;
        }
    }
}

int RobustDeterminant::OrientationIndex(double lnx1,double lny1,double lnx2,double lny2,double x,double y){
    return SignOfDet2x2(lnx2-lnx1,lny2-lny1,x-lnx2,y-lny2);
}

int RobustDeterminant::ReverseOrientationIndex(double lnx1,double lny1,double lnx2,double lny2,double x,double y){
    return SignOfDet2x2(lnx1-lnx2,lny1-lny2,x-lnx1,y-lny1);
}

int RobustDeterminant::OrientationIndex(const Segment&seg,const Point&point){
    double lnx1,lny1,lnx2,lny2;
    ((Segment*)&seg)->GetCoord(&lnx1,&lny1,&lnx2,&lny2);
    double x,y;
    ((Point*)&point)->GetCoord(&x,&y);
    return OrientationIndex(lnx1,lny1,lnx2,lny2,x,y);
}

int RobustDeterminant::ReverseOrientationIndex(const Segment&seg,const Point&point){
    double lnx1,lny1,lnx2,lny2;
    ((Segment*)&seg)->GetCoord(&lnx1,&lny1,&lnx2,&lny2);
    double x,y;
    ((Point*)&point)->GetCoord(&x,&y);
    return ReverseOrientationIndex(lnx1,lny1,lnx2,lny2,x,y);
}

int RobustDeterminant::OrientationIndex(Segment*seg,Point*point){
    double lnx1,lny1,lnx2,lny2;
    seg->GetCoord(&lnx1,&lny1,&lnx2,&lny2);
    double x,y;
    point->GetCoord(&x,&y);
    return OrientationIndex(lnx1,lny1,lnx2,lny2,x,y);
}

int RobustDeterminant::ReverseOrientationIndex(Segment*seg,Point*point){
    double lnx1,lny1,lnx2,lny2;
    seg->GetCoord(&lnx1,&lny1,&lnx2,&lny2);
    double x,y;
    point->GetCoord(&x,&y);
    return ReverseOrientationIndex(lnx1,lny1,lnx2,lny2,x,y);
}

REGISTER(Point)

Point::Point(){
    x=y=z=0;
}

Point::Point(double x,double y,double z){
    this->x=x;
    this->y=y;
    this->z=z;
}

Point::Point(const Point&other){
    this->x=other.x;
    this->y=other.y;
    this->z=other.z;
}
Point::~Point(){

}
AutoPtr<Envelope>Point::GetExtent(){
   return AutoPtr<Envelope>(new Envelope(x,y,x,y));
}
GeometryType Point::GetType(){
    return gtPoint;
}

bool Point::Equals(Geometry*other){
    if(GetType()!=this->GetType()) return false;
    Point*gt=(Point*)other;
    return ((x==gt->x)&&(y==gt->y));
}

bool Point::Intersects(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        DOUBLE ox,oy;
        ppt->GetCoord(&ox,&oy);
        return ((fabs(x-ox)<=VertexTolerance)&&(fabs(y-oy)<=VertexTolerance));
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->IsPointOnSegment(x,y,VertexTolerance);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->IsPointIn(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        DOUBLE W,H;
        W=pElli->Width();
        H=pElli->Height();
        return ((x/W)*(x/W)+(y/H)*(y/H)<=1+VertexTolerance);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        if(!pEnv->IsPointIn(this)) return false;
        LONG ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            if((fabs(x-ox)<=VertexTolerance)&&(fabs(y-oy)<=VertexTolerance)) return true;
        }
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv(polyline->GetExtent());
        if(!pEnv->IsPointIn(this)) return false;
        bool IsOn=polyline->CanCaptureBorder(this,VertexTolerance);
        return IsOn;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv(polygon->GetExtent());
        if(!pEnv->IsPointIn(this)) return false;
        return polygon->IsPointIn(this);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv(polyline->GetExtent());
        if(!pEnv->IsPointIn(this)) return false;
        return mpolygon->IsPointIn(this);
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo=pCol->GetItem(k);
            IsInter=Intersects(pGeo.get());
            if(IsInter) return true;
        }
        return false;
    }
    return false;
}
bool Point::Disjoint(Geometry*other){
    return (!Intersects(other));
}
bool Point::Touches(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        DOUBLE ox,oy;
        ppt->GetCoord(&ox,&oy);
        return ((fabs(x-ox)<=VertexTolerance)&&(fabs(y-oy)<=VertexTolerance));
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->IsPointOnSegment(ppt->X(),ppt->Y(),VertexTolerance);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        DOUBLE x1,y1,x2,y2;
        pEnv->GetCoord(&x1,&y1,&x2,&y2);
        Segment2D<double>seg;
        seg.lnx1=x1;seg.lny1=y1;seg.lnx2=x2;seg.lny2=y1;
        if(seg.IsPointOnSegment(x,y)) return true;
        seg.lnx1=x2;seg.lny1=y1;seg.lnx2=x2;seg.lny2=y2;
        if(seg.IsPointOnSegment(x,y)) return true;
        seg.lnx1=x2;seg.lny1=y2;seg.lnx2=x1;seg.lny2=y2;
        if(seg.IsPointOnSegment(x,y)) return true;
        seg.lnx1=x1;seg.lny1=y2;seg.lnx2=x1;seg.lny2=y1;
        if(seg.IsPointOnSegment(x,y)) return true;
        return false;
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        DOUBLE W,H;
        W=pElli->Width();
        H=pElli->Height();
        return (fabs((x/W)*(x/W)+(y/H)*(y/H)-1)<=VertexTolerance);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        if(!pEnv->IsPointIn(this)) return false;
        LONG ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            if((fabs(x-ox)<=VertexTolerance)&&(fabs(y-oy)<=VertexTolerance)) return true;
        }
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        if(!pEnv->IsPointIn(this)) return false;
        return polyline->CanCaptureBorder(this,VertexTolerance);
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        if(!pEnv->IsPointIn(this)) return false;
        return polygon->CanCaptureBorder(this,VertexTolerance);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        if(!pEnv->IsPointIn(this)) return false;
        return mpolygon->CanCaptureBorder(this,VertexTolerance);
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=NULL)
    {
        LONG subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo=pCol->GetItem(k);
            IsInter=Touches(pGeo.get());
            if(IsInter) return true;
        }
        return false;
    }
    return false;
}
bool Point::Crosses(Geometry*other){
    return false;
}
bool Point::Within(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        DOUBLE ox,oy;
        ppt->GetCoord(&ox,&oy);
        return ((fabs(x-ox)<=VertexTolerance)&&(fabs(y-oy)<=VertexTolerance));
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->IsPointOnSegment(x,y,VertexTolerance);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->IsPointIn(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        DOUBLE W,H;
        W=pElli->Width();
        H=pElli->Height();
        return ((x/W)*(x/W)+(y/H)*(y/H)<=1+VertexTolerance);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        if(!pEnv->IsPointIn(this)) return false;
        LONG ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            if((fabs(x-ox)<=VertexTolerance)&&(fabs(y-oy)<=VertexTolerance))
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polyline->GetExtent();
        if(!pEnv->IsPointIn(this)) return false;
        return polyline->CanCaptureBorder(this,VertexTolerance);
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polygon->GetExtent();
        if(!pEnv->IsPointIn(this)) return false;
        return polygon->IsPointIn(this);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=NULL)
    {
        AutoPtr<Envelope>pEnv=mpolygon->GetExtent();
        if(!pEnv->IsPointIn(this)) return false;
        return mpolygon->IsPointIn(this);
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=NULL)
    {
        LONG subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo=pCol->GetItem(k);
            if(Within(pGeo.get())) return true;
        }
        return false;
    }
    return false;
}
bool Point::Contains(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        DOUBLE ox,oy;
        ppt->GetCoord(&ox,&oy);
        return ((fabs(x-ox)<=VertexTolerance)&&(fabs(y-oy)<=VertexTolerance));
    }
    return false;
}
bool Point::Overlaps(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        DOUBLE ox,oy;
        ppt->GetCoord(&ox,&oy);
        return ((fabs(x-ox)<=VertexTolerance)&&(fabs(y-oy)<=VertexTolerance));
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        if(!pEnv->IsPointIn(this)) return false;
        LONG ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            if((fabs(x-ox)<=VertexTolerance)&&(fabs(y-oy)<=VertexTolerance))
            {
                return true;
            }
        }
        return false;
    }
    return false;
}
void Point::CopyFromOther(Point*other){
    x=other->x;
    y=other->y;
    z=other->z;
}
double Point::distanceToPath(Points*path){
    int count=path->GetSize();
    if(count<1) return 0;
    double x1,y1,x2,y2;
    double*xys=path->GetXYs();
    x1=xys[0];y1=xys[1];
    if(count==1){
        return sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
    }
    double mindist;
    int minindex=-1;
    AutoPtr<Segment>seg(new Segment());
    for(int k=1;k<count;k++){
        x2=xys[k<<1];y2=xys[(k<<1)+1];
        seg->PutCoord(x1,y1,x2,y2);
        double dist=DistanceToSegment(seg.get());
        if(minindex==-1){
            minindex=k;
            mindist=dist;
        }
        else if(dist<mindist){
            minindex=k;
            mindist=dist;
        }
        x1=x2;
        y1=y2;
    }
    return mindist;
}
double Point::distanceToRing(Points*ring){
    if(ring->IsPointIn(this)) return 0;
    int count=ring->GetSize();
    if(count<3) return 0;
    double x1,y1,x2,y2;
    double*xys=ring->GetXYs();
    x1=xys[0];y1=xys[1];
    double mindist;
    int minindex=-1;
    AutoPtr<Segment>seg(new Segment());
    for(int k=1;k<=count;k++){
        if(k<count){
           x2=xys[k<<1];y2=xys[(k<<1)+1];
        }
        else{
            x2=xys[0];y2=xys[1];
        }
        seg->PutCoord(x1,y1,x2,y2);
        double dist=DistanceToSegment(seg.get());
        if(minindex==-1){
            minindex=k;
            mindist=dist;
        }
        else if(dist<mindist){
            minindex=k;
            mindist=dist;
        }
        x1=x2;
        y1=y2;
    }
    return mindist;
}

double Point::DistanceTo(Geometry*geo){
    GeometryType type=geo->GetType();
    switch(type)
    {
    case gtPoint:
    {
        Point*ppt=(Point*)geo;
        return sqrt((x-ppt->x)*(x-ppt->x)+(y-ppt->y)*(y-ppt->y));
    }
    case gtPoints:
    {
        Points*ppts=(Points*)geo;
        int count=ppts->GetSize();
        double mindist=0;
        int minindex=-1;
        double ox,oy;
        for(int k=0;k<count;k++){
            ppts->GetItem(k,ox,oy);
            double dist=sqrt((x-ox)*(x-ox)+(y-oy)*(y-oy));
            if(minindex==-1){
                minindex=k;
                mindist=dist;
            }
            else if(dist<mindist){
                minindex=k;
                mindist=dist;
            }
        }
        return mindist;
    }
    case gtEnvelope:
    {
        Envelope*env=(Envelope*)geo;
        if(env->IsPointIn(this)) return 0;
        AutoPtr<Segment>seg(new Segment());
        double left,top,right,bottom;
        env->GetCoord(&left,&top,&right,&bottom);
        seg->PutCoord(left,top,right,top);
        double dist=this->DistanceToSegment(seg.get());
        seg->PutCoord(right,top,right,bottom);
        dist=min(dist,this->DistanceToSegment(seg.get()));
        seg->PutCoord(right,bottom,left,bottom);
        dist=min(dist,this->DistanceToSegment(seg.get()));
        seg->PutCoord(left,bottom,left,top);
        dist=min(dist,this->DistanceToSegment(seg.get()));
        return dist;
    }
    case gtEllipse:
    {
        Ellipse*elli=(Ellipse*)geo;
        AutoPtr<Points>pts=elli->ReturnPoints(false);
        return distanceToRing(pts.get());
    }
    case gtSegment:
    {
        Segment*seg=(Segment*)geo;
        return DistanceToSegment(seg);
    }
    case gtPolyline:
    {
        Polyline*poly=(Polyline*)geo;
        int count=poly->GetSize();
        double mindist;
        int minindex=-1;
        for(int k=0;k<count;k++){
            AutoPtr<Points>pts=poly->GetItem(k);
            double dist=distanceToPath(pts.get());
            if(minindex==-1){
                minindex=k;
                mindist=dist;
            }
            else if(dist<mindist){
                minindex=k;
                mindist=dist;
            }
        }
        return mindist;
    }
    case gtPolygon:
    {
        Polygon*poly=(Polygon*)geo;
        if(poly->IsPointIn(this)) return 0;
        int count=poly->GetSize();
        double mindist;
        int minindex=-1;
        for(int k=0;k<count;k++){
            AutoPtr<Points>pts=poly->GetItem(k);
            if(pts->GetSize()==0) continue;
            double dist=distanceToPath(pts.get());
            if(minindex==-1){
                minindex=k;
                mindist=dist;
            }
            else if(dist<mindist){
                minindex=k;
                mindist=dist;
            }
        }
        return mindist;
    }
    case gtMultiPolygon:
    {
        MultiPolygon*mpoly=(MultiPolygon*)geo;
        if(mpoly->IsPointIn(this)) return 0;
        int count=mpoly->GetSize();
        double mindist;
        int minindex=-1;
        for(int k=0;k<count;k++){
            AutoPtr<Polygon>poly=mpoly->GetItem(k);
            int subcount=poly->GetSize();
            for(int j=0;j<subcount;j++){
                AutoPtr<Points>pts=poly->GetItem(j);
                double dist=distanceToPath(pts.get());
                if(minindex==-1){
                    minindex=k;
                    mindist=dist;
                }
                else if(dist<mindist){
                    minindex=k;
                    mindist=dist;
                }
            }
        }
        return mindist;
    }
    case gtCollection:
    {
        GeometryCollection*geoCol=(GeometryCollection*)geo;
        int count=geoCol->GetSize();
        double mindist;
        int minindex=-1;
        for(int k=0;k<count;k++){
            AutoPtr<Geometry>subgeo=geoCol->GetItem(k);
            double dist=DistanceTo(subgeo.get());
            if(minindex==-1){
                minindex=k;
                mindist=dist;
            }
            else if(dist<mindist){
                minindex=k;
                mindist=dist;
            }
        }
        return mindist;
    }
    }
}

AutoPtr<Geometry>Point::Clone(){
    return new Point(x,y,z);
}

void Point::AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY){
    DOUBLE X,Y;
    AnchorPoint->GetCoord(&X,&Y);
    if(RotateAngle!=0)
    {
        double cosv=cos(RotateAngle);
        double sinv=sin(RotateAngle);
        double px=x-X;
        double py=y-Y;
        x=(px*cosv-py*sinv)*ZoomXRatio+DiffX+X;
        y=(px*sinv+py*cosv)*ZoomYRatio+DiffY+Y;
    }
    else
    {
        x=(x-X)*ZoomXRatio+DiffX+X;
        y=(y-Y)*ZoomYRatio+DiffY+Y;
    }
}

bool Point::Project(CoordinateTransformation*pTrans){
    return pTrans->TransformPoint(this);
}

void Point::Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr){
    x=x*xr;
    y=y*yr;
    z=z*zr;
}
void Point::Move(DOUBLE difx,DOUBLE dify,DOUBLE difz){
    x+=difx;
    y+=dify;
    z+=difz;
}

bool Point::Read(BufferReader*br){
    x=br->Read<double>();
    y=br->Read<double>();
    return true;
}

void Point::Write(BufferWriter*bw){
    bw->Write<double>(x);
    bw->Write<double>(y);
}

string Point::GetTypeName(){
    return "Point";
}

bool Point::ReadFromJson(JsonReader*jr,JsonNode node){
    if(node.size()<2) return false;
    x=node[0].asDouble();
    y=node[1].asDouble();
    return true;
}

string Point::ToJson(){
    return "[" + to_string(x) + "," + to_string(y)+ "]";
}

AutoPtr<Envelope>Point::GetExtent3D(){
    return AutoPtr<Envelope>(new Envelope(x,y,x,y,z,z));
}

void Point::Rotate(DOUBLE RotateAngle,Vector*axis){
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
    innerRotate(sina,cosa,rx,ry,rz,x,y,z);
}

bool Point::Read3D(BufferReader*br){
    x=br->Read<double>();
    y=br->Read<double>();
    z=br->Read<double>();
    return true;
}
void Point::Write3D(BufferWriter*bw){
    bw->Write<double>(x);
    bw->Write<double>(y);
    bw->Write<double>(z);
}

bool Point::ReadFromJson3D(JsonReader*jr,JsonNode node){
    if(node.size()<2) return false;
    x=node[0].asDouble();
    y=node[1].asDouble();
    if(node.size()>2)
        z=node[2].asDouble();
    else
        z=0;
    return true;
}
string Point::ToJson3D(){
    return "[" + to_string(x) + "," + to_string(y)+ "," + to_string(z)+ "]";
}

double&Point::X(){
    return x;
}
double&Point::Y(){
    return y;
}

double&Point::Z(){
    return z;
}

Point Point::operator + (const Point&other){
   return Point(x+other.x,y+other.y);
}
Point Point::operator - (const Point&other){
   return Point(x-other.x,y-other.y);
}
Point Point::operator * (const Point&other){
   return Point(x*other.x,y*other.y);
}
Point Point::operator / (const Point&other){
   return Point(x/other.x,y/other.y);
}
Point&Point::operator += (const Point&other){
   x+=other.x;
   y+=other.y;
   return *this;
}
Point&Point::operator -= (const Point&other){
   x-=other.x;
   y-=other.y;
   return *this;
}
Point&Point::operator *= (const Point&other){
   x*=other.x;
   y*=other.y;
   return *this;
}
Point&Point::operator /= (const Point&other){
   x/=other.x;
   y/=other.y;
   return *this;
}

void Point::GetCoord(double*x,double*y){
    *x=this->x;
    *y=this->y;
}
void Point::PutCoord(double x,double y){
    this->x=x;
    this->y=y;
}

double Point::DistanceToPoint(const Point&other){
    return sqrt((x-other.x)*(x-other.x)+(y-other.y)*(y-other.y));
}

double Point::CaptureSegment(Segment*seg,Point&capturedPoint){
    double dist1,dist2;
    double ptx=x;
    double pty=y;
    double lnx1,lny1,lnx2,lny2;
    seg->GetCoord(&lnx1,&lny1,&lnx2,&lny2);
    if((lnx1==lnx2)&&(lny1==lny2)) {
        capturedPoint.x=lnx1;capturedPoint.y=lny1;
        return sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
    }
    if(lnx1==lnx2) {
        if((pty>=min(lny1,lny2))&&(pty<=max(lny1,lny2))) {
            capturedPoint.x=lnx1;capturedPoint.y=pty;
            return fabs(ptx-lnx1);
        }
        dist1=sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
        dist2=sqrt(pow(ptx-lnx2,2)+pow(pty-lny2,2));
        if(dist1<=dist2) {
            capturedPoint.x=lnx1;capturedPoint.y=lny1;
            return dist1;
        }
        else {
            capturedPoint.x=lnx2;capturedPoint.y=lny2;
            return dist2;
        }
    }
    if(lny1==lny2) {
        if((ptx>=min(lnx1,lnx2))&&(ptx<=max(lnx1,lnx2))) {
            capturedPoint.x=ptx;capturedPoint.y=lny1;
            return fabs(pty-lny1);
        }
        dist1=sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
        dist2=sqrt(pow(ptx-lnx2,2)+pow(pty-lny2,2));
        if(dist1<=dist2)
        {
            capturedPoint.x=lnx1;capturedPoint.y=lny1;
            return dist1;
        }
        else
        {
            capturedPoint.x=lnx2;capturedPoint.y=lny2;
            return dist2;
        }
    }
    double k=(lny2-lny1)/(lnx2-lnx1);
    double tempx=(k*lnx1+ptx/k-lny1+pty)/(k+1/k);
    if((tempx>=min(lnx1,lnx2))&&(tempx<=max(lnx1,lnx2))) {
        double tempy=k*(tempx-lnx1)+lny1;
        capturedPoint.x=tempx;capturedPoint.y=tempy;
        return sqrt(pow(ptx-tempx,2)+pow(pty-tempy,2));
    }
    dist1=sqrt(pow(ptx-lnx1,2)+pow(pty-lny1,2));
    dist2=sqrt(pow(ptx-lnx2,2)+pow(pty-lny2,2));
    if(dist1<=dist2) {
        capturedPoint.x=lnx1;capturedPoint.y=lny1;
        return dist1;
    }
    capturedPoint.x=lnx2;capturedPoint.y=lny2;
    return dist2;
}

double Point::DistanceToSegment(Segment *seg){
    double _x1,_y1,_x2,_y2;
    seg->GetCoord(&_x1,&_y1,&_x2,&_y2);
    if((_x1==_x2)&&(_y1==_y2)) {
        return sqrt((_x1-x)*(_x1-x)+(_y1-y)*(_y1-y));
    }
    double length2=(_x2-_x1)*(_x2-_x1)+(_y2-_y1)*(_y2-_y1);
    double r=((x-_x1)*(_x2-_x1)+(y-_y1)*(_y2-_y1))/length2;
    if (r<=0.0) return sqrt((x-_x1)*(x-_x1)+(y-_y1)*(y-_y1));
    if (r>=1.0) return sqrt((x-_x2)*(x-_x2)+(y-_y2)*(y-_y2));
    double s=((_y1-y)*(_x2-_x1)-(_x1-x)*(_y2-_y1))/length2;
    return fabs(s)*sqrt(length2);
}

bool Point::IntersectsSegment(Segment*seg){
    double _x1,_y1,_x2,_y2;
    seg->GetCoord(&_x1,&_y1,&_x2,&_y2);
    double minx=_x1;if(_x2<minx) minx=_x2;
    double maxx=_x1;if(_x2>minx) maxx=_x2;
    double miny=_y1;if(_y2<miny) miny=_y2;
    double maxy=_y1;if(_y2>miny) maxy=_y2;
    if((x>=minx-VertexTolerance)&&(x<=maxx+VertexTolerance)&&(y>=miny-VertexTolerance)&&(y<=maxy+VertexTolerance)){
        if ((RobustDeterminant::OrientationIndex(_x1,_y1,_x2,_y2, x,y) == 0) && (RobustDeterminant::ReverseOrientationIndex(_x1,_y1,_x2,_y2, x,y) == 0)) {
            return true;
        }
    }
    return false;
}

void Point::GetCoord(double*x,double*y,double*z){
    *x=this->x;
    *y=this->y;
    *z=this->z;
}
void Point::PutCoord(double x,double y,double z){
    this->x=x;
    this->y=y;
    this->z=z;
}
double Point::GetModule(){
    return sqrt(x*x+y*y+z*z);
}
double Point::Dot(Vector*other){
    DOUBLE X,Y,Z;
    other->GetCoord(&X,&Y,&Z);
    return X*x+Y*y+Z*z;
}
AutoPtr<Vector>Point::Cross(Vector*other){
    DOUBLE X,Y,Z;
    other->GetCoord(&X,&Y,&Z);
    AutoPtr<Vector>pNew(new Point(y*Z-z*Y,z*X-x*Z,x*Y-y*X));
    return pNew;
}
void Point::RotatePointByAnchor(Point*anchorPoint,AxisType aType,DOUBLE planeangle,DOUBLE axisangle){
    DOUBLE X0,Y0,Z0;
    anchorPoint->GetCoord(&X0,&Y0);
    Z0=anchorPoint->z;
    FLOAT length=sqrt((x-X0)*(x-X0)+(y-Y0)*(y-Y0)+(z-Z0)*(z-Z0));
    if(aType==atX)
    {
        DOUBLE angyz=0,angx=0;
        if(length>0)
        {
            if(Z0==z)
            {
                if(Y0>y)
                    angyz= PI*3/2;
                else if(y==Y0)
                    angyz=0;
                else
                    angyz=PI/2;
            }
            else
            {
                angyz=atan((y-Y0)/(z-Z0));
                if(z<Z0) angyz=angyz+PI;
                if(angyz<0) angyz=angyz+PI*2;
            }
            angx=asin((x-X0)/length);
        }
        angyz+=planeangle;
        angx+=axisangle;
        double lenxz=length*cos(angx);
        z=Z0+lenxz*cosf(angyz);
        y=Y0+lenxz*sinf(angyz);
        x=X0+length*sinf(angx);
    }
    else if(aType==atY)
    {
        DOUBLE angxz=0,angy=0;
        if(length>0)
        {
            if(X0==x)
            {
                if(Z0>z)
                    angxz= PI*3/2;
                else if(z==Z0)
                    angxz=0;
                else
                    angxz=PI/2;
            }
            else
            {
                angxz=atan((z-Z0)/(x-X0));
                if(x<X0) angxz=angxz+PI;
                if(angxz<0) angxz=angxz+PI*2;
            }
            angy=asin((y-Y0)/length);
        }
        angxz+=planeangle;
        angy+=axisangle;
        float lenxz=length*cos(angy);
        x=X0+lenxz*cosf(angxz);
        z=Z0+lenxz*sinf(angxz);
        y=Y0+length*sinf(angy);
    }
    else
    {
        DOUBLE angxy=0,angz=0;
        if(length>0)
        {
            if(X0==x)
            {
                if(Y0>y)
                    angxy= PI*3/2;
                else if(y==Y0)
                    angxy=0;
                else
                    angxy=PI/2;
            }
            else
            {
                angxy=atan((y-Y0)/(x-X0));
                if(x<X0) angxy=angxy+PI;
                if(angxy<0) angxy=angxy+PI*2;
            }
            angz=asin((z-Z0)/length);
        }
        angxy+=planeangle;
        angz+=axisangle;
        float lenxy=length*cos(angz);
        x=X0+lenxy*cosf(angxy);
        y=Y0+lenxy*sinf(angxy);
        z=Z0+length*sinf(angz);
    }
}
void Point::RotatePoint(Vector*axis,double RotateAngle){
    DOUBLE rx,ry,rz;
    axis->GetCoord(&rx,&ry,&rz);
    double L=sqrt(rx*rx+ry*ry+rz*rz);
    DOUBLE cosa=cos(RotateAngle);
    DOUBLE sina=sin(RotateAngle);
    if(L!=0)
    {
        rx=rx/L;
        ry=ry/L;
        rz=rz/L;
    }
    float cosa1=(1-cosa);
    double tx=(cosa+rx*rx*cosa1)*x+(rx*ry*cosa1-rz*sina)*y+(rx*rz*cosa1+ry*sina)*z;
    double ty=(rx*ry*cosa1+rz*sina)*x+(cosa+ry*ry*cosa1)*y+(ry*rz*cosa1-rx*sina)*z;
    double tz=(rx*rz*cosa1-ry*sina)*x+(ry*rz*cosa1+rx*sina)*y+(cosa+rz*rz*cosa1)*z;
    x=tx;
    y=ty;
    z=tz;
}
void Point::ScaleByAnchor(Point*anchor,double ratio){
    DOUBLE X0,Y0,Z0;
    anchor->GetCoord(&X0,&Y0);
    Z0=anchor->z;
    x=X0+(x-X0)*ratio;
    y=Y0+(y-Y0)*ratio;
    z=Z0+(z-Z0)*ratio;
}

AutoPtr<Vector>Point::Multiply(Matrix4x4*matrix){
    double*matrixdata=matrix->GetRawData();
    AutoPtr<Vector>pNew(new Point());
    double X,Y,Z,W;
    X=x*matrixdata[0]+y*matrixdata[1]+z*matrixdata[2]+matrixdata[3];
    Y=x*matrixdata[4]+y*matrixdata[5]+z*matrixdata[6]+matrixdata[7];
    Z=x*matrixdata[8]+y*matrixdata[9]+z*matrixdata[10]+matrixdata[11];
    W=x*matrixdata[12]+y*matrixdata[13]+z*matrixdata[14]+matrixdata[15];
    if(W!=0)
        pNew->PutCoord(X/W,Y/W,Z/W);
    else
        pNew->PutCoord(X,Y,Z);
    return pNew;
}


}
