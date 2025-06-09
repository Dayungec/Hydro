#include "polyline.h"
#include "envelope.h"
#include "math.h"
#include "spatialreference.h"
#include "Base/classfactory.h"
#include "segmentindexes.h"

namespace SGIS{

REGISTER(Polyline)
REGISTER(CurvePolyline);

Polyline::Polyline(){

}
Polyline::~Polyline(){

}
//IGeometry:
AutoPtr<Envelope>Polyline::GetExtent(){
    int Size=parts.size();
    if(Size==0) return nullptr;
    AutoPtr<Envelope>pEnv;
    for(int k=0;k<Size;k++)
    {
        AutoPtr<Points>points(parts[k]);
        if(pEnv==nullptr)
            pEnv=points->GetExtent();
        else
            pEnv->UnionOther(points->GetExtent().get());
    }
    return pEnv;
}

GeometryType Polyline::GetType(){
    return gtPolyline;
}
AutoPtr<Geometry>Polyline::Clone(){
   Polyline*newpoly=new Polyline();
   for(int k=0;k<parts.size();k++){
       AutoPtr<Points>points(parts[k]);
       AutoPtr<Path>npts(points->Clone());
       newpoly->parts.push_back(npts);
   }
   return newpoly;
}
bool Polyline::Equals(Geometry*other){
    if(GetType()!=other->GetType()) return false;
    Polyline*po=(Polyline*)other;
    int size=parts.size();
    if(size!=po->parts.size()) return false;
    for(int k=0;k<size;k++){
        AutoPtr<Points>points(parts[k]);
        AutoPtr<Path>npts(po->parts[k]);
        if(!points->Equals((Geometry*)npts.get())) return false;
    }
    return true;
}

bool Polyline::Intersects(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        bool IsOn;
        IsOn=CanCaptureBorder(ppt.get(),VertexTolerance);
        return IsOn;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Intersects(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Intersects(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Intersects(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return ppts->Intersects(this);
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
        SegmentIndexes si(AutoPtr<Polyline>(this,true),8);
        si.BuildSpatialIndex();
        return si.IntersectGeometry(pply.get());
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        int partCount=parts.size();
        if(partCount==0)
        {
            return false;
        }
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
        SegmentIndexes si(ppoly,8);
        si.BuildSpatialIndex();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath=parts[k];
            if(si.IntersectPath(pPath.get()))
            {
                return true;
            }
            if(k<partCount-1) si.ResetSegmentStates();
            AutoPtr<Points>ppts=pPath;
            LONG ptCount;
            ptCount=ppts->GetSize();
            if(ptCount==0) continue;
            DOUBLE X,Y;
            ppts->GetItem(0,X,Y);
            AutoPtr<Point>ppt(new Point());
            ppt->PutCoord(X,Y);
            IsIn=ppt->Within(ppoly.get());
            if(IsIn)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
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
    if(pCol!=nullptr)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
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
bool Polyline::Disjoint(Geometry*other){
    return (!Intersects(other));
}
bool Polyline::Touches(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return ppt->Touches(this);
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Touches(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Touches(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Touches(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return ppts->Touches(this);
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
        SegmentIndexes si(AutoPtr<Polyline>(this,true),8);
        si.BuildSpatialIndex();
        int state=si.GetPolylineTouchState(pply.get());
        return (state==1);
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
        SegmentIndexes si(ppoly,8);
        si.BuildSpatialIndex();
        LONG partCount=parts.size();
        if(partCount==0)
        {
            return false;
        }
        bool HasSuc=false;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath=parts[k];
            int state=si.IsPathTouchRing(pPath.get());
            if(state==0)
            {
                return false;
            }
            if(state==2)
            {
                AutoPtr<Points>ppts=pPath;
                LONG ptCount;
                ptCount=ppts->GetSize();
                if(ptCount==0)
                {
                    state=0;
                }
                else
                {
                    DOUBLE X,Y;
                    ppts->GetItem(0,X,Y);
                    AutoPtr<Point>ppt(new Point());
                    ppt->PutCoord(X,Y);
                    IsIn=ppt->Within(ppoly.get());
                    if(IsIn) state=0;
                }
            }
            if(state==1) HasSuc=true;
            if(k<partCount-1) si.ResetSegmentStates();
        }
        return HasSuc;
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
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
    if(pCol!=nullptr)
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
bool Polyline::Crosses(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Crosses(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Crosses(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Crosses(this);
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
        SegmentIndexes si(AutoPtr<Polyline>(this,true),8);
        si.BuildSpatialIndex();
        return (3==si.GetPolylineCrossState(pply.get()));
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
        SegmentIndexes si(ppoly,8);
        si.BuildSpatialIndex();
        LONG partCount=parts.size();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath=parts[k];
            int state=si.IsPathCrossRing(pPath.get());
            if(state==1)
            {
                return true;
            }
            if(k<partCount-1) si.ResetSegmentStates();
        }
        return false;
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
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
    if(pCol!=nullptr)
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

bool Polyline::IsInPath(Points*outerPoints,Points*innerPoints){
    LONG outerCount=outerPoints->GetSize();
    if(outerCount<2) return false;
    LONG innerCount=innerPoints->GetSize();
    if(innerCount<2) return false;
    DOUBLE lnx1,lny1,lnx2,lny2;
    innerPoints->GetItem(0,lnx1,lny1);
    innerPoints->GetItem(innerCount-1,lnx2,lny2);
    Segment2D<double> seg2;
    outerPoints->GetItem(0,seg2.lnx1,seg2.lny1);
    int posf=-1;
    int post=-1;
    for(int k=1;k<outerCount;k++)
    {
        outerPoints->GetItem(k,seg2.lnx2,seg2.lny2);
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
    DOUBLE X,Y;
    int fromPos=1;
    for(int k=posf+1;k<=post;k++)
    {
        outerPoints->GetItem(k,X,Y);
        Segment2D<double> seg;
        innerPoints->GetItem(fromPos,seg.lnx1,seg.lny1);
        bool HasFind=false;
        for(int p=fromPos;p<innerCount;p++)
        {
            innerPoints->GetItem(p,seg.lnx2,seg.lny2);
            if((X>=min(seg.lnx1,seg.lnx2)-VertexTolerance)&&(X<=max(seg.lnx1,seg.lnx2)+VertexTolerance)&&(Y>=min(seg.lny1,seg.lny2)-VertexTolerance)&&(Y<=max(seg.lny1,seg.lny2)+VertexTolerance))
            {
                if(seg.IsPointOnSegment(X,Y,VertexTolerance))
                {
                    fromPos=p;
                    HasFind=true;
                    break;
                }
            }
        }
        if(!HasFind)
        {
            return false;
        }
    }
    fromPos=posf+1;
    for(int k=1;k<innerCount-1;k++)
    {
        innerPoints->GetItem(k,X,Y);
        Segment2D<double> seg;
        outerPoints->GetItem(fromPos,seg.lnx1,seg.lny1);
        bool HasFind=false;
        for(int p=fromPos;p<=post;p++)
        {
            outerPoints->GetItem(p,seg.lnx2,seg.lny2);
            if((X>=min(seg.lnx1,seg.lnx2)-VertexTolerance)&&(X<=max(seg.lnx1,seg.lnx2)+VertexTolerance)&&(Y>=min(seg.lny1,seg.lny2)-VertexTolerance)&&(Y<=max(seg.lny1,seg.lny2)+VertexTolerance))
            {
                if(seg.IsPointOnSegment(X,Y,VertexTolerance))
                {
                    fromPos=p;
                    HasFind=true;
                    break;
                }
            }
        }
        if(!HasFind)
        {
            return false;
        }
    }
    return true;
}
bool Polyline::IsPathIn(Polyline*polyline,Points*innerPoints){
    LONG partCount=polyline->GetSize();
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Path>pPath=polyline->GetItem(k);
        AutoPtr<Points>ppts=pPath;
        if(IsInPath(ppts.get(),innerPoints)) return true;
    }
    return false;
}
bool Polyline::Within(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Contains(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Contains(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Contains(this);
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
        bool IsIn=pEnv->Contains(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        AutoPtr<Polyline>pply;
        pply=Geometry::ConvertToPolyline(polyline.get());
        LONG count2;
        count2=pply->GetSize();
        if(count2==0)
        {
            return false;
        }
        LONG count1=parts.size();
        if(count1==0)
        {
            return false;
        }
        for(int k=0;k<count1;k++)
        {
            AutoPtr<Path>pPath=parts[k];
            AutoPtr<Points>pts=pPath;
            if(!IsPathIn(pply.get(),pts.get()))
            {
                return false;
            }
        }
        return true;
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
        SegmentIndexes si(ppoly,8);
        si.BuildSpatialIndex();
        LONG partCount=parts.size();
        if(partCount==0)
        {
            return false;
        }
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath=parts[k];
            int state=si.GetRingSegmentIntersectOuterState(pPath.get());
            if(k<partCount-1) si.ResetSegmentStates();
            if(state&4)
            {
                return false;
            }
            if(state==0)
            {
                AutoPtr<Points>ppts=pPath;
                LONG ptCount;
                ptCount=ppts->GetSize();
                if(ptCount==0) continue;
                DOUBLE X,Y;
                ppts->GetItem(0,X,Y);
                AutoPtr<Point>ppt(new Point());
                ppt->PutCoord(X,Y);
                IsIn=ppt->Within(ppoly.get());
                if(!IsIn)
                {
                    return false;
                }
            }
        }
        return true;
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
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
    if(pCol!=nullptr)
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
bool Polyline::Contains(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return ppt->Within(this);
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Within(this);
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
        return ppts->Within(this);
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
        LONG count2;
        count2=pply->GetSize();
        if(count2==0)
        {
            return false;
        }
        LONG count1=parts.size();
        if(count1==0)
        {
            return false;
        }
        for(int k=0;k<count2;k++)
        {
            AutoPtr<Path>pPath;
            pPath=pply->GetItem(k);
            AutoPtr<Points>pts=pPath;
            if(!IsPathIn(this,pts.get()))
            {
                return false;
            }
        }
        return true;
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
    if(pCol!=nullptr)
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
bool Polyline::Overlaps(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Intersects(this);
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
        return Intersects(polyline.get());
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
    if(pCol!=nullptr)
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

void Polyline::AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Points>points(parts[k]);
        points->AffineTrans(AnchorPoint, RotateAngle,ZoomXRatio,ZoomYRatio,DiffX,DiffY);
    }
}
bool Polyline::Project(CoordinateTransformation*pTrans){
    for(int k=parts.size()-1;k>=0;k--)
    {
        AutoPtr<Points>points(parts[k]);
        if(!points->Project(pTrans)) return false;
    }
    return true;
}
void Polyline::Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Points>points(parts[k]);
        points->Scale(xr,yr,zr);
    }
}
void Polyline::Move(DOUBLE difx,DOUBLE dify,DOUBLE difz){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Points>points(parts[k]);
        points->Move(difx,dify,difz);
    }
}

AutoPtr<Point>Polyline::GetCenter(){
    int partSize=parts.size();
    if(partSize==0) return nullptr;
    if(partSize==1){
       AutoPtr<Path>points=parts[0];
       double len=points->GetLength();
       return points->ReturnPoint(len/2);
    }
    else{
        double sx=0,sy=0;
        int nCount=0;
        for(int k=0;k<parts.size();k++){
            AutoPtr<Points>pts=parts[k];
            DOUBLE*XYs=pts->GetXYs();
            int ptCount=pts->GetSize();
            int nIndex=0;
            for(int j=0;j<ptCount;j++){
                sx+=XYs[nIndex];
                sy+=XYs[nIndex+1];
                nIndex++;
            }
            nCount+=ptCount;
        }
        if(nCount>0){
            sx/=nCount;
            sy/=nCount;
        }
        return new Point(sx,sy);
    } 
}

AutoPtr<Point>Polyline::GetFromPoint(){
    for(int k=0;k<parts.size();k++){
        AutoPtr<Points>pts=parts[k];
        if(pts->GetSize()>0){
            double x,y;
            pts->GetItem(0,x,y);
            return new Point(x,y);
        }
    }
    return nullptr;
}

AutoPtr<Point>Polyline::GetToPoint(){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Points>pts=parts[k];
        if(pts->GetSize()>0){
            double x,y;
            pts->GetItem(pts->GetSize()-1,x,y);
            return new Point(x,y);
        }
    }
    return nullptr;
}

//BufferWritable:
bool Polyline::Read(BufferReader*br){
    int size=br->Read<int>();
    parts.clear();
    for(int k=0;k<size;k++){
        Points*pts=new Points();
        if(!pts->Read(br)){
            delete pts;
            return false;
        }
        parts.push_back(AutoPtr<Path>(pts));
    }
    return true;
}
void Polyline::Write(BufferWriter*bw){
    int size=parts.size();
    bw->Write<int>(size);
    for(int k=0;k<size;k++){
        AutoPtr<Points>points(parts[k]);
        points->Write(bw);
    }
}
string Polyline::GetTypeName(){
    return "Polyline";
}
//JsonWritable:
bool Polyline::ReadFromJson(JsonReader*jr,JsonNode node){
    parts.clear();
    for(int k=0;k<node.size();k++){
        AutoPtr<Points>pts(new Points());
        if(!pts->ReadFromJson(jr,node[k])) return false;
        parts.push_back(pts);
    }
    return true;
}
string Polyline::ToJson(){
    JsonArrayWriter writer;
    for(int k=0;k<parts.size();k++){
        AutoPtr<JsonWritable>pts(parts[k]);
        writer.AddWritable(pts.get());
    }
    return writer.GetJsonString();
}
//Geometry3D:
AutoPtr<Envelope>Polyline::GetExtent3D(){
    int Size=parts.size();
    if(Size==0) return nullptr;
    AutoPtr<Envelope>pEnv;
    for(int k=0;k<Size;k++)
    {
        AutoPtr<Points>points(parts[k]);
        if(pEnv==nullptr)
            pEnv=points->GetExtent3D();
        else
            pEnv->UnionOther(points->GetExtent3D().get());
    }
    return pEnv;
}
void Polyline::Rotate(DOUBLE RotateAngle,Vector*axis){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Points>points(parts[k]);
        points->Rotate(RotateAngle,axis);
    }
}
bool Polyline::Read3D(BufferReader*br){
    int size=br->Read<int>();
    parts.clear();
    for(int k=0;k<size;k++){
        Points*pts=new Points();
        if(!pts->Read3D(br)){
            delete pts;
            return false;
        }
        parts.push_back(AutoPtr<Path>(pts));
    }
    return true;
}
void Polyline::Write3D(BufferWriter*bw){
    int size=parts.size();
    bw->Write<int>(size);
    for(int k=0;k<size;k++){
        AutoPtr<Points>points(parts[k]);
        points->Write3D(bw);
    }
}
bool Polyline::ReadFromJson3D(JsonReader*jr,JsonNode node){
    parts.clear();
    for(int k=0;k<node.size();k++){
        AutoPtr<Points>pts(new Points());
        if(!pts->ReadFromJson3D(jr,node[k])) return false;
        parts.push_back(pts);
    }
    return true;
}
string Polyline::ToJson3D(){
    string str="[";
    for(int k=0;k<parts.size();k++){
        AutoPtr<Points>points(parts[k]);
        str+=points->ToJson3D();
        if(k<parts.size()-1) str+=',';
    }
    str+="]";
    return str;
}

int Polyline::GetSize(){
    return parts.size();
}
AutoPtr<Path>Polyline::operator[](int nIndex){
    return parts[nIndex];
}
AutoPtr<Path>Polyline::GetItem(int nIndex){
    return parts[nIndex];
}

void Polyline::SetItem(int nIndex,AutoPtr<Path>path){
    parts[nIndex]=path;
}

void Polyline::Add(AutoPtr<Path>item){
    parts.push_back(item);
}

void Polyline::Insert(int nIndex,AutoPtr<Path>item){
    parts.insert(std::begin(parts)+nIndex,item);
}
void Polyline::Clear(){
    parts.clear();
}

void Polyline::Remove(int nIndex){
    parts.erase(begin(parts)+nIndex);
}

double Polyline::GetLength(){
    double len=0;
    for(int k=parts.size()-1;k>=0;k--)
    {
        len+=parts[k]->GetLength();
    }
    return len;
}

double Polyline::GetDiameter(){
    if(parts.size()==0) return 0;
    AutoPtr<Points>pts=parts[0];
    double x1,y1,x2,y2;
    double diam=0;
    bool findFirst=false;
    for(int k=0;k<parts.size();k++){
        AutoPtr<Points>pts=parts[k];
        if(pts->GetSize()>0){
            pts->GetItem(0,x1,y1);
            x2=x1;y2=y1;
            findFirst=true;
            break;
        }
    }
    if(!findFirst) return 0;
    double tx=x2+1,ty=y2+1;
    while(true){
        for(int k=0;k<parts.size();k++){
            AutoPtr<Points>pts=parts[k];
            DOUBLE*XYs=pts->GetXYs();
            int ptCount=pts->GetSize();
            int nIndex=0;
            for(int j=0;j<ptCount;j++){
                double x=XYs[nIndex];
                double y=XYs[nIndex+1];
                double dist=sqrt((x1-x)*(x1-x)+(y1-y)*(y1-y));
                if(dist>diam){
                    tx=x;
                    ty=y;
                    diam=dist;
                }
                nIndex+=2;
            }
        }
        if((tx==x2)&&(ty==y2)){
            break;
        }
        x2=x1;
        y2=y1;
        x1=tx;
        y1=ty;
    }
    return diam;
}

bool Polyline::CanCaptureBorder(Point*ppt,double toler){
    LONG partCount=parts.size();
    bool IsOk;
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Path>path=parts[k];
        if(path->CanCaptureBorder(ppt,toler)) return true;
    }
    return false;
}
AutoPtr<Point>Polyline::CaptureBorder(Point*ppt,double toler){
    LONG partCount=parts.size();
    DOUBLE nearX,nearY;
    DOUBLE minDist=-1;
    DOUBLE dx,dy;
    DOUBLE dist;
    DOUBLE X,Y;
    ppt->GetCoord(&X,&Y);
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Path>path=parts[k];
        AutoPtr<Point>droppt=path->CaptureBorder(ppt,toler);
        if(droppt!=nullptr)
        {
            droppt->GetCoord(&dx,&dy);
            dist=sqrt((X-dx)*(X-dx)+(Y-dy)*(Y-dy));
            if(minDist==-1)
            {
                minDist=dist;
                nearX=dx;
                nearY=dy;
            }
            else if(dist<minDist)
            {
                minDist=dist;
                nearX=dx;
                nearY=dy;
            }
        }
    }
    if(minDist==-1) return nullptr;
    AutoPtr<Point>tpt(new Point());
    tpt->PutCoord(nearX,nearY);
    return tpt;
}

CurvePolyline::CurvePolyline(){

}

CurvePolyline::~CurvePolyline(){

}

AutoPtr<Geometry>CurvePolyline::Clone(){
    CurvePolyline*newpoly=new CurvePolyline();
    for(int k=0;k<parts.size();k++){
        AutoPtr<Points>points(parts[k]);
        AutoPtr<Path>npts(points->Clone());
        newpoly->parts.push_back(npts);
    }
    return newpoly;
}
double CurvePolyline::GetLength(){
    AutoPtr<Polyline>poly=ConvertToDefaultPolyline(false);
    return poly->GetLength();
}

AutoPtr<Polyline>CurvePolyline::ConvertToPolyline(double toler){
    AutoPtr<Polyline>poly(new Polyline());
    for(int k=0;k<parts.size();k++)
    {
        AutoPtr<Path>path=parts[k];
        AutoPtr<Points>ppts(path);
        AutoPtr<Points>newpts(ppts->Clone());
        newpts->Smooth(toler);
        poly->Add(newpts);
    }
    return poly;
}

AutoPtr<Polyline>CurvePolyline::ConvertToDefaultPolyline(bool hQuality){
    AutoPtr<Polyline>poly(new Polyline());
    bool IsOk;
    int num=(hQuality?10:5);
    for(int k=0;k<parts.size();k++)
    {
        AutoPtr<Path>path=parts[k];
        DOUBLE len=path->GetLength();
        AutoPtr<Points>ppts(path);
        LONG ptCount=ppts->GetSize();
        AutoPtr<Points>newpts(ppts->Clone());
        newpts->Smooth(len/ptCount/num);
        poly->Add(newpts);
    }
    return poly;
}

bool CurvePolyline::CanCaptureBorder(Point*ppt,double toler){
    AutoPtr<Polyline>poly=ConvertToDefaultPolyline(false);
    return poly->CanCaptureBorder(ppt,toler);
}
AutoPtr<Point> CurvePolyline::CaptureBorder(Point*ppt,double toler){
    AutoPtr<Polyline>poly=ConvertToDefaultPolyline(false);
    return poly->CaptureBorder(ppt,toler);
}

string CurvePolyline::GetTypeName(){
    return "CurvePolyline";
}

}
