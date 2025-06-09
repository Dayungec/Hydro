#include "vectorprovider.h"
#include "Base/xmldoc.h"
#include "Base/FilePath.h"
namespace SGIS{

REGISTER(MercatorVectorProvider)
REGISTER(CGISMercatorVectorProvider)

VectorProvider::VectorProvider()
{
    minLevel=0;
    maxLevel=19;
    tileFormat=vtfJson;
    srand((int)time(0));
}
VectorProvider::~VectorProvider()
{

}

int&VectorProvider::MinLevel(){
    return minLevel;
}
int&VectorProvider::MaxLevel(){
    return maxLevel;
}

string&VectorProvider::Name(){
    return name;
}

VectorTileFormat VectorProvider::GetVectorTileFormat(){
    return tileFormat;
}

int VectorProvider::GetServerNum(int max){
    if(max==0) return 0;
    return rand()%max;
}


MercatorVectorProvider::MercatorVectorProvider(VectorTileFormat tileFormat){
    name="MercatorVector";
    mp=new MercatorProjection();
    boundingBox=new Envelope(-180,90,180,-90);
    serverNumber=0;
    globalMaxLevel=0;
    this->tileFormat=tileFormat;
}

MercatorVectorProvider::~MercatorVectorProvider(){

}
LONG&MercatorVectorProvider::ServerNumber(){
    return serverNumber;
}

string&MercatorVectorProvider::ServerIp(){
    return serverIp;
}

LONG&MercatorVectorProvider::GlobalMaxLevel(){
    return globalMaxLevel;
}
AutoPtr<Envelope>MercatorVectorProvider::GetBoundingBox(){
    return boundingBox->Clone();
}

void MercatorVectorProvider::SetBoundingBox(Envelope*env){
    boundingBox->CopyFromOther(env);
}

AutoPtr<MapProjection>MercatorVectorProvider::GetMapProjection(){
    return mp;
}

string MercatorVectorProvider::GetVectorUrl(MapTile tile){
    int iX=tile.TileX,iY=tile.TileY;
    bool bValid=true;
    string url="";
    if((tile.Level>globalMaxLevel)&&(globalMaxLevel>0))
    {
        double px,py;
        mp->FromTileXYToPixel(iX,iY,&px,&py);
        double mx1,my1,mx2,my2;
        mp->FromPixelToLatLng(px,py,tile.Level,&mx1,&my1);
        mp->FromTileXYToPixel(iX+1,iY+1,&px,&py);
        mp->FromPixelToLatLng(px,py,tile.Level,&mx2,&my2);
        double l,t,r,b;
        boundingBox->GetCoord(&l,&t,&r,&b);
        if ((l > mx2) || (r < mx1) || (t < my2) || (b > my1)) bValid=false;
    }
    if(!bValid) return "";
    int serverNum=GetServerNum(serverNumber);
    url=serverIp;
    url=StringHelper::Replace(url,"{s}",to_string(serverNum));
    url=StringHelper::Replace(url,"{x}",to_string(tile.TileX));
    url=StringHelper::Replace(url,"{y}",to_string(tile.TileY));
    url=StringHelper::Replace(url,"{z}",to_string(tile.Level));
    return url;
}

AutoPtr<VectorProvider>MercatorVectorProvider::Clone(){
    MercatorVectorProvider*pro=new MercatorVectorProvider();
    pro->boundingBox=boundingBox->Clone();
    pro->globalMaxLevel=globalMaxLevel;
    pro->serverNumber=serverNumber;
    pro->serverIp=serverIp;
    return pro;
}

bool MercatorVectorProvider::Read(BufferReader*br){
    boundingBox->Read(br);
    globalMaxLevel=br->Read<LONG>();
    serverNumber=br->Read<LONG>();
    serverIp=br->ReadString();
    return true;
}
void MercatorVectorProvider::Write(BufferWriter*bw){
    boundingBox->Write(bw);
    bw->Write<LONG>(globalMaxLevel);
    bw->Write<LONG>(serverNumber);
    bw->WriteString(serverIp);
}

bool MercatorVectorProvider::ReadFromJson(JsonReader*jr,JsonNode node){
    boundingBox->ReadFromJson(jr,node["boundingBox"]);
    globalMaxLevel=node["globalMaxLevel"].asInt();
    serverNumber=node["serverNumber"].asInt();
    serverIp=node["serverIp"].asInt();
    return true;
}

string MercatorVectorProvider::ToJson(){
    JsonObjectWriter jow;
    jow.AddWritable("boundingBox",boundingBox.get());
    jow.Add<int>("globalMaxLevel",globalMaxLevel);
    jow.Add<int>("serverNumber",serverNumber);
    jow.Add<string>("serverIp",serverIp);
    return jow.GetJsonString();
}

string MercatorVectorProvider::GetTypeName(){
    return "MercatorVectorProvider";
}

CGISMercatorVectorProvider::CGISMercatorVectorProvider(){
    name="CGISMercatorVector";
    mp=new MercatorProjection();
    boundingBox=new Envelope(-180,90,180,-90);
    globalMaxLevel=0;
    tileUrl="";
}

CGISMercatorVectorProvider::~CGISMercatorVectorProvider(){

}

void CGISMercatorVectorProvider::AddServerIp(string serverIp){
    serverIps.push_back(serverIp);
}
int CGISMercatorVectorProvider::GetServerIpCount(){
    return serverIps.size();
}
string CGISMercatorVectorProvider::GetServerIp(int nIndex){
    return serverIps[nIndex];
}

string&CGISMercatorVectorProvider::TileUrl(){
    return tileUrl;
}

LONG&CGISMercatorVectorProvider::GlobalMaxLevel(){
    return globalMaxLevel;
}

AutoPtr<Envelope>CGISMercatorVectorProvider::GetBoundingBox(){
    return boundingBox->Clone();
}

void CGISMercatorVectorProvider::SetBoundingBox(Envelope*env){
    boundingBox->CopyFromOther(env);
}

AutoPtr<MapProjection>CGISMercatorVectorProvider::GetMapProjection(){
    return mp;
}

string CGISMercatorVectorProvider::GetVectorUrl(MapTile tile){
    if(serverIps.size()==0) return "";
    int iX=tile.TileX,iY=tile.TileY;
    string url="";
    bool bValid=true;
    if((tile.Level>globalMaxLevel)&&(globalMaxLevel>0)){
        double px,py;
        mp->FromTileXYToPixel(iX,iY,&px,&py);
        double mx1,my1,mx2,my2;
        mp->FromPixelToLatLng(px,py,tile.Level,&mx1,&my1);
        mp->FromTileXYToPixel(iX+1,iY+1,&px,&py);
        mp->FromPixelToLatLng(px,py,tile.Level,&mx2,&my2);
        double l,t,r,b;
        boundingBox->GetCoord(&l,&t,&r,&b);
        if ((l > mx2) || (r < mx1) || (t < my2) || (b > my1)) bValid=false;
    }
    if(!bValid) return "";
    int seId=rand()%serverIps.size();
    string serverIp=serverIps[seId];
    if(tileUrl!="")
    {
        url=serverIp+"/"+tileUrl;
        url=StringHelper::Replace(url,"{x}",to_string(tile.TileX));
        url=StringHelper::Replace(url,"{y}",to_string(tile.TileY));
        url=StringHelper::Replace(url,"{z}",to_string(tile.Level));
        return url;
    }
    string exa="json";
    if(tileFormat==VectorTileFormat::vtfStream) exa="vt";
    return serverIp+"/"+to_string(tile.Level)+"/"+to_string(tile.TileY)+"_"+to_string(tile.TileX)+"."+exa;

}

bool CGISMercatorVectorProvider::LoadFromXml(string xmlPath){
    AutoPtr<XmlDoc>xmlDoc(new XmlDoc());
    if(!xmlDoc->Load(xmlPath)) return false;
    AutoPtr<XmlNode>rootNode=xmlDoc->GetRootNode();
    if(rootNode==nullptr) return false;
    if(rootNode->GetName()!="MapTiles") return false;
    AutoPtr<XmlNode>boundNode=rootNode->GetFirstChildNode("BoundingBox");
    if(boundNode==nullptr) return false;
    string sBound=boundNode->GetData();
    vector<string>sBounds=StringHelper::Split(sBound,";");
    if(sBounds.size()!=2) return false;
    vector<string>lts=StringHelper::Split(sBounds[0],",");
    vector<string>rbs=StringHelper::Split(sBounds[1],",");
    if((lts.size()!=2)||(rbs.size()!=2)) return false;
    this->boundingBox->PutCoord(atof(lts[0].c_str()),atof(lts[1].c_str()),atof(rbs[0].c_str()),atof(rbs[1].c_str()));
    AutoPtr<XmlNode>maxLevelNode=rootNode->GetFirstChildNode("MaxLevel");
    if(maxLevelNode!=nullptr){
        maxLevel=atoi(maxLevelNode->GetData().c_str());
    }
    AutoPtr<XmlNode>maxPerNode=rootNode->GetFirstChildNode("MaxPerLevel");
    if(maxPerNode!=nullptr){
        int maxPer=atoi(maxPerNode->GetData().c_str());
        if(maxPer!=0) return false;
    }
    AutoPtr<XmlNode>globalNode=rootNode->GetFirstChildNode("GlobalMaxLevel");
    if(globalNode!=nullptr){
        globalMaxLevel=atoi(globalNode->GetData().c_str());
    }
    string tileUrl="{z}/{y}/{x}.json";
    AutoPtr<XmlNode>tileUrlNode=rootNode->GetFirstChildNode("TileUrl");
    if(tileUrlNode!=nullptr){
        tileUrl=tileUrlNode->GetData();
    }
    string exa=FilePath::GetFileExa(tileUrl);
    StringHelper::TrimToLower(exa);
    tileFormat=((exa=="json")?vtfJson:vtfStream);
    this->tileUrl=tileUrl;
    return true;
}

AutoPtr<VectorProvider>CGISMercatorVectorProvider::Clone(){
    CGISMercatorVectorProvider*pro=new CGISMercatorVectorProvider();
    pro->serverIps.resize(serverIps.size());
    for(int k=0;k<serverIps.size();k++) pro->serverIps[k]=serverIps[k];
    pro->boundingBox=boundingBox->Clone();
    pro->globalMaxLevel=globalMaxLevel;
    pro->tileUrl=tileUrl;
    return pro;
}

bool CGISMercatorVectorProvider::Read(BufferReader*br){
    boundingBox->Read(br);
    globalMaxLevel=br->Read<LONG>();
    int serverNum=br->Read<int>();
    serverIps.resize(serverNum);
    for(int k=0;k<serverNum;k++) serverIps[k]=br->ReadString();
    tileUrl=br->ReadString();
    return true;
}
void CGISMercatorVectorProvider::Write(BufferWriter*bw){
    boundingBox->Write(bw);
    bw->Write<LONG>(globalMaxLevel);
    bw->Write<int>(serverIps.size());
    for(int k=0;k<serverIps.size();k++) bw->WriteString(serverIps[k]);
    bw->WriteString(tileUrl);
}

string CGISMercatorVectorProvider::GetTypeName(){
    return "CGISMercatorVectorProvider";
}


bool CGISMercatorVectorProvider::ReadFromJson(JsonReader*jr,JsonNode node){
    boundingBox->ReadFromJson(jr,node["boundingBox"]);
    globalMaxLevel=node["globalMaxLevel"].asInt();
    JsonNode ipNode=node["serverIps"];
    serverIps.resize(ipNode.size());
    for(int k=0;k<serverIps.size();k++) serverIps[k]= ipNode[k].asString();
    tileUrl=node["tileUrl"].asInt();
    return true;
}

string CGISMercatorVectorProvider::ToJson(){
    JsonObjectWriter jow;
    jow.AddWritable("boundingBox",boundingBox.get());
    jow.Add<int>("globalMaxLevel",globalMaxLevel);
    JsonArrayWriter jaw;
    for(int k=0;k<serverIps.size();k++) jaw.Add<string>(serverIps[k]);
    jow.AddWriter("serverIps",&jaw);
    jow.Add<string>("tileUrl",tileUrl);
    return jow.GetJsonString();
}

}
