#include "geometrybordercapture.h"
namespace SGIS{

GeometryIntersectPoints::GeometryIntersectPoints()
{
}
GeometryIntersectPoints::~GeometryIntersectPoints()
{
}
int GeometryIntersectPoints::GetSize()
{
    return points.size();
}
GeometryIntersectPoint GeometryIntersectPoints::GetItem(int nIndex)
{
    return points[nIndex];
}
void GeometryIntersectPoints::AddPoint(GeometryIntersectPoint point)
{
    points.push_back(point);
}
void GeometryIntersectPoints::SetPoint(int nIndex,GeometryIntersectPoint point)
{
    points[nIndex]=point;
}
void GeometryIntersectPoints::Clear()
{
    points.clear();
}
void GeometryIntersectPoints::Remove(int nIndex)
{
    points.erase(begin(points)+nIndex);
}
bool GeometryIntersectPoints::ContainFeatureIndex(int featureIndex)
{
    for(int k=points.size()-1;k>=0;k--)
    {
        if(points[k].featureIndex==featureIndex) return true;
    }
    return false;
}
void GeometryIntersectPoints::ReparePoins()
{
    if(points.size()<2) return;
    GeometryIntersectPoint frompt,topt;
    frompt=points[points.size()-1];
    for(int k=points.size()-1;k>=1;k--)
    {
         topt=points[k-1];
         if((topt.dpt.X==frompt.dpt.X)&&(topt.dpt.Y==frompt.dpt.Y))
         {
             points.erase(begin(points)+k);
         }
         frompt=topt;
    }
}
void GeometryIntersectPoints::CopyFromOther(GeometryIntersectPoints&other)
{
    points.clear();
    for(int k=0;k<other.GetSize();k++)
    {
        points.push_back(other.GetItem(k));
    }
}

GeometryTopo::GeometryTopo(AutoPtr<Geometry>geo)
{
    geometry=geo;
}
GeometryTopo::~GeometryTopo()
{

}

AutoPtr<GeometryTopo>GeometryTopo::CreateGeometryTopo(AutoPtr<Geometry>geo)
{
    GeometryType gType=geo->GetType();
    switch(gType)
    {
    case gtPoints:
        {
            AutoPtr<PointsTopo>topo=new PointsTopo(geo);
            return topo;
        }
    case gtPolyline:
        {
            AutoPtr<PolylineTopo>topo=new PolylineTopo(geo);
            return topo;
        }
    case gtPolygon:
        {
            AutoPtr<PolygonTopo>topo=new PolygonTopo(geo);
            return topo;
        }
    case gtMultiPolygon:
        {
            AutoPtr<MultiPolygonTopo>topo=new MultiPolygonTopo(geo);
            return topo;
        }
    }
    return nullptr;
}
void GeometryTopo::IntersectLine(Points*pts,GeometryIntersectPoints&points)
{
    LONG ptCount=pts->GetSize();
    if(ptCount<2) return;
    DOUBLE px1,py1,px2,py2;
    pts->GetItem(0,px1,py1);
    for(int k=1;k<ptCount;k++)
    {
        pts->GetItem(k,px2,py2);
        Segment2D<double> oseg(px1,py1,px2,py2);
        GeometryIntersectPoints tpts;
        IntersectSegment(oseg,tpts);
        for(int p=0;p<tpts.GetSize();p++)
        {
            GeometryIntersectPoint py=tpts.GetItem(p);
            py.reserved=k-1;
            points.AddPoint(py);
        }
        px1=px2;
        py1=py2;
    }
}
void GeometryTopo::IntersectClosedLine(Points*pts,GeometryIntersectPoints&points)
{
    LONG ptCount=pts->GetSize();
    if(ptCount<2) return;
    DOUBLE px1,py1,px2,py2;
    pts->GetItem(0,px1,py1);
    for(int k=1;k<=ptCount;k++)
    {
        if(k<ptCount)
            pts->GetItem(k,px2,py2);
        else
            pts->GetItem(0,px2,py2);
        Segment2D<double> oseg(px1,py1,px2,py2);
        GeometryIntersectPoints tpts;
        IntersectSegment(oseg,tpts);
        for(int p=0;p<tpts.GetSize();p++)
        {
            GeometryIntersectPoint py=tpts.GetItem(p);
            py.reserved=k-1;
            points.AddPoint(py);
        }
        px1=px2;
        py1=py2;
    }
}
bool GeometryTopo::CanDeleteVertex(GeometryCapture gc){
    return false;
}
bool GeometryTopo::DeleteVertex(GeometryCapture gc){
    return false;
}
Point2D<double> PointsTopo::GetVertex(GeometryCapture gc)
{
    Point2D<double> dpt;
    AutoPtr<Points>points=geometry;
    if(points==nullptr) return dpt;
    points->GetItem(gc.pointIndex,dpt.X,dpt.Y);
    return dpt;
}
Segment2D<double> PointsTopo::GetSegment(GeometryCapture gc)
{
    Segment2D<double> seg;
    Point2D<double> dpt1,dpt2;
    AutoPtr<Points>points=geometry;
    if(points==NULL) return seg;
    points->GetItem(gc.pointIndex,dpt1.X,dpt1.Y);
    LONG ptCount=points->GetSize();
    if(gc.pointIndex==ptCount-1)
        points->GetItem(0,dpt2.X,dpt2.Y);
    else
        points->GetItem(gc.pointIndex+1,dpt2.X,dpt2.Y);
    seg.lnx1=dpt1.X;
    seg.lny1=dpt1.Y;
    seg.lnx2=dpt2.X;
    seg.lny2=dpt2.Y;
    return seg;
}
GeometryCapture PointsTopo::CaptureVertex(DOUBLE x,DOUBLE y,DOUBLE tolerance)
{
    GeometryCapture gc;
    AutoPtr<Points>points=geometry;
    if(points==NULL) return gc;
    LONG ptCount=points->GetSize();
    DOUBLE px,py;
    DOUBLE dist;
    int index=-1;
    DOUBLE mindist;
    for(int k=0;k<ptCount;k++)
    {
        points->GetItem(k,px,py);
        dist=sqrt((x-px)*(x-px)+(y-py)*(y-py));
        if(dist>tolerance) continue;
        if(index==-1)
        {
            index=k;
            mindist=dist;
        }
        else if(dist<mindist)
        {
            index=k;
            mindist=dist;
        }
    }
    gc.pointIndex=index;
    return gc;
}
GeometryCapture PointsTopo::CaptureVertex(DOUBLE x,DOUBLE y)
{
    GeometryCapture gc;
    AutoPtr<Points>points=geometry;
    if(points==NULL) return gc;
    LONG ptCount=points->GetSize();
    DOUBLE px,py;
    DOUBLE dist;
    int index=-1;
    DOUBLE mindist;
    for(int k=0;k<ptCount;k++)
    {
        points->GetItem(k,px,py);
        dist=sqrt((x-px)*(x-px)+(y-py)*(y-py));
        if(index==-1)
        {
            index=k;
            mindist=dist;
        }
        else if(dist<mindist)
        {
            index=k;
            mindist=dist;
        }
    }
    gc.pointIndex=index;
    return gc;
}


void PointsTopo::MoveVertex(GeometryCapture gc,DOUBLE x,DOUBLE y)
{
    AutoPtr<Points>points=geometry;
    if(points==NULL) return;
    points->SetItem(gc.pointIndex,x,y);
    points->ComputeEnvelope();
}
void PointsTopo::InsertVertex(GeometryCapture gc,DOUBLE x,DOUBLE y)
{
    AutoPtr<Points>points=geometry;
    LONG ptCount=points->GetSize();
    if(gc.pointIndex==ptCount-1)
        points->Add(x,y);
    else
        points->Insert(gc.pointIndex+1,x,y);
    points->ComputeEnvelope();
}
GeometryCapture PointsTopo::CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt)
{
    GeometryCapture gc;
    AutoPtr<Points>pts=geometry;
    if(pts==NULL) return gc;
    LONG ptCount=pts->GetSize();
    if(ptCount<2) return gc;
    DOUBLE px1,py1,px2,py2;
    DOUBLE dist;
    DOUBLE mindist;
    int index=-1;
    pts->GetItem(0,px1,py1);
    DOUBLE nearx,neary;
    for(int k=1;k<ptCount;k++)
    {
        pts->GetItem(k,px2,py2);
        Segment2D<double> seg(px1,py1,px2,py2);
        dist=seg.PtMinDistToSegment(x,y,nearx,neary);
        if(dist>tolerance)
        {
            px1=px2;
            py1=py2;
            continue;
        }
        if(index==-1)
        {
            mindist=dist;
            pt.X=nearx;
            pt.Y=neary;
            index=k-1;
        }
        else if(dist<mindist)
        {
            mindist=dist;
            pt.X=nearx;
            pt.Y=neary;
            index=k-1;
        }
        px1=px2;
        py1=py2;
    }
    gc.pointIndex=index;
    return gc;
}
AutoPtr<Points>PointsTopo::GetPoints(GeometryCapture gc)
{
    AutoPtr<Points>pts=geometry;
    if(pts==NULL) return nullptr;
    if(gc.partIndex!=0) return nullptr;
    return pts;
}
void PointsTopo::IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points)
{
    AutoPtr<Points>pts=geometry;
    if(pts==NULL) return;
    LONG ptCount;
    ptCount=pts->GetSize();
    if(ptCount<2) return;
    DOUBLE px1,py1,px2,py2;
    pts->GetItem(0,px1,py1);
    for(int k=1;k<ptCount;k++)
    {
        pts->GetItem(k,px2,py2);
        Segment2D<double> oseg(px1,py1,px2,py2);
        Point2D<double> dpt;
        if(!oseg.IntersectofSegment(seg,dpt))
        {
            px1=px2;
            py1=py2;
            continue;
        }
        GeometryIntersectPoint gip(-1,-1,k-1,dpt);
        points.AddPoint(gip);
        px1=px2;
        py1=py2;
    }
}
Point2D<double> PathTopo::GetPoint(double percentage)
{
    AutoPtr<Points>pts=geometry;
    AutoPtr<Path>path=pts;
    Point2D<double> dpt;
    if(percentage<0) return dpt;
    LONG ptsCount;
    ptsCount=pts->GetSize();
    if(ptsCount<2) return dpt;
    double allLength=path->GetLength();
    double toLen=allLength*percentage;
    DOUBLE X1,Y1,X2,Y2;
    pts->GetItem(0,X1,Y1);
    if(percentage==0)
    {
        dpt.X=X1;
        dpt.Y=Y1;
        return dpt;
    }
    else if(percentage==1)
    {
        pts->GetItem(ptsCount-1,X2,Y2);
        dpt.X=X2;
        dpt.Y=Y2;
        return dpt;
    }
    double len=0;
    for(int k=1;k<ptsCount;k++)
    {
        pts->GetItem(k,X2,Y2);
        double curLen=sqrt((X1-X2)*(X1-X2)+(Y1-Y2)*(Y1-Y2));
        if(len+curLen>=toLen)
        {
            double difLen=len+curLen-toLen;
            double difper=1-difLen/curLen;
            dpt.X=(X2-X1)*difper+X1;
            dpt.Y=(Y2-Y1)*difper+Y1;
            return dpt;
        }
        len+=curLen;
        X1=X2;
        Y1=Y2;
    }
    return dpt;
}
double PathTopo::GetPercentage(GeometryIntersectPoint gc)
{
    AutoPtr<Points>pts=geometry;
    AutoPtr<Path>path=pts;
    Point2D<double> dpt=gc.dpt;
    LONG ptsCount;
    ptsCount=pts->GetSize();
    if(ptsCount<2) return 0;
    double allLength=path->GetLength();
    DOUBLE X1,Y1,X2,Y2;
    pts->GetItem(0,X1,Y1);
    double len=0;
    for(int k=1;k<ptsCount;k++)
    {
        pts->GetItem(k,X2,Y2);
        double curLen=sqrt((X1-X2)*(X1-X2)+(Y1-Y2)*(Y1-Y2));
        if(gc.pointIndex==k-1)
        {
            double dist1=sqrt((X1-dpt.X)*(X1-dpt.X)+(Y1-dpt.Y)*(Y1-dpt.Y));
            return (len+dist1)/allLength;
        }
        len+=curLen;
        X1=X2;
        Y1=Y2;
    }
    return 1;
}

bool PathTopo::CanDeleteVertex(GeometryCapture gc){
    AutoPtr<Points>pts=geometry;
    int ptCount=pts->GetSize();
    if(ptCount<3) return false;
    return true;
}

bool PathTopo::DeleteVertex(GeometryCapture gc){
    AutoPtr<Points>pts=geometry;
    int ptCount=pts->GetSize();
    if(ptCount<3) return false;
    pts->Remove(gc.pointIndex);
    return true;
}

GeometryCapture RingTopo::CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt)
{
    GeometryCapture gc;
    AutoPtr<Points>pts=geometry;
    if(pts==NULL) return gc;
    LONG ptCount;
    ptCount=pts->GetSize();
    if(ptCount<2) return gc;
    DOUBLE px1,py1,px2,py2;
    DOUBLE dist;
    DOUBLE mindist;
    int index=-1;
    pts->GetItem(0,px1,py1);
    DOUBLE nearx,neary;
    for(int k=1;k<=ptCount;k++)
    {
        if(k<ptCount)
            pts->GetItem(k,px2,py2);
        else
            pts->GetItem(0,px2,py2);
        Segment2D<double> seg(px1,py1,px2,py2);
        dist=seg.PtMinDistToSegment(x,y,nearx,neary);
        if((dist>tolerance)&&(tolerance>0))
        {
            px1=px2;
            py1=py2;
            continue;
        }
        if(index==-1)
        {
            mindist=dist;
            pt.X=nearx;
            pt.Y=neary;
            index=k-1;
        }
        else if(dist<mindist)
        {
            mindist=dist;
            pt.X=nearx;
            pt.Y=neary;
            index=k-1;
        }
        px1=px2;
        py1=py2;
    }
    gc.pointIndex=index;
    return gc;
}

void RingTopo::MoveVertex(GeometryCapture gc,DOUBLE x,DOUBLE y)
{
    AutoPtr<Points>points=geometry;
    if(points==NULL) return;
    LONG ptCount;
    ptCount=points->GetSize();
    if((gc.pointIndex>0)&&(gc.pointIndex<ptCount-1))
        PointsTopo::MoveVertex(gc,x,y);
    else
    {
        DOUBLE px1,py1;
        points->GetItem(0,px1,py1);
        DOUBLE px2,py2;
        points->GetItem(ptCount-1,px2,py2);
        if((px1==px2)&&(py1==py2))
        {
            points->SetItem(0,x,y);
            points->SetItem(ptCount-1,x,y);
        }
    }
    points->ComputeEnvelope();
}
void RingTopo::IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points)
{
    AutoPtr<Points>pts=geometry;
    if(pts==NULL) return;
    LONG ptCount;
    ptCount=pts->GetSize();
    if(ptCount<2) return;
    DOUBLE px1,py1,px2,py2;
    pts->GetItem(0,px1,py1);
    for(int k=1;k<=ptCount;k++)
    {
        if(k<ptCount)
            pts->GetItem(k,px2,py2);
        else
            pts->GetItem(0,px2,py2);
        Segment2D<double> oseg(px1,py1,px2,py2);
        Point2D<double> dpt;
        if(!oseg.IntersectofSegment(seg,dpt))
        {
            px1=px2;
            py1=py2;
            continue;
        }
        GeometryIntersectPoint gip(-1,-1,k-1,dpt);
        points.AddPoint(gip);
        px1=px2;
        py1=py2;
    }
}
void RingTopo::IntersectPath(Path*path,GeometryIntersectPoints&points)
{
    AutoPtr<Ring>pRing=geometry;
    if(pRing==NULL) return;
    AutoPtr<Points>ppts=AutoPtr<Path>(path,true);
    LONG ptCount=ppts->GetSize();
    DOUBLE X1,Y1,X2,Y2;
    if(ptCount<2) return;
    ppts->GetItem(0,X1,Y1);
    int FormerState=-1;
    for(int k=1;k<=ptCount;k++)
    {
        if(k<ptCount)
            ppts->GetItem(k,X2,Y2);
        else
            ppts->GetItem(0,X2,Y2);
        IntersectSegment(Segment2D<double>(X1,Y1,X2,Y2),points);
        X1=X2;
        Y1=Y2;
    }
}
Point2D<double> RingTopo::GetPoint(double percentage)
{
    AutoPtr<Points>pts=geometry;
    AutoPtr<Ring>ring=pts;
    Point2D<double> dpt;
    if((percentage<0)||(percentage>1)) return dpt;
    double allLength=ring->GetPerimeter();
    double toLen=allLength*percentage;
    LONG ptsCount;
    ptsCount=pts->GetSize();
    if(ptsCount<2) return dpt;
    DOUBLE X1,Y1,X2,Y2;
    pts->GetItem(0,X1,Y1);
    if((percentage==0)||(percentage==1))
    {
        dpt.X=X1;
        dpt.Y=Y1;
        return dpt;
    }
    double len=0;
    for(int k=1;k<=ptsCount;k++)
    {
        if(k<ptsCount)
            pts->GetItem(k,X2,Y2);
        else
            pts->GetItem(0,X2,Y2);
        double curLen=sqrt((X1-X2)*(X1-X2)+(Y1-Y2)*(Y1-Y2));
        if(len+curLen>=toLen)
        {
            double difLen=len+curLen-toLen;
            double difper=1-difLen/curLen;
            dpt.X=(X2-X1)*difper+X1;
            dpt.Y=(Y2-Y1)*difper+Y1;
            return dpt;
        }
        len+=curLen;
        X1=X2;
        Y1=Y2;
    }
    return dpt;
}
double RingTopo::GetPercentage(GeometryIntersectPoint gc)
{
    AutoPtr<Points>pts=geometry;
    AutoPtr<Ring>ring=pts;
    Point2D<double> dpt=gc.dpt;
    LONG ptsCount;
    ptsCount=pts->GetSize();
    if(ptsCount<2) return 0;
    double allLength=ring->GetPerimeter();
    DOUBLE X1,Y1,X2,Y2;
    pts->GetItem(0,X1,Y1);
    double len=0;
    for(int k=1;k<=ptsCount;k++)
    {
        if(k<ptsCount)
            pts->GetItem(k,X2,Y2);
        else
            pts->GetItem(0,X2,Y2);
        double curLen=sqrt((X1-X2)*(X1-X2)+(Y1-Y2)*(Y1-Y2));
        if(gc.pointIndex==k-1)
        {
            double dist1=sqrt((X1-dpt.X)*(X1-dpt.X)+(Y1-dpt.Y)*(Y1-dpt.Y));
            return (len+dist1)/allLength;
        }
        len+=curLen;
        X1=X2;
        Y1=Y2;
    }
    return 1;
}

bool RingTopo::CanDeleteVertex(GeometryCapture gc){
    AutoPtr<Points>pts=geometry;
    int ptCount=pts->GetSize();
    double*coords=pts->GetXYs();
    bool isSameFrom=((coords[0]==coords[ptCount*2-2])&&(coords[1]==coords[ptCount*2-1]));
    if(ptCount<4+(isSameFrom?1:0)) return false;
    return true;
}
bool RingTopo::DeleteVertex(GeometryCapture gc){
    AutoPtr<Points>pts=geometry;
    int ptCount=pts->GetSize();
    double*coords=pts->GetXYs();
    bool isSameFrom=((coords[0]==coords[ptCount*2-2])&&(coords[1]==coords[ptCount*2-1]));
    if(ptCount<4+(isSameFrom?1:0)) return false;
    if(isSameFrom){
        if((gc.pointIndex==0)||(gc.pointIndex==ptCount-1)){
            pts->Remove(0);
            ptCount=pts->GetSize();
            coords[ptCount*2-2]=coords[0];
            coords[ptCount*2-1]=coords[1];
        }
        else
            pts->Remove(gc.pointIndex);
    }
    else
        pts->Remove(gc.pointIndex);
    return true;
}

Point2D<double> PolylineTopo::GetVertex(GeometryCapture gc)
{
    Point2D<double> dpt;
    AutoPtr<Polyline>polyline=geometry;
    if(polyline==nullptr) return dpt;
    AutoPtr<Path>pPath=polyline->GetItem(gc.partIndex);
    PathTopo pathTopo(pPath);
    return pathTopo.GetVertex(gc);
}
Segment2D<double> PolylineTopo::GetSegment(GeometryCapture gc)
{
    Segment2D<double> seg;
    AutoPtr<Polyline>polyline=geometry;
    if(polyline==nullptr) return seg;
    AutoPtr<Path>pPath=polyline->GetItem(gc.partIndex);
    PathTopo pathTopo(pPath);
    return pathTopo.GetSegment(gc);
}
GeometryCapture PolylineTopo::CaptureVertex(DOUBLE x,DOUBLE y,DOUBLE tolerance)
{
    GeometryCapture pc;
    AutoPtr<Polyline>polyline=geometry;
    if(polyline==nullptr) return pc;
    LONG polyCount=polyline->GetSize();
    double mindist;
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Path>pPath=polyline->GetItem(k);
        AutoPtr<Points>pts=pPath;
        PathTopo pathTopo(pts);
        GeometryCapture gc=pathTopo.CaptureVertex(x,y,tolerance);
        if(gc.pointIndex<0) continue;
        Point2D<double> dpt=pathTopo.GetVertex(gc);
        double dist=dpt.DistanceTo(x,y);
        if(pc.partIndex==-1)
        {
           pc=gc;
           pc.partIndex=k;
           mindist=dist;
        }
        else if(dist<mindist)
        {
            pc=gc;
            pc.partIndex=k;
            mindist=dist;
        }
    }
    return pc;
}
GeometryCapture PolylineTopo::CaptureVertex(DOUBLE x,DOUBLE y)
{
    GeometryCapture pc;
    AutoPtr<Polyline>polyline=geometry;
    if(polyline==nullptr) return pc;
    LONG polyCount=polyline->GetSize();
    double mindist;
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Path>pPath=polyline->GetItem(k);
        AutoPtr<Points>pts=pPath;
        PathTopo pathTopo(pts);
        GeometryCapture gc=pathTopo.CaptureVertex(x,y);
        if(gc.pointIndex<0) continue;
        Point2D<double> dpt=pathTopo.GetVertex(gc);
        double dist=dpt.DistanceTo(x,y);
        if(pc.partIndex==-1)
        {
           pc=gc;
           pc.partIndex=k;
           mindist=dist;
        }
        else if(dist<mindist)
        {
            pc=gc;
            pc.partIndex=k;
            mindist=dist;
        }
    }
    return pc;
}
GeometryCapture PolylineTopo::CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt)
{
    GeometryCapture pc;
    AutoPtr<Polyline>polyline=geometry;
    if(polyline==nullptr) return pc;
    AutoPtr<CurvePolyline>curvePolyline=polyline;
    if(curvePolyline!=nullptr)
    {
        polyline=curvePolyline->ConvertToPolyline(tolerance);
        LONG polyCount=polyline->GetSize();
        double mindist;
        for(int k=0;k<polyCount;k++)
        {
            AutoPtr<Path>pPath=polyline->GetItem(k);
            AutoPtr<Points>pts=pPath;
            PathTopo pathTopo(pts);
            Point2D<double> dpt;
            GeometryCapture gc=pathTopo.CaptureSegment(x,y,tolerance,dpt);
            if(gc.pointIndex<0) continue;
            double dist=dpt.DistanceTo(x,y);
            bool hasUpdated=false;
            if(pc.partIndex==-1)
            {
               pc=gc;
               pc.partIndex=k;
               mindist=dist;
               pt=dpt;
               hasUpdated=true;
            }
            else if(dist<mindist)
            {
                pc=gc;
                pc.partIndex=k;
                mindist=dist;
                pt=dpt;
                hasUpdated=true;
            }
            if(hasUpdated)
            {
                AutoPtr<Polyline>opolyline=geometry;
                AutoPtr<Path>oPath=opolyline->GetItem(k);
                AutoPtr<Points>opts=oPath;
                LONG oldptsCount=opts->GetSize();
                LONG ptsCount;
                ptsCount=pts->GetSize();
                int originIndex=0;
                int oi=pc.pointIndex;
                pc.pointIndex=0;
                bool hasFind=false;
                for(int p=0;p<oldptsCount;p++)
                {
                    double ox,oy;
                    opts->GetItem(p,ox,oy);
                    for(int l=originIndex;l<ptsCount;l++)
                    {
                        if(l==oi)
                        {
                            pc.pointIndex=p-1;
                            if(pc.pointIndex<0) pc.partIndex=0;
                            hasFind=true;
                            break;
                        }
                        double nx,ny;
                        pts->GetItem(l,nx,ny);
                        if((ox==nx)&&(oy==ny))
                        {
                            originIndex=l+1;
                            break;
                        }
                    }
                    if(hasFind) break;
                }
            }
        }
    }
    else
    {
        LONG polyCount=polyline->GetSize();
        double mindist;
        for(int k=0;k<polyCount;k++)
        {
            AutoPtr<Path>pPath=polyline->GetItem(k);
            AutoPtr<Points>pts=pPath;
            PathTopo pathTopo(pts);
            Point2D<double> dpt;
            GeometryCapture gc=pathTopo.CaptureSegment(x,y,tolerance,dpt);
            if(gc.pointIndex<0) continue;
            double dist=dpt.DistanceTo(x,y);
            if(pc.partIndex==-1)
            {
               pc=gc;
               pc.partIndex=k;
               mindist=dist;
               pt=dpt;
            }
            else if(dist<mindist)
            {
                pc=gc;
                pc.partIndex=k;
                mindist=dist;
                pt=dpt;
            }
        }
    }
    return pc;
}

void PolylineTopo::MoveVertex(GeometryCapture pc,DOUBLE x,DOUBLE y)
{
    AutoPtr<Polyline>polyline=geometry;
    if(polyline==nullptr) return;
    AutoPtr<Path>pPath=polyline->GetItem(pc.partIndex);
    AutoPtr<Points>points=pPath;
    PathTopo topo(points);
    topo.MoveVertex(pc,x,y);
}
void PolylineTopo::InsertVertex(GeometryCapture gc,DOUBLE x,DOUBLE y)
{
    AutoPtr<Polyline>polyline=geometry;
    if(polyline==nullptr) return;
    AutoPtr<Path>pPath=polyline->GetItem(gc.partIndex);
    AutoPtr<Points>points=pPath;
    PathTopo topo(points);
    topo.InsertVertex(gc,x,y);
}
AutoPtr<Points> PolylineTopo::GetPoints(GeometryCapture gc)
{
    AutoPtr<Polyline>polyline=geometry;
    if(polyline==nullptr) return nullptr;
    LONG count=polyline->GetSize();
    if((gc.partIndex<0)||(gc.partIndex>=count)) return nullptr;
    AutoPtr<Path>pPath=polyline->GetItem(gc.partIndex);
    return pPath;
}
void PolylineTopo::IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points)
{
    AutoPtr<Polyline>polyline=geometry;
    if(polyline==nullptr) return;
    LONG polyCount=polyline->GetSize();
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Path>pPath=polyline->GetItem(k);
        AutoPtr<Points>pts=pPath;
        PathTopo pathTopo(pts);
        pathTopo.IntersectSegment(seg,points);
    }
}

bool PolylineTopo::CanDeleteVertex(GeometryCapture gc){
    AutoPtr<Polyline>polyline=geometry;
    AutoPtr<Path>pPath=polyline->GetItem(gc.partIndex);
    AutoPtr<PathTopo>topo=new PathTopo(pPath);
    return topo->CanDeleteVertex(gc);
}
bool PolylineTopo::DeleteVertex(GeometryCapture gc){
    AutoPtr<Polyline>polyline=geometry;
    AutoPtr<Path>pPath=polyline->GetItem(gc.partIndex);
    AutoPtr<PathTopo>topo=new PathTopo(pPath);
    return topo->DeleteVertex(gc);
}
Point2D<double> PolygonTopo::GetVertex(GeometryCapture gc)
{
    Point2D<double> dpt;
    AutoPtr<Polygon>polygon=geometry;
    if(polygon==nullptr) return dpt;
    AutoPtr<Ring>pPath=polygon->GetItem(gc.partIndex);
    RingTopo pathTopo(pPath);
    return pathTopo.GetVertex(gc);
}
Segment2D<double> PolygonTopo::GetSegment(GeometryCapture gc)
{
    Segment2D<double> seg;
    AutoPtr<Polygon>polygon=geometry;
    if(polygon==nullptr) return seg;
    AutoPtr<Ring>pPath=polygon->GetItem(gc.partIndex);
    RingTopo pathTopo(pPath);
    return pathTopo.GetSegment(gc);
}
GeometryCapture PolygonTopo::CaptureVertex(DOUBLE x,DOUBLE y,DOUBLE tolerance)
{
    GeometryCapture pc;
    AutoPtr<Polygon>polygon=geometry;
    if(polygon==nullptr) return pc;
    LONG polyCount=polygon->GetSize();
    double mindist;
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Ring>pPath=polygon->GetItem(k);
        AutoPtr<Points>pts=pPath;
        RingTopo pathTopo(pts);
        GeometryCapture gc=pathTopo.CaptureVertex(x,y,tolerance);
        if(gc.pointIndex<0) continue;
        Point2D<double> dpt=pathTopo.GetVertex(gc);
        double dist=dpt.DistanceTo(x,y);
        if(pc.partIndex==-1)
        {
           pc=gc;
           pc.partIndex=k;
           mindist=dist;
        }
        else if(dist<mindist)
        {
            pc=gc;
            pc.partIndex=k;
            mindist=dist;
        }
    }
    return pc;
}
GeometryCapture PolygonTopo::CaptureVertex(DOUBLE x,DOUBLE y)
{
    GeometryCapture pc;
    AutoPtr<Polygon>polygon=geometry;
    if(polygon==nullptr) return pc;
    LONG polyCount=polygon->GetSize();
    double mindist;
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Ring>pPath=polygon->GetItem(k);
        AutoPtr<Points>pts=pPath;
        RingTopo pathTopo(pts);
        GeometryCapture gc=pathTopo.CaptureVertex(x,y);
        if(gc.pointIndex<0) continue;
        Point2D<double> dpt=pathTopo.GetVertex(gc);
        double dist=dpt.DistanceTo(x,y);
        if(pc.partIndex==-1)
        {
           pc=gc;
           pc.partIndex=k;
           mindist=dist;
        }
        else if(dist<mindist)
        {
            pc=gc;
            pc.partIndex=k;
            mindist=dist;
        }
    }
    return pc;
}
GeometryCapture PolygonTopo::CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt)
{
    GeometryCapture pc;
    AutoPtr<Polygon>polygon=geometry;
    if(polygon==nullptr) return pc;
    AutoPtr<CurvePolygon>curvePolygon=polygon;
    if(curvePolygon!=nullptr)
    {
        polygon=curvePolygon->ConvertToPolygon(tolerance);
        LONG polyCount=polygon->GetSize();
        double mindist;
        for(int k=0;k<polyCount;k++)
        {
            AutoPtr<Ring>pPath=polygon->GetItem(k);
            AutoPtr<Points>pts=pPath;
            RingTopo pathTopo(pts);
            Point2D<double> dpt;
            GeometryCapture gc=pathTopo.CaptureSegment(x,y,tolerance,dpt);
            if(gc.pointIndex<0) continue;
            double dist=dpt.DistanceTo(x,y);
            bool hasUpdated=false;
            if(pc.partIndex==-1)
            {
               pc=gc;
               pc.partIndex=k;
               mindist=dist;
               pt=dpt;
               hasUpdated=true;
            }
            else if(dist<mindist)
            {
                pc=gc;
                pc.partIndex=k;
                mindist=dist;
                pt=dpt;
                hasUpdated=true;
            }
            if(hasUpdated)
            {
                AutoPtr<Polygon>opolygon=geometry;
                AutoPtr<Ring>oPath=opolygon->GetItem(k);
                AutoPtr<Points>opts=oPath;
                LONG oldptsCount=opts->GetSize();
                LONG ptsCount;
                ptsCount=pts->GetSize();
                int originIndex=0;
                int oi=pc.pointIndex;
                pc.pointIndex=0;
                bool hasFind=false;
                for(int p=0;p<oldptsCount;p++)
                {
                    double ox,oy;
                    opts->GetItem(p,ox,oy);
                    for(int l=originIndex;l<ptsCount;l++)
                    {
                        if(oi==l)
                        {
                            pc.pointIndex=p-1;
                            if(pc.pointIndex<0) pc.partIndex=0;
                            hasFind=true;
                            break;
                        }
                        double nx,ny;
                        pts->GetItem(l,nx,ny);
                        if((ox==nx)&&(oy==ny))
                        {
                            originIndex=l+1;
                            break;
                        }
                    }
                    if(hasFind) break;
                }
            }
        }
    }
    else
    {
        LONG polyCount=polygon->GetSize();
        double mindist;
        for(int k=0;k<polyCount;k++)
        {
            AutoPtr<Ring>pPath=polygon->GetItem(k);
            AutoPtr<Points>pts=pPath;
            RingTopo pathTopo(pts);
            Point2D<double> dpt;
            GeometryCapture gc=pathTopo.CaptureSegment(x,y,tolerance,dpt);
            if(gc.pointIndex<0) continue;
            double dist=dpt.DistanceTo(x,y);
            if(pc.partIndex==-1)
            {
               pc=gc;
               pc.partIndex=k;
               mindist=dist;
               pt=dpt;
            }
            else if(dist<mindist)
            {
                pc=gc;
                pc.partIndex=k;
                mindist=dist;
                pt=dpt;
            }
        }
    }
    return pc;
}
void PolygonTopo::MoveVertex(GeometryCapture pc,DOUBLE x,DOUBLE y)
{
    AutoPtr<Polygon>polygon=geometry;
    if(polygon==nullptr) return;
    AutoPtr<Ring>pPath=polygon->GetItem(pc.partIndex);
    AutoPtr<Points>points=pPath;
    RingTopo topo(points);
    topo.MoveVertex(pc,x,y);
}
void PolygonTopo::InsertVertex(GeometryCapture gc,DOUBLE x,DOUBLE y)
{
    AutoPtr<Polygon>polygon=geometry;
    if(polygon==nullptr) return;
    AutoPtr<Ring>pPath=polygon->GetItem(gc.partIndex);
    AutoPtr<Points>points=pPath;
    PathTopo topo(points);
    topo.InsertVertex(gc,x,y);
}
AutoPtr<Points> PolygonTopo::GetPoints(GeometryCapture gc)
{
    AutoPtr<Polygon>polygon=geometry;
    if(polygon==nullptr) return nullptr;
    LONG count=polygon->GetSize();
    if((gc.partIndex<0)||(gc.partIndex>=count)) return nullptr;
    AutoPtr<Ring>pPath=polygon->GetItem(gc.partIndex);
    return pPath;
}
void PolygonTopo::IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points)
{
    AutoPtr<Polygon>polygon=geometry;
    if(polygon==nullptr) return;
    LONG polyCount=polygon->GetSize();
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Ring>pPath=polygon->GetItem(k);
        AutoPtr<Points>pts=pPath;
        RingTopo pathTopo(pts);
        pathTopo.IntersectSegment(seg,points);
    }
}

bool PolygonTopo::CanDeleteVertex(GeometryCapture gc){
    AutoPtr<Polygon>poly=geometry;
    AutoPtr<Ring>pPath=poly->GetItem(gc.partIndex);
    AutoPtr<RingTopo>topo=new RingTopo(pPath);
    return topo->CanDeleteVertex(gc);
}
bool PolygonTopo::DeleteVertex(GeometryCapture gc){
    AutoPtr<Polygon>poly=geometry;
    AutoPtr<Ring>pPath=poly->GetItem(gc.partIndex);
    AutoPtr<RingTopo>topo=new RingTopo(pPath);
    return topo->DeleteVertex(gc);
}

Point2D<double> MultiPolygonTopo::GetVertex(GeometryCapture gc)
{
    Point2D<double> dpt;
    AutoPtr<MultiPolygon>polygon=geometry;
    if(polygon==nullptr) return dpt;
    AutoPtr<Polygon>poly=polygon->GetItem(gc.polyIndex);
    PolygonTopo polyTopo(poly);
    return polyTopo.GetVertex(gc);
}
Segment2D<double> MultiPolygonTopo::GetSegment(GeometryCapture gc)
{
    Segment2D<double> seg;
    AutoPtr<MultiPolygon>polygon=geometry;
    if(polygon==nullptr) return seg;
    AutoPtr<Polygon>poly=polygon->GetItem(gc.polyIndex);
    PolygonTopo polyTopo(poly);
    return polyTopo.GetSegment(gc);
}
GeometryCapture MultiPolygonTopo::CaptureVertex(DOUBLE x,DOUBLE y,DOUBLE tolerance)
{
    GeometryCapture pc;
    AutoPtr<MultiPolygon>polygon=geometry;
    if(polygon==nullptr) return pc;
    LONG polyCount=polygon->GetSize();
    double mindist;
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Polygon>poly=polygon->GetItem(k);
        PolygonTopo polyTopo(poly);
        GeometryCapture gc=polyTopo.CaptureVertex(x,y,tolerance);
        if(gc.pointIndex<0) continue;
        Point2D<double> dpt=polyTopo.GetVertex(gc);
        double dist=dpt.DistanceTo(x,y);
        if(pc.partIndex==-1)
        {
           pc=gc;
           pc.polyIndex=k;
           mindist=dist;
        }
        else if(dist<mindist)
        {
            pc=gc;
            pc.polyIndex=k;
            mindist=dist;
        }
    }
    return pc;
}
GeometryCapture MultiPolygonTopo::CaptureVertex(DOUBLE x,DOUBLE y)
{
    GeometryCapture pc;
    AutoPtr<MultiPolygon>polygon=geometry;
    if(polygon==nullptr) return pc;
    LONG polyCount=polygon->GetSize();
    double mindist;
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Polygon>poly=polygon->GetItem(k);
        PolygonTopo polyTopo(poly);
        GeometryCapture gc=polyTopo.CaptureVertex(x,y);
        if(gc.pointIndex<0) continue;
        Point2D<double> dpt=polyTopo.GetVertex(gc);
        double dist=dpt.DistanceTo(x,y);
        if(pc.partIndex==-1)
        {
           pc=gc;
           pc.polyIndex=k;
           mindist=dist;
        }
        else if(dist<mindist)
        {
            pc=gc;
            pc.polyIndex=k;
            mindist=dist;
        }
    }
    return pc;
}
GeometryCapture MultiPolygonTopo::CaptureSegment(DOUBLE x,DOUBLE y,DOUBLE tolerance,Point2D<double>&pt)
{
    GeometryCapture pc;
    AutoPtr<MultiPolygon>polygon=geometry;
    if(polygon==nullptr) return pc;
    LONG polyCount=polygon->GetSize();
    double mindist;
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Polygon>poly=polygon->GetItem(k);
        PolygonTopo polyTopo(poly);
        Point2D<double> dpt;
        GeometryCapture gc=polyTopo.CaptureSegment(x,y,tolerance,dpt);
        if(gc.pointIndex<0) continue;
        double dist=dpt.DistanceTo(x,y);
        if(pc.partIndex==-1)
        {
           pc=gc;
           pc.polyIndex=k;
           mindist=dist;
           pt=dpt;
        }
        else if(dist<mindist)
        {
            pc=gc;
            pc.polyIndex=k;
            mindist=dist;
            pt=dpt;
        }
    }
    return pc;
}
void MultiPolygonTopo::MoveVertex(GeometryCapture pc,DOUBLE x,DOUBLE y)
{
    AutoPtr<MultiPolygon>polygon=geometry;
    if(polygon==nullptr) return;
    AutoPtr<Polygon>poly=polygon->GetItem(pc.polyIndex);
    PolygonTopo topo(poly);
    topo.MoveVertex(pc,x,y);
}
void MultiPolygonTopo::InsertVertex(GeometryCapture gc,DOUBLE x,DOUBLE y)
{
    AutoPtr<MultiPolygon>polygon=geometry;
    if(polygon==nullptr) return;
    AutoPtr<Polygon>poly=polygon->GetItem(gc.polyIndex);
    PolygonTopo topo(poly);
    topo.InsertVertex(gc,x,y);
}
AutoPtr<Points> MultiPolygonTopo::GetPoints(GeometryCapture gc)
{
    AutoPtr<MultiPolygon>polygon=geometry;
    if(polygon==NULL) return nullptr;
    LONG count=polygon->GetSize();
    if((gc.polyIndex<0)||(gc.polyIndex>=count)) return nullptr;
    AutoPtr<Polygon>poly=polygon->GetItem(gc.polyIndex);
    PolygonTopo topo(poly);
    return topo.GetPoints(gc);
}
void MultiPolygonTopo::IntersectSegment(Segment2D<double> seg,GeometryIntersectPoints&points)
{
    AutoPtr<MultiPolygon>polygon=geometry;
    if(polygon==nullptr) return;
    LONG polyCount=polygon->GetSize();
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Polygon>poly=polygon->GetItem(k);
        PolygonTopo polyTopo(poly);
        polyTopo.IntersectSegment(seg,points);
    }
}

bool MultiPolygonTopo::CanDeleteVertex(GeometryCapture gc){
    AutoPtr<MultiPolygon>mpoly=geometry;
    AutoPtr<Polygon>poly=mpoly->GetItem(gc.partIndex);
    AutoPtr<PolygonTopo>topo=new PolygonTopo(poly);
    return topo->CanDeleteVertex(gc);
}
bool MultiPolygonTopo::DeleteVertex(GeometryCapture gc){
    AutoPtr<MultiPolygon>mpoly=geometry;
    AutoPtr<Polygon>poly=mpoly->GetItem(gc.partIndex);
    AutoPtr<PolygonTopo>topo=new PolygonTopo(poly);
    return topo->DeleteVertex(gc);
}

GeometryBorderCapture::GeometryBorderCapture(){
    X=0;
    Y=0;
}
GeometryBorderCapture::~GeometryBorderCapture(){

}
AutoPtr<Point>GeometryBorderCapture::GetPosition(){
   return new Point(X,Y);
}
void GeometryBorderCapture::SetPosition(AutoPtr<Point>pos){
   pos->GetCoord(&X,&Y);
}
void GeometryBorderCapture::SetPosition(double x,double y){
   X=x;
   Y=y;
}
GeometryBorderCaptureData GeometryBorderCapture::CaptureVertex(Geometry*geo){
    AutoPtr<GeometryTopo>topo=GeometryTopo::CreateGeometryTopo(AutoPtr<Geometry>(geo,true));
    GeometryBorderCaptureData captureData;
    captureData.CaptureType=gbctNone;
    captureData.FeatureID=-1;
    if(topo==NULL)
    {
        captureData.PolygonIndex=-1;
        captureData.PartIndex=-1;
        captureData.PointIndex=-1;
        captureData.CapturedX=captureData.CapturedY=0;
        return captureData;
    }
    GeometryCapture gc=topo->CaptureVertex(X,Y);
    if(gc.pointIndex==-1)
    {
        captureData.PolygonIndex=-1;
        captureData.PartIndex=-1;
        captureData.PointIndex=-1;
        captureData.CapturedX=captureData.CapturedY=0;
    }
    else
    {
        captureData.CaptureType=gbctVertex;
        captureData.PolygonIndex=gc.polyIndex;
        captureData.PartIndex=gc.partIndex;
        captureData.PointIndex=gc.pointIndex;
        Point2D<double> dpt=topo->GetVertex(gc);
        captureData.CapturedX=dpt.X;
        captureData.CapturedY=dpt.Y;
    }
    return captureData;
}
GeometryBorderCaptureData GeometryBorderCapture::CaptureVertex(Geometry*geo,double toler){
    AutoPtr<GeometryTopo>topo=GeometryTopo::CreateGeometryTopo(AutoPtr<Geometry>(geo,true));
    GeometryBorderCaptureData captureData;
    captureData.CaptureType=gbctNone;
    captureData.FeatureID=-1;
    if(topo==NULL)
    {
        captureData.PolygonIndex=-1;
        captureData.PartIndex=-1;
        captureData.PointIndex=-1;
        captureData.CapturedX=captureData.CapturedY=0;
        return captureData;
    }
    GeometryCapture gc=topo->CaptureVertex(X,Y,toler);
    if(gc.pointIndex==-1)
    {
        captureData.PolygonIndex=-1;
        captureData.PartIndex=-1;
        captureData.PointIndex=-1;
        captureData.CapturedX=captureData.CapturedY=0;
    }
    else
    {
        captureData.CaptureType=gbctVertex;
        captureData.PolygonIndex=gc.polyIndex;
        captureData.PartIndex=gc.partIndex;
        captureData.PointIndex=gc.pointIndex;
        Point2D<double> dpt=topo->GetVertex(gc);
        captureData.CapturedX=dpt.X;
        captureData.CapturedY=dpt.Y;
    }
    return captureData;
}
GeometryBorderCaptureData GeometryBorderCapture::CaptureSegment(Geometry*geo,double toler){
    AutoPtr<GeometryTopo>topo=GeometryTopo::CreateGeometryTopo(AutoPtr<Geometry>(geo,true));
    GeometryBorderCaptureData captureData;
    captureData.CaptureType=gbctNone;
    captureData.FeatureID=-1;
    if(topo==NULL)
    {
        captureData.PolygonIndex=-1;
        captureData.PartIndex=-1;
        captureData.PointIndex=-1;
        captureData.CapturedX=captureData.CapturedY=0;
        return captureData;
    }
    Point2D<double> dpt;
    GeometryCapture gc=topo->CaptureSegment(X,Y,toler,dpt);
    if(gc.pointIndex==-1)
    {
        captureData.PolygonIndex=-1;
        captureData.PartIndex=-1;
        captureData.PointIndex=-1;
        captureData.CapturedX=captureData.CapturedY=0;
    }
    else
    {
        captureData.CaptureType=gbctSegment;
        captureData.PolygonIndex=gc.polyIndex;
        captureData.PartIndex=gc.partIndex;
        captureData.PointIndex=gc.pointIndex;
        captureData.CapturedX=dpt.X;
        captureData.CapturedY=dpt.Y;
    }
    return captureData;
}

GeometryBorderCaptureData GeometryBorderCapture::CaptureBorder(Geometry*geo,double toler){
    GeometryBorderCaptureData captureData=CaptureVertex(geo,toler);
    if(captureData.CaptureType!=GeometryBorderCaptureType::gbctVertex) captureData=CaptureSegment(geo,toler);
    return captureData;
}

void GeometryBorderCapture::MoveVertex(Geometry*geo,GeometryBorderCaptureData capdata,double x,double y){
    GeometryCapture gc;
    gc.polyIndex=capdata.PolygonIndex;
    gc.partIndex=capdata.PartIndex;
    gc.pointIndex=capdata.PointIndex;
    AutoPtr<GeometryTopo>topo=GeometryTopo::CreateGeometryTopo(AutoPtr<Geometry>(geo,true));
    topo->MoveVertex(gc,x,y);
}

void GeometryBorderCapture::InsertVertex(Geometry*geo,GeometryBorderCaptureData capdata,double x,double y){
    GeometryCapture gc;
    gc.polyIndex=capdata.PolygonIndex;
    gc.partIndex=capdata.PartIndex;
    gc.pointIndex=capdata.PointIndex;
    AutoPtr<GeometryTopo>topo=GeometryTopo::CreateGeometryTopo(AutoPtr<Geometry>(geo,true));
    topo->InsertVertex(gc,x,y);
}

bool GeometryBorderCapture::CanDeleteVertex(Geometry*geo, GeometryBorderCaptureData capdata){
    GeometryCapture gc;
    gc.polyIndex=capdata.PolygonIndex;
    gc.partIndex=capdata.PartIndex;
    gc.pointIndex=capdata.PointIndex;
    AutoPtr<GeometryTopo>topo=GeometryTopo::CreateGeometryTopo(AutoPtr<Geometry>(geo,true));
    return topo->CanDeleteVertex(gc);
}

bool GeometryBorderCapture::DeleteVertex(Geometry*geo,GeometryBorderCaptureData capdata){
    GeometryCapture gc;
    gc.polyIndex=capdata.PolygonIndex;
    gc.partIndex=capdata.PartIndex;
    gc.pointIndex=capdata.PointIndex;
    AutoPtr<GeometryTopo>topo=GeometryTopo::CreateGeometryTopo(AutoPtr<Geometry>(geo,true));
    return topo->DeleteVertex(gc);
}

}

