#include "overlaptilegeometrygrid.h"
namespace SGIS{



OverlapTileElement::OverlapTileElement(BYTE subType) {
    this->subType = subType;
    father = NULL;
    child = NULL;
}
OverlapTileElement::~OverlapTileElement() {

}
BYTE OverlapTileElement::GetSubType() {
    return subType;
}

void OverlapTileElement::RegisterCell(OverlapTileElementCell*cell) {
    bool hasAdd=false;
    for(int k=0;k<cells.size();k++){
        if((cells[k]->cellX==cell->cellX)&&(cells[k]->cellY==cell->cellY)){
            hasAdd=true;
            break;
        }
    }
    if(!hasAdd){
        cells.push_back(cell);
        cell->RegisterTileElement(this);
    }
}

int OverlapTileElement::GetCellCount() {
    return cells.size();
}
OverlapTileElementCell*OverlapTileElement::GetCell(int nIndex) {
    return cells[nIndex];
}

OverlapTileElements::OverlapTileElements() {
    rootGeo = nullptr;
    tailGeo = nullptr;
    geoCount = 0;
}

OverlapTileElements::~OverlapTileElements() {
    OverlapTileElement*cur = tailGeo;
    while (cur != NULL)
    {
        OverlapTileElement*temp = cur->father;
        delete cur;
        cur = temp;
    }
}
int OverlapTileElements::GetSize()
{
    return geoCount;
}
void OverlapTileElements::AddElement(OverlapTileElement*geo) {
    if (rootGeo == NULL)
    {
        rootGeo = geo;
        tailGeo = geo;
    }
    else
    {
        tailGeo->child = geo;
        geo->father = tailGeo;
        tailGeo = geo;
    }
    geoCount++;
}
void OverlapTileElements::RemoveElement(OverlapTileElement*geo) {
    if (geo == rootGeo) rootGeo = geo->child;
    if (geo == tailGeo) tailGeo = tailGeo->father;
    if (geo->father != NULL) geo->father->child = geo->child;
    if (geo->child != NULL) geo->child->father = geo->father;
    geo->father = NULL;
    geo->child = NULL;
    delete geo;
    geoCount--;
}
void OverlapTileElements::Clear() {
    OverlapTileElement*cur = tailGeo;
    while (cur != NULL)
    {
        OverlapTileElement*temp = cur->father;
        delete cur;
        cur = temp;
    }
    geoCount = 0;
}

OverlapTileElementCell::OverlapTileElementCell() {

}
OverlapTileElementCell::~OverlapTileElementCell() {

}

void OverlapTileElementCell::RegisterTileElement(OverlapTileElement*el) {
    for (int k = els.size() - 1; k >= 0; k--) {
        if (els[k] == el) return;
    }
    els.push_back(el);
}
void OverlapTileElementCell::RemoveTileElement(int nIndex) {
    els.erase(std::begin(els) + nIndex);
}
void OverlapTileElementCell::RemoveTileElement(OverlapTileElement*geo) {
    for (int k = els.size() - 1; k >= 0; k--) {
        if (els[k] == geo) {
            RemoveTileElement(k);
            return;
        }
    }
}
int OverlapTileElementCell::GetSize() {
    return els.size();
}
OverlapTileElement*OverlapTileElementCell::GetItem(int nIndex) {
    return els[nIndex];
}


OverlapTileElementGrid::OverlapTileElementGrid(int tileWidth, int tileHeight) {
    this->tileWidth = tileWidth;
    this->tileHeight = tileHeight;
    cells = new OverlapTileElementCell[tileWidth*tileHeight];
    int pos = 0;
    for (int i = 0; i < tileHeight; i++) {
        for (int j = 0; j < tileWidth; j++) {
            cells[pos].cellX = j;
            cells[pos].cellY = i;
            pos++;
        }
    }
    tgs = new OverlapTileElements();
}
OverlapTileElementGrid::~OverlapTileElementGrid() {
    delete[]cells;
    delete tgs;
}
int OverlapTileElementGrid::GetSize() {
    return tgs->GetSize();
}

OverlapTileElement*OverlapTileElementGrid::GetRootElement() {
    return tgs->rootGeo;
}

OverlapTileElementCell*OverlapTileElementGrid::GetCell(int tileX, int tileY) {
    return &cells[tileX + tileY*tileWidth];
}

void OverlapTileElementGrid::RemoveElement(OverlapTileElement*dg) {
    int cellCount = dg->GetCellCount();
    for (int j = 0; j < cellCount; j++) {
        OverlapTileElementCell*cell = dg->GetCell(j);
        cell->RemoveTileElement(dg);
    }
    tgs->RemoveElement(dg);
}
bool OverlapTileElementGrid::IsElementCompleted(OverlapTileElement*tileGeo, int tileY) {
    int cellCount = tileGeo->GetCellCount();
    for (int k = 0; k < cellCount; k++) {
        if (tileGeo->cells[k]->cellY >= tileY-1) return false;
    }
    return true;
}
void OverlapTileElementGrid::RegisterElement(OverlapTileElement*dg, int tileX, int tileY) {
    tgs->AddElement(dg);
    dg->RegisterCell(GetCell(tileX, tileY));
    for (int i = -1; i <= 1;i++) {
        int n = tileY+i;
        if ((n < 0) || (n >= tileHeight)) continue;
        for (int j = -1; j <= 1; j++) {
            int m = tileX + j;
            if ((m < 0) || (m >= tileWidth)) continue;
            OverlapTileElementCell*otherCell = GetCell(m, n);
            int geoCount = otherCell->GetSize();
            for (int p = geoCount - 1; p >= 0; p--) {
                OverlapTileElement*otherGeo = otherCell->GetItem(p);
                if (otherGeo == dg) continue;
                CombineElement(dg, otherGeo);
            }
        }
    }
}


OverlapTileEnvelope::OverlapTileEnvelope(AutoPtr<Envelope>env, BYTE subType)
    :OverlapTileElement(subType){
    this->env = env;
}

OverlapTileEnvelope::~OverlapTileEnvelope() {

}

AutoPtr<Envelope>OverlapTileEnvelope::GetEnvelope() {
    return env;
}
void OverlapTileEnvelope::SetEnvelope(AutoPtr<Envelope>env) {
    this->env = env;
}

OverlapTileEnvelopeGrid::OverlapTileEnvelopeGrid(int tileWidth, int tileHeight)
    :OverlapTileElementGrid(tileWidth,tileHeight)
{

}
OverlapTileEnvelopeGrid::~OverlapTileEnvelopeGrid() {

}

bool OverlapTileEnvelopeGrid::CombineElement(OverlapTileElement*tg1, OverlapTileElement*tg2) {
    if (tg1->subType != tg2->subType) return false;
    OverlapTileEnvelope*tileGeo1=(OverlapTileEnvelope*)tg1;
    OverlapTileEnvelope*tileGeo2=(OverlapTileEnvelope*)tg2;
    bool bInter=tileGeo1->env->Intersects(tileGeo2->env.get());
    if (!bInter) return false;
    AutoPtr<Envelope>newGeo=tileGeo1->env->Union(tileGeo2->env.get());
    if (newGeo == nullptr) return false;
    tileGeo1->SetEnvelope(newGeo);
    int cellCount = tileGeo2->GetCellCount();
    for (int j = 0; j < cellCount; j++) {
        OverlapTileElementCell*cell = tileGeo2->GetCell(j);
        tileGeo1->RegisterCell(cell);
    }
    RemoveElement(tileGeo2);
    return true;
}

OverlapTileGeometry::OverlapTileGeometry(AutoPtr<Geometry>geo, BYTE subType)
    :OverlapTileElement(subType){
    this->geo = geo;
}
OverlapTileGeometry::~OverlapTileGeometry() {

}

AutoPtr<Geometry>OverlapTileGeometry::GetGeometry() {
    return geo;
}
void OverlapTileGeometry::SetGeometry(AutoPtr<Geometry>geo) {
    this->geo = geo;
}

OverlapTileGeometryGrid::OverlapTileGeometryGrid(int tileWidth, int tileHeight)
    :OverlapTileElementGrid(tileWidth,tileHeight)
{
    pTopo=new GeometryTopology();
    pOper=new GeometryOperator();
}
OverlapTileGeometryGrid::~OverlapTileGeometryGrid() {

}

bool OverlapTileGeometryGrid::CombineElement(OverlapTileElement*tg1, OverlapTileElement*tg2) {
    if (tg1->subType != tg2->subType) return false;
    OverlapTileGeometry*tileGeo1=(OverlapTileGeometry*)tg1;
    OverlapTileGeometry*tileGeo2=(OverlapTileGeometry*)tg2;
    pTopo->SetGeometry(tileGeo1->geo);
    bool bInter=pTopo->Intersects(tileGeo2->geo.get());
    if (!bInter) return false;
    pOper->SetGeometry(tileGeo1->geo);
    AutoPtr<Geometry>newGeo=pOper->Union(tileGeo2->geo.get());
    if (newGeo == NULL) return false;
    tileGeo1->SetGeometry(newGeo);
    int cellCount = tileGeo2->GetCellCount();
    for (int j = 0; j < cellCount; j++) {
        OverlapTileElementCell*cell = tileGeo2->GetCell(j);
        tileGeo1->RegisterCell(cell);
    }
    RemoveElement(tileGeo2);
    return true;
}

}
