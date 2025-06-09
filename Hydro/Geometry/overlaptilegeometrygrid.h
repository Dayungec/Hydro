#ifndef OVERLAPTILEGEOMETRYGRID_H
#define OVERLAPTILEGEOMETRYGRID_H
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
class OverlapTileElement;
class OverlapTileElementCell;
class OverlapTileElements;

class SGIS_EXPORT OverlapTileElement{
public:
    OverlapTileElement(BYTE subType = 0);
    virtual~OverlapTileElement();
    BYTE GetSubType();
    void RegisterCell(OverlapTileElementCell*cell);
    int GetCellCount();
    OverlapTileElementCell*GetCell(int nIndex);
public:
    OverlapTileElement*father;
    OverlapTileElement*child;
    BYTE subType;
    vector<OverlapTileElementCell*>cells;
};

class SGIS_EXPORT OverlapTileElements {
public:
    OverlapTileElements();
    virtual~OverlapTileElements();
    int GetSize();
    void AddElement(OverlapTileElement*geo);
    void RemoveElement(OverlapTileElement*geo);
    void Clear();
public:
    OverlapTileElement*rootGeo;
    OverlapTileElement*tailGeo;
    int geoCount;
};

class SGIS_EXPORT OverlapTileElementCell
{
public:
    OverlapTileElementCell();
    virtual~OverlapTileElementCell();
    void RegisterTileElement(OverlapTileElement*el);
    void RemoveTileElement(int nIndex);
    void RemoveTileElement(OverlapTileElement*el);
    int GetSize();
    OverlapTileElement*GetItem(int nIndex);
public:
    int cellX;
    int cellY;
protected:
    vector<OverlapTileElement*>els;
};

class SGIS_EXPORT OverlapTileElementGrid
{
public:
    OverlapTileElementGrid(int tileWidth, int tileHeight);
    virtual~OverlapTileElementGrid();
    int GetSize();
    void RegisterElement(OverlapTileElement*dg, int tileX, int tileY);
    void RemoveElement(OverlapTileElement*dg);
    OverlapTileElement*GetRootElement();
    virtual bool CombineElement(OverlapTileElement*tileGeo1, OverlapTileElement*tileGeo2)=0;
    bool IsElementCompleted(OverlapTileElement*tileGeo,int tileY);
protected:
    OverlapTileElementCell* GetCell(int tileX, int tileY);
protected:
    OverlapTileElementCell*cells;
    OverlapTileElements*tgs;
    int tileWidth;
    int tileHeight;
};

class SGIS_EXPORT OverlapTileEnvelope :public OverlapTileElement
{
public:
    OverlapTileEnvelope(AutoPtr<Envelope>env, BYTE subType = 0);
    virtual~OverlapTileEnvelope();
    AutoPtr<Envelope> GetEnvelope();
    void SetEnvelope(AutoPtr<Envelope>geo);
public:
    AutoPtr<Envelope>env;
};

class SGIS_EXPORT OverlapTileEnvelopeGrid :public OverlapTileElementGrid
{
public:
    OverlapTileEnvelopeGrid(int tileWidth, int tileHeight);
    virtual~OverlapTileEnvelopeGrid();
    bool CombineElement(OverlapTileElement*tileGeo1, OverlapTileElement*tileGeo2);
};

class SGIS_EXPORT OverlapTileGeometry :public OverlapTileElement
{
public:
    OverlapTileGeometry(AutoPtr<Geometry>geo, BYTE subType = 0);
    virtual~OverlapTileGeometry();
    AutoPtr<Geometry> GetGeometry();
    void SetGeometry(AutoPtr<Geometry>geo);
public:
    AutoPtr<Geometry>geo;
};

class SGIS_EXPORT OverlapTileGeometryGrid :public OverlapTileElementGrid
{
public:
    OverlapTileGeometryGrid(int tileWidth, int tileHeight);
    virtual~OverlapTileGeometryGrid();
    bool CombineElement(OverlapTileElement*tileGeo1, OverlapTileElement*tileGeo2);
protected:
    AutoPtr<GeometryTopology>pTopo;
    AutoPtr<GeometryOperator>pOper;
};


};
#endif // OVERLAPTILEGEOMETRYGRID_H
