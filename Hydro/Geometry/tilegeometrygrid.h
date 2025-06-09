#ifndef TILEGEOMETRYGRID_H
#define TILEGEOMETRYGRID_H
#include "Base/base_globle.h"
#include "geometry.h"
#include "Point.h"
#include "points.h"
#include "envelope.h"
#include "polyline.h"
#include "polygon.h"
#include "multipolygon.h"
#include "geometrytopology.h"
#include "geometryoperator.h"

namespace SGIS{

class TileCorner;
class TileGeometry;
class TileGeometryCell;
class TileGeometryGrid;

class TileCorner {
public:
    TileCorner(int cellX, int cellY, BYTE borderCode);
    virtual~TileCorner();
    int GetCellX();
    int GetCellY();
    int GetBorderCode();
    int GetNeighborCellX();
    int GetNeighborCellY();
    BYTE GetNeighborCode();
    int GetLineNeighborCellX();
    int GetLineNeighborCellY();
    BYTE GetLineNeighborCode();
    int GetCount() ;
    bool IsSame(int cellX, int cellY, BYTE borderCode);
    Point2D<double>GetCornerPoint(int nIndex);
    void AddCorner(DOUBLE x, double y);
    void RemoveCorner(int nIndex) ;
    void CombineCorner(TileCorner*other);
    void LabelJoined();
    bool GetHasJoined();
protected:
    vector<Point2D<double>>intersectpts;
    LONG cellX;
    LONG cellY;
    BYTE borderCode;
    bool hasJoined;
};

class SGIS_EXPORT TileGeometry
{
public:
    TileGeometry(AutoPtr<Geometry>geo, USHORT subType = 0);
    virtual~TileGeometry();
    USHORT GetSubType();
    AutoPtr<Geometry> GetGeometry();
    void SetGeometry(AutoPtr<Geometry>geo);
    bool IsCompleted(int tileY);
    int GetCornerCount();
    int GetUnjoinedCornerCount();
    TileCorner*GetCorner(int nIndex);
    TileCorner*GetCorner(int cellX, int cellY, BYTE borderCode);
    void AddCorner(TileCorner*corner);
    void AddCorner(int cellX, int cellY, BYTE borderCode,double x,double y);
    void RemoveCorner(TileCorner*corner);
    void RemoveCorner(int cellX, int cellY, BYTE borderCode);
    void CombineCorner(TileGeometry*other);
    void RegisterCell(TileGeometryCell*cell);
    int GetCellCount();
    TileGeometryCell*GetCell(int nIndex);
protected:
    void CombineCorner(TileGeometry*other,int cornerIndex);
public:
    TileGeometry*father;
    TileGeometry*child;
protected:
    AutoPtr<Geometry>geo;
    USHORT subType;
    vector<TileCorner*>corners;
    vector<TileGeometryCell*>cells;
};

class TileGeometies {
public:
    TileGeometies();
    virtual~TileGeometies();
    int GetSize();
    void AddGeometry(TileGeometry*geo);
    void RemoveGeometry(TileGeometry*geo);
    void Clear();
public:
    TileGeometry*rootGeo;
    TileGeometry*tailGeo;
    int geoCount;
};

class TileGeometryCell
{
public:
    TileGeometryCell();
    virtual~TileGeometryCell();
    void RegisterTileGeometry(TileGeometry*el);
    void RemoveTileGeometry(int nIndex);
    void RemoveTileGeometry(TileGeometry*geo);
    int GetSize();
    TileGeometry*GetItem(int nIndex);
protected:
    vector<TileGeometry*>els;
};

class SGIS_EXPORT TileGeometryGrid
{
public:
    TileGeometryGrid(int tileWidth, int tileHeight, double left, double top, double tileMapWidth, double tileMapHeight);
    virtual~TileGeometryGrid();
    int GetSize();
    void GetTileExtent(int tileX, int tileY, double&l, double&t, double&r, double&b);
    void RegisterGeometry(TileGeometry*dg, int tileX, int tileY);
    void RemoveGeometry(TileGeometry*dg);
    TileGeometry*GetRootGeometry();
    double GetLeft();
    double GetTop();
    double GetTileMapWidth();
    double GetTileMapHeight();
protected:
    void RegisterRing(TileGeometry*dg, Points*dpts, int tileX, int tileY);
    void RegisterPath(TileGeometry*dg, Points*dpts, int tileX, int tileY);
    void RegisterPolygon(TileGeometry*dg, int tileX, int tileY);
    void RegisterPolyline(TileGeometry*dg, int tileX, int tileY);

    int GetCornerFromNode(TileGeometry*tg, TileCorner*corner, int intersectIndex);
    int GetNeigborCornerFromNode(TileGeometry*tg, TileCorner*corner,int intersectIndex);
    void SplitRing(TileGeometry*tg, TileCorner*corner);
    bool UnionPolygon(TileGeometry*tg, TileCorner*corner,TileGeometry*other);
    bool UnionPolygon(TileGeometry*tg, TileCorner*corner);

    bool CaptureNode(Polyline*poly, double X, double Y,int&partIndex,int&nodeIndex);
    bool UnionPolyline(TileGeometry*tg, TileCorner*corner, TileGeometry*other);
protected:
    TileGeometryCell*cells;
    TileGeometies*tgs;
    int tileWidth;
    int tileHeight;
    double tileMapWidth;
    double tileMapHeight;
    double left;
    double top;
    double right;
    double bottom;
    double polyToler;
    double lineToler;
};

};
#endif // TILEGEOMETRYGRID_H
