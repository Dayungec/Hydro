#include "topobuilder.h"
#include "segment.h"
#include "polyline.h"
#include "geometrycollection.h"


namespace SGIS{

TopoPoint::TopoPoint()
{
    father=nullptr;
    child=nullptr;
}
TopoPoint::TopoPoint(double x,double y)
{
    X=x;
    Y=y;
    father=nullptr;
    child=nullptr;
}
TopoPoint::~TopoPoint()
{

}
bool TopoPoint::IsSamePoint(TopoPoint*other,double fuzzy)
{
    return ((fabs(X-other->X)<=fuzzy)&&(fabs(Y-other->Y)<=fuzzy));
}
bool TopoPoint::IsSamePoint(Point2D<double> dpt,double fuzzy)
{
    return ((fabs(X-dpt.X)<=fuzzy)&&(fabs(Y-dpt.Y)<=fuzzy));
}
void TopoPoint::RemoveLine(TopoLine*line)
{
    for(int k=lines.size()-1;k>=0;k--)
    {
        if(lines[k]==line)
        {
            lines.erase(begin(lines)+k);
            break;
        }
    }
}
void TopoPoint::AddLine(TopoLine*line)
{
    for(int k=lines.size()-1;k>=0;k--)
    {
        if(lines[k]==line)
        {
            return;
        }
    }
    lines.push_back(line);
}
bool TopoPoint::HasMultipleAttributes()
{
    int lnSize=lines.size();
    if(lnSize==0) return false;
    int formerat=0;
    int valid=0;
    for(int k=0;k<lnSize;k++)
    {
        TopoLine*tLine=lines[k];
        int atSize=tLine->GetAttributesCount();
        if(atSize==0) continue;
        if(atSize>1) return true;
        if(valid==0)
            formerat=tLine->GetAttribute(0);
        else if(formerat!=tLine->GetAttribute(0))
            return true;
        valid++;
    }
    return false;
}
bool TopoPoint::FindLine(TopoPoint*other)
{
    for(int k=lines.size()-1;k>=0;k--)
    {
        TopoLine*tpLine=lines[k];
        if((tpLine->fromNode==other)||(tpLine->toNode==other))
            return true;
    }
    return false;
}

TopoLine::TopoLine()
{
    father=nullptr;
    child=nullptr;
    state=0;
    leftRing=rightRing=nullptr;
}
TopoLine::TopoLine(int attribute)
{
    father=nullptr;
    child=nullptr;
    state=0;
    attributes.push_back(attribute);
    leftRing=rightRing=nullptr;
}
TopoLine::TopoLine(TopoPoint*fN,TopoPoint*tN)
{
    father=nullptr;
    child=nullptr;
    fromNode=fN;
    toNode=tN;
    fromNode->AddLine(this);
    toNode->AddLine(this);
    state=0;
    leftRing=rightRing=nullptr;
}
TopoLine::TopoLine(TopoPoint*fN,TopoPoint*tN,int attribute)
{
    father=nullptr;
    child=nullptr;
    fromNode=fN;
    toNode=tN;
    fromNode->AddLine(this);
    toNode->AddLine(this);
    state=0;
    attributes.push_back(attribute);
    leftRing=rightRing=nullptr;
}
TopoLine::~TopoLine()
{

}

int TopoLine::GetAttributesCount()
{
    return attributes.size();
}
int TopoLine::GetAttribute(int nIndex)
{
    return attributes[nIndex];
}
void TopoLine::AddAttributes(int at)
{
    for(int k=attributes.size()-1;k>=0;k--)
    {
        if(attributes[k]==at) return;
        if(attributes[k]==-at)
        {
            attributes.erase(begin(attributes)+k);
            return;
        }
    }
    attributes.push_back(at);
};
bool TopoLine::FindAttributes(int at)
{
    for(int k=attributes.size()-1;k>=0;k--)
    {
        if(attributes[k]==at) return true;
    }
    return false;
}
int TopoLine::GetMinPositiveAttribute()
{
    int minV=0;
    for(int k=attributes.size()-1;k>=0;k--)
    {
        if(attributes[k]>0)
        {
            if((minV==0)||(minV>attributes[k])) minV=attributes[k];
        }
    }
    return minV;
}
int TopoLine::GetMaxNegtiveAttribute()
{
    int maxV=0;
    for(int k=attributes.size()-1;k>=0;k--)
    {
        if(attributes[k]<0)
        {
            if((maxV==0)||(maxV<attributes[k])) maxV=attributes[k];
        }
    }
    return maxV;
}
int TopoLine::GetMaxAbsAttribute(){
    int maxV=0;
    for(int k=attributes.size()-1;k>=0;k--)
    {
        int v=abs(attributes[k]);
        if(maxV<v) maxV=v;
    }
    return maxV;
}
void TopoLine::CombineAttributes(TopoLine*otherLine,bool negative)
{
    if(negative)
    {
        for(int k=otherLine->attributes.size()-1;k>=0;k--)
        {
            AddAttributes(-otherLine->attributes[k]);
        }
    }
    else
    {
        for(int k=otherLine->attributes.size()-1;k>=0;k--)
        {
            AddAttributes(otherLine->attributes[k]);
        }
    }
};
void TopoLine::CopyAttributes(TopoLine*otherLine)
{
    attributes.clear();
    for(int k=otherLine->attributes.size()-1;k>=0;k--)
    {
        attributes.push_back(otherLine->attributes[k]);
    }
}
void TopoLine::CombineFromPoint(TopoPoint*point)
{
    if((fromNode==point)||(toNode==point)) return;
    int lineSize=point->lines.size();
    for(int k=0;k<lineSize;k++)
    {
        TopoLine*tLine=point->lines[k];
        if(tLine->fromNode==point)
            tLine->fromNode=fromNode;
        else
            tLine->toNode=fromNode;
        fromNode->AddLine(tLine);
    }
    point->lines.clear();
}
void TopoLine::CombineToPoint(TopoPoint*point)
{
    if((fromNode==point)||(toNode==point)) return;
    int lineSize=point->lines.size();
    for(int k=0;k<lineSize;k++)
    {
        TopoLine*tLine=point->lines[k];
        if(tLine->fromNode==point)
            tLine->fromNode=toNode;
        else
            tLine->toNode=toNode;
        toNode->AddLine(tLine);
    }
    point->lines.clear();
}
void TopoLine::SetFromNode(TopoPoint*point)
{
    fromNode->RemoveLine(this);
    fromNode=point;
    fromNode->AddLine(this);
}
void TopoLine::SetToNode(TopoPoint*point)
{
    toNode->RemoveLine(this);
    toNode=point;
    toNode->AddLine(this);
}
AutoPtr<Envelope>TopoLine::ComputeEnvelope()
{
    double xmin,ymin,xmax,ymax;
    xmin=min(fromNode->X,toNode->X);
    xmax=max(fromNode->X,toNode->X);
    ymin=min(fromNode->Y,toNode->Y);
    ymax=max(fromNode->Y,toNode->Y);
    for(int k=vertices.size()-1;k>=0;k--)
    {
        Point2D<double> vertex=vertices[k];
        if(vertex.X<xmin) xmin=vertex.X;
        if(vertex.X>xmax) xmax=vertex.X;
        if(vertex.Y<ymin) ymin=vertex.Y;
        if(vertex.Y>ymax) ymax=vertex.Y;
    }
    Envelope*pEnv=new Envelope();
    pEnv->PutCoord(xmin,ymax,xmax,ymin);
    return pEnv;
}

AutoPtr<Polyline>TopoLine::CreatePolyline(){
    int pointCount=vertices.size()+2;
    AutoPtr<Polyline>polyline(new Polyline());
    AutoPtr<Points>points(new Points());
    points->SetSize(pointCount);
    DOUBLE*XYs=points->GetXYs();
    XYs[0]=fromNode->X;
    XYs[1]=fromNode->Y;
    if(vertices.size()>0)
    {
        DOUBLE*rawData=(DOUBLE*)vertices.data();
        memcpy(((BYTE*)XYs)+sizeof(double)*2,rawData,sizeof(double)*vertices.size()*2);
    }
    XYs[(vertices.size()+1)*2]=toNode->X;
    XYs[(vertices.size()+1)*2+1]=toNode->Y;
    polyline->Add(points);
    return polyline;
}
LineRegionElement::LineRegionElement(LineRegion*region,TopoLine*tLine)
{
    father=nullptr;
    child=nullptr;
    topoLine=tLine;
    lineRegion=region;
}
LineRegionElement::~LineRegionElement()
{

}

LineRegion::LineRegion()
{
    rootElement=nullptr;
    tailElement=nullptr;
    count=0;
}
LineRegion::~LineRegion()
{
    RemoveAll();
}
int LineRegion::GetCount()
{
    return count;
}
LineRegionElement*LineRegion::AddLine(TopoLine*tLine)
{
    LineRegionElement*el=new LineRegionElement(this,tLine);
    if(rootElement==nullptr)
    {
        rootElement=el;
        tailElement=el;
    }
    else
    {
        tailElement->child=el;
        el->father=tailElement;
        tailElement=el;
    }
    count++;
    return el;
}
void LineRegion::RemoveLine(LineRegionElement*el)
{
    if(el==rootElement) rootElement=el->child;
    if(el==tailElement) tailElement=tailElement->father;
    if(el->father!=nullptr) el->father->child=el->child;
    if(el->child!=nullptr) el->child->father=el->father;
    el->father=nullptr;
    el->child=nullptr;
    delete el;
}
void LineRegion::RemoveAll()
{
    LineRegionElement*cur=tailElement;
    while(cur!=nullptr)
    {
        LineRegionElement*temp=cur->father;
        delete cur;
        cur=temp;
    }
    rootElement=nullptr;
    tailElement=nullptr;
    count=0;
}


LineRegions::LineRegions(Envelope*fullExtent)
{
    left=0;
    top=0;
    right=0;
    bottom=0;
    if(fullExtent!=nullptr) fullExtent->GetCoord(&left,&top,&right,&bottom);
    xNum=50;
    yNum=50;
    regions.resize(xNum*yNum);
    for(int k=xNum*yNum-1;k>=0;k--)
    {
        LineRegion*orr=new LineRegion();
        regions[k]=orr;
    }
    InitialCells();
}
LineRegions::LineRegions(Envelope*fullExtent,int xSize,int ySize)
{
    left=0;
    top=0;
    right=0;
    bottom=0;
    if(fullExtent!=nullptr)  fullExtent->GetCoord(&left,&top,&right,&bottom);
    xNum=xSize;
    yNum=ySize;
    regions.resize(xNum*yNum);
    for(int k=xNum*yNum-1;k>=0;k--)
    {
        LineRegion*orr=new LineRegion();
        regions[k]=orr;
    }
    InitialCells();
}
LineRegions::~LineRegions()
{
    for(int k=regions.size()-1;k>=0;k--)
    {
        delete regions[k];
    }
    regions.clear();
}
int LineRegions::GetRegionXNum()
{
    return xNum;
}
int LineRegions::GetRegionYNum()
{
    return yNum;
}
LineRegion*LineRegions::GetRegion(int nIndex)
{
    return regions[nIndex];
}
int LineRegions::GetRegionX(double x)
{
    int xid=(x-left)/difx;
    if(xid<0) return 0;
    if(xid>=xNum) xid=xNum-1;
    return xid;
}
int LineRegions::GetRegionY(double y)
{
    int yid=(top-y)/dify;
    if(yid<0) return 0;
    if(yid>=yNum) yid=yNum-1;
    return yid;
}
int LineRegions::GetRealRegionX(double x)
{
    int xid=(x-left)/difx;
    return xid;
}
int LineRegions::GetRealRegionY(double y)
{
    int yid=(top-y)/dify;
    return yid;
}
bool LineRegions::IsSameRegion(TopoPoint*firstPoint,TopoPoint*secondPoint)
{
    if(GetRegionX(firstPoint->X)!=GetRegionX(secondPoint->X)) return false;
    if(GetRegionY(firstPoint->Y)!=GetRegionY(secondPoint->Y)) return false;
    return true;
}
void LineRegions::AppendToArray(AutoArray<Point2D<int>>&pArray,int ix,int iy){
    if((ix<0)||(ix>=xNum)||(iy<0)||(iy>=yNum)) return;
    if(pArray.GetSize()>0){
        Point2D<int>former=pArray[pArray.GetSize()-1];
        if((former.X==ix)&&(former.Y==iy)) return;
    }
    pArray.Add(Point2D<int>(ix,iy));
}

void LineRegions::InitialCells()
{
    difx=(right-left)/xNum;
    dify=(top-bottom)/yNum;
    for(int k=regions.size()-1;k>=0;k--)
    {
        regions[k]->RemoveAll();
    }
}
void LineRegions::GetLineRegions(TopoLine*tLine,AutoArray<Point2D<int>>&pArray)
{
    pArray.Clear();
    double x,y,x1,y1,x2,y2;
    x1=tLine->fromNode->X;
    y1=tLine->fromNode->Y;
    x2=tLine->toNode->X;
    y2=tLine->toNode->Y;
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
void LineRegions::RegisterLine(TopoLine*tLine)
{
    AutoArray<Point2D<int>> pArray;
    GetLineRegions(tLine,pArray);
    int count=pArray.GetSize();
    tLine->regionEls.resize(count);
    for(int k=0;k<count;k++)
    {
        Point2D<int>point=pArray.GetItem(k);
        LineRegionElement*el=regions[point.Y*xNum+point.X]->AddLine(tLine);
        tLine->regionEls[k]=el;
    }
}
void LineRegions::UpdateLine(TopoLine*tLine)
{
    UnregisterLine(tLine);
    RegisterLine(tLine);
}
void LineRegions::RegisterContour(TopoLine*tLine)
{
    double x1,y1,x2,y2;
    x1=tLine->fromNode->X;
    y1=tLine->fromNode->Y;
    x2=tLine->toNode->X;
    y2=tLine->toNode->Y;
    int ix1=GetRegionX(x1);
    int iy1=GetRegionY(y1);
    int ix2=GetRegionX(x2);
    int iy2=GetRegionY(y2);
    int num=1;
    if((ix2!=ix1)||(iy2!=iy1)) num=2;
    tLine->regionEls.resize(num);
    LineRegionElement*el=regions[iy1*xNum+ix1]->AddLine(tLine);
    tLine->regionEls[0]=el;
    if(num==2)
    {
        LineRegionElement*el=regions[iy2*xNum+ix2]->AddLine(tLine);
        tLine->regionEls[1]=el;
    }
}
void LineRegions::UnregisterLine(TopoLine*tLine)
{
    for(int k=tLine->regionEls.size()-1;k>=0;k--)
    {
        tLine->regionEls[k]->lineRegion->RemoveLine(tLine->regionEls[k]);
    }
    tLine->regionEls.clear();
}
void LineRegions::Clear()
{
    for(int k=regions.size()-1;k>=0;k--)
    {
        regions[k]->RemoveAll();
    }
}

TopoPoints::TopoPoints()
{
    rootPoint=nullptr;
    tailPoint=nullptr;
    pointCount=0;
}
TopoPoints::~TopoPoints()
{
    TopoPoint*cur=tailPoint;
    while(cur!=nullptr)
    {
         TopoPoint*temp=cur->father;
         delete cur;
         cur=temp;
    }
}
int TopoPoints::GetPointCount()
{
    return pointCount;
}

TopoPoint*TopoPoints::AddPoint(double x,double y)
{
    TopoPoint*pt=new TopoPoint();
    pt->X=x;
    pt->Y=y;
    if(rootPoint==nullptr)
    {
        rootPoint=pt;
        tailPoint=pt;
    }
    else
    {
        tailPoint->child=pt;
        pt->father=tailPoint;
        tailPoint=pt;
    }
    pointCount++;
    return pt;
}
AutoPtr<Envelope>TopoPoints::ComputeEnvelope()
{
    Envelope*env=new Envelope();
    TopoPoint*point=rootPoint;
    double xmin=0,ymin=0,xmax=0,ymax=0;
    while(point!=nullptr)
    {
        if(point==rootPoint)
        {
            xmin=xmax=point->X;
            ymin=ymax=point->Y;
        }
        else
        {
            if(point->X<xmin) xmin=point->X;
            if(point->X>xmax) xmax=point->X;
            if(point->Y<ymin) ymin=point->Y;
            if(point->Y>ymax) ymax=point->Y;
        }
        point=point->child;
    }
    env->PutCoord(xmin,ymax,xmax,ymin);
    return env;
}
void TopoPoints::AddPoint(TopoPoint*point)
{
    if(rootPoint==nullptr)
    {
        rootPoint=point;
        tailPoint=point;
    }
    else
    {
        tailPoint->child=point;
        point->father=tailPoint;
        tailPoint=point;
    }
    pointCount++;
}
void TopoPoints::RemovePoint(TopoPoint*point)
{
    if(point==rootPoint) rootPoint=point->child;
    if(point==tailPoint) tailPoint=tailPoint->father;
    if(point->father!=nullptr) point->father->child=point->child;
    if(point->child!=nullptr) point->child->father=point->father;
    point->father=nullptr;
    point->child=nullptr;
    delete point;
    pointCount--;
}
void TopoPoints::Clear()
{
    TopoPoint*cur=tailPoint;
    while(cur!=nullptr)
    {
            TopoPoint*temp=cur->father;
            delete cur;
            cur=temp;
    }
    rootPoint=tailPoint=nullptr;
    pointCount=0;
}

TopoLines::TopoLines()
{
    rootLine=nullptr;
    tailLine=nullptr;
    regions=nullptr;
    lineCount=0;
}
TopoLines::~TopoLines()
{
    TopoLine*cur=tailLine;
    while(cur!=nullptr)
    {
         TopoLine*temp=cur->father;
         delete cur;
         cur=temp;
    }
    if(regions!=nullptr) delete regions;
}

void TopoLines::AddLine(TopoLine*line)
{
    if(rootLine==nullptr)
    {
        rootLine=line;
        tailLine=line;
    }
    else
    {
        tailLine->child=line;
        line->father=tailLine;
        tailLine=line;
    }
    if(regions!=nullptr)
    {
        regions->RegisterLine(line);
    }
    lineCount++;
}
void TopoLines::RemoveLine(TopoLine*line)
{
    if(line==rootLine) rootLine=line->child;
    if(line==tailLine) tailLine=tailLine->father;
    if(line->father!=nullptr) line->father->child=line->child;
    if(line->child!=nullptr) line->child->father=line->father;
    line->father=nullptr;
    line->child=nullptr;
    line->fromNode->RemoveLine(line);
    line->toNode->RemoveLine(line);
    if(regions!=nullptr)
    {
        regions->UnregisterLine(line);
    }
    delete line;
    lineCount--;
}
void TopoLines::Clear()
{
    TopoLine*cur=tailLine;
    while(cur!=nullptr)
    {
        TopoLine*temp=cur->father;
        delete cur;
        cur=temp;
    }
    rootLine=tailLine=nullptr;
    if(regions!=nullptr) delete regions;
    regions=nullptr;
    lineCount=0;
}
int TopoLines::GetLineCount()
{
    return lineCount;
}
void TopoLines::BuildSpatialIndex(Envelope*fullExtent,int regionNum)
{
    if(regions!=nullptr) delete regions;
    regions=new LineRegions(fullExtent,regionNum,regionNum);
    TopoLine*line=rootLine;
    while(line!=nullptr)
    {
        regions->RegisterLine(line);
        line=line->child;
    }
}
void TopoLines::BuildContourSpatialIndex(Envelope*fullExtent,int regionNum){
    if(regions!=nullptr) delete regions;
    regions=new LineRegions(fullExtent,regionNum,regionNum);
    TopoLine*line=rootLine;
    while(line!=nullptr)
    {
        regions->RegisterContour(line);
        line=line->child;
    }
}
TopoRing::TopoRing()
{
    extent=nullptr;
    father=nullptr;
    child=nullptr;
    fromPoint=nullptr;
    area=0;
    attachRing=nullptr;
}
TopoRing::~TopoRing()
{
    for(int k=innerRings.size()-1;k>=0;k--) delete innerRings[k];
    innerRings.clear();
}
void TopoRing::ComputeEnvelope()
{
    if(extent!=nullptr) return;
    int lineSize=lines.size();
    if(lineSize==0) return;
    extent=lines[0]->ComputeEnvelope();
    for(int k=1;k<lineSize;k++)
    {
        AutoPtr<Envelope>pEnv=lines[k]->ComputeEnvelope();
        extent->UnionOther(pEnv.get());
    }
}
AutoPtr<Envelope>TopoRing::GetEnvelope()
{
    if(extent==nullptr) ComputeEnvelope();
    if(extent!=nullptr) {
        Envelope*env=new Envelope();
        env->CopyFromOther(extent.get());
        return env;
    }
    return nullptr;
}
void TopoRing::AddFirstLine(TopoPoint*fp,TopoLine*tLine)
{
    fromPoint=fp;
    lines.push_back(tLine);
}
void TopoRing::AddLine(TopoLine*tLine)
{
    lines.push_back(tLine);
}
void TopoRing::AddInnerRing(TopoRing*ring)
{
    innerRings.push_back(ring);
}
int TopoRing::GetInnerRingCount()
{
    return innerRings.size();
}
TopoRing*TopoRing::GetInnerRing(int nIndex)
{
    return innerRings[nIndex];
}
int TopoRing::GetLineCount()
{
    return lines.size();
}
TopoLine*TopoRing::GetLine(int nIndex)
{
    return lines[nIndex];
}
void TopoRing::CloseRing()
{
    int lineSize=lines.size();
    area=0;
    if(lineSize==0) return;
    TopoPoint*fp=fromPoint;
    DOUBLE X1,Y1,X2,Y2;
    X1=fp->X;
    Y1=fp->Y;
    for(int k=0;k<lineSize;k++)
    {
        TopoLine*tLine=lines[k];
        int vertexCount=tLine->vertices.size();
        if(tLine->fromNode==fp)
        {
             if(vertexCount>0)
             {
                 for(int p=0;p<vertexCount;p++)
                 {
                    Point2D<double> dpt=tLine->vertices[p];
                    X2=dpt.X;
                    Y2=dpt.Y;
                    area+=(X2-X1)*(Y2+Y1);
                    X1=X2;
                    Y1=Y2;
                 }
             }
             X2=tLine->toNode->X;
             Y2=tLine->toNode->Y;
             area+=(X2-X1)*(Y2+Y1);
             X1=X2;
             Y1=Y2;
             fp=tLine->toNode;
        }
        else
        {
             if(vertexCount>0)
             {
                for(int p=vertexCount-1;p>=0;p--)
                {
                    Point2D<double> dpt=tLine->vertices[p];
                    X2=dpt.X;
                    Y2=dpt.Y;
                    area+=(X2-X1)*(Y2+Y1);
                    X1=X2;
                    Y1=Y2;
                }
             }
             X2=tLine->fromNode->X;
             Y2=tLine->fromNode->Y;
             area+=(X2-X1)*(Y2+Y1);
             X1=X2;
             Y1=Y2;
             fp=tLine->fromNode;
        }
    }
    area=area/2.0;
}
bool TopoRing::GetIsClockWise()
{
    return (area>=0);
}
double TopoRing::GetArea()
{
    return area;
}
Ring* TopoRing::CreateRing()
{
    int lineSize=lines.size();
    if(lineSize==0) return nullptr;
    TopoPoint*fp=fromPoint;
    Points*points=new Points();
    Ring*pRing=(Ring*)points;
    int pointCount=0;
    for(int k=0;k<lineSize;k++)
    {
        TopoLine*tLine=lines[k];
        pointCount+=tLine->vertices.size()+1;
    }
    pointCount++;
    Point2D<double>*dpts=new Point2D<double>[pointCount];
    int currentPin=0;
    if(lineSize==1)
    {
        TopoLine*tLine=lines[0];
        int vertexCount=tLine->vertices.size();
        dpts[currentPin++]=Point2D<double>(fp->X,fp->Y);
        if(area>=0)
        {
            if(vertexCount>0)
            {
                Point2D<double>*pts=tLine->vertices.data();
                memcpy(((BYTE*)dpts)+currentPin*sizeof(double)*2,pts,sizeof(double)*2*vertexCount);
                currentPin+=vertexCount;
            }
            dpts[currentPin++]=Point2D<double>(tLine->toNode->X,tLine->toNode->Y);
        }
        else
        {
            for(int p=vertexCount-1;p>=0;p--)
            {
                Point2D<double> dpt=tLine->vertices[p];
                dpts[currentPin++]=dpt;
            }
            dpts[currentPin++]=Point2D<double>(tLine->toNode->X,tLine->toNode->Y);
        }
    }
    else if(area>=0)
    {
        for(int k=0;k<lineSize;k++)
        {
            TopoLine*tLine=lines[k];
            int vertexCount=tLine->vertices.size();
            if(k==0) dpts[currentPin++]=Point2D<double>(fp->X,fp->Y);
            if(tLine->fromNode==fp)
            {
                if(vertexCount>0)
                {
                    Point2D<double>*pts=tLine->vertices.data();
                    memcpy(((BYTE*)dpts)+currentPin*sizeof(double)*2,pts,sizeof(double)*2*vertexCount);
                    currentPin+=vertexCount;
                }
                dpts[currentPin++]=Point2D<double>(tLine->toNode->X,tLine->toNode->Y);
                fp=tLine->toNode;
            }
            else
            {
                for(int p=vertexCount-1;p>=0;p--)
                {
                    Point2D<double> dpt=tLine->vertices[p];
                    dpts[currentPin++]=dpt;
                }
                dpts[currentPin++]=Point2D<double>(tLine->fromNode->X,tLine->fromNode->Y);
                fp=tLine->fromNode;
            }
        }
    }
    else
    {
        for(int k=lineSize-1;k>=0;k--)
        {
            TopoLine*tLine=lines[k];
            int vertexCount=tLine->vertices.size();
            if(k==0) dpts[currentPin++]=Point2D<double>(fp->X,fp->Y);
            if(tLine->fromNode==fp)
            {
                if(vertexCount>0)
                {
                    Point2D<double>*pts=tLine->vertices.data();
                    memcpy(((BYTE*)dpts)+currentPin*sizeof(double)*2,pts,sizeof(double)*2*vertexCount);
                    currentPin+=vertexCount;
                }
                dpts[currentPin++]=Point2D<double>(tLine->toNode->X,tLine->toNode->Y);
                fp=tLine->toNode;
            }
            else
            {
                for(int p=vertexCount-1;p>=0;p--)
                {
                    Point2D<double> dpt=tLine->vertices[p];
                    dpts[currentPin++]=dpt;
                }
                dpts[currentPin++]=Point2D<double>(tLine->fromNode->X,tLine->fromNode->Y);
                fp=tLine->fromNode;
            }
        }
    }
    points->SetSize(pointCount);
    memcpy(points->GetXYs(),dpts,sizeof(double)*pointCount*2);
    delete []dpts;
    return pRing;
}

AutoPtr<Polygon>TopoRing::CreatePolygon(bool isCurve)
{
    Polygon*polygon;
    if(isCurve)
        polygon=new CurvePolygon();
    else
        polygon=new Polygon();
    Ring*pRing=CreateRing();
    polygon->Add(AutoPtr<Ring>(pRing));
    for(int k=0;k<innerRings.size();k++)
    {
        Ring*pRing=innerRings[k]->CreateRing();
        Points*points=(Points*)pRing;
        points->Reverse();
        polygon->Add(AutoPtr<Ring>(pRing));
    }
    return polygon;
}
void TopoRing::RegisterClockwiseLines()
{
    if(area>=0)
    {
        TopoPoint*fp=fromPoint;
        int lnSize=lines.size();
        if(lnSize==1)
            lines[0]->rightRing=this;
        else
        {
            for(int k=0;k<lnSize;k++)
            {
                TopoLine*tLine=lines[k];
                if(tLine->fromNode==fp)
                {
                    tLine->rightRing=this;
                    fp=tLine->toNode;
                }
                else
                {
                    tLine->leftRing=this;
                    fp=tLine->fromNode;
                }
            }
        }
    }
    else
    {
        TopoPoint*fp=fromPoint;
        int lnSize=lines.size();
        if(lnSize==1)
            lines[0]->leftRing=this;
        else
        {
            for(int k=0;k<lnSize;k++)
            {
                TopoLine*tLine=lines[k];
                if(tLine->fromNode==fp)
                {
                    tLine->leftRing=this;
                    fp=tLine->toNode;
                }
                else
                {
                    tLine->rightRing=this;
                    fp=tLine->fromNode;
                }
            }
        }
    }
}
void TopoRing::UnregisterClockwiseLines()
{
    if(area>=0)
    {
        TopoPoint*fp=fromPoint;
        int lnSize=lines.size();
        if(lnSize==1)
            lines[0]->rightRing=nullptr;
        else
        {
            for(int k=0;k<lnSize;k++)
            {
                TopoLine*tLine=lines[k];
                if(tLine->fromNode==fp)
                {
                    tLine->rightRing=nullptr;
                    fp=tLine->toNode;
                }
                else
                {
                    tLine->leftRing=nullptr;
                    fp=tLine->fromNode;
                }
            }
        }
    }
    else
    {
        TopoPoint*fp=fromPoint;
        int lnSize=lines.size();
        if(lnSize==1)
            lines[0]->leftRing=nullptr;
        else
        {
            for(int k=0;k<lnSize;k++)
            {
                TopoLine*tLine=lines[k];
                if(tLine->fromNode==fp)
                {
                    tLine->leftRing=nullptr;
                    fp=tLine->toNode;
                }
                else
                {
                    tLine->rightRing=nullptr;
                    fp=tLine->fromNode;
                }
            }
        }
    }
}
void TopoRing::RegisterCounterClockwiseLines(TopoRing*outerRing)
{
    if(area>=0)
    {
        TopoPoint*fp=fromPoint;
        int lnSize=lines.size();
        if(lnSize==1)
            lines[0]->leftRing=outerRing;
        else
        {
            for(int k=0;k<lnSize;k++)
            {
                TopoLine*tLine=lines[k];
                if(tLine->fromNode==fp)
                {
                    tLine->leftRing=outerRing;
                    fp=tLine->toNode;
                }
                else
                {
                    tLine->rightRing=outerRing;
                    fp=tLine->fromNode;
                }
            }
        }
    }
    else
    {
        TopoPoint*fp=fromPoint;
        int lnSize=lines.size();
        if(lnSize==1)
            lines[0]->rightRing=outerRing;
        else
        {
            for(int k=0;k<lnSize;k++)
            {
                TopoLine*tLine=lines[k];
                if(tLine->fromNode==fp)
                {
                    tLine->rightRing=outerRing;
                    fp=tLine->toNode;
                }
                else
                {
                    tLine->leftRing=outerRing;
                    fp=tLine->fromNode;
                }
            }
        }
    }
}
void TopoRing::UnregisterCounterClockwiseLines()
{
    if(area>=0)
    {
        TopoPoint*fp=fromPoint;
        int lnSize=lines.size();
        if(lnSize==1)
            lines[0]->leftRing=nullptr;
        else
        {
            for(int k=0;k<lnSize;k++)
            {
                TopoLine*tLine=lines[k];
                if(tLine->fromNode==fp)
                {
                    tLine->leftRing=nullptr;
                    fp=tLine->toNode;
                }
                else
                {
                    tLine->rightRing=nullptr;
                    fp=tLine->fromNode;
                }
            }
        }
    }
    else
    {
        TopoPoint*fp=fromPoint;
        int lnSize=lines.size();
        if(lnSize==1)
            lines[0]->rightRing=nullptr;
        else
        {
            for(int k=0;k<lnSize;k++)
            {
                TopoLine*tLine=lines[k];
                if(tLine->fromNode==fp)
                {
                    tLine->rightRing=nullptr;
                    fp=tLine->toNode;
                }
                else
                {
                    tLine->leftRing=nullptr;
                    fp=tLine->fromNode;
                }
            }
        }
    }
}
void TopoRing::CopyLines(TopoRing*other)
{
    int lnSize=other->lines.size();
    lines.resize(lnSize);
    for(int k=0;k<lnSize;k++) lines[k]=other->lines[k];
    area=other->area;
    fromPoint=other->fromPoint;
}
bool TopoRing::IsPointIn(double x,double y)
{
    int lineSize=lines.size();
    if(lineSize==0) return false;
    TopoPoint*fp=fromPoint;
    int pointCount=0;
    for(int k=0;k<lineSize;k++)
    {
        TopoLine*tLine=lines[k];
        pointCount+=tLine->vertices.size()+1;
    }
    pointCount++;
    if(pointCount<3) return false;
    double af,at,dt,a=0;
    double pi=PI;
    double dpi=DPI;
    DOUBLE X1,Y1,X2,Y2;
    X1=fp->X;Y1=fp->Y;
    if((x==X1)&&(y==Y1)) return true;
    if(x==X1)
    {
        if(Y1>y)
             af=pi/2;
        else if(y==Y1)
             af=0;
        else
             af=pi*3/2;
    }
    else
    {
       af=atan((Y1-y)/(X1-x));
       if(X1<x) af=af+pi;
    }
    for(int k=0;k<lineSize;k++)
    {
        TopoLine*tLine=lines[k];
        int vertexCount=tLine->vertices.size();
        if(k==0)
        {
            X2=fp->X;Y2=fp->Y;
            if((x==X2)&&(y==Y2)) return true;
            if(x==X2)
            {
                if(Y2>y)
                     at=pi/2;
                else if(y==Y2)
                     at=0;
                else
                     at=pi*3/2;
            }
            else
            {
               at=atan((Y2-y)/(X2-x));
               if(X2<x) at=at+pi;
            }
            dt=at-af;
            if(dt<0) dt=dpi+dt;
            if(dt>pi) dt=-(dpi-dt);
            a=a+dt;
            af=at;
            X1=X2;
            Y1=Y2;
        }
        if(tLine->fromNode==fp)
        {
             if(vertexCount>0)
             {
                 for(int p=0;p<vertexCount;p++)
                 {
                     Point2D<double> dpt=tLine->vertices[p];
                     X2=dpt.X;Y2=dpt.Y;
                     if((x==X2)&&(y==Y2)) return true;
                     if(x==X2)
                     {
                        if(Y2>y)
                             at=pi/2;
                        else if(y==Y2)
                             at=0;
                        else
                             at=pi*3/2;
                    }
                    else
                    {
                       at=atan((Y2-y)/(X2-x));
                       if(X2<x) at=at+pi;
                    }
                    dt=at-af;
                    if(dt<0) dt=dpi+dt;
                    if(dt>pi) dt=-(dpi-dt);
                    a=a+dt;
                    af=at;
                    X1=X2;
                    Y1=Y2;
                 }
             }
            X2=tLine->toNode->X;Y2=tLine->toNode->Y;
            if((x==X2)&&(y==Y2)) return true;
            if(x==X2)
            {
                if(Y2>y)
                     at=pi/2;
                else if(y==Y2)
                     at=0;
                else
                     at=pi*3/2;
            }
            else
            {
               at=atan((Y2-y)/(X2-x));
               if(X2<x) at=at+pi;
            }
            dt=at-af;
            if(dt<0) dt=dpi+dt;
            if(dt>pi) dt=-(dpi-dt);
            a=a+dt;
            af=at;
            X1=X2;
            Y1=Y2;
            fp=tLine->toNode;
        }
        else
        {
             for(int p=vertexCount-1;p>=0;p--)
             {
                 Point2D<double> dpt=tLine->vertices[p];
                 X2=dpt.X;Y2=dpt.Y;
                 if((x==X2)&&(y==Y2)) return true;
                 if(x==X2)
                 {
                    if(Y2>y)
                         at=pi/2;
                    else if(y==Y2)
                         at=0;
                    else
                         at=pi*3/2;
                 }
                 else
                 {
                   at=atan((Y2-y)/(X2-x));
                   if(X2<x) at=at+pi;
                 }
                 dt=at-af;
                 if(dt<0) dt=dpi+dt;
                 if(dt>pi) dt=-(dpi-dt);
                 a=a+dt;
                 af=at;
                 X1=X2;
                 Y1=Y2;
             }
             X2=tLine->fromNode->X;Y2=tLine->fromNode->Y;
             if((x==X2)&&(y==Y2)) return true;
             if(x==X2)
             {
                if(Y2>y)
                        at=pi/2;
                else if(y==Y2)
                        at=0;
                else
                        at=pi*3/2;
             }
            else
            {
                at=atan((Y2-y)/(X2-x));
                if(X2<x) at=at+pi;
            }
            dt=at-af;
            if(dt<0) dt=dpi+dt;
            if(dt>pi) dt=-(dpi-dt);
            a=a+dt;
            af=at;
            X1=X2;
            Y1=Y2;
            fp=tLine->fromNode;
        }
    }
    if(fabs(a)>=0.000001) return (fabs(fmod(a,DPI))<0.000001);
    return false;
}
bool TopoRing::IsPointIn(TopoPoint*point)
{
    return IsPointIn(point->X,point->Y);
}
bool TopoRing::IsRingIn(TopoRing*other)
{
    int lineSize=lines.size();
    if(lineSize==0) return false;
    int otherSize=other->lines.size();
    if(otherSize==0) return false;
    if(lines[0]==other->lines[0]) return false;
    if(fabs(other->area)>=fabs(area)) return false;
    TopoLine*tLine=other->lines[0];
    if(tLine->vertices.size()>0)
    {
        Point2D<double> v1=tLine->vertices[0];
        return IsPointIn(v1.X,v1.Y);
    }
    else
    {
        double x=(tLine->fromNode->X+tLine->toNode->X)/2.0;
        double y=(tLine->fromNode->Y+tLine->toNode->Y)/2.0;
        return IsPointIn(x,y);
    }
    return true;
}
TopoRing*TopoRing::FindOuterRing()
{
    int lnSize=lines.size();
    if(lnSize!=1) return nullptr;
    TopoLine*tLine=lines[0];
    if(area>=0) return tLine->leftRing;
    return tLine->rightRing;
}
void TopoRing::FindNeighborRings(vector<TopoRing*>&neighbors)
{
    TopoRing*mRing=this;
    int ringSize=innerRings.size();
    for(int k=0;k<=ringSize;k++)
    {
        if(k==ringSize)
            mRing=this;
        else
            mRing=innerRings[k];
        int sublnSize=mRing->lines.size();
        for(int p=0;p<sublnSize;p++)
        {
            TopoLine*tLine=mRing->lines[p];
            TopoRing*tRing=nullptr;
            if(tLine->leftRing==this)
                tRing=tLine->rightRing;
            else if(tLine->rightRing==this)
                tRing=tLine->leftRing;
            if(tRing==nullptr) continue;
            bool HasFind=false;
            for(int m=neighbors.size()-1;m>=0;m--)
            {
                if(neighbors[m]==tRing)
                {
                    HasFind=true;
                    break;
                }
            }
            if(HasFind) continue;
            neighbors.push_back(tRing);
        }
    }
}
bool TopoRing::FindAttribute(int at)
{
    TopoRing*mRing=this;
    int ringSize=innerRings.size();
    for(int k=0;k<=ringSize;k++)
    {
        if(k==ringSize)
            mRing=this;
        else
            mRing=innerRings[k];
        int sublnSize=mRing->lines.size();
        bool clockWise=(k==ringSize);
        if(!mRing->GetIsClockWise()) clockWise=(!clockWise);
        TopoPoint*fromPoint=mRing->fromPoint;
        bool IsFrom;
        for(int p=0;p<sublnSize;p++)
        {
            TopoLine*tLine=mRing->lines[p];
            if(sublnSize==1)
                IsFrom=true;
            else
                IsFrom=(tLine->fromNode==fromPoint);
            if(IsFrom^clockWise)
            {
                if(tLine->FindAttributes(-at)) return true;
            }
            else
            {
                if(tLine->FindAttributes(at)) return true;
            }
            fromPoint=((IsFrom)?tLine->toNode:tLine->fromNode);
        }
    }
    return false;
}
bool TopoRing::FindNeighborAttribute(int at)
{
    TopoRing*mRing=this;
    int ringSize=innerRings.size();
    for(int k=0;k<=ringSize;k++)
    {
        if(k==ringSize)
            mRing=this;
        else
            mRing=innerRings[k];
        int sublnSize=mRing->lines.size();
        for(int p=0;p<sublnSize;p++)
        {
            TopoLine*tLine=mRing->lines[p];
            TopoRing*nr=nullptr;
            if(tLine->leftRing==this)
                nr=tLine->rightRing;
            else if(tLine->rightRing==this)
                nr=tLine->leftRing;
            if(nr!=nullptr)
            {
                if(nr->FindAttribute(at)) return true;
            }
        }
    }
    return false;
}
int TopoRing::GetMinPositiveAttribute()
{
    TopoRing*mRing=this;
    int ringSize=innerRings.size();
    int minV=0;
    for(int k=0;k<=ringSize;k++)
    {
        if(k==ringSize)
            mRing=this;
        else
            mRing=innerRings[k];
        int sublnSize=mRing->lines.size();
        bool clockWise=(k==ringSize);
        if(!mRing->GetIsClockWise()) clockWise=(!clockWise);
        TopoPoint*fromPoint=mRing->fromPoint;
        bool IsFrom;
        for(int p=0;p<sublnSize;p++)
        {
            TopoLine*tLine=mRing->lines[p];
            if(sublnSize==1)
                IsFrom=true;
            else
                IsFrom=(tLine->fromNode==fromPoint);
            if(IsFrom^clockWise)
            {
                int tv=-tLine->GetMaxNegtiveAttribute();
                if(tv==1) return 1;
                if(tv>0)
                {
                    if((minV==0)||(minV>tv)) minV=tv;
                }
            }
            else
            {
                int tv=tLine->GetMinPositiveAttribute();
                if(tv==1) return 1;
                if(tv>0)
                {
                    if((minV==0)||(minV>tv)) minV=tv;
                }
            }
            fromPoint=((IsFrom)?tLine->toNode:tLine->fromNode);
        }
    }
    return minV;
}
int TopoRing::GetMaxNegtiveAttribute()
{
    TopoRing*mRing=this;
    int ringSize=innerRings.size();
    int maxV=0;
    for(int k=0;k<=ringSize;k++)
    {
        if(k==ringSize)
            mRing=this;
        else
            mRing=innerRings[k];
        int sublnSize=mRing->lines.size();
        bool clockWise=(k==ringSize);
        if(!mRing->GetIsClockWise()) clockWise=(!clockWise);
        TopoPoint*fromPoint=mRing->fromPoint;
        bool IsFrom;
        for(int p=0;p<sublnSize;p++)
        {
            TopoLine*tLine=mRing->lines[p];
            if(sublnSize==1)
                IsFrom=true;
            else
                IsFrom=(tLine->fromNode==fromPoint);
            if(IsFrom^clockWise)
            {
                int tv=-tLine->GetMinPositiveAttribute();
                if(tv==-1) return -1;
                if(tv>0)
                {
                    if((maxV==0)||(maxV<tv)) maxV=tv;
                }
            }
            else
            {
                int tv=tLine->GetMaxNegtiveAttribute();
                if(tv==-1) return -1;
                if(tv>0)
                {
                    if((maxV==0)||(maxV<tv)) maxV=tv;
                }
            }
            fromPoint=((IsFrom)?tLine->toNode:tLine->fromNode);
        }
    }
    return maxV;
}

RingRegion::RingRegion()
{

}
RingRegion::~RingRegion()
{

}
void RingRegion::AddRing(TopoRing*ring)
{
    rings.push_back(ring);
}

TopoRing*RingRegion::GetItem(int nIndex)
{
    return rings[nIndex];
}
int RingRegion::GetCount()
{
    return rings.size();
}
void RingRegion::RemoveAll()
{
    rings.clear();
}

RingRegions::RingRegions(Envelope*fullExtent)
{
    left=0;
    top=0;
    right=0;
    bottom=0;
    if(fullExtent!=nullptr) fullExtent->GetCoord(&left,&top,&right,&bottom);
    xNum=50;
    yNum=50;
    regions.resize(xNum*yNum);
    for(int k=xNum*yNum-1;k>=0;k--)
    {
        RingRegion*orr=new RingRegion();
        regions[k]=orr;
    }
    InitialCells();
}
RingRegions::RingRegions(Envelope*fullExtent,int xSize,int ySize)
{
    left=0;
    top=0;
    right=0;
    bottom=0;
    if(fullExtent!=nullptr)  fullExtent->GetCoord(&left,&top,&right,&bottom);
    xNum=xSize;
    yNum=ySize;
    regions.resize(xNum*yNum);
    for(int k=xNum*yNum-1;k>=0;k--)
    {
        RingRegion*orr=new RingRegion();
        regions[k]=orr;
    }
    InitialCells();
}
RingRegions::~RingRegions()
{
    for(int k=regions.size()-1;k>=0;k--)
    {
        delete regions[k];
    }
    regions.clear();
}
int RingRegions::GetRegionXNum()
{
    return xNum;
}
int RingRegions::GetRegionYNum()
{
    return yNum;
}
int RingRegions::GetRegionX(double x)
{
    int xid=(x-left)/difx;
    if(xid<0) return 0;
    if(xid>=xNum) xid=xNum-1;
    return xid;
}
int RingRegions::GetRegionY(double y)
{
    int yid=(top-y)/dify;
    if(yid<0) return 0;
    if(yid>=yNum) yid=yNum-1;
    return yid;
}
RingRegion*RingRegions::GetRegion(int nIndex)
{
    return regions[nIndex];
}

void RingRegions::GetRegionEnvelope(Envelope*env,int&il,int&it,int&ir,int&ib)
{
    double l,t,r,b;
    env->GetCoord(&l,&t,&r,&b);
    il=GetRegionX(l);
    ir=GetRegionX(r);
    it=GetRegionY(t);
    ib=GetRegionY(b);
    if(ir<il)
    {
        int temp=ir;
        ir=il;
        il=temp;
    }
    if(ib<it)
    {
        int temp=ib;
        ib=it;
        it=temp;
    }
}
void RingRegions::InitialCells()
{
    difx=(right-left)/xNum;
    dify=(top-bottom)/yNum;
    for(int k=regions.size()-1;k>=0;k--)
    {
        regions[k]->RemoveAll();
    }
}
void RingRegions::RegisterRing(TopoRing*ring)
{
    AutoPtr<Envelope>extent(ring->GetEnvelope());
    int il,it,ir,ib;
    GetRegionEnvelope(extent.get(),il,it,ir,ib);
    for(int i=it;i<=ib;i++)
    {
        int regionNum=i*xNum+il;
        for(int j=il;j<=ir;j++)
        {
            regions[regionNum]->AddRing(ring);
            regionNum++;
        }
    }
}
void RingRegions::Clear()
{
    for(int k=regions.size()-1;k>=0;k--)
    {
        regions[k]->RemoveAll();
    }
}

TopoRings::TopoRings()
{
    rootRing=nullptr;
    tailRing=nullptr;
    rRegions=nullptr;
    ringCount=0;
}
TopoRings::~TopoRings()
{
    TopoRing*cur=tailRing;
    while(cur!=nullptr)
    {
         TopoRing*temp=cur->father;
         delete cur;
         cur=temp;
    }
    if(rRegions!=nullptr) delete rRegions;
}

void TopoRings::AddRing(TopoRing*ring)
{
    if(rootRing==nullptr)
    {
        rootRing=ring;
        tailRing=ring;
    }
    else
    {
        tailRing->child=ring;
        ring->father=tailRing;
        tailRing=ring;
    }
    ringCount++;
}
void TopoRings::RemoveRing(TopoRing*ring)
{
    if(ring==rootRing) rootRing=ring->child;
    if(ring==tailRing) tailRing=tailRing->father;
    if(ring->father!=nullptr) ring->father->child=ring->child;
    if(ring->child!=nullptr) ring->child->father=ring->father;
    ring->father=nullptr;
    ring->child=nullptr;
    delete ring;
    ringCount--;
}
void TopoRings::DetachRing(TopoRing*ring)
{
    if(ring==rootRing) rootRing=ring->child;
    if(ring==tailRing) tailRing=tailRing->father;
    if(ring->father!=nullptr) ring->father->child=ring->child;
    if(ring->child!=nullptr) ring->child->father=ring->father;
    ring->father=nullptr;
    ring->child=nullptr;
    ringCount--;
}
void TopoRings::Clear()
{
    TopoRing*cur=tailRing;
    while(cur!=nullptr)
    {
        TopoRing*temp=cur->father;
        delete cur;
        cur=temp;
    }
    rootRing=tailRing=nullptr;
    if(rRegions!=nullptr) rRegions->Clear();
    ringCount=0;
}
int TopoRings::GetRingCount()
{
    return ringCount;
}
TopoRing*TopoRings::FindOuterRing(TopoRing*innerRing)
{
    AutoPtr<Envelope>pEnv(innerRing->GetEnvelope());
    int il,it,ir,ib;
    rRegions->GetRegionEnvelope(pEnv.get(),il,it,ir,ib);
    int xNum=rRegions->GetRegionXNum();
    double minarea=0;
    TopoRing*selectedRing=nullptr;
    for(int i=it;i<=ib;i++)
    {
        int regionNum=i*xNum+il;
        for(int j=il;j<=ir;j++)
        {
            RingRegion*rr=rRegions->GetRegion(regionNum);
            int ringCount=rr->GetCount();
            for(int p=0;p<ringCount;p++)
            {
                TopoRing*outRing=rr->GetItem(p);
                if(outRing->attachRing==innerRing) continue;
                outRing->attachRing=innerRing;
                bool IsRingIn=outRing->IsRingIn(innerRing);
                if(!IsRingIn) continue;
                double area=fabs(outRing->GetArea());
                if(selectedRing==nullptr)
                {
                    minarea=area;
                    selectedRing=outRing;
                }
                else if(area<minarea)
                {
                    minarea=area;
                    selectedRing=outRing;
                }
            }
            regionNum++;
        }
    }
    return selectedRing;
}
void TopoRings::BuildSpatialIndex(Envelope*fullExtent,int regionNum)
{
    if(rRegions!=nullptr) delete rRegions;
    rRegions=new RingRegions(fullExtent,regionNum,regionNum);
    TopoRing*ring=rootRing;
    while(ring!=nullptr)
    {
        rRegions->RegisterRing(ring);
        ring=ring->child;
    }
}

TopoBuilder::TopoBuilder()
    :fuzzy(0.000001)
{
    tPoints=new TopoPoints();
    tLines=new TopoLines();
    oRings=new TopoRings();
    iRings=new TopoRings();
    fullExtent=nullptr;
};
TopoBuilder::TopoBuilder(double fuzy)
{
    fuzzy=fuzy;
    tPoints=new TopoPoints();
    tLines=new TopoLines();
    oRings=new TopoRings();
    iRings=new TopoRings();
    fullExtent=nullptr;
};
TopoBuilder::~TopoBuilder()
{
    delete tPoints;
    delete tLines;
    delete oRings;
    delete iRings;
}
TopoLines*TopoBuilder::GetLines()
{
    return tLines;
}
TopoPoints*TopoBuilder::GetPoints()
{
    return tPoints;
}
TopoRings*TopoBuilder::GetRings()
{
    return oRings;
}

void TopoBuilder::ConnectPoint(TopoPoint*topoPt1,TopoPoint*topoPt2,int attribute){
    for(int k=0;k<topoPt1->lines.size();k++){
        TopoLine*otherln=topoPt1->lines[k];
        if((otherln->fromNode==topoPt1)&&(otherln->toNode==topoPt2)){
            otherln->AddAttributes(attribute);
            return;
        }
        if((otherln->toNode==topoPt1)&&(otherln->fromNode==topoPt2)){
            otherln->AddAttributes(-attribute);
            return;
        }
    }
    TopoLine*tln=new TopoLine(topoPt1,topoPt2,attribute);
    tLines->AddLine(tln);
}

void TopoBuilder::ConnectPointEx(TopoPoint*topoPt1,TopoPoint*topoPt2,vector<Point2D<double>>&vertices,int attribute){
    for(int k=0;k<topoPt1->lines.size();k++){
        TopoLine*otherln=topoPt1->lines[k];
        if((otherln->fromNode==topoPt1)&&(otherln->toNode==topoPt2)){
            bool bSame=false;
            if(vertices.size()==otherln->vertices.size()){
                bSame=true;
                int vsize=vertices.size();
                for(int l=0;l<vsize;l++){
                    if((vertices[l].X!=otherln->vertices[l].X)||(vertices[l].Y!=otherln->vertices[l].Y)){
                        bSame=false;
                        break;
                    }
                }
            }
            if(bSame){
                otherln->AddAttributes(attribute);
                return;
            }
        }
        if((otherln->toNode==topoPt1)&&(otherln->fromNode==topoPt2)){
            bool bSame=false;
            if(vertices.size()==otherln->vertices.size()){
                bSame=true;
                int vsize=vertices.size();
                for(int l=0;l<vsize;l++){
                    if((vertices[vsize-l-1].X!=otherln->vertices[l].X)||(vertices[vsize-l-1].Y!=otherln->vertices[l].Y)){
                        bSame=false;
                        break;
                    }
                }
            }
            if(bSame){
                otherln->AddAttributes(-attribute);
                return;
            }
        }
    }
    TopoLine*tln=new TopoLine(topoPt1,topoPt2,attribute);
    tln->vertices.resize(vertices.size());
    for(int k=0;k<vertices.size();k++) tln->vertices[k]=vertices[k];
    tLines->AddLine(tln);
}

void TopoBuilder::SetFullExtent(Envelope*env){
    if(fullExtent==nullptr) fullExtent=new Envelope();
    fullExtent->CopyFromOther(env);
}

TopoPoint* TopoBuilder::AppendPoint(Point*pt){
    AutoPtr<Envelope>pEnv=pt->GetExtent();
    if(fullExtent==nullptr)
        fullExtent=pEnv;
    else
        fullExtent->UnionOther(pEnv.get());
    return this->tPoints->AddPoint(pt->X(),pt->Y());
}

TopoLine*TopoBuilder::AppendSegment(Segment*seg,int attribute){
    AutoPtr<Envelope>pEnv=seg->GetExtent();
    if(fullExtent==nullptr)
        fullExtent=pEnv;
    else
        fullExtent->UnionOther(pEnv.get());
    TopoPoint*ftp=new TopoPoint(seg->Lx1(),seg->Ly1());
    tPoints->AddPoint(ftp);
    TopoPoint*ttp=new TopoPoint(seg->Lx2(),seg->Ly2());
    tPoints->AddPoint(ttp);
    TopoLine*tln=new TopoLine(ftp,ttp,attribute);
    tLines->AddLine(tln);
    return tln;
}

void TopoBuilder::AppendPoints(Points*ppts){
    int ptsSize=ppts->GetSize();
    double x,y;
    for(int k=0;k<ptsSize;k++){
        ppts->GetItem(k,x,y);
        this->tPoints->AddPoint(x,y);
    }
}

void TopoBuilder::AppendPath(Points*pts)
{
    LONG ptCount=pts->GetSize();
    if(ptCount<2) return;
    AutoPtr<Envelope>pEnv=pts->GetExtent();
    if(fullExtent==nullptr){
        fullExtent=new Envelope();
        fullExtent->CopyFromOther(pEnv.get());
    }
    else
    {
        fullExtent->UnionOther(pEnv.get());
    }
    DOUBLE X1,Y1,X2,Y2;
    pts->GetItem(0,X1,Y1);
    TopoPoint*ftp=new TopoPoint(X1,Y1);
    TopoPoint*firsttp=ftp;
    tPoints->AddPoint(ftp);
    for(int k=1;k<ptCount;k++)
    {
        pts->GetItem(k,X2,Y2);
        if((X2==X1)&&(Y2==Y1)) continue;
        TopoPoint*ttp=new TopoPoint(X2,Y2);
        tPoints->AddPoint(ttp);
        TopoLine*tln=new TopoLine(ftp,ttp);
        tLines->AddLine(tln);
        X1=X2;
        Y1=Y2;
        ftp=ttp;
    }
    if(ftp==firsttp)
    {
        tPoints->RemovePoint(ftp);
    }
}


void TopoBuilder::AppendRing(Points*pts)
{
    LONG ptCount=pts->GetSize();
    if(ptCount<2) return;
    AutoPtr<Envelope>pEnv(pts->GetExtent());
    if(fullExtent==nullptr){
        fullExtent=new Envelope();
        fullExtent->CopyFromOther(pEnv.get());
    }
    else
    {
        fullExtent->UnionOther(pEnv.get());
    }
    DOUBLE X1,Y1,X2,Y2;
    pts->GetItem(0,X1,Y1);
    DOUBLE X0,Y0;
    X0=X1;
    Y0=Y1;
    TopoPoint*ftp=new TopoPoint(X1,Y1);
    TopoPoint*firsttp=ftp;
    tPoints->AddPoint(ftp);
    for(int k=1;k<ptCount;k++)
    {
        pts->GetItem(k,X2,Y2);
        if(k==ptCount-1)
        {
            if((X2==X0)&&(Y2==Y0)) break;
        }
        if((X2==X1)&&(Y2==Y1)) continue;
        TopoPoint*ttp=new TopoPoint(X2,Y2);
        tPoints->AddPoint(ttp);
        TopoLine*tln=new TopoLine(ftp,ttp);
        tLines->AddLine(tln);
        X1=X2;
        Y1=Y2;
        ftp=ttp;
    }
    if(ftp==firsttp)
    {
        tPoints->RemovePoint(ftp);
        return;
    }
    TopoLine*tln=new TopoLine(ftp,firsttp);
    tLines->AddLine(tln);
}

void TopoBuilder::AppendPath(Points*pts,int attribute)
{
    LONG ptCount=pts->GetSize();
    if(ptCount<2) return;
    AutoPtr<Envelope>pEnv(pts->GetExtent());
    if(fullExtent==nullptr){
        fullExtent=new Envelope();
        fullExtent->CopyFromOther(pEnv.get());
    }
    else
    {
        fullExtent->UnionOther(pEnv.get());
    }
    DOUBLE X1,Y1,X2,Y2;
    pts->GetItem(0,X1,Y1);
    TopoPoint*ftp=new TopoPoint(X1,Y1);
    TopoPoint*firsttp=ftp;
    tPoints->AddPoint(ftp);
    for(int k=1;k<ptCount;k++)
    {
        pts->GetItem(k,X2,Y2);
        if((X2==X1)&&(Y2==Y1)) continue;
        TopoPoint*ttp=new TopoPoint(X2,Y2);
        tPoints->AddPoint(ttp);
        TopoLine*tln=new TopoLine(ftp,ttp,attribute);
        tLines->AddLine(tln);
        X1=X2;
        Y1=Y2;
        ftp=ttp;
    }
    if(ftp==firsttp)
    {
        tPoints->RemovePoint(ftp);
    }
}

void TopoBuilder::AppendRing(Points*pts,int attribute)
{
    LONG ptCount=pts->GetSize();
    if(ptCount<2) return;
    AutoPtr<Envelope>pEnv(pts->GetExtent());
    if(fullExtent==nullptr){
        fullExtent=new Envelope();
        fullExtent->CopyFromOther(pEnv.get());
    }
    else
    {
        fullExtent->UnionOther(pEnv.get());
    }
    DOUBLE X1,Y1,X2,Y2;
    pts->GetItem(0,X1,Y1);
    DOUBLE X0,Y0;
    X0=X1;
    Y0=Y1;
    TopoPoint*ftp=new TopoPoint(X1,Y1);
    TopoPoint*firsttp=ftp;
    tPoints->AddPoint(ftp);
    for(int k=1;k<ptCount;k++)
    {
        pts->GetItem(k,X2,Y2);
        if(k==ptCount-1)
        {
            if((X2==X0)&&(Y2==Y0)) break;
        }
        if((X2==X1)&&(Y2==Y1)) continue;
        TopoPoint*ttp=new TopoPoint(X2,Y2);
        tPoints->AddPoint(ttp);
        TopoLine*tln=new TopoLine(ftp,ttp,attribute);
        tLines->AddLine(tln);
        X1=X2;
        Y1=Y2;
        ftp=ttp;
    }
    if(ftp==firsttp)
    {
        tPoints->RemovePoint(ftp);
        return;
    }
    TopoLine*tln=new TopoLine(ftp,firsttp,attribute);
    tLines->AddLine(tln);
}

void TopoBuilder::AppendPolyline(Polyline*polyline,int attribute){
    AutoPtr<CurvePolyline>curpoly=AutoPtr<Polyline>(polyline,true);
    if(curpoly!=nullptr){
        AutoPtr<Polyline>newpoly=curpoly->ConvertToDefaultPolyline(true);
        for(int k=0;k<newpoly->GetSize();k++){
            AutoPtr<Points>ppts=newpoly->GetItem(k);
            AppendPath(ppts.get(),attribute);
        }
    }else{
        for(int k=0;k<polyline->GetSize();k++){
            AutoPtr<Points>ppts=polyline->GetItem(k);
            AppendPath(ppts.get(),attribute);
        }
    }
}

void TopoBuilder::AppendPolygon(Polygon*poly,int attribute){
    AutoPtr<CurvePolygon>curpoly=AutoPtr<Polygon>(poly,true);
    if(curpoly!=nullptr){
        AutoPtr<Polygon>newpoly=curpoly->ConvertToDefaultPolygon(true);
        for(int k=0;k<newpoly->GetSize();k++){
            AutoPtr<Points>ppts=newpoly->GetItem(k);
            AppendRing(ppts.get(),attribute);
        }
    }else{
        for(int k=0;k<poly->GetSize();k++){
            AutoPtr<Points>ppts=poly->GetItem(k);
            AppendRing(ppts.get(),attribute);
        }
    }
}

void TopoBuilder::AppendEnvelope(Envelope*env,int attribute){
    AutoPtr<Polygon>poly=env->ConvertToPolygon();
    AppendPolygon(poly.get(),attribute);
}

void TopoBuilder::AppendEllipse(Ellipse*env,int attribute){
    AutoPtr<Polygon>poly=env->ConvertToPolygon(true);
    AppendPolygon(poly.get(),attribute);
}

void TopoBuilder::AppendMultiPolygon(MultiPolygon*mpoly,int attribute){
    for(int k=0;k<mpoly->GetSize();k++){
        AutoPtr<Polygon>subpoly=mpoly->GetItem(k);
        AppendPolygon(subpoly.get(),attribute);
    }
}

void TopoBuilder::AppendGeometry(Geometry*geo,int attribute){
    switch(geo->GetType()){
    case GeometryType::gtPoint:
        AppendPoint((Point*)geo);
        break;
    case GeometryType::gtPoints:
        AppendPoints((Points*)geo);
        break;
    case GeometryType::gtSegment:
        AppendSegment((Segment*)geo,attribute);
        break;
    case GeometryType::gtEnvelope:
        AppendEnvelope((Envelope*)geo,attribute);
        break;
    case GeometryType::gtEllipse:
        AppendEllipse((Ellipse*)geo,attribute);
        break;
    case GeometryType::gtPolyline:
        AppendPolyline((Polyline*)geo,attribute);
        break;
    case GeometryType::gtPolygon:
        AppendPolygon((Polygon*)geo,attribute);
        break;
    case GeometryType::gtMultiPolygon:
        AppendMultiPolygon((MultiPolygon*)geo,attribute);
        break;
    case GeometryType::gtCollection:
        GeometryCollection*gCol=(GeometryCollection*)geo;
        for(int k=0;k<gCol->GetSize();k++) AppendGeometry(gCol->GetItem(k).get(),attribute);
        break;
    }
}

bool TopoBuilder::IntersectLineOfSameNode(TopoLine*tLine1,TopoLine*tLine2,bool&delLine)
{
    if((tLine1->fromNode==tLine2->fromNode)&&(tLine1->toNode==tLine2->toNode))
    {
        tLine1->CombineAttributes(tLine2);
        tLines->RemoveLine(tLine2);
        delLine=true;
        return false;
    }
    if((tLine1->fromNode==tLine2->toNode)&&(tLine1->toNode==tLine2->fromNode)){
        tLine1->CombineAttributes(tLine2,true);
        tLines->RemoveLine(tLine2);
        delLine=true;
        return false;
    }
    if(tLine2->fromNode->IsSamePoint(tLine2->toNode,0.000001)){
        if(tLine1->fromNode==tLine2->fromNode){
            tLine1->CombineFromPoint(tLine2->toNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return false;
        }
        if(tLine1->toNode==tLine2->fromNode){
            tLine1->CombineToPoint(tLine2->toNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return false;
        }
        if(tLine1->fromNode==tLine2->toNode){
            tLine1->CombineFromPoint(tLine2->fromNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return false;
        }
        if(tLine1->toNode==tLine2->toNode)
        {
            tLine1->CombineToPoint(tLine2->fromNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return false;
        }
    }
    if(tLine1->fromNode->IsSamePoint(tLine1->toNode,0.000001))
    {
        if(tLine1->fromNode==tLine2->fromNode)
        {
            tLine1->CopyAttributes(tLine2);
            tLine1->SetToNode(tLine2->toNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return true;
        }
        if(tLine1->fromNode==tLine2->toNode)
        {
            tLine1->CopyAttributes(tLine2);
            tLine1->SetToNode(tLine2->fromNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return true;
        }
        if(tLine1->toNode==tLine2->fromNode)
        {
            tLine1->CopyAttributes(tLine2);
            tLine1->SetFromNode(tLine2->toNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return true;
        }
        if(tLine1->toNode==tLine2->toNode)
        {
            tLine1->CopyAttributes(tLine2);
            tLine1->SetFromNode(tLine2->fromNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return true;
        }
    }
    Segment2D<double> seg1(tLine1->fromNode->X,tLine1->fromNode->Y,tLine1->toNode->X,tLine1->toNode->Y);
    Segment2D<double> seg2(tLine2->fromNode->X,tLine2->fromNode->Y,tLine2->toNode->X,tLine2->toNode->Y);
    if(tLine1->fromNode==tLine2->fromNode)
    {
        if(tLine1->toNode->IsSamePoint(tLine2->toNode,0.000001))
        {
            tLine1->CombineAttributes(tLine2);
            tLine1->CombineToPoint(tLine2->toNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return false;
        }
        bool IsFSegToPtIn=seg2.IsPointOnSegment(tLine1->toNode->X,tLine1->toNode->Y,fuzzy);
        if(IsFSegToPtIn)
        {
            tLine1->CombineAttributes(tLine2);
            tLine2->SetFromNode(tLine1->toNode);
            return false;
        }
        bool IsTSegToPtIn=seg1.IsPointOnSegment(tLine2->toNode->X,tLine2->toNode->Y,fuzzy);
        if(IsTSegToPtIn)
        {
            tLine2->CombineAttributes(tLine1);
            bool IsSameRegion=tLines->regions->IsSameRegion(tLine1->fromNode,tLine2->toNode);
            tLine1->SetFromNode(tLine2->toNode);
            return (!IsSameRegion);
        }
    }
    if(tLine1->toNode==tLine2->toNode)
    {
        if(tLine1->fromNode->IsSamePoint(tLine2->fromNode,0.000001))
        {
            tLine1->CombineAttributes(tLine2);
            tLine1->CombineFromPoint(tLine2->fromNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return false;
        }
        bool IsFSegFromPtIn=seg2.IsPointOnSegment(tLine1->fromNode->X,tLine1->fromNode->Y,fuzzy);
        if(IsFSegFromPtIn)
        {
            tLine1->CombineAttributes(tLine2);
            tLine2->SetToNode(tLine1->fromNode);
            return false;
        }
        bool IsTSegFromPtIn=seg1.IsPointOnSegment(tLine2->fromNode->X,tLine2->fromNode->Y,fuzzy);
        if(IsTSegFromPtIn)
        {
            tLine2->CombineAttributes(tLine1);
            tLine1->SetToNode(tLine2->fromNode);
            return true;
        }
    }
    if(tLine1->fromNode==tLine2->toNode)
    {
        if(tLine1->toNode->IsSamePoint(tLine2->fromNode,0.000001))
        {
            tLine1->CombineAttributes(tLine2,true);
            tLine1->CombineToPoint(tLine2->fromNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return false;
        }
        bool IsFSegToPtIn=seg2.IsPointOnSegment(tLine1->toNode->X,tLine1->toNode->Y,fuzzy);
        if(IsFSegToPtIn)
        {
            tLine1->CombineAttributes(tLine2,true);
            tLine2->SetToNode(tLine1->toNode);
            return false;
        }
        bool IsTSegFromPtIn=seg1.IsPointOnSegment(tLine2->fromNode->X,tLine2->fromNode->Y,fuzzy);
        if(IsTSegFromPtIn)
        {
            tLine2->CombineAttributes(tLine1,true);
            tLine1->SetFromNode(tLine2->fromNode);
            return true;
        }
    }
    if(tLine1->toNode==tLine2->fromNode)
    {
        if(tLine1->fromNode->IsSamePoint(tLine2->toNode,0.000001))
        {
            tLine1->CombineAttributes(tLine2,true);
            tLine1->CombineFromPoint(tLine2->toNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return false;
        }
        bool IsFSegFromPtIn=seg2.IsPointOnSegment(tLine1->fromNode->X,tLine1->fromNode->Y,fuzzy);
        if(IsFSegFromPtIn)
        {
            tLine1->CombineAttributes(tLine2,true);
            tLine2->SetFromNode(tLine1->fromNode);
            return false;
        }
        bool IsTSegToPtIn=seg1.IsPointOnSegment(tLine2->toNode->X,tLine2->toNode->Y,fuzzy);
        if(IsTSegToPtIn)
        {
            tLine2->CombineAttributes(tLine1,true);
            tLine1->SetToNode(tLine2->toNode);
            return true;
        }
    }
    return false;
}
bool TopoBuilder::IntersectLine(TopoLine*tLine1,TopoLine*tLine2,bool&delLine)
{
    delLine=false;
    if((tLine1->fromNode==tLine2->fromNode)||(tLine1->fromNode==tLine2->toNode)||(tLine1->toNode==tLine2->fromNode)||(tLine1->toNode==tLine2->toNode))
    {
        return IntersectLineOfSameNode(tLine1,tLine2,delLine);
    }
    double xmin,ymin,xmax,ymax;
    double xmin2,ymin2,xmax2,ymax2;
    if(tLine1->fromNode->X<=tLine1->toNode->X)
    {
        xmin=tLine1->fromNode->X;
        xmax=tLine1->toNode->X;
    }
    else
    {
        xmax=tLine1->fromNode->X;
        xmin=tLine1->toNode->X;
    }
    if(tLine2->fromNode->X<=tLine2->toNode->X)
    {
        xmin2=tLine2->fromNode->X;
        xmax2=tLine2->toNode->X;
    }
    else
    {
        xmax2=tLine2->fromNode->X;
        xmin2=tLine2->toNode->X;
    }
    if((xmin2>xmax+fuzzy)||(xmax2<xmin-fuzzy)) return false;
    if(tLine1->fromNode->Y<=tLine1->toNode->Y)
    {
        ymin=tLine1->fromNode->Y;
        ymax=tLine1->toNode->Y;
    }
    else
    {
        ymax=tLine1->fromNode->Y;
        ymin=tLine1->toNode->Y;
    }
    if(tLine2->fromNode->Y<=tLine2->toNode->Y)
    {
        ymin2=tLine2->fromNode->Y;
        ymax2=tLine2->toNode->Y;
    }
    else
    {
        ymax2=tLine2->fromNode->Y;
        ymin2=tLine2->toNode->Y;
    }
    if((ymin2>ymax+fuzzy)||(ymax2<ymin-fuzzy)) return false;
    if(tLine2->fromNode->IsSamePoint(tLine2->toNode,0.000001))
    {
        if(tLine1->fromNode->IsSamePoint(tLine2->fromNode,0.000001))
        {
            tLine1->CombineFromPoint(tLine2->fromNode);
            tLine1->CombineFromPoint(tLine2->toNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return false;
        }
        if(tLine1->toNode->IsSamePoint(tLine2->fromNode,0.000001))
        {
            tLine1->CombineToPoint(tLine2->fromNode);
            tLine1->CombineToPoint(tLine2->toNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return false;
        }
    }
    if(tLine1->fromNode->IsSamePoint(tLine1->toNode,0.000001))
    {
        if(tLine2->fromNode->IsSamePoint(tLine1->fromNode,0.000001))
        {
            tLine1->CopyAttributes(tLine2);
            tLine1->SetToNode(tLine2->toNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
            return true;
        }
        if(tLine1->toNode->IsSamePoint(tLine2->fromNode,0.000001))
        {
            tLine1->CopyAttributes(tLine2);
            tLine1->SetFromNode(tLine2->toNode);
            tLines->RemoveLine(tLine2);
            delLine=true;
             return true;
        }
    }
    if((tLine1->fromNode->IsSamePoint(tLine2->fromNode,0.000001))&&(tLine1->toNode->IsSamePoint(tLine2->toNode,0.000001)))
    {
        tLine1->CombineAttributes(tLine2);
        tLine1->CombineFromPoint(tLine2->fromNode);
        tLine1->CombineToPoint(tLine2->toNode);
        tLines->RemoveLine(tLine2);
        delLine=true;
        return false;
    }
    if((tLine1->toNode->IsSamePoint(tLine2->fromNode,0.000001))&&(tLine1->fromNode->IsSamePoint(tLine2->toNode,0.000001)))
    {
        tLine1->CombineAttributes(tLine2,true);
        tLine1->CombineFromPoint(tLine2->toNode);
        tLine1->CombineToPoint(tLine2->fromNode);
        tLines->RemoveLine(tLine2);
        delLine=true;
        return false;
    }
    Segment2D<double> seg1(tLine1->fromNode->X,tLine1->fromNode->Y,tLine1->toNode->X,tLine1->toNode->Y);
    Segment2D<double> seg2(tLine2->fromNode->X,tLine2->fromNode->Y,tLine2->toNode->X,tLine2->toNode->Y);
    if(tLine1->fromNode->IsSamePoint(tLine2->fromNode,0.000001))
    {
        tLine1->CombineFromPoint(tLine2->fromNode);
        bool IsFSegToPtIn=seg2.IsPointOnSegment(tLine1->toNode->X,tLine1->toNode->Y,fuzzy);
        if(IsFSegToPtIn)
        {
            tLine1->CombineAttributes(tLine2);
            tLine2->SetFromNode(tLine1->toNode);
            return false;
        }
        bool IsTSegToPtIn=seg1.IsPointOnSegment(tLine2->toNode->X,tLine2->toNode->Y,fuzzy);
        if(IsTSegToPtIn)
        {
            tLine2->CombineAttributes(tLine1);
            tLine1->SetFromNode(tLine2->toNode);
            return true;
        }
        return false;
    }
    if(tLine1->fromNode->IsSamePoint(tLine2->toNode,0.000001))
    {
        tLine1->CombineFromPoint(tLine2->toNode);
        bool IsFSegToPtIn=seg2.IsPointOnSegment(tLine1->toNode->X,tLine1->toNode->Y,fuzzy);
        if(IsFSegToPtIn)
        {
            tLine1->CombineAttributes(tLine2,true);
            tLine2->SetToNode(tLine1->toNode);
            return false;
        }
        bool IsTSegFromPtIn=seg1.IsPointOnSegment(tLine2->fromNode->X,tLine2->fromNode->Y,fuzzy);
        if(IsTSegFromPtIn)
        {
            tLine2->CombineAttributes(tLine1,true);
            tLine1->SetFromNode(tLine2->fromNode);
            return true;
        }
        return false;
    }
    if(tLine1->toNode->IsSamePoint(tLine2->fromNode,0.000001))
    {
        tLine1->CombineToPoint(tLine2->fromNode);
        bool IsFSegFromPtIn=seg2.IsPointOnSegment(tLine1->fromNode->X,tLine1->fromNode->Y,fuzzy);
        if(IsFSegFromPtIn)
        {
            tLine1->CombineAttributes(tLine2,true);
            tLine2->SetFromNode(tLine1->fromNode);
            return false;
        }
        bool IsTSegToPtIn=seg1.IsPointOnSegment(tLine2->toNode->X,tLine2->toNode->Y,fuzzy);
        if(IsTSegToPtIn)
        {
            tLine2->CombineAttributes(tLine1,true);
            tLine1->SetToNode(tLine2->toNode);
            return true;
        }
        return false;
    }
    if(tLine1->toNode->IsSamePoint(tLine2->toNode,0.000001))
    {
        tLine1->CombineToPoint(tLine2->toNode);
        bool IsFSegFromPtIn=seg2.IsPointOnSegment(tLine1->fromNode->X,tLine1->fromNode->Y,fuzzy);
        if(IsFSegFromPtIn)
        {
            tLine1->CombineAttributes(tLine2);
            tLine2->SetToNode(tLine1->fromNode);
            return false;
        }
        bool IsTSegFromPtIn=seg1.IsPointOnSegment(tLine2->fromNode->X,tLine2->fromNode->Y,fuzzy);
        if(IsTSegFromPtIn)
        {
            tLine2->CombineAttributes(tLine1);
            tLine1->SetToNode(tLine2->fromNode);
            return true;
        }
        return false;
    }
    bool IsFSegFromPtIn=seg2.IsPointOnSegment(tLine1->fromNode->X,tLine1->fromNode->Y,fuzzy);
    bool IsFSegToPtIn=seg2.IsPointOnSegment(tLine1->toNode->X,tLine1->toNode->Y,fuzzy);
    if((IsFSegFromPtIn)&&(IsFSegToPtIn))
    {
        double dist1=sqrt((tLine2->fromNode->X-tLine1->fromNode->X)*(tLine2->fromNode->X-tLine1->fromNode->X)+(tLine2->fromNode->Y-tLine1->fromNode->Y)*(tLine2->fromNode->Y-tLine1->fromNode->Y));
        double dist2=sqrt((tLine2->fromNode->X-tLine1->toNode->X)*(tLine2->fromNode->X-tLine1->toNode->X)+(tLine2->fromNode->Y-tLine1->toNode->Y)*(tLine2->fromNode->Y-tLine1->toNode->Y));
        if(dist1<=dist2)
        {
            TopoLine*newl=new TopoLine(tLine1->toNode,tLine2->toNode);
            newl->CopyAttributes(tLine2);
            tLines->AddLine(newl);
            tLine1->CombineAttributes(tLine2);
            tLine2->SetToNode(tLine1->fromNode);
        }
        else
        {
             TopoLine*newl=new TopoLine(tLine1->fromNode,tLine2->toNode);
             newl->CopyAttributes(tLine2);
             tLines->AddLine(newl);
             tLine1->CombineAttributes(tLine2,true);
             tLine2->SetToNode(tLine1->toNode);
        }
        return false;
    }
    bool IsTSegFromPtIn=seg1.IsPointOnSegment(tLine2->fromNode->X,tLine2->fromNode->Y,fuzzy);
    bool IsTSegToPtIn=seg1.IsPointOnSegment(tLine2->toNode->X,tLine2->toNode->Y,fuzzy);
    if((IsTSegFromPtIn)&&(IsTSegToPtIn))
    {
        double dist1=sqrt((tLine1->fromNode->X-tLine2->fromNode->X)*(tLine1->fromNode->X-tLine2->fromNode->X)+(tLine1->fromNode->Y-tLine2->fromNode->Y)*(tLine1->fromNode->Y-tLine2->fromNode->Y));
        double dist2=sqrt((tLine1->fromNode->X-tLine2->toNode->X)*(tLine1->fromNode->X-tLine2->toNode->X)+(tLine1->fromNode->Y-tLine2->toNode->Y)*(tLine1->fromNode->Y-tLine2->toNode->Y));
        if(dist1<=dist2)
        {
            TopoLine*newl=new TopoLine(tLine2->toNode,tLine1->toNode);
            newl->CopyAttributes(tLine1);
            tLines->AddLine(newl);
            tLine2->CombineAttributes(tLine1);
            tLine1->SetToNode(tLine2->fromNode);
        }
        else
        {
            TopoLine*newl=new TopoLine(tLine2->fromNode,tLine1->toNode);
            newl->CopyAttributes(tLine1);
            tLines->AddLine(newl);
            tLine2->CombineAttributes(tLine1,true);
            tLine1->SetToNode(tLine2->toNode);
        }
        return true;
    }
    if((IsFSegFromPtIn)&&(IsTSegToPtIn))
    {
        TopoLine*newl=new TopoLine(tLine2->toNode,tLine1->toNode);
        newl->CopyAttributes(tLine1);
        tLines->AddLine(newl);
        tLine1->SetToNode(tLine2->toNode);
        tLine1->CombineAttributes(tLine2);
        tLine2->SetToNode(tLine1->fromNode);
        return true;
    }
    if((IsFSegFromPtIn)&&(IsTSegFromPtIn))
    {
        TopoLine*newl=new TopoLine(tLine1->fromNode,tLine2->toNode);
        newl->CopyAttributes(tLine2);
        tLines->AddLine(newl);
        tLine2->SetToNode(tLine1->fromNode);
        tLine2->CombineAttributes(tLine1,true);
        tLine1->SetFromNode(tLine2->fromNode);
        return true;
    }
    if((IsFSegToPtIn)&&(IsTSegFromPtIn))
    {
        TopoLine*newl=new TopoLine(tLine1->toNode,tLine2->toNode);
        newl->CopyAttributes(tLine2);
        tLines->AddLine(newl);
        tLine2->SetToNode(tLine1->toNode);
        tLine2->CombineAttributes(tLine1);
        tLine1->SetToNode(tLine2->fromNode);
        return true;
    }
    if((IsFSegToPtIn)&&(IsTSegToPtIn))
    {
        TopoLine*newl=new TopoLine(tLine2->fromNode,tLine1->toNode);
        newl->CopyAttributes(tLine2);
        tLines->AddLine(newl);
        tLine2->SetFromNode(tLine1->toNode);
        tLine2->CombineAttributes(tLine1,true);
        tLine1->SetToNode(tLine2->toNode);
        return true;
    }
    TopoPoint*fp1=tLine1->fromNode;
    TopoPoint*tp1=tLine1->toNode;
    TopoPoint*fp2=tLine2->fromNode;
    TopoPoint*tp2=tLine2->toNode;
    if(IsFSegFromPtIn)
    {
        TopoPoint*inp=fp1;
        TopoLine*newln4=new TopoLine(inp,tp2);
        newln4->CopyAttributes(tLine2);
        tLines->AddLine(newln4);
        tLine2->SetToNode(inp);
        return false;
    }
    else if(IsFSegToPtIn)
    {
        TopoPoint*inp=tp1;
        TopoLine*newln4=new TopoLine(inp,tp2);
        newln4->CopyAttributes(tLine2);
        tLines->AddLine(newln4);
        tLine2->SetToNode(inp);
        return false;
    }
    else if(IsTSegFromPtIn)
    {
        TopoPoint*inp=fp2;
        TopoLine*newln2=new TopoLine(inp,tp1);
        newln2->CopyAttributes(tLine1);
        tLines->AddLine(newln2);
        tLine1->SetToNode(inp);
        return true;
    }
    else if(IsTSegToPtIn)
    {
        TopoPoint*inp=tp2;
        TopoLine*newln2=new TopoLine(inp,tp1);
        newln2->CopyAttributes(tLine1);
        tLines->AddLine(newln2);
        tLine1->SetToNode(inp);
        return true;
    }
    double dist1=sqrt((seg1.lnx1-seg2.lnx1)*(seg1.lnx1-seg2.lnx1)+(seg1.lny1-seg2.lny1)*(seg1.lny1-seg2.lny1));
    double dist2=sqrt((seg1.lnx1-seg2.lnx2)*(seg1.lnx1-seg2.lnx2)+(seg1.lny1-seg2.lny2)*(seg1.lny1-seg2.lny2));
    double dist3=sqrt((seg1.lnx2-seg2.lnx1)*(seg1.lnx2-seg2.lnx1)+(seg1.lny2-seg2.lny1)*(seg1.lny2-seg2.lny1));
    double dist4=sqrt((seg1.lnx2-seg2.lnx2)*(seg1.lnx2-seg2.lnx2)+(seg1.lny2-seg2.lny2)*(seg1.lny2-seg2.lny2));
    double mindist=dist1;
    if(dist2<dist1) mindist=dist2;
    if(dist3<mindist) mindist=dist3;
    if(dist4<mindist) mindist=dist4;
    if(mindist<=fuzzy)
    {
        if(mindist==dist1)
        {
            tLine2->SetFromNode(tLine1->fromNode);
        }
        else if(mindist==dist2)
        {
            tLine2->SetToNode(tLine1->fromNode);
        }
        else if(mindist==dist3)
        {
            tLine2->SetFromNode(tLine1->toNode);
        }
        else if(mindist==dist4)
        {
            tLine2->SetToNode(tLine1->toNode);
        }
        return false;
    }
    Point2D<double> dpt;
    if(!seg1.IntersectofSegment(seg2,dpt)) return false;
    seg1.Fuzzy=fuzzy;
    seg2.Fuzzy=fuzzy;
    dist1=dpt.DistanceTo(seg1.lnx1,seg1.lny1);
    dist2=dpt.DistanceTo(seg1.lnx2,seg1.lny2);
    dist3=dpt.DistanceTo(seg2.lnx1,seg2.lny1);
    dist4=dpt.DistanceTo(seg2.lnx2,seg2.lny2);
    double mindist1=dist1;
    if(dist2<mindist1) mindist1=dist2;
    double mindist2=dist3;
    if(dist4<mindist2) mindist2=dist4;
    if((mindist1<=fuzzy)&&(mindist1<mindist2))
    {
        if(dist1<dist2)
        {
            if(!fp2->FindLine(fp1))
            {
                TopoLine*newline=new TopoLine();
                newline->fromNode=fp2;
                newline->toNode=fp1;
                tLines->AddLine(newline);
                newline->CopyAttributes(tLine2);
            }
            tLine2->SetFromNode(fp1);
        }
        else
        {
            if(!fp2->FindLine(tp1))
            {
                TopoLine*newline=new TopoLine();
                newline->fromNode=fp2;
                newline->toNode=tp1;
                tLines->AddLine(newline);
                newline->CopyAttributes(tLine2);
            }
            tLine2->SetFromNode(tp1);
        }
        return false;
    }
    if((mindist2<=fuzzy)&&(mindist2<mindist1))
    {
        if(dist3<dist4)
        {
            if(!fp1->FindLine(fp2))
            {
                TopoLine*newline=new TopoLine();
                newline->fromNode=fp1;
                newline->toNode=fp2;
                tLines->AddLine(newline);
                newline->CopyAttributes(tLine1);
            }
            tLine1->SetFromNode(fp2);
        }
        else
        {
            if(!fp1->FindLine(tp2))
            {
                TopoLine*newline=new TopoLine();
                newline->fromNode=fp1;
                newline->toNode=tp2;
                tLines->AddLine(newline);
                newline->CopyAttributes(tLine1);
            }
            tLine1->SetFromNode(tp2);
        }
        return true;
    }
    TopoPoint*inp=new TopoPoint(dpt.X,dpt.Y);
    tPoints->AddPoint(inp);
    TopoLine*newln2=new TopoLine(inp,tp1);
    newln2->CopyAttributes(tLine1);
    tLines->AddLine(newln2);
    TopoLine*newln4=new TopoLine(inp,tp2);
    newln4->CopyAttributes(tLine2);
    tLines->AddLine(newln4);
    tLine1->SetToNode(inp);
    tLine2->SetToNode(inp);
    return true;
}
bool TopoBuilder::RemoveZeroLine(TopoLine*tLine)
{
    if((tLine->fromNode->X==tLine->toNode->X)&&(tLine->fromNode->Y==tLine->toNode->Y))
    {
        int vertexCount=tLine->vertices.size();
        if(vertexCount>0) return false;
        if(tLine->fromNode!=tLine->toNode)
        {
            int lineCount=tLine->toNode->lines.size();
            for(int k=0;k<lineCount;k++)
            {
                TopoLine*line=tLine->toNode->lines[k];
                if(line==tLine) continue;
                if(line->fromNode==tLine->toNode)
                    line->fromNode=tLine->fromNode;
                if(line->toNode==tLine->toNode)
                    line->toNode=tLine->fromNode;
                tLine->fromNode->AddLine(line);
            }
            tLine->toNode->lines.clear();
            tPoints->RemovePoint(tLine->toNode);
        }
        tLines->RemoveLine(tLine);
        return true;
    }
    return false;
}
void TopoBuilder::IntersectLine(TopoLine*tLine)
{
    LineRegions*regions=tLines->regions;
    int xNum=regions->GetRegionXNum();
    int yNum=regions->GetRegionYNum();
    int count=tLine->regionEls.size();
    bool HasChanged=false;
    for(int k=0;k<count;k++)
    {
        LineRegion*region=tLine->regionEls[k]->lineRegion;
        int regCount=region->GetCount();
        if(regCount==1)
        {
            continue;
        }
        LineRegionElement*lineEl=region->rootElement;
        while(lineEl!=nullptr)
        {
            TopoLine*nextLine=lineEl->topoLine;
            if(nextLine==tLine)
            {
                lineEl=lineEl->child;
                regCount--;
                continue;
            }
            if(nextLine->state==1)
            {
                lineEl=lineEl->child;
                regCount--;
                continue;
            }
            LineRegionElement*father=lineEl->father;
            bool delLine;
            if(IntersectLine(tLine,nextLine,delLine)) HasChanged=true;
            if(delLine)
            {
                if(father==nullptr)
                    lineEl=region->rootElement;
                else
                    lineEl=father->child;
            }
            else
                lineEl=lineEl->child;
            regCount--;
            if(regCount<=0) break;
        }
    }
    if(HasChanged)
    {
        tLines->regions->UpdateLine(tLine);
    }
}
void TopoBuilder::IntersectLines(int regionNum)
{
    if(fullExtent==nullptr) return;
    tLines->BuildSpatialIndex(fullExtent.get(),regionNum);
    TopoLine*currentLine=tLines->rootLine;
    while(currentLine!=nullptr)
    {
        if(currentLine->state==1)
        {
            currentLine=currentLine->child;
            continue;
        }
        currentLine->state=1;
        IntersectLine(currentLine);
        currentLine=currentLine->child;
    }
    currentLine=tLines->rootLine;
    while(currentLine!=nullptr)
    {
        currentLine->state=0;
        TopoLine*father=currentLine->father;
        if(RemoveZeroLine(currentLine))
        {
            if(father==nullptr)
                currentLine=tLines->rootLine;
            else
                currentLine=father->child;
        }
        else
            currentLine=currentLine->child;
    }
    TopoPoint*currentPoint=tPoints->rootPoint;
    while(currentPoint!=nullptr)
    {
        TopoPoint*father=currentPoint->father;
        if(currentPoint->lines.size()==0)
        {
            tPoints->RemovePoint(currentPoint);
            if(father==nullptr)
                currentPoint=tPoints->rootPoint;
            else
                currentPoint=father->child;
        }
        else
            currentPoint=currentPoint->child;
    }
}
void TopoBuilder::PursuitLine(TopoLine*tLine,TopoPoint*fromPoint)
{
    tLine->state=1;
    TopoPoint*nextPoint;
    bool IsFrom=true;
    if(tLine->fromNode==fromPoint)
        nextPoint=tLine->toNode;
    else
    {
        nextPoint=tLine->fromNode;
        IsFrom=false;
    }
    pArray.InitialFromArray(tLine->vertices,!IsFrom);
    while(true)
    {
        int segCount=nextPoint->lines.size();
        if(segCount!=2) break;
        if(nextPoint==fromPoint) break;
        TopoLine*nextLine;
        if(nextPoint->lines[0]==tLine)
            nextLine=nextPoint->lines[1];
        else
            nextLine=nextPoint->lines[0];
        if(nextLine->state==1) break;
        TopoPoint*next;
        pArray.Add(Point2D<double>(nextPoint->X,nextPoint->Y));
        if(nextLine->fromNode==nextPoint)
        {
            int vertexCount=nextLine->vertices.size();
            for(int p=0;p<vertexCount;p++) pArray.Add(nextLine->vertices[p]);
            next=nextLine->toNode;
        }
        else
        {
            int vertexCount=nextLine->vertices.size();
            for(int p=vertexCount-1;p>=0;p--) pArray.Add(nextLine->vertices[p]);
            next=nextLine->fromNode;
        }
        nextPoint->RemoveLine(tLine);
        nextPoint->RemoveLine(nextLine);
        next->RemoveLine(nextLine);
        tLines->RemoveLine(nextLine);
        tPoints->RemovePoint(nextPoint);
        next->AddLine(tLine);
        if(IsFrom)
           tLine->toNode=next;
        else
           tLine->fromNode=next;
        nextPoint=next;
    }
    int dataCount=pArray.GetSize();
    if(dataCount==0) return;
    tLine->vertices.resize(dataCount);
    if(IsFrom)
    {
        Point2D<double>*data=tLine->vertices.data();
        Point2D<double>*data2=pArray.GetData();
        memcpy((BYTE*)data,(BYTE*)data2,sizeof(double)*2*dataCount);
    }
    else
    {
        int pin=0;
        vector<Point2D<double>>*data=pArray.GetArray();
        for(int k=dataCount-1;k>=0;k--)
        {
            tLine->vertices[pin++]=(*data)[k];
        }
    }
}
void TopoBuilder::BuildLineTopo()
{
    //首先，对于线条数大于等于3个的结点完成追踪
    if(tLines->regions!=nullptr)
    {
        delete tLines->regions;
        tLines->regions=nullptr;
    }
    TopoPoint*currentPoint=tPoints->rootPoint;
    while(currentPoint!=nullptr)
    {
        int segCount=currentPoint->lines.size();
        if(segCount==2)
        {
            currentPoint=currentPoint->child;
            continue;
        }
        for(int k=segCount-1;k>=0;k--)
        {
            TopoLine*tLine=currentPoint->lines[k];
            if(tLine->state!=0) continue;
            PursuitLine(tLine,currentPoint);
        }
        currentPoint=currentPoint->child;
    }
    currentPoint=tPoints->rootPoint;
    //然后，对于线条数等于2个的结点完成追踪，这些都是孤立闭合的线
    while(currentPoint!=nullptr)
    {
        int segCount=currentPoint->lines.size();
        if(segCount!=2)
        {
            currentPoint=currentPoint->child;
            continue;
        }
        for(int k=segCount-1;k>=0;k--)
        {
            TopoLine*tLine=currentPoint->lines[k];
            if(tLine->state!=0) continue;
            PursuitLine(tLine,currentPoint);
        }
        currentPoint=currentPoint->child;
    }
}
void TopoBuilder::RemovePhesudos()
{
    TopoPoint*currentPoint=tPoints->rootPoint;
    while(currentPoint!=nullptr)
    {
        if(currentPoint->lines.size()!=2)
        {
            currentPoint=currentPoint->child;
            continue;
        }
        TopoPoint*father=currentPoint->father;
        if(RemovePhesudo(currentPoint))
        {
            if(father==nullptr)
                currentPoint=tPoints->rootPoint;
            else
                currentPoint=father->child;
        }
        else
            currentPoint=currentPoint->child;
    }
}
void TopoBuilder::RemoveIsolateLine()
{
    TopoLine*currentLine=tLines->rootLine;
    while(currentLine!=nullptr)
    {
        TopoLine*father=currentLine->father;
        if(((currentLine->fromNode->lines.size()<=1)||(currentLine->toNode->lines.size()<=1))&&(currentLine->fromNode!=currentLine->toNode))
        {
            tLines->RemoveLine(currentLine);
            if(father==nullptr)
                currentLine=tLines->rootLine;
            else
                currentLine=father->child;
        }
        else
            currentLine=currentLine->child;
    }
    TopoPoint*currentPoint=tPoints->rootPoint;
    while(currentPoint!=nullptr)
    {
        TopoPoint*father=currentPoint->father;
        if(currentPoint->lines.size()==0)
        {
            tPoints->RemovePoint(currentPoint);
            if(father==nullptr)
                currentPoint=tPoints->rootPoint;
            else
                currentPoint=father->child;
        }
        else
            currentPoint=currentPoint->child;
    }
}
double TopoBuilder::TriangleAngle(Point2D<double>&fromPoint,Point2D<double>&centerPoint,Point2D<double>&toPoint)
{
    Segment2D<double> seg1(centerPoint.X,centerPoint.Y,fromPoint.X,fromPoint.Y);
    double angle1=seg1.Angle();
    Segment2D<double> seg2(centerPoint.X,centerPoint.Y,toPoint.X,toPoint.Y);
    double angle2=seg2.Angle();
    angle2-=angle1;
    if(angle2<0) angle2=DPI+angle2;
    return angle2;
}
TopoLine* TopoBuilder::GetNextLine(TopoPoint*centerPoint,TopoLine*fromLine)
{
    Point2D<double> fromPoint;
    if(fromLine->fromNode==centerPoint)
    {
        if(fromLine->vertices.size()>0)
            fromPoint=fromLine->vertices[0];
        else
            fromPoint=Point2D<double>(fromLine->toNode->X,fromLine->toNode->Y);
    }
    else
    {
        if(fromLine->vertices.size()>0)
            fromPoint=fromLine->vertices[fromLine->vertices.size()-1];
        else
            fromPoint=Point2D<double>(fromLine->fromNode->X,fromLine->fromNode->Y);
    }
    double angle=6.3;
    TopoLine*targetLine=nullptr;
    for(int k=centerPoint->lines.size()-1;k>=0;k--)
    {
        TopoLine*nextLine=centerPoint->lines[k];
        if(nextLine==fromLine) continue;
        Point2D<double> toPoint;
        if(nextLine->fromNode==centerPoint)
        {
            if(nextLine->vertices.size()>0)
                toPoint=nextLine->vertices[0];
            else
                toPoint=Point2D<double>(nextLine->toNode->X,nextLine->toNode->Y);
        }
        else
        {
            if(nextLine->vertices.size()>0)
                toPoint=nextLine->vertices[nextLine->vertices.size()-1];
            else
                toPoint=Point2D<double>(nextLine->fromNode->X,nextLine->fromNode->Y);
        }
        Point2D<double> centerpt(centerPoint->X,centerPoint->Y);
        double tangle=TriangleAngle(fromPoint,centerpt,toPoint);
        if(tangle<angle)
        {
            targetLine=nextLine;
            angle=tangle;
        }
    }
    return targetLine;
}
bool TopoBuilder::RemovePhesudo(TopoPoint*topoPoint)
{
    if(topoPoint->lines.size()!=2) return false;
    TopoLine*tLine1=topoPoint->lines[0];
    TopoLine*tLine2=topoPoint->lines[1];
    if(tLine1->fromNode==tLine1->toNode) return false;
    if(tLine2->fromNode==tLine2->toNode) return false;
    if(tLine1->toNode==topoPoint)
    {
        bool IsFrom=(tLine2->fromNode==topoPoint);
        int pin=tLine1->vertices.size();
        int totalSize=pin+tLine2->vertices.size()+1;
        tLine1->vertices.resize(totalSize);
        tLine1->vertices[pin++]=Point2D<double>(tLine1->toNode->X,tLine1->toNode->Y);
        tLine1->toNode->lines.clear();
        if(IsFrom)
        {
            for(int k=0;k<tLine2->vertices.size();k++) tLine1->vertices[pin++]=tLine2->vertices[k];
            tLine2->toNode->RemoveLine(tLine2);
            tLine1->toNode=tLine2->toNode;
            tLine1->toNode->AddLine(tLine1);
        }
        else
        {
            for(int k=tLine2->vertices.size()-1;k>=0;k--) tLine1->vertices[pin++]=tLine2->vertices[k];
            tLine2->fromNode->RemoveLine(tLine2);
            tLine1->toNode=tLine2->fromNode;
            tLine1->toNode->AddLine(tLine1);
        }
    }
    else
    {
        bool IsFrom=(tLine2->fromNode==topoPoint);
        int lnSize=tLine2->vertices.size()+1;
        vector<Point2D<double>>tps;
        tps.resize(lnSize);
        int pin=lnSize-1;
        tps[pin--]=Point2D<double>(tLine1->fromNode->X,tLine1->fromNode->Y);
        tLine1->fromNode->lines.clear();
        if(IsFrom)
        {
            for(int k=0;k<tLine2->vertices.size();k++) tps[pin--]=tLine2->vertices[k];
            tLine2->toNode->RemoveLine(tLine2);
            tLine1->fromNode=tLine2->toNode;
            tLine1->fromNode->AddLine(tLine1);
        }
        else
        {
            for(int k=tLine2->vertices.size()-1;k>=0;k--) tps[pin--]=tLine2->vertices[k];
            tLine2->fromNode->RemoveLine(tLine2);
            tLine1->fromNode=tLine2->fromNode;
            tLine1->fromNode->AddLine(tLine1);
        }

        for(int p=tps.size()-1;p>=0;p--){
            tLine1->vertices.insert(begin(tLine1->vertices),tps[p]);
        }
    }
    tLines->RemoveLine(tLine2);
    tPoints->RemovePoint(topoPoint);
    return true;
}
int TopoBuilder::GetBestRegionNum()
{
    int pointCount=tPoints->GetPointCount();
    int regionNum=8;
    if(pointCount<20)
        regionNum=2;
    else if(pointCount<50)
        regionNum=4;
    else if(pointCount<100)
        regionNum=8;
    else if(pointCount<200)
        regionNum=10;
    else if(pointCount<400)
        regionNum=20;
    else if(pointCount<800)
        regionNum=40;
    else
        regionNum=50;
    return regionNum;
}
void TopoBuilder::BuidLineTopology()
{
    int regionNum=GetBestRegionNum();
    IntersectLines(regionNum);
    BuildLineTopo();//构建线拓扑
}
void TopoBuilder::BuidLineTopologyNoIntersect() {
    BuildLineTopo();//构建线拓扑
}

bool TopoBuilder::BuildContour(TopoLine*tLine1,TopoLine*tLine2)
{
    if((tLine1->fromNode==tLine2->fromNode)||(tLine1->fromNode==tLine2->toNode)||(tLine1->toNode==tLine2->fromNode)||(tLine1->toNode==tLine2->toNode))
    {
        return false;
    }
    double xmin,ymin,xmax,ymax;
    double xmin2,ymin2,xmax2,ymax2;
    if(tLine1->fromNode->X<=tLine1->toNode->X)
    {
        xmin=tLine1->fromNode->X;
        xmax=tLine1->toNode->X;
    }
    else
    {
        xmax=tLine1->fromNode->X;
        xmin=tLine1->toNode->X;
    }
    if(tLine2->fromNode->X<=tLine2->toNode->X)
    {
        xmin2=tLine2->fromNode->X;
        xmax2=tLine2->toNode->X;
    }
    else
    {
        xmax2=tLine2->fromNode->X;
        xmin2=tLine2->toNode->X;
    }
    if((xmin2>xmax+fuzzy)||(xmax2<xmin-fuzzy)) return false;
    if(tLine1->fromNode->Y<=tLine1->toNode->Y)
    {
        ymin=tLine1->fromNode->Y;
        ymax=tLine1->toNode->Y;
    }
    else
    {
        ymax=tLine1->fromNode->Y;
        ymin=tLine1->toNode->Y;
    }
    if(tLine2->fromNode->Y<=tLine2->toNode->Y)
    {
        ymin2=tLine2->fromNode->Y;
        ymax2=tLine2->toNode->Y;
    }
    else
    {
        ymax2=tLine2->fromNode->Y;
        ymin2=tLine2->toNode->Y;
    }
    if((ymin2>ymax+fuzzy)||(ymax2<ymin-fuzzy)) return false;
    if(tLine1->fromNode->IsSamePoint(tLine2->fromNode,fuzzy))
    {
        tLine1->CombineFromPoint(tLine2->fromNode);
        return true;
    }
    if(tLine1->fromNode->IsSamePoint(tLine2->toNode,fuzzy))
    {
        tLine1->CombineFromPoint(tLine2->toNode);
        return true;
    }
    if(tLine1->toNode->IsSamePoint(tLine2->fromNode,fuzzy))
    {
        tLine1->CombineToPoint(tLine2->fromNode);
        return true;
    }
    if(tLine1->toNode->IsSamePoint(tLine2->toNode,fuzzy))
    {
        tLine1->CombineToPoint(tLine2->toNode);
        return true;
    }
    return false;
}

void TopoBuilder::BuildContour(TopoLine*tLine){
    LineRegions*regions=tLines->regions;
    int xNum=regions->GetRegionXNum();
    int yNum=regions->GetRegionYNum();
    int count=tLine->regionEls.size();
    int changedCount=0;
    for(int k=0;k<count;k++)
    {
        LineRegion*region=tLine->regionEls[k]->lineRegion;
        int regCount=region->GetCount();
        if(regCount==1) continue;
        LineRegionElement*lineEl=region->rootElement;
        while(lineEl!=nullptr)
        {
            TopoLine*nextLine=lineEl->topoLine;
            if(nextLine==tLine)
            {
                lineEl=lineEl->child;
                continue;
            }
            if(nextLine->state==1)
            {
                lineEl=lineEl->child;
                continue;
            }
            LineRegionElement*father=lineEl->father;
            if(BuildContour(tLine,nextLine)) changedCount++;
            lineEl=lineEl->child;
            if(changedCount>=2) break;
        }
    }
}
void TopoBuilder::BuildContours(CallBack*callBack){
    if(fullExtent==nullptr) return;
    int regionNum=GetBestRegionNum();
    tLines->BuildContourSpatialIndex(fullExtent.get(),regionNum);
    TopoLine*currentLine=tLines->rootLine;
    int lineCount=tLines->lineCount;
    int currentPin=0;
    while(currentLine!=nullptr)
    {
        currentPin++;
        if(currentLine->state==1)
        {
            currentLine=currentLine->child;
            continue;
        }
        currentLine->state=1;
        BuildContour(currentLine);
        currentLine=currentLine->child;
        if(currentPin%100==0)
        {
            int ipos=(FLOAT)currentPin*80.0/lineCount;
            if(ipos>80) ipos=80;
            CallBackHelper::SendProgress(callBack,ipos);
        }
    }
    currentLine=tLines->rootLine;
    lineCount=tLines->lineCount;
    currentPin=0;
    while(currentLine!=nullptr)
    {
        currentLine->state=0;
        currentPin++;
        currentLine=currentLine->child;
        if(currentPin%100==0)
        {
            int ipos=(FLOAT)currentPin*5.0/lineCount;
            if(ipos>5) ipos=5;
            CallBackHelper::SendProgress(callBack,80+ipos);
        }
    }
    TopoPoint*currentPoint=tPoints->rootPoint;
    int pointCount=tPoints->pointCount;
    currentPin=0;
    while(currentPoint!=nullptr)
    {
        currentPin++;
        TopoPoint*father=currentPoint->father;
        if(currentPoint->lines.size()==0)
        {
            tPoints->RemovePoint(currentPoint);
            if(father==nullptr)
                currentPoint=tPoints->rootPoint;
            else
                currentPoint=father->child;
        }
        else
            currentPoint=currentPoint->child;
        if(currentPin%100==0)
        {
            int ipos=(FLOAT)currentPin*15.0/pointCount;
            if(ipos>15) ipos=15;
            CallBackHelper::SendProgress(callBack,85+ipos);
        }
    }
    BuildLineTopo();
}

void TopoBuilder::PursuitRings()
{
    oRings->Clear();
    iRings->Clear();
    TopoLine*currentLine=tLines->rootLine;
    bool NeedRemove=false;
    while(currentLine!=nullptr)
    {
        if(currentLine->fromNode==currentLine->toNode)
        {
            TopoRing*pRing=new TopoRing();
            pRing->AddFirstLine(currentLine->fromNode,currentLine);
            pRing->CloseRing();
            pRing->RegisterClockwiseLines();
            oRings->AddRing(pRing);
            TopoRing*pRing2=new TopoRing();
            pRing2->CopyLines(pRing);
            iRings->AddRing(pRing2);
            if(currentLine->fromNode->lines.size()>1)
            {
                TopoPoint*topoPoint=currentLine->fromNode;
                TopoPoint*newPoint=new TopoPoint(topoPoint->X,topoPoint->Y);
                newPoint->lines.push_back(currentLine);
                currentLine->fromNode=currentLine->toNode=newPoint;
                tPoints->AddPoint(newPoint);//新建一个点，将该环孤立出来
                topoPoint->RemoveLine(currentLine);
                currentLine->state=4;
                pRing->fromPoint=newPoint;
                pRing2->fromPoint=newPoint;
                NeedRemove=true;
                //去掉环后，需要连接原来的线
                if(topoPoint->lines.size()==2) RemovePhesudo(topoPoint);
            }
            currentLine=currentLine->child;
        }
        else
            currentLine=currentLine->child;
    }
    if(NeedRemove) RemoveIsolateLine();
    for(int l=0;l<2;l++)
    {
        TopoPoint*currentPoint=tPoints->rootPoint;
        while(currentPoint!=nullptr)
        {
            int segCount=currentPoint->lines.size();
            if(l==0)
            {
                if(segCount<3)
                {
                    currentPoint=currentPoint->child;
                    continue;
                }
            }
            else
            {
                if(segCount!=2)
                {
                    currentPoint=currentPoint->child;
                    continue;
                }
            }
            for(int k=0;k<segCount;k++)
            {
                TopoLine*tLine=currentPoint->lines[k];
                if(tLine->state==4) continue;
                bool IsFrom;
                TopoLine*curLine=tLine;
                TopoPoint*fromPoint=currentPoint;
                TopoPoint*toPoint=nullptr;
                TopoRing*pRing=new TopoRing();
                pRing->AddFirstLine(fromPoint,curLine);
                while(true)
                {
                    if(curLine->fromNode==fromPoint)
                    {
                        if((curLine->state==2)||(curLine->state==4)) break;
                        IsFrom=true;
                        toPoint=curLine->toNode;
                    }
                    else
                    {
                        if((curLine->state==3)||(curLine->state==4)) break;
                        IsFrom=false;
                        toPoint=curLine->fromNode;
                    }
                    if(l==0)
                        curLine->state+=((IsFrom)?1:2);
                    else
                        curLine->state=4;
                    if(toPoint==currentPoint) break;
                    curLine=GetNextLine(toPoint,curLine);
                    if(curLine==nullptr) break;
                    fromPoint=toPoint;
                    pRing->AddLine(curLine);
                }
                if(toPoint!=currentPoint)
                {
                    delete pRing;
                    continue;
                }
                pRing->CloseRing();
                bool bClockwise=pRing->GetIsClockWise();
                if(l==0)
                {
                    if(bClockwise)
                    {
                        oRings->AddRing(pRing);
                        pRing->RegisterClockwiseLines();
                    }
                    else
                        iRings->AddRing(pRing);
                }
                else
                {
                    oRings->AddRing(pRing);
                    pRing->RegisterClockwiseLines();
                    TopoRing*newRing=new TopoRing();
                    newRing->CopyLines(pRing);
                    iRings->AddRing(newRing);
                }
            }
            currentPoint=currentPoint->child;
        }
    }
}
void TopoBuilder::BuildPolygons()
{
    oRings->BuildSpatialIndex(fullExtent.get(),GetBestRegionNum());
    TopoRing*tRing=iRings->rootRing;
    while(tRing!=nullptr)
    {
        TopoRing*ring=oRings->FindOuterRing(tRing);
        if(ring==nullptr)
        {
           tRing=tRing->child;
           continue;
        }
        TopoRing*father=tRing->father;
        iRings->DetachRing(tRing);
        ring->AddInnerRing(tRing);
        tRing->RegisterCounterClockwiseLines(ring);
        if(father==nullptr)
            tRing=iRings->rootRing;
        else
            tRing=father->child;
    }
    iRings->Clear();
}
void TopoBuilder::BuildPolygonTopology()
{
    RemoveIsolateLine();
    RemovePhesudos();
    PursuitRings();
    BuildPolygons();
}
void TopoBuilder::RemoveNoAttributeLines()
{
    TopoLine*currentLine=tLines->rootLine;
    while(currentLine!=nullptr)
    {
        if(currentLine->GetAttributesCount()==0)
        {
            TopoLine*father=currentLine->father;
            tLines->RemoveLine(currentLine);
            if(father==nullptr)
                currentLine=tLines->rootLine;
            else
                currentLine=father->child;
        }
        else
            currentLine=currentLine->child;
    }
}
void TopoBuilder::CreatePolygons(vector<AutoPtr<Polygon>>&polys)
{
    TopoRing*tRing=oRings->rootRing;
    while(tRing!=nullptr)
    {
        bool IsRingvalid=true;
        if(IsRingvalid)
        {
            AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
            polys.push_back(poly);
        }
        tRing=tRing->child;
    }
}
void TopoBuilder::CombineBuilder(TopoBuilder&tb){
    TopoLine*tLine=tLines->rootLine;
    while(tLine!=nullptr)
    {
        tLine->state=0;
        tLine=tLine->child;
    }
    TopoPoints*tpts=tb.GetPoints();
    TopoLines*tlns=tb.GetLines();
    TopoPoint*tPoint=tpts->rootPoint;
    while(tPoint!=nullptr)
    {
        tPoints->AddPoint(tPoint);
        tPoint=tPoint->child;
    }
    if(tlns->regions!=nullptr) delete tlns->regions;
    tlns->regions=nullptr;
    if(tLines->regions!=nullptr) delete tLines->regions;
    tLines->regions=nullptr;
    tLine=tlns->rootLine;
    while(tLine!=nullptr)
    {
        tLine->leftRing=tLine->rightRing=nullptr;
        tLine->state=0;
        tLines->AddLine(tLine);
        tLine=tLine->child;
    }
    tpts->rootPoint=tpts->tailPoint=nullptr;
    tlns->rootLine=tlns->tailLine=nullptr;
    tpts->pointCount=tlns->lineCount=0;
    delete tpts;
    delete tlns;
    tb.tPoints=nullptr;
    tb.tLines=nullptr;
}

void TopoBuilder::Clear(){
    tPoints->Clear();
    tLines->Clear();
    oRings->Clear();
    iRings->Clear();
    fullExtent=nullptr;
}

PolylineOper::PolylineOper(){

}
PolylineOper::~PolylineOper(){

}
AutoPtr<Polyline>PolylineOper::Build(Polyline*geo,double toler){
    AutoPtr<TopoBuilder>pBuilder=new TopoBuilder(toler);
    pBuilder->AppendGeometry(geo,1);
    pBuilder->BuidLineTopology();
    TopoLines*topoLines=pBuilder->GetLines();
    TopoLine*tLine=topoLines->rootLine;
    AutoPtr<Polyline>mpoly=new Polyline();
    while(tLine!=nullptr) {
        AutoPtr<Polyline>pl=tLine->CreatePolyline();
        for(int p=0;p<pl->GetSize();p++)
            mpoly->Add(pl->GetItem(p));
        tLine=tLine->child;
    }
    if(mpoly->GetSize()==0) return nullptr;
    return mpoly;
}

AutoPtr<Polyline>PolylineOper::Union(Polyline*poly1,Polyline*poly2,double toler){
    AutoPtr<TopoBuilder>pBuilder=new TopoBuilder(toler);
    pBuilder->AppendGeometry(poly1,1);
    pBuilder->AppendGeometry(poly2,2);
    pBuilder->BuidLineTopology();
    TopoLines*topoLines=pBuilder->GetLines();
    AutoPtr<Polyline>mPoly=new Polyline();
    TopoLine* tLine=topoLines->rootLine;
    while(tLine!=nullptr) {
        AutoPtr<Polyline>pl=tLine->CreatePolyline();
        for(int p=0;p<pl->GetSize();p++)
            mPoly->Add(pl->GetItem(p));
        tLine=tLine->child;
    }
    return mPoly;
}

PolygonSplitter::PolygonSplitter()
{

}

PolygonSplitter::~PolygonSplitter()
{

}

bool PolygonSplitter::IsRingValid(TopoRing*tRing)
{
    if(tRing->attachRing!=nullptr) return true;
    int lineCount=tRing->GetLineCount();
    TopoPoint*fp=tRing->fromPoint;
    bool IsFrom;
    bool IsClockwise=tRing->GetIsClockWise();
    tRing->attachRing=nullptr;
    bool hasOrigin=false;
    for(int k=0;k<lineCount;k++)
    {
        TopoLine*tLine=tRing->GetLine(k);
        if(lineCount==1)
            IsFrom=true;
        else
            IsFrom=(tLine->fromNode==fp);
        if(IsFrom^IsClockwise)
        {
            if(tLine->FindAttributes(1))
            {
                return false;
            }
            else if(tLine->FindAttributes(-1))
            {
                hasOrigin=true;
            }
        }
        else
        {
            if(tLine->FindAttributes(-1))
            {
                return false;
            }
            else if(tLine->FindAttributes(1))
            {
                hasOrigin=true;
            }
        }
        fp=((IsFrom)?tLine->toNode:tLine->fromNode);
    }
    if(!hasOrigin)
    {
        TopoLine*tLine=tRing->GetLine(0);
        TopoRing*otherRing=((tLine->leftRing!=tRing)?tLine->leftRing:tLine->rightRing);
        if(otherRing!=nullptr)
        {
            bool IsValid=IsRingValid(otherRing);
            if(IsValid) tRing->attachRing=tRing;
            return IsValid;
        }
        else
            return false;
    }
    tRing->attachRing=tRing;
    return true;
}

vector<AutoPtr<Polygon>>PolygonSplitter::SplitPolygon(Polygon*polygon,Points*drawLine)
{
    LONG ringCount=polygon->GetSize();
    TopoBuilder pBuilder;
    pBuilder.AppendPath(drawLine,2);
    for(int k=0;k<ringCount;k++)
    {
        AutoPtr<Ring>pRing=polygon->GetItem(k);
        Points*pts=(Points*)pRing.get();
        pBuilder.AppendRing(pts,1);
    }
    pBuilder.BuidLineTopology();
    pBuilder.BuildPolygonTopology();
    TopoRings*topoRings=pBuilder.GetRings();
    TopoRing*tRing=topoRings->rootRing;
    while(tRing!=nullptr)
    {
        tRing->attachRing=nullptr;
        tRing=tRing->child;
    }
    tRing=topoRings->rootRing;
    vector<AutoPtr<Polygon>>polys;
    while(tRing!=nullptr)
    {
        if(IsRingValid(tRing))
        {
            AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
            polys.push_back(poly);
        }
        tRing=tRing->child;
    }
    return polys;
}

vector<AutoPtr<Polygon>>PolygonSplitter::SplitMultiPolygon(MultiPolygon*mpolygon,Points*splitLine){
    vector<AutoPtr<Polygon>>polys;
    for(int k=0;k<mpolygon->GetSize();k++){
        vector<AutoPtr<Polygon>>spolys=SplitPolygon(mpolygon->GetItem(k).get(),splitLine);
        for(int j=0;j<spolys.size();j++) polys.push_back(spolys[j]);
    }
    return polys;
}
PolygonOper::PolygonOper()
{

}
PolygonOper::~PolygonOper()
{

}

AutoPtr<Geometry>PolygonOper::Build(Geometry*poly){
    if(poly->GetType()==GeometryType::gtPolygon){
        return ReparePolygon((Polygon*)poly);
    }
    if(poly->GetType()==GeometryType::gtMultiPolygon){
        return ReparePolygon((MultiPolygon*)poly);
    }
    return nullptr;
}
AutoPtr<Geometry>PolygonOper::ConvertPolygon(Geometry*geo)
{
    AutoPtr<CurvePolygon>curvePoly=AutoPtr<Geometry>(geo,true);
    if(curvePoly!=nullptr)
    {
        AutoPtr<Polygon>newpoly=curvePoly->ConvertToDefaultPolygon(true);
        return newpoly;
    }
    else
    {
        AutoPtr<Polygon>poly=AutoPtr<Geometry>(geo,true);
        if(poly!=nullptr)
        {
            return poly;
        }
         AutoPtr<MultiPolygon>mpoly=AutoPtr<Geometry>(geo,true);
        if(mpoly!=nullptr)
        {
            LONG count=mpoly->GetSize();
            for(int k=0;k<count;k++)
            {
                AutoPtr<Polygon>subpoly=mpoly->GetItem(k);
                AutoPtr<Geometry>newgeo=ConvertPolygon(subpoly.get());
                if(newgeo!=subpoly)
                {
                    mpoly->Remove(k);
                    AutoPtr<Polygon>newpoly(newgeo);
                    mpoly->Insert(k,newpoly);
                }
            }
            return geo;
        }
    }
}

void PolygonOper::RegisterPolygon(TopoBuilder*tb,Polygon*polygon,int attribute)
{
    LONG ringCount=polygon->GetSize();
    for(int k=0;k<ringCount;k++)
    {
        AutoPtr<Points>pRing=polygon->GetItem(k);
        tb->AppendRing(pRing.get(),attribute);
    }
}
void PolygonOper::RegisterPolygon(TopoBuilder*tb,MultiPolygon*mpolygon,int attribute)
{
    LONG polyCount=mpolygon->GetSize();
    for(int k=0;k<polyCount;k++)
    {
        AutoPtr<Polygon>polygon=mpolygon->GetItem(k);
        RegisterPolygon(tb,polygon.get(),attribute);
    }
}
void PolygonOper::RegisterGeometry(TopoBuilder*tb,Geometry*geo,int attribute)
{
    AutoPtr<Geometry>rgeo=ConvertPolygon(geo);
    AutoPtr<Polygon>poly(rgeo);
    if(poly!=nullptr) RegisterPolygon(tb,poly.get(),attribute);
    AutoPtr<MultiPolygon>mpoly(rgeo);
    if(mpoly!=nullptr) RegisterPolygon(tb,mpoly.get(),attribute);
}
AutoPtr<Geometry>PolygonOper::ReparePolygon(Polygon*poly)
{
    TopoBuilder pBuilder;
    LONG ringCount=poly->GetSize();
    for(int k=0;k<ringCount;k++)
    {
        AutoPtr<Points>pRing=poly->GetItem(k);
        pBuilder.AppendRing(pRing.get(),1);
    }
    pBuilder.BuidLineTopology();
    pBuilder.RemoveNoAttributeLines();
    pBuilder.BuildPolygonTopology();
    TopoRings*topoRings=pBuilder.GetRings();
    TopoRing*tRing=topoRings->rootRing;
    vector<AutoPtr<Polygon>>polys;
    AutoPtr<CurvePolygon>curvePolygon=AutoPtr<Polygon>(poly,true);
    bool HasUpdated=true;
    while(HasUpdated)
    {
        HasUpdated=false;
        TopoLines*tLines=pBuilder.GetLines();
        TopoLine*tLine=tLines->rootLine;
        while(tLine!=nullptr)
        {
            TopoRing*ring=nullptr;
            if((tLine->leftRing==nullptr)&&(tLine->rightRing!=nullptr))
                ring=tLine->rightRing;
            else if((tLine->leftRing!=nullptr)&&(tLine->rightRing==nullptr))
                ring=tLine->leftRing;
            if(ring==nullptr)
            {
                tLine=tLine->child;
                continue;
            }
            vector<TopoRing*>neighbors;
            ring->FindNeighborRings(neighbors);
            if(neighbors.size()>0)
            {
                HasUpdated=true;
                for(int k=neighbors.size()-1;k>=0;k--)
                {
                    neighbors[k]->UnregisterClockwiseLines();
                }
            }
            tLine=tLine->child;
        }
    }
    while(tRing!=nullptr)
    {
        bool IsRingvalid=false;
        int lineCount=tRing->GetLineCount();
        for(int k=0;k<lineCount;k++)
        {
            TopoLine*tLine=tRing->GetLine(k);
            if((tLine->leftRing==tRing)||(tLine->rightRing==tRing))
            {
                IsRingvalid=true;
                break;
            }
        }
        if(IsRingvalid)
        {
            AutoPtr<Polygon>poly=tRing->CreatePolygon((curvePolygon!=nullptr));
            polys.push_back(poly);
        }
        tRing=tRing->child;
    }
    if(polys.size()==0)
    {
        return nullptr;
    }
    if(polys.size()==1)
        return polys[0];
    else
    {
        MultiPolygon*mpoly=new MultiPolygon();
        for(int k=0;k<polys.size();k++)
        {
            mpoly->Add(polys[k]);
        }
        return mpoly;
    }
}
AutoPtr<Geometry>PolygonOper::ReparePolygon(MultiPolygon*mpoly)
{
    LONG partCount=mpoly->GetSize();
    AutoPtr<MultiPolygon>newmpoly=new MultiPolygon();
    int validCount=0;
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Polygon>subpoly=mpoly->GetItem(k);
        AutoPtr<Geometry>pGeo=ReparePolygon(subpoly.get());
        AutoPtr<MultiPolygon>mpoly=pGeo;
        if(mpoly!=nullptr)
        {
           LONG subCount=mpoly->GetSize();
           for(int p=0;p<subCount;p++)
           {
              AutoPtr<Polygon>np=mpoly->GetItem(p);
              newmpoly->Add(np);
           }
           validCount+=subCount;
        }
        AutoPtr<Polygon>poly=pGeo;
        if(poly!=nullptr)
        {
            newmpoly->Add(poly);
            validCount++;
        }
    }
    if(validCount==0)
    {
        return nullptr;
    }
    TopoBuilder pBuilder;
    RegisterGeometry(&pBuilder,newmpoly.get(),1);
    pBuilder.BuidLineTopology();
    pBuilder.RemoveNoAttributeLines();
    pBuilder.BuildPolygonTopology();
    TopoLines*tLines=pBuilder.GetLines();
    TopoLine*currentLine=tLines->rootLine;
    while(currentLine!=nullptr)
    {
        currentLine->state=0;
        if((currentLine->leftRing!=nullptr)&&(currentLine->rightRing!=nullptr))
        {
            bool leftValid=((currentLine->leftRing->FindAttribute(1))||(currentLine->leftRing->FindAttribute(2)));
            bool rightValid=((currentLine->rightRing->FindAttribute(1))||(currentLine->rightRing->FindAttribute(2)));
            if(leftValid&&rightValid)
            {
                currentLine->state=-1;
            }
        }
        currentLine=currentLine->child;
    }
    currentLine=tLines->rootLine;
    while(currentLine!=nullptr)
    {
        if(currentLine->state==-1)
        {
            TopoLine*father=currentLine->father;
            tLines->RemoveLine(currentLine);
            if(father==nullptr)
                currentLine=tLines->rootLine;
            else
                currentLine=father->child;
            continue;
        }
        currentLine=currentLine->child;
    }
    pBuilder.BuildPolygonTopology();
    TopoRings*topoRings=pBuilder.GetRings();
    AutoPtr<MultiPolygon>mmPoly(new MultiPolygon());
    TopoRing*tRing=topoRings->rootRing;
    vector<Polygon*>polys;
    while(tRing!=nullptr)
    {
        if(tRing->FindAttribute(1))
        {
            AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
            mmPoly->Add(poly);
        }
        tRing=tRing->child;
    }
    LONG polySize=mmPoly->GetSize();
    if(polySize==0)
    {
        return nullptr;
    }
    else if(polySize==1)
    {
        AutoPtr<Polygon>poly=mmPoly->GetItem(0);
        return poly;
    }
    return mmPoly;
}
AutoPtr<Geometry> PolygonOper::Intersect(Geometry*polygon,Geometry*otherPoly)
{
    TopoBuilder pBuilder;
    RegisterGeometry(&pBuilder,polygon,1);
    RegisterGeometry(&pBuilder,otherPoly,2);
    pBuilder.BuidLineTopology();
    pBuilder.RemoveNoAttributeLines();
    pBuilder.BuildPolygonTopology();
    TopoRings*topoRings=pBuilder.GetRings();
    AutoPtr<MultiPolygon>mPoly(new MultiPolygon());
    TopoRing*tRing=topoRings->rootRing;
    vector<Polygon*>polys;
    while(tRing!=nullptr)
    {
        bool hasTwo=tRing->FindAttribute(2);
        bool hasOne=tRing->FindAttribute(1);
        if(hasTwo&&hasOne)
        {
            AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
            mPoly->Add(poly);
        }
        else if(hasTwo)
        {
            if(!tRing->FindAttribute(-1))
            {
                if(tRing->FindNeighborAttribute(1))
                {
                    AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
                    mPoly->Add(poly);
                }
            }
        }
        else if(hasOne)
        {
            if(!tRing->FindAttribute(-2))
            {
                if(tRing->FindNeighborAttribute(2))
                {
                    AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
                    mPoly->Add(poly);
                }
            }
        }
        tRing=tRing->child;
    }
    LONG polySize=mPoly->GetSize();
    if(polySize==0)
    {
        return nullptr;
    }
    else if(polySize==1)
    {
        AutoPtr<Polygon>poly=mPoly->GetItem(0);
        return poly;
    }
    return mPoly;
}
AutoPtr<Geometry>PolygonOper::Union(Geometry*polygon,Geometry*otherPoly)
{
    TopoBuilder pBuilder;
    RegisterGeometry(&pBuilder,polygon,1);
    RegisterGeometry(&pBuilder,otherPoly,2);
    pBuilder.BuidLineTopology();
    pBuilder.RemoveNoAttributeLines();
    pBuilder.BuildPolygonTopology();
    TopoLines*tLines=pBuilder.GetLines();
    TopoLine*currentLine=tLines->rootLine;
    while(currentLine!=nullptr)
    {
        currentLine->state=0;
        if((currentLine->leftRing!=nullptr)&&(currentLine->rightRing!=nullptr))
        {
            bool leftValid=((currentLine->leftRing->FindAttribute(1))||(currentLine->leftRing->FindAttribute(2)));
            bool rightValid=((currentLine->rightRing->FindAttribute(1))||(currentLine->rightRing->FindAttribute(2)));
            if(leftValid&&rightValid) currentLine->state=-1;
        }
        currentLine=currentLine->child;
    }
    currentLine=tLines->rootLine;
    while(currentLine!=nullptr)
    {
        if(currentLine->state==-1)
        {
            TopoLine*father=currentLine->father;
            tLines->RemoveLine(currentLine);
            if(father==nullptr)
                currentLine=tLines->rootLine;
            else
                currentLine=father->child;
            continue;
        }
        currentLine=currentLine->child;
    }
    pBuilder.BuildPolygonTopology();
    TopoRings*topoRings=pBuilder.GetRings();
    AutoPtr<MultiPolygon>mPoly(new MultiPolygon());
    TopoRing*tRing=topoRings->rootRing;
    while(tRing!=nullptr)
    {
        if((tRing->FindAttribute(2))||(tRing->FindAttribute(1)))
        {
            AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
            mPoly->Add(poly);
        }
        tRing=tRing->child;
    }
    LONG polySize=mPoly->GetSize();
    if(polySize==0)
    {
        return nullptr;
    }
    else if(polySize==1)
    {
        AutoPtr<Polygon>poly=mPoly->GetItem(0);
        return poly;
    }
    return mPoly;
}
AutoPtr<Geometry>PolygonOper::Difference(Geometry*polygon,Geometry*otherPoly)
{
    TopoBuilder pBuilder;
    RegisterGeometry(&pBuilder,polygon,1);
    RegisterGeometry(&pBuilder,otherPoly,2);
    pBuilder.BuidLineTopology();
    pBuilder.RemoveNoAttributeLines();
    pBuilder.BuildPolygonTopology();
    TopoRings*topoRings=pBuilder.GetRings();
    AutoPtr<MultiPolygon>mPoly(new MultiPolygon());
    TopoRing*tRing=topoRings->rootRing;
    vector<Polygon*>polys;
    while(tRing!=nullptr)
    {
        if((!tRing->FindAttribute(2))&&(tRing->FindAttribute(1)))
        {
            bool IsValid=true;
            if(!tRing->FindAttribute(-2))
            {
                if(tRing->FindNeighborAttribute(2)) IsValid=false;
            }
            if(IsValid)
            {
                AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
                mPoly->Add(poly);
            }
        }
        tRing=tRing->child;
    }
    LONG polySize=mPoly->GetSize();
    if(polySize==0)
    {
        return nullptr;
    }
    else if(polySize==1)
    {
        AutoPtr<Polygon>poly=mPoly->GetItem(0);
        return poly;
    }
    return mPoly;
}
AutoPtr<Geometry>PolygonOper::SymmetricDifference(Geometry*polygon,Geometry*otherPoly)
{
    TopoBuilder pBuilder;
    RegisterGeometry(&pBuilder,polygon,1);
    RegisterGeometry(&pBuilder,otherPoly,2);
    pBuilder.BuidLineTopology();
    pBuilder.RemoveNoAttributeLines();
    pBuilder.BuildPolygonTopology();
    TopoRings*topoRings=pBuilder.GetRings();
    AutoPtr<MultiPolygon>mPoly(new MultiPolygon());
    TopoRing*tRing=topoRings->rootRing;
    vector<Polygon*>polys;
    while(tRing!=nullptr)
    {
        bool hasOne=tRing->FindAttribute(1);
        bool hasTwo=tRing->FindAttribute(2);
        if((!hasTwo)&&(hasOne))
        {
            bool IsValid=true;
            if(!tRing->FindAttribute(-2))
            {
                if(tRing->FindNeighborAttribute(2)) IsValid=false;
            }
            if(IsValid)
            {
                AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
                mPoly->Add(poly);
            }
        }
        else if((!hasOne)&&(hasTwo))
        {
            bool IsValid=true;
            if(!tRing->FindAttribute(-1))
            {
                if(tRing->FindNeighborAttribute(1)) IsValid=false;
            }
            if(IsValid)
            {
                AutoPtr<Polygon>poly=tRing->CreatePolygon(false);
                mPoly->Add(poly);
            }
        }
        tRing=tRing->child;
    }
    LONG polySize=mPoly->GetSize();
    if(polySize==0)
    {
        return nullptr;
    }
    else if(polySize==1)
    {
        AutoPtr<Polygon>poly=mPoly->GetItem(0);
        return poly;
    }
    return mPoly;
}


}
