#include "geometryfactory.h"

namespace SGIS{


OGRPoint*GDALGeometryFactory::ConvertPoint(Point*pDef)
{
    OGRPoint*def=(OGRPoint*)OGRGeometryFactory::createGeometry(wkbPoint);
    DOUBLE X,Y;
    pDef->GetCoord(&X,&Y);
    def->setX(X);
    def->setY(Y);
    return def;
}
OGRMultiPoint*GDALGeometryFactory::ConvertPoints(Points*pDef)
{
    OGRMultiPoint*def=(OGRMultiPoint*)OGRGeometryFactory::createGeometry(wkbMultiPoint);
    long Count=pDef->GetSize();
    OGRPoint*newpt;
    DOUBLE X,Y;
    for(long k=0;k<Count;k++)
    {
        pDef->GetItem(k,X,Y);
        newpt=(OGRPoint*)OGRGeometryFactory::createGeometry(wkbPoint);
        newpt->setX(X);
        newpt->setY(Y);
        def->addGeometryDirectly(newpt);
    }
    return def;
}
OGRMultiLineString*GDALGeometryFactory::ConvertPolyline(Polyline*pDef)
{
    AutoPtr<Polyline>poly(pDef,true);
    AutoPtr<CurvePolyline>cpoly(poly);
    if(cpoly!=nullptr)
    {
        AutoPtr<Polyline>newpoly=cpoly->ConvertToDefaultPolyline(false);
        poly=newpoly;
    }
    OGRMultiLineString*def=(OGRMultiLineString*)OGRGeometryFactory::createGeometry(wkbMultiLineString);
    long Count,lCount;
    Count=poly->GetSize();
    for(long k=0;k<Count;k++)
    {
        OGRLineString*part=(OGRLineString*)OGRGeometryFactory::createGeometry(wkbLineString);
        AutoPtr<Points>pts=poly->GetItem(k);
        lCount=pts->GetSize();
        DOUBLE*xys=pts->GetXYs();
        OGRRawPoint*rawPoints=(OGRRawPoint*)xys;
        part->setPoints(lCount,rawPoints);
        def->addGeometryDirectly(part);
    }
    return def;
}
OGRPolygon*GDALGeometryFactory::ConvertPolygon(Polygon*pDef)
{
    AutoPtr<Polygon>poly(pDef,true);
    AutoPtr<CurvePolygon>cpoly(poly);
    if(cpoly!=NULL)
    {
        AutoPtr<Polygon>newpoly=cpoly->ConvertToDefaultPolygon(false);
        poly=newpoly;
    }
    OGRPolygon*def=(OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
    long Count,lCount;
    Count=poly->GetSize();
    for(long k=0;k<Count;k++)
    {
        OGRLinearRing*part=(OGRLinearRing*)OGRGeometryFactory::createGeometry(wkbLinearRing);
        AutoPtr<Points>pts=poly->GetItem(k);
        lCount=pts->GetSize();
        DOUBLE*xys=pts->GetXYs();
        OGRRawPoint*rawPoints=(OGRRawPoint*)xys;
        part->setPoints(lCount,rawPoints);
        def->addRingDirectly(part);
    }
    def->closeRings();
    return def;
}
OGRMultiPolygon*GDALGeometryFactory::ConvertMultiPolygon(MultiPolygon*pDef)
{
    OGRMultiPolygon*def=(OGRMultiPolygon*)OGRGeometryFactory::createGeometry(wkbMultiPolygon);
    long Count=pDef->GetSize();
    OGRGeometry*pGeo;
    for(long k=0;k<Count;k++)
    {
        AutoPtr<Polygon>poly=pDef->GetItem(k);
        pGeo=ConvertPolygon(poly.get());
        def->addGeometryDirectly(pGeo);
    }
    def->closeRings();
    return def;
}
OGRGeometryCollection*GDALGeometryFactory::ConvertGeometryCollection(GeometryCollection*pDef)
{
    OGRGeometryCollection*def=(OGRGeometryCollection*)OGRGeometryFactory::createGeometry(wkbGeometryCollection);
    long Count=pDef->GetSize();
    OGRGeometry*pGeo;
    for(long k=0;k<Count;k++)
    {
        AutoPtr<Geometry>geo=pDef->GetItem(k);
        pGeo=ConvertGeometry(geo.get());
        def->addGeometryDirectly(pGeo);
    }
    return def;
}
OGRGeometry*GDALGeometryFactory::ConvertGeometry(Geometry*pDef)
{
    GeometryType type=pDef->GetType();
    switch(type)
    {
    case gtPoint:
        {
            return ConvertPoint((Point*)pDef);
        }
    case gtPoints:
        {
            return ConvertPoints((Points*)pDef);
        }
    case gtEnvelope:
        {
            AutoPtr<Geometry>geo(pDef,true);
            AutoPtr<Envelope>rt(geo);
            AutoPtr<Points>pts=rt->ReturnPoints();
            AutoPtr<Polygon>pPoly(new Polygon());
            pPoly->Add(pts);
            OGRGeometry*pGeo=ConvertPolygon(pPoly.get());
            return pGeo;
        }
    case gtEllipse:
        {
           AutoPtr<Geometry>geo(pDef,true);
           AutoPtr<Ellipse>rt(geo);
           AutoPtr<Points>pts=rt->ReturnPoints(false);
           AutoPtr<Polygon>pPoly(new Polygon());
           pPoly->Add(pts);
           OGRGeometry*pGeo=ConvertPolygon(pPoly.get());
           return pGeo;
        }
    case gtSegment:
        {
            AutoPtr<Geometry>geo(pDef,true);
            AutoPtr<Segment>seg(geo);
            AutoPtr<Points>pts(new Points);
            DOUBLE lnx1,lny1,lnx2,lny2;
            seg->GetCoord(&lnx1,&lny1,&lnx2,&lny2);
            pts->Add(lnx1,lny1);
            pts->Add(lnx2,lny2);
            AutoPtr<Polyline>pPoly(new Polyline);
            pPoly->Add(pts);
            OGRGeometry*pGeo=ConvertPolyline(pPoly.get());
            return pGeo;
        }
    case gtPolyline:
        {
            AutoPtr<Geometry>geo(pDef,true);
            AutoPtr<Polyline>poly(geo);
            return ConvertPolyline(poly.get());
        }
    case gtPolygon:
        {
            AutoPtr<Geometry>geo(pDef,true);
            AutoPtr<Polygon>poly(geo);
            return ConvertPolygon(poly.get());
        }
    case gtMultiPolygon:
        {
            AutoPtr<Geometry>geo(pDef,true);
            AutoPtr<MultiPolygon>poly(geo);
            return ConvertMultiPolygon(poly.get());
        }
    case gtCollection:
        {
            AutoPtr<Geometry>geo(pDef,true);
            AutoPtr<GeometryCollection>poly(geo);
            return ConvertGeometryCollection(poly.get());
        }
    }
    return NULL;
}
OGRGeometry* GDALGeometryFactory::ConvertPoint(DOUBLE X,DOUBLE Y)
{
    OGRPoint*def=(OGRPoint*)OGRGeometryFactory::createGeometry(wkbPoint);
    def->setX(X);
    def->setY(Y);
    def->setZ(0);
    return def;
}
OGRGeometry*GDALGeometryFactory::ConvertPolyline(DOUBLE*xy,int*counts,LONG partscount)
{
    LONG Count=partscount;
    OGRMultiLineString*def=(OGRMultiLineString*)OGRGeometryFactory::createGeometry(wkbMultiLineString);
    LONG Pos=0;
    BYTE*bVs=(BYTE*)xy;
    for(long k=0;k<Count;k++)
    {
        OGRLineString*part=(OGRLineString*)OGRGeometryFactory::createGeometry(wkbLineString);
        OGRRawPoint*rawPoints=(OGRRawPoint*)(bVs+(Pos<<4));
        part->setPoints(counts[k],rawPoints);
        def->addGeometryDirectly(part);
        Pos+=counts[k];
    }
    return def;
}
OGRGeometry*GDALGeometryFactory::ConvertPolygon(DOUBLE*xy,int*counts,LONG partscount)
{
    OGRPolygon*def=(OGRPolygon*)OGRGeometryFactory::createGeometry(wkbPolygon);
    LONG Count=partscount;
    LONG Pos=0;
    BYTE*bVs=(BYTE*)xy;
    for(long k=0;k<Count;k++)
    {
        OGRLinearRing*part=(OGRLinearRing*)OGRGeometryFactory::createGeometry(wkbLinearRing);
        OGRRawPoint*rawPoints=(OGRRawPoint*)(bVs+(Pos<<4));
        part->setPoints(counts[k],rawPoints);
        def->addRingDirectly(part);
        Pos+=counts[k];
    }
    def->closeRings();
    return def;
}
OGRGeometry*GDALGeometryFactory::ConvertMultiPoint(DOUBLE*xy,LONG pointscount)
{
    OGRMultiPoint*def=(OGRMultiPoint*)OGRGeometryFactory::createGeometry(wkbMultiPoint);
    LONG Count=pointscount;
    OGRPoint*newpt;
    LONG Pos=0;
    for(long k=0;k<Count;k++)
    {
        newpt=(OGRPoint*)OGRGeometryFactory::createGeometry(wkbPoint);
        newpt->setX(xy[Pos]);
        newpt->setY(xy[Pos+1]);
        def->addGeometryDirectly(newpt);
        Pos+=2;
    }
    return def;
}

AutoPtr<Point>GeometryFactory::CreatePoint(OGRPoint*def)
{
    return new Point(def->getX(),def->getY(),def->getZ());
}
AutoPtr<Points>GeometryFactory::CreatePoints(OGRMultiPoint*def)
{
    long Count=def->getNumGeometries();
    Points*pts=new Points();
    pts->SetSize(Count);
    for(long k=0;k<Count;k++)
    {
        OGRPoint*opt=(OGRPoint*)def->getGeometryRef(k);
        pts->SetItem(k,opt->getX(),opt->getY(),opt->getZ());
    }
    return pts;
}
AutoPtr<Polyline>GeometryFactory::CreatePolyline(OGRLineString*def)
{
    long Count=def->getNumPoints();
    Polyline*poly=new Polyline();
    Points*pts=new Points();
    pts->SetSize(Count);
    DOUBLE*pData=pts->GetXYs();
    DOUBLE*zs=pts->GetZs();
    OGRRawPoint*rawPoints=(OGRRawPoint*)pData;
    def->getPoints(rawPoints,zs);
    poly->Add(pts);
    return poly;
}
AutoPtr<Polyline>GeometryFactory::CreatePolyline(OGRMultiLineString*def)
{
    long Count=def->getNumGeometries();
    Polyline*poly=new Polyline();
    OGRLineString*lr;
    long lCount;
    for(long k=0;k<Count;k++)
    {
        lr=(OGRLineString*)def->getGeometryRef(k);
        Points*pts=new Points();
        lCount=lr->getNumPoints();
        pts->SetSize(lCount);
        DOUBLE*pData=pts->GetXYs();
        DOUBLE*zs=pts->GetZs();
        OGRRawPoint*rawPoints=(OGRRawPoint*)pData;
        lr->getPoints(rawPoints,zs);
        poly->Add(pts);
    }
    return poly;
}
AutoPtr<Polygon>GeometryFactory::CreatePolygon(OGRPolygon*def)
{
    Polygon*poly=new Polygon();
    OGRLineString*lr=def->getExteriorRing();
    Points*pts=new Points();
    long lCount=lr->getNumPoints();
    pts->SetSize(lCount);
    DOUBLE*pData=pts->GetXYs();
    DOUBLE*zs=pts->GetZs();
    OGRRawPoint*rawPoints=(OGRRawPoint*)pData;
    lr->getPoints(rawPoints,zs);
    poly->Add(pts);
    long Count=def->getNumInteriorRings();
    for(long k=0;k<Count;k++)
    {
        lr=(OGRLineString*)def->getInteriorRing(k);
        Points*pts=new Points();
        lCount=lr->getNumPoints();
        DOUBLE*pData=pts->GetXYs();
        DOUBLE*zs=pts->GetZs();
        OGRRawPoint*rawPoints=(OGRRawPoint*)pData;
        lr->getPoints(rawPoints,zs);
        poly->Add(pts);
    }
    return poly;
}
AutoPtr<MultiPolygon>GeometryFactory::CreateMultiPolygon(OGRMultiPolygon*def)
{
    long Count=def->getNumGeometries();
    MultiPolygon*poly=new MultiPolygon();
    for(long k=0;k<Count;k++)
    {
        OGRPolygon*po=(OGRPolygon*)def->getGeometryRef(k);
        AutoPtr<Polygon>pts=CreatePolygon(po);
        poly->Add(pts);
    }
    return poly;
}
AutoPtr<GeometryCollection>GeometryFactory::CreateCollection(OGRGeometryCollection*def)
{
    long Count=def->getNumGeometries();
    GeometryCollection*poly=new GeometryCollection();
    for(long k=0;k<Count;k++)
    {
        AutoPtr<Geometry>pts=CreateGeometry(def->getGeometryRef(k));
        poly->Add(pts);
    }
    return poly;
}

AutoPtr<Geometry>GeometryFactory::CreateGeometry(OGRGeometry*pGeo)
{
     OGRwkbGeometryType type=pGeo->getGeometryType();
     switch(type)
     {
     case wkbPoint :
     case wkbPoint25D:
         {
             AutoPtr<Point>ppt=CreatePoint((OGRPoint*)pGeo);
             return ppt;
         }
     case wkbLineString :
     case wkbLineString25D:
         {
             AutoPtr<Polyline>poly=CreatePolyline((OGRLineString*)pGeo);
             return poly;
         }
     case wkbPolygon :
     case wkbPolygon25D:
         {
             AutoPtr<Polygon>poly=CreatePolygon((OGRPolygon*)pGeo);
             return poly;
         }
     case wkbMultiPoint :
     case wkbMultiPoint25D :
         {
             AutoPtr<Points>pts=CreatePoints((OGRMultiPoint*)pGeo);
             return pts;
         }
     case wkbMultiLineString :
     case wkbMultiLineString25D:
         {
             AutoPtr<Polyline>poly=CreatePolyline((OGRMultiLineString*)pGeo);
             return poly;
         }
     case wkbMultiPolygon :
     case wkbMultiPolygon25D:
         {
             AutoPtr<MultiPolygon>poly=CreateMultiPolygon((OGRMultiPolygon*)pGeo);
             return poly;
         }
     case wkbGeometryCollection:
     case wkbGeometryCollection25D:
         {
             AutoPtr<GeometryCollection>poly=CreateCollection((OGRGeometryCollection*)pGeo);
             return poly;
         }
     }
     return nullptr;
}

AutoPtr<Geometry>GeometryFactory::CreatePoint(DOUBLE X,DOUBLE Y)
{
    Point*ppt=new Point();
    ppt->PutCoord(X,Y);
    return ppt;
}

AutoPtr<Geometry>GeometryFactory::CreatePolyline(DOUBLE*xy,int*counts,LONG partscount)
{
    LONG Count=partscount;
    Polyline*poly=new Polyline();
    LONG Pos=0;
    BYTE*bVs=(BYTE*)xy;
    for(long k=0;k<Count;k++)
    {
        Points*pts=new Points();
        DOUBLE*datas=(DOUBLE*)(bVs+(Pos<<4));
        pts->SetSize(counts[k]);
        memcpy(pts->GetXYs(),datas,sizeof(double)*counts[k]);
        poly->Add(pts);
        Pos+=counts[k];
    }
    return poly;
}

AutoPtr<Geometry>GeometryFactory::CreatePolygon(DOUBLE*xy,int*counts,LONG partscount)
{
    LONG Count=partscount;
    Polygon*poly=new Polygon();
    LONG Pos=0;
    BYTE*bVs=(BYTE*)xy;
    for(long k=0;k<Count;k++)
    {
        Points*pts=new Points();
        DOUBLE*datas=(DOUBLE*)(bVs+(Pos<<4));
        pts->SetSize(counts[k]);
        memcpy(pts->GetXYs(),datas,sizeof(double)*counts[k]);
        poly->Add(pts);
        Pos+=counts[k];
    }
    return poly;
}

AutoPtr<Geometry>GeometryFactory::CreateMultiPoint(DOUBLE*xy,LONG pointscount)
{
    Points*pts=new Points();
    pts->SetSize(pointscount);
    memcpy(pts->GetXYs(),xy,sizeof(double)*pointscount);
    return pts;
}

GDALSpatialReference::GDALSpatialReference(){
#ifdef Q_OS_LINUX
   pSpatial.SetAxisMappingStrategy(OSRAxisMappingStrategy::OAMS_TRADITIONAL_GIS_ORDER);
#endif
}
GDALSpatialReference::~GDALSpatialReference(){

}


OGRSpatialReference*GDALSpatialReference::ptr(){
    return &pSpatial;
}

}
