#include "mapprojection.h"
namespace SGIS{

MapProjection::MapProjection()
{

}

MapProjection::~MapProjection()
{

}
double MapProjection::clip(double value,double min,double max)
{
    if(value<min) return min;
    if(value>max) return max;
    return value;
}

Point2D<ULONGLONG>MapProjection::GetTileMatrixSizePixel(int zoom){
    Point2D<ULONGLONG> s = GetTileMatrixSizeXY(zoom);
    return Point2D<ULONGLONG>(s.X * tileSize.X, s.Y * tileSize.Y);
}
Point2D<ULONGLONG>MapProjection::GetTileMatrixSizeXY(int zoom){
   Point2D<ULONGLONG> sMin = GetTileMatrixMinXY(zoom);
   Point2D<ULONGLONG> sMax = GetTileMatrixMaxXY(zoom);
   return Point2D<ULONGLONG>(sMax.X- sMin.X + 1, sMax.Y - sMin.Y + 1);
}
double&MapProjection::DifLon(){
    return difx;
}
double&MapProjection::DifLat(){
    return dify;
}
Point2D<int>MapProjection::GetTileSize(){
    return tileSize;
}

MercatorProjection::MercatorProjection(){
    MinLatitude = -85.05112878;
    MaxLatitude = 85.05112878;
    MinLongitude = -180;
    MaxLongitude = 180;
    tileSize.X=256;
    tileSize.Y=256;
    difx=0;
    dify=0;
}
MercatorProjection::~MercatorProjection(){

}

Point2D<ULONGLONG> MercatorProjection::GetTileMatrixMinXY(int zoom){
    return Point2D<ULONGLONG>(0,0);
}
Point2D<ULONGLONG> MercatorProjection::GetTileMatrixMaxXY(int zoom){
    int xy = (1 << zoom);
    return Point2D<ULONGLONG>(xy - 1, xy - 1);
}

ULONGLONG MercatorProjection::GetTotalTiles(int level){
    int xy = (1 << level);
    return xy*xy;
}

ULONGLONG MercatorProjection::GetTotalXTiles(int level){
    int xy = (1 << level);
    return xy;
}

ULONGLONG MercatorProjection::GetTotalYTiles(int level){
    int xy = (1 << level);
    return xy;
}
double MercatorProjection::GetAxis(){
    return 6378137;
}
double MercatorProjection::GetFlattening(){
    return 1.0 / 298.257223563;
}
void MercatorProjection::FromLatLngToPixel(DOUBLE lng,DOUBLE lat,LONG level,DOUBLE*pixelX,DOUBLE*pixelY){
    lat = clip(lat, MinLatitude, MaxLatitude);
    lng = clip(lng, MinLongitude, MaxLongitude);
    lat+=dify;
    lng+=difx;
    double x = (lng + 180) / 360;
    double sinLatitude = sin(lat * PI / 180);
    double y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * PI);
    Point2D<ULONGLONG> s = GetTileMatrixSizePixel(level);
    ULONGLONG mapSizeX = s.X;
    ULONGLONG mapSizeY = s.Y;
    Point2D<ULONGLONG> ret;
    ret.X = (ULONGLONG)clip(x * mapSizeX + 0.5, 0, mapSizeX - 1);
    ret.Y = (ULONGLONG)clip(y * mapSizeY + 0.5, 0, mapSizeY - 1);
    *pixelX=ret.X;
    *pixelY=ret.Y;
}

void MercatorProjection::FromPixelToLatLng(DOUBLE x,DOUBLE y,LONG level,DOUBLE*lon,DOUBLE*lat){
    Point2D<ULONGLONG> s = GetTileMatrixSizePixel(level);
    double mapSizeX = s.X;
    double mapSizeY = s.Y;
    double xx = (clip(x, 0, mapSizeX - 1) / mapSizeX) - 0.5;
    double yy = 0.5 - (clip(y, 0, mapSizeY - 1) / mapSizeY);
    *lat = 90 - 360 * atan(exp(-yy * 2 * PI)) / PI;
    *lon= 360 * xx;
    *lat=*lat-dify;
    *lon=*lon-difx;
}
void MercatorProjection::FromPixelToTileXY(DOUBLE x,DOUBLE y,LONG*tileX,LONG*tileY){
    *tileX=x/tileSize.X;
    *tileY=y/tileSize.Y;
}
void MercatorProjection::FromTileXYToPixel(LONG x,LONG y,DOUBLE*pixelX,DOUBLE*pixelY){
    *pixelX=x*tileSize.X;
    *pixelY=y*tileSize.Y;
}
AutoPtr<SpatialReference>MercatorProjection::GetSpatialReference(){
    AutoPtr<SpatialReference>psp(new SpatialReference());
    psp->ImportFromWkt("PROJCS[\"WGS_1984_Web_Mercator_Auxiliary_Sphere\",GEOGCS[\"GCS_WGS_1984\",DATUM[\"D_WGS_1984\",SPHEROID[\"WGS_1984\",6378137.0,298.257223563]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Mercator_Auxiliary_Sphere\"],PARAMETER[\"False_Easting\",0.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",0.0],PARAMETER[\"Standard_Parallel_1\",0.0],PARAMETER[\"Auxiliary_Sphere_Type\",0.0],UNIT[\"Meter\",1.0]]");
    return psp;
}

}
