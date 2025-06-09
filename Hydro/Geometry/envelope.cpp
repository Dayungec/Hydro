#include "envelope.h"
#include "math.h"
#include "spatialreference.h"
#include "Base/classfactory.h"
#include "points.h"
#include "polygon.h"
#include "geometryoperator.h"

namespace SGIS{

REGISTER(Envelope)

Envelope::Envelope(){
    left=top=right=bottom=0;
    floor=roof=0;
}

Envelope::Envelope(double left,double top,double right,double bottom,double floor,double roof){
    this->left=left;
    this->top=top;
    this->right=right;
    this->bottom=bottom;
    this->floor=floor;
    this->roof=roof;
}

Envelope::Envelope(const Envelope&other){
    this->left=other.left;
    this->top=other.top;
    this->right=other.right;
    this->bottom=other.bottom;
    this->floor=other.floor;
    this->roof=other.roof;
}

Envelope::~Envelope(){

}

Envelope&Envelope::operator=(const Envelope&other){
    this->left=other.left;
    this->top=other.top;
    this->right=other.right;
    this->bottom=other.bottom;
    this->floor=other.floor;
    this->roof=other.roof;
    return *this;
}

AutoPtr<Geometry>Envelope::Clone(){
    return new Envelope(left,top,right,bottom,floor,roof);
}

bool Envelope::Equals(Geometry*other){
    if(GetType()!=other->GetType()) return false;
    Envelope*gs=(Envelope*)other;
    return ((left==gs->left)&&(top==gs->top)&&(right==gs->right)&&(bottom==gs->bottom));
}

bool Envelope::Intersects(Geometry*other){
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
        return IntersectsEnvelope(pEnv.get());
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        AutoPtr<Envelope>pEnv=pElli->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        IsIn=pEnv->Within(this);
        if(IsIn) return true;
        DOUBLE W,H;
        W=pElli->Width();
        H=pElli->Height();
        if((left/W)*(left/W)+(top/H)*(top/H)<=1+VertexTolerance) return true;
        if((left/W)*(left/W)+(bottom/H)*(bottom/H)<=1+VertexTolerance) return true;
        if((right/W)*(right/W)+(bottom/H)*(bottom/H)<=1+VertexTolerance) return true;
        if((right/W)*(right/W)+(top/H)*(top/H)<=1+VertexTolerance) return true;
        AutoPtr<Polygon>poly=pElli->ConvertToPolygon(true);
        AutoPtr<Ring>pRing=poly->GetItem(0);
        AutoPtr<Points>ppts=pRing;
        LONG ptCount=ppts->GetSize();
        AutoPtr<Point>ppt(new Point());
        DOUBLE X,Y;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,X,Y);
            ppt->PutCoord(X,Y);
            IsIn=IsPointIn(ppt.get());
            if(IsIn) return true;
        }
        return false;
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        IsIn=pEnv->Within(this);
        if(IsIn) return true;
        LONG ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        AutoPtr<Point>ppt(new Point());
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            ppt->PutCoord(ox,oy);
            IsIn=IsPointIn(ppt.get());
            if(IsIn) return true;
        }
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polyline->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        IsIn=pEnv->Within(this);
        if(IsIn) return true;
        AutoPtr<Polygon>poly=ConvertToPolygon();
        AutoPtr<Polyline>pply=ConvertToPolyline(polyline.get());
        return poly->Intersects(pply.get());
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polygon->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        IsIn=pEnv->Within(this);
        if(IsIn) return true;
        AutoPtr<Polygon>poly=ConvertToPolygon();
        AutoPtr<Polygon>ppoly=Geometry::ConvertToPolygon(polygon.get());
        return poly->Intersects(ppoly.get());
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=mpolygon->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        IsIn=pEnv->Within(this);
        if(IsIn) return true;
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
bool Envelope::Disjoint(Geometry*other){
    return (!Intersects(other));
}
bool Envelope::Touches(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
     if(ppt!=nullptr)
    {
        return ppt->Touches(this);
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Intersects(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon();
        AutoPtr<Polygon>poly2=pEnv->ConvertToPolygon();
        return poly->Touches(poly2.get());
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        AutoPtr<Envelope>pEnv=pElli->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon();
        AutoPtr<Polygon>poly2=pElli->ConvertToPolygon(true);
        return poly->Touches(poly2.get());
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        AutoPtr<Envelope>pEnv=ppts->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        LONG ptCount=ppts->GetSize();
        DOUBLE ox,oy;
        AutoPtr<Point>ppt(new Point());
        bool IsTouch;
        for(int k=0;k<ptCount;k++)
        {
            ppts->GetItem(k,ox,oy);
            ppt->PutCoord(ox,oy);
            IsTouch=Touches(ppt.get());
            if(IsTouch) return true;
        }
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polyline->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon();
        AutoPtr<Polyline>pply=ConvertToPolyline(polyline.get());
        return poly->Touches(pply.get());
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polygon->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon();
        AutoPtr<Polygon>ppoly=Geometry::ConvertToPolygon(polygon.get());
        return poly->Touches(ppoly.get());
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polygon->GetExtent();
        bool IsIn=Intersects(pEnv.get());
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
    if(pCol!=nullptr)
    {
        LONG subCount;
        subCount=pCol->GetSize();
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
bool Envelope::Crosses(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr) return false;
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Crosses(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon();
        AutoPtr<Polygon>poly2=pEnv->ConvertToPolygon();
        return poly->Crosses(poly2.get());
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        AutoPtr<Envelope>pEnv=pElli->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon();
        AutoPtr<Polygon>poly2;
        poly2=pElli->ConvertToPolygon(true);
        return poly->Crosses(poly2.get());
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr) return false;
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polyline->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly=ConvertToPolygon();
        AutoPtr<Polyline>pply=ConvertToPolyline(polyline.get());
        return poly->Crosses(pply.get());
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polygon->GetExtent();
        bool IsIn;
        IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly;
        poly=ConvertToPolygon();
        AutoPtr<Polygon>ppoly=Geometry::ConvertToPolygon(polygon.get());
        return poly->Crosses(ppoly.get());
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=polygon->GetExtent();
        bool IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        LONG partCount;
        partCount=mpolygon->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            mpolygon->GetItem(k);
            IsIn=Crosses(poly.get());
            if(IsIn) return true;
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
            if(IsInter) return true;
        }
        return false;
    }
    return false;
}
bool Envelope::Within(Geometry*other){
    AutoPtr<Envelope>pEnv=other->GetExtent();
    if(pEnv!=nullptr)
    {
        DOUBLE xMin,yMin,xMax,yMax;
        pEnv->GetCoord(&xMin,&yMax,&xMax,&yMin);
        if(((right<=xMax+VertexTolerance)&&(left>=xMin-VertexTolerance)&&(top<=yMax+VertexTolerance)&&(bottom>=yMin-VertexTolerance))==false) return false;
    }
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr) return false;
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr) return false;
    AutoPtr<Envelope>pEnv2=AutoPtr<Geometry>(other,true);
    if(pEnv2!=nullptr) return true;
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        DOUBLE W,H;
        W=pElli->Width();
        H=pElli->Height();
        if((left/W)*(left/W)+(top/H)*(top/H)>1+VertexTolerance) return false;
        if((left/W)*(left/W)+(bottom/H)*(bottom/H)>1+VertexTolerance) return false;
        if((right/W)*(right/W)+(bottom/H)*(bottom/H)>1+VertexTolerance) return false;
        if((right/W)*(right/W)+(top/H)*(top/H)>1+VertexTolerance) return false;
        return true;
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr) return false;
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr) return false;
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Polygon>ppoly=Geometry::ConvertToPolygon(polygon.get());
        AutoPtr<Polygon>poly;
        poly=ConvertToPolygon();
        return poly->Within(ppoly.get());
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        LONG partCount;
        partCount=mpolygon->GetSize();
        bool IsIn;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            mpolygon->GetItem(k);
            IsIn=Within(poly.get());
            if(!IsIn) return false;
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
            IsInter=Within(pGeo.get());
            if(!IsInter) return false;
        }
        return true;
    }
    return false;
}
bool Envelope::Contains(Geometry*other){
    return other->Within(this);
}
bool Envelope::Overlaps(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr) return false;
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr) return false;
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
    if(ppts!=nullptr) return false;
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr) return false;
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=polygon->GetExtent();
        bool IsIn;
        IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly;
        poly=ConvertToPolygon();
        return poly->Overlaps(polygon.get());
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv=mpolygon->GetExtent();
        bool IsIn;
        IsIn=Intersects(pEnv.get());
        if(!IsIn) return false;
        AutoPtr<Polygon>poly;
        poly=ConvertToPolygon();
        return poly->Overlaps(mpolygon.get());
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
            if(IsInter) return true;
        }
        return false;
    }
    return false;
}

void Envelope::AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY){
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
        px=left-X;
        py=top-Y;
        left=(px*cosv-py*sinv)*ZoomXRatio+difx;
        top=(px*sinv+py*cosv)*ZoomYRatio+dify;
        px=right-X;
        py=bottom-Y;
        right=(px*cosv-py*sinv)*ZoomXRatio+difx;
        bottom=(px*sinv+py*cosv)*ZoomYRatio+dify;
    }
    else
    {
        left=(left-X)*ZoomXRatio+difx;
        top=(top-Y)*ZoomYRatio+dify;
        right=(right-X)*ZoomXRatio+difx;
        bottom=(bottom-Y)*ZoomYRatio+dify;
    }
    if(left>right)
    {
        double temp=left;
        left=right;
        right=temp;
    }
    if(bottom>top)
    {
        double temp=bottom;
        bottom=top;
        top=temp;
    }
}

bool Envelope::Project(CoordinateTransformation*pTrans){
    bool CanTransform=pTrans->CanTransform();
    if(!CanTransform) return false;
    DOUBLE XMin,YMin,XMax,YMax;
    GetCoord(&XMin,&YMax,&XMax,&YMin);
    double DifX=(XMax-XMin)/5;
    double DifY=(YMax-YMin)/5;
    DOUBLE desx,desy;
    bool HasSuc=false;
    DOUBLE xmin,ymin,xmax,ymax;
    for(int i=0;i<6;i++)
    {
        for(int j=0;j<6;j++)
        {
            desy=YMin+DifY*i;
            desx=XMin+DifX*j;
            if(pTrans->TransformXY(&desx,&desy))
            {
                if(HasSuc)
                {
                    if(desx<xmin) xmin=desx;
                    if(desx>xmax) xmax=desx;
                    if(desy<ymin) ymin=desy;
                    if(desy>ymax) ymax=desy;
                }
                else
                {
                    xmin=xmax=desx;
                    ymin=ymax=desy;
                }
                HasSuc=true;
            }
        }
    }
    if(HasSuc)
    {
        if(YMax<YMin)
            PutCoord(xmin,ymin,xmax,ymax);
        else
            PutCoord(xmin,ymax,xmax,ymin);
    }
    return HasSuc;
}

void Envelope::Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr){
    left*=xr;
    top*=yr;
    right*=xr;
    bottom*=yr;
    floor*=zr;
    roof*=zr;
}
void Envelope::Move(DOUBLE difx,DOUBLE dify,DOUBLE difz){
    left+=difx;
    top+=dify;
    right+=difx;
    bottom+=dify;
    floor+=difz;
    roof+=difz;
}
void Envelope::ParseInt(){
    left=(int)left;
    top=(int)top;
    right=(int)right;
    bottom=(int)bottom;
}
GeometryType Envelope::GetType(){
    return gtEnvelope;
}
AutoPtr<Envelope>Envelope::GetExtent(){
    return AutoPtr<Envelope>(new Envelope(left,top,right,bottom));
}

bool Envelope::Read(BufferReader*br){
    left=br->Read<double>();
    top=br->Read<double>();
    right=br->Read<double>();
    bottom=br->Read<double>();
    return true;
}

void Envelope::Write(BufferWriter*bw){
    bw->Write<double>(left);
    bw->Write<double>(top);
    bw->Write<double>(right);
    bw->Write<double>(bottom);
}

string Envelope::GetTypeName(){
    return "Envelope";
}

bool Envelope::ReadFromJson(JsonReader*jr,JsonNode node){
    if(node.size()<4) return false;
    left=node[0].asDouble();
    top=node[1].asDouble();
    right=node[2].asDouble();
    bottom=node[3].asDouble();
    return true;
}

string Envelope::ToJson(){
    JsonArrayWriter writer;
    writer.Add<double>(left);
    writer.Add<double>(top);
    writer.Add<double>(right);
    writer.Add<double>(bottom);
    return writer.GetJsonString();
}
AutoPtr<Envelope>Envelope::GetExtent3D(){
    return AutoPtr<Envelope>(new Envelope(left,top,right,bottom,floor,roof));
}

void Envelope::Rotate(DOUBLE RotateAngle,Vector*axis){
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
    DOUBLE xmin,ymin,zmin,xmax,ymax,zmax;
    DOUBLE x,y,z;
    x=left;y=top;z=floor;
    innerRotate(sina,cosa,rx,ry,rx,x,y,z);
    xmin=xmax=x;
    ymin=ymax=y;
    zmin=zmax=z;

    x=left;y=top;z=roof;
    innerRotate(sina,cosa,rx,ry,rz,x,y,z);
    xmin=min(x,xmin);ymin=min(y,ymin);zmin=min(z,zmin);
    xmax=max(x,xmax);ymax=max(y,ymax);zmax=max(z,zmax);

    x=left;y=bottom;z=floor;
    innerRotate(sina,cosa,rx,ry,rz,x,y,z);
    xmin=min(x,xmin);ymin=min(y,ymin);zmin=min(z,zmin);
    xmax=max(x,xmax);ymax=max(y,ymax);zmax=max(z,zmax);

    x=left;y=bottom;z=roof;
    innerRotate(sina,cosa,rx,ry,rz,x,y,z);
    xmin=min(x,xmin);ymin=min(y,ymin);zmin=min(z,zmin);
    xmax=max(x,xmax);ymax=max(y,ymax);zmax=max(z,zmax);

    x=right;y=top;z=floor;
    innerRotate(sina,cosa,rx,ry,rz,x,y,z);
    xmin=min(x,xmin);ymin=min(y,ymin);zmin=min(z,zmin);
    xmax=max(x,xmax);ymax=max(y,ymax);zmax=max(z,zmax);

    x=right;y=top;z=roof;
    innerRotate(sina,cosa,rx,ry,rz,x,y,z);
    xmin=min(x,xmin);ymin=min(y,ymin);zmin=min(z,zmin);
    xmax=max(x,xmax);ymax=max(y,ymax);zmax=max(z,zmax);

    x=right;y=bottom;z=floor;
    innerRotate(sina,cosa,rx,ry,rz,x,y,z);
    xmin=min(x,xmin);ymin=min(y,ymin);zmin=min(z,zmin);
    xmax=max(x,xmax);ymax=max(y,ymax);zmax=max(z,zmax);

    x=right;y=bottom;z=roof;
    innerRotate(sina,cosa,rx,ry,rz,x,y,z);
    xmin=min(x,xmin);ymin=min(y,ymin);zmin=min(z,zmin);
    xmax=max(x,xmax);ymax=max(y,ymax);zmax=max(z,zmax);
    if(bottom>top)
    {
        left=xmin;top=ymin;floor=zmin;right=xmax;bottom=ymax;roof=zmax;
    }
    else
    {
        left=xmin;bottom=ymin;floor=zmin;right=xmax;top=ymax;roof=zmax;
    }
}
bool Envelope::Read3D(BufferReader*br){
    left=br->Read<double>();
    top=br->Read<double>();
    right=br->Read<double>();
    bottom=br->Read<double>();
    floor=br->Read<double>();
    roof=br->Read<double>();
    return true;
}

void Envelope::Write3D(BufferWriter*bw){
    bw->Write<double>(left);
    bw->Write<double>(top);
    bw->Write<double>(right);
    bw->Write<double>(bottom);
    bw->Write<double>(floor);
    bw->Write<double>(roof);
}
bool Envelope::ReadFromJson3D(JsonReader*jr,JsonNode node){
    if(node.size()<4) return false;
    left=node[0].asDouble();
    top=node[1].asDouble();
    right=node[2].asDouble();
    bottom=node[3].asDouble();
    if(node.size()>=6){
       floor=node[4].asDouble();
       roof=node[5].asDouble();
    }
    else{
        floor=roof=0;
    }
    return true;
}
string Envelope::ToJson3D(){
    return "[" + to_string(left) + "," + to_string(top)+ "," + to_string(right)+ "," + to_string(bottom)+ "," + to_string(floor)+ "," + to_string(roof)+ "]";
}

void Envelope::GetCoord(double*left,double*top,double*right,double*bottom){
    *left=this->left;
    *top=this->top;
    *right=this->right;
    *bottom=this->bottom;
}

void Envelope::PutCoord(double left,double top,double right,double bottom){
    this->left=left;
    this->top=top;
    this->right=right;
    this->bottom=bottom;
}


double&Envelope::Left(){
    return left;
}
double&Envelope::Top(){
    return top;
}
double&Envelope::Right(){
    return right;
}
double&Envelope::Bottom(){
    return bottom;
}

double&Envelope::Floor(){
    return floor;
}

double&Envelope::Roof(){
    return roof;
}

double Envelope::Width(){
    return right-left;
}
double Envelope::Height(){
    return top-bottom;
}

AutoPtr<Point>Envelope::GetCenterPoint(){
    return new Point((left+right)/2,(top+bottom)/2);
}
void Envelope::Normalize(){
    if(right<left){
        double temp=left;
        left=right;
        right=temp;
    }
    if(top<bottom){
        double temp=top;
        top=bottom;
        bottom=temp;
    }
    if(floor>roof){
        double temp=floor;
        floor=roof;
        roof=temp;
    }
}

void Envelope::NormalizeClient(){
    if(right<left){
        double temp=left;
        left=right;
        right=temp;
    }
    if(top>bottom){
        double temp=top;
        top=bottom;
        bottom=temp;
    }
    if(floor>roof){
        double temp=floor;
        floor=roof;
        roof=temp;
    }
}

bool Envelope::IntersectsEnvelope(Envelope*env){
    if((left>env->right+VertexTolerance)||(right<env->left-VertexTolerance)||(top<env->bottom-VertexTolerance)||(bottom>env->top+VertexTolerance)) return false;
    return true;
}
bool Envelope::IntersectsEnvelopeInClient(Envelope*env){
    if((left>env->right+VertexTolerance)||(right<env->left-VertexTolerance)||(bottom<env->top-VertexTolerance)||(top>env->bottom+VertexTolerance)) return false;
    return true;
}

bool Envelope::WithinEnvelope(Envelope*env){
     return ((left>=env->left-VertexTolerance)&&(right<=env->right+VertexTolerance)&&(bottom>=env->bottom-VertexTolerance)&&(top<=env->top+VertexTolerance));
}

bool Envelope::WithinEnvelopeInClient(Envelope*env){
     return ((left>=env->left-VertexTolerance)&&(right<=env->right+VertexTolerance)&&(top>=env->top-VertexTolerance)&&(bottom<=env->bottom+VertexTolerance));
}

AutoPtr<Envelope>Envelope::IntersectEnvelope(Envelope*env){
    if((left>env->right+VertexTolerance)||(right<env->left-VertexTolerance)||(top<env->bottom-VertexTolerance)||(bottom>env->top+VertexTolerance)) return nullptr;
    return AutoPtr<Envelope>(new Envelope(max(left,env->left),min(top,env->top),min(right,env->right),max(bottom,env->bottom)));
}

AutoPtr<Envelope>Envelope::IntersectEnvelopeInClient(Envelope*env){
    if((left>env->right+VertexTolerance)||(right<env->left-VertexTolerance)||(bottom<env->top-VertexTolerance)||(top>env->bottom+VertexTolerance)) return nullptr;
    return new Envelope(max(left,env->left),max(top,env->top),min(right,env->right),min(bottom,env->bottom));
}

bool Envelope::IsPointIn(Point*point){
    double x,y;
    point->GetCoord(&x,&y);
    return ((x>=left+VertexTolerance)&&(x<=right-VertexTolerance)&&(y>=bottom+VertexTolerance)&&(y<=top-VertexTolerance));
}

bool Envelope::IsPointIn(double x,double y){
    return ((x>=left+VertexTolerance)&&(x<=right-VertexTolerance)&&(y>=bottom+VertexTolerance)&&(y<=top-VertexTolerance));
}
bool Envelope::IsPointInClient(Point*point){
    double x,y;
    point->GetCoord(&x,&y);
    return ((x>=left+VertexTolerance)&&(x<=right-VertexTolerance)&&(y>=top+VertexTolerance)&&(y<=bottom-VertexTolerance));
}

void Envelope::Inflate(double sizex,double sizey,bool isratio){
    if(isratio){
        sizex=(right-left)*sizex;
        sizey=(top-bottom)*sizey;
    }
    left-=sizex;
    right+=sizex;
    bottom-=sizey;
    top+=sizey;
}

void Envelope::InflateClient(double sizex,double sizey,bool isratio){
    if(isratio){
        sizex=(right-left)*sizex;
        sizey=(bottom-top)*sizey;
    }
    left-=sizex;
    right+=sizex;
    bottom+=sizey;
    top-=sizey;
}

void Envelope::Inflate(double sl,double st,double sr,double sb,bool isratio){
    if(isratio){
        sl=(right-left)*sl;
        sr=(right-left)*sr;
        st=(top-bottom)*st;
        sb=(top-bottom)*sb;
    }
    left-=sl;
    right+=sr;
    bottom-=sb;
    top+=st;
}

void Envelope::InflateClient(double sl,double st,double sr,double sb,bool isratio){
    if(isratio){
        sl=(right-left)*sl;
        sr=(right-left)*sr;
        st=(bottom-top)*st;
        sb=(bottom-top)*sb;
    }
    left-=sl;
    right+=sr;
    bottom+=sb;
    top-=st;
}

AutoPtr<Envelope>Envelope::Union(Envelope*env){
    return new Envelope(min(left,env->left),max(top,env->top),max(right,env->right),min(bottom,env->bottom));
}
AutoPtr<Envelope>Envelope::UnionClient(Envelope*env){
    return new Envelope(min(left,env->left),min(top,env->top),max(right,env->right),max(bottom,env->bottom));
}

void Envelope::UnionOther(Envelope*env){
    left=min(left,env->left);
    top=max(top,env->top);
    right=max(right,env->right);
    bottom=min(bottom,env->bottom);
}
void Envelope::UnionOtherClient(Envelope*env){
    left=min(left,env->left);
    top=min(top,env->top);
    right=max(right,env->right);
    bottom=max(bottom,env->bottom);
}

AutoPtr<Points>Envelope::ReturnPoints(){
    AutoPtr<Points>pNew(new Points());
    pNew->Add(left,top);
    pNew->Add(right,top);
    pNew->Add(right,bottom);
    pNew->Add(left,bottom);
    pNew->Add(left,top);
    return pNew;
}
AutoPtr<Polygon>Envelope::ConvertToPolygon(){
    AutoPtr<Points>pNew=ReturnPoints();
    AutoPtr<Ring>pRing=pNew;
    AutoPtr<Polygon>poly(new Polygon());
    poly->Add(pRing);
    return poly;
}

void Envelope::CopyFromOther(Envelope*other){
    PutCoord(other->left,other->top,other->right,other->bottom);
}

AutoPtr<Geometry>Envelope::ClipGeometry(Geometry*geo){
    EnvelopeClipper clipper(left, bottom, right, top);
    return clipper.ClipGeometry(geo);
}
}
