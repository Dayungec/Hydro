#ifndef MAPPROJECTION_H
#define MAPPROJECTION_H
#include "Base/autoptr.h"
#include "Geometry/geometry2d.h"
#include "Geometry/spatialreference.h"

namespace SGIS{

struct MapTile
{
   LONG Level;
   LONG TileX;
   LONG TileY;
   MapTile(){
       Level=-1;
       TileX=0;
       TileY=0;
   };
   MapTile(LONG x,LONG y,LONG level){
       TileX=x;
       TileY=y;
       Level=level;
   };
   MapTile(const MapTile&other){
       Level=other.Level;
       TileX=other.TileX;
       TileY=other.TileY;
   };
   MapTile&operator=(const MapTile&other){
       Level=other.Level;
       TileX=other.TileX;
       TileY=other.TileY;
       return *this;
   };
   bool Equals(const MapTile&other){
       return ((other.Level==Level)&&(other.TileX==TileX)&&(other.TileY==TileY));
   };
};

class SGIS_EXPORT MapProjection
{
public:
    MapProjection();
    virtual~MapProjection();
    double&DifLon();
    double&DifLat();
    Point2D<int>GetTileSize();
    virtual ULONGLONG GetTotalTiles(int level)=0;
    virtual ULONGLONG GetTotalXTiles(int level)=0;
    virtual ULONGLONG GetTotalYTiles(int level)=0;
    virtual double GetAxis()=0;
    virtual double GetFlattening()=0;
    virtual void FromLatLngToPixel(DOUBLE lon,DOUBLE lat,LONG level,DOUBLE*pixelX,DOUBLE*pixelY)=0;
    virtual void FromPixelToLatLng(DOUBLE x,DOUBLE y,LONG level,DOUBLE*lon,DOUBLE*lat)=0;
    virtual void FromPixelToTileXY(DOUBLE x,DOUBLE y,LONG*tileX,LONG*tileY)=0;
    virtual void FromTileXYToPixel(LONG x,LONG y,DOUBLE*pixelX,DOUBLE*pixelY)=0;
    virtual AutoPtr<SpatialReference>GetSpatialReference()=0;
protected:
    double clip(double value,double min,double max);
    virtual Point2D<ULONGLONG> GetTileMatrixMinXY(int zoom)=0;
    virtual Point2D<ULONGLONG> GetTileMatrixMaxXY(int zoom)=0;
    Point2D<ULONGLONG> GetTileMatrixSizePixel(int zoom);
    Point2D<ULONGLONG> GetTileMatrixSizeXY(int zoom);
protected:
    double MinLatitude;
    double MaxLatitude;
    double MinLongitude;
    double MaxLongitude;
    double difx;
    double dify;
    Point2D<int>tileSize;
};

class SGIS_EXPORT MercatorProjection: public MapProjection{
public:
    MercatorProjection();
    virtual~MercatorProjection();
    ULONGLONG GetTotalTiles(int level);
    ULONGLONG GetTotalXTiles(int level);
    ULONGLONG GetTotalYTiles(int level);
    double GetAxis();
    double GetFlattening();
    void FromLatLngToPixel(DOUBLE lon,DOUBLE lat,LONG level,DOUBLE*pixelX,DOUBLE*pixelY);
    void FromPixelToLatLng(DOUBLE x,DOUBLE y,LONG level,DOUBLE*lon,DOUBLE*lat);
    void FromPixelToTileXY(DOUBLE x,DOUBLE y,LONG*tileX,LONG*tileY);
    void FromTileXYToPixel(LONG x,LONG y,DOUBLE*pixelX,DOUBLE*pixelY);
    AutoPtr<SpatialReference>GetSpatialReference();
protected:
    Point2D<ULONGLONG> GetTileMatrixMinXY(int zoom);
    Point2D<ULONGLONG> GetTileMatrixMaxXY(int zoom);
};

};
#endif // MAPPROJECTION_H
