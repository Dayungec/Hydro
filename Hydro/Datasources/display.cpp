#include "display.h"


namespace SGIS{

REGISTER(Display)
Display::Display()
{
    neighborCodes[0]=1;
    neighborCodes[1]=2;
    neighborCodes[2]=3;
    neighborCodes[3]=8;
    neighborCodes[4]=0;
    neighborCodes[5]=4;
    neighborCodes[6]=7;
    neighborCodes[7]=6;
    neighborCodes[8]=5;
    pSpatial=new SpatialReference();
    angle=0;
    ax=0;
    ay=0;
    bx=0;
    by=0;
    angle=0;
    sina=0;
    cosa=1;
    valid=true;
}
Display::~Display()
{

}

int Display::GetDefaultDPI(){
    return 96;
}

AutoPtr<Display>Display::Clone()
{
    Display*pNew=new Display();
    pNew->ClientRect=ClientRect;
    pNew->MapRect=MapRect;
    pNew->cosa=cosa;
    pNew->sina=sina;
    pNew->angle=angle;
    pNew->ComputeTransform();
    pNew->valid=valid;
    if(pSpatial!=NULL)
    {
        pNew->SetSpatialReference(pSpatial->Clone());
    }
    return pNew;
}

AutoPtr<SpatialReference>Display::GetSpatialReference()
{
    return pSpatial->Clone();
}

void Display::SetSpatialReference(AutoPtr<SpatialReference>psp)
{
    pSpatial->CopyFromOther(psp.get());
}

void Display::ComputeTransform()
{
    if(ClientRect.Right-ClientRect.Left==0)
        ax=0;
    else
        ax=(MapRect.Right-MapRect.Left)/(ClientRect.Right-ClientRect.Left);
    //图像的Width/客户区(视图)的Width
    bx=-ClientRect.Left*ax+MapRect.Left;
    if(ClientRect.Bottom-ClientRect.Top==0)
        ay=0;
    else
        ay=(MapRect.Bottom-MapRect.Top)/(ClientRect.Bottom-ClientRect.Top);
    by=-ClientRect.Top*ay+MapRect.Top;
    MapCenterX=(MapRect.Left+MapRect.Right)/2;
    MapCenterY=(MapRect.Top+MapRect.Bottom)/2;
}
void Display::ToMapPoint(DOUBLE*X,DOUBLE*Y)
{
    if(angle==0)
    {
        *X=*X*ax+bx;
        *Y=*Y*ay+by;
    }
    else
    {
        *X=*X*ax+bx;
        *Y=*Y*ay+by;
        DOUBLE x=(*X-MapCenterX)*cosa-(*Y-MapCenterY)*sina;
        DOUBLE y=(*X-MapCenterX)*sina+(*Y-MapCenterY)*cosa;
        *X=x+MapCenterX;
        *Y=y+MapCenterY;
    }
}
void Display::ToMapPoints(DOUBLE*xys,int Count){
    if(angle==0)
    {
        LONG Pos=0;
        for(int k=0;k<Count;k++)
        {
           xys[Pos]=xys[Pos]*ax+bx;
           xys[Pos+1]=xys[Pos+1]*ay+by;
           Pos+=2;
        }
    }
    else
    {
        LONG Pos=0;
        for(int k=0;k<Count;k++)
        {
           xys[Pos]=xys[Pos]*ax+bx;
           xys[Pos+1]=xys[Pos+1]*ay+by;
           DOUBLE x=(xys[Pos]-MapCenterX)*cosa-(xys[Pos+1]-MapCenterY)*sina;
           DOUBLE y=(xys[Pos]-MapCenterX)*sina+(xys[Pos+1]-MapCenterY)*cosa;
           xys[Pos]=x+MapCenterX;
           xys[Pos+1]=y+MapCenterY;
           Pos+=2;
        }
    }
}
void Display::ToMapPoints(vector<double>&XYs)
{
    LONG Count=XYs.size()/2;
    DOUBLE*xys=XYs.data();
    ToMapPoints(xys,Count);
}

double Display::ToMapXDistance(DOUBLE dist)
{
    return dist*ax;
}
double Display::ToMapYDistance(DOUBLE dist)
{
    return dist*ay;
}
double Display::FromMapXDistance(DOUBLE dist)
{
    return dist/ax;
}

double Display::FromMapYDistance(DOUBLE dist)
{
    return dist/ay;
}

void Display::ToMapPoint(Point*pGeo)
{
    DOUBLE X,Y;
    pGeo->GetCoord(&X,&Y);
    if(angle==0)
    {
        X=X*ax+bx;
        Y=Y*ay+by;
    }
    else
    {
        X=X*ax+bx;
        Y=Y*ay+by;
        DOUBLE x=(X-MapCenterX)*cosa-(Y-MapCenterY)*sina;
        DOUBLE y=(X-MapCenterX)*sina+(Y-MapCenterY)*cosa;
        X=x+MapCenterX;
        Y=y+MapCenterY;
    }
    pGeo->PutCoord(X,Y);
}

void Display::ToMapPoints(Points*pGeo)
{
    DOUBLE*xys=pGeo->GetXYs();
    LONG Count=pGeo->GetSize();
    if(angle==0)
    {
        LONG Pos=0;
        for(int k=0;k<Count;k++)
        {
           xys[Pos]=xys[Pos]*ax+bx;
           xys[Pos+1]=xys[Pos+1]*ay+by;
           Pos+=2;
        }
    }
    else
    {
        LONG Pos=0;
        for(int k=0;k<Count;k++)
        {
           xys[Pos]=xys[Pos]*ax+bx;
           xys[Pos+1]=xys[Pos+1]*ay+by;
           DOUBLE x=(xys[Pos]-MapCenterX)*cosa-(xys[Pos+1]-MapCenterY)*sina;
           DOUBLE y=(xys[Pos]-MapCenterX)*sina+(xys[Pos+1]-MapCenterY)*cosa;
           xys[Pos]=x+MapCenterX;
           xys[Pos+1]=y+MapCenterY;
           Pos+=2;
        }
    }
    pGeo->ComputeEnvelope();
}
void Display::ToMapSegment(Segment*pGeo)
{
    DOUBLE lnX1,lnY1,lnX2,lnY2;
    pGeo->GetCoord(&lnX1,&lnY1,&lnX2,&lnY2);
    if(angle==0)
    {
        lnX1=lnX1*ax+bx;
        lnY1=lnY1*ay+by;
        lnX2=lnX2*ax+bx;
        lnY2=lnY2*ay+by;
    }
    else
    {
        lnX1=lnX1*ax+bx;
        lnY1=lnY1*ay+by;
        DOUBLE x=(lnX1-MapCenterX)*cosa-(lnY1-MapCenterY)*sina;
        DOUBLE y=(lnX1-MapCenterX)*sina+(lnY1-MapCenterY)*cosa;
        lnX1=x+MapCenterX;
        lnY1=y+MapCenterY;
        lnX2=lnX2*ax+bx;
        lnY2=lnY2*ay+by;
        x=(lnX2-MapCenterX)*cosa-(lnY2-MapCenterY)*sina;
        y=(lnX2-MapCenterX)*sina+(lnY2-MapCenterY)*cosa;
        lnX2=x+MapCenterX;
        lnY2=y+MapCenterY;
    }
    pGeo->PutCoord(lnX1,lnY1,lnX2,lnY2);
}
void Display::ToMapEllipse(Ellipse*pGeo)
{
    AutoPtr<Points>pts=pGeo->ReturnPoints(false);
    ToMapPoints(pts.get());
    AutoPtr<Envelope>rect=pts->GetExtent();
    double Width,Height;
    Width=rect->Width();
    Height=rect->Height();
    pGeo->Width()=Width;
    pGeo->Height()=Height;
    AutoPtr<Point>ppt=rect->GetCenterPoint();
    pGeo->SetCenterPoint(ppt.get());
}
void Display::ToMapEnvelope(Envelope*pGeo)
{
    if(angle==0)
    {
        DOUBLE XMin,YMin,XMax,YMax;
        pGeo->GetCoord(&XMin,&YMin,&XMax,&YMax);
        pGeo->PutCoord(XMin*ax+bx,YMin*ay+by,XMax*ax+bx,YMax*ay+by);
    }
    else
    {
        DOUBLE XMin,YMin,XMax,YMax;
        pGeo->GetCoord(&XMin,&YMin,&XMax,&YMax);
        double xmin,ymin,xmax,ymax;
        DOUBLE x,y,X,Y;
        X=XMin*ax+bx;
        Y=YMin*ay+by;
        x=(X-MapCenterX)*cosa-(Y-MapCenterY)*sina+MapCenterX;
        y=(X-MapCenterX)*sina+(Y-MapCenterY)*cosa+MapCenterY;
        xmin=xmax=x;
        ymin=ymax=y;
        X=XMin*ax+bx;
        Y=YMax*ay+by;
        x=(X-MapCenterX)*cosa-(Y-MapCenterY)*sina+MapCenterX;
        y=(X-MapCenterX)*sina+(Y-MapCenterY)*cosa+MapCenterY;
        if(x<xmin) xmin=x;
        if(x>xmax) xmax=x;
        if(y<ymin) ymin=y;
        if(y>ymax) ymax=y;
        X=XMax*ax+bx;
        Y=YMax*ay+by;
        x=(X-MapCenterX)*cosa-(Y-MapCenterY)*sina+MapCenterX;
        y=(X-MapCenterX)*sina+(Y-MapCenterY)*cosa+MapCenterY;
        if(x<xmin) xmin=x;
        if(x>xmax) xmax=x;
        if(y<ymin) ymin=y;
        if(y>ymax) ymax=y;
        X=XMax*ax+bx;
        Y=YMin*ay+by;
        x=(X-MapCenterX)*cosa-(Y-MapCenterY)*sina+MapCenterX;
        y=(X-MapCenterX)*sina+(Y-MapCenterY)*cosa+MapCenterY;
        if(x<xmin) xmin=x;
        if(x>xmax) xmax=x;
        if(y<ymin) ymin=y;
        if(y>ymax) ymax=y;
        pGeo->PutCoord(xmin,ymax,xmax,ymin);
    }
}
void Display::ToMapPolyline(Polyline*pGeo)
{
    LONG Count=pGeo->GetSize();
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Points>path=pGeo->GetItem(k);
        ToMapPoints(path.get());
    }
}
void Display::ToMapPolygon(Polygon*pGeo)
{
    LONG Count=pGeo->GetSize();
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Points>path=pGeo->GetItem(k);
        ToMapPoints(path.get());
    }
}
void Display::ToMapMultiPolygon(MultiPolygon*pGeo)
{
    LONG Count=pGeo->GetSize();
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Polygon>poly=pGeo->GetItem(k);
        ToMapPolygon(poly.get());
    }
}
void Display::ToMapCollection(GeometryCollection*pGeo)
{
    LONG Count=pGeo->GetSize();
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Geometry>poly=pGeo->GetItem(k);
        ToMapGeometry(poly.get());
    }
}
void Display::ToMapGeometry(Geometry*pGeo)
{
    GeometryType type=pGeo->GetType();
    switch(type)
    {
    case gtPoint:
        ToMapPoint((Point*)pGeo);
        break;
    case gtPoints:
        ToMapPoints((Points*)pGeo);
        break;
    case gtSegment:
        ToMapSegment((Segment*)pGeo);
        break;
    case gtEnvelope:
        ToMapEnvelope((Envelope*)pGeo);
        break;
    case gtEllipse:
        ToMapEllipse((Ellipse*)pGeo);
        break;
    case gtPolyline:
        ToMapPolyline((Polyline*)pGeo);
        break;
    case gtPolygon:
        ToMapPolygon((Polygon*)pGeo);
        break;
    case gtMultiPolygon:
        ToMapMultiPolygon((MultiPolygon*)pGeo);
        break;
    case gtCollection:
        ToMapCollection((GeometryCollection*)pGeo);
        break;
    }
}
void Display::FromMapPoint(DOUBLE*X,DOUBLE*Y)
{
    if(angle==0)
    {
        *X=(*X-bx)/ax;
        *Y=(*Y-by)/ay;
    }
    else
    {
        double x=(*X-MapCenterX)*cosa+(*Y-MapCenterY)*sina+MapCenterX;
        double y=-(*X-MapCenterX)*sina+(*Y-MapCenterY)*cosa+MapCenterY;
        *X=(x-bx)/ax;
        *Y=(y-by)/ay;
    }
}
void Display::FromMapPoints(DOUBLE*xys,int Count){
    if(angle==0)
    {
        LONG Pos=0;
        for(int k=0;k<Count;k++)
        {
           xys[Pos]=(xys[Pos]-bx)/ax;
           xys[Pos+1]=(xys[Pos+1]-by)/ay;
           Pos+=2;
        }
    }
    else
    {
        LONG Pos=0;
        for(int k=0;k<Count;k++)
        {
           double x=(xys[Pos]-MapCenterX)*cosa+(xys[Pos+1]-MapCenterY)*sina+MapCenterX;
           double y=-(xys[Pos]-MapCenterX)*sina+(xys[Pos+1]-MapCenterY)*cosa+MapCenterY;
           xys[Pos]=(x-bx)/ax;
           xys[Pos+1]=(y-by)/ay;
           Pos+=2;
        }
    }
}
void Display::FromMapPoints(vector<double>&XYs)
{
    LONG Count=XYs.size();
    DOUBLE*xys=XYs.data();
    FromMapPoints(xys,Count);
}

void Display::FromMapPoint(Point*pGeo)
{
    DOUBLE X,Y;
    pGeo->GetCoord(&X,&Y);
    if(angle==0)
    {
        X=(X-bx)/ax;
        Y=(Y-by)/ay;
    }
    else
    {
        double x=(X-MapCenterX)*cosa+(Y-MapCenterY)*sina+MapCenterX;
        double y=-(X-MapCenterX)*sina+(Y-MapCenterY)*cosa+MapCenterY;
        X=(x-bx)/ax;
        Y=(y-by)/ay;
    }
    pGeo->PutCoord(X,Y);
}
void Display::FromMapPoints(Points*pGeo)
{
    DOUBLE*xys=pGeo->GetXYs();
    LONG Count=pGeo->GetSize();
    if(angle==0)
    {
        LONG Pos=0;
        for(int k=0;k<Count;k++)
        {
           xys[Pos]=(xys[Pos]-bx)/ax;
           xys[Pos+1]=(xys[Pos+1]-by)/ay;
           Pos+=2;
        }
    }
    else
    {
        LONG Pos=0;
        for(int k=0;k<Count;k++)
        {
           double x=(xys[Pos]-MapCenterX)*cosa+(xys[Pos+1]-MapCenterY)*sina+MapCenterX;
           double y=-(xys[Pos]-MapCenterX)*sina+(xys[Pos+1]-MapCenterY)*cosa+MapCenterY;
           xys[Pos]=(x-bx)/ax;
           xys[Pos+1]=(y-by)/ay;
           Pos+=2;
        }
    }
    pGeo->ComputeEnvelope();
}
void Display::FromMapSegment(Segment*pGeo)
{
    DOUBLE lnX1,lnY1,lnX2,lnY2;
    pGeo->GetCoord(&lnX1,&lnY1,&lnX2,&lnY2);
    if(angle==0)
    {
        lnX1=(lnX1-bx)/ax;
        lnY1=(lnY1-by)/ay;
        lnX2=(lnX2-bx)/ax;
        lnY2=(lnY2-by)/ay;
    }
    else
    {
        double x=(lnX1-MapCenterX)*cosa+(lnY1-MapCenterY)*sina+MapCenterX;
        double y=-(lnX1-MapCenterX)*sina+(lnY1-MapCenterY)*cosa+MapCenterY;
        lnX1=(x-bx)/ax;
        lnY1=(y-by)/ay;
        x=(lnX2-MapCenterX)*cosa+(lnY2-MapCenterY)*sina+MapCenterX;
        y=-(lnX2-MapCenterX)*sina+(lnY2-MapCenterY)*cosa+MapCenterY;
        lnX2=(x-bx)/ax;
        lnY2=(y-by)/ay;
    }
    pGeo->PutCoord(lnX1,lnY1,lnX2,lnY2);
}
void Display::FromMapEllipse(Ellipse*pGeo)
{
    AutoPtr<Points>pts=pGeo->ReturnPoints(false);
    FromMapPoints(pts.get());
    AutoPtr<Envelope>rect=pts->GetExtent();
    double Width,Height;
    Width=rect->Width();
    Height=rect->Height();
    pGeo->Width()=Width;
    pGeo->Height()=Height;
    AutoPtr<Point>ppt=rect->GetCenterPoint();
    pGeo->SetCenterPoint(ppt.get());
}
void Display::FromMapEnvelope(Envelope*pGeo)
{
    if(angle==0)
    {
        DOUBLE XMin,YMin,XMax,YMax;
        pGeo->GetCoord(&XMin,&YMin,&XMax,&YMax);
        pGeo->PutCoord((XMin-bx)/ax,(YMin-by)/ay,(XMax-bx)/ax,(YMax-by)/ay);
    }
    else
    {
        DOUBLE XMin,YMin,XMax,YMax;
        pGeo->GetCoord(&XMin,&YMin,&XMax,&YMax);
        double xmin,ymin,xmax,ymax;
        DOUBLE x,y;
        x=(XMin-MapCenterX)*cosa+(YMin-MapCenterY)*sina+MapCenterX;
        y=-(XMin-MapCenterX)*sina+(YMin-MapCenterY)*cosa+MapCenterY;
        x=(x-bx)/ax;y=(y-by)/ay;
        xmin=xmax=x;
        ymin=ymax=y;
        x=(XMin-MapCenterX)*cosa+(YMax-MapCenterY)*sina+MapCenterX;
        y=-(XMin-MapCenterX)*sina+(YMax-MapCenterY)*cosa+MapCenterY;
        x=(x-bx)/ax;y=(y-by)/ay;
        if(x<xmin) xmin=x;
        if(x>xmax) xmax=x;
        if(y<ymin) ymin=y;
        if(y>ymax) ymax=y;
        x=(XMax-MapCenterX)*cosa+(YMax-MapCenterY)*sina+MapCenterX;
        y=-(XMax-MapCenterX)*sina+(YMax-MapCenterY)*cosa+MapCenterY;
        x=(x-bx)/ax;y=(y-by)/ay;
        if(x<xmin) xmin=x;
        if(x>xmax) xmax=x;
        if(y<ymin) ymin=y;
        if(y>ymax) ymax=y;
        x=(XMax-MapCenterX)*cosa+(YMin-MapCenterY)*sina+MapCenterX;
        y=-(XMax-MapCenterX)*sina+(YMin-MapCenterY)*cosa+MapCenterY;
        x=(x-bx)/ax;y=(y-by)/ay;
        if(x<xmin) xmin=x;
        if(x>xmax) xmax=x;
        if(y<ymin) ymin=y;
        if(y>ymax) ymax=y;
        pGeo->PutCoord(xmin,ymin,xmax,ymax);
    }
}
void Display::FromMapPolyline(Polyline*pGeo)
{
    LONG Count=pGeo->GetSize();
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Points>path=pGeo->GetItem(k);
        FromMapPoints(path.get());
    }
}
void Display::FromMapPolygon(Polygon*pGeo)
{
    LONG Count=pGeo->GetSize();
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Points>path=pGeo->GetItem(k);
        FromMapPoints(path.get());
    }
}
void Display::FromMapMultiPolygon(MultiPolygon*pGeo)
{
    LONG Count=pGeo->GetSize();
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Polygon>poly=pGeo->GetItem(k);
        FromMapPolygon(poly.get());
    }
}
void Display::FromMapCollection(GeometryCollection*pGeo)
{
    LONG Count=pGeo->GetSize();
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Geometry>poly=pGeo->GetItem(k);
        FromMapGeometry(poly.get());
    }
}
void Display::FromMapGeometry(Geometry*pGeo)
{
    GeometryType type=pGeo->GetType();
    switch(type)
    {
    case gtPoint:
        FromMapPoint((Point*)pGeo);
        break;
    case gtPoints:
        FromMapPoints((Points*)pGeo);
        break;
    case gtSegment:
        FromMapSegment((Segment*)pGeo);
        break;
    case gtEnvelope:
        FromMapEnvelope((Envelope*)pGeo);
        break;
    case gtEllipse:
        FromMapEllipse((Ellipse*)pGeo);
        break;
    case gtPolyline:
        FromMapPolyline((Polyline*)pGeo);
        break;
    case gtPolygon:
        FromMapPolygon((Polygon*)pGeo);
        break;
    case gtMultiPolygon:
        FromMapMultiPolygon((MultiPolygon*)pGeo);
        break;
    case gtCollection:
        FromMapCollection((GeometryCollection*)pGeo);
        break;
    }
}
AutoPtr<Envelope>Display::GetClientExtent()
{
    Envelope*env=new Envelope();
    env->PutCoord(ClientRect.Left,ClientRect.Top,ClientRect.Right,ClientRect.Bottom);
    return env;
}
AutoPtr<Envelope>Display::GetMapExtent()
{
    Envelope*env=new Envelope();
    env->PutCoord(MapRect.Left,MapRect.Top,MapRect.Right,MapRect.Bottom);
    return env;
}

void Display::SetExtent(Envelope*client,Envelope*map)
{
    //将地图范围按比例缩放至客户区范围
    DOUBLE xmin,ymin,xmax,ymax;
    DOUBLE XMin,YMin,XMax,YMax;
    client->GetCoord(&xmin,&ymin,&xmax,&ymax);
    map->GetCoord(&XMin,&YMax,&XMax,&YMin);
    if((ymax!=ymin)&&(YMax!=YMin))
    {
        double ratioMap=fabs((XMax-XMin)/(YMax-YMin));
        double ratioClient=fabs((xmax-xmin)/(ymax-ymin));
        if(ratioMap>=ratioClient)
        {
            //如果地图的长宽比大于客户区的长宽比，则扩展地图的Top和Bottom
            double dify=(fabs((XMax-XMin)/ratioClient)-fabs((YMax-YMin)))/2;
            YMax+=dify;
            YMin-=dify;
        }
        else
        {
            //否则扩展地图的Left和Right
            double difx=(fabs((YMax-YMin)*ratioClient)-fabs((XMax-XMin)))/2;
            XMin-=difx;
            XMax+=difx;
        }
    }
    ClientRect.Left=xmin;
    ClientRect.Top=ymin;
    ClientRect.Right=xmax;
    ClientRect.Bottom=ymax;
    MapRect.Left=XMin;
    MapRect.Top=YMax;
    MapRect.Right=XMax;
    MapRect.Bottom=YMin;
    ComputeTransform();
}
void Display::SetExtentDirectly(Envelope*client,Envelope*map)
{
    //将地图范围按比例缩放至客户区范围
    DOUBLE xmin,ymin,xmax,ymax;
    DOUBLE XMin,YMin,XMax,YMax;
    client->GetCoord(&xmin,&ymin,&xmax,&ymax);
    map->GetCoord(&XMin,&YMax,&XMax,&YMin);
    ClientRect.Left=xmin;
    ClientRect.Top=ymin;
    ClientRect.Right=xmax;
    ClientRect.Bottom=ymax;
    MapRect.Left=XMin;
    MapRect.Top=YMax;
    MapRect.Right=XMax;
    MapRect.Bottom=YMin;
    ComputeTransform();
}
void Display::ZoomInMap(DOUBLE ratio)
{
    double difx=fabs(MapRect.Width()*ratio/2);
    double dify=fabs(MapRect.Height()*ratio/2);
    MapRect.Left+=difx;
    MapRect.Right-=difx;
    MapRect.Top-=dify;
    MapRect.Bottom+=dify;
    ComputeTransform();
}
void Display::ZoomOutMap(DOUBLE ratio)
{
    double difx=fabs(MapRect.Width()*ratio/2);
    double dify=fabs(MapRect.Height()*ratio/2);
    MapRect.Left-=difx;
    MapRect.Right+=difx;
    MapRect.Top+=dify;
    MapRect.Bottom-=dify;
    ComputeTransform();
}
void Display::SizeMap(Envelope*Client)
{
    DOUBLE XMin,YMin,XMax,YMax;
    Client->GetCoord(&XMin,&YMin,&XMax,&YMax);
    Rect2D<int>client(XMin,YMin,XMax,YMax);
    ClientRect=client;
    float ratio1;
    if(client.Height()==0)
        ratio1=0;
    else
        ratio1=(float)client.Width()/client.Height();
    Rect2D<double>NewExtent;
    NewExtent.Top=MapRect.Top;
    NewExtent.Bottom=MapRect.Bottom;
    float dif=fabs(ratio1*NewExtent.Height())-MapRect.Width();
    NewExtent.Left=MapRect.Left-dif/2;
    NewExtent.Right=MapRect.Right+dif/2;
    MapRect=NewExtent;
    ComputeTransform();
}
void Display::PanMap(Point*orginMap,Point*targetMap)
{
    DOUBLE X1,Y1,X2,Y2;
    orginMap->GetCoord(&X1,&Y1);
    targetMap->GetCoord(&X2,&Y2);
    if(angle==0)
    {
        X1=X2-X1;
        Y1=Y2-Y1;
        MapRect.Left+=X1;
        MapRect.Right+=X1;
        MapRect.Top+=Y1;
        MapRect.Bottom+=Y1;
        ComputeTransform();
    }
    else
    {
        X2=X2-(X1-MapCenterX);
        Y2=Y2-(Y1-MapCenterY);
        X1=(X2-MapCenterX)*cosa-(Y2-MapCenterY)*sina;
        Y1=(X2-MapCenterX)*sina+(Y2-MapCenterY)*cosa;
        MapRect.Left+=X1;
        MapRect.Right+=X1;
        MapRect.Top+=Y1;
        MapRect.Bottom+=Y1;
        ComputeTransform();
    }
}
void Display::CenterMap(Point*centerPoint)
{
    Point2D<double>CenterMapPoint;
    double cx,cy;
    centerPoint->GetCoord(&cx,&cy);
    CenterMapPoint.X=cx;
    CenterMapPoint.Y=cy;
    double dx=CenterMapPoint.X-(MapRect.Right+MapRect.Left)/2.0;
    double dy=CenterMapPoint.Y-(MapRect.Bottom+MapRect.Top)/2.0;
    MapRect.Left+=dx;
    MapRect.Right+=dx;
    MapRect.Top+=dy;
    MapRect.Bottom+=dy;
    ComputeTransform();
}
void Display::ZoomToRect(Envelope*extent)
{
    if(angle==0)
    {
        AutoPtr<Envelope>client(new Envelope());
        client->PutCoord(ClientRect.Left,ClientRect.Top,ClientRect.Right,ClientRect.Bottom);
        SetExtent(client.get(),extent);
    }
    else
    {
        DOUBLE XMin,YMin,XMax,YMax;
        extent->GetCoord(&XMin,&YMin,&XMax,&YMax);
        double xmin,ymin,xmax,ymax;
        DOUBLE x,y;
        x=XMin;
        y=YMin;
        x=(x-MapCenterX)*cosa-(y-MapCenterY)*sina+MapCenterX;
        y=(x-MapCenterX)*sina+(y-MapCenterY)*cosa+MapCenterY;
        xmin=xmax=x;
        ymin=ymax=y;
        x=XMin;
        y=YMax;
        x=(x-MapCenterX)*cosa-(y-MapCenterY)*sina+MapCenterX;
        y=(x-MapCenterX)*sina+(y-MapCenterY)*cosa+MapCenterY;
        if(x<xmin) xmin=x;
        if(x>xmax) xmax=x;
        if(y<ymin) ymin=y;
        if(y>ymax) ymax=y;
        x=XMax;
        y=YMax;
        x=(x-MapCenterX)*cosa-(y-MapCenterY)*sina+MapCenterX;
        y=(x-MapCenterX)*sina+(y-MapCenterY)*cosa+MapCenterY;
        if(x<xmin) xmin=x;
        if(x>xmax) xmax=x;
        if(y<ymin) ymin=y;
        if(y>ymax) ymax=y;
        x=XMax;
        y=YMin;
        x=(x-MapCenterX)*cosa-(y-MapCenterY)*sina+MapCenterX;
        y=(x-MapCenterX)*sina+(y-MapCenterY)*cosa+MapCenterY;
        if(x<xmin) xmin=x;
        if(x>xmax) xmax=x;
        if(y<ymin) ymin=y;
        if(y>ymax) ymax=y;
        AutoPtr<Envelope>pEnv(new Envelope());
        if(YMax>YMin)
            pEnv->PutCoord(xmin,ymin,xmax,ymax);
        else
            pEnv->PutCoord(xmin,ymax,xmax,ymin);
        AutoPtr<Envelope>client(new Envelope());
        client->PutCoord(ClientRect.Left,ClientRect.Top,ClientRect.Right,ClientRect.Bottom);
        SetExtent(client.get(),pEnv.get());
    }
}
double Display::GetAngle()
{
    return angle;
}

void Display::SetAngle(double ang){
    angle=ang;
    double a=angle/180*PI;
    cosa=cos(-a);
    sina=sin(-a);
}

bool Display::ProjectFromDisplay(Geometry*geo,SpatialReference*psp){
    if((this->pSpatial==nullptr)||(psp==nullptr)) return false;
    if(this->pSpatial->IsSame(psp)) return true;
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation());
    pTrans->SetFromSpatialReference(this->pSpatial);
    pTrans->SetToSpatialReference(AutoPtr<SpatialReference>(psp,true));
    pTrans->BeginTransform();
    pTrans->TransformGeometry(geo);
    return true;
}
bool Display::ProjectToDisplay(Geometry*geo,SpatialReference*psp){
    if((this->pSpatial==nullptr)||(psp==nullptr)) return false;
    if(this->pSpatial->IsSame(psp)) return true;
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation());
    pTrans->SetFromSpatialReference(AutoPtr<SpatialReference>(psp,true));
    pTrans->SetToSpatialReference(this->pSpatial);
    pTrans->BeginTransform();
    pTrans->TransformGeometry(geo);
    return true;
}

double Display::DistanceToDisplay(Point*point,double dist,SpatialReference*psp){
    if((this->pSpatial==nullptr)||(psp==nullptr)) return dist;
    if(this->pSpatial->IsSame(psp)) return dist;
    AutoPtr<Point>centerPoint=point->Clone();
    AutoPtr<CoordinateTransformation>trans=new CoordinateTransformation();
    trans->SetFromSpatialReference(AutoPtr<SpatialReference>(psp,true));
    trans->SetToSpatialReference(this->pSpatial);
    trans->BeginTransform();
    if(!trans->CanTransform()) return dist;
    double x=centerPoint->X();
    double y=centerPoint->Y();
    centerPoint->PutCoord(x-dist/2.0,y);
    centerPoint->Project(trans.get());
    double x1=centerPoint->X();
    double y1=centerPoint->Y();
    centerPoint->PutCoord(x+dist/2.0,y);
    centerPoint->Project(trans.get());
    double x2=centerPoint->X();
    double y2=centerPoint->Y();
    double xdist=sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
    centerPoint->PutCoord(x,y-dist/2.0);
    centerPoint->Project(trans.get());
    x1=centerPoint->X();
    y1=centerPoint->Y();
    centerPoint->PutCoord(x,y+dist/2.0);
    centerPoint->Project(trans.get());
    x2=centerPoint->X();
    y2=centerPoint->Y();
    double ydist=sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
    return sqrt(xdist*xdist+ydist*ydist);
}


double Display::DistanceFromDisplay(Point*point,double dist,SpatialReference*psp){
    if((this->pSpatial==nullptr)||(psp==nullptr)) return dist;
    if(this->pSpatial->IsSame(psp)) return dist;
    AutoPtr<Point>centerPoint=point->Clone();
    AutoPtr<CoordinateTransformation>trans=new CoordinateTransformation();
    trans->SetFromSpatialReference(this->pSpatial);
    trans->SetToSpatialReference(AutoPtr<SpatialReference>(psp,true));
    trans->BeginTransform();
    if(!trans->CanTransform()) return dist;
    double x=centerPoint->X();
    double y=centerPoint->Y();
    centerPoint->PutCoord(x-dist/2.0,y);
    centerPoint->Project(trans.get());
    double x1=centerPoint->X();
    double y1=centerPoint->Y();
    centerPoint->PutCoord(x+dist/2.0,y);
    centerPoint->Project(trans.get());
    double x2=centerPoint->X();
    double y2=centerPoint->Y();
    double xdist=sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
    centerPoint->PutCoord(x,y-dist/2.0);
    centerPoint->Project(trans.get());
    x1=centerPoint->X();
    y1=centerPoint->Y();
    centerPoint->PutCoord(x,y+dist/2.0);
    centerPoint->Project(trans.get());
    x2=centerPoint->X();
    y2=centerPoint->Y();
    double ydist=sqrt((x1-x2)*(x1-x2)+(y1-y2)*(y1-y2));
    return sqrt(xdist*xdist+ydist*ydist);
}


int Display::ClipFromPoints(Points*points,bool isRing,bool bClient)
{
    DOUBLE*xys=points->GetXYs();
    LONG Count=points->GetSize();
    if(Count<1)
    {
        if(!bClient) FromMapPoints(points);
        return Count;
    }
    double mx,my,tx,ty;
    int x,y;
    int xPos,yPos;
    int Pos=0;
    int rPos=0;
    int formerNeighbor=0;
    double formerX,formerY;
    bool IsFormerInputed=true;
    bool NeedUpdate;
    int difNeighbor;
    for(int k=0;k<Count;k++)
    {
        if(bClient) {
            x = xys[Pos++];
            y = xys[Pos++];
        }
        else{
            if(angle==0){
                x = (xys[Pos++] - bx) / ax;
                y = (xys[Pos++] - by) / ay;
            }
            else {
                mx = xys[Pos++];
                my = xys[Pos++];
                tx= (mx - MapCenterX) * cosa + (my - MapCenterY) * sina + MapCenterX;
                ty = -(mx - MapCenterX) * sina + (my - MapCenterY) * cosa + MapCenterY;
                x = (tx - bx) / ax;
                y =(ty - by) / ay;
            }
        }
        if (k > 0) {
            if ((formerX == x) && (formerY == y)) {
                continue;
            }
        }
        xPos = ((x < ClientRect.Left-10) ? -1 : ((x > ClientRect.Right+10) ? 1 : 0));
        yPos = ((y < ClientRect.Top-10) ? -1 : ((y > ClientRect.Bottom+10) ? 1 : 0));
        int neighborPos = neighborCodes[yPos * 3 + xPos + 4];
        if (neighborPos == 0) {
            if (!IsFormerInputed) {
                xys[rPos++] = formerX;
                xys[rPos++] = formerY;
            }
            xys[rPos++] = x;
            xys[rPos++] = y;
            formerNeighbor = 0;
            IsFormerInputed = true;
            formerX = x;
            formerY = y;
            continue;
        }
        NeedUpdate = (formerNeighbor == 0);
        if (!NeedUpdate) {
            difNeighbor = abs(neighborPos - formerNeighbor) % 9;
            if (difNeighbor > 4) difNeighbor = 8 - difNeighbor;
            NeedUpdate = ((formerNeighbor % 2 == 1) ? (difNeighbor > 2) : (difNeighbor > 1));
        }
        if (!NeedUpdate) {
            formerX = x;
            formerY = y;
            IsFormerInputed = false;
            continue;
        }
        if (!IsFormerInputed) {
            xys[rPos++] = formerX;
            xys[rPos++] = formerY;
        }
        xys[rPos++] = x;
        xys[rPos++] = y;
        formerX = x;
        formerY = y;
        IsFormerInputed = true;
        formerNeighbor = neighborPos;
    }
    if(!IsFormerInputed)
    {
        if((isRing)||(NeedUpdate))
        {
            xys[rPos++]=formerX;
            xys[rPos++]=formerY;
        }
    }
    if(isRing)
    {
        if(rPos<6) rPos=0;
    }
    else
    {
        if(rPos<4) rPos=0;
    }
    points->SetSize(rPos/2);
    if(!bClient) points->ComputeEnvelope();
    return (rPos>>1);
}

bool Display::ClipFromMapGeometry(Geometry*geo){
    GeometryType gType=geo->GetType();
    if(geo->GetType()==gtPoint){
        Point*pt=(Point*)geo;
        FromMapPoint(pt);
        DOUBLE X,Y;
        pt->GetCoord(&X,&Y);
        if(!ClientRect.PtInRect(X,Y)) return false;
        return true;
    }
    else {
        if(gType==gtPolyline){
            AutoPtr<CurvePolyline>curve=AutoPtr<Geometry>(geo,true);
            if(curve!=nullptr)
            {
                FromMapGeometry(geo);
                return true;
            }
            AutoPtr<Polyline>poly=AutoPtr<Geometry>(geo,true);
            for(int k=poly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(poly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,false);
                if(clipedNum==0) poly->Remove(k);
            }
            if(poly->GetSize()==0) return false;
            return true;
        }
        else if(gType==gtPolygon){
            AutoPtr<CurvePolygon>curve=AutoPtr<Geometry>(geo,true);
            if(curve!=nullptr)
            {
                FromMapGeometry(geo);
                return true;
            }
            AutoPtr<Polygon>poly=AutoPtr<Geometry>(geo,true);
            for(int k=poly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(poly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,false);
                if(clipedNum==0) poly->Remove(k);
            }
            if(poly->GetSize()==0) return false;
            return true;
        }
        else if(gType==gtMultiPolygon){
            AutoPtr<MultiPolygon>mpoly=AutoPtr<Geometry>(geo,true);
            for(int k=mpoly->GetSize()-1;k>=0;k--){
                AutoPtr<Polygon>poly(mpoly->GetItem(k));
                for(int p=poly->GetSize()-1;p>=0;p--){
                    AutoPtr<Points>pts(poly->GetItem(p));
                    int clipedNum=ClipFromPoints(pts.get(),false,false);
                    if(clipedNum==0) poly->Remove(p);
                }
                if(poly->GetSize()==0) mpoly->Remove(k);
            }
            if(mpoly->GetSize()==0) return false;
            return true;
        }
        else{
            FromMapGeometry(geo);
            return true;
        }
    }
}

bool Display::ClipFromMapGeometry(Geometry*geo,SpatialReference*psp){
    AutoPtr<CoordinateTransformation>trans(new CoordinateTransformation());
    trans->SetFromSpatialReference(AutoPtr<SpatialReference>(psp,true));
    trans->SetToSpatialReference(this->pSpatial);
    trans->BeginTransform();
    bool CanTrans=trans->CanTransform();
    GeometryType gType=geo->GetType();
    if(geo->GetType()==gtPoint){
        if(CanTrans) geo->Project(trans.get());
        Point*pt=(Point*)geo;
        FromMapPoint(pt);
        DOUBLE X,Y;
        pt->GetCoord(&X,&Y);
        if(!ClientRect.PtInRect(X,Y)) return false;
        return true;
    }
    else {
        AutoPtr<Envelope>env(geo->GetExtent());
        if (CanTrans) env->Project(trans.get());
        FromMapEnvelope(env.get());
        if (!ClientRect.IntersectRectInClient(env.get())) return false;
        if (CanTrans) geo->Project(trans.get());
        if(gType==gtPolyline){
            AutoPtr<CurvePolyline>curve=AutoPtr<Geometry>(geo,true);
            if(curve!=nullptr)
            {
                FromMapGeometry(geo);
                return true;
            }
            AutoPtr<Polyline>poly=AutoPtr<Geometry>(geo,true);
            for(int k=poly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(poly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,false);
                if(clipedNum==0) poly->Remove(k);
            }
            if(poly->GetSize()==0) return false;
            return true;
        }
        else if(gType==gtPolygon){
            AutoPtr<CurvePolygon>curve=AutoPtr<Geometry>(geo,true);
            if(curve!=nullptr)
            {
                FromMapGeometry(geo);
                return true;
            }
            AutoPtr<Polygon>poly=AutoPtr<Geometry>(geo,true);
            for(int k=poly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(poly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,false);
                if(clipedNum==0) poly->Remove(k);
            }
            if(poly->GetSize()==0) return false;
            return true;
        }
        else if(gType==gtMultiPolygon){
            AutoPtr<MultiPolygon>mpoly=AutoPtr<Geometry>(geo,true);
            for(int k=mpoly->GetSize()-1;k>=0;k--){
                AutoPtr<Polygon>poly(mpoly->GetItem(k));
                for(int p=poly->GetSize()-1;p>=0;p--){
                    AutoPtr<Points>pts(poly->GetItem(p));
                    int clipedNum=ClipFromPoints(pts.get(),false,false);
                    if(clipedNum==0) poly->Remove(p);
                }
                if(poly->GetSize()==0) mpoly->Remove(k);
            }
            if(mpoly->GetSize()==0) return false;
            return true;
        }
        else{
            FromMapGeometry(geo);
            return true;
        }
    }
}

bool Display::ClipFromClientGeometry(Geometry*geo){
    GeometryType gType=geo->GetType();
    if(geo->GetType()==gtPoint){

        Point*pt=(Point*)geo;
        DOUBLE X,Y;
        pt->GetCoord(&X,&Y);
        if(!ClientRect.PtInRect(X,Y)) return false;
        return true;
    }
    else {
        if(gType==gtPolyline){
            AutoPtr<CurvePolyline>curve=AutoPtr<Geometry>(geo,true);
            if(curve!=nullptr)
            {
                return true;
            }
            AutoPtr<Polyline>poly=AutoPtr<Geometry>(geo,true);
            for(int k=poly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(poly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,true);
                if(clipedNum==0) poly->Remove(k);
            }
            if(poly->GetSize()==0) return false;
            return true;
        }
        else if(gType==gtPolygon){
            AutoPtr<CurvePolygon>curve=AutoPtr<Geometry>(geo,true);
            if(curve!=nullptr)
            {
                return true;
            }
            AutoPtr<Polygon>poly=AutoPtr<Geometry>(geo,true);
            for(int k=poly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(poly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,true);
                if(clipedNum==0) poly->Remove(k);
            }
            if(poly->GetSize()==0) return false;
            return true;
        }
        else if(gType==gtMultiPolygon){
            AutoPtr<MultiPolygon>mpoly=AutoPtr<Geometry>(geo,true);
            for(int k=mpoly->GetSize()-1;k>=0;k--){
                AutoPtr<Polygon>poly(mpoly->GetItem(k));
                for(int p=poly->GetSize()-1;p>=0;p--){
                    AutoPtr<Points>pts(poly->GetItem(p));
                    int clipedNum=ClipFromPoints(pts.get(),false,true);
                    if(clipedNum==0) poly->Remove(p);
                }
                if(poly->GetSize()==0) mpoly->Remove(k);
            }
            if(mpoly->GetSize()==0) return false;
            return true;
        }
        else{
            return true;
        }
    }
}

AutoPtr<Geometry>Display::CopyFromMapGeometry(Geometry*geo){
    GeometryType gType=geo->GetType();
    if(geo->GetType()==gtPoint){
        Point*pt=(Point*)geo;
        DOUBLE X,Y;
        pt->GetCoord(&X,&Y);
        this->FromMapPoint(&X,&Y);
        if(!ClientRect.PtInRect(X,Y)) return nullptr;
        return new Point(X,Y);
    }
    else {
        AutoPtr<Envelope>env=this->GetMapExtent();
        AutoPtr<Envelope>geoenv=geo->GetExtent();
        if(!env->IntersectsEnvelope(geoenv.get())) return nullptr;
        if(gType==gtPolyline){
            AutoPtr<Polyline>poly=AutoPtr<Geometry>(geo,true);
            AutoPtr<CurvePolyline>curve=AutoPtr<Geometry>(geo,true);
            if(curve!=nullptr)
            {
                AutoPtr<CurvePolyline>newCurve=curve->Clone();
                FromMapGeometry(newCurve.get());
                return newCurve;
            }
            AutoPtr<Polyline>newpoly=poly->Clone();
            for(int k=newpoly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(newpoly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,false);
                if(clipedNum==0) newpoly->Remove(k);
            }
            if(newpoly->GetSize()==0) return nullptr;
            return newpoly;
        }
        else if(gType==gtPolygon){
            AutoPtr<Polygon>poly=AutoPtr<Geometry>(geo,true);
            AutoPtr<CurvePolygon>curve=AutoPtr<Geometry>(geo,true);
            if(curve!=nullptr)
            {
                AutoPtr<CurvePolygon>newCurve=curve->Clone();
                FromMapGeometry(newCurve.get());
                return newCurve;
            }
            AutoPtr<Polygon>newpoly=poly->Clone();
            for(int k=newpoly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(newpoly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,false);
                if(clipedNum==0) newpoly->Remove(k);
            }
            if(newpoly->GetSize()==0) return nullptr;
            return newpoly;
        }
        else if(gType==gtMultiPolygon){
            AutoPtr<MultiPolygon>mpoly=geo->Clone();
            for(int k=mpoly->GetSize()-1;k>=0;k--){
                AutoPtr<Polygon>poly(mpoly->GetItem(k));
                for(int p=poly->GetSize()-1;p>=0;p--){
                    AutoPtr<Points>pts(poly->GetItem(p));
                    int clipedNum=ClipFromPoints(pts.get(),false,false);
                    if(clipedNum==0) poly->Remove(p);
                }
                if(poly->GetSize()==0) mpoly->Remove(k);
            }
            if(mpoly->GetSize()==0) return nullptr;
            return mpoly;
        }
        else{
            AutoPtr<Geometry>mpoly=geo->Clone();
            FromMapGeometry(mpoly.get());
            return mpoly;
        }
    }
}

AutoPtr<Geometry>Display::CopyFromMapGeometry(Geometry*geo,SpatialReference*psp){
    AutoPtr<CoordinateTransformation>trans(new CoordinateTransformation());
    trans->SetFromSpatialReference(AutoPtr<SpatialReference>(psp,true));
    trans->SetToSpatialReference(this->pSpatial);
    trans->BeginTransform();
    bool CanTrans=trans->CanTransform();
    GeometryType gType=geo->GetType();
    if(geo->GetType()==gtPoint){
        Point*pt=(Point*)geo;
        DOUBLE X,Y;
        pt->GetCoord(&X,&Y);
        if(CanTrans) trans->TransformXY(&X,&Y);
        this->FromMapPoint(&X,&Y);
        if(!ClientRect.PtInRect(X,Y)) return nullptr;
        return new Point(X,Y);
    }
    else {
        AutoPtr<Envelope>env(geo->GetExtent());
        if (CanTrans) env->Project(trans.get());
        FromMapEnvelope(env.get());
        if (!ClientRect.IntersectRectInClient(env.get())) return nullptr;
        AutoPtr<Geometry>newgeo=geo->Clone();
        if (CanTrans) newgeo->Project(trans.get());
        if(gType==gtPolyline){
            AutoPtr<CurvePolyline>curve=newgeo;
            if(curve!=nullptr)
            {
                FromMapGeometry(newgeo.get());
                return newgeo;
            }
            AutoPtr<Polyline>poly=newgeo;
            for(int k=poly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(poly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,false);
                if(clipedNum==0) poly->Remove(k);
            }
            if(poly->GetSize()==0) return nullptr;
            return newgeo;
        }
        else if(gType==gtPolygon){
            AutoPtr<CurvePolygon>curve=newgeo;
            if(curve!=nullptr)
            {
                FromMapGeometry(newgeo.get());
                return newgeo;
            }
            AutoPtr<Polygon>poly=newgeo;
            for(int k=poly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(poly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,false);
                if(clipedNum==0) poly->Remove(k);
            }
            if(poly->GetSize()==0) return nullptr;
            return newgeo;
        }
        else if(gType==gtMultiPolygon){
            AutoPtr<MultiPolygon>mpoly=newgeo;
            for(int k=mpoly->GetSize()-1;k>=0;k--){
                AutoPtr<Polygon>poly(mpoly->GetItem(k));
                for(int p=poly->GetSize()-1;p>=0;p--){
                    AutoPtr<Points>pts(poly->GetItem(p));
                    int clipedNum=ClipFromPoints(pts.get(),false,false);
                    if(clipedNum==0) poly->Remove(p);
                }
                if(poly->GetSize()==0) mpoly->Remove(k);
            }
            if(mpoly->GetSize()==0) return nullptr;
            return newgeo;
        }
        else{
            FromMapGeometry(newgeo.get());
            return newgeo;
        }
    }
}

AutoPtr<Geometry>Display::CopyFromClientGeometry(Geometry*geo){
    GeometryType gType=geo->GetType();
    if(geo->GetType()==gtPoint){
        Point*pt=(Point*)geo;
        DOUBLE X,Y;
        pt->GetCoord(&X,&Y);
        if(!ClientRect.PtInRect(X,Y)) return nullptr;
        return new Point(X,Y);
    }
    else {
        AutoPtr<Envelope>env(geo->GetExtent());
        if (!ClientRect.IntersectRectInClient(env.get())) return nullptr;
        AutoPtr<Geometry>newgeo=geo->Clone();
        if(gType==gtPolyline){

            AutoPtr<CurvePolyline>curve=newgeo;
            if(curve!=nullptr)
            {
                return newgeo;
            }
            AutoPtr<Polyline>poly=newgeo;
            for(int k=poly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(poly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,true);
                if(clipedNum==0) poly->Remove(k);
            }
            if(poly->GetSize()==0) return nullptr;
            return poly;
        }
        else if(gType==gtPolygon){
            AutoPtr<CurvePolygon>curve=newgeo;
            if(curve!=nullptr)
            {
                return newgeo;
            }
            AutoPtr<Polygon>poly=newgeo;
            for(int k=poly->GetSize()-1;k>=0;k--){
                AutoPtr<Points>pts(poly->GetItem(k));
                int clipedNum=ClipFromPoints(pts.get(),false,true);
                if(clipedNum==0) poly->Remove(k);
            }
            if(poly->GetSize()==0) return nullptr;
            return newgeo;
        }
        else if(gType==gtMultiPolygon){
            AutoPtr<MultiPolygon>mpoly=newgeo;
            for(int k=mpoly->GetSize()-1;k>=0;k--){
                AutoPtr<Polygon>poly(mpoly->GetItem(k));
                for(int p=poly->GetSize()-1;p>=0;p--){
                    AutoPtr<Points>pts(poly->GetItem(p));
                    int clipedNum=ClipFromPoints(pts.get(),false,true);
                    if(clipedNum==0) poly->Remove(p);
                }
                if(poly->GetSize()==0) mpoly->Remove(k);
            }
            if(mpoly->GetSize()==0) return nullptr;
            return newgeo;
        }
        else{
            return newgeo;
        }
    }
}

float Display::GetCurrentScale(float zoomratio){
    Rect2D<int>Client=ClientRect;
    Rect2D<double>MapExt=MapRect;
    double cWidth,dWidth;
    cWidth=Client.Right-Client.Left;
    dWidth=MapExt.Right-MapExt.Left;
    cWidth=cWidth/zoomratio/Display::GetDefaultDPI()*25.4/1000;
    AutoPtr<SpatialReference> psp=this->GetSpatialReference();
    float metersRatio=1;
    if(psp!=nullptr) {
        bool IsUndefine=psp->IsUndefined();
        if(!IsUndefine){
            bool IsGeoj=psp->IsGeographic();
            if(IsGeoj) {
                double left,top,right,bottom;
                left=MapExt.Left;
                top=MapExt.Top;
                right=MapExt.Right;
                bottom=MapExt.Bottom;
                double centerX,centerY;
                centerX=(left+right)/2;
                centerY=(top+bottom)/2;
                AutoPtr<SpatialReference>pProj=new SpatialReference();
                pProj->ImportFromEPSG(3395);
                AutoPtr<CoordinateTransformation>pTrans=new CoordinateTransformation(psp,pProj);
                bool IsOk=pTrans->BeginTransform();
                if(IsOk){
                    AutoPtr<Point>pt=new Point();
                    double x1,y1,x2,y2;
                    pt->PutCoord(centerX-0.5,centerY);
                    pTrans->TransformPoint(pt.get());
                    x1=pt->X();y1=pt->Y();
                    pt->PutCoord(centerX+0.5,centerY);
                    pTrans->TransformPoint(pt.get());
                    x2=pt->X();y2=pt->Y();
                    metersRatio=sqrt(pow(x1-x2,2.0)+pow(y1-y2,2.0));
                }
            }
        }
    }
    dWidth=dWidth*metersRatio;
    double scale=dWidth/cWidth;
    if(fabs(round(scale)-scale)<=0.00001) return round(scale);
    if(fabs(round(scale*10)-scale*10)<=0.0001) return round(scale*10)/10;
    if(fabs(round(scale*100)-scale*100)<=0.001) return round(scale*100)/100;
    if(fabs(round(scale*1000)-scale*1000)<=0.01) return round(scale*1000)/1000;
    if(fabs(round(scale*10000)-scale*10000)<=0.1) return round(scale*10000)/10000;
    return scale;
}

bool&Display::Valid(){
    return valid;
}
//BufferWritable:
bool Display::Read(BufferReader*br){
    ClientRect.Left=br->Read<int>();
    ClientRect.Top=br->Read<int>();
    ClientRect.Right=br->Read<int>();
    ClientRect.Bottom=br->Read<int>();
    MapRect.Left=br->Read<double>();
    MapRect.Top=br->Read<double>();
    MapRect.Right=br->Read<double>();
    MapRect.Bottom=br->Read<double>();
    angle=br->Read<double>();
    string wkt=br->ReadString();
    AutoPtr<Envelope>client(new Envelope(ClientRect.Left,ClientRect.Top,ClientRect.Right,ClientRect.Bottom));
    AutoPtr<Envelope>map(new Envelope(MapRect.Left,MapRect.Top,MapRect.Right,MapRect.Bottom));
    this->SetExtentDirectly(client.get(),map.get());
    pSpatial->ImportFromWkt(wkt);
    return true;
}
void Display::Write(BufferWriter*bw){
   bw->Write<int>(ClientRect.Left);
   bw->Write<int>(ClientRect.Top);
   bw->Write<int>(ClientRect.Right);
   bw->Write<int>(ClientRect.Bottom);
   bw->Write<double>(MapRect.Left);
   bw->Write<double>(MapRect.Top);
   bw->Write<double>(MapRect.Right);
   bw->Write<double>(MapRect.Bottom);
   bw->Write<double>(angle);
   string wkt=pSpatial->ExportToWkt();
   bw->WriteString(wkt);
}

string Display::GetTypeName(){
    return "Display";
}
//JsonWritable:
bool Display::ReadFromJson(JsonReader*jr,JsonNode node){
    JsonNode cliNode=node["Client"];
    ClientRect.Left=cliNode[0].asInt();
    ClientRect.Top=cliNode[1].asInt();
    ClientRect.Right=cliNode[2].asInt();
    ClientRect.Bottom=cliNode[3].asInt();
    JsonNode mapNode=node["Map"];
    MapRect.Left=mapNode[0].asDouble();
    MapRect.Top=mapNode[1].asDouble();
    MapRect.Right=mapNode[2].asDouble();
    MapRect.Bottom=mapNode[3].asDouble();
    JsonNode angNode=node["Angle"];
    angle=angNode.asDouble();
    JsonNode pspNode=node["SpatialRef"];
    string wkt=pspNode.asString();
    AutoPtr<Envelope>client(new Envelope(ClientRect.Left,ClientRect.Top,ClientRect.Right,ClientRect.Bottom));
    AutoPtr<Envelope>map(new Envelope(MapRect.Left,MapRect.Top,MapRect.Right,MapRect.Bottom));
    this->SetExtentDirectly(client.get(),map.get());
    pSpatial->ImportFromWkt(wkt);
    return true;
}
string Display::ToJson(){
    JsonObjectWriter writer;
    writer.Add<int>("Client",{ClientRect.Left,ClientRect.Top,ClientRect.Right,ClientRect.Bottom});
    writer.Add<double>("Map",{MapRect.Left,MapRect.Top,MapRect.Right,MapRect.Bottom});
    writer.Add<double>("Angle",angle);
    string wkt=pSpatial->ExportToWkt();
    writer.Add<string>("SpatialRef",wkt);
    return writer.GetJsonString();
}

}
