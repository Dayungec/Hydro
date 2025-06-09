#include "ellipse.h"
#include "Base/classfactory.h"
#include "math.h"
#include "points.h"
#include "polygon.h"
#include "spatialreference.h"
#include "segmentindexes.h"

namespace SGIS{

REGISTER(Ellipse)

Ellipse::Ellipse(){
    centerX=centerY=width=height=0;
}
Ellipse::Ellipse(double centerX,double centerY,double width,double height){
    this->centerX=centerX;
    this->centerY=centerY;
    this->width=width;
    this->height=height;
}
Ellipse::Ellipse(const Ellipse&other){
    this->centerX=other.centerX;
    this->centerY=other.centerY;
    this->width=other.width;
    this->height=other.height;
}
Ellipse::~Ellipse(){

}

Ellipse&Ellipse::operator=(const Ellipse&other){
    this->centerX=other.centerX;
    this->centerY=other.centerY;
    this->width=other.width;
    this->height=other.height;
    return *this;
}
//IGeometry:
AutoPtr<Envelope>Ellipse::GetExtent(){
    AutoPtr<Envelope>pEnv(new Envelope());
    pEnv->PutCoord(centerX-width/2,centerY+height/2,centerX+width/2,centerY-height/2);
    return pEnv;
}
GeometryType Ellipse::GetType(){
    return gtEllipse;
}
AutoPtr<Geometry>Ellipse::Clone(){
    Ellipse*elli=new Ellipse();
    elli->centerX=centerX;
    elli->centerY=centerY;
    elli->width=width;
    elli->height=height;
    return elli;
}
bool Ellipse::Equals(Geometry*other){
    if(GetType()!=other->GetType()) return false;
    Ellipse*eo=(Ellipse*)other;
    if(eo->centerX!=centerX) return false;
    if(eo->centerY!=centerY) return false;
    if(eo->width!=width) return false;
    if(eo->height!=height) return false;
    return true;
}

bool Ellipse::Intersects(Geometry*other){
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
        AutoPtr<Envelope>pEnv=pElli->GetExtent();
        AutoPtr<Envelope>tEnv=this->GetExtent();
        if(!tEnv->Intersects(pEnv.get())) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        AutoPtr<Polygon>poly2=pElli->ConvertToPolygon(true);
        AutoPtr<Ring>pRing=poly->GetItem(0);
        AutoPtr<Ring>pRing2=poly2->GetItem(0);
        SegmentIndexes si(poly,8);
        si.BuildSpatialIndex();
        if(si.IntersectRing(pRing2.get()))
        {
            return true;
        }
        AutoPtr<Point>ppt(new Point);
        DOUBLE X,Y;
        AutoPtr<Points>ppts=pRing;
        ppts->GetItem(0,X,Y);
        ppt->PutCoord(X,Y);
        bool IsIn=poly2->IsPointIn(ppt.get());
        if(IsIn)
        {
            return true;
        }
        AutoPtr<Points>ppts2=pRing2;
        ppts2->GetItem(0,X,Y);
        ppt->PutCoord(X,Y);
        return poly->IsPointIn(ppt.get());
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        LONG ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        AutoPtr<Point>ppt(new Point());
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            ppt->PutCoord(ox,oy);
            IsIn=Intersects(ppt.get());
            if(IsIn) return true;
        }
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polyline->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        AutoPtr<Polyline>pply=Geometry::ConvertToPolyline(polyline.get());
        return pply->Intersects(poly.get());
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polygon->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        AutoPtr<Polygon>ppoly=Geometry::ConvertToPolygon(polygon.get());
        return ppoly->Intersects(poly.get());
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=NULL)
    {
        AutoPtr<Envelope>pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        LONG partCount=mpolygon->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly=mpolygon->GetItem(k);
            IsIn=Intersects(poly.get());
            if(IsIn) return true;
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=NULL)
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
}
bool Ellipse::Disjoint(Geometry*other){
    return (!Intersects(other));
}
bool Ellipse::Touches(Geometry*other){
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
        AutoPtr<Envelope>pEnv=pElli->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        AutoPtr<Polygon>poly2=pElli->ConvertToPolygon(true);
        return poly->Touches(poly2.get());
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        LONG ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        AutoPtr<Point>ppt(new Point());
        bool IsTouch;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            ppt->PutCoord(ox,oy);
            IsTouch=ppt->Touches(this);
            if(IsTouch) return true;
        }
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polyline->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        AutoPtr<Polyline>pply=ConvertToPolyline(polyline.get());
        return pply->Touches(poly.get());
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polygon->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        AutoPtr<Polygon>ppoly=Geometry::ConvertToPolygon(polygon.get());
        return ppoly->Touches(poly.get());
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        LONG partCount=mpolygon->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly=mpolygon->GetItem(k);
            IsIn=Touches(poly.get());
            if(IsIn) return true;
        }
        return false;
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
bool Ellipse::Crosses(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr) return false;
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Crosses(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=NULL)
    {
        return pEnv->Crosses(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=NULL)
    {
        AutoPtr<Envelope>pEnv=pElli->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        SegmentIndexes si(poly,8);
        si.BuildSpatialIndex();
        AutoPtr<Polygon>poly2=pElli->ConvertToPolygon(true);
        AutoPtr<Ring>pRing=poly2->GetItem(0);
        int state=si.IsRingCrossRing(pRing.get());
        return (state==1);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr) return false;
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polyline->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        AutoPtr<Polyline>pply=ConvertToPolyline(polyline.get());
        return poly->Crosses(pply.get());
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polygon->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        AutoPtr<Polygon>ppoly=Geometry::ConvertToPolygon(polygon.get());
        return poly->Crosses(ppoly.get());
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>tEnvi=GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        LONG partCount=mpolygon->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly=mpolygon->GetItem(k);
            IsIn=Crosses(poly.get());
            if(IsIn) return true;
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo=pCol->GetItem(k);
            IsInter=Crosses(pGeo.get());
            if(IsInter) return true;
        }
        return false;
    }
    return false;
}
bool Ellipse::Within(Geometry*other){
    AutoPtr<Envelope>tEnvi=GetExtent();
    AutoPtr<Envelope>pEnv=other->GetExtent();
    bool IsIn=pEnv->Contains(tEnvi.get());
    if(!IsIn) return false;
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr) return false;
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr) return false;
    AutoPtr<Envelope>pEnv2=AutoPtr<Geometry>(other,true);
    if(pEnv2!=nullptr)
    {
        return pEnv2->Contains(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Contains(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr) return false;
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr) return false;
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Polygon>ppoly=Geometry::ConvertToPolygon(polygon.get());
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        return poly->Within(ppoly.get());
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        LONG partCount=mpolygon->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly=mpolygon->GetItem(k);
            IsIn=Within(poly.get());
            if(!IsIn) return false;
        }
        return true;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo=pCol->GetItem(k);
            IsInter=Within(pGeo.get());
            if(!IsInter) return false;
        }
        return true;
    }
    return false;
}
bool Ellipse::Contains(Geometry*other){
    AutoPtr<Envelope>tEnvi=GetExtent();
    AutoPtr<Envelope>pEnv=other->GetExtent();
    bool IsIn=pEnv->Within(tEnvi.get());
    if(!IsIn) return false;
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
    AutoPtr<Envelope>pEnv2=AutoPtr<Geometry>(other,true);
    if(pEnv2!=nullptr)
    {
        return pEnv2->Within(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=NULL)
    {
        AutoPtr<Polygon>ppoly=pElli->ConvertToPolygon(true);
        return Contains(ppoly.get());
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=NULL)
    {
        LONG ptCount=ppts->GetSize();
        AutoPtr<Point>ppt(new Point());
        for(int k=0;k<ptCount;k++)
        {
            DOUBLE X,Y;
            ppts->GetItem(0,X,Y);
            ppt->PutCoord(X,Y);
            IsIn=ppt->Within(this);
            if(!IsIn) return false;
        }
        return true;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=NULL)
    {
        AutoPtr<Polyline>ppoly=ConvertToPolyline(polyline.get());
        LONG partCount=ppoly->GetSize();
        if(partCount==0) return false;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Path>pPath=ppoly->GetItem(k);
            AutoPtr<Points>points=pPath;
            LONG ptCount=points->GetSize();
            DOUBLE x,y;
            for(int p=0;p<ptCount;p++)
            {
                points->GetItem(p,x,y);
                if((x/width)*(x/width)+(y/height)*(y/height)>1+VertexTolerance) return false;
            }
        }
        return true;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Polygon>ppoly=Geometry::ConvertToPolygon(polygon.get());
        LONG partCount=ppoly->GetSize();
        if(partCount==0) return false;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Ring>pPath=ppoly->GetItem(k);
            AutoPtr<Points>points=pPath;
            LONG ptCount=points->GetSize();
            DOUBLE x,y;
            for(int p=0;p<ptCount;p++)
            {
                points->GetItem(p,x,y);
                if((x/width)*(x/width)+(y/height)*(y/height)>1+VertexTolerance) return false;
            }
        }
        return true;
    }

    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        LONG partCount=mpolygon->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly=mpolygon->GetItem(k);
            IsIn=Contains(poly.get());
            if(!IsIn) return false;
        }
        return true;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo=pCol->GetItem(k);
            IsInter=Contains(pGeo.get());
            if(!IsInter) return false;
        }
        return true;
    }
    return false;
}
bool Ellipse::Overlaps(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr) return false;
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr) return false;
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Overlaps(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Intersects(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr) return false;
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr) return false;
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=NULL)
    {
        AutoPtr<Envelope>tEnvi=GetExtent();
        AutoPtr<Envelope>pEnv=other->GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        return poly->Overlaps(polygon.get());
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>tEnvi=GetExtent();
        AutoPtr<Envelope>pEnv=other->GetExtent();
        bool IsIn=tEnvi->Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon(true);
        return poly->Overlaps(mpolygon.get());
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo=pCol->GetItem(k);
            IsInter=Overlaps(pGeo.get());
            if(IsInter) return true;
        }
        return false;
    }
    return false;
}

void Ellipse::AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY){
    DOUBLE X,Y;
    AnchorPoint->GetCoord(&X,&Y);
    if(RotateAngle!=0)
    {
        double px,py;
        double cosv=cos(RotateAngle);
        double sinv=sin(RotateAngle);
        px=centerX-X;
        py=centerY-Y;
        centerX=(px*cosv-py*sinv)*ZoomXRatio+DiffX+X;
        centerY=(px*sinv+py*cosv)*ZoomYRatio+DiffY+Y;
        width=width*ZoomXRatio;
        height=height*ZoomYRatio;
    }
    else
    {
        centerX=(centerX-X)*ZoomXRatio+DiffX+X;
        centerY=(centerY-Y)*ZoomYRatio+DiffY+Y;
        width=width*ZoomXRatio;
        height=height*ZoomYRatio;
    }
}
bool Ellipse::Project(CoordinateTransformation*pTrans){
    bool CanTransform=pTrans->CanTransform();
    if(!CanTransform) return false;
    AutoPtr<Points>pts=ReturnPoints(false);
    pTrans->TransformPoints(pts.get());
    AutoPtr<Envelope>prt=pts->GetExtent();
    DOUBLE XMin,YMin,XMax,YMax;
    prt->GetCoord(&XMin,&YMax,&XMax,&YMin);
    centerX=(XMin+XMax)/2;
    centerY=(YMin+YMax)/2;
    width=XMax-XMin;
    height=YMax-YMin;
    return true;
}
void Ellipse::Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr){
    centerX=centerX*xr;
    centerY=centerY*yr;
    width=width*xr;
    height=height*yr;
}
void Ellipse::Move(DOUBLE difx,DOUBLE dify,DOUBLE difz){
    centerX+=difx;
    centerY+=dify;
}
//BufferWritable:
bool Ellipse::Read(BufferReader*br){
    centerX=br->Read<double>();
    centerY=br->Read<double>();
    width=br->Read<double>();
    height=br->Read<double>();
    return true;
}
void Ellipse::Write(BufferWriter*bw){
    bw->Write<double>(centerX);
    bw->Write<double>(centerY);
    bw->Write<double>(width);
    bw->Write<double>(height);
}
string Ellipse::GetTypeName(){
    return "Ellipse";
}
//JsonWritable:
bool Ellipse::ReadFromJson(JsonReader*jr,JsonNode node){
    if(node.size()<4) return false;
    centerX=node[0].asDouble();
    centerY=node[1].asDouble();
    width=node[2].asDouble();
    height=node[3].asDouble();
}
string Ellipse::ToJson(){
    JsonArrayWriter writer;
    writer.Add<double>(centerX);
    writer.Add<double>(centerY);
    writer.Add<double>(width);
    writer.Add<double>(height);
    return writer.GetJsonString();
}

AutoPtr<Points>Ellipse::ReturnPoints(bool HighQuality){
    AutoPtr<Points>pNew(new Points());
    if((width==0)&&(height==0))
    {
        pNew->Add(centerX,centerY);
        return pNew;
    }
    double difPi;
    if(HighQuality)
        difPi=PI/64;
    else
        difPi=PI/32;
    double sina,cosa;
    double X,Y;
    double sa=width/2;
    double sb=height/2;
    double sa2=sa*sa;
    double sb2=sb*sb;
    double p;
    for(float a=DPI;a>0;a-=difPi)
    {
        sina=sin(a);
        cosa=cos(a);
        p=sa*sb/sqrt(sa2*sina*sina+sb2*cosa*cosa);
        X=centerX+p*cosa;
        Y=centerY+p*sina;
        pNew->Add(X,Y);
    }
    pNew->Add(X,Y);
    return pNew;
}

AutoPtr<Polygon>Ellipse::ConvertToPolygon(bool highQuality){
    AutoPtr<Points>pNew=ReturnPoints(highQuality);
    AutoPtr<Ring>pRing=pNew;
    AutoPtr<Polygon>poly(new Polygon());
    poly->Add(pRing);
    return poly;
}
void Ellipse::SetCenterPoint(Point*centerPt){
    centerPt->GetCoord(&centerX,&centerY);
}

double&Ellipse::CenterX(){
    return centerX;
}

double&Ellipse::CenterY(){
   return centerY;
}

double&Ellipse::Width(){
    return width;
}
double&Ellipse::Height(){
    return height;
}

void Ellipse::PutCoord(double centerX,double centerY,double width,double height){
    this->centerX=centerX;
    this->centerY=centerY;
    this->width=width;
    this->height=height;
}

void Ellipse::GetCoord(double*centerX,double*centerY,double*width,double*height){
    *centerX=this->centerX;
    *centerY=this->centerY;
    *width=this->width;
    *height=this->height;
}

}

