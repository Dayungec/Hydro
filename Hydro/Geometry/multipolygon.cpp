#include "multipolygon.h"
#include "envelope.h"
#include "math.h"
#include "spatialreference.h"
#include "Base/classfactory.h"
#include "topobuilder.h"

namespace SGIS{

REGISTER(MultiPolygon)

MultiPolygon::MultiPolygon(){

}
MultiPolygon::~MultiPolygon(){

}
//IGeometry:
AutoPtr<Envelope>MultiPolygon::GetExtent(){
    int Size=parts.size();
    if(Size==0) return nullptr;
    AutoPtr<Envelope>pEnv;
    for(int k=0;k<Size;k++)
    {
        AutoPtr<Polygon>poly=parts[k];
        if(pEnv==nullptr)
            pEnv=poly->GetExtent();
        else
            pEnv->UnionOther(poly->GetExtent().get());
    }
    return pEnv;
}

GeometryType MultiPolygon::GetType(){
    return gtMultiPolygon;
}
AutoPtr<Geometry>MultiPolygon::Clone(){
   MultiPolygon*newpoly=new MultiPolygon();
   for(int k=0;k<parts.size();k++){
       newpoly->parts.push_back(parts[k]->Clone());
   }
   return newpoly;
}
bool MultiPolygon::Equals(Geometry*other){
    if(GetType()!=other->GetType()) return false;
    MultiPolygon*po=(MultiPolygon*)other;
    int size=parts.size();
    if(size!=po->parts.size()) return false;
    for(int k=0;k<size;k++){
        AutoPtr<Polygon>poly1(parts[k]);
        AutoPtr<Polygon>poly2(po->parts[k]);
        if(!poly1->Equals((Geometry*)poly2.get())) return false;
    }
    return true;
}

bool MultiPolygon::Intersects(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return ppt->Intersects(this);
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Intersects(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Intersects(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Intersects(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return ppts->Intersects(this);
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        return polyline->Intersects(this);
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        return polygon->Intersects(this);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount;
        partCount=mpolygon->GetSize();
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            poly=mpolygon->GetItem(k);
            IsIn=Intersects(poly.get());
            if(IsIn)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Intersects(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}
bool MultiPolygon::Disjoint(Geometry*other){
    return (!Intersects(other));
}
bool MultiPolygon::Touches(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return ppt->Touches(this);
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Touches(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Touches(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Touches(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return ppts->Touches(this);
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        return polyline->Touches(this);
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        return polygon->Touches(this);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount;
        partCount=mpolygon->GetSize();
        bool IsInter;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            poly=mpolygon->GetItem(k);
            IsInter=Touches(poly.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Touches(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}
bool MultiPolygon::Crosses(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Crosses(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Crosses(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Crosses(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        return polyline->Crosses(this);
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        return polygon->Crosses(this);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount;
        partCount=mpolygon->GetSize();
        bool IsInter;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            poly=mpolygon->GetItem(k);
            IsInter=Crosses(poly.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Crosses(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}
bool MultiPolygon::Within(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return false;
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Contains(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Contains(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        return false;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        return polygon->Contains(this);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn=pEnv->Contains(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount;
        partCount=mpolygon->GetSize();
        bool IsInter;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            poly=mpolygon->GetItem(k);
            IsInter=Within(poly.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Within(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}
bool MultiPolygon::Contains(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return ppt->Within(this);
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return pSeg->Within(this);
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Within(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Within(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return ppts->Within(this);
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        return polyline->Within(this);
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        return polygon->Within(this);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn=pEnv->Within(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount;
        partCount=mpolygon->GetSize();
        bool IsInter;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            poly=mpolygon->GetItem(k);
            IsInter=Contains(poly.get());
            if(!IsInter)
            {
                return false;
            }
        }
        return true;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Contains(pGeo.get());
            if(!IsInter)
            {
                return false;
            }
        }
        return true;
    }
    return false;
}
bool MultiPolygon::Overlaps(Geometry*other){
    AutoPtr<Point>ppt=AutoPtr<Geometry>(other,true);
    if(ppt!=nullptr)
    {
        return false;
    }
    AutoPtr<Segment>pSeg=AutoPtr<Geometry>(other,true);
    if(pSeg!=nullptr)
    {
        return false;
    }
    AutoPtr<Envelope>pEnv=AutoPtr<Geometry>(other,true);
    if(pEnv!=nullptr)
    {
        return pEnv->Overlaps(this);
    }
    AutoPtr<Ellipse>pElli=AutoPtr<Geometry>(other,true);
    if(pElli!=nullptr)
    {
        return pElli->Overlaps(this);
    }
    AutoPtr<Points>ppts=AutoPtr<Geometry>(other,true);
    if(ppts!=nullptr)
    {
        return false;
    }
    AutoPtr<Polyline>polyline=AutoPtr<Geometry>(other,true);
    if(polyline!=nullptr)
    {
        return false;
    }
    AutoPtr<Polygon>polygon=AutoPtr<Geometry>(other,true);
    if(polygon!=nullptr)
    {
        return polygon->Intersects(this);
    }
    AutoPtr<MultiPolygon>mpolygon=AutoPtr<Geometry>(other,true);
    if(mpolygon!=nullptr)
    {
        AutoPtr<Envelope>pEnv;
        pEnv=mpolygon->GetExtent();
        AutoPtr<Envelope>segEnv;
        segEnv=GetExtent();
        bool IsIn;
        IsIn=pEnv->Intersects(segEnv.get());
        if(!IsIn)
        {
            return false;
        }
        LONG partCount;
        partCount=mpolygon->GetSize();
        bool IsInter;
        for(int k=0;k<partCount;k++)
        {
            AutoPtr<Polygon>poly;
            poly=mpolygon->GetItem(k);
            IsInter=Overlaps(poly.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    AutoPtr<GeometryCollection>pCol=AutoPtr<Geometry>(other,true);
    if(pCol!=nullptr)
    {
        LONG subCount;
        subCount=pCol->GetSize();
        bool IsInter;
        for(int k=0;k<subCount;k++)
        {
            AutoPtr<Geometry>pGeo;
            pGeo=pCol->GetItem(k);
            IsInter=Overlaps(pGeo.get());
            if(IsInter)
            {
                return true;
            }
        }
        return false;
    }
    return false;
}

void MultiPolygon::AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Polygon>poly(parts[k]);
        poly->AffineTrans(AnchorPoint, RotateAngle,ZoomXRatio,ZoomYRatio,DiffX,DiffY);
    }
}
bool MultiPolygon::Project(CoordinateTransformation*pTrans){
    for(int k=parts.size()-1;k>=0;k--)
    {
        AutoPtr<Polygon>poly(parts[k]);
        if(!poly->Project(pTrans)) return false;
    }
    return true;
}
void MultiPolygon::Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Polygon>poly(parts[k]);
        poly->Scale(xr,yr,zr);
    }
}
void MultiPolygon::Move(DOUBLE difx,DOUBLE dify,DOUBLE difz){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Polygon>poly(parts[k]);
        poly->Move(difx,dify,difz);
    }
}

double MultiPolygon::GetDiameter(){
    if(parts.size()==0) return 0;
    AutoPtr<Points>pts=parts[0];
    double x1,y1,x2,y2;
    double diam=0;
    bool findFirst=false;
    for(int k=0;k<parts.size();k++){
        AutoPtr<Polygon>poly=parts[k];
        int partSize=poly->GetSize();
        for(int p=0;p<partSize;p++){
            AutoPtr<Points>pts=poly->GetItem(p);
            if(pts->GetSize()>0){
                pts->GetItem(0,x1,y1);
                x2=x1;y2=y1;
                findFirst=true;
                break;
            }
        }
    }
    if(!findFirst) return 0;
    double tx=x2+1,ty=y2+1;
    while(true){
        for(int k=0;k<parts.size();k++){
            AutoPtr<Polygon>poly=parts[k];
            int partSize=poly->GetSize();
            for(int p=0;p<partSize;p++){
                AutoPtr<Points>pts=poly->GetItem(p);
                DOUBLE*XYs=pts->GetXYs();
                int ptCount=pts->GetSize();
                int nIndex=0;
                for(int j=0;j<ptCount;j++){
                    double x=XYs[nIndex];
                    double y=XYs[nIndex+1];
                    double dist=sqrt((x1-x)*(x1-x)+(y1-y)*(y1-y));
                    if(dist>diam){
                        tx=x;
                        ty=y;
                        diam=dist;
                    }
                    nIndex+=2;
                }
            }
        }
        if((tx==x2)&&(ty==y2)){
            break;
        }
        x2=x1;
        y2=y1;
        x1=tx;
        y1=ty;
    }
    return diam;
}

AutoPtr<Point>MultiPolygon::GetCenter(){
    int partSize=parts.size();
    if(partSize==0) return nullptr;
    double sx=0,sy=0;
    int nCount=0;
    for(int k=0;k<parts.size();k++){
        AutoPtr<Polygon>poly=parts[k];
        int partSize=poly->GetSize();
        for(int p=0;p<partSize;p++){
            AutoPtr<Points>pts=poly->GetItem(p);
            DOUBLE*XYs=pts->GetXYs();
            int ptCount=pts->GetSize();
            int nIndex=0;
            for(int j=0;j<ptCount;j++){
                sx+=XYs[nIndex];
                sy+=XYs[nIndex+1];
                nIndex++;
            }
            nCount+=ptCount;
        }
    }
    if(nCount>0){
        sx/=nCount;
        sy/=nCount;
    }
    return new Point(sx,sy);
}

//BufferWritable:
bool MultiPolygon::Read(BufferReader*br){
    int size=br->Read<int>();
    parts.clear();
    for(int k=0;k<size;k++){
        Polygon*poly=new Polygon();
        if(!poly->Read(br)){
            delete poly;
            return false;
        }
        parts.push_back(AutoPtr<Polygon>(poly));
    }
    return true;
}
void MultiPolygon::Write(BufferWriter*bw){
    int size=parts.size();
    bw->Write<int>(size);
    for(int k=0;k<size;k++){
        AutoPtr<Polygon>poly(parts[k]);
        poly->Write(bw);
    }
}
string MultiPolygon::GetTypeName(){
    return "MultiPolygon";
}
//JsonWritable:
bool MultiPolygon::ReadFromJson(JsonReader*jr,JsonNode node){
    parts.clear();
    for(int k=0;k<node.size();k++){
        AutoPtr<Polygon>poly(new Polygon());
        if(!poly->ReadFromJson(jr,node[k])) return false;
        parts.push_back(poly);
    }
    return true;
}
string MultiPolygon::ToJson(){
    JsonArrayWriter writer;
    for(int k=0;k<parts.size();k++){
        AutoPtr<JsonWritable>pts(parts[k]);
        writer.AddWritable(pts.get());
    }
    return writer.GetJsonString();
}
//Geometry3D:
AutoPtr<Envelope>MultiPolygon::GetExtent3D(){
    int Size=parts.size();
    if(Size==0) return nullptr;
    AutoPtr<Envelope>pEnv;
    for(int k=0;k<Size;k++)
    {
        AutoPtr<Polygon>poly=parts[k];
        if(pEnv==nullptr)
            pEnv=poly->GetExtent3D();
        else
            pEnv->UnionOther(poly->GetExtent3D().get());
    }
    return pEnv;
}
void MultiPolygon::Rotate(DOUBLE RotateAngle,Vector*axis){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Polygon>poly=parts[k];
        poly->Rotate(RotateAngle,axis);
    }
}
bool MultiPolygon::Read3D(BufferReader*br){
    int size=br->Read<int>();
    parts.clear();
    for(int k=0;k<size;k++){
        Polygon*poly=new Polygon();
        if(!poly->Read3D(br)){
            delete poly;
            return false;
        }
        parts.push_back(AutoPtr<Polygon>(poly));
    }
    return true;
}
void MultiPolygon::Write3D(BufferWriter*bw){
    int size=parts.size();
    bw->Write<int>(size);
    for(int k=0;k<size;k++){
        AutoPtr<Polygon>poly=parts[k];
        poly->Write3D(bw);
    }
}
bool MultiPolygon::ReadFromJson3D(JsonReader*jr,JsonNode node){
    parts.clear();
    for(int k=0;k<node.size();k++){
        AutoPtr<Polygon>poly(new Polygon());
        if(!poly->ReadFromJson3D(jr,node[k])) return false;
        parts.push_back(poly);
    }
    return true;
}
string MultiPolygon::ToJson3D(){
    string str="[";
    for(int k=0;k<parts.size();k++){
        AutoPtr<Polygon>points=parts[k];
        str+=points->ToJson3D();
        if(k<parts.size()-1) str+=',';
    }
    str+="]";
    return str;
}

int MultiPolygon::GetSize(){
    return parts.size();
}
AutoPtr<Polygon>MultiPolygon::operator[](int nIndex){
    return parts[nIndex];
}
AutoPtr<Polygon>MultiPolygon::GetItem(int nIndex){
    return parts[nIndex];
}

void MultiPolygon::SetItem(int nIndex,AutoPtr<Polygon>poly){
    parts[nIndex]=poly;
}

void MultiPolygon::Add(AutoPtr<Polygon>item){
    parts.push_back(item);
}

void MultiPolygon::Insert(int nIndex,AutoPtr<Polygon>item){
    parts.insert(std::begin(parts)+nIndex,item);
}

void MultiPolygon::Clear(){
    parts.clear();
}

void MultiPolygon::Remove(int nIndex){
    parts.erase(begin(parts)+nIndex);
}

bool MultiPolygon::IsPointIn(Point*point){
    for(int k=0;k<parts.size();k++){
        if(parts[k]->IsPointIn(point)) return true;
    }
    return false;
}
double MultiPolygon::GetPerimeter(){
    double per=0;
    for(int k=parts.size()-1;k>=0;k--)
    {
        per+=parts[k]->GetPerimeter();
    }
    return per;
}

double MultiPolygon::GetArea(){
    double ara=0;
    for(int k=parts.size()-1;k>=0;k--)
    {
        ara+=parts[k]->GetArea();
    }
    return ara;
}

bool MultiPolygon::CanCaptureBorder(Point*ppt,double toler){
    LONG partCount=parts.size();
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Polygon>poly=parts[k];
        if(poly->CanCaptureBorder(ppt,toler)) return true;
    }
    return false;
}

AutoPtr<Point>MultiPolygon::CaptureBorder(Point*ppt,double toler){
    LONG partCount=parts.size();
    DOUBLE nearX,nearY;
    DOUBLE minDist=-1;
    DOUBLE dx,dy;
    DOUBLE dist;
    DOUBLE X,Y;
    ppt->GetCoord(&X,&Y);
    for(int k=0;k<partCount;k++)
    {
        AutoPtr<Polygon>poly=parts[k];
        AutoPtr<Point>droppt=poly->CaptureBorder(ppt,toler);
        if(droppt!=nullptr)
        {
            droppt->GetCoord(&dx,&dy);
            dist=sqrt((X-dx)*(X-dx)+(Y-dy)*(Y-dy));
            if(minDist==-1)
            {
                minDist=dist;
                nearX=dx;
                nearY=dy;
            }
            else if(dist<minDist)
            {
                minDist=dist;
                nearX=dx;
                nearY=dy;
            }
        }
    }
    if(minDist==-1) return nullptr;
    AutoPtr<Point>tpt(new Point());
    tpt->PutCoord(nearX,nearY);
    return tpt;
}

AutoPtr<GeometryCollection>MultiPolygon::Split(Path*pPath){
    vector<AutoPtr<Polygon>>polys;
    LONG count=GetSize();
    vector<bool>bSplitteds;
    bSplitteds.resize(count);
    int sucCount=0;
    for(int k=0;k<count;k++)
    {
        bSplitteds[k]=false;
        AutoPtr<Polygon>npolygon(GetItem(k));
        AutoPtr<GeometryCollection>pGeoCol=npolygon->Split(pPath);
        LONG scount=pGeoCol->GetSize();
        if(scount<=1) continue;
        for(int p=0;p<scount;p++)
        {
            AutoPtr<Geometry>geo=pGeoCol->GetItem(p);
            AutoPtr<Polygon>subpoly(geo);
            polys.push_back(subpoly);
        }
        bSplitteds[k]=true;
        sucCount++;
    }
    if(polys.size()<=0) return nullptr;
    AutoPtr<GeometryCollection>gCol(new GeometryCollection());
    if(sucCount<count)
    {
        AutoPtr<MultiPolygon>multiPoly(new MultiPolygon());
        for(int k=0;k<count;k++)
        {
            if(!bSplitteds[k])
            {
                AutoPtr<Polygon>subPoly=GetItem(k);
                multiPoly->Add(subPoly);
            }
        }
        multiPoly->Add(polys[0]);
        gCol->Add(multiPoly);
    }
    else
        gCol->Add(polys[0]);
    for(int k=1;k<polys.size();k++) gCol->Add(polys[k]);
    return gCol;
}
AutoPtr<Geometry>MultiPolygon::Rebuild(){
    return PolygonOper::ReparePolygon(this);
}

}

