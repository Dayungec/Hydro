#include "geometrycollection.h"
#include "envelope.h"
#include "segment.h"
#include "ellipse.h"
#include "polyline.h"
#include "polygon.h"
#include "multipolygon.h"
#include "math.h"
#include "spatialreference.h"
#include "Base/jsonio.h"
#include "Base/classfactory.h"

namespace SGIS{

REGISTER(GeometryCollection)

GeometryCollection::GeometryCollection(){

}
GeometryCollection::~GeometryCollection(){

}
//IGeometry:
AutoPtr<Envelope>GeometryCollection::GetExtent(){
    int Size=parts.size();
    if(Size==0) return nullptr;
    AutoPtr<Envelope>pEnv;
    for(int k=0;k<Size;k++)
    {
        AutoPtr<Geometry>poly=parts[k];
        if(pEnv==nullptr)
            pEnv=poly->GetExtent();
        else
            pEnv->UnionOther(poly->GetExtent().get());
    }
    return pEnv;
}

GeometryType GeometryCollection::GetType(){
    return gtCollection;
}
AutoPtr<Geometry>GeometryCollection::Clone(){
   GeometryCollection*newpoly=new GeometryCollection();
   for(int k=0;k<parts.size();k++){
       AutoPtr<Geometry>poly1(parts[k]);
       AutoPtr<Geometry>poly2(poly1->Clone());
       newpoly->parts.push_back(poly2);
   }
   return newpoly;
}
bool GeometryCollection::Equals(Geometry*other){
    if(GetType()!=other->GetType()) return false;
    GeometryCollection*po=(GeometryCollection*)other;
    int size=parts.size();
    if(size!=po->parts.size()) return false;
    for(int k=0;k<size;k++){
        AutoPtr<Geometry>poly1(parts[k]);
        AutoPtr<Geometry>poly2(po->parts[k]);
        if(!poly1->Equals(poly2.get())) return false;
    }
    return true;
}

bool GeometryCollection::Intersects(Geometry*other){
    LONG Count=parts.size();
    bool bs;
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Geometry>geo=parts[k];
        bs=geo->Intersects(other);
        if(bs)
        {
            return true;
        }
    }
    return false;
}
bool GeometryCollection::Disjoint(Geometry*other){
    return (!Intersects(other));
}
bool GeometryCollection::Touches(Geometry*other){
    LONG Count=parts.size();
    bool bs;
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Geometry>geo=parts[k];
        bs=geo->Touches(other);
        if(bs)
        {
            return true;
        }
    }
    return false;
}
bool GeometryCollection::Crosses(Geometry*other){
    LONG Count=parts.size();
    bool bs;
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Geometry>geo=parts[k];
        bs=geo->Crosses(other);
        if(bs)
        {
            return true;
        }
    }
    return false;
}
bool GeometryCollection::Within(Geometry*other){
    LONG Count=parts.size();
    bool bs;
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Geometry>geo=parts[k];
        bs=geo->Within(other);
        if(!bs)
        {
            return false;
        }
    }
    return true;
}
bool GeometryCollection::Contains(Geometry*other){
    LONG Count=parts.size();
    bool bs;
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Geometry>geo=parts[k];
        bs=geo->Contains(other);
        if(bs)
        {
            return true;
        }
    }
    return false;
}
bool GeometryCollection::Overlaps(Geometry*other){
    LONG Count=parts.size();
    bool bs;
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Geometry>geo=parts[k];
        bs=geo->Overlaps(other);
        if(bs)
        {
            return true;
        }
    }
    return false;
}

void GeometryCollection::AffineTrans(Point*AnchorPoint,DOUBLE RotateAngle,DOUBLE ZoomXRatio,DOUBLE ZoomYRatio,DOUBLE DiffX,DOUBLE DiffY){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Geometry>poly=parts[k];
        poly->AffineTrans(AnchorPoint, RotateAngle,ZoomXRatio,ZoomYRatio,DiffX,DiffY);
    }
}
bool GeometryCollection::Project(CoordinateTransformation*pTrans){
    for(int k=parts.size()-1;k>=0;k--)
    {
        AutoPtr<Geometry>poly=parts[k];
        if(!poly->Project(pTrans)) return false;
    }
    return true;
}
void GeometryCollection::Scale(DOUBLE xr,DOUBLE yr,DOUBLE zr){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Geometry>poly(parts[k]);
        poly->Scale(xr,yr,zr);
    }
}
void GeometryCollection::Move(DOUBLE difx,DOUBLE dify,DOUBLE difz){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Geometry>poly(parts[k]);
        poly->Move(difx,dify,difz);
    }
}
//BufferWritable:
bool GeometryCollection::Read(BufferReader*br){
    int size=br->Read<int>();
    parts.clear();
    for(int k=0;k<size;k++){
        AutoPtr<BufferWritable>bwta=br->ReadObject(false);
        AutoPtr<Geometry>geo(bwta);
        parts.push_back(geo);
    }
    return true;
}
void GeometryCollection::Write(BufferWriter*bw){
    int size=parts.size();
    bw->Write<int>(size);
    for(int k=0;k<size;k++){
        bw->WriteObject(parts[k].get(),false);
    }
}
string GeometryCollection::GetTypeName(){
    return "GeometryCollection";
}
//JsonWritable:
bool GeometryCollection::ReadFromJson(JsonReader*jr,JsonNode node){

    parts.clear();
    for(int k=0;k<node.size();k++){
        JsonNode childNode=node[k];
        string type=childNode["type"].asString();
        JsonNode coordNode=childNode["coordinates"];
        if(type=="Point"){
            AutoPtr<Point>geo(new Point());
            geo->ReadFromJson(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="Envelope"){
            AutoPtr<Envelope>geo(new Envelope());
            geo->ReadFromJson(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="Ellipse"){
            AutoPtr<Ellipse>geo(new Ellipse());
            geo->ReadFromJson(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="Segment"){
            AutoPtr<Segment>geo(new Segment());
            geo->ReadFromJson(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="LineString"){
            AutoPtr<Polyline>geo(new Polyline());
            AutoPtr<Points>pts(new Points());
            pts->ReadFromJson(jr,coordNode);
            geo->Add(pts);
            parts.push_back(geo);
        }
        else if(type=="MultiLineString"){
            AutoPtr<Polyline>geo(new Polyline());
            geo->ReadFromJson(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="Polygon"){
            AutoPtr<Polygon>geo(new Polygon());
            geo->ReadFromJson(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="MultiPolygon"){
            AutoPtr<MultiPolygon>geo(new MultiPolygon());
            geo->ReadFromJson(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="Collection"){
            AutoPtr<GeometryCollection>geo(new GeometryCollection());
            geo->ReadFromJson(jr,coordNode);
            parts.push_back(geo);
        }
    }

    return true;
}

string GeometryCollection::ToJson(){
    string str="[";
    for(int k=0;k<parts.size();k++){
        string subjson="{type:\"";
        GeometryType gType=parts[k]->GetType();
        AutoPtr<Geometry>poly=parts[k];
        string sType="";
        string geoJson="";
        switch(gType){
        case gtPoint:
            sType="Point";
            geoJson=poly->ToJson();
            break;
        case gtEnvelope:
            sType="Envelope";
             geoJson=poly->ToJson();
            break;
        case gtEllipse:
            sType="Ellipse";
             geoJson=poly->ToJson();
            break;
        case gtSegment:
            sType="Segment";
            geoJson=poly->ToJson();
            break;
        case gtPolyline:
        {
            AutoPtr<Polyline>ply(parts[k]);
            if(ply->GetSize()>1){
                sType="MultiLineString";
                geoJson=poly->ToJson();
            }
            else if(ply->GetSize()==1){
                sType="LineString";
                AutoPtr<Points>points(ply->GetItem(0));
                geoJson=points->ToJson();
            }
            else{
                sType="LineString";
                geoJson="[]";
            }
            break;
        }
        case gtPolygon:
            sType="Polygon";
            geoJson=poly->ToJson();
            break;
        case gtMultiPolygon:
            sType="MultiPolygon";
            geoJson=poly->ToJson();
            break;
        case gtCollection:
            sType="Collection";
             geoJson=poly->ToJson();
            break;
        }
        subjson+=sType+"\",coordinates:"+geoJson+"}";
        str+=subjson;
        if(k<parts.size()-1) str+=',';
    }
    str+="]";
    return str;
}
//Geometry3D:
AutoPtr<Envelope>GeometryCollection::GetExtent3D(){
    int Size=parts.size();
    if(Size==0) return nullptr;
    AutoPtr<Envelope>pEnv;
    for(int k=0;k<Size;k++)
    {
        AutoPtr<Geometry3D>poly(parts[k]);
        if(pEnv==nullptr)
            pEnv=poly->GetExtent3D();
        else
            pEnv->UnionOther(poly->GetExtent3D().get());
    }
    return pEnv;
}
void GeometryCollection::Rotate(DOUBLE RotateAngle,Vector*axis){
    for(int k=parts.size()-1;k>=0;k--){
        AutoPtr<Geometry3D>poly(parts[k]);
        poly->Rotate(RotateAngle,axis);
    }
}

bool GeometryCollection::Read3D(BufferReader*br){
    int size=br->Read<int>();
    parts.clear();
    for(int k=0;k<size;k++){
        BYTE type=br->Read<BYTE>();
        GeometryType gType=(GeometryType)type;
        switch(gType){
        case gtPoint:{
            AutoPtr<Point>geo(new Point());
            if(!geo->Read3D(br)) return false;
            parts.push_back(geo);
            break;
        }
        case gtEnvelope:{
            AutoPtr<Envelope>geo(new Envelope());
            if(!geo->Read3D(br)) return false;
            parts.push_back(geo);
            break;
        }
        case gtEllipse:{
            AutoPtr<Ellipse>geo(new Ellipse());
            if(!geo->Read(br)) return false;
            parts.push_back(geo);
            break;
        }
        case gtPoints:{
            AutoPtr<Points>geo(new Points());
            if(!geo->Read3D(br)) return false;
            parts.push_back(geo);
            break;
        }
        case gtSegment:{
            AutoPtr<Segment>geo(new Segment());
            if(!geo->Read(br)) return false;
            parts.push_back(geo);
            break;
        }
        case gtPolyline:{
            AutoPtr<Polyline>geo(new Polyline());
            if(!geo->Read(br)) return false;
            parts.push_back(geo);
            break;
        }
        case gtPolygon:{
            AutoPtr<Polygon>geo(new Polygon());
            if(!geo->Read(br)) return false;
            parts.push_back(geo);
            break;
        }
        case gtMultiPolygon:{
            AutoPtr<MultiPolygon>geo(new MultiPolygon());
            if(!geo->Read(br)) return false;
            parts.push_back(geo);
            break;
        }
        case gtCollection:{
            AutoPtr<GeometryCollection>geo(new GeometryCollection());
            if(!geo->Read(br)) return false;
            parts.push_back(geo);
            break;
        }
        }
    }
    return true;
}

void GeometryCollection::Write3D(BufferWriter*bw){
    int size=parts.size();
    bw->Write<int>(size);
    for(int k=0;k<size;k++){
        AutoPtr<Geometry>poly=parts[k];
        BYTE gType=(BYTE)poly->GetType();
        bw->Write<BYTE>(gType);
        AutoPtr<Geometry3D>poly3(parts[k]);
        poly3->Write3D(bw);
    }
}

bool GeometryCollection::ReadFromJson3D(JsonReader*jr,JsonNode node){
    parts.clear();
    for(int k=0;k<node.size();k++){
        JsonNode childNode=node[k];
        string type=childNode["type"].asString();
        JsonNode coordNode=childNode["coordinates"];
        if(type=="Point"){
            AutoPtr<Point>geo(new Point());
            geo->ReadFromJson3D(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="Envelope"){
            AutoPtr<Envelope>geo(new Envelope());
            geo->ReadFromJson3D(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="Ellipse"){
            AutoPtr<Ellipse>geo(new Ellipse());
            geo->ReadFromJson(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="Segment"){
            AutoPtr<Segment>geo(new Segment());
            geo->ReadFromJson3D(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="LineString"){
            AutoPtr<Polyline>geo(new Polyline());
            AutoPtr<Points>pts(new Points());
            pts->ReadFromJson3D(jr,coordNode);
            geo->Add(pts);
            parts.push_back(geo);
        }
        else if(type=="MultiLineString"){
            AutoPtr<Polyline>geo(new Polyline());
            geo->ReadFromJson3D(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="Polygon"){
            AutoPtr<Polygon>geo(new Polygon());
            geo->ReadFromJson3D(jr,coordNode);
            parts.push_back(geo);
        }
        else if(type=="MultiPolygon"){
           AutoPtr<MultiPolygon>geo(new MultiPolygon());
           geo->ReadFromJson3D(jr,coordNode);
           parts.push_back(geo);
        }
        else if(type=="Collection"){
            AutoPtr<GeometryCollection>geo(new GeometryCollection());
            geo->ReadFromJson3D(jr,coordNode);
            parts.push_back(geo);
        }
    }
    return true;
}
string GeometryCollection::ToJson3D(){
    string str="[";
    for(int k=0;k<parts.size();k++){
        string subjson="{type:\"";
        GeometryType gType=parts[k]->GetType();
        AutoPtr<Geometry3D>poly=nullptr;
        if(gType!=gtEllipse) poly=AutoPtr<Geometry3D>(parts[k]);
        string sType="";
        string geoJson="";
        switch(gType){
        case gtPoint:
            sType="Point";
            geoJson=poly->ToJson3D();
            break;
        case gtEnvelope:
            sType="Envelope";
             geoJson=poly->ToJson3D();
            break;
        case gtEllipse:
            sType="Ellipse";
             geoJson=parts[k]->ToJson();
            break;
        case gtSegment:
            sType="Segment";
            geoJson=poly->ToJson3D();
            break;
        case gtPolyline:
        {
            AutoPtr<Polyline>ply(parts[k]);
            if(ply->GetSize()>1){
                sType="MultiLineString";
                geoJson=poly->ToJson3D();
            }
            else if(ply->GetSize()==1){
                sType="LineString";
                AutoPtr<Points>points(ply->GetItem(0));
                geoJson=points->ToJson3D();
            }
            else{
                sType="LineString";
                geoJson="[]";
            }
            break;
        }
        case gtPolygon:
            sType="Polygon";
            geoJson=poly->ToJson3D();
            break;
        case gtMultiPolygon:
            sType="MultiPolygon";
            geoJson=poly->ToJson3D();
            break;
        case gtCollection:
            sType="Collection";
             geoJson=poly->ToJson3D();
            break;
        }
        subjson+=sType+"\",coordinates:"+geoJson+"}";
        str+=subjson;
        if(k<parts.size()-1) str+=',';
    }
    str+="]";
    return str;
}

int GeometryCollection::GetSize(){
    return parts.size();
}
AutoPtr<Geometry>GeometryCollection::operator[](int nIndex){
    return parts[nIndex];
}
AutoPtr<Geometry>GeometryCollection::GetItem(int nIndex){
    return parts[nIndex];
}

void GeometryCollection::Add(AutoPtr<Geometry>item){
    parts.push_back(item);
}

void GeometryCollection::Insert(int nIndex,AutoPtr<Geometry>item){
    parts.insert(std::begin(parts)+nIndex,item);
}

void GeometryCollection::Clear(){
    parts.clear();
}

void GeometryCollection::Remove(int nIndex){
    parts.erase(begin(parts)+nIndex);
}

}
