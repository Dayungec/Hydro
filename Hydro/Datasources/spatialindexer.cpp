#include "spatialindexer.h"

namespace SGIS{

SegmentRegionsComputer::SegmentRegionsComputer()
{
    this->left=0;
    this->top=0;
    this->cellSizeX=0;
    this->cellSizeY=0;
    this->xNum=0;
    this->yNum=0;
}

SegmentRegionsComputer::~SegmentRegionsComputer(){

}

void SegmentRegionsComputer::Initialize(double left,double top,double cellSizeX,double cellSizeY,int xNum,int yNum){
    this->left=left;
    this->top=top;
    this->cellSizeX=cellSizeX;
    this->cellSizeY=cellSizeY;
    this->xNum=xNum;
    this->yNum=yNum;
}

int SegmentRegionsComputer::GetRegionX(double x)
{
    int xid=(x-left)/cellSizeX;
    if(xid<0) return 0;
    if(xid>=xNum) xid=xNum-1;
    return xid;
}
int SegmentRegionsComputer::GetRegionY(double y)
{
    int yid=(top-y)/cellSizeY;
    if(yid<0) return 0;
    if(yid>=yNum) yid=yNum-1;
    return yid;
}
int SegmentRegionsComputer::GetRealRegionX(double x)
{
    int xid=(x-left)/cellSizeX;
    return xid;
}
int SegmentRegionsComputer::GetRealRegionY(double y)
{
    int yid=(top-y)/cellSizeY;
    return yid;
}

double SegmentRegionsComputer::GetCellSizeX(){
    return cellSizeX;
}
double SegmentRegionsComputer::GetCellSizeY(){
    return cellSizeY;
}
Point2D<double>SegmentRegionsComputer::GetLeftTop(){
    return Point2D<double>(left,top);
}
void SegmentRegionsComputer::AppendToArray(AutoArray<Point2D<int>>&pArray,int ix,int iy){
    if((ix<0)||(ix>=xNum)||(iy<0)||(iy>=yNum)) return;
    if(pArray.GetSize()>0){
        Point2D<int>former=pArray[pArray.GetSize()-1];
        if((former.X==ix)&&(former.Y==iy)) return;
    }
    pArray.Add(Point2D<int>(ix,iy));
}

void SegmentRegionsComputer::innerGetSegmentRegions(const Segment2D<double>&seg,AutoArray<Point2D<int>>&pArray){
    double x,y,x1,y1,x2,y2;
    x1=seg.lnx1;
    y1=seg.lny1;
    x2=seg.lnx2;
    y2=seg.lny2;
    int ix1=GetRealRegionX(x1);
    int iy1=GetRealRegionY(y1);
    int ix2=GetRealRegionX(x2);
    int iy2=GetRealRegionY(y2);

    double lx,ly,cx,cy,nx,ny;
    double ilx,ily,icx,icy,inx,iny;
    bool IsCorner;
    if(abs(ix1-ix2)>=abs(iy1-iy2))
    {
        /*if(ix1>ix2)
        {
            int tempi=ix1;ix1=ix2;ix2=tempi;
            tempi=iy1;iy1=iy2;iy2=tempi;
            double tempf=x1;x1=x2;x2=tempf;
            tempf=y1;y1=y2;y2=tempf;
        }*/
        if(ix1<=ix2){
            double k=(y2-y1)/(x2-x1);
            lx=x1;ly=y1;
            cx=left+cellSizeX*(ix1+1);
            cy=k*(cx-x1)+y1;
            nx=cx+cellSizeX/2.0;
            ny=k*(nx-x1)+y1;
            ilx=GetRealRegionX(lx);
            ily=GetRealRegionY(ly);
            icx=GetRealRegionX(cx);
            icy=GetRealRegionY(cy);
            inx=GetRealRegionX(nx);
            iny=GetRealRegionY(ny);
            AppendToArray(pArray,ilx,ily);
            for(int p=ix1+1;p<=ix2+1;p++)
            {
                IsCorner=(fabs(top-icy*cellSizeY-cy)<=VertexTolerance);
                if(IsCorner){
                    if(k<0){
                        AppendToArray(pArray,ilx,ily+1);
                        AppendToArray(pArray,ilx+1,ily);
                    }
                    else if(k>0){
                        AppendToArray(pArray,ilx,ily-1);
                        AppendToArray(pArray,ilx+1,ily);
                    }
                    else{
                        AppendToArray(pArray,ilx,ily-1);
                        AppendToArray(pArray,ilx+1,ily-1);
                    }
                }
                else{
                    if(icy!=ily){
                        AppendToArray(pArray,ilx,icy);
                    }
                    if(iny!=icy){
                        AppendToArray(pArray,ilx+1,icy);
                    }
                }
                AppendToArray(pArray,inx,iny);
                ilx=inx;
                ily=iny;
                lx=nx;
                ly=ny;
                cx=left+cellSizeX*(p+1);
                cy=k*(cx-x1)+y1;
                nx=cx+cellSizeX/2.0;
                ny=k*(nx-x1)+y1;
                icx=GetRealRegionX(cx);
                icy=GetRealRegionY(cy);
                inx=GetRealRegionX(nx);
                iny=GetRealRegionY(ny);
            }
        }
        else{
            double k=(y2-y1)/(x2-x1);
            lx=x1;ly=y1;
            cx=left+cellSizeX*ix1;
            cy=k*(cx-x1)+y1;
            nx=cx-cellSizeX/2.0;
            ny=k*(nx-x1)+y1;
            ilx=GetRealRegionX(lx);
            ily=GetRealRegionY(ly);
            icx=GetRealRegionX(cx);
            icy=GetRealRegionY(cy);
            inx=GetRealRegionX(nx);
            iny=GetRealRegionY(ny);
            AppendToArray(pArray,ilx,ily);
            for(int p=ix1-1;p>=ix2-1;p--)
            {
                IsCorner=(fabs(top-icy*cellSizeY-cy)<=VertexTolerance);
                if(IsCorner){
                    if(k<0){
                        AppendToArray(pArray,ilx,ily-1);
                        AppendToArray(pArray,ilx-1,ily);
                    }
                    else if(k>0){
                        AppendToArray(pArray,ilx,ily+1);
                        AppendToArray(pArray,ilx-1,ily);
                    }
                    else{
                        AppendToArray(pArray,ilx,ily-1);
                        AppendToArray(pArray,ilx-1,ily-1);
                    }
                }
                else{
                    if(icy!=ily){
                        AppendToArray(pArray,ilx,icy);
                    }
                    if(iny!=icy){
                        AppendToArray(pArray,ilx-1,icy);
                    }
                }
                AppendToArray(pArray,inx,iny);
                ilx=inx;
                ily=iny;
                lx=nx;
                ly=ny;
                cx=left+cellSizeX*p;
                cy=k*(cx-x1)+y1;
                nx=cx-cellSizeX/2.0;
                ny=k*(nx-x1)+y1;
                icx=GetRealRegionX(cx);
                icy=GetRealRegionY(cy);
                inx=GetRealRegionX(nx);
                iny=GetRealRegionY(ny);
            }
        }
    }
    else{
        /*
        if(iy1>iy2)
        {
            int tempi=ix1;ix1=ix2;ix2=tempi;
            tempi=iy1;iy1=iy2;iy2=tempi;
            double tempf=x1;x1=x2;x2=tempf;
            tempf=y1;y1=y2;y2=tempf;
        }*/
        if(iy1<=iy2){
            double k=(x2-x1)/(y2-y1);
            lx=x1;ly=y1;
            cy=top-cellSizeY*(iy1+1);
            cx=k*(cy-y1)+x1;
            ny=cy-cellSizeY/2.0;
            nx=k*(ny-y1)+x1;
            ilx=GetRealRegionX(lx);
            ily=GetRealRegionY(ly);
            icx=GetRealRegionX(cx);
            icy=GetRealRegionY(cy);
            inx=GetRealRegionX(nx);
            iny=GetRealRegionY(ny);
            AppendToArray(pArray,ilx,ily);
            for(int p=iy1+1;p<=iy2+1;p++)
            {
                IsCorner=(fabs(left+icx*cellSizeX-cx)<=VertexTolerance);
                if(IsCorner){
                    if(k<0){
                       AppendToArray(pArray,ilx+1,ily);
                       AppendToArray(pArray,ilx,ily+1);
                    }
                    else if(k>0){
                        AppendToArray(pArray,ilx-1,ily);
                        AppendToArray(pArray,ilx,ily+1);
                    }
                    else{
                        AppendToArray(pArray,ilx-1,ily);
                        AppendToArray(pArray,ilx-1,ily+1);
                    }
                }
                else{
                    if(icx!=ilx){
                        AppendToArray(pArray,icx,ily);
                    }
                    if(inx!=icx){
                        AppendToArray(pArray,icx,ily+1);
                    }
                }
                AppendToArray(pArray,inx,iny);
                ilx=inx;
                ily=iny;
                lx=nx;
                ly=ny;
                cy=top-cellSizeY*(p+1);
                cx=k*(cy-y1)+x1;
                ny=cy-cellSizeY/2.0;
                nx=k*(ny-y1)+x1;
                icx=GetRealRegionX(cx);
                icy=GetRealRegionY(cy);
                inx=GetRealRegionX(nx);
                iny=GetRealRegionY(ny);
            }
        }else{
            double k=(x2-x1)/(y2-y1);
            lx=x1;ly=y1;
            cy=top-cellSizeY*iy1;
            cx=k*(cy-y1)+x1;
            ny=cy+cellSizeY/2.0;
            nx=k*(ny-y1)+x1;
            ilx=GetRealRegionX(lx);
            ily=GetRealRegionY(ly);
            icx=GetRealRegionX(cx);
            icy=GetRealRegionY(cy);
            inx=GetRealRegionX(nx);
            iny=GetRealRegionY(ny);
            AppendToArray(pArray,ilx,ily);
            for(int p=iy1-1;p>=iy2-1;p--)
            {
                IsCorner=(fabs(left+icx*cellSizeX-cx)<=VertexTolerance);
                if(IsCorner){
                    if(k<0){
                        AppendToArray(pArray,ilx-1,ily);
                        AppendToArray(pArray,ilx,ily-1);
                    }
                    else if(k>0){
                        AppendToArray(pArray,ilx+1,ily);
                        AppendToArray(pArray,ilx,ily-1);
                    }
                    else{
                        AppendToArray(pArray,ilx-1,ily);
                        AppendToArray(pArray,ilx-1,ily-1);
                    }
                }
                else{
                    if(icx!=ilx){
                        AppendToArray(pArray,icx,ily);
                    }
                    if(inx!=icx){
                        AppendToArray(pArray,icx,ily-1);
                    }
                }
                AppendToArray(pArray,inx,iny);
                ilx=inx;
                ily=iny;
                lx=nx;
                ly=ny;
                cy=top-cellSizeY*p;
                cx=k*(cy-y1)+x1;
                ny=cy+cellSizeY/2.0;
                nx=k*(ny-y1)+x1;
                icx=GetRealRegionX(cx);
                icy=GetRealRegionY(cy);
                inx=GetRealRegionX(nx);
                iny=GetRealRegionY(ny);
            }
        }
    }
}
void SegmentRegionsComputer::GetSegmentRegions(const Segment2D<double>&seg,AutoArray<Point2D<int>>&pArray){
    pArray.Clear();
    innerGetSegmentRegions(seg,pArray);
}

void SegmentRegionsComputer::GetSegmentRegions(Path*path,AutoArray<Point2D<int>>&pArray){
    pArray.Clear();
    Points*pts=(Points*)path;
    LONG count=pts->GetSize();
    if(count<2) return;
    Segment2D<double>seg;
    pts->GetItem(0,seg.lnx1,seg.lny1);
    for(int k=1;k<count;k++){
        pts->GetItem(k,seg.lnx2,seg.lny2);
        innerGetSegmentRegions(seg,pArray);
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
}

DistanceArray::DistanceArray(){

}

DistanceArray::~DistanceArray() {

}

int DistanceArray::GetSize() {
    return indices.size();
}

double OrderDistanceArray::GetDistance(int nIndex) {
    return dists[nIndex];
}

void OrderDistanceArray::GetFeatureAndDistance(int nIndex,int&featureIndex,double&dist){
    featureIndex=indices[nIndex];
    dist=dists[nIndex];
}

float OrderDistanceArray::GetIDWValue(int nearcount,int power,vector<double>&fValues){
    float outv=0;
    float sumd=0;
    float v=0;
    for(int p=0;p<nearcount;p++){
        if(dists[p]==0){
            outv=fValues[indices[p]];
            sumd=1.0;
            break;
        }
        v=1/pow(dists[p],2);
        sumd+=v;
        outv+=fValues[indices[p]]*v;
    }
    return outv/=sumd;
}

int DistanceArray::GetFeatureIndex(int nIndex) {
    return indices[nIndex];
}

void DistanceArray::Trim(int count) {
    if(indices.size()<=count) return;
    indices.resize(count);
}

void DistanceArray::TrimToOther(int count, DistanceArray*other) {
    if (indices.size() < count) count = indices.size();
    other->indices.resize(count);
    memcpy(other->indices.data(), indices.data(), sizeof(int)*count);
}

void DistanceArray::Clear() {
    indices.clear();
}

bool DistanceArray::IsSameAs(DistanceArray*other) {
    int size=indices.size();
    if (other->indices.size() != size) return false;
    OrderArray<int>odis;
    for(int k=0;k<size;k++){
        odis.Add(other->GetFeatureIndex(k));
    }
    for (int k = indices.size() - 1; k >= 0; k--) {
        if(odis.Find(indices[k])==-1) return false;
    }
    return true;
}

void DistanceArray::CopyFrom(DistanceArray*other) {
    this->Clear();
    int count = other->GetSize();
    for (int k = 0; k < count; k++) {
        Add(other->GetFeatureIndex(k), other->GetDistance(k));
    }
}

OrderDistanceArray::OrderDistanceArray()
    :dists(true) {

}
OrderDistanceArray::~OrderDistanceArray() {

}

void OrderDistanceArray::Add(int index, double dist) {
    int idx = dists.Add(dist);
    indices.insert(begin(indices) + idx, index);
}

void OrderDistanceArray::Trim(int count) {
    DistanceArray::Trim(count);
    dists.Trim(count);
}


void OrderDistanceArray::Clear() {
    DistanceArray::Clear();
    dists.Clear();
}

void OrderDistanceArray::TrimToOther(int count, DistanceArray*other) {
    AutoPtr<IndexDistanceArray>iother=AutoPtr<DistanceArray>(other,true);
    if(iother!=nullptr){
        int pcount=this->GetSize();
        if(count<pcount) count=pcount;
        iother->Clear();
        for(int k=0;k<pcount;k++){
            iother->Add(indices[k], dists[k]);
        }
        return;
    }
    AutoPtr<OrderDistanceArray>oother=AutoPtr<DistanceArray>(other,true);
    if(oother!=nullptr){
        DistanceArray::TrimToOther(count, other);
        dists.TrimToOther(count, oother->dists);
        return;
    }
    AutoPtr<MirrorDistanceArray>mother=AutoPtr<DistanceArray>(other,true);
    if(mother!=nullptr){
        DistanceArray::TrimToOther(count, other);
        dists.CopyTo(count,mother->dists);
    }
}

IndexDistanceArray::IndexDistanceArray(){

}

IndexDistanceArray::~IndexDistanceArray(){

}

void IndexDistanceArray::Add(int index,double dist){
    int idx=dists.Add(dist);
    indices.insert(begin(indices)+idx,index);
    iis.Add(index);
}

void IndexDistanceArray::Trim(int count){
    OrderDistanceArray::Trim(count);
    iis.Clear();
}

void IndexDistanceArray::TrimToOther(int count, DistanceArray*other) {
    AutoPtr<IndexDistanceArray>iother=AutoPtr<DistanceArray>(other,true);
    if(iother!=nullptr){
        int pcount=this->GetSize();
        if(count<pcount) count=pcount;
        iother->Clear();
        for(int k=0;k<pcount;k++){
            iother->Add(indices[k], dists[k]);
        }
        return;
    }
    AutoPtr<OrderDistanceArray>oother=AutoPtr<DistanceArray>(other,true);
    if(oother!=nullptr){
        DistanceArray::TrimToOther(count, other);
        dists.TrimToOther(count, oother->dists);
        return;
    }
    AutoPtr<MirrorDistanceArray>mother=AutoPtr<DistanceArray>(other,true);
    if(mother!=nullptr){
        DistanceArray::TrimToOther(count, other);
        dists.CopyTo(count,mother->dists);
    }
}

void IndexDistanceArray::Clear(){
    OrderDistanceArray::Clear();
    iis.Clear();
}

bool IndexDistanceArray::HasFeature(int featureIndex){
    return (iis.Find(featureIndex)>=0);
}

MirrorDistanceArray::MirrorDistanceArray() {

}
MirrorDistanceArray::~MirrorDistanceArray() {

}

void MirrorDistanceArray::Add(int index, double dist) {
    dists.push_back(dist);
    indices.push_back(index);
}

double MirrorDistanceArray::GetDistance(int nIndex) {
    return dists[nIndex];
}

void MirrorDistanceArray::GetFeatureAndDistance(int nIndex,int&featureIndex,double&dist){
    featureIndex=indices[nIndex];
    dist=dists[nIndex];
}


float MirrorDistanceArray::GetIDWValue(int nearcount,int power,vector<double>&fValues){
    float outv=0;
    float sumd=0;
    float v=0;
    for(int p=0;p<nearcount;p++){
        v=1/pow(dists[p],2);
        sumd+=v;
        outv+=fValues[indices[p]]*v;
    }
    return outv/=sumd;
}


void MirrorDistanceArray::Clear() {
    DistanceArray::Clear();
    dists.clear();
}

void MirrorDistanceArray::FromDistanceArray(LocationSpatialIndexer*sind,OrderDistanceArray*distArray,double centerX,double centerY,int ptNum) {
    int Num = distArray->GetSize();
    dists.resize(Num);
    indices.resize(Num);
    this->ptNum=ptNum;
    for (int k = 0; k < Num; k++) {
        dists[k] = distArray->dists[k];
        indices[k] = distArray->indices[k];
    }
    this->refPos.X=centerX;
    this->refPos.Y=centerY;
    if (ptNum < Num){
       freeDist=(distArray->dists[ptNum] - distArray->dists[ptNum-1]) / 2;
    }
}

void MirrorDistanceArray::MoveCenter(LocationSpatialIndexer*sind,double centerX,double centerY) {
    int Num = indices.size();
    bool reset = false;
    if (ptNum < Num) {
        if(refPos.Y==refPos.Y){
            if(fabs(centerX-refPos.X)>=freeDist) reset = true;
        }
        else if(sqrt((refPos.X - centerX)*(refPos.X - centerX) + (refPos.Y - centerY)*(refPos.Y - centerY))>=freeDist)
            reset = true;
    }
    if (reset) {
        tempArray.Clear();
        Point2D<double> fpt;
        for (int k = 0; k < Num; k++) {
            int fIndex = indices[k];
            sind->GetFeaturePoint(fIndex,fpt);
            double dist = sqrt((centerX - fpt.X)*(centerX - fpt.X) + (centerY - fpt.Y)*(centerY - fpt.Y));
            tempArray.Add(fIndex, dist);
        }
        FromDistanceArray(sind,&tempArray, centerX,centerY,ptNum);
    }
    else {
        if (ptNum > Num) ptNum = Num;
        Point2D<double>fpt ;
        for (int k = 0; k < ptNum; k++) {
            sind->GetFeaturePoint(indices[k],fpt);
            dists[k] = sqrt((centerX - fpt.X)*(centerX - fpt.X) + (centerY - fpt.Y)*(centerY - fpt.Y));
        }
    }
}

void MirrorDistanceArray::Trim(int count) {
    DistanceArray::Trim(count);
    if (dists.size() > count) {
        dists.resize(count);
    }
}

void MirrorDistanceArray::TrimToOther(int count, DistanceArray*other) {
    AutoPtr<IndexDistanceArray>iother=AutoPtr<DistanceArray>(other,true);
    if(iother!=nullptr){
        int pcount=this->GetSize();
        if(count<pcount) count=pcount;
        iother->Clear();
        for(int k=0;k<pcount;k++){
            iother->Add(indices[k], dists[k]);
        }
        return;
    }
    AutoPtr<OrderDistanceArray>oother=AutoPtr<DistanceArray>(other,true);
    if(oother!=nullptr){
        DistanceArray::TrimToOther(count, other);
        int pcount=this->GetSize();
        if(count<pcount) count=pcount;
        oother->dists.elements.resize(count);
        memcpy(oother->dists.elements.data(),this->dists.data(),count*sizeof(double));
        return;
    }
    AutoPtr<MirrorDistanceArray>mother=AutoPtr<DistanceArray>(other,true);
    if(mother!=nullptr){
        DistanceArray::TrimToOther(count, other);
        int pcount=this->GetSize();
        if(count<pcount) count=pcount;
        mother->dists.resize(count);
        memcpy(mother->dists.data(),dists.data(),count*sizeof(double));
    }
}

SpatialCell::SpatialCell(){

}
SpatialCell::~SpatialCell(){

}

void SpatialCell::Add(int findex){
    findices.Add(findex);
}

int SpatialCell::GetSize(){
    return findices.GetSize();
}
int SpatialCell::GetFeatureIndex(int nIndex){
    return findices[nIndex];
}

int SpatialCell::operator[](int nIndex){
    return findices[nIndex];
}

void SpatialCell::Clear(){
    findices.Clear();
}

SpatialIndexer::SpatialIndexer(int maxWidthOrHeight){
   this->maxWidthOrHeight=maxWidthOrHeight;
   if(this->maxWidthOrHeight<=0) this->maxWidthOrHeight=1;
   psp=nullptr;
}

SpatialIndexer::~SpatialIndexer(){
    for(int k=regions.size()-1;k>=0;k--) delete regions[k];
    regions.clear();
}
void SpatialIndexer::Clear(){
    for(int k=regions.size()-1;k>=0;k--) delete regions[k];
    regions.clear();
}

void SpatialIndexer::InitialCells(Envelope*fullExtent){
    Clear();
    if(fullExtent!=nullptr) fullExtent->GetCoord(&left,&top,&right,&bottom);
    if(right-left>=top-bottom){
        xNum=maxWidthOrHeight;
        cellSize=(right-left)/xNum;
        yNum=(top-bottom)/cellSize;
        if(yNum*cellSize<top-bottom) yNum++;
    }else{
        yNum=maxWidthOrHeight;
        cellSize=(top-bottom)/yNum;
        xNum=(top-bottom)/cellSize;
        if(xNum*cellSize<top-bottom) xNum++;
    }
    regions.resize(xNum*yNum);
    for(int k=xNum*yNum-1;k>=0;k--)
    {
        regions[k]=new SpatialCell();
    }
    sgc.Initialize(left,top,cellSize,cellSize,xNum,yNum);
}

int SpatialIndexer::GetRegionX(double x)
{
    int xid=(x-left)/cellSize;
    if(xid<0) return 0;
    if(xid>=xNum) xid=xNum-1;
    return xid;
}
int SpatialIndexer::GetRegionY(double y)
{
    int yid=(top-y)/cellSize;
    if(yid<0) return 0;
    if(yid>=yNum) yid=yNum-1;
    return yid;
}
int SpatialIndexer::GetRealRegionX(double x)
{
    if(x<left) return (x-left)/cellSize-1;
    return (x-left)/cellSize;
}
int SpatialIndexer::GetRealRegionY(double y)
{
    if(top<y) return (top-y)/cellSize-1;
    return (top-y)/cellSize;
}

void SpatialIndexer::RegisterPoint(Point*pt,int fIndex){
    int rx=GetRegionX(pt->X());
    int ry=GetRegionY(pt->Y());
    regions[rx+ry*xNum]->Add(fIndex);
}
void SpatialIndexer::RegisterPath(Path*path,int fIndex){
    Points*points=(Points*)path;
    int count=points->GetSize();
    if(count<2) return;
    Segment2D<double>seg;
    double*XYs=points->GetXYs();
    seg.lnx1=XYs[0];
    seg.lny1=XYs[1];
    int nIndex=2;
    for(int k=1;k<count;k++){
        seg.lnx2=XYs[nIndex++];
        seg.lny2=XYs[nIndex++];
        AutoArray<Point2D<int>>arrys;
        sgc.GetSegmentRegions(seg,arrys);
        for(int p=arrys.GetSize()-1;p>=0;p--){
            Point2D<int>cellPos=arrys[p];
            regions[cellPos.X+cellPos.Y*xNum]->Add(fIndex);
        }
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lnx1;
    }

}

void SpatialIndexer::RegisterPolyline(Polyline*poly,int fIndex){
    int count=poly->GetSize();
    for(int k=0;k<count;k++){
        AutoPtr<Path>path=poly->GetItem(k);
        RegisterPath(path.get(),fIndex);
    }
}

void SpatialIndexer::RegisterEnvelope(Envelope*env,int fIndex){
    double l,t,r,b;
    env->GetCoord(&l,&t,&r,&b);
    int rx1,ry1,rx2,ry2;
    rx1=GetRegionX(l);
    ry1=GetRegionY(t);
    rx2=GetRegionX(r);
    ry2=GetRegionY(b);
    for(int m=ry1;m<=ry2;m++){
        for(int n=rx1;n<=rx2;n++){
            regions[n+m*xNum]->Add(fIndex);
        }
    }
}
void SpatialIndexer::RegisterGeometry(Geometry*geo,int fIndex){
    GeometryType gType=geo->GetType();
    switch(gType){
    case gtPoint:
        RegisterPoint((Point*)geo,fIndex);
        return;
    case gtPolyline:
        RegisterPolyline((Polyline*)geo,fIndex);
        return;
    default:
    {
        AutoPtr<Envelope>env=geo->GetExtent();
        RegisterEnvelope(env.get(),fIndex);
        return;
    }
    }
}

void SpatialIndexer::BuildSpatialIndices(AutoPtr<FeatureClass>pfc,AutoPtr<SpatialReference>psp){
    this->psp=psp;
    AutoPtr<Envelope>fullExtent=pfc->GetExtent();
    canTransform=false;
    if(this->psp!=nullptr){
        AutoPtr<SpatialReference>lypsp=pfc->GetSpatialReference();
        pTrans=new CoordinateTransformation(lypsp,psp);
        pTrans->BeginTransform();
        canTransform=pTrans->CanTransform();
        if(canTransform){
            fullExtent->Project(pTrans.get());
        }
    }
    InitialCells(fullExtent.get());
    this->pfc=pfc;
    int featureCount=pfc->GetFeatureCount();
    VectorShapeType vType=pfc->GetShapeType();
    switch(vType){
    case vstPoint:{
        for(int k=0;k<featureCount;k++){
            AutoPtr<Point>geo=pfc->GetFeature(k);
            if(geo==nullptr) continue;
            if(canTransform) geo->Project(pTrans.get());
            RegisterPoint(geo.get(),k);
        }
        break;
    }
    case vstPolyline:{
        for(int k=0;k<featureCount;k++){
            AutoPtr<Polyline>geo=pfc->GetFeature(k);
            if(geo==nullptr) continue;
            if(canTransform) geo->Project(pTrans.get());
            RegisterPolyline(geo.get(),k);
        }
        break;
    }
    default:{
        AutoPtr<Envelope>env(new Envelope());
        for(int k=0;k<featureCount;k++){
            if(pfc->GetFeatureEnvelope(k,env.get())){
               if(canTransform) env->Project(pTrans.get());
               RegisterEnvelope(env.get(),k);
            }
        }
        break;
    }
    }
}


void SpatialIndexer::BuildSpatialIndicesEx(AutoPtr<FeatureClass>pfc,double mintoler,AutoPtr<SpatialReference>psp){
    this->psp=psp;
    AutoPtr<Envelope>fullExtent=pfc->GetExtent();
    canTransform=false;
    if(this->psp!=nullptr){
        AutoPtr<SpatialReference>lypsp=pfc->GetSpatialReference();
        pTrans=new CoordinateTransformation(lypsp,psp);
        pTrans->BeginTransform();
        canTransform=pTrans->CanTransform();
        if(canTransform){
            fullExtent->Project(pTrans.get());
        }
    }
    InitialCells(fullExtent.get());
    this->pfc=pfc;
    int featureCount=pfc->GetFeatureCount();
    VectorShapeType vType=pfc->GetShapeType();
    switch(vType){
    case vstPoint:{
        for(int k=0;k<featureCount;k++){
            AutoPtr<Point>geo=pfc->GetFeature(k);
            if(geo==nullptr) continue;
            if(canTransform) geo->Project(pTrans.get());
            RegisterPoint(geo.get(),k);
        }
        break;
    }
    case vstPolyline:{
        AutoPtr<Envelope>fEnv(new Envelope());
        for(int k=0;k<featureCount;k++){
            pfc->GetFeatureEnvelope(k,fEnv.get());
            if(canTransform) fEnv->Project(pTrans.get());
            if((fEnv->Width()<mintoler)&&(fEnv->Height()<mintoler)){
                continue;
            }
            AutoPtr<Polyline>geo=pfc->GetFeature(k);
            if(geo==nullptr) continue;
            if(canTransform) geo->Project(pTrans.get());
            RegisterPolyline(geo.get(),k);
        }
        break;
    }
    default:{
        AutoPtr<Envelope>env(new Envelope());
        for(int k=0;k<featureCount;k++){
            if(pfc->GetFeatureEnvelope(k,env.get())){
               if(canTransform) env->Project(pTrans.get());
               if((env->Width()<mintoler)&&(env->Height()<mintoler)){
                   continue;
               }
               RegisterEnvelope(env.get(),k);
            }
        }
        break;
    }
    }
}

void SpatialIndexer::GetFeatures(Envelope*env,OrderArray<int>&pArray){
    double l,t,r,b;
    env->GetCoord(&l,&t,&r,&b);
    int ix1=GetRegionX(l);
    int ix2=GetRegionX(r);
    int iy1=GetRegionY(t);
    int iy2=GetRegionY(b);
    for(int m=iy1;m<=iy2;m++){
        int cellPos=ix1+m*xNum;
        for(int n=ix1;n<=ix2;n++){
            SpatialCell*cell=regions[cellPos++];
            int featureCount=cell->GetSize();
            for(int p=0;p<featureCount;p++){
                int fIndex=cell->GetFeatureIndex(p);
                pArray.Add(fIndex);
            }
        }
    }
}

void SpatialIndexer::SearchNearstFeaturesByRaius(double X,double Y, double toler, IndexDistanceArray &results){
    results.Clear();
    int ix1=GetRegionX(X-toler);
    int ix2=GetRegionX(X+toler);
    int iy1=GetRegionY(Y+toler);
    int iy2=GetRegionY(Y-toler);
    AutoPtr<Point>dpt(new Point());
    dpt->PutCoord(X,Y);
    AutoPtr<Envelope>senv(new Envelope(X-toler,Y+toler,X+toler,Y-toler));
    AutoPtr<Envelope>env(new Envelope);
    for(int m=iy1;m<=iy2;m++){
        int cellPos=ix1+m*xNum;
        for(int n=ix1;n<=ix2;n++){
            SpatialCell*cell=regions[cellPos++];
            int featureCount=cell->GetSize();
            for(int p=0;p<featureCount;p++){
                int fIndex=cell->GetFeatureIndex(p);
                if(results.HasFeature(fIndex)) continue;
                if(!pfc->GetFeatureEnvelope(fIndex,env.get())) continue;
                if(!senv->Intersects(env.get())) continue;
                AutoPtr<Geometry>geo=pfc->GetFeature(fIndex);
                if(canTransform) geo->Project(pTrans.get());
                double dis=dpt->DistanceTo(geo.get());
                if(dis>toler) continue;
                results.Add(fIndex,dis);
            }
        }
    }
}

void SpatialIndexer::SearchNearstFeaturesByNum(double x,double y,int Num,IndexDistanceArray&results){
    AutoPtr<Point>dpt(new Point());
    dpt->PutCoord(x,y);
    float fRadius = cellSize;
    double right = left + xNum * cellSize;
    double bottom = top - yNum * cellSize;
    results.Clear();
    double maxradius = sqrt((left - x)*(left - x) + (top - y)*(top - y));
    maxradius = max(maxradius, sqrt((right - x)*(right - x) + (top - y)*(top - y)));
    maxradius = max(maxradius, sqrt((right - x)*(right - x) + (bottom - y)*(bottom - y)));
    maxradius = max(maxradius, sqrt((left - x)*(left - x) + (bottom - y)*(bottom - y)));
    maxradius+=2*cellSize;
    int ix=GetRealRegionX(x);
    int iy=GetRealRegionY(y);
    double cellleft=left+ix*cellSize;
    double celltop=top-iy*cellSize;
    double cellright=cellleft+cellSize;
    double cellbottom=celltop-cellSize;
    float validRadius=x-cellleft;
    if(validRadius>cellright-x) validRadius=cellright-x;
    if(validRadius>y-cellbottom) validRadius=y-cellbottom;
    if(validRadius>celltop-y) validRadius=celltop-y;
    int d=0;
    if(ix<0)
        d=-ix;
    else if(ix>=xNum)
        d=ix-xNum+1;
    int dy=0;
    if(iy<0)
        dy=-iy;
    else if(iy>=yNum)
        dy=iy-yNum+1;
    if(dy>d) d=dy;
    validRadius+=d*cellSize;
    fRadius=(d+1)*cellSize;
    AutoPtr<Envelope>senv(new Envelope());
    AutoPtr<Envelope>env(new Envelope);
    double semiCellSize=cellSize*0.05;
    while (true) {
        int fx=ix-d;
        int tx=ix+d;
        int fy=iy-d;
        int ty=iy+d;
        if((fx>=0)&&(fx<xNum)){
            senv->Left()=left+cellSize*fx-semiCellSize;
            senv->Right()=senv->Left()+cellSize+semiCellSize;
            for(int k=fy;k<=ty;k++){
                if(k<0){
                   k=0;
                   if(k>ty) break;
                }
                if(k>=yNum) break;
                senv->Top()=top-cellSize*k+semiCellSize;
                senv->Bottom()=senv->Top()-cellSize-semiCellSize;
                SpatialCell*cell=regions[fx+k*xNum];
                int featureCount=cell->GetSize();
                for(int p=0;p<featureCount;p++){
                    int fIndex=cell->GetFeatureIndex(p);
                    if(results.HasFeature(fIndex)) continue;
                    if(!pfc->GetFeatureEnvelope(fIndex,env.get())) continue;
                    if(!senv->Intersects(env.get())) continue;
                    AutoPtr<Geometry>geo=pfc->GetFeature(fIndex);
                    if(canTransform) geo->Project(pTrans.get());
                    double dis=dpt->DistanceTo(geo.get());
                    results.Add(fIndex,dis);
                }
            }
        }
        if(d>0){
            if((tx>=0)&&(tx<xNum)){
                senv->Left()=left+cellSize*tx-semiCellSize;
                senv->Right()=senv->Left()+cellSize+semiCellSize;
                for(int k=fy;k<=ty;k++){
                    if(k<0){
                        k=0;
                        if(k>ty) break;
                    }
                    if(k>=yNum) break;
                    senv->Top()=top-cellSize*k+semiCellSize;
                    senv->Bottom()=senv->Top()-cellSize-semiCellSize;
                    SpatialCell*cell=regions[tx+k*xNum];
                    int featureCount=cell->GetSize();
                    for(int p=0;p<featureCount;p++){
                        int fIndex=cell->GetFeatureIndex(p);
                        if(results.HasFeature(fIndex)) continue;
                        if(!pfc->GetFeatureEnvelope(fIndex,env.get())) continue;
                        if(!senv->Intersects(env.get())) continue;
                        AutoPtr<Geometry>geo=pfc->GetFeature(fIndex);
                        if(canTransform) geo->Project(pTrans.get());
                        double dis=dpt->DistanceTo(geo.get());
                        results.Add(fIndex,dis);
                    }
                }
            }
            if((fy>=0)&&(fy<yNum)){
                senv->Top()=top-cellSize*fy+semiCellSize;
                senv->Bottom()=senv->Top()-cellSize-semiCellSize;
                for(int k=fx+1;k<tx;k++){
                    if(k<0){
                        k=0;
                        if(k>=tx) break;
                    }
                    if(k>=xNum) break;
                    senv->Left()=left+cellSize*k-semiCellSize;
                    senv->Right()=senv->Left()+cellSize+semiCellSize;
                    SpatialCell*cell=regions[k+fy*xNum];
                    int featureCount=cell->GetSize();
                    for(int p=0;p<featureCount;p++){
                        int fIndex=cell->GetFeatureIndex(p);
                        if(results.HasFeature(fIndex)) continue;
                        if(!pfc->GetFeatureEnvelope(fIndex,env.get())) continue;
                        if(!senv->Intersects(env.get())) continue;
                        AutoPtr<Geometry>geo=pfc->GetFeature(fIndex);
                        if(canTransform) geo->Project(pTrans.get());
                        double dis=dpt->DistanceTo(geo.get());
                        results.Add(fIndex,dis);
                    }
                }
            }
            if((ty>=0)&&(ty<yNum)){
                senv->Top()=top-cellSize*ty+semiCellSize;
                senv->Bottom()=senv->Top()-cellSize-semiCellSize;
                for(int k=fx+1;k<tx;k++){
                    if(k<0){
                        k=0;
                        if(k>=tx) break;
                    }
                    if(k>=xNum) break;
                    senv->Left()=left+cellSize*k-semiCellSize;
                    senv->Right()=senv->Left()+cellSize+semiCellSize;
                    SpatialCell*cell=regions[k+ty*xNum];
                    int featureCount=cell->GetSize();
                    for(int p=0;p<featureCount;p++){
                        int fIndex=cell->GetFeatureIndex(p);
                        if(results.HasFeature(fIndex)) continue;
                        if(!pfc->GetFeatureEnvelope(fIndex,env.get())) continue;
                        if(!senv->Intersects(env.get())) continue;
                        AutoPtr<Geometry>geo=pfc->GetFeature(fIndex);
                        if(canTransform) geo->Project(pTrans.get());
                        double dis=dpt->DistanceTo(geo.get());
                        results.Add(fIndex,dis);
                    }
                }
            }
        }
        if(results.GetSize()>=Num){
            if(results.GetDistance(Num-1)<=validRadius) break;
        }
        if (fRadius >= maxradius) break;
        fRadius += cellSize;
        validRadius+=cellSize;
        d++;
    }
    results.Trim(Num);
}


void SpatialIndexer::SearchNearstFeaturesByRaius(Point *dpt, double toler, IndexDistanceArray &results){
    SearchNearstFeaturesByRaius(dpt->X(),dpt->Y(),toler,results);
}

void SpatialIndexer::SearchNearstFeaturesByNum(Point*dpt,int Num,IndexDistanceArray&results){
    SearchNearstFeaturesByNum(dpt->X(),dpt->Y(),Num,results);
}

SpatialPoints::SpatialPoints(){
    dpts=new Points();
}
SpatialPoints::~SpatialPoints(){

}
Point2D<double>SpatialPoints::GetLocation(int nIndex){
    double x,y;
    dpts->GetItem(nIndex,x,y);
    return Point2D<double>(x,y);
}

void SpatialPoints::GetLocation(int nIndex,Point2D<double>&loc){
    dpts->GetItem(nIndex,loc.X,loc.Y);
}
int SpatialPoints::GetSize(){
    return dpts->GetSize();
}
AutoPtr<Envelope>SpatialPoints::GetExtent(){
    return dpts->GetExtent();
}

void SpatialPoints::Add(Point2D<double>pos){
    dpts->Add(pos.X,pos.Y);
}

void SpatialPoints::Clear(){
    dpts->Clear();
}

LocationSpatialIndexer::LocationSpatialIndexer(int maxWidthOrHeight){
    this->dpts=nullptr;
    this->maxWidthOrHeight=maxWidthOrHeight;
    if(this->maxWidthOrHeight<=0) this->maxWidthOrHeight=1;
    formerPtNum=0;
    onlyRef=true;
    kToler=0;
    distArray=new OrderDistanceArray();
    bufferdistArray=new OrderDistanceArray();
    buffermirrorArray=new MirrorDistanceArray();
    mdistArray=new MirrorDistanceArray();
}
LocationSpatialIndexer::~LocationSpatialIndexer(){
    for(int k=regions.size()-1;k>=0;k--) delete regions[k];
    regions.clear();
    if(!onlyRef){
        if(dpts!=nullptr) delete dpts;
    }
}

bool LocationSpatialIndexer::HasBuildSpatialIndices(){
    return (dpts!=nullptr);
}
void LocationSpatialIndexer::Clear(){
    for(int k=regions.size()-1;k>=0;k--) delete regions[k];
    regions.clear();
}
void LocationSpatialIndexer::InitialCells(Envelope*fullExtent){
    Clear();
    if(fullExtent!=nullptr) fullExtent->GetCoord(&left,&top,&right,&bottom);
    if(right-left>=top-bottom){
        xNum=maxWidthOrHeight;
        cellSize=(right-left)/xNum;
        if(cellSize==0) cellSize=0.1;
        yNum=(top-bottom)/cellSize;
        if(yNum*cellSize<top-bottom) yNum++;
    }else{
        yNum=maxWidthOrHeight;
        cellSize=(top-bottom)/yNum;
        if(cellSize==0) cellSize=0.1;
        xNum=(top-bottom)/cellSize;
        if(xNum*cellSize<top-bottom) xNum++;
    }
    if(xNum==0) xNum=1;
    if(yNum==0) yNum=1;
    regions.resize(xNum*yNum);
    for(int k=xNum*yNum-1;k>=0;k--)
    {
        regions[k]=new SpatialCell();
    }
}

int LocationSpatialIndexer::GetRegionX(double x)
{
    int xid=(x-left)/cellSize;
    if(xid<0) return 0;
    if(xid>=xNum) xid=xNum-1;
    return xid;
}
int LocationSpatialIndexer::GetRegionY(double y)
{
    int yid=(top-y)/cellSize;
    if(yid<0) return 0;
    if(yid>=yNum) yid=yNum-1;
    return yid;
}
int LocationSpatialIndexer::GetRealRegionX(double x)
{
    if(x<left) return (x-left)/cellSize-1;
    return (x-left)/cellSize;
}
int LocationSpatialIndexer::GetRealRegionY(double y)
{
    if(top<y) return (top-y)/cellSize-1;
    return (top-y)/cellSize;
}

AutoPtr<Envelope>LocationSpatialIndexer::GetFullExtent(){
    return new Envelope(left,top,right,bottom);
}

Point2D<double> LocationSpatialIndexer::GetFeaturePoint(int fIndex){
    return dpts->GetLocation(fIndex);
}

void LocationSpatialIndexer::GetFeaturePoint(int fIndex,Point2D<double>&loc){
    dpts->GetLocation(fIndex,loc);
}

void LocationSpatialIndexer::RegisterPoint(Point*pt,int fIndex){
    int rx=GetRegionX(pt->X());
    int ry=GetRegionY(pt->Y());
    regions[rx+ry*xNum]->Add(fIndex);
}

void LocationSpatialIndexer::RegisterPoint(DOUBLE X,double Y,int fIndex){
    int rx=GetRegionX(X);
    int ry=GetRegionY(Y);
    regions[rx+ry*xNum]->Add(fIndex);
}

bool LocationSpatialIndexer::BuildSpatialIndices(SpatialLocations*dpts){
    if(!onlyRef){
        if(this->dpts!=nullptr) delete this->dpts;
    }
    this->dpts=dpts;
    onlyRef=true;
    formerPtNum=0;
    AutoPtr<Envelope>fullExtent=dpts->GetExtent();
    int featureCount=dpts->GetSize();
    int idealNum = sqrt(featureCount)/2;
    if (idealNum < 1)
        idealNum = 1;
     else if (idealNum > 50)
        idealNum = 50;
    maxWidthOrHeight = idealNum;
    InitialCells(fullExtent.get());
    this->pfc=nullptr;
    for(int k=0;k<featureCount;k++){
        Point2D<double>pos=dpts->GetLocation(k);
        RegisterPoint(pos.X,pos.Y,k);
    }
    return true;
}
AutoPtr<DistanceArray>LocationSpatialIndexer::SearchNearstFeaturesByRaius(double X,double Y, double toler){
    distArray->Clear();
    formerPtNum=0;
    int ix1=GetRegionX(X-toler);
    int ix2=GetRegionX(X+toler);
    int iy1=GetRegionY(Y+toler);
    int iy2=GetRegionY(Y-toler);
    for(int m=iy1;m<=iy2;m++){
        int cellPos=ix1+m*xNum;
        for(int n=ix1;n<=ix2;n++){
            SpatialCell*cell=regions[cellPos++];
            int featureCount=cell->GetSize();
            for(int p=0;p<featureCount;p++){
                int fIndex=cell->GetFeatureIndex(p);
                Point2D<double>ppt=dpts->GetLocation(fIndex);
                double dis=sqrt((ppt.X-X)*(ppt.X-X)+(ppt.Y-Y)*(ppt.Y-Y));
                if(dis>toler) continue;
                distArray->Add(fIndex,dis);
            }
        }
    }
    return distArray;
}

AutoPtr<DistanceArray>LocationSpatialIndexer::SearchNearstFeaturesByNumEx(double dptx,double dpty,int Num,int M){
    if(Num>=dpts->GetSize()){
        bufferdistArray->Clear();
        int size=dpts->GetSize();
        for(int k=0;k<size;k++){
            Point2D<double>ppt=dpts->GetLocation(k);
            double dis=sqrt((ppt.X-dptx)*(ppt.X-dptx)+(ppt.Y-dpty)*(ppt.Y-dpty));
            int fIndex=k;
            bufferdistArray->Add(fIndex,dis);
        }
        return bufferdistArray;
    }
    bool needReset=false;
    int ptNum=bufferdistArray->GetSize();
    if(formerPtNum==Num){
        if(ptNum>Num){
            double diffdist=sqrt((dptx-formerPos.X)*(dptx-formerPos.X)+(dpty-formerPos.Y)*(dpty-formerPos.Y));
            if(diffdist>=kToler) needReset=true;
        }
        else
            needReset=true;
    }
    else
        needReset=true;
    int bufferNum=Num+M;
    if(needReset){
        float fRadius = cellSize;
        double right = left + xNum * cellSize;
        double bottom = top - yNum * cellSize;
        bufferdistArray->Clear();
        double maxradius = sqrt((left - dptx)*(left - dptx) + (top - dpty)*(top - dpty));
        maxradius = max(maxradius, sqrt((right - dptx)*(right - dptx) + (top - dpty)*(top - dpty)));
        maxradius = max(maxradius, sqrt((right - dptx)*(right - dptx) + (bottom - dpty)*(bottom - dpty)));
        maxradius = max(maxradius, sqrt((left - dptx)*(left - dptx) + (bottom - dpty)*(bottom - dpty)));
        maxradius+=2*cellSize;
        int ix=GetRealRegionX(dptx);
        int iy=GetRealRegionY(dpty);
        double cellleft=left+ix*cellSize;
        double celltop=top-iy*cellSize;
        double cellright=cellleft+cellSize;
        double cellbottom=celltop-cellSize;
        float validRadius=dptx-cellleft;
        if(validRadius>cellright-dptx) validRadius=cellright-dptx;
        if(validRadius>dpty-cellbottom) validRadius=dpty-cellbottom;
        if(validRadius>celltop-dpty) validRadius=celltop-dpty;
        int d=0;
        if(ix<0)
            d=-ix;
        else if(ix>=xNum)
            d=ix-xNum+1;
        int dy=0;
        if(iy<0)
            dy=-iy;
        else if(iy>=yNum)
            dy=iy-yNum+1;
        if(dy>d) d=dy;
        validRadius+=d*cellSize;
        fRadius=(d+1)*cellSize;
        while (true) {
            int fx=ix-d;
            int tx=ix+d;
            int fy=iy-d;
            int ty=iy+d;
            if((fx>=0)&&(fx<xNum)){
                for(int k=fy;k<=ty;k++){
                    if(k<0){
                        k=0;
                        if(k>ty) break;
                    }
                    if(k>=yNum) break;
                    SpatialCell*cell=regions[fx+k*xNum];
                    int featureCount=cell->GetSize();
                    Point2D<double>ppt;
                    for(int p=0;p<featureCount;p++){
                        int fIndex=cell->GetFeatureIndex(p);
                        dpts->GetLocation(fIndex,ppt);
                        double dis=sqrt((ppt.X-dptx)*(ppt.X-dptx)+(ppt.Y-dpty)*(ppt.Y-dpty));
                        bufferdistArray->Add(fIndex,dis);
                    }
                }
            }
            if(d>0){
                if((tx>=0)&&(tx<xNum)){
                    for(int k=fy;k<=ty;k++){
                        if(k<0){
                            k=0;
                            if(k>ty) break;
                        }
                        if(k>=yNum) break;
                        SpatialCell*cell=regions[tx+k*xNum];
                        int featureCount=cell->GetSize();
                        Point2D<double>ppt;
                        for(int p=0;p<featureCount;p++){
                            int fIndex=cell->GetFeatureIndex(p);
                            dpts->GetLocation(fIndex,ppt);
                            double dis=sqrt((ppt.X-dptx)*(ppt.X-dptx)+(ppt.Y-dpty)*(ppt.Y-dpty));
                            bufferdistArray->Add(fIndex,dis);
                        }
                    }
                }
                if((fy>=0)&&(fy<yNum)){
                    for(int k=fx+1;k<tx;k++){
                        if(k<0){
                            k=0;
                            if(k>=tx) break;
                        }
                        if(k>=xNum) break;
                        SpatialCell*cell=regions[k+fy*xNum];
                        int featureCount=cell->GetSize();
                        Point2D<double>ppt;
                        for(int p=0;p<featureCount;p++){
                            int fIndex=cell->GetFeatureIndex(p);
                            dpts->GetLocation(fIndex,ppt);
                            double dis=sqrt((ppt.X-dptx)*(ppt.X-dptx)+(ppt.Y-dpty)*(ppt.Y-dpty));
                            bufferdistArray->Add(fIndex,dis);
                        }
                    }
                }
                if((ty>=0)&&(ty<yNum)){
                    for(int k=fx+1;k<tx;k++){
                        if(k<0){
                            k=0;
                            if(k>=tx) break;
                        }
                        if(k>=xNum) break;
                        SpatialCell*cell=regions[k+ty*xNum];
                        int featureCount=cell->GetSize();
                        Point2D<double>ppt;
                        for(int p=0;p<featureCount;p++){
                            int fIndex=cell->GetFeatureIndex(p);
                            dpts->GetLocation(fIndex,ppt);
                            double dis=sqrt((ppt.X-dptx)*(ppt.X-dptx)+(ppt.Y-dpty)*(ppt.Y-dpty));
                            bufferdistArray->Add(fIndex,dis);
                        }
                    }
                }
            }
            if(bufferdistArray->GetSize()>=bufferNum){
                if(bufferdistArray->GetDistance(bufferNum-1)<=validRadius) break;
            }
            fRadius += cellSize;
            if (fRadius >= maxradius) break;
            validRadius+=cellSize;
            d++;
        }
        bufferdistArray->Trim(bufferNum);
        formerPtNum=Num;
        formerPos.X=dptx;
        formerPos.Y=dpty;
        int size=bufferdistArray->GetSize();
        if(size>Num){
            kToler=(bufferdistArray->GetDistance(size-1)-bufferdistArray->GetDistance(Num-1))/2.0;
        }
        bufferdistArray->TrimToOther(Num, distArray.get());
        buffermirrorArray->FromDistanceArray(this,bufferdistArray.get(),dptx, dpty,Num);
        return distArray;
    }
    else{
        buffermirrorArray->MoveCenter(this, dptx, dpty);
        buffermirrorArray->TrimToOther(Num, mdistArray.get());
        return mdistArray;
    }
    return nullptr;
}

AutoPtr<DistanceArray>LocationSpatialIndexer::SearchNearstFeaturesByNum(double dptx,double dpty, int Num){
    if(Num>=dpts->GetSize()){
        bufferdistArray->Clear();
        int size=dpts->GetSize();
        for(int k=0;k<size;k++){
            Point2D<double>ppt=dpts->GetLocation(k);
            double dis=sqrt((ppt.X-dptx)*(ppt.X-dptx)+(ppt.Y-dpty)*(ppt.Y-dpty));
            int fIndex=k;
            bufferdistArray->Add(fIndex,dis);
        }
        return bufferdistArray;
    }
    bool needReset=false;
    int ptNum=bufferdistArray->GetSize();
    if(formerPtNum==Num){
        if(ptNum>Num){
            double fromDist=bufferdistArray->GetDistance(Num-1);
            double toDist=bufferdistArray->GetDistance(ptNum-1);
            double diffdist=sqrt((dptx-formerPos.X)*(dptx-formerPos.X)+(dpty-formerPos.Y)*(dpty-formerPos.Y));
            if(diffdist>=(toDist-fromDist)/2) needReset=true;
        }
        else
            needReset=true;
    }
    else
        needReset=true;
    int bufferNum=Num;
    if(needReset){
        float fRadius = cellSize;
        double right = left + xNum * cellSize;
        double bottom = top - yNum * cellSize;
        bufferdistArray->Clear();
        double maxradius = sqrt((left - dptx)*(left - dptx) + (top - dpty)*(top - dpty));
        maxradius = max(maxradius, sqrt((right - dptx)*(right - dptx) + (top - dpty)*(top - dpty)));
        maxradius = max(maxradius, sqrt((right - dptx)*(right - dptx) + (bottom - dpty)*(bottom - dpty)));
        maxradius = max(maxradius, sqrt((left - dptx)*(left - dptx) + (bottom - dpty)*(bottom - dpty)));
        maxradius+=2*cellSize;
        int ix=GetRealRegionX(dptx);
        int iy=GetRealRegionY(dpty);
        double cellleft=left+ix*cellSize;
        double celltop=top-iy*cellSize;
        double cellright=cellleft+cellSize;
        double cellbottom=celltop-cellSize;
        float validRadius=dptx-cellleft;
        if(validRadius>cellright-dptx) validRadius=cellright-dptx;
        if(validRadius>dpty-cellbottom) validRadius=dpty-cellbottom;
        if(validRadius>celltop-dpty) validRadius=celltop-dpty;
        int d=0;
        if(ix<0)
            d=-ix;
        else if(ix>=xNum)
            d=ix-xNum+1;
        int dy=0;
        if(iy<0)
            dy=-iy;
        else if(iy>=yNum)
            dy=iy-yNum+1;
        if(dy>d) d=dy;
        validRadius+=d*cellSize;
        fRadius=(d+1)*cellSize;
        while (true) {
            int fx=ix-d;
            int tx=ix+d;
            int fy=iy-d;
            int ty=iy+d;
            if((fx>=0)&&(fx<xNum)){
                for(int k=fy;k<=ty;k++){
                    if(k<0) k=0;
                    if(k>=yNum) break;
                    SpatialCell*cell=regions[fx+k*xNum];
                    int featureCount=cell->GetSize();
                    for(int p=0;p<featureCount;p++){
                        int fIndex=cell->GetFeatureIndex(p);
                        Point2D<double>ppt=dpts->GetLocation(fIndex);
                        double dis=sqrt((ppt.X-dptx)*(ppt.X-dptx)+(ppt.Y-dpty)*(ppt.Y-dpty));
                        bufferdistArray->Add(fIndex,dis);
                    }
                }
            }
            if(d>0){
                if((tx>=0)&&(tx<xNum)){
                    for(int k=fy;k<=ty;k++){
                        if(k<0) k=0;
                        if(k>=yNum) break;
                        SpatialCell*cell=regions[tx+k*xNum];
                        int featureCount=cell->GetSize();
                        for(int p=0;p<featureCount;p++){
                            int fIndex=cell->GetFeatureIndex(p);
                            Point2D<double>ppt=dpts->GetLocation(fIndex);
                            double dis=sqrt((ppt.X-dptx)*(ppt.X-dptx)+(ppt.Y-dpty)*(ppt.Y-dpty));
                            bufferdistArray->Add(fIndex,dis);
                        }
                    }
                }
                if((fy>=0)&&(fy<yNum)){
                    for(int k=fx+1;k<tx;k++){
                        if(k<0) k=0;
                        if(k>=xNum) break;
                        SpatialCell*cell=regions[k+fy*xNum];
                        int featureCount=cell->GetSize();
                        for(int p=0;p<featureCount;p++){
                            int fIndex=cell->GetFeatureIndex(p);
                            Point2D<double>ppt=dpts->GetLocation(fIndex);
                            double dis=sqrt((ppt.X-dptx)*(ppt.X-dptx)+(ppt.Y-dpty)*(ppt.Y-dpty));
                            bufferdistArray->Add(fIndex,dis);
                        }
                    }
                }
                if((ty>=0)&&(ty<yNum)){
                    for(int k=fx+1;k<tx;k++){
                        if(k<0) k=0;
                        if(k>=xNum) break;
                        SpatialCell*cell=regions[k+ty*xNum];
                        int featureCount=cell->GetSize();
                        for(int p=0;p<featureCount;p++){
                            int fIndex=cell->GetFeatureIndex(p);
                            Point2D<double>ppt=dpts->GetLocation(fIndex);
                            double dis=sqrt((ppt.X-dptx)*(ppt.X-dptx)+(ppt.Y-dpty)*(ppt.Y-dpty));
                            bufferdistArray->Add(fIndex,dis);
                        }
                    }
                }
            }
            if(bufferdistArray->GetSize()>=bufferNum){
                if(bufferdistArray->GetDistance(bufferNum-1)<=validRadius) break;
            }
            if (fRadius >= maxradius) break;
            fRadius += cellSize;
            validRadius+=cellSize;
            d++;
        }
        bufferdistArray->Trim(bufferNum);
        formerPtNum=Num;
        formerPos.X=dptx;
        formerPos.Y=dpty;
        bufferdistArray->TrimToOther(Num, distArray.get());
        buffermirrorArray->FromDistanceArray(this,bufferdistArray.get(),dptx, dpty,Num);
        return distArray;
    }
    else{
        buffermirrorArray->MoveCenter(this,dptx, dpty);
        buffermirrorArray->TrimToOther(Num, mdistArray.get());
        return mdistArray;
    }
    return nullptr;
}

AutoPtr<DistanceArray>LocationSpatialIndexer::SearchNearstFeaturesByRaius(Point *dpt, double toler){
    return SearchNearstFeaturesByRaius(dpt->X(),dpt->Y(),toler);
}

AutoPtr<DistanceArray>LocationSpatialIndexer::SearchNearstFeaturesByNum(Point*dpt,int Num){
    return SearchNearstFeaturesByNum(dpt->X(),dpt->Y(),Num);
}

PointSpatialIndexer::PointSpatialIndexer(int maxWidthOrHeight)
    :LocationSpatialIndexer(maxWidthOrHeight){
   dpts=new SpatialPoints();
   onlyRef=false;
}

PointSpatialIndexer::~PointSpatialIndexer(){

}

bool PointSpatialIndexer::BuildSpatialIndices(AutoPtr<FeatureClass>pfc,AutoPtr<SpatialReference>psp){
    VectorShapeType vType=pfc->GetShapeType();
    if(vType!=vstPoint) return false;
    bool IsValid=(dpts!=nullptr);
    if(dpts!=nullptr)
    {
        AutoPtr<SpatialPoints>sps=AutoPtr<SpatialLocations>(dpts,true);
        if(sps==nullptr) IsValid=false;
    }
    if(!IsValid){
        if(!onlyRef) delete dpts;
        onlyRef=false;
        dpts=new SpatialPoints();
    }
    else{
         ((SpatialPoints*)dpts)->Clear();
    }
    SpatialPoints*sps=((SpatialPoints*)dpts);
    formerPtNum=0;
    AutoPtr<Envelope>fullExtent=pfc->GetExtent();
    AutoPtr<CoordinateTransformation>pTrans;
    bool canTransform=false;
    if(psp!=nullptr){
        AutoPtr<SpatialReference>lypsp=pfc->GetSpatialReference();
        pTrans=new CoordinateTransformation(lypsp,psp);
        pTrans->BeginTransform();
        canTransform=pTrans->CanTransform();
        if(canTransform){
            fullExtent->Project(pTrans.get());
        }
    }
    int featureCount=pfc->GetFeatureCount();
    int idealNum = sqrt(featureCount) / 2;
    if (idealNum < 1)
        idealNum = 1;
     else if (idealNum > 50)
        idealNum = 50;
    maxWidthOrHeight = idealNum;
    InitialCells(fullExtent.get());
    this->pfc=pfc;
    for(int k=0;k<featureCount;k++){
        AutoPtr<Point>geo=pfc->GetFeature(k);
        if(geo==nullptr) continue;
        if(canTransform) geo->Project(pTrans.get());
        sps->Add(Point2D<double>(geo->X(),geo->Y()));
        RegisterPoint(geo.get(),k);
    }
    return true;
}

bool PointSpatialIndexer::BuildSpatialIndices(AutoPtr<Points>pts){
    bool IsValid=(dpts!=nullptr);
    if(dpts!=nullptr)
    {
        AutoPtr<SpatialPoints>sps=AutoPtr<SpatialLocations>(dpts,true);
        if(sps==nullptr) IsValid=false;
    }
    if(!IsValid){
        if(!onlyRef) delete dpts;
        onlyRef=false;
        dpts=new SpatialPoints();
    }
    else{
         ((SpatialPoints*)dpts)->Clear();
    }
    SpatialPoints*sps=((SpatialPoints*)dpts);
    formerPtNum=0;
    AutoPtr<Envelope>fullExtent=pts->GetExtent();
    int featureCount=pts->GetSize();
    int idealNum = sqrt(featureCount) / 2;
    if (idealNum < 1)
        idealNum = 1;
     else if (idealNum > 50)
        idealNum = 50;
    maxWidthOrHeight = idealNum;
    InitialCells(fullExtent.get());
    this->pfc=nullptr;
    DOUBLE X,Y;
    for(int k=0;k<featureCount;k++){
        pts->GetItem(k,X,Y);
        sps->Add(Point2D<double>(X,Y));
        RegisterPoint(X,Y,k);
    }
    return true;
}

ManualPointSpatialIndexer::ManualPointSpatialIndexer(int widthOrHeight)
    :LocationSpatialIndexer(widthOrHeight){
    dpts=new SpatialPoints();
    onlyRef=false;
}
ManualPointSpatialIndexer::~ManualPointSpatialIndexer(){

}
void ManualPointSpatialIndexer::Create(AutoPtr<Envelope>fullExtent){
    bool IsValid=(dpts!=nullptr);
    if(dpts!=nullptr)
    {
        AutoPtr<SpatialPoints>sps=AutoPtr<SpatialLocations>(dpts,true);
        if(sps==nullptr) IsValid=false;
    }
    if(!IsValid){
        if(!onlyRef) delete dpts;
        onlyRef=false;
        dpts=new SpatialPoints();
    }
    else{
         ((SpatialPoints*)dpts)->Clear();
    }
    formerPtNum=0;
    AutoPtr<CoordinateTransformation>pTrans;
    InitialCells(fullExtent.get());
    nIndex=0;
}

bool ManualPointSpatialIndexer::Add(AutoPtr<Point>dpt){
    if(dpts==nullptr) return false;
    SpatialPoints*sps=((SpatialPoints*)dpts);
    sps->Add(Point2D<double>(dpt->X(),dpt->Y()));
    RegisterPoint(dpt.get(),nIndex++);
    return true;
}

bool ManualPointSpatialIndexer::CanSearchNearFeature(double X,double Y,double distance){
    int ix1=GetRegionX(X-distance);
    int ix2=GetRegionX(X+distance);
    int iy1=GetRegionY(Y+distance);
    int iy2=GetRegionY(Y-distance);
    for(int m=iy1;m<=iy2;m++){
        int cellPos=ix1+m*xNum;
        for(int n=ix1;n<=ix2;n++){
            SpatialCell*cell=regions[cellPos++];
            int featureCount=cell->GetSize();
            for(int p=0;p<featureCount;p++){
                int fIndex=cell->GetFeatureIndex(p);
                Point2D<double>ppt=dpts->GetLocation(fIndex);
                double dis=sqrt((ppt.X-X)*(ppt.X-X)+(ppt.Y-Y)*(ppt.Y-Y));
                if(dis<=distance) return true;
            }
        }
    }
    return false;
}

}
