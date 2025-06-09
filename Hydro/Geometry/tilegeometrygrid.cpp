#include "tilegeometrygrid.h"
namespace SGIS{

TileCorner::TileCorner(int cellX, int cellY, BYTE borderCode) {
    this->cellX = cellX;
    this->cellY = cellY;
    this->borderCode = borderCode;
    hasJoined=false;
}
TileCorner::~TileCorner() {

}
int TileCorner::GetCellX() {
    return cellX;
}
int TileCorner::GetCellY() {
    return cellY;
}
int TileCorner::GetBorderCode() {
    return borderCode;
}
int TileCorner::GetNeighborCellX() {
    if (borderCode == 0)
        return cellX - 1;
    else if(borderCode == 2)
        return cellX + 1;
    return cellX;
}
int TileCorner::GetNeighborCellY() {
    if (borderCode == 1)
        return cellY - 1;
    else if (borderCode == 3)
        return cellY + 1;
    return cellY;
}
BYTE TileCorner::GetNeighborCode() {
    BYTE bc = borderCode + 2;
    if (bc > 3) bc -= 4;
    return bc;
}

int TileCorner::GetLineNeighborCellX() {
    return cellX +aroundx8[borderCode];
}
int TileCorner::GetLineNeighborCellY() {
    return cellY + aroundy8[borderCode];
}
BYTE TileCorner::GetLineNeighborCode() {
    BYTE bc = borderCode + 4;
    if (bc >= 8) bc -= 8;
    return bc;
}
int TileCorner::GetCount() {
    return intersectpts.size();
}
bool TileCorner::IsSame(int cellX, int cellY, BYTE borderCode) {
    return ((this->cellX == cellX) && (this->cellY == cellY) && (this->borderCode == borderCode));
}
Point2D<double>TileCorner::GetCornerPoint(int nIndex) {
    return intersectpts[nIndex];
};
void TileCorner::AddCorner(DOUBLE x, double y) {
    intersectpts.push_back(Point2D<double>(x, y));
};
void TileCorner::RemoveCorner(int nIndex) {
    intersectpts.erase(std::begin(intersectpts) + nIndex);
};
void TileCorner::CombineCorner(TileCorner*other) {
    if ((other->cellX != cellX) || (other->cellY != cellY) || (other->borderCode != borderCode)) return;
    if (borderCode == 0) {
        for (int p = other->intersectpts.size() - 1; p >= 0; p--) {
            Point2D<double>pt = other->intersectpts[p];
            bool hasInserted = false;
            int count = intersectpts.size();
            for (int k = 0; k < count; k++) {
                if (pt.Y < intersectpts[k].Y) {
                    intersectpts.insert(std::begin(intersectpts) + k, pt);
                    hasInserted = true;
                    break;
                }
            }
            if (!hasInserted) {
                intersectpts.push_back(pt);
            }
        }
    }
    else if (borderCode == 1) {
        for (int p = other->intersectpts.size() - 1; p >= 0; p--) {
            Point2D<double>pt = other->intersectpts[p];
            bool hasInserted = false;
            int count = intersectpts.size();
            for (int k = 0; k < count; k++) {
                if (pt.X < intersectpts[k].X) {
                    intersectpts.insert(std::begin(intersectpts) + k, pt);
                    hasInserted = true;
                    break;
                }
            }
            if (!hasInserted) {
                intersectpts.push_back(pt);
            }
        }
    }
    else if (borderCode == 2) {
        for (int p = other->intersectpts.size() - 1; p >= 0; p--) {
            Point2D<double>pt = other->intersectpts[p];
            bool hasInserted = false;
            int count = intersectpts.size();
            for (int k = 0; k < count; k++) {
                if (pt.Y > intersectpts[k].Y) {
                    intersectpts.insert(std::begin(intersectpts) + k, pt);
                    hasInserted = true;
                    break;
                }
            }
            if (!hasInserted) {
                intersectpts.push_back(pt);
            }
        }
    }
    else if (borderCode == 3) {
        for (int p = other->intersectpts.size() - 1; p >= 0; p--) {
            Point2D<double>pt = other->intersectpts[p];
            bool hasInserted = false;
            int count = intersectpts.size();
            for (int k = 0; k < count; k++) {
                if (pt.X > intersectpts[k].X) {
                    intersectpts.insert(std::begin(intersectpts) + k, pt);
                    hasInserted = true;
                    break;
                }
            }
            if (!hasInserted) {
                intersectpts.push_back(pt);
            }
        }
    }
}

void TileCorner::LabelJoined(){
    hasJoined=true;
}

bool TileCorner::GetHasJoined(){
    return hasJoined;
}

TileGeometry::TileGeometry(AutoPtr<Geometry>geo, USHORT subType) {
    this->geo = geo;
    this->subType = subType;
    father = nullptr;
    child = nullptr;
}
TileGeometry::~TileGeometry() {
    for (int k = corners.size() - 1; k >= 0; k--) delete corners[k];
}
USHORT TileGeometry::GetSubType() {
    return subType;
}
AutoPtr<Geometry>TileGeometry::GetGeometry() {
    return geo;
};
void TileGeometry::SetGeometry(AutoPtr<Geometry>geo) {
    this->geo = geo;
}
bool TileGeometry::IsCompleted(int tileY) {
    AutoPtr<Point>pt(geo);
    if(pt!=nullptr) return true;
    AutoPtr<Polyline>ln(geo);
    if (ln == nullptr) {
        for (int k = corners.size() - 1; k >= 0; k--) {
            TileCorner*corner = corners[k];
            if (corner->GetCellY() >= tileY) return false;
            if ((corner->GetCellY() == tileY-1)&&(corner->GetBorderCode() == 3)) return false;
        }
    }
    else {
        for (int k = corners.size() - 1; k >= 0; k--) {
            TileCorner*corner = corners[k];
            if (corner->GetCellY() >= tileY) return false;
            if ((corner->GetCellY() == tileY-1) && (corner->GetBorderCode() >= 4) && (corner->GetBorderCode() <= 6)) return false;
        }
    }
    return true;
}
int TileGeometry::GetCornerCount() {
    return corners.size();
}
int TileGeometry::GetUnjoinedCornerCount(){
    int unjoined=0;
    for(int k=corners.size()-1;k>=0;k--){
        if(!corners[k]->GetHasJoined()){
            unjoined++;
        }
    }
    return unjoined;
}
TileCorner*TileGeometry::GetCorner(int nIndex) {
    return corners[nIndex];
}
TileCorner*TileGeometry::GetCorner(int cellX, int cellY, BYTE borderCode) {
    for (int k = corners.size() - 1; k >= 0; k--) {
        TileCorner*corner = corners[k];
        if (corner->IsSame(cellX, cellY, borderCode)) return corner;
    }
    return NULL;
}
void TileGeometry::AddCorner(TileCorner*corner) {
    corners.push_back(corner);
}
void TileGeometry::AddCorner(int cellX, int cellY, BYTE borderCode,double x,double y){
    TileCorner*corner = GetCorner(cellX, cellY, borderCode);
    if (corner != NULL) {
        corner->AddCorner(x, y);
    }
    else {
        corner = new TileCorner(cellX, cellY, borderCode);
        corner->AddCorner(x, y);
        corners.push_back(corner);
    }
}
void TileGeometry::RemoveCorner(TileCorner*corner) {
    for (int k = corners.size() - 1; k >= 0; k--) {
        if (corners[k] == corner) {
            delete corners[k];
            corners.erase(std::begin(corners) + k);
            return;
        }
    }
}
void TileGeometry::RemoveCorner(int cellX, int cellY, BYTE borderCode) {
    for (int k = corners.size() - 1; k >= 0; k--) {
        TileCorner*corner = corners[k];
        if (corner->IsSame(cellX, cellY, borderCode)) {
            corners.erase(std::begin(corners) + k);
            return;
        }
    }
}

void TileGeometry::RegisterCell(TileGeometryCell*cell) {
    cells.push_back(cell);
}
int TileGeometry::GetCellCount() {
    return cells.size();
}
TileGeometryCell*TileGeometry::GetCell(int nIndex) {
    return cells[nIndex];
}

void TileGeometry::CombineCorner(TileGeometry*other, int cornerIndex) {
    TileCorner*corner = other->GetCorner(cornerIndex);
    for (int k = corners.size() - 1; k >= 0; k--) {
        TileCorner*cn = corners[k];
        int borderCode = corner->GetBorderCode();
        if (cn->IsSame(corner->GetCellX(), corner->GetCellY(), borderCode)) {
            //将corner所有顶点按顺序插入到cn中
            cn->CombineCorner(corner);
            delete other->corners[cornerIndex];
            other->corners.erase(std::begin(other->corners) + cornerIndex);
            return;
        }
    }
    corners.push_back(corner);
    other->corners.erase(std::begin(other->corners) + cornerIndex);
}
void TileGeometry::CombineCorner(TileGeometry*other) {
    for (int k = other->corners.size()-1; k >=0 ; k--) {
        CombineCorner(other, k);
    }
    for (int k = 0; k < other->cells.size(); k++) {
        TileGeometryCell*cell = other->cells[k];
        bool hasFind = false;
        for (int p = cells.size() - 1; p >= 0; p--) {
            if (cells[p] == cell) {
                hasFind = true;
                break;
            }
        }
        if(!hasFind) cells.push_back(other->cells[k]);
        other->cells[k]->RegisterTileGeometry(this);
    }
}

TileGeometies::TileGeometies() {
    rootGeo = NULL;
    tailGeo = NULL;
    geoCount = 0;
}
TileGeometies::~TileGeometies() {
    TileGeometry*cur = tailGeo;
    while (cur != NULL)
    {
        TileGeometry*temp = cur->father;
        delete cur;
        cur = temp;
    }
}
int TileGeometies::GetSize()
{
    return geoCount;
}
void TileGeometies::AddGeometry(TileGeometry*geo) {
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
void TileGeometies::RemoveGeometry(TileGeometry*geo) {
    if (geo == rootGeo) rootGeo = geo->child;
    if (geo == tailGeo) tailGeo = tailGeo->father;
    if (geo->father != NULL) geo->father->child = geo->child;
    if (geo->child != NULL) geo->child->father = geo->father;
    geo->father = NULL;
    geo->child = NULL;
    delete geo;
    geoCount--;
}
void TileGeometies::Clear() {
    TileGeometry*cur = tailGeo;
    while (cur != NULL)
    {
        TileGeometry*temp = cur->father;
        delete cur;
        cur = temp;
    }
    geoCount = 0;
}

TileGeometryCell::TileGeometryCell() {

}
TileGeometryCell::~TileGeometryCell() {

}
void TileGeometryCell::RegisterTileGeometry(TileGeometry*el) {
    for (int k = els.size() - 1; k >= 0; k--) {
        if (els[k] == el) return;
    }
    els.push_back(el);
}
void TileGeometryCell::RemoveTileGeometry(int nIndex) {
    els.erase(std::begin(els) + nIndex);
}
void TileGeometryCell::RemoveTileGeometry(TileGeometry*geo) {
    for (int k = els.size() - 1; k >= 0; k--) {
        if (els[k] == geo) {
            RemoveTileGeometry(k);
            return;
        }
    }
}
int TileGeometryCell::GetSize() {
    return els.size();
}
TileGeometry*TileGeometryCell::GetItem(int nIndex) {
    return els[nIndex];
}

TileGeometryGrid::TileGeometryGrid(int tileWidth, int tileHeight, double left,double top,double tileMapWidth, double tileMapHeight) {
    this->tileWidth = tileWidth;
    this->tileHeight = tileHeight;
    this->tileMapWidth=tileMapHeight;
    this->tileMapHeight=tileMapHeight;
    this->left = left;
    this->top = top;
    this->right = this->left + this->tileWidth*tileMapWidth;
    this->bottom = this->top - this->tileHeight*tileMapHeight;
    cells = new TileGeometryCell[tileWidth*tileHeight];
    tgs = new TileGeometies();
    polyToler = tileMapWidth/256.0/100.0;
    lineToler = tileMapWidth / 25;
}
TileGeometryGrid::~TileGeometryGrid() {
    delete[]cells;
    delete tgs;
}
int TileGeometryGrid::GetSize() {
    return tgs->GetSize();
}

TileGeometry*TileGeometryGrid::GetRootGeometry() {
    return tgs->rootGeo;
}

double TileGeometryGrid::GetLeft(){
    return left;
}
double TileGeometryGrid::GetTop(){
    return top;
}
double TileGeometryGrid::GetTileMapWidth() {
    return tileMapWidth;
}
double TileGeometryGrid::GetTileMapHeight() {
    return tileMapHeight;
}


void TileGeometryGrid::GetTileExtent(int tileX, int tileY, double&l, double&t, double&r, double&b) {
    l = left + tileMapWidth * tileX;
    t = top - tileMapHeight * tileY;
    r = l + tileMapWidth;
    b = t - tileMapHeight;
}

void TileGeometryGrid::RegisterRing(TileGeometry*dg, Points*dpts, int tileX, int tileY) {
    LONG ptsCount=dpts->GetSize();
    DOUBLE*dataPin=dpts->GetXYs();
    DOUBLE X, Y;
    double l, t, r, b;
    GetTileExtent(tileX, tileY, l, t, r, b);
    int nIndex = 0;
    DOUBLE X0, Y0, X2, Y2;
    X0= dataPin[0];
    Y0 = dataPin[1];
    X2= dataPin[ptsCount*2-2];
    Y2= dataPin[ptsCount * 2 - 1];
    int total = ptsCount;
    if ((X0 == X2) && (Y0 == Y2)) total = ptsCount - 1;
    if (tileX > 0) {
        int firstPos = -1;
        double miny = 0;
        nIndex = 0;
        for (int k = 0; k < total; k++) {
            X = dataPin[nIndex];
            Y = dataPin[nIndex + 1];
            if (fabs(X - l) <= polyToler) {
                if (firstPos == -1) {
                    miny = Y;
                    firstPos = k;
                }
                else if (Y < miny) {
                    miny = Y;
                    firstPos = k;
                }
            }
            nIndex += 2;
        }
        int toPos = firstPos + total;
        int state = 0;
        double formerX, formerY;
        int validCount = 0;
        for (int k = firstPos; k < toPos; k++) {
            int pos = k;
            if (pos >= total) pos -= total;
            X = dataPin[pos<<1];
            Y = dataPin[(pos<<1) + 1];
            if (fabs(X - l) <= polyToler) {
                if (state == 0) {
                    formerX = X;
                    formerY = Y;
                    state = 1;
                    validCount++;
                }
                else
                    validCount++;
            }
            else {
                if (state == 1) {
                    if (validCount > 1) {
                        dg->AddCorner(tileX, tileY, 0, formerX, formerY);
                        int nPos = pos - 1;
                        if (nPos < 0) nPos += total;
                        nPos *= 2;
                        dg->AddCorner(tileX, tileY, 0, dataPin[nPos], dataPin[nPos+1]);
                    }
                }
                validCount = 0;
                state = 0;
            }
        }
    }
    if (tileY > 0) {
        int firstPos = -1;
        double minx = 0;
        nIndex = 0;
        for (int k = 0; k < total; k++) {
            X = dataPin[nIndex];
            Y = dataPin[nIndex + 1];
            if (fabs(t-Y) <= polyToler) {
                if (firstPos == -1) {
                    minx = X;
                    firstPos = k;
                }
                else if (X < minx) {
                    minx = X;
                    firstPos = k;
                }
            }
            nIndex += 2;
        }
        int toPos = firstPos + total;
        int state = 0;
        double formerX, formerY;
        int validCount = 0;
        for (int k = firstPos; k < toPos; k++) {
            int pos = k;
            if (pos >= total) pos -= total;
            X = dataPin[pos << 1];
            Y = dataPin[(pos << 1) + 1];
            if (fabs(t - Y) <= polyToler) {
                if (state == 0) {
                    formerX = X;
                    formerY = Y;
                    state = 1;
                    validCount++;
                }
                else
                    validCount++;
            }
            else {
                if (state == 1) {
                    if (validCount > 1) {
                        dg->AddCorner(tileX, tileY, 1, formerX, formerY);
                        int nPos = pos - 1;
                        if (nPos < 0) nPos += total;
                        nPos *= 2;
                        dg->AddCorner(tileX, tileY, 1, dataPin[nPos], dataPin[nPos + 1]);
                    }
                }
                validCount = 0;
                state = 0;
            }
        }
    }
    if (tileX < tileWidth-1) {
        int firstPos = -1;
        double maxy = 0;
        nIndex = 0;
        for (int k = 0; k < total; k++) {
            X = dataPin[nIndex];
            Y = dataPin[nIndex + 1];
            if (fabs(r - X) <= polyToler) {
                if (firstPos == -1) {
                    maxy = Y;
                    firstPos = k;
                }
                else if (Y > maxy) {
                    maxy = Y;
                    firstPos = k;
                }
            }
            nIndex += 2;
        }
        int toPos = firstPos + total;
        int state = 0;
        double formerX, formerY;
        int validCount = 0;
        for (int k = firstPos; k < toPos; k++) {
            int pos = k;
            if (pos >= total) pos -= total;
            X = dataPin[pos << 1];
            Y = dataPin[(pos << 1) + 1];
            if (fabs(r - X) <= polyToler) {
                if (state == 0) {
                    formerX = X;
                    formerY = Y;
                    state = 1;
                    validCount++;
                }
                else
                    validCount++;
            }
            else {
                if (state == 1) {
                    if (validCount > 1) {
                        dg->AddCorner(tileX, tileY, 2, formerX, formerY);
                        int nPos = pos - 1;
                        if (nPos < 0) nPos += total;
                        nPos *= 2;
                        dg->AddCorner(tileX, tileY, 2, dataPin[nPos], dataPin[nPos + 1]);
                    }
                }
                validCount = 0;
                state = 0;
            }
        }
    }
    if (tileY < tileHeight-1) {
        int firstPos = -1;
        double maxx = 0;
        nIndex = 0;
        for (int k = 0; k < total; k++) {
            X = dataPin[nIndex];
            Y = dataPin[nIndex + 1];
            if (fabs(b - Y) <= polyToler) {
                if (firstPos == -1) {
                    maxx = X;
                    firstPos = k;
                }
                else if (X > maxx) {
                    maxx = X;
                    firstPos = k;
                }
            }
            nIndex += 2;
        }
        int toPos = firstPos + total;
        int state = 0;
        double formerX, formerY;
        int validCount = 0;
        for (int k = firstPos; k < toPos; k++) {
            int pos = k;
            if (pos >= total) pos -= total;
            X = dataPin[pos << 1];
            Y = dataPin[(pos << 1) + 1];
            if (fabs(b - Y) <= polyToler) {
                if (state == 0) {
                    formerX = X;
                    formerY = Y;
                    state = 1;
                    validCount++;
                }
                else
                    validCount++;
            }
            else {
                if (state == 1) {
                    if (validCount > 1) {
                        dg->AddCorner(tileX, tileY, 3, formerX, formerY);
                        int nPos = pos - 1;
                        if (nPos < 0) nPos += total;
                        nPos *= 2;
                        dg->AddCorner(tileX, tileY, 3, dataPin[nPos], dataPin[nPos + 1]);
                    }
                }
                validCount = 0;
                state = 0;
            }
        }
    }
}
void TileGeometryGrid::RegisterPath(TileGeometry*dg, Points*dpts, int tileX, int tileY) {
    LONG ptsCount=dpts->GetSize();
    DOUBLE*dataPin=dpts->GetXYs();
    DOUBLE X1, Y1, X2, Y2;
    X1 = dataPin[0];
    Y1 = dataPin[1];
    X2 = dataPin[ptsCount*2 - 2];
    Y2 = dataPin[ptsCount*2 - 1];
    double l, t, r, b;
    GetTileExtent(tileX, tileY, l, t, r, b);
    if ((tileX > 0) && (tileY > 0)) {
        double dist = sqrt((X1 - l)*(X1 - l) + (Y1 - t)*(Y1 - t));
        if(dist< lineToler) dg->AddCorner(tileX, tileY, 0, X1, Y1);
    }
    if (tileY > 0) {
        double dist = t - Y1;
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 1, X1, Y1);
    }
    if ((tileX < tileWidth-1) && (tileY > tileHeight-1)) {
        double dist = sqrt((X1 - r)*(X1 - r) + (Y1 - t)*(Y1 - t));
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 2, X1, Y1);
    }
    if (tileX < tileWidth-1) {
        double dist = r-X1;
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 3, X1, Y1);
    }
    if ((tileX < tileWidth-1) && (tileY < tileHeight-1)) {
        double dist = sqrt((X1 - r)*(X1 - r) + (Y1 - b)*(Y1 - b));
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 4, X1, Y1);
    }
    if (tileY < tileHeight-1) {
        double dist = Y1-b;
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 5, X1, Y1);
    }
    if ((tileX > 0) && (tileY < tileHeight-1)) {
        double dist = sqrt((X1 - l)*(X1 - l) + (Y1 - b)*(Y1 - b));
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 6, X1, Y1);
    }
    if (tileX > 0) {
        double dist = X1-l;
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 7, X1, Y1);
    }

    if ((tileX > 0) && (tileY > 0)) {
        double dist = sqrt((X2 - l)*(X2 - l) + (Y2 - t)*(Y2 - t));
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 0, X2, Y2);
    }
    if (tileY > 0) {
        double dist = t - Y2;
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 1, X2, Y2);
    }
    if ((tileX < tileWidth-1) && (tileY > 0)) {
        double dist = sqrt((X2 - r)*(X2 - r) + (Y2 - t)*(Y2 - t));
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 2, X2, Y2);
    }
    if (tileX < tileWidth-1) {
        double dist = r - X2;
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 3, X2, Y2);
    }
    if ((tileX < tileWidth-1) && (tileY < tileHeight-1)) {
        double dist = sqrt((X2 - r)*(X2 - r) + (Y2 - b)*(Y2 - b));
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 4, X2, Y2);
    }
    if (tileY < tileHeight-1) {
        double dist = Y2 - b;
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 5, X2, Y2);
    }
    if ((tileX > 0) && (tileY < tileHeight-1)) {
        double dist = sqrt((X2 - l)*(X2 - l) + (Y2 - b)*(Y2 - b));
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 6, X2, Y2);
    }
    if (tileX > 0) {
        double dist = X2 - l;
        if (dist < lineToler) dg->AddCorner(tileX, tileY, 7, X2, Y2);
    }
}

void TileGeometryGrid::RegisterPolygon(TileGeometry*dg, int tileX, int tileY) {
    AutoPtr<Geometry>geo=dg->GetGeometry();
    AutoPtr<Polygon>poly(geo);
    LONG count=poly->GetSize();
    if (count == 0) return;
    AutoPtr<Points>pRing=poly->GetItem(0);
    RegisterRing(dg, pRing.get(), tileX, tileY);
}
void TileGeometryGrid::RegisterPolyline(TileGeometry*dg, int tileX, int tileY) {
    AutoPtr<Geometry>geo=dg->GetGeometry();
    AutoPtr<Polyline>poly(geo);
    LONG count=poly->GetSize();
    if (count == 0) return;
    AutoPtr<Points>dpts=poly->GetItem(0);
    RegisterPath(dg, dpts.get(), tileX, tileY);
}

void TileGeometryGrid::RemoveGeometry(TileGeometry*dg) {
    int cellCount = dg->GetCellCount();
    for (int k = 0; k < cellCount; k++) {
        TileGeometryCell*cell = dg->GetCell(k);
        cell->RemoveTileGeometry(dg);
    }
    tgs->RemoveGeometry(dg);
}

int TileGeometryGrid::GetCornerFromNode(TileGeometry*tg, TileCorner*corner, int intersectIndex) {
    int borderCode = corner->GetBorderCode();
    DOUBLE sX = corner->GetCornerPoint(intersectIndex).X;
    DOUBLE sY = corner->GetCornerPoint(intersectIndex).Y;
    AutoPtr<Geometry>geo=tg->GetGeometry();
    AutoPtr<Polygon>poly(geo);
    AutoPtr<Points>pts=poly->GetItem(0);
    DOUBLE*XYs=pts->GetXYs();
    LONG ptsCount=pts->GetSize();
    DOUBLE X1, Y1, X2, Y2;
    int fromPos = -1;
    if (borderCode == 0) {
        double miny = 0;
        for (int k = ptsCount-1; k >=0; k--) {
            X1 = XYs[k << 1];
            Y1 = XYs[(k << 1) + 1];
            if (k == ptsCount - 1) {
                X2 = XYs[0];
                Y2 = XYs[1];
            }
            if ((fabs(X1 - X2) <= polyToler) && (fabs(Y1 - Y2) <= polyToler)) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if (Y1 < sY - polyToler) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if ((fabs(X1 - sX) <= polyToler)&& (fabs(X2 - sX) <= polyToler)) {
                if (Y2 < Y1- polyToler) {
                    X2 = X1;
                    Y2 = Y1;
                    continue;
                }
                if (fromPos == -1) {
                    miny = Y1;
                    fromPos = k;
                }
                else if (Y1 < miny) {
                    miny = Y1;
                    fromPos = k;
                }
            }
            X2 = X1;
            Y2 = Y1;
        }
    }
    else if (borderCode == 1) {
        double minx = 0;
        for (int k = ptsCount - 1; k >= 0; k--) {
            X1 = XYs[k<<1];
            Y1 = XYs[(k<<1)+1];
            if (k == ptsCount-1) {
                X2 = XYs[0];
                Y2 = XYs[1];
            }
            if ((fabs(X1 - X2) <= polyToler) && (fabs(Y1 - Y2) <= polyToler)) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if (X1 < sX - polyToler) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if ((fabs(Y1 - sY) <= polyToler)&&(fabs(Y2 - sY) <= polyToler)) {
                if (X2 < X1-polyToler) {
                    X2 = X1;
                    Y2 = Y1;
                    continue;
                }
                if (fromPos == -1) {
                    minx = X1;
                    fromPos = k;
                }
                else if (X1 < minx) {
                    minx = X1;
                    fromPos = k;
                }
            }
            X2 = X1;
            Y2 = Y1;
        }
    }
    else if (borderCode == 2) {
        double maxy = 0;
        for (int k = ptsCount - 1; k >= 0; k--) {
            X1 = XYs[k << 1];
            Y1 = XYs[(k << 1) + 1];
            if (k == ptsCount - 1) {
                X2 = XYs[0];
                Y2 = XYs[1];
            }
            if ((fabs(X1 - X2) <= polyToler) && (fabs(Y1 - Y2) <= polyToler)) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if (Y1 > sY + polyToler) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if ((fabs(X1 - sX) <= polyToler)&& (fabs(X2 - sX) <= polyToler)) {
                if(Y2 > Y1+ polyToler) {
                    X2 = X1;
                    Y2 = Y1;
                    continue;
                }
                if (fromPos == -1) {
                    maxy = Y1;
                    fromPos = k;
                }
                else if (Y1 > maxy) {
                    maxy = Y1;
                    fromPos = k;
                }
            }
            X2 = X1;
            Y2 = Y1;
        }
    }
    else {
        double maxx = 0;
        for (int k = ptsCount - 1; k >= 0; k--) {
            X1 = XYs[k << 1];
            Y1 = XYs[(k << 1) + 1];
            if (k == ptsCount - 1) {
                X2 = XYs[0];
                Y2 = XYs[1];
            }
            if ((fabs(X1 - X2) <= polyToler) && (fabs(Y1 - Y2) <= polyToler)) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if (X1 > sX + polyToler) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if ((fabs(Y1 - sY) <= polyToler)&& (fabs(Y2 - sY) <= polyToler)) {
                if (X2 > X1+ polyToler) {
                    X2 = X1;
                    Y2 = Y1;
                    continue;
                }
                if (fromPos == -1) {
                    maxx = X1;
                    fromPos = k;
                }
                else if (X1 > maxx) {
                    maxx = X1;
                    fromPos = k;
                }
            }
            X2 = X1;
            Y2 = Y1;
        }
    }
    return fromPos;
}

int TileGeometryGrid::GetNeigborCornerFromNode(TileGeometry*tg, TileCorner*corner, int intersectIndex) {
    int neightX = corner->GetNeighborCellX();
    int neightY = corner->GetNeighborCellY();
    int borderCode = corner->GetBorderCode();
    TileCorner*ocorner = tg->GetCorner(neightX, neightY, corner->GetNeighborCode());
    int ocount = ocorner->GetCount();
    if (ocount == 0) return -1;
    AutoPtr<Geometry>geo=tg->GetGeometry();
    AutoPtr<Polygon>poly(geo);
    AutoPtr<Points>pts=poly->GetItem(0);
    DOUBLE*XYs=pts->GetXYs();
    LONG ptsCount=pts->GetSize();
    DOUBLE X1, Y1, X2, Y2;
    int fromPos = -1;
    if (borderCode == 0) {
        if (intersectIndex + 1 >= corner->GetCount()) return -1;
        DOUBLE sY1 = corner->GetCornerPoint(intersectIndex).Y;
        DOUBLE sY2 = corner->GetCornerPoint(intersectIndex+1).Y;
        DOUBLE sX, sY;
        bool hasFind = false;
        for (int k = ocount-2; k >=0; k-=2) {
            DOUBLE oY1 = ocorner->GetCornerPoint(k).Y;
            DOUBLE oY2 = ocorner->GetCornerPoint(k+1).Y;
            if ((oY2 <= sY2 + polyToler) && (oY1 >= sY1 - polyToler)) {
                sX = ocorner->GetCornerPoint(k + 1).X;
                sY = oY2;
                hasFind = true;
                break;
            }
        }
        if (!hasFind) return -1;
        double miny = 0;
        int nIndex = 0;
        for (int k = 0; k < ptsCount; k++) {
            X1 = XYs[nIndex++];
            Y1 = XYs[nIndex++];
            if (k == 0) {
                X2 = XYs[(ptsCount - 1) << 1];
                Y2 = XYs[((ptsCount - 1) << 1) + 1];
            }
            if ((fabs(X1 - X2) <= polyToler) && (fabs(Y1 - Y2) <= polyToler)) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if (Y1 < sY - polyToler) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if ((fabs(X1 - sX) <= polyToler)&& (fabs(X2 - sX) <= polyToler)) {
                if (Y2 < Y1- polyToler) {
                    X2 = X1;
                    Y2 = Y1;
                    continue;
                }
                if (fromPos == -1) {
                    miny = Y1;
                    fromPos = k;
                }
                else if (Y1 < miny) {
                    miny = Y1;
                    fromPos = k;
                }
            }
            X2 = X1;
            Y2 = Y1;
        }
    }
    else if (borderCode == 1) {
        if (intersectIndex + 1 >= corner->GetCount()) return -1;
        DOUBLE sX1 = corner->GetCornerPoint(intersectIndex).X;
        DOUBLE sX2 = corner->GetCornerPoint(intersectIndex+1).X;
        DOUBLE sX, sY;
        bool hasFind = false;
        for (int k = ocount - 2; k >= 0; k-=2) {
            DOUBLE oX1 = ocorner->GetCornerPoint(k).X;
            DOUBLE oX2 = ocorner->GetCornerPoint(k + 1).X;
            if ((oX2 <= sX2 + polyToler) && (oX1 >= sX1 - polyToler)) {
                sX = oX2;
                sY = ocorner->GetCornerPoint(k + 1).Y;
                hasFind = true;
                break;
            }
        }
        if (!hasFind) return -1;
        double minx = 0;
        int nIndex = 0;
        for (int k = 0; k < ptsCount; k++) {
            X1 = XYs[nIndex++];
            Y1 = XYs[nIndex++];
            if (k == 0) {
                X2 = XYs[(ptsCount - 1) << 1];
                Y2 = XYs[((ptsCount - 1) << 1) + 1];
            }
            if ((fabs(X1 - X2) <= polyToler) && (fabs(Y1 - Y2) <= polyToler)) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if (X1 < sX - polyToler) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if ((fabs(Y1 - sY) <= polyToler)&& (fabs(Y2 - sY) <= polyToler)) {
                if (X2 < X1- polyToler)  {
                    X2 = X1;
                    Y2 = Y1;
                    continue;
                }
                if (fromPos == -1) {
                    minx = X1;
                    fromPos = k;
                }
                else if (X1 < minx) {
                    minx = X1;
                    fromPos = k;
                }
            }
            X2 = X1;
            Y2 = Y1;
        }
    }
    else if (borderCode == 2) {
        if (intersectIndex + 1 >= corner->GetCount()) return -1;
        DOUBLE sY1 = corner->GetCornerPoint(intersectIndex).Y;
        DOUBLE sY2 = corner->GetCornerPoint(intersectIndex+1).Y;
        DOUBLE sX, sY;
        bool hasFind = false;
        for (int k = ocount - 2; k >= 0; k-=2) {
            DOUBLE oY1 = ocorner->GetCornerPoint(k).Y;
            DOUBLE oY2 = ocorner->GetCornerPoint(k + 1).Y;
            if ((oY2 >= sY2 + polyToler) && (oY1 <= sY1 - polyToler)) {
                sX = ocorner->GetCornerPoint(k + 1).X;
                sY = oY2;
                hasFind = true;
                break;
            }
        }
        if (!hasFind) return -1;
        double maxy = 0;
        int nIndex = 0;
        for (int k = 0; k < ptsCount; k++) {
            X1 = XYs[nIndex++];
            Y1 = XYs[nIndex++];
            if (k == 0) {
                X2 = XYs[(ptsCount - 1) << 1];
                Y2 = XYs[((ptsCount - 1) << 1) + 1];
            }
            if ((fabs(X1 - X2) <= polyToler) && (fabs(Y1 - Y2) <= polyToler)) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if (Y1 > sY + polyToler) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if ((fabs(X1 - sX) <= polyToler)&& (fabs(X2 - sX) <= polyToler)) {
                if (Y2 > Y1+ polyToler) {
                    X2 = X1;
                    Y2 = Y1;
                    continue;
                }
                if (fromPos == -1) {
                    maxy = Y1;
                    fromPos = k;
                }
                else if (Y1 > maxy) {
                    maxy = Y1;
                    fromPos = k;
                }
            }
            X2 = X1;
            Y2 = Y1;
        }
    }
    else {
        if (intersectIndex + 1 >= corner->GetCount()) return -1;
        DOUBLE sX1 = corner->GetCornerPoint(intersectIndex).X;
        DOUBLE sX2 = corner->GetCornerPoint(intersectIndex+1).X;
        DOUBLE sX, sY;
        bool hasFind = false;
        for (int k = ocount - 2; k >= 0; k-=2) {
            DOUBLE oX1 = ocorner->GetCornerPoint(k).X;
            DOUBLE oX2 = ocorner->GetCornerPoint(k + 1).X;
            if ((oX2 >= sX2 + polyToler) && (oX1 <= sX1 - polyToler)) {
                sX = oX2;
                sY = ocorner->GetCornerPoint(k + 1).Y;
                hasFind = true;
                break;
            }
        }
        if (!hasFind) return -1;
        double maxx = 0;
        int nIndex = 0;
        for (int k = 0; k < ptsCount; k++) {
            X1 = XYs[nIndex++];
            Y1 = XYs[nIndex++];
            if (k == 0) {
                X2 = XYs[(ptsCount - 1) << 1];
                Y2 = XYs[((ptsCount - 1) << 1) + 1];
            }
            if ((fabs(X1 - X2) <= polyToler) && (fabs(Y1 - Y2) <= polyToler)) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if (X1 > sX + polyToler) {
                X2 = X1;
                Y2 = Y1;
                continue;
            }
            if ((fabs(Y1 - sY) <= polyToler)&& (fabs(Y2 - sY) <= polyToler)) {
                if (X2 > X1+ polyToler){
                    X2 = X1;
                    Y2 = Y1;
                    continue;
                }
                if (fromPos == -1) {
                    maxx = X1;
                    fromPos = k;
                }
                else if (X1 > maxx) {
                    maxx = X1;
                    fromPos = k;
                }
            }
            X2 = X1;
            Y2 = Y1;
        }
    }
    return fromPos;
}

void TileGeometryGrid::SplitRing(TileGeometry*tg, TileCorner*corner) {
    int borderCode = corner->GetBorderCode();
    AutoPtr<Geometry>geo=tg->GetGeometry();
    AutoPtr<Polygon>poly(geo);
    AutoPtr<Points>pts=poly->GetItem(0);
    LONG ptsCount=pts->GetSize();
    if (ptsCount < 3) return;
    DOUBLE*XYs=pts->GetXYs();
    DOUBLE X1, Y1, X2, Y2;
    LONG cornerCount = corner->GetCount();
    if (cornerCount == 0) return;
    if (borderCode == 0) {
        int curCorner = 0;
        for (int c = 0; c < cornerCount; c++) {
            DOUBLE sX = corner->GetCornerPoint(c).X;
            int fromPos = GetNeigborCornerFromNode(tg, corner, c);
            if (fromPos == -1) continue;
            curCorner = c;
            X1 = XYs[fromPos << 1];
            Y1 = XYs[(fromPos << 1) + 1];
            for (int k = 1; k <= ptsCount; k++) {
                int toPos = fromPos - k;
                if (toPos < 0) toPos += ptsCount;
                X2 = XYs[toPos << 1];
                Y2 = XYs[(toPos << 1) + 1];
                if ((X1 == X2) && (Y1 == Y2)) continue;
                if ((fabs(X2 - sX) > polyToler) || (fabs(X1 - sX) > polyToler)) {
                    X1 = X2;
                    Y1 = Y2;
                    continue;
                }
                for (int p = curCorner; p < cornerCount; p++) {
                    Point2D<double>ip = corner->GetCornerPoint(curCorner);
                    if (ip.Y < Y1 + polyToler) {
                        curCorner++;
                        continue;
                    }
                    if (ip.Y > Y2 + polyToler) break;
                    if (fabs(ip.Y - Y2) <= polyToler) {
                        curCorner++;
                        break;
                    }
                    int nPos = toPos + 1;
                    if (nPos >= ptsCount) nPos -= ptsCount;
                    pts->Insert(nPos, ip.X, ip.Y);
                    ptsCount++;
                    X1 = ip.X;
                    Y1 = ip.Y;
                    curCorner++;
                }
                if (curCorner >= cornerCount) break;
                X1 = X2;
                Y1 = Y2;
            }
        }
    }
    else if (borderCode == 1) {
        int curCorner = 0;
        for (int c = 0; c < cornerCount; c++) {
            DOUBLE sY = corner->GetCornerPoint(c).Y;
            int fromPos = GetNeigborCornerFromNode(tg, corner, c);
            if (fromPos == -1) continue;
            X1 = XYs[fromPos << 1];
            Y1 = XYs[(fromPos << 1) + 1];
            for (int k = 1; k <= ptsCount; k++) {
                int toPos = fromPos - k;
                if (toPos < 0) toPos += ptsCount;
                X2 = XYs[toPos << 1];
                Y2 = XYs[(toPos << 1) + 1];
                if ((X1 == X2) && (Y1 == Y2)) continue;
                if ((fabs(Y2 - sY) > polyToler) || (fabs(Y1 - sY) > polyToler)) {
                    X1 = X2;
                    Y1 = Y2;
                    continue;
                }
                for (int p = curCorner; p < cornerCount; p++) {
                    Point2D<double>ip = corner->GetCornerPoint(curCorner);
                    if (ip.X < X1 + polyToler) {
                        curCorner++;
                        continue;
                    }
                    if (ip.X > X2 + polyToler) break;
                    if (fabs(ip.X - X2) <= polyToler) {
                        curCorner++;
                        break;
                    }
                    int nPos = toPos + 1;
                    if (nPos >= ptsCount) nPos -= ptsCount;
                    pts->Insert(nPos, ip.X, ip.Y);
                    ptsCount++;
                    X1 = ip.X;
                    Y1 = ip.Y;
                    curCorner++;
                }
                if (curCorner >= cornerCount) break;
                X1 = X2;
                Y1 = Y2;
            }
        }
    }
    else if (borderCode == 2) {
        int curCorner = 0;
        for (int c = 0; c < cornerCount; c++) {
            DOUBLE sX = corner->GetCornerPoint(c).X;
            int fromPos = GetNeigborCornerFromNode(tg, corner, c);
            if (fromPos == -1) continue;
            X1 = XYs[fromPos << 1];
            Y1 = XYs[(fromPos << 1) + 1];
            for (int k = 1; k <= ptsCount; k++) {
                int toPos = fromPos - k;
                if (toPos < 0) toPos += ptsCount;
                X2 = XYs[toPos << 1];
                Y2 = XYs[(toPos << 1) + 1];
                if ((X1 == X2) && (Y1 == Y2)) continue;
                if ((fabs(X2 - sX) > polyToler) || (fabs(X1 - sX) > polyToler)) {
                    X1 = X2;
                    Y1 = Y2;
                    continue;
                }
                for (int p = curCorner; p < cornerCount; p++) {
                    Point2D<double>ip = corner->GetCornerPoint(curCorner);
                    if (ip.Y > Y1 - polyToler) {
                        curCorner++;
                        continue;
                    }
                    if (ip.Y < Y2 - polyToler) break;
                    if (fabs(ip.Y - Y2) <= polyToler) {
                        curCorner++;
                        break;
                    }
                    int nPos = toPos + 1;
                    if (nPos >= ptsCount) nPos -= ptsCount;
                    pts->Insert(nPos, ip.X, ip.Y);
                    ptsCount++;
                    X1 = ip.X;
                    Y1 = ip.Y;
                    curCorner++;
                }
                if (curCorner >= cornerCount) break;
                X1 = X2;
                Y1 = Y2;
            }
        }
    }
    else if (borderCode == 3) {
        int curCorner = 0;
        for (int c = 0; c < cornerCount; c++) {
            DOUBLE sY = corner->GetCornerPoint(c).Y;
            int fromPos = GetNeigborCornerFromNode(tg, corner, c);
            if (fromPos == -1) continue;
            X1 = XYs[fromPos << 1];
            Y1 = XYs[(fromPos << 1) + 1];
            for (int k = 1; k <= ptsCount; k++) {
                int toPos = fromPos - k;
                if (toPos < 0) toPos += ptsCount;
                X2 = XYs[toPos << 1];
                Y2 = XYs[(toPos << 1) + 1];
                if ((X1 == X2) && (Y1 == Y2)) continue;
                if ((fabs(Y2 - sY) > polyToler) || (fabs(Y1 - sY) > polyToler)) {
                    X1 = X2;
                    Y1 = Y2;
                    continue;
                }
                for (int p = curCorner; p < cornerCount; p++) {
                    Point2D<double>ip = corner->GetCornerPoint(curCorner);
                    if (ip.X > X1 - polyToler) {
                        curCorner++;
                        continue;
                    }
                    if (ip.X < X2 - polyToler) break;
                    if (fabs(ip.X - X2) <= polyToler) {
                        curCorner++;
                        break;
                    }
                    int nPos = toPos + 1;
                    if (nPos >= ptsCount) nPos -= ptsCount;
                    pts->Insert(nPos, ip.X, ip.Y);
                    ptsCount++;
                    X1 = ip.X;
                    Y1 = ip.Y;
                    curCorner++;
                }
                if (curCorner >= cornerCount) break;
                X1 = X2;
                Y1 = Y2;
            }
        }
    }
}
bool TileGeometryGrid::UnionPolygon(TileGeometry*tg, TileCorner*corner) {
    //自我合并
    TileGeometry*other = tg;
    int neightX = corner->GetNeighborCellX();
    int neightY = corner->GetNeighborCellY();
    int borderCode = corner->GetBorderCode();
    TileCorner*ocorner = other->GetCorner(neightX, neightY, corner->GetNeighborCode());
    if (ocorner == NULL) return false;
    SplitRing(tg, corner);
    SplitRing(tg, ocorner);
    if (ocorner->GetCount() == 0) return false;
    if (borderCode == 0) {
        double miny = corner->GetCornerPoint(0).Y;
        double maxy = corner->GetCornerPoint(corner->GetCount() - 1).Y;
        double miny2 = ocorner->GetCornerPoint(ocorner->GetCount() - 1).Y;
        double maxy2 = ocorner->GetCornerPoint(0).Y;
        if ((miny > maxy2) || (miny2 > maxy)) return false;
    }
    else if (borderCode == 1) {
        double minx = corner->GetCornerPoint(0).X;
        double maxx = corner->GetCornerPoint(corner->GetCount() - 1).X;
        double minx2 = ocorner->GetCornerPoint(ocorner->GetCount() - 1).X;
        double maxx2 = ocorner->GetCornerPoint(0).X;
        if ((minx > maxx2) || (minx2 > maxx)) return false;
    }
    else if (borderCode == 2) {
        double miny = corner->GetCornerPoint(corner->GetCount() - 1).Y;
        double maxy = corner->GetCornerPoint(0).Y;
        double miny2 = ocorner->GetCornerPoint(0).Y;
        double maxy2 = ocorner->GetCornerPoint(ocorner->GetCount() - 1).Y;
        if ((miny > maxy2) || (miny2 > maxy)) return false;
    }
    else if (borderCode == 3) {
        double minx = corner->GetCornerPoint(corner->GetCount() - 1).X;
        double maxx = corner->GetCornerPoint(0).X;
        double minx2 = ocorner->GetCornerPoint(0).X;
        double maxx2 = ocorner->GetCornerPoint(ocorner->GetCount() - 1).X;
        if ((minx > maxx2) || (minx2 > maxx)) return false;
    }
    AutoPtr<Geometry>geo1=tg->GetGeometry();
    AutoPtr<Geometry>geo2=other->GetGeometry();
    AutoPtr<Polygon>poly1(geo1);
    AutoPtr<Polygon>poly2(geo2);
    AutoPtr<Points>points1=poly1->GetItem(0);
    AutoPtr<Points>points2=poly2->GetItem(0);
    LONG ptsCount1=points1->GetSize();
    LONG ptsCount2=points2->GetSize();
    vector<int>capNodes, ocapNodes;
    int cornerCount = corner->GetCount();
    DOUBLE X1, Y1;
    if (borderCode == 0) {
        for (int k = 0; k < cornerCount; k += 2) {
            DOUBLE*XYs=points2->GetXYs();
            DOUBLE sX = corner->GetCornerPoint(k).X;
            DOUBLE sY2 = ocorner->GetCornerPoint(0).Y;
            int fromPos1 = GetCornerFromNode(tg, corner, k);
            int fromPos2 = GetNeigborCornerFromNode(other, corner, k);
            if (fromPos2 == -1) continue;
            Point2D<double>dpt1 = corner->GetCornerPoint(k);
            Point2D<double>dpt2 = corner->GetCornerPoint(k + 1);
            double dpt1Y = dpt1.Y;
            double dpt2Y = dpt2.Y;
            bool bValid = true;
            while (bValid) {
                bValid = false;
                LONG lnode1 = -1, lnode2 = -1;
                LONG rnode1 = -1, rnode2 = -1;
                XYs=points2->GetXYs();
                bool isOut = false;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY2) <= polyToler) {
                        isOut = true;
                    }
                    if (fabs(X1 - sX) > polyToler) {
                        if (isOut) break;
                        continue;
                    }
                    if ((Y1 >= dpt1Y - polyToler) && (Y1 <= dpt2Y + polyToler)) {
                        rnode1 = nPos;
                        fromPos2 = nPos - 1;
                        dpt1.Y = Y1;
                        if (fromPos2 < 0) fromPos2 += ptsCount2;
                        break;
                    }
                }
                if (rnode1 == -1) break;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                    if ((Y1 >= dpt1Y - polyToler) && (Y1 <= dpt2Y + polyToler)) {
                        dpt2.Y = Y1;
                        rnode2 = nPos;
                    }
                    if (Y1 > dpt2Y + polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                }
                if (rnode2 == -1) break;
                XYs=points1->GetXYs();
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        break;
                    }
                    if (fabs(Y1 - dpt1.Y) <= polyToler) {
                        lnode1 = nPos;
                        fromPos1 = nPos + 1;
                        if (fromPos1 >= ptsCount1) fromPos1 -= ptsCount1;
                        break;
                    }
                }
                if (lnode1 == -1) break;
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                    if (fabs(Y1 - dpt2.Y) <= polyToler) {
                        lnode2 = nPos;
                    }
                    else if (Y1 > dpt2.Y + polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                }
                if ((lnode1 >= 0) && (lnode2 >= 0) && (rnode1 >= 0) && (rnode2 >= 0)) {
                    capNodes.push_back(lnode1);
                    capNodes.push_back(lnode2);
                    ocapNodes.push_back(rnode1);
                    ocapNodes.push_back(rnode2);
                    bValid = true;
                    continue;
                }
                break;
            }
        }
    }
    else if (borderCode == 1) {
        for (int k = 0; k < cornerCount; k += 2) {
            DOUBLE*XYs=points2->GetXYs();
            DOUBLE sY = corner->GetCornerPoint(k).Y;
            DOUBLE sX2 = ocorner->GetCornerPoint(0).X;
            int fromPos1 = GetCornerFromNode(tg, corner, k);
            int fromPos2 = GetNeigborCornerFromNode(other, corner, k);
            if (fromPos2 == -1) continue;
            Point2D<double>dpt1 = corner->GetCornerPoint(k);
            Point2D<double>dpt2 = corner->GetCornerPoint(k + 1);
            double dpt1X = dpt1.X;
            double dpt2X = dpt2.X;
            bool bValid = true;
            while (bValid) {
                bValid = false;
                LONG lnode1 = -1, lnode2 = -1;
                LONG rnode1 = -1, rnode2 = -1;
                XYs=points2->GetXYs();
                bool isOut = false;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX2) <= polyToler) {
                        isOut = true;
                    }
                    if (fabs(Y1 - sY) > polyToler) {
                        if (isOut) break;
                        continue;
                    }
                    if ((X1 >= dpt1X - polyToler) && (X1 <= dpt2X + polyToler)) {
                        rnode1 = nPos;
                        fromPos2 = nPos - 1;
                        dpt1.X = X1;
                        if (fromPos2 < 0) fromPos2 += ptsCount2;
                        break;
                    }
                }
                if (rnode1 == -1) break;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                    if ((X1 >= dpt1X - polyToler) && (X1 <= dpt2X + polyToler)) {
                        dpt2.X = X1;
                        rnode2 = nPos;
                    }
                    if (X1 > dpt2X + polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                }
                if (rnode2 == -1) break;
                XYs=points1->GetXYs();
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        break;
                    }
                    if (fabs(X1 - dpt1.X) <= polyToler) {
                        lnode1 = nPos;
                        fromPos1 = nPos + 1;
                        if (fromPos1 >= ptsCount1) fromPos1 -= ptsCount1;
                        break;
                    }
                }
                if (lnode1 == -1) break;
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                    if (fabs(X1 - dpt2.X) <= polyToler) {
                        lnode2 = nPos;
                    }
                    else if (X1 > dpt2.X + polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                }
                if ((lnode1 >= 0) && (lnode2 >= 0) && (rnode1 >= 0) && (rnode2 >= 0)) {
                    capNodes.push_back(lnode1);
                    capNodes.push_back(lnode2);
                    ocapNodes.push_back(rnode1);
                    ocapNodes.push_back(rnode2);
                    bValid = true;
                    continue;
                }
                break;
            }
        }
    }
    else if (borderCode == 2) {
        for (int k = 0; k < cornerCount; k += 2) {
            DOUBLE*XYs=points2->GetXYs();
            DOUBLE sX = corner->GetCornerPoint(k).X;
            DOUBLE sY = corner->GetCornerPoint(k).Y;
            DOUBLE sY2 = ocorner->GetCornerPoint(0).Y;
            int fromPos1 = GetCornerFromNode(tg, corner, k);
            int fromPos2 = GetNeigborCornerFromNode(other, corner, k);
            if (fromPos2 == -1) continue;
            Point2D<double>dpt1 = corner->GetCornerPoint(k);
            Point2D<double>dpt2 = corner->GetCornerPoint(k + 1);
            double dpt1Y = dpt1.Y;
            double dpt2Y = dpt2.Y;
            bool bValid = true;
            while (bValid) {
                bValid = false;
                LONG lnode1 = -1, lnode2 = -1;
                LONG rnode1 = -1, rnode2 = -1;
                XYs=points2->GetXYs();
                bool isOut = false;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY2) <= polyToler) {
                        isOut = true;
                    }
                    if (fabs(X1 - sX) > polyToler) {
                        if (isOut) break;
                        continue;
                    }
                    if ((Y1 <= dpt1Y + polyToler) && (Y1 >= dpt2Y - polyToler)) {
                        rnode1 = nPos;
                        fromPos2 = nPos - 1;
                        dpt1.Y = Y1;
                        if (fromPos2 < 0) fromPos2 += ptsCount2;
                        break;
                    }
                }
                if (rnode1 == -1) break;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                    if ((Y1 <= dpt1Y + polyToler) && (Y1 >= dpt2Y - polyToler)) {
                        dpt2.Y = Y1;
                        rnode2 = nPos;
                    }
                    if (Y1 < dpt2Y - polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                }
                if (rnode2 == -1) break;
                XYs=points1->GetXYs();
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        break;
                    }
                    if (fabs(Y1 - dpt1.Y) <= polyToler) {
                        lnode1 = nPos;
                        fromPos1 = nPos + 1;
                        if (fromPos1 >= ptsCount1) fromPos1 -= ptsCount1;
                        break;
                    }
                }
                if (lnode1 == -1) break;
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                    if (fabs(Y1 - dpt2.Y) <= polyToler) {
                        lnode2 = nPos;
                    }
                    else if (Y1 < dpt2.Y - polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                }
                if ((lnode1 >= 0) && (lnode2 >= 0) && (rnode1 >= 0) && (rnode2 >= 0)) {
                    capNodes.push_back(lnode1);
                    capNodes.push_back(lnode2);
                    ocapNodes.push_back(rnode1);
                    ocapNodes.push_back(rnode2);
                    bValid = true;
                    continue;
                }
                break;
            }
        }
    }
    else if (borderCode == 3) {
        for (int k = 0; k < cornerCount; k += 2) {
            DOUBLE*XYs=points2->GetXYs();
            DOUBLE sY = corner->GetCornerPoint(k).Y;
            DOUBLE sX2 = ocorner->GetCornerPoint(0).X;
            int fromPos1 = GetCornerFromNode(tg, corner, k);
            int fromPos2 = GetNeigborCornerFromNode(other, corner, k);
            if (fromPos2 == -1) continue;
            Point2D<double>dpt1 = corner->GetCornerPoint(k);
            Point2D<double>dpt2 = corner->GetCornerPoint(k + 1);
            double dpt1X = dpt1.X;
            double dpt2X = dpt2.X;
            bool bValid = true;
            while (bValid) {
                bValid = false;
                LONG lnode1 = -1, lnode2 = -1;
                LONG rnode1 = -1, rnode2 = -1;
                XYs=points2->GetXYs();
                bool isOut = false;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX2) <= polyToler) {
                        isOut = true;
                    }
                    if (fabs(Y1 - sY) > polyToler) {
                        if (isOut) break;
                        continue;
                    }
                    if ((X1 <= dpt1X + polyToler) && (X1 >= dpt2X - polyToler)) {
                        rnode1 = nPos;
                        fromPos2 = nPos - 1;
                        dpt1.X = X1;
                        if (fromPos2 < 0) fromPos2 += ptsCount2;
                        break;
                    }
                }
                if (rnode1 == -1) break;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                    if ((X1 <= dpt1X + polyToler) && (X1 >= dpt2X - polyToler)) {
                        dpt2.X = X1;
                        rnode2 = nPos;
                    }
                    if (X1 < dpt2X - polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                }
                if (rnode2 == -1) break;
                XYs=points1->GetXYs();
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        break;
                    }
                    if (fabs(X1 - dpt1.X) <= polyToler) {
                        lnode1 = nPos;
                        fromPos1 = nPos + 1;
                        if (fromPos1 >= ptsCount1) fromPos1 -= ptsCount1;
                        break;
                    }
                }
                if (lnode1 == -1) break;
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                    if (fabs(X1 - dpt2.X) <= polyToler) {
                        lnode2 = nPos;
                    }
                    else if (X1 < dpt2.Y - polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                }
                if ((lnode1 >= 0) && (lnode2 >= 0) && (rnode1 >= 0) && (rnode2 >= 0)) {
                    capNodes.push_back(lnode1);
                    capNodes.push_back(lnode2);
                    ocapNodes.push_back(rnode1);
                    ocapNodes.push_back(rnode2);
                    bValid = true;
                    continue;
                }
                break;
            }
        }
    }
    if (capNodes.size() == 0) return false;
    AutoPtr<Polygon>newPoly(new Polygon());
    AutoPtr<Points>outerPoints(new Points());
    int attCount = capNodes.size();
    int fromNode = capNodes[attCount - 1];
    int toNode = ocapNodes[attCount - 1];
    if (toNode < fromNode) toNode += ptsCount1;
    for (int k = fromNode; k <= toNode; k++) {
        int nPos = k;
        if (nPos >= ptsCount1) nPos -= ptsCount1;
        DOUBLE X, Y;
        points1->GetItem(nPos, X, Y);
        outerPoints->Add(X, Y);
    }
    AutoPtr<Ring>outerRing(outerPoints);
    AutoPtr<Points>outerPoints2(new Points());
    fromNode = ocapNodes[0];
    toNode = capNodes[0];
    if (toNode < fromNode) toNode += ptsCount1;
    for (int k = fromNode; k <= toNode; k++) {
        int nPos = k;
        if (nPos >= ptsCount1) nPos -= ptsCount1;
        DOUBLE X, Y;
        points1->GetItem(nPos, X, Y);
        outerPoints2->Add(X, Y);
    }
    AutoPtr<Ring>outerRing2(outerPoints2);
    double area=outerRing->GetArea();
    double area2=outerRing2->GetArea();
    if (area > 0) {
        newPoly->Add(outerRing);
        if(fabs(area2) >polyToler) newPoly->Add(outerRing2);
    }
    else {
        newPoly->Add(outerRing2);
        if (fabs(area) > polyToler) newPoly->Add(outerRing);
    }
    for (int p = attCount - 2; p > 0; p -= 2) {
        AutoPtr<Points>innerPoints(new Points());
        int fromNode = capNodes[p];
        int toNode = capNodes[p + 1];
        if (toNode < fromNode) toNode += ptsCount1;
        for (int k = fromNode; k <= toNode; k++) {
            int nPos = k;
            if (nPos >= ptsCount1) nPos -= ptsCount1;
            DOUBLE X, Y;
            points1->GetItem(nPos, X, Y);
            innerPoints->Add(X, Y);
        }
        fromNode = ocapNodes[p + 1];
        toNode = ocapNodes[p];
        if (toNode < fromNode) toNode += ptsCount2;
        for (int k = fromNode; k <= toNode; k++) {
            int nPos = k;
            if (nPos >= ptsCount2) nPos -= ptsCount2;
            DOUBLE X, Y;
            points2->GetItem(nPos, X, Y);
            innerPoints->Add(X, Y);
        }
        AutoPtr<Ring>innerRing(innerPoints);
        newPoly->Add(innerRing);
    }
    //将原两侧的多边形的内环添加至新的拼接多边形中
    LONG partsCount=poly1->GetSize();
    for (int k = 1; k < partsCount; k++) {
        AutoPtr<Ring>pRing=poly1->GetItem(k);
        newPoly->Add(pRing);
    }
    //other->RemoveCorner(ocorner);
    //现在新的几何体生成了，更新dg
    tg->SetGeometry(newPoly);
    return true;
}
bool TileGeometryGrid::UnionPolygon(TileGeometry*tg, TileCorner*corner, TileGeometry*other) {
    //获取边对应的领域Cell
    //return true;
    int neightX = corner->GetNeighborCellX();
    int neightY = corner->GetNeighborCellY();
    int borderCode = corner->GetBorderCode();
    TileCorner*ocorner = other->GetCorner(neightX, neightY, corner->GetNeighborCode());
    if (ocorner == NULL) return false;
    if (ocorner->GetCount() == 0) return false;
    if (borderCode == 0) {
        double miny = corner->GetCornerPoint(0).Y;
        double maxy = corner->GetCornerPoint(corner->GetCount() - 1).Y;
        double miny2 = ocorner->GetCornerPoint(ocorner->GetCount() - 1).Y;
        double maxy2 = ocorner->GetCornerPoint(0).Y;
        if ((miny > maxy2)||(miny2 > maxy)) return false;
    }
    else if(borderCode == 1) {
        double minx = corner->GetCornerPoint(0).X;
        double maxx = corner->GetCornerPoint(corner->GetCount() - 1).X;
        double minx2 = ocorner->GetCornerPoint(ocorner->GetCount() - 1).X;
        double maxx2 = ocorner->GetCornerPoint(0).X;
        if ((minx > maxx2)||(minx2 > maxx)) return false;
    }
    else if (borderCode == 2) {
        double miny = corner->GetCornerPoint(corner->GetCount() - 1).Y;
        double maxy = corner->GetCornerPoint(0).Y;
        double miny2 = ocorner->GetCornerPoint(0).Y;
        double maxy2 = ocorner->GetCornerPoint(ocorner->GetCount() - 1).Y;
        if ((miny > maxy2) || (miny2 > maxy)) return false;
    }
    else if (borderCode == 3) {
        double minx = corner->GetCornerPoint(corner->GetCount() - 1).X;
        double maxx = corner->GetCornerPoint(0).X;
        double minx2 = ocorner->GetCornerPoint(0).X;
        double maxx2 = ocorner->GetCornerPoint(ocorner->GetCount() - 1).X;
        if ((minx > maxx2) || (minx2 > maxx)) return false;
    }
    SplitRing(other, corner);
    SplitRing(tg, ocorner);
    AutoPtr<Geometry>geo1=tg->GetGeometry();
    AutoPtr<Geometry>geo2=other->GetGeometry();
    AutoPtr<Polygon>poly1(geo1);
    AutoPtr<Polygon>poly2(geo2);
    AutoPtr<Points>points1=poly1->GetItem(0);
    AutoPtr<Points>points2=poly2->GetItem(0);
    LONG ptsCount1=points1->GetSize();
    LONG ptsCount2=points2->GetSize();
    vector<int>capNodes, ocapNodes;
    int cornerCount = corner->GetCount();
    DOUBLE X1, Y1;
    if (borderCode == 0) {
        for (int k = 0; k < cornerCount; k += 2) {
            DOUBLE*XYs=points2->GetXYs();
            DOUBLE sX = corner->GetCornerPoint(k).X;
            DOUBLE sY2 = ocorner->GetCornerPoint(0).Y;
            int fromPos1 = GetCornerFromNode(tg, corner,k);
            int fromPos2 = GetNeigborCornerFromNode(other, corner,k);
            if (fromPos2 == -1) continue;
            Point2D<double>dpt1 = corner->GetCornerPoint(k);
            Point2D<double>dpt2 = corner->GetCornerPoint(k + 1);
            double dpt1Y = dpt1.Y;
            double dpt2Y = dpt2.Y;
            bool bValid = true;
            while (bValid) {
                bValid = false;
                LONG lnode1 = -1, lnode2 = -1;
                LONG rnode1 = -1, rnode2 = -1;
                XYs=points2->GetXYs();
                bool isOut = false;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1-sY2)<=polyToler) {
                        isOut = true;
                    }
                    if (fabs(X1 - sX) > polyToler) {
                        if (isOut) break;
                        continue;
                    }
                    if ((Y1 >= dpt1Y - polyToler) && (Y1 <= dpt2Y + polyToler)) {
                        rnode1 = nPos;
                        fromPos2 = nPos - 1;
                        dpt1.Y = Y1;
                        if (fromPos2 < 0) fromPos2 += ptsCount2;
                        break;
                    }
                }
                if (rnode1 == -1) break;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                    if ((Y1 >= dpt1Y - polyToler) && (Y1 <= dpt2Y + polyToler)) {
                        dpt2.Y = Y1;
                        rnode2 = nPos;
                    }
                    if (Y1 > dpt2Y + polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                }
                if (rnode2 == -1) break;
                XYs=points1->GetXYs();
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        break;
                    }
                    if (fabs(Y1 - dpt1.Y) <= polyToler) {
                        lnode1 = nPos;
                        fromPos1 = nPos + 1;
                        if (fromPos1 >= ptsCount1) fromPos1 -= ptsCount1;
                        break;
                    }
                }
                if (lnode1 == -1) break;
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                    if (fabs(Y1 - dpt2.Y) <= polyToler) {
                        lnode2 = nPos;
                    }
                    else if (Y1 > dpt2.Y + polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                }
                if ((lnode1 >= 0) && (lnode2 >= 0) && (rnode1 >= 0) && (rnode2 >= 0)) {
                    capNodes.push_back(lnode1);
                    capNodes.push_back(lnode2);
                    ocapNodes.push_back(rnode1);
                    ocapNodes.push_back(rnode2);
                    bValid = true;
                    continue;
                }
                break;
            }
        }
    }
    else if (borderCode == 1) {
        for (int k = 0; k < cornerCount; k += 2) {
            DOUBLE*XYs=points2->GetXYs();
            DOUBLE sY = corner->GetCornerPoint(k).Y;
            DOUBLE sX2 = ocorner->GetCornerPoint(0).X;
            int fromPos1 = GetCornerFromNode(tg, corner,k);
            int fromPos2 = GetNeigborCornerFromNode(other, corner,k);
            if (fromPos2 == -1) continue;
            Point2D<double>dpt1 = corner->GetCornerPoint(k);
            Point2D<double>dpt2 = corner->GetCornerPoint(k + 1);
            double dpt1X = dpt1.X;
            double dpt2X = dpt2.X;
            bool bValid = true;
            while (bValid) {
                bValid = false;
                LONG lnode1 = -1, lnode2 = -1;
                LONG rnode1 = -1, rnode2 = -1;
                XYs=points2->GetXYs();
                bool isOut = false;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX2) <= polyToler) {
                        isOut = true;
                    }
                    if (fabs(Y1 - sY) > polyToler) {
                        if (isOut) break;
                        continue;
                    }
                    if ((X1 >= dpt1X - polyToler) && (X1 <= dpt2X + polyToler)) {
                        rnode1 = nPos;
                        fromPos2 = nPos - 1;
                        dpt1.X = X1;
                        if (fromPos2 < 0) fromPos2 += ptsCount2;
                        break;
                    }
                }
                if (rnode1 == -1) break;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                    if ((X1 >= dpt1X - polyToler) && (X1 <= dpt2X + polyToler)) {
                        dpt2.X = X1;
                        rnode2 = nPos;
                    }
                    if (X1 > dpt2X + polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                }
                if (rnode2 == -1) break;
                XYs=points1->GetXYs();
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        break;
                    }
                    if (fabs(X1 - dpt1.X) <= polyToler) {
                        lnode1 = nPos;
                        fromPos1 = nPos + 1;
                        if (fromPos1 >= ptsCount1) fromPos1 -= ptsCount1;
                        break;
                    }
                }
                if (lnode1 == -1) break;
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                    if (fabs(X1 - dpt2.X) <= polyToler) {
                        lnode2 = nPos;
                    }
                    else if (X1 > dpt2.X + polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                }
                if ((lnode1 >= 0) && (lnode2 >= 0) && (rnode1 >= 0) && (rnode2 >= 0)) {
                    capNodes.push_back(lnode1);
                    capNodes.push_back(lnode2);
                    ocapNodes.push_back(rnode1);
                    ocapNodes.push_back(rnode2);
                    bValid = true;
                    continue;
                }
                break;
            }
        }
    }
    else if (borderCode == 2) {
        for (int k = 0; k < cornerCount; k += 2) {
            DOUBLE*XYs=points2->GetXYs();
            DOUBLE sX = corner->GetCornerPoint(k).X;
            DOUBLE sY2 = ocorner->GetCornerPoint(0).Y;
            int fromPos1 = GetCornerFromNode(tg, corner,k);
            int fromPos2 = GetNeigborCornerFromNode(other, corner,k);
            if (fromPos2 == -1) continue;
            Point2D<double>dpt1 = corner->GetCornerPoint(k);
            Point2D<double>dpt2 = corner->GetCornerPoint(k + 1);
            double dpt1Y = dpt1.Y;
            double dpt2Y = dpt2.Y;
            bool bValid = true;
            while (bValid) {
                bValid = false;
                LONG lnode1 = -1, lnode2 = -1;
                LONG rnode1 = -1, rnode2 = -1;
                XYs=points2->GetXYs();
                bool isOut = false;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY2) <= polyToler) {
                        isOut = true;
                    }
                    if (fabs(X1 - sX) > polyToler) {
                        if (isOut) break;
                        continue;
                    }
                    if ((Y1 <= dpt1Y + polyToler) && (Y1 >= dpt2Y - polyToler)) {
                        rnode1 = nPos;
                        fromPos2 = nPos - 1;
                        dpt1.Y = Y1;
                        if (fromPos2 < 0) fromPos2 += ptsCount2;
                        break;
                    }
                }
                if (rnode1 == -1) break;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                    if ((Y1 <= dpt1Y + polyToler) && (Y1 >= dpt2Y - polyToler)) {
                        dpt2.Y = Y1;
                        rnode2 = nPos;
                    }
                    if (Y1 < dpt2Y - polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                }
                if (rnode2 == -1) break;
                XYs=points1->GetXYs();
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        break;
                    }
                    if (fabs(Y1 - dpt1.Y) <= polyToler) {
                        lnode1 = nPos;
                        fromPos1 = nPos + 1;
                        if (fromPos1 >= ptsCount1) fromPos1 -= ptsCount1;
                        break;
                    }
                }
                if (lnode1 == -1) break;
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX) > polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                    if (fabs(Y1 - dpt2.Y) <= polyToler) {
                        lnode2 = nPos;
                    }
                    else if (Y1 < dpt2.Y - polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                }
                if ((lnode1 >= 0) && (lnode2 >= 0) && (rnode1 >= 0) && (rnode2 >= 0)) {
                    capNodes.push_back(lnode1);
                    capNodes.push_back(lnode2);
                    ocapNodes.push_back(rnode1);
                    ocapNodes.push_back(rnode2);
                    bValid = true;
                    continue;
                }
                break;
            }
        }
    }
    else if (borderCode == 3) {
        for (int k = 0; k < cornerCount; k += 2) {
            DOUBLE*XYs=points2->GetXYs();
            DOUBLE sY = corner->GetCornerPoint(k).Y;
            DOUBLE sX2 = ocorner->GetCornerPoint(0).X;
            int fromPos1 = GetCornerFromNode(tg, corner,k);
            int fromPos2 = GetNeigborCornerFromNode(other, corner,k);
            if (fromPos2 == -1) continue;
            Point2D<double>dpt1 = corner->GetCornerPoint(k);
            Point2D<double>dpt2 = corner->GetCornerPoint(k + 1);
            double dpt1X = dpt1.X;
            double dpt2X = dpt2.X;
            bool bValid = true;
            while (bValid) {
                bValid = false;
                LONG lnode1 = -1, lnode2 = -1;
                LONG rnode1 = -1, rnode2 = -1;
                XYs=points2->GetXYs();
                bool isOut = false;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(X1 - sX2) <= polyToler) {
                        isOut = true;
                    }
                    if (fabs(Y1 - sY) > polyToler) {
                        if (isOut) break;
                        continue;
                    }
                    if ((X1 <= dpt1X + polyToler) && (X1 >= dpt2X - polyToler)) {
                        rnode1 = nPos;
                        fromPos2 = nPos - 1;
                        dpt1.X = X1;
                        if (fromPos2 < 0) fromPos2 += ptsCount2;
                        break;
                    }
                }
                if (rnode1 == -1) break;
                for (int p = 0; p < ptsCount2; p++) {
                    int nPos = fromPos2 - p;
                    if (nPos < 0) nPos += ptsCount2;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                    if ((X1 <= dpt1X + polyToler) && (X1 >= dpt2X - polyToler)) {
                        dpt2.X = X1;
                        rnode2 = nPos;
                    }
                    if (X1 < dpt2X - polyToler) {
                        fromPos2 = nPos;
                        break;
                    }
                }
                if (rnode2 == -1) break;
                XYs=points1->GetXYs();
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        break;
                    }
                    if (fabs(X1 - dpt1.X) <= polyToler) {
                        lnode1 = nPos;
                        fromPos1 = nPos + 1;
                        if (fromPos1 >= ptsCount1) fromPos1 -= ptsCount1;
                        break;
                    }
                }
                if (lnode1 == -1) break;
                for (int p = 0; p < ptsCount1; p++) {
                    int nPos = fromPos1 + p;
                    if (nPos >= ptsCount1) nPos -= ptsCount1;
                    X1 = XYs[(nPos << 1)];
                    Y1 = XYs[(nPos << 1) + 1];
                    if (fabs(Y1 - sY) > polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                    if (fabs(X1 - dpt2.X) <= polyToler) {
                        lnode2 = nPos;
                    }
                    else if (X1 < dpt2.Y - polyToler) {
                        fromPos1 = nPos;
                        break;
                    }
                }
                if ((lnode1 >= 0) && (lnode2 >= 0) && (rnode1 >= 0) && (rnode2 >= 0)) {
                    capNodes.push_back(lnode1);
                    capNodes.push_back(lnode2);
                    ocapNodes.push_back(rnode1);
                    ocapNodes.push_back(rnode2);
                    bValid = true;
                    continue;
                }
                break;
            }
        }
    }
    if (capNodes.size() == 0) return false;
    //现在开始拼接相邻面。第一个顶点到最后一个顶点的面为外环，其余为内环
    AutoPtr<Polygon>newPoly(new Polygon());
    AutoPtr<Points>outerPoints(new Points());
    int attCount = capNodes.size();
    int fromNode = capNodes[attCount - 1];
    int toNode = capNodes[0];
    if (toNode < fromNode) toNode += ptsCount1;
    for (int k = fromNode; k <= toNode; k++) {
        int nPos = k;
        if (nPos >= ptsCount1) nPos -= ptsCount1;
        DOUBLE X, Y;
        points1->GetItem(nPos, X, Y);
        outerPoints->Add(X, Y);
    }
    fromNode = ocapNodes[0];
    toNode = ocapNodes[attCount - 1];
    if (toNode < fromNode) toNode += ptsCount2;
    for (int k = fromNode; k <= toNode; k++) {
        int nPos = k;
        if (nPos >= ptsCount2) nPos -= ptsCount2;
        DOUBLE X, Y;
        points2->GetItem(nPos, X, Y);
        outerPoints->Add(X, Y);
    }
    AutoPtr<Ring>outerRing(outerPoints);
    newPoly->Add(outerRing);
    for (int p = 1; p < attCount - 1; p += 2) {
        AutoPtr<Points>innerPoints(new Points());
        int fromNode = capNodes[p];
        int toNode = capNodes[p + 1];
        if (toNode < fromNode) toNode += ptsCount1;
        for (int k = fromNode; k <= toNode; k++) {
            int nPos = k;
            if (nPos >= ptsCount1) nPos -= ptsCount1;
            DOUBLE X, Y;
            points1->GetItem(nPos, X, Y);
            innerPoints->Add(X, Y);
        }
        fromNode = ocapNodes[p + 1];
        toNode = ocapNodes[p];
        if (toNode < fromNode) toNode += ptsCount2;
        for (int k = fromNode; k <= toNode; k++) {
            int nPos = k;
            if (nPos >= ptsCount2) nPos -= ptsCount2;
            DOUBLE X, Y;
            points2->GetItem(nPos, X, Y);
            innerPoints->Add(X, Y);
        }
        AutoPtr<Ring>innerRing(innerPoints);
        newPoly->Add(innerRing);
    }
    //将原两侧的多边形的内环添加至新的拼接多边形中
    LONG partsCount=poly1->GetSize();
    for (int k = 1; k < partsCount; k++) {
        AutoPtr<Ring>pRing=poly1->GetItem(k);
        newPoly->Add(pRing);
    }
    partsCount=poly2->GetSize();
    for (int k = 1; k < partsCount; k++) {
        AutoPtr<Ring>pRing=poly2->GetItem(k);
        newPoly->Add(pRing);
    }
    //现在新的几何体生成了，更新dg，同时将邻域的边界未接边的corner添加到当前dg,同时删除邻域other
    tg->SetGeometry(newPoly);
    //other->RemoveCorner(ocorner);
    tg->CombineCorner(other);
    this->RemoveGeometry(other);
    corner->LabelJoined();
    return true;
}

bool TileGeometryGrid::CaptureNode(Polyline*poly, double X, double Y, int&partIndex, int&nodeIndex) {
    LONG partCount=poly->GetSize();
    partIndex = -1;
    nodeIndex = -1;
    double mindist = 0;
    for (int k = 0; k < partCount; k++) {
        AutoPtr<Path>pPath=poly->GetItem(k);
        AutoPtr<Points>pts(pPath);
        LONG ptCount=pts->GetSize();
        if (ptCount < 2) continue;
        DOUBLE sX, sY;
        pts->GetItem(0, sX, sY);
        double dist = sqrt((sX - X)*(sX - X) + (sY - Y)*(sY - Y));
        if (partIndex == -1) {
            partIndex = k;
            nodeIndex = 0;
            mindist = dist;
        }
        else if (dist < mindist) {
            partIndex = k;
            nodeIndex = 0;
            mindist = dist;
        }
        pts->GetItem(ptCount - 1, sX, sY);
        dist = sqrt((sX - X)*(sX - X) + (sY - Y)*(sY - Y));
        if (partIndex == -1) {
            partIndex = k;
            nodeIndex = ptCount - 1;
            mindist = dist;
        }
        else if (dist < mindist) {
            partIndex = k;
            nodeIndex = ptCount - 1;
            mindist = dist;
        }
    }
    return (mindist < lineToler);
}

bool TileGeometryGrid::UnionPolyline(TileGeometry*tg, TileCorner*corner, TileGeometry*other) {
    //获取边对应的领域Cell
    int neightX = corner->GetLineNeighborCellX();
    int neightY = corner->GetLineNeighborCellY();
    BYTE bc = corner->GetLineNeighborCode();
    TileCorner*ocorner = other->GetCorner(neightX, neightY, bc);
    if (ocorner == NULL) return false;
    AutoPtr<Geometry>geo1=tg->GetGeometry();
    AutoPtr<Geometry>geo2=other->GetGeometry();
    AutoPtr<Polyline>poly1(geo1);
    AutoPtr<Polyline>poly2(geo2);
    int cornerCount = corner->GetCount();
    int ocornerCount = ocorner->GetCount();
    bool hasUnion = false;
    for (int k = 0; k < cornerCount; k++) {
        Point2D<double>dpt = corner->GetCornerPoint(k);
        int partIndex1, nodeIndex1;
        if (!CaptureNode(poly1.get(), dpt.X, dpt.Y, partIndex1, nodeIndex1)) continue;
        LONG nPos = -1;
        double minDist = 0;
        for (int j = 0; j < ocornerCount; j++) {
            Point2D<double>dpt2 = ocorner->GetCornerPoint(j);
            double dist = sqrt((dpt.X - dpt2.X)*(dpt.X - dpt2.X) + (dpt.Y - dpt2.Y)*(dpt.Y - dpt2.Y));
            if (nPos == -1) {
                nPos = j;
                minDist = dist;
            }
            else if (dist < minDist) {
                nPos = j;
                minDist = dist;
            }
        }
        if (nPos == -1) continue;
        if (minDist > lineToler) continue;
        int partIndex2, nodeIndex2;
        if (!CaptureNode(poly2.get(), dpt.X, dpt.Y, partIndex2, nodeIndex2)) continue;
        AutoPtr<Path>path1=poly1->GetItem(partIndex1);
        AutoPtr<Path>path2=poly2->GetItem(partIndex2);
        AutoPtr<Points>pts1(path1);
        AutoPtr<Points>pts2(path2);
        LONG pts2Count=pts2->GetSize();
        if (nodeIndex1 == 0) {
            if (nodeIndex2 == 0) {
                for (int j = 1; j < pts2Count; j++) {
                    DOUBLE x, y;
                    pts2->GetItem(j, x, y);
                    pts1->Insert(0, x, y);
                }
            }
            else {
                for (int j = pts2Count-2; j >=0; j--) {
                    DOUBLE x, y;
                    pts2->GetItem(j, x, y);
                    pts1->Insert(0, x, y);
                }
            }
        }
        else {
            if (nodeIndex2 == 0) {
                for (int j = 1; j < pts2Count; j++) {
                    DOUBLE x, y;
                    pts2->GetItem(j, x, y);
                    pts1->Add(x, y);
                }
            }
            else {
                for (int j = pts2Count - 2; j >= 0; j--) {
                    DOUBLE x, y;
                    pts2->GetItem(j, x, y);
                    pts1->Add(x, y);
                }
            }
        }
        hasUnion = true;
        break;
    }
    if (!hasUnion) return false;
    tg->RemoveCorner(corner);
    other->RemoveCorner(ocorner);
    tg->CombineCorner(other);
    this->RemoveGeometry(other);
    return true;
}
void TileGeometryGrid::RegisterGeometry(TileGeometry*dg, int tileX, int tileY) {
    AutoPtr<Geometry>geo=dg->GetGeometry();
    tgs->AddGeometry(dg);
    TileGeometryCell*cell = &cells[tileX + tileY * tileWidth];
    AutoPtr<Point>ppt(geo);
    if(ppt!=nullptr){
        cell->RegisterTileGeometry(dg);
        dg->RegisterCell(cell);
        return;
    }
    AutoPtr<Polyline>pln(geo);
    if (pln != nullptr)
        RegisterPolyline(dg, tileX, tileY);
    else
        RegisterPolygon(dg, tileX, tileY);
    cell->RegisterTileGeometry(dg);
    dg->RegisterCell(cell);
    if (pln != nullptr) {
        int subType = dg->GetSubType();
        int cornerCount = dg->GetCornerCount();
        for (int k = 0; k < cornerCount; k++) {
            TileCorner*corner = dg->GetCorner(k);
            int neightX = corner->GetLineNeighborCellX();
            int neightY = corner->GetLineNeighborCellY();
            if ((neightX < 0) || (neightX >= tileWidth) || (neightY < 0) || (neightY >= tileHeight)) continue;
            TileGeometryCell*cell = &cells[neightX + tileWidth * neightY];
            int geoCount = cell->GetSize();
            for (int p = 0; p < geoCount; p++) {
                TileGeometry*ntg = cell->GetItem(p);
                if ((ntg->GetSubType() != subType) || (ntg->GetCornerCount() == 0)) continue;
                if (dg == ntg)
                    dg->RemoveCorner(corner);
                else
                    UnionPolyline(dg, corner, ntg);
            }
        }
    }
    else
    {
        int subType = dg->GetSubType();
        int cornerCount = dg->GetCornerCount();
        for (int k = 0; k < cornerCount; k++) {
            TileCorner*corner = dg->GetCorner(k);
            int neightX = corner->GetNeighborCellX();
            int neightY = corner->GetNeighborCellY();
            if ((neightX < 0) || (neightX >= tileWidth) || (neightY < 0) || (neightY >= tileHeight)) continue;
            TileGeometryCell*cell = &cells[neightX + tileWidth * neightY];
            int geoCount = cell->GetSize();
            for (int p = geoCount-1; p >=0; p--) {
                TileGeometry*ntg = cell->GetItem(p);
                if ((ntg->GetSubType() != subType)||(ntg->GetCornerCount()==0)) continue;
                if (dg == ntg)
                {
                    UnionPolygon(dg, corner);
                }
                else
                {
                    UnionPolygon(dg, corner, ntg);
                }
            }
        }
    }
}

}
