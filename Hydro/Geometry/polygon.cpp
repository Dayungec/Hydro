#include "polygon.h"
#include "envelope.h"
#include "math.h"
#include "geometrycollection.h"
#include "spatialreference.h"
#include "Base/classfactory.h"
#include "labelcomputer.h"
#include "topobuilder.h"
#include "segmentindexes.h"
namespace SGIS{

REGISTER(Polygon)
REGISTER(CurvePolygon);

Polygon::Polygon(){

}
Polygon::~Polygon(){

}
//IGeometry:
AutoPtr<Envelope>Polygon::GetExtent(){
    int Size=parts.size();
    if(Size==0) return nullptr;
    AutoPtr<Envelope>pEnv;
    for(int k=0;k<Size;k++)
    {
        AutoPtr<Points>points(parts[k]);
        if(points->GetSize()==0) continue;
        if(pEnv==nullptr)
            pEnv=points->GetExtent();
        else
            pEnv->UnionOther(points->GetExtent().get());
    }
    return pEnv;
}

GeometryType Polygon::GetType(){
    return gtPolygon;
}
AutoPtr<Geometry>Polygon::Clone(){
   Polygon*newpoly=new Polygon();
   for(int k=0;k<parts.size();k++){
       AutoPtr<Points>points(parts[k]);
       AutoPtr<Ring>npts(points->Clone());
       newpoly->parts.push_back(npts);
   }
   return newpoly;
}
bool Polygon::Equals(Geometry*other){
    if(GetType()!=other->GetType()) return false;
    Polygon*po=(Polygon*)other;
    int size=parts.size();
    if(size!=po->parts.size()) return false;
    for(int k=0;k<size;k++){
        AutoPtr<Points>points(parts[k]);
        AutoPtr<Ring>npts(po->parts[k]);
        if(!points->Equals((Geometry*)npts.get())) return false;
    }
    return true;
}

bool Polygon::Intersects(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return ppt->Intersects(this);
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
        return polyline->Intersects(this);
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
        int partCount=parts.size();
        if(partCount==0)
        {
            return false;
        }
        AutoPtr<Polygon>ppoly;
        ppoly=ConvertToPolygon(polygon.get());
        LONG count2;
        count2=ppoly->GetSize();
        if(count2==0)
        {
            return false;
        }
        AutoPtr<Point>firstPoint(new Point);
        bool hasValidPoint=false;
        for(int k=0;k<count2;k++)
        {
            AutoPtr<Ring>ring;
            ring=ppoly->GetItem(k);
            AutoPtr<Points>ppts=ring;
            LONG ptc;
            ptc=ppts->GetSize();
            if(ptc==0) continue;
            DOUBLE X,Y;
            ppts->GetItem(0,X,Y);
            firstPoint->PutCoord(X,Y);
            hasValidPoint=true;
            break;
        }
        if(!hasValidPoint)
        {
            return false;
        }
        SegmentIndexes si(ppoly,8);
        si.BuildSpatialIndex();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Ring>pRing=parts[k];
            if(si.IntersectRing(pRing.get()))
            {
                return true;
            }
            if(k<partCount-1) si.ResetSegmentStates();
            AutoPtr<Points>ppts=pRing;
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
            IsIn=firstPoint->Within(this);
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
        if(pEnv==nullptr)
        {
            return false;
        }
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
bool Polygon::Disjoint(Geometry*other){
    return (!Intersects(other));
}
bool Polygon::Touches(Geometry*other){
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
        return polyline->Touches(this);
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
        LONG count2;
        count2=ppoly->GetSize();
        if(count2==0)
        {
            return false;
        }
        AutoPtr<Point>firstPoint(new Point());
        bool hasValidPoint=false;
        for(int k=0;k<count2;k++)
        {
            AutoPtr<Ring>ring;
            ring=ppoly->GetItem(k);
            AutoPtr<Points>ppts=ring;
            LONG ptc;
            ptc=ppts->GetSize();
            if(ptc==0) continue;
            DOUBLE X,Y;
            ppts->GetItem(0,X,Y);
            firstPoint->PutCoord(X,Y);
            hasValidPoint=true;
        }
        if(!hasValidPoint)
        {
            return false;
        }
        bool HasSuc=false;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Ring>pRing=parts[k];
            int state=si.IsRingTouchRing(pRing.get());
            if(state==0)
            {
                return false;
            }
            if(state==2)
            {
                AutoPtr<Points>ppts=pRing;
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
            if(state==2)
            {
                IsIn=firstPoint->Within(this);
                if(IsIn) state=0;
            }
            if(state==3)
            {
                if(k>0) state=1;
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
bool Polygon::Crosses(Geometry*other){
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
        return polyline->Crosses(this);
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
            AutoPtr<Ring>pRing=parts[k];
            int state=si.IsRingCrossRing(pRing.get());
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
bool Polygon::Within(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return false;
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
        return false;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Contains(segEnv.get());
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
            AutoPtr<Ring>pRing=parts[k];
            int state=si.GetRingSegmentIntersectOuterState(pRing.get());
            if(k<partCount-1) si.ResetSegmentStates();
            if(state&4)
            {
                return false;
            }
            if(state==0)
            {
                AutoPtr<Points>ppts=pRing;
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
        bool IsIn;
        IsIn=pEnv->Contains(segEnv.get());
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
bool Polygon::Contains(Geometry*other){
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
        return pEnv->Within(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Within(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return ppts->Within(this);
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        return polyline->Within(this);
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        return polygon->Within(this);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Within(segEnv.get());
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
            IsInter=Contains(poly.get());
            if(!IsInter)
            {
                return false;
            }
        }
        return true;
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
bool Polygon::Overlaps(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return false;
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Overlaps(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Overlaps(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        return false;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        return polygon->Intersects(this);
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
            IsInter=Overlaps(poly.get());
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

void Polygon::AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Points>points(parts[k]);
        points->AffineTrans(AnchorPoint, RotateAngle,ZoomXRatio,ZoomYRatio,DiffX,DiffY);
    }
}
bool Polygon::Project(CoordinateTransformation*pTrans){
    for(int k=parts.size()-1;k>=0;k--)
    {
        AutoPtr<Points>points(parts[k]);
        if(!points->Project(pTrans)) return false;
    }
    return true;
}
void Polygon::Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Points>points(parts[k]);
        points->Scale(xr,yr,zr);
    }
}
void Polygon::Move(DOUBLE difx,DOUBLE dify,DOUBLE difz){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Points>points(parts[k]);
        points->Move(difx,dify,difz);
    }
}

AutoPtr<Point>Polygon::GetCenter(){
    int partSize=parts.size();
    if(partSize==0) return nullptr;
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
            nIndex+=2;
        }
        nCount+=ptCount;
    }
    if(nCount>0){
        sx/=nCount;
        sy/=nCount;
    }
    return new Point(sx,sy);
}

//BufferWritable:
bool Polygon::Read(BufferReader*br){
    int size=br->Read<int>();
    parts.clear();
    for(int k=0;k<size;k++){
        Points*pts=new Points();
        if(!pts->Read(br)){
            delete pts;
            return false;
        }
        parts.push_back(AutoPtr<Ring>(pts));
    }
    return true;
}
void Polygon::Write(BufferWriter*bw){
    int size=parts.size();
    bw->Write<int>(size);
    for(int k=0;k<size;k++){
        AutoPtr<Points>points(parts[k]);
        points->Write(bw);
    }
}
string Polygon::GetTypeName(){
    return "Polygon";
}
//JsonWritable:
bool Polygon::ReadFromJson(JsonReader*jr,JsonNode node){
    parts.clear();
    for(int k=0;k<node.size();k++){
        AutoPtr<Points>pts(new Points());
        if(!pts->ReadFromJson(jr,node[k])) return false;
        parts.push_back(pts);
    }
    return true;
}
string Polygon::ToJson(){
    JsonArrayWriter writer;
    for(int k=0;k<parts.size();k++){
        AutoPtr<JsonWritable>pts(parts[k]);
        writer.AddWritable(pts.get());
    }
    return writer.GetJsonString();
}
//Geometry3D:
AutoPtr<Envelope>Polygon::GetExtent3D(){
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
void Polygon::Rotate(DOUBLE RotateAngle,Vector*axis){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Points>points(parts[k]);
        points->Rotate(RotateAngle,axis);
    }
}
bool Polygon::Read3D(BufferReader*br){
    int size=br->Read<int>();
    parts.clear();
    for(int k=0;k<size;k++){
        Points*pts=new Points();
        if(!pts->Read3D(br)){
            delete pts;
            return false;
        }
        parts.push_back(AutoPtr<Ring>(pts));
    }
    return true;
}
void Polygon::Write3D(BufferWriter*bw){
    int size=parts.size();
    bw->Write<int>(size);
    for(int k=0;k<size;k++){
        AutoPtr<Points>points(parts[k]);
        points->Write3D(bw);
    }
}
bool Polygon::ReadFromJson3D(JsonReader*jr,JsonNode node){
    parts.clear();
    for(int k=0;k<node.size();k++){
        AutoPtr<Points>pts(new Points());
        if(!pts->ReadFromJson3D(jr,node[k])) return false;
        parts.push_back(pts);
    }
    return true;
}
string Polygon::ToJson3D(){
    string str="[";
    for(int k=0;k<parts.size();k++){
        AutoPtr<Points>points(parts[k]);
        str+=points->ToJson3D();
        if(k<parts.size()-1) str+=',';
    }
    str+="]";
    return str;
}

int Polygon::GetSize(){
    return parts.size();
}
AutoPtr<Ring>Polygon::operator[](int nIndex){
    return parts[nIndex];
}
AutoPtr<Ring>Polygon::GetItem(int nIndex){
    return parts[nIndex];
}
void Polygon::SetItem(int nIndex,AutoPtr<Ring>ring){
    parts[nIndex]=ring;
}

void Polygon::Add(AutoPtr<Ring>item){
    parts.push_back(item);
}

void Polygon::Insert(int nIndex,AutoPtr<Ring>item){
     parts.insert(std::begin(parts)+nIndex,item);
}
void Polygon::Clear(){
    parts.clear();
}

void Polygon::Remove(int nIndex){
    parts.erase(begin(parts)+nIndex);
}

bool Polygon::IsPointIn(Point*point){
    int Count=0;
    bool IsIn;
    for(int k=parts.size()-1;k>=0;k--)
    {
        IsIn=parts[k]->IsPointIn(point);
        if(IsIn) Count++;
    }
    return ((Count%2)==1);
}

double Polygon::GetPerimeter(){
    double per=0;
    for(int k=parts.size()-1;k>=0;k--)
    {
        per+=parts[k]->GetPerimeter();
    }
    return per;
}

double Polygon::GetArea(){
    double ara=0;
    for(int k=parts.size()-1;k>=0;k--)
    {
        ara+=parts[k]->GetArea();
    }
    return ara;
}

double Polygon::GetDiameter(){
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

bool Polygon::CanCaptureBorder(Point*ppt,double toler){
    LONG partCount=parts.size();
    bool IsOk;
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Ring>pRing=parts[k];
        if(pRing->CanCaptureRingBorder(ppt,toler)) return true;
    }
    return false;
}
AutoPtr<Point>Polygon::CaptureBorder(Point*ppt,double toler){
    LONG partCount=parts.size();
    DOUBLE nearX,nearY;
    DOUBLE minDist=-1;
    DOUBLE dx,dy;
    DOUBLE dist;
    DOUBLE X,Y;
    ppt->GetCoord(&X,&Y);
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Ring>pRing=parts[k];
        AutoPtr<Point>droppt=pRing->CaptureRingBorder(ppt,toler);
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

AutoPtr<Points>Polygon::GetLabelPoint(Envelope*clipEnv,LONG maxGridNum){
    AutoPtr<Envelope>curEnv=GetExtent();
    bool bInter=curEnv->IntersectsEnvelope(clipEnv);
    if(!bInter) return nullptr;
    AutoPtr<Envelope>ext=clipEnv->IntersectEnvelope(curEnv.get());
    LabelPointComputer labelComputer;
    DOUBLE left,top,right,bottom;
    ext->GetCoord(&left,&top,&right,&bottom);
    double w=right-left;
    double h=top-bottom;
    double cellX=w/maxGridNum;
    double cellY=h/maxGridNum;
    int Rows=maxGridNum;
    int Cols=maxGridNum;
    double cX=cellX;
    if(cX<0) cX=-cX;
    double cY=cellY;
    if(cY<0) cY=-cY;
    if(cX>cY*20)
    {
        cellY=cellY*4;
        Rows/=4;
    }
    if(cX>cY*10)
    {
        cellY=cellY*2;
        Rows/=2;
    }
    if(cY>cX*20)
    {
        cellX=cellX*4;
        Cols/=4;
    }
    if(cY>cX*10)
    {
        cellX=cellX*2;
        Cols/=2;
    }
    labelComputer.SetLeftTop(left,top);
    labelComputer.SetCellSize(cellX,cellY);
    labelComputer.SetCols(Cols);
    labelComputer.SetRows(Rows);

    vector<Point2D<double>>res=labelComputer.GetLabelPoint(this);
    int ValidCount=res.size();
    if(ValidCount==0) return nullptr;
    AutoPtr<Points>labelPoints(new Points());
    for(int k=0;k<ValidCount;k++) labelPoints->Add(res[k].X,res[k].Y);
    return labelPoints;
}

AutoPtr<GeometryCollection>Polygon::Split(Path*pPath){
    vector<AutoPtr<Polygon>>polys=PolygonSplitter::SplitPolygon(this,(Points*)pPath);
    AutoPtr<GeometryCollection>pCol(new GeometryCollection());
    LONG polySize=polys.size();
    for(int k=0;k<polySize;k++)
    {
        pCol->Add(polys[k]);
    }
    return pCol;
}
AutoPtr<Geometry>Polygon::Rebuild(){
   return PolygonOper::ReparePolygon(this);
}

CurvePolygon::CurvePolygon(){

}

CurvePolygon::~CurvePolygon(){

}

AutoPtr<Geometry>CurvePolygon::Clone(){
    CurvePolygon*newpoly=new CurvePolygon();
    for(int k=0;k<parts.size();k++){
        AutoPtr<Points>points(parts[k]);
        AutoPtr<Ring>npts(points->Clone());
        newpoly->parts.push_back(npts);
    }
    return newpoly;
}

double CurvePolygon::GetPerimeter(){
    AutoPtr<Polygon>poly=ConvertToDefaultPolygon(false);
    return poly->GetPerimeter();
}
double CurvePolygon::GetArea(){
    AutoPtr<Polygon>poly=ConvertToDefaultPolygon(false);
    return poly->GetArea();
}
AutoPtr<Polygon>CurvePolygon::ConvertToPolygon(double toler){
    AutoPtr<Polygon>poly(new Polygon());
    bool IsOk;
    for(int k=0;k<parts.size();k++)
    {
        AutoPtr<Ring>path=parts[k];
        AutoPtr<Points>ppts(path);
        AutoPtr<Points>newpts(ppts->Clone());
        newpts->SmoothRing(toler);
        poly->Add(newpts);
    }
    return poly;
}

AutoPtr<Polygon>CurvePolygon::ConvertToDefaultPolygon(bool hQuality){
    AutoPtr<Polygon>poly(new Polygon());
    bool IsOk;
    int num=(hQuality?10:5);
    for(int k=0;k<parts.size();k++)
    {
        AutoPtr<Ring>path=parts[k];
        DOUBLE len=path->GetPerimeter();
        AutoPtr<Points>ppts(path);
        LONG ptCount=ppts->GetSize();
        AutoPtr<Points>newpts(ppts->Clone());
        newpts->SmoothRing(len/(ptCount+1)/num);
        poly->Add(newpts);
    }
    return poly;
}

bool CurvePolygon::CanCaptureBorder(Point*ppt,double toler){
    AutoPtr<Polygon>poly=ConvertToDefaultPolygon(false);
    return poly->CanCaptureBorder(ppt,toler);
}
AutoPtr<Point> CurvePolygon::CaptureBorder(Point*ppt,double toler){
    AutoPtr<Polygon>poly=ConvertToDefaultPolygon(false);
    return poly->CaptureBorder(ppt,toler);
}

AutoPtr<GeometryCollection>CurvePolygon::Split(Path*pPath){
    AutoPtr<Polygon>poly=ConvertToDefaultPolygon(false);
    vector<AutoPtr<Polygon>>polys=PolygonSplitter::SplitPolygon(poly.get(),(Points*)pPath);
    AutoPtr<GeometryCollection>pCol(new GeometryCollection());
    LONG polySize=polys.size();
    for(int k=0;k<polySize;k++)
    {
        pCol->Add(polys[k]);
    }
    return pCol;
}
string CurvePolygon::GetTypeName(){
    return "CurvePolygon";
}

}
