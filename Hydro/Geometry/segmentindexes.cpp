#include "segmentindexes.h"
#include "Base/BaseArray.h"

namespace SGIS{
SegmentRegion::SegmentRegion()
{
}
SegmentRegion::~SegmentRegion()
{
}
void SegmentRegion::AddSegment(SegmentIndex seg)
{
    segments.push_back(seg);
}
int SegmentRegion::GetSize()
{
    return segments.size();
}
SegmentRegions::SegmentRegions()
{
    xNum=50;
    yNum=50;
    left=0;
    top=0;
    right=0;
    bottom=0;
    regions.resize(xNum*yNum);
    for(int k=xNum*yNum-1;k>=0;k--)
    {
        SegmentRegion*ors=new SegmentRegion();
        regions[k]=ors;
    }
}
SegmentRegions::SegmentRegions(int xSize,int ySize)
{
    xNum=xSize;
    yNum=ySize;
    left=0;
    top=0;
    right=0;
    bottom=0;
    regions.resize(xNum*yNum);
    for(int k=xNum*yNum-1;k>=0;k--)
    {
        SegmentRegion*ors=new SegmentRegion();
        regions[k]=ors;
    }
};
SegmentRegions::~SegmentRegions()
{
    for(int k=regions.size()-1;k>=0;k--)
    {
        delete regions[k];
    }
    regions.clear();
}
int SegmentRegions::GetRegionXNum()
{
    return xNum;
}
int SegmentRegions::GetRegionYNum()
{
    return yNum;
}
SegmentRegion*SegmentRegions::GetRegion(int nIndex)
{
    return regions[nIndex];
}

int SegmentRegions::GetRegionX(double x)
{
    int xid=(x-left)/difx;
    if(xid<0) return 0;
    if(xid>=xNum) xid=xNum-1;
    return xid;
}
int SegmentRegions::GetRegionY(double y)
{
    int yid=(top-y)/dify;
    if(yid<0) return 0;
    if(yid>=yNum) yid=yNum-1;
    return yid;
}
int SegmentRegions::GetRealRegionX(double x)
{
    int xid=(x-left)/difx;
    return xid;
}
int SegmentRegions::GetRealRegionY(double y)
{
    int yid=(top-y)/dify;
    return yid;
}
void SegmentRegions::innerGetLineRegions(double x1,double y1,double x2,double y2,AutoArray<Point2D<int>>&pArray){
    int ix1=GetRealRegionX(x1);
    int iy1=GetRealRegionY(y1);
    int ix2=GetRealRegionX(x2);
    int iy2=GetRealRegionY(y2);
    double lx,ly,cx,cy,nx,ny;
    double ilx,ily,icx,icy,inx,iny;
    bool IsCorner;
    if(abs(ix1-ix2)>=abs(iy1-iy2))
    {
        if(ix1>ix2)
        {
            int tempi=ix1;ix1=ix2;ix2=tempi;
            tempi=iy1;iy1=iy2;iy2=tempi;
            double tempf=x1;x1=x2;x2=tempf;
            tempf=y1;y1=y2;y2=tempf;
        }
        double k=(y2-y1)/(x2-x1);
        lx=x1;ly=y1;
        cx=left+difx*(ix1+1);
        cy=k*(cx-x1)+y1;
        nx=cx+difx/2.0;
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
            IsCorner=(fabs(top-icy*dify-cy)<=VertexTolerance);
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
                    AppendToArray(pArray,ilx+1,iny);
                }
            }
            AppendToArray(pArray,inx,iny);
            ilx=inx;
            ily=iny;
            lx=nx;
            ly=ny;
            cx=left+difx*(p+1);
            cy=k*(cx-x1)+y1;
            nx=cx+difx/2.0;
            ny=k*(nx-x1)+y1;
            icx=GetRealRegionX(cx);
            icy=GetRealRegionY(cy);
            inx=GetRealRegionX(nx);
            iny=GetRealRegionY(ny);
        }
    }
    else{
        if(iy1>iy2)
        {
            int tempi=ix1;ix1=ix2;ix2=tempi;
            tempi=iy1;iy1=iy2;iy2=tempi;
            double tempf=x1;x1=x2;x2=tempf;
            tempf=y1;y1=y2;y2=tempf;
        }
        double k=(x2-x1)/(y2-y1);
        lx=x1;ly=y1;
        cy=top-dify*(iy1+1);
        cx=k*(cy-y1)+x1;
        ny=cy-dify/2.0;
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
            IsCorner=(fabs(left+icx*difx-cx)<=VertexTolerance);
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
                    AppendToArray(pArray,inx,ily+1);
                }
            }
            AppendToArray(pArray,inx,iny);
            ilx=inx;
            ily=iny;
            lx=nx;
            ly=ny;
            cy=top-dify*(p+1);
            cx=k*(cy-y1)+x1;
            ny=cy-dify/2.0;
            nx=k*(ny-y1)+x1;
            icx=GetRealRegionX(cx);
            icy=GetRealRegionY(cy);
            inx=GetRealRegionX(nx);
            iny=GetRealRegionY(ny);
        }
    }
}
void SegmentRegions::GetLineRegions(double x1,double y1,double x2,double y2,AutoArray<Point2D<int>>&pArray)
{
    pArray.Clear();
    innerGetLineRegions(x1,y1,x2,y2,pArray);
}

void SegmentRegions::GetLineRegions(Path*path,AutoArray<Point2D<int>>&pArray){
    pArray.Clear();
    Points*pts=(Points*)path;
    LONG count=pts->GetSize();
    if(count<2) return;
    DOUBLE x1,y1,x2,y2;
    pts->GetItem(0,x1,y1);
    for(int k=1;k<count;k++){
        pts->GetItem(k,x2,y2);
        innerGetLineRegions(x1,y1,x2,y2,pArray);
        x1=x2;
        y1=y2;
    }
}
void SegmentRegions::GetLineRegions(SegmentIndex&index,AutoArray<Point2D<int>>&pArray)
{
    GetLineRegions(index.seg.lnx1,index.seg.lny1,index.seg.lnx2,index.seg.lny2,pArray);
}
void SegmentRegions::RegisterSegment(SegmentIndex index)
{
    AutoArray<Point2D<int>> pArray;
    GetLineRegions(index,pArray);
    int aCount=pArray.GetSize();
    Point2D<int> rp;
    for(int k=0;k<aCount;k++)
    {
        rp=pArray.GetItem(k);
        regions[rp.Y*xNum+rp.X]->AddSegment(index);
    }
}
void SegmentRegions::InitialCells(Envelope*fullExtent)
{
    if(fullExtent!=NULL) fullExtent->GetCoord(&left,&top,&right,&bottom);
    difx=(right-left)/xNum;
    dify=(top-bottom)/yNum;
    for(int k=xNum*yNum-1;k>=0;k--)
    {
        regions[k]->segments.clear();
    }
}

void SegmentRegions::AppendToArray(AutoArray<Point2D<int>>&pArray,int ix,int iy){
    if((ix<0)||(ix>=xNum)||(iy<0)||(iy>=yNum)) return;
    if(pArray.GetSize()>0){
        Point2D<int>former=pArray[pArray.GetSize()-1];
        if((former.X==ix)&&(former.Y==iy)) return;
    }
    pArray.Add(Point2D<int>(ix,iy));
}

SegmentIndexes::SegmentIndexes(AutoPtr<Geometry>geo,int cn)
{
    pGeo=NULL;
    regions=NULL;
    cellNum=cn;
    pGeo=ConvertGeometry(geo);
    if(pGeo!=NULL) BuildGeometry();
}
SegmentIndexes::~SegmentIndexes()
{
    if(regions!=NULL) delete regions;
}
AutoPtr<Geometry>SegmentIndexes::ConvertGeometry(AutoPtr<Geometry>geo)
{
    GeometryType gType=geo->GetType();
    if(gType==gtEnvelope)
    {
        return AutoPtr<Envelope>(geo)->ConvertToPolygon();
    }
    else if(gType==gtSegment){
       return AutoPtr<Segment>(geo)->ConvertToPolyline();
    }
    else if(gType==gtEllipse){
       return AutoPtr<Ellipse>(geo)->ConvertToPolygon();
    }
    else if(gType==gtPolyline){
       return ConvertToPolyline(geo);
    }
    else if(gType==gtPolygon){
       return ConvertToPolygon(geo);
    }
    else if(gType==gtMultiPolygon){
        AutoPtr<MultiPolygon>mpolygon(geo);
        AutoPtr<MultiPolygon>newpoly(new MultiPolygon());
        int partCount=mpolygon->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>subpoly=mpolygon->GetItem(k);
            AutoPtr<Polygon>poly=ConvertToPolygon(subpoly);
            newpoly->Add(poly);
        }
        return newpoly;
    }
}
void SegmentIndexes::BuildSpatialIndex()
{
    if(regions!=NULL) delete regions;
    regions=NULL;
    if(pGeo!=NULL)
    {
        regions=new SegmentRegions(cellNum,cellNum);
        AutoPtr<Envelope>pEnv=pGeo->GetExtent();
        regions->InitialCells(pEnv.get());
        int segCount=indexes.size();
        for(int k=0;k<segCount;k++) regions->RegisterSegment(indexes[k]);
    }
}
AutoPtr<Polyline>SegmentIndexes::ConvertToPolyline(AutoPtr<Polyline>geo)
{
    AutoPtr<CurvePolyline>pCurve(geo);
    if(pCurve!=nullptr)
    {
        LONG partCount=pCurve->GetSize();
        if(partCount>0)
        {
            AutoPtr<Path>pPath=pCurve->GetItem(0);
            DOUBLE len=pPath->GetLength();
            AutoPtr<Points>ppts(pPath);
            LONG ptCount=ppts->GetSize();
            if(ptCount>0)
            {
                AutoPtr<Polyline>newpoly=pCurve->ConvertToPolyline(len/ptCount/10);
                return newpoly;
            }
        }
    }
    return geo;
}
AutoPtr<Polygon>SegmentIndexes::ConvertToPolygon(AutoPtr<Polygon>geo)
{
    AutoPtr<CurvePolygon>pCurve(geo);
    if(pCurve!=nullptr)
    {
        LONG partCount=pCurve->GetSize();
        if(partCount>0)
        {
            AutoPtr<Ring>pPath=pCurve->GetItem(0);
            DOUBLE len=pPath->GetPerimeter();
            AutoPtr<Points>ppts(pPath);
            LONG ptCount=ppts->GetSize();
            if(ptCount>0)
            {
                AutoPtr<Polygon>newpoly=pCurve->ConvertToPolygon(len/ptCount/10);
                return newpoly;
            }
        }
    }
    return geo;
}

void SegmentIndexes::SetSize(int size)
{
    Clear();
    indexes.resize(size);
    for(int k=0;k<size;k++)
    {
        indexes[k]=SegmentIndex();
    }
}
void SegmentIndexes::Clear()
{
    indexes.clear();
}
bool SegmentIndexes::IsCurrentGeometryPolygon()
{
    AutoPtr<Polygon>poly(pGeo);
    if(poly!=NULL) return true;
    AutoPtr<MultiPolygon>mpoly(pGeo);
    if(mpoly!=NULL) return true;
    return false;
}
void SegmentIndexes::RegisterPoints(Points*points,int polyIndex,int partIndex,bool IsRing)
{
    LONG ptCount=points->GetSize();
    DOUBLE X1,Y1,X2,Y2;
    if(ptCount<2) return;
    points->GetItem(0,X1,Y1);
    Segment2D<double> formerSeg;
    bool formersegValid=false;
    if(IsRing)
    {
        int firstPin=-1;
        for(int k=1;k<=ptCount;k++)
        {
            if(k<ptCount)
                points->GetItem(k,X2,Y2);
            else
                points->GetItem(0,X2,Y2);
            if((fabs(X1-X2)<=VertexTolerance)&&(fabs(Y1-Y2)<=VertexTolerance)) continue;
            SegmentIndex si;
            si.polyIndex=polyIndex;
            si.partIndex=partIndex;
            si.pointIndex=k-1;
            si.pointCount=ptCount;
            if(formersegValid)
                si.formerseg=formerSeg;
            else
                firstPin=indexes.size();
            si.seg=Segment2D<double>(X1,Y1,X2,Y2);
            indexes.push_back(si);
            formerSeg=si.seg;
            X1=X2;
            Y1=Y2;
            formersegValid=true;
        }
        if(firstPin>=0) indexes[firstPin].formerseg=formerSeg;
    }
    else
    {
        for(int k=1;k<ptCount;k++)
        {
            points->GetItem(k,X2,Y2);
            if((fabs(X1-X2)<=VertexTolerance)&&(fabs(Y1-Y2)<=VertexTolerance)) continue;
            SegmentIndex si;
            si.polyIndex=polyIndex;
            si.partIndex=partIndex;
            si.pointIndex=k-1;
            si.pointCount=ptCount;
            if(formersegValid)
                si.formerseg=formerSeg;
            else
                si.formerseg=Segment2D<double>(X1,Y1,X1,Y1);
            si.seg=Segment2D<double>(X1,Y1,X2,Y2);
            indexes.push_back(si);
            X1=X2;
            Y1=Y2;
            formersegValid=true;
        }
    }
}
void SegmentIndexes::BuildPolyline(Polyline*poly)
{
    LONG partCount=poly->GetSize();
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Path>pPath=poly->GetItem(k);
        AutoPtr<Points>points(pPath);
        RegisterPoints(points.get(),-1,k,false);
    }
}
void SegmentIndexes::BuildPolygon(int polyIndex,Polygon*poly)
{
    LONG partCount=poly->GetSize();
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Ring>pRing=poly->GetItem(k);
        AutoPtr<Points>points(pRing);
        RegisterPoints(points.get(),polyIndex,k,true);
    }
}
void SegmentIndexes::BuildMultiPolygon(MultiPolygon*mpoly)
{
    LONG partCount=mpoly->GetSize();
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Polygon>subpoly=mpoly->GetItem(k);
        BuildPolygon(k,subpoly.get());
    }
}
void SegmentIndexes::BuildGeometry()
{
     Clear();
     AutoPtr<Polyline>polyline(pGeo);
     if(polyline!=nullptr) BuildPolyline(polyline.get());
     AutoPtr<Polygon>polygon(pGeo);
     if(polygon!=nullptr) BuildPolygon(-1,polygon.get());
     AutoPtr<MultiPolygon>mpolygon(pGeo);
     if(mpolygon!=nullptr) BuildMultiPolygon(mpolygon.get());
}
bool SegmentIndexes::IntersectSegment(Segment2D<double>&seg,int label)
{
    AutoArray<Point2D<int>> pArray;
    regions->GetLineRegions(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,pArray);
    int aCount=pArray.GetSize();
    int xNum=regions->GetRegionXNum();
    Point2D<double> intersectPoint;
    Point2D<int> rp;
    for(int k=0;k<aCount;k++)
    {
        rp=pArray.GetItem(k);
        SegmentRegion*sr=regions->GetRegion(rp.Y*xNum+rp.X);
        int subCount=sr->GetSize();
        for(int p=0;p<subCount;p++)
        {
            SegmentIndex&si=sr->segments[p];
            if(si.state==label) continue;
            Segment2D<double>&otherseg=si.seg;
            if(seg.IntersectofSegment(otherseg,intersectPoint)) return true;
            si.state=label;
        }
    }
    return false;
}
int SegmentIndexes::GetSegmentIntersectState(Segment2D<double>&seg,int label,Segment2D<double>&intersectSeg)
{
    Point2D<double> intersectPoint;
    int state=0;
    Point2D<double> interPoint;
    AutoArray<Point2D<int>> pArray;
    regions->GetLineRegions(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,pArray);
    int aCount=pArray.GetSize();
    int xNum=regions->GetRegionXNum();
    Point2D<int> rp;
    for(int k=0;k<aCount;k++)
    {
        rp=pArray.GetItem(k);
        SegmentRegion*sr=regions->GetRegion(rp.Y*xNum+rp.X);
        int subCount=sr->GetSize();
        for(int p=0;p<subCount;p++)
        {
            SegmentIndex&si=sr->segments[p];
            if(si.state==label) continue;
            Segment2D<double>&otherseg=si.seg;
            int s=seg.GetSegmentIntersectState(otherseg);
            if((s==5)||(s==6))
            {
                float f1=Triangle2D<double>::TriangleArea(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,si.formerseg.lnx1,si.formerseg.lny1);
                float f2=Triangle2D<double>::TriangleArea(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,otherseg.lnx2,otherseg.lny2);
                if(f1*f2<0) return 3;
                if(s==6)
                    s=4;
                else
                    s=1;
            }
            if(s==1)
            {
                if((state==0)||(state==2)) state=1;
            }
            else if(s==4)
            {
                intersectSeg=otherseg;
                state=4;
            }
            else if(s==2)
            {
                if(state==0) state=2;
            }
            else if(s==3)
            {
                return 3;
            }
            si.state=label;
        }
    }
    return state;
}
int SegmentIndexes::GetSegmentTouchState(Segment2D<double>&seg,int label,Segment2D<double>&intersectSeg)
{
    Point2D<double> intersectPoint;
    int state=0;
    Point2D<double> interPoint;
    AutoArray<Point2D<int>> pArray;
    regions->GetLineRegions(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,pArray);
    int aCount=pArray.GetSize();
    int xNum=regions->GetRegionXNum();
    Point2D<int> rp;
    for(int k=0;k<aCount;k++)
    {
        rp=pArray.GetItem(k);
        SegmentRegion*sr=regions->GetRegion(rp.Y*xNum+rp.X);
        int subCount=sr->GetSize();
        for(int p=0;p<subCount;p++)
        {
            SegmentIndex&si=sr->segments[p];
            if(si.state==label) continue;
            Segment2D<double>&otherseg=si.seg;
            int s=seg.GetSegmentIntersectState(otherseg);
            if((s==5)||(s==6))
            {
                float f1=Triangle2D<double>::TriangleArea(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,si.formerseg.lnx1,si.formerseg.lny1);
                float f2=Triangle2D<double>::TriangleArea(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,otherseg.lnx2,otherseg.lny2);
                if(f1*f2<0) return 3;
                if(s==6)
                    s=4;
                else
                    s=1;
            }
            if(s==1)
            {
                if((state==0)||(state==2)) state=1;
            }
            else if(s==4)
            {
                intersectSeg=otherseg;
                state=4;
            }
            else if(s==2)
            {
                return 2;
            }
            else if(s==3)
            {
                return 3;
            }
            si.state=label;
        }
    }
    return state;
}
int SegmentIndexes::GetRingSegmentIntersectState(Segment2D<double>&seg,int label)
{
    Point2D<double> intersectPoint;
    int state=0;
    Point2D<double> interPoint;
    AutoArray<Point2D<int>> pArray;
    regions->GetLineRegions(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,pArray);
    int aCount=pArray.GetSize();
    int xNum=regions->GetRegionXNum();
    Point2D<int> rp;
    for(int k=0;k<aCount;k++)
    {
        rp=pArray.GetItem(k);
        SegmentRegion*sr=regions->GetRegion(rp.Y*xNum+rp.X);
        int subCount=sr->GetSize();
        for(int p=0;p<subCount;p++)
        {
            SegmentIndex&si=sr->segments[p];
            if(si.state==label) continue;
            Segment2D<double>&otherseg=si.seg;
            Point2D<double>dpt(si.formerseg.lnx1,si.formerseg.lny1);
            int s=seg.GetRingSegmentIntersectState(otherseg,dpt);
            state=(state|s);
            if(state==7) return state;
            si.state=label;
        }
    }
    return state;
}
int SegmentIndexes::GetRingSegmentIntersectOuterState(Segment2D<double>&seg,int label)
{
    Point2D<double> intersectPoint;
    int state=0;
    Point2D<int> interPoint;
    AutoArray<Point2D<int>> pArray;
    regions->GetLineRegions(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,pArray);
    int aCount=pArray.GetSize();
    int xNum=regions->GetRegionXNum();
    Point2D<int> rp;
    for(int k=0;k<aCount;k++)
    {
        rp=pArray.GetItem(k);
        SegmentRegion*sr=regions->GetRegion(rp.Y*xNum+rp.X);
        int subCount=sr->GetSize();
        for(int p=0;p<subCount;p++)
        {
            SegmentIndex&si=sr->segments[p];
            if(si.state==label) continue;
            Segment2D<double>&otherseg=si.seg;
            Point2D<double>dpt(si.formerseg.lnx1,si.formerseg.lny1);
            int s=seg.GetRingSegmentIntersectState(otherseg,dpt);
            state=(state|s);
            if(state&4) return state;
            si.state=label;
        }
    }
    return state;
}
int SegmentIndexes::GetRingSegmentIntersectInnerState(Segment2D<double>&seg,int label)
{
    Point2D<double> intersectPoint;
    int state=0;
    Point2D<int> interPoint;
    AutoArray<Point2D<int>> pArray;
    regions->GetLineRegions(seg.lnx1,seg.lny1,seg.lnx2,seg.lny2,pArray);
    int aCount=pArray.GetSize();
    int xNum=regions->GetRegionXNum();
    Point2D<int> rp;
    for(int k=0;k<aCount;k++)
    {
        rp=pArray.GetItem(k);
        SegmentRegion*sr=regions->GetRegion(rp.Y*xNum+rp.X);
        int subCount=sr->GetSize();
        for(int p=0;p<subCount;p++)
        {
            SegmentIndex&si=sr->segments[p];
            if(si.state==label) continue;
            Segment2D<double>&otherseg=si.seg;
            Point2D<double>dpt(si.formerseg.lnx1,si.formerseg.lny1);
            int s=seg.GetRingSegmentIntersectState(otherseg,dpt);
            state=(state|s);
            if(state&1) return state;
            si.state=label;
        }
    }
    return state;
}
int SegmentIndexes::GetRingSegmentIntersectState(Path*pPath)
{
    Points*points=(Points*)pPath;
    LONG ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg;
    points->GetItem(0,seg.lnx1,seg.lny1);
    DOUBLE fx,fy;
    fx=seg.lnx1;
    fy=seg.lny1;
    int state=0;
    for(int p=1;p<ptCount;p++)
    {
        points->GetItem(p,seg.lnx2,seg.lny2);
        int s=GetRingSegmentIntersectState(seg,p);
        state=(state|s);
        if(state==7) return state;
        fx=seg.lnx1;
        fy=seg.lny1;
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    return state;
}
int SegmentIndexes::GetRingSegmentIntersectState(Ring*pPath)
{
    Points*points=(Points*)pPath;
    LONG ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg;
    points->GetItem(0,seg.lnx1,seg.lny1);
    DOUBLE fx,fy;
    fx=seg.lnx1;
    fy=seg.lny1;
    int state=0;
    for(int p=1;p<=ptCount;p++)
    {
        if(p<ptCount)
            points->GetItem(p,seg.lnx2,seg.lny2);
        else
            points->GetItem(0,seg.lnx2,seg.lny2);
        int s=GetRingSegmentIntersectState(seg,p);
        state=(state|s);
        if(state==7) return state;
        fx=seg.lnx1;
        fy=seg.lny1;
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    return state;
}
int SegmentIndexes::GetRingSegmentIntersectOuterState(Path*pPath)
{
    Points*points=(Points*)pPath;
    LONG ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg;
    points->GetItem(0,seg.lnx1,seg.lny1);
    DOUBLE fx,fy;
    fx=seg.lnx1;
    fy=seg.lny1;
    int state=0;
    for(int p=1;p<ptCount;p++)
    {
        points->GetItem(p,seg.lnx2,seg.lny2);
        int s=GetRingSegmentIntersectOuterState(seg,p);
        state=(state|s);
        if(state&4) return state;
        fx=seg.lnx1;
        fy=seg.lny1;
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    return state;
}
int SegmentIndexes::GetRingSegmentIntersectOuterState(Ring*pPath)
{
    Points*points=(Points*)pPath;
    LONG ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg;
    points->GetItem(0,seg.lnx1,seg.lny1);
    DOUBLE fx,fy;
    fx=seg.lnx1;
    fy=seg.lny1;
    int state=0;
    for(int p=1;p<=ptCount;p++)
    {
        if(p<ptCount)
            points->GetItem(p,seg.lnx2,seg.lny2);
        else
            points->GetItem(0,seg.lnx2,seg.lny2);
        int s=GetRingSegmentIntersectOuterState(seg,p);
        state=(state|s);
        if(state&4) return state;
        fx=seg.lnx1;
        fy=seg.lny1;
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    return state;
}
int SegmentIndexes::GetRingSegmentIntersectInnerState(Path*pPath)
{
    Points*points=(Points*)pPath;
    LONG ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg;
    points->GetItem(0,seg.lnx1,seg.lny1);
    DOUBLE fx,fy;
    fx=seg.lnx1;
    fy=seg.lny1;
    int state=0;
    for(int p=1;p<ptCount;p++)
    {
        points->GetItem(p,seg.lnx2,seg.lny2);
        int s=GetRingSegmentIntersectInnerState(seg,p);
        state=(state|s);
        if(state&1) return state;
        fx=seg.lnx1;
        fy=seg.lny1;
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    return state;
}
int SegmentIndexes::GetRingSegmentIntersectInnerState(Ring*pPath)
{
    Points*points=(Points*)pPath;
    LONG ptCount=points->GetSize();
    if(ptCount<2) return 0;
    Segment2D<double> seg;
    points->GetItem(0,seg.lnx1,seg.lny1);
    DOUBLE fx,fy;
    fx=seg.lnx1;
    fy=seg.lny1;
    int state=0;
    for(int p=1;p<=ptCount;p++)
    {
        if(p<ptCount)
            points->GetItem(p,seg.lnx2,seg.lny2);
        else
            points->GetItem(0,seg.lnx2,seg.lny2);
        int s=GetRingSegmentIntersectOuterState(seg,p);
        state=(state|s);
        if(state&1) return state;
        fx=seg.lnx1;
        fy=seg.lny1;
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    return state;
}
int SegmentIndexes::GetPolylineIntersectState(Polyline*polyline)
{
    LONG partCount=polyline->GetSize();
    int state=0;
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Path>pPath=polyline->GetItem(k);
        AutoPtr<Points>points(pPath);
        LONG ptCount=points->GetSize();
        if(ptCount<2) continue;
        Segment2D<double> seg;
        points->GetItem(0,seg.lnx1,seg.lny1);
        DOUBLE fx,fy;
        fx=seg.lnx1;
        fy=seg.lny1;
        Segment2D<double> intersectSeg;
        for(int p=1;p<ptCount;p++)
        {
            points->GetItem(p,seg.lnx2,seg.lny2);
            int s=GetSegmentIntersectState(seg,p,intersectSeg);
            if((s==4)&&(p>1))
            {
                float f1=Triangle2D<double>::TriangleArea(intersectSeg.lnx1,intersectSeg.lny1,intersectSeg.lnx2,intersectSeg.lny2,fx,fy);
                float f2=Triangle2D<double>::TriangleArea(intersectSeg.lnx1,intersectSeg.lny1,intersectSeg.lnx2,intersectSeg.lny2,seg.lnx2,seg.lny2);
                if(f1*f2<0) return 3;
                s=1;
            }
            if(s==1)
            {
                state=1;
            }
            else if(s==2)
            {
                if(state==0) state=2;
            }
            else if(s==3)
            {
                return 3;
            }
            fx=seg.lnx1;
            fy=seg.lny1;
            seg.lnx1=seg.lnx2;
            seg.lny1=seg.lny2;
        }
        if(k<partCount-1) ResetSegmentStates();
    }
    return state;
}
int SegmentIndexes::GetPolylineTouchState(Polyline*polyline)
{
    LONG partCount=polyline->GetSize();
    int state=0;
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Path>pPath=polyline->GetItem(k);
        AutoPtr<Points>points(pPath);
        LONG ptCount=points->GetSize();
        if(ptCount<2) continue;
        Segment2D<double> seg;
        points->GetItem(0,seg.lnx1,seg.lny1);
        DOUBLE fx,fy;
        fx=seg.lnx1;
        fy=seg.lny1;
        Segment2D<double> intersectSeg;
        for(int p=1;p<ptCount;p++)
        {
            points->GetItem(p,seg.lnx2,seg.lny2);
            int s=GetSegmentTouchState(seg,p,intersectSeg);
            if(s==4)
            {
                if(p>1)
                {
                    float f1=Triangle2D<double>::TriangleArea(intersectSeg.lnx1,intersectSeg.lny1,intersectSeg.lnx2,intersectSeg.lny2,fx,fy);
                    float f2=Triangle2D<double>::TriangleArea(intersectSeg.lnx1,intersectSeg.lny1,intersectSeg.lnx2,intersectSeg.lny2,seg.lnx2,seg.lny2);
                    if(f1*f2<0) return 3;
                    s=1;
                }
                else
                    s=1;
            }
            if(s==1)
            {
                state=1;
            }
            else if(s==2)
            {
                return 2;
            }
            else if(s==3)
            {
                return 3;
            }
            fx=seg.lnx1;
            fy=seg.lny1;
            seg.lnx1=seg.lnx2;
            seg.lny1=seg.lny2;
        }
        if(k<partCount-1) ResetSegmentStates();
    }
    return state;
}
int SegmentIndexes::GetPolylineCrossState(Polyline*polyline)
{
    LONG partCount=polyline->GetSize();
    int state=0;
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Path>pPath=polyline->GetItem(k);
        AutoPtr<Points>points(pPath);
        LONG ptCount=points->GetSize();
        if(ptCount<2) continue;
        Segment2D<double> seg;
        points->GetItem(0,seg.lnx1,seg.lny1);
        DOUBLE fx,fy;
        fx=seg.lnx1;
        fy=seg.lny1;
        Segment2D<double> intersectSeg;
        for(int p=1;p<ptCount;p++)
        {
            points->GetItem(p,seg.lnx2,seg.lny2);
            int s=GetSegmentTouchState(seg,p,intersectSeg);
            if((s==4)&&(p>1))
            {
                float f1=Triangle2D<double>::TriangleArea(intersectSeg.lnx1,intersectSeg.lny1,intersectSeg.lnx2,intersectSeg.lny2,fx,fy);
                float f2=Triangle2D<double>::TriangleArea(intersectSeg.lnx1,intersectSeg.lny1,intersectSeg.lnx2,intersectSeg.lny2,seg.lnx2,seg.lny2);
                if(f1*f2<0)
                    s=3;
                else
                    s=1;
            }
            if(s==1)
            {
                if(state==0) state=1;
            }
            else if(s==2)
            {
                return 2;
            }
            else if(s==3)
            {
                state=3;
            }
            fx=seg.lnx1;
            fy=seg.lny1;
            seg.lnx1=seg.lnx2;
            seg.lny1=seg.lny2;
        }
        if(k<partCount-1) ResetSegmentStates();
    }
    return state;
}
int SegmentIndexes::IsPathTouchRing(Path*pPath)
{
    Points*points=(Points*)pPath;
    LONG partCount=points->GetSize();
    if(partCount<2) return 0;
    Segment2D<double> seg;
    points->GetItem(0,seg.lnx1,seg.lny1);
    int state=0;
    for(int k=1;k<partCount;k++)
    {
        points->GetItem(k,seg.lnx2,seg.lny2);
        if((fabs(seg.lnx1-seg.lnx2)<=VertexTolerance)&&(fabs(seg.lny1-seg.lny2)<=VertexTolerance)) continue;
        int s=GetRingSegmentIntersectState(seg,k);
        state=(state|s);
        if(state&1)
        {
            return 0;
        }
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    if(state==0) return 2;
    return 1;
}
int SegmentIndexes::IsRingTouchRing(Ring*pRing)
{
    Points*points=(Points*)pRing;
    LONG partCount=points->GetSize();
    if(partCount<2) return 0;
    Segment2D<double> seg;
    points->GetItem(0,seg.lnx1,seg.lny1);
    int state=0;
    for(int k=1;k<=partCount;k++)
    {
        if(k<partCount)
            points->GetItem(k,seg.lnx2,seg.lny2);
        else
            points->GetItem(0,seg.lnx2,seg.lny2);
        if((fabs(seg.lnx1-seg.lnx2)<=VertexTolerance)&&(fabs(seg.lny1-seg.lny2)<=VertexTolerance)) continue;
        int s=GetRingSegmentIntersectState(seg,k);
        state=(state|s);
        if(state&1)
        {
            return 0;
        }
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    if(state==0) return 2;
    if(state==2) return 3;
    return 1;
}
int SegmentIndexes::IsPathCrossRing(Path*pPath)
{
    Points*points=(Points*)pPath;
    LONG partCount=points->GetSize();
    if(partCount<2) return 0;
    Segment2D<double> seg;
    points->GetItem(0,seg.lnx1,seg.lny1);
    int state=0;
    for(int k=1;k<partCount;k++)
    {
        points->GetItem(k,seg.lnx2,seg.lny2);
        if((fabs(seg.lnx1-seg.lnx2)<=VertexTolerance)&&(fabs(seg.lny1-seg.lny2)<=VertexTolerance)) continue;
        int s=GetRingSegmentIntersectState(seg,k);
        state=(state|s);
        if(state==7) return 1;
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    if(state==0) return 2;
    return 0;
}
int SegmentIndexes::IsRingCrossRing(Ring*pRing)
{
    Points*points=(Points*)pRing;
    LONG partCount=points->GetSize();
    if(partCount<2) return 0;
    Segment2D<double> seg;
    points->GetItem(0,seg.lnx1,seg.lny1);
    int state=0;
    for(int k=1;k<=partCount;k++)
    {
        if(k<partCount)
            points->GetItem(k,seg.lnx2,seg.lny2);
        else
            points->GetItem(0,seg.lnx2,seg.lny2);
        if((fabs(seg.lnx1-seg.lnx2)<=VertexTolerance)&&(fabs(seg.lny1-seg.lny2)<=VertexTolerance)) continue;
        int s=GetRingSegmentIntersectState(seg,k);
        state=(state|s);
        if(state==7) return 1;
        seg.lnx1=seg.lnx2;
        seg.lny1=seg.lny2;
    }
    if(state==0) return 2;
    return 0;
}
bool SegmentIndexes::Intersect(SegmentIndexes*other)
{
    AutoPtr<Envelope>pEnv=pGeo->GetExtent();
    AutoPtr<Envelope>otherEnv=other->pGeo->GetExtent();
    bool bIns=pEnv->IntersectsEnvelope(otherEnv.get());
    if(!bIns) return false;
    int otherCount=other->indexes.size();
    Point2D<int> intersectPoint;
    for(int k=0;k<otherCount;k++)
    {
        Segment2D<double>&seg=other->indexes[k].seg;
        if(IntersectSegment(seg,k+1)) return true;
    }
    return false;
}
bool SegmentIndexes::IntersectPoint(Point*point)
{
    AutoPtr<Envelope>pEnv=pGeo->GetExtent();
    bool IsIn=pEnv->IsPointIn(point);
    if(!IsIn) return false;
    DOUBLE X,Y;
    point->GetCoord(&X,&Y);
    int l=regions->GetRegionX(X);
    int t=regions->GetRegionY(Y);
    int xNum=regions->GetRegionXNum();
    int RegionID=t*xNum+l;
    SegmentRegion*sr=regions->GetRegion(RegionID);
    int subCount=sr->GetSize();
    for(int p=0;p<subCount;p++)
    {
        SegmentIndex si=sr->segments[p];
        Segment2D<double>&otherseg=si.seg;
        if(otherseg.IsPointOnSegment(X,Y,VertexTolerance)) return true;
    }
    return false;
}
bool SegmentIndexes::IntersectPath(Path*pPath)
{
    AutoPtr<Envelope>pEnv=pGeo->GetExtent();
    AutoPtr<Envelope>otherEnv=((Points*)pPath)->GetExtent();
    bool bIns=pEnv->IntersectsEnvelope(otherEnv.get());
    if(!bIns) return false;
    Points*points=(Points*)pPath;
    LONG ptCount=points->GetSize();
    if(ptCount<2) return false;
    DOUBLE X1,Y1,X2,Y2;
    points->GetItem(0,X1,Y1);
    for(int k=1;k<ptCount;k++)
    {
        points->GetItem(k,X2,Y2);
        if((fabs(X1-X2)<=VertexTolerance)&&(fabs(Y1-Y2)<=VertexTolerance)) continue;
        Segment2D<double>tseg(X1,Y1,X2,Y2);
        if(IntersectSegment(tseg,k))
        {
            return true;
        }
        X1=X2;
        Y1=Y2;
    }
    return false;
}
bool SegmentIndexes::IntersectRing(Ring*pRing)
{
    AutoPtr<Envelope>pEnv=pGeo->GetExtent();
    AutoPtr<Envelope>otherEnv=((Points*)pRing)->GetExtent();
    bool bIns=pEnv->IntersectsEnvelope(otherEnv.get());
    if(!bIns) return false;
    Points*points=(Points*)pRing;
    LONG ptCount=points->GetSize();
    if(ptCount<2) return false;
    DOUBLE X1,Y1,X2,Y2;
    points->GetItem(0,X1,Y1);
    for(int k=1;k<=ptCount;k++)
    {
        if(k<ptCount)
            points->GetItem(k,X2,Y2);
        else
            points->GetItem(0,X2,Y2);
        if((fabs(X1-X2)<=VertexTolerance)&&(fabs(Y1-Y2)<=VertexTolerance)) continue;
        Segment2D<double>tseg(X1,Y1,X2,Y2);
        if(IntersectSegment(tseg,k)) return true;
        X1=X2;
        Y1=Y2;
    }
    return false;
}
bool SegmentIndexes::IntersectPolyline(Polyline*polyline)
{
    LONG partCount=polyline->GetSize();
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Path>pPath=polyline->GetItem(k);
        if(IntersectPath(pPath.get())) return true;
        if(k<partCount-1) ResetSegmentStates();
    }
    return false;
}
bool SegmentIndexes::IntersectPolygon(Polygon*polygon)
{
    LONG partCount=polygon->GetSize();
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Ring>pPath=polygon->GetItem(k);
        if(IntersectRing(pPath.get())) return true;
        if(k<partCount-1) ResetSegmentStates();
    }
    return false;
}
bool SegmentIndexes::IntersectMultiPolygon(MultiPolygon*polygon)
{
    LONG partCount=polygon->GetSize();
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Polygon>poly=polygon->GetItem(k);
        LONG subCount=poly->GetSize();
        for(int p=0;p<subCount;p++)
        {
            AutoPtr<Ring>pPath=poly->GetItem(p);
            if(IntersectRing(pPath.get())) return true;
            if((p<subCount-1)||(k<partCount-1)) ResetSegmentStates();
        }
    }
    return false;
}
void SegmentIndexes::ResetSegmentStates()
{
    for(int k=indexes.size()-1;k>=0;k--) indexes[k].state=0;
}
bool SegmentIndexes::IntersectGeometry(AutoPtr<Geometry>geo)
{
    AutoPtr<Geometry>pGeo=ConvertGeometry(geo);
    if(pGeo==nullptr) return false;
    AutoPtr<Point>ppt(pGeo);
    if(ppt!=nullptr) return IntersectPoint(ppt.get());
    AutoPtr<Polyline>polyline(pGeo);
    if(polyline!=nullptr) return IntersectPolyline(polyline.get());
    AutoPtr<Polygon>polygon(pGeo);
    if(polygon!=nullptr) return IntersectPolygon(polygon.get());
    AutoPtr<MultiPolygon>mpolygon(pGeo);
    if(mpolygon!=NULL) return IntersectMultiPolygon(mpolygon.get());
    return false;
}

}
