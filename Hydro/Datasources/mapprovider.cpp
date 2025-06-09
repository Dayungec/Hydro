#include "mapprovider.h"
#include "Base/StringHelper.h"
#include <time.h>
#include "Base/StringHelper.h"
#include "Base/classfactory.h"
#include "Base/xmldoc.h"
namespace SGIS{

REGISTER(MercatorMapProvider)
REGISTER(TiandituMapProvider)
REGISTER(TiandituSatelliteMapProvider)
REGISTER(TiandituLabelSatelliteMapProvider)
REGISTER(BingSatelliteMapProvider)
REGISTER(CGISMercatorMapProvider)

MapProvider::MapProvider()
{
    minLevel=0;
    maxLevel=19;
    srand((int)time(0));
}
MapProvider::~MapProvider()
{

}

int&MapProvider::MinLevel(){
    return minLevel;
}
int&MapProvider::MaxLevel(){
    return maxLevel;
}

string&MapProvider::Name(){
    return name;
}
int MapProvider::GetServerNum(int max){
    if(max==0) return 0;
    return rand()%max;
}

MercatorMapProvider::MercatorMapProvider(){
    name="MercatorMap";
    mp=new MercatorProjection();
    boundingBox=new Envelope(-180,90,180,-90);
    serverNumber=0;
    globalMaxLevel=0;
}

MercatorMapProvider::~MercatorMapProvider(){

}
LONG&MercatorMapProvider::ServerNumber(){
    return serverNumber;
}

string&MercatorMapProvider::ServerIp(){
    return serverIp;
}

LONG&MercatorMapProvider::GlobalMaxLevel(){
    return globalMaxLevel;
}
AutoPtr<Envelope>MercatorMapProvider::GetBoundingBox(){
    return boundingBox->Clone();
}

void MercatorMapProvider::SetBoundingBox(Envelope*env){
    boundingBox->CopyFromOther(env);
}

AutoPtr<MapProjection>MercatorMapProvider::GetMapProjection(){
    return mp;
}

string MercatorMapProvider::GetImageUrl(MapTile tile){
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

void MercatorMapProvider::SetAsTiandituSatProvider(string userKey){
    serverIp="http://t{s}.tianditu.gov.cn/img_w/wmts?request=GetTile&service=wmts&version=1.0.0&LAYER=img&tileMatrixSet=w&TileMatrix={z}&TileRow={y}&TileCol={x}&STYLE=default&tk="+userKey;
    serverNumber=8;
}

AutoPtr<MapProvider>MercatorMapProvider::Clone(){
    MercatorMapProvider*pro=new MercatorMapProvider();
    pro->boundingBox=boundingBox->Clone();
    pro->globalMaxLevel=globalMaxLevel;
    pro->serverNumber=serverNumber;
    pro->serverIp=serverIp;
    return pro;
}

bool MercatorMapProvider::Read(BufferReader*br){
    boundingBox->Read(br);
    globalMaxLevel=br->Read<LONG>();
    serverNumber=br->Read<LONG>();
    serverIp=br->ReadString();
    return true;
}
void MercatorMapProvider::Write(BufferWriter*bw){
    boundingBox->Write(bw);
    bw->Write<LONG>(globalMaxLevel);
    bw->Write<LONG>(serverNumber);
    bw->WriteString(serverIp);
}

bool MercatorMapProvider::ReadFromJson(JsonReader*jr,JsonNode node){
    boundingBox->ReadFromJson(jr,node["boundingBox"]);
    globalMaxLevel=node["globalMaxLevel"].asInt();
    serverNumber=node["serverNumber"].asInt();
    serverIp=node["serverIp"].asString();
    return true;
}

string MercatorMapProvider::ToJson(){
    JsonObjectWriter jow;
    jow.AddWritable("boundingBox",boundingBox.get());
    jow.Add<int>("globalMaxLevel",globalMaxLevel);
    jow.Add<int>("serverNumber",serverNumber);
    jow.Add<string>("serverIp",serverIp);
    return jow.GetJsonString();
}

string MercatorMapProvider::GetTypeName(){
    return "MercatorMapProvider";
}

TiandituMapProvider::TiandituMapProvider(string userKey){
    name="天地图";
    mp=new MercatorProjection();
    serverIp="";
    this->userKey=userKey;
    labels=true;
}
TiandituMapProvider::~TiandituMapProvider(){

}
string&TiandituMapProvider::UserKey(){
    return userKey;
}
string&TiandituMapProvider::ServerIp(){
    return serverIp;
}
bool&TiandituMapProvider::Labels(){
    return labels;
}

AutoPtr<MapProjection>TiandituMapProvider::GetMapProjection(){
    return mp;
}

bool TiandituMapProvider::Read(BufferReader*br){
    serverIp=br->ReadString();
    userKey=br->ReadString();
    labels=br->Read<bool>();
    return true;
}
void TiandituMapProvider::Write(BufferWriter*bw){
    bw->WriteString(serverIp);
    bw->WriteString(userKey);
    bw->Write<bool>(labels);
}

string TiandituMapProvider::GetTypeName(){
    return "TiandituMapProvider";
}

bool TiandituMapProvider::ReadFromJson(JsonReader*jr,JsonNode node){
    serverIp=node["serverIp"].asString();
    userKey=node["userKey"].asString();
    labels=JsonReader::GetBoolValue(node["labels"]);
    return true;
}

string TiandituMapProvider::ToJson(){
    JsonObjectWriter jow;
    jow.Add<string>("serverIp",serverIp);
    jow.Add<string>("userKey",userKey);
    jow.Add<string>("labels",(labels?"true":"false"));
    return jow.GetJsonString();
}


string TiandituMapProvider::GetImageUrl(MapTile tile){
    int serverNum=GetServerNum(8);
    string server=to_string(serverNum);
    int iX=tile.TileX,iY=tile.TileY;
    string url;
    if(serverIp=="")
        url="http://t"+server+".tianditu.gov.cn";
    else
        url="http://"+serverIp;
    if(labels){
        string sFormat=StringHelper::Format("/DataServer?T=cva_w&x=%d&y=%d&l=%d",iX,iY,tile.Level);
        sFormat=url+sFormat+"&STYLE=default&tk=" + userKey;
        string sFormat2=StringHelper::Format("/DataServer?T=vec_w&x=%d&y=%d&l=%d",iX,iY,tile.Level);
        sFormat+="\r\n"+url+sFormat2;
        sFormat+="&STYLE=default&tk=" + userKey;
        return sFormat;
    }
    else{
        string sFormat2=StringHelper::Format("/DataServer?T=vec_w&x=%d&y=%d&l=%d",iX,iY,tile.Level);
        sFormat2=url+sFormat2;
        sFormat2+="&STYLE=default&tk=" + userKey;
        return sFormat2;
    }
}

AutoPtr<MapProvider>TiandituMapProvider::Clone(){
    TiandituMapProvider*pro=new TiandituMapProvider();
    pro->userKey=userKey;
    pro->serverIp=serverIp;
    pro->labels=labels;
    return pro;
}

TiandituSatelliteMapProvider::TiandituSatelliteMapProvider(string userKey){
    name="天地图遥感影像";
    mp=new MercatorProjection();
    serverIp="";
    this->userKey=userKey;
}
TiandituSatelliteMapProvider::~TiandituSatelliteMapProvider(){

}
string&TiandituSatelliteMapProvider::UserKey(){
    return userKey;
}
string&TiandituSatelliteMapProvider::ServerIp(){
    return serverIp;
}
AutoPtr<MapProjection>TiandituSatelliteMapProvider::GetMapProjection(){
    return mp;
}

bool TiandituSatelliteMapProvider::Read(BufferReader*br){
    serverIp=br->ReadString();
    userKey=br->ReadString();
    return true;
}
void TiandituSatelliteMapProvider::Write(BufferWriter*bw){
    bw->WriteString(serverIp);
    bw->WriteString(userKey);
}

string TiandituSatelliteMapProvider::GetTypeName(){
    return "TiandituSatelliteMapProvider";
}

bool TiandituSatelliteMapProvider::ReadFromJson(JsonReader*jr,JsonNode node){
    serverIp=node["serverIp"].asString();
    userKey=node["userKey"].asString();
    return true;
}

string TiandituSatelliteMapProvider::ToJson(){
    JsonObjectWriter jow;
    jow.Add<string>("serverIp",serverIp);
    jow.Add<string>("userKey",userKey);
    return jow.GetJsonString();
}


string TiandituSatelliteMapProvider::GetImageUrl(MapTile tile){
    int serverNum=GetServerNum(8);
    string server=to_string(serverNum);
    string url;
    if(serverIp=="")
        url="http://t"+server+".tianditu.gov.cn";
    else
        url="http://"+serverIp;
    int iX=tile.TileX,iY=tile.TileY;
    string sFormat=StringHelper::Format("/img_w/wmts?request=GetTile&service=wmts&version=1.0.0&LAYER=img&tileMatrixSet=w&TileMatrix=%d&TileRow=%d&TileCol=%d",tile.Level,iY,iX);
    sFormat=url+sFormat;
    sFormat+="&STYLE=default&tk=" + userKey;
    return sFormat;
}

AutoPtr<MapProvider>TiandituSatelliteMapProvider::Clone(){
    TiandituSatelliteMapProvider*pro=new TiandituSatelliteMapProvider();
    pro->userKey=userKey;
    pro->serverIp=serverIp;
    return pro;
}

TiandituLabelSatelliteMapProvider::TiandituLabelSatelliteMapProvider(string userKey){
    name="天地图标注天地图遥感影像";
    mp=new MercatorProjection();
    serverIp="";
    this->userKey=userKey;
}
TiandituLabelSatelliteMapProvider::~TiandituLabelSatelliteMapProvider(){

}
string&TiandituLabelSatelliteMapProvider::UserKey(){
    return userKey;
}
string&TiandituLabelSatelliteMapProvider::ServerIp(){
    return serverIp;
}
AutoPtr<MapProjection>TiandituLabelSatelliteMapProvider::GetMapProjection(){
    return mp;
}

bool TiandituLabelSatelliteMapProvider::Read(BufferReader*br){
    serverIp=br->ReadString();
    userKey=br->ReadString();
    return true;
}
void TiandituLabelSatelliteMapProvider::Write(BufferWriter*bw){
    bw->WriteString(serverIp);
    bw->WriteString(userKey);
}

string TiandituLabelSatelliteMapProvider::GetTypeName(){
    return "TiandituLabelSatelliteMapProvider";
}


bool TiandituLabelSatelliteMapProvider::ReadFromJson(JsonReader*jr,JsonNode node){
    serverIp=node["serverIp"].asString();
    userKey=node["userKey"].asString();
    return true;
}

string TiandituLabelSatelliteMapProvider::ToJson(){
    JsonObjectWriter jow;
    jow.Add<string>("serverIp",serverIp);
    jow.Add<string>("userKey",userKey);
    return jow.GetJsonString();
}

string TiandituLabelSatelliteMapProvider::GetImageUrl(MapTile tile){
    int serverNum=GetServerNum(8);
    string server=to_string(serverNum);
    string url;
    if(serverIp=="")
        url="http://t"+server+".tianditu.gov.cn";
    else
        url="http://"+serverIp;
    int iX=tile.TileX,iY=tile.TileY;

    string sFormat;
    sFormat=StringHelper::Format("/cia_w/wmts?request=GetTile&service=wmts&version=1.0.0&LAYER=cia&tileMatrixSet=w&TileMatrix=%d&TileRow=%d&TileCol=%d",tile.Level,iY,iX);
    sFormat=url+sFormat+"&STYLE=default&tk=" + userKey;
    string sFormat2=StringHelper::Format("/img_w/wmts?request=GetTile&service=wmts&version=1.0.0&LAYER=img&tileMatrixSet=w&TileMatrix=%d&TileRow=%d&TileCol=%d",tile.Level,iY,iX);
    sFormat+="\r\n"+url+sFormat2;
    sFormat+="&STYLE=default&tk="+ userKey;
    return sFormat;
}

AutoPtr<MapProvider>TiandituLabelSatelliteMapProvider::Clone(){
    TiandituLabelSatelliteMapProvider*pro=new TiandituLabelSatelliteMapProvider();
    pro->userKey=userKey;
    pro->serverIp=serverIp;
    return pro;
}

BingSatelliteMapProvider::BingSatelliteMapProvider(){
    name="Bing遥感影像";
    mp=new MercatorProjection();
    serverIp="";
}
BingSatelliteMapProvider::~BingSatelliteMapProvider(){

}

string&BingSatelliteMapProvider::ServerIp(){
    return serverIp;
}
AutoPtr<MapProjection>BingSatelliteMapProvider::GetMapProjection(){
    return mp;
}
string BingSatelliteMapProvider::TileXYToQuadKey(int tileX, int tileY, int levelOfDetail){
    string quadKey;
    for(int i = levelOfDetail; i > 0; i--)
    {
        char digit = '0';
        int mask = 1 << (i - 1);
        if((tileX & mask) != 0)
        {
            digit++;
        }
        if((tileY & mask) != 0)
        {
            digit++;
            digit++;
        }
        quadKey.push_back(digit);
    }
    return quadKey;
}
bool BingSatelliteMapProvider::Read(BufferReader*br){
    serverIp=br->ReadString();
    return true;
}
void BingSatelliteMapProvider::Write(BufferWriter*bw){
    bw->WriteString(serverIp);
}

string BingSatelliteMapProvider::GetTypeName(){
    return "BingSatelliteMapProvider";
}

bool BingSatelliteMapProvider::ReadFromJson(JsonReader*jr,JsonNode node){
    serverIp=node["serverIp"].asString();
    return true;
}

string BingSatelliteMapProvider::ToJson(){
    JsonObjectWriter jow;
    jow.Add<string>("serverIp",serverIp);
    return jow.GetJsonString();
}

string BingSatelliteMapProvider::GetImageUrl(MapTile tile){
    int iX=tile.TileX;
    int iY=tile.TileY;
    string key = TileXYToQuadKey(iX, iY, tile.Level);
    string url="http://ecn.t";
    int serverNum=GetServerNum(4);
    string sServerNum=to_string(serverNum);
    url+=sServerNum;
    string Version = "845";
    string Language="zh-CN";
    if(serverIp=="")
        url+=".tiles.virtualearth.net/tiles/a"+key+".jpeg?g="+Version+"&mkt="+Language;
    else
        url="http://"+serverIp+"/tiles/a"+key+".jpeg?g="+Version+"&mkt="+Language;
    return url;
}


AutoPtr<MapProvider>BingSatelliteMapProvider::Clone(){
    BingSatelliteMapProvider*pro=new BingSatelliteMapProvider();
    pro->serverIp=serverIp;
    return pro;
}

CGISMercatorMapProvider::CGISMercatorMapProvider(){
    name="CGISMercatorMap";
    mp=new MercatorProjection();
    boundingBox=new Envelope(-180,90,180,-90);
    globalMaxLevel=0;
    tileUrl="";
}

CGISMercatorMapProvider::~CGISMercatorMapProvider(){

}

void CGISMercatorMapProvider::AddServerIp(string serverIp){
    serverIps.push_back(serverIp);
}
int CGISMercatorMapProvider::GetServerIpCount(){
    return serverIps.size();
}
string CGISMercatorMapProvider::GetServerIp(int nIndex){
    return serverIps[nIndex];
}

string&CGISMercatorMapProvider::TileUrl(){
    return tileUrl;
}

LONG&CGISMercatorMapProvider::GlobalMaxLevel(){
    return globalMaxLevel;
}

AutoPtr<Envelope>CGISMercatorMapProvider::GetBoundingBox(){
    return boundingBox->Clone();
}

void CGISMercatorMapProvider::SetBoundingBox(Envelope*env){
    boundingBox->CopyFromOther(env);
}

AutoPtr<MapProjection>CGISMercatorMapProvider::GetMapProjection(){
    return mp;
}

string CGISMercatorMapProvider::GetImageUrl(MapTile tile){
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
        if(serverIp.empty())
            url=tileUrl;
        else
            url=serverIp+"/"+tileUrl;
        int nPos=url.find("{s");
        if(nPos>=0){
            int nPos2=url.find("}",nPos);
            if(nPos2>0){
                string sInf=url.substr(nPos+2,nPos2-nPos-2);
                int serverNum=atoi(sInf.c_str());
                if(serverNum>0){
                    serverNum=rand()%serverNum;
                    string sInfo=to_string(serverNum);
                    url=StringHelper::Replace(url,"{s"+sInf+"}",sInfo);
                }
            }
        }
        url=StringHelper::Replace(url,"{x}",to_string(tile.TileX));
        url=StringHelper::Replace(url,"{y}",to_string(tile.TileY));
        url=StringHelper::Replace(url,"{z}",to_string(tile.Level));
        return url;
    }
    return serverIp+"/"+to_string(tile.Level)+"/"+to_string(tile.TileY)+"_"+to_string(tile.TileX)+".png";
}

bool CGISMercatorMapProvider::LoadFromXml(string xmlPath){
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
    string tileUrl="{z}/{x}_{y}.png";
    AutoPtr<XmlNode>tileUrlNode=rootNode->GetFirstChildNode("TileUrl");
    if(tileUrlNode!=nullptr){
        tileUrl=tileUrlNode->GetData();
    }
    this->tileUrl=tileUrl;
    return true;
}

AutoPtr<MapProvider>CGISMercatorMapProvider::Clone(){
    CGISMercatorMapProvider*pro=new CGISMercatorMapProvider();
    pro->serverIps.resize(serverIps.size());
    for(int k=0;k<serverIps.size();k++) pro->serverIps[k]=serverIps[k];
    pro->boundingBox=boundingBox->Clone();
    pro->globalMaxLevel=globalMaxLevel;
    pro->tileUrl=tileUrl;
    return pro;
}

bool CGISMercatorMapProvider::Read(BufferReader*br){
    boundingBox->Read(br);
    globalMaxLevel=br->Read<LONG>();
    int serverNum=br->Read<int>();
    serverIps.resize(serverNum);
    for(int k=0;k<serverNum;k++) serverIps[k]=br->ReadString();
    tileUrl=br->ReadString();
    return true;
}
void CGISMercatorMapProvider::Write(BufferWriter*bw){
    boundingBox->Write(bw);
    bw->Write<LONG>(globalMaxLevel);
    bw->Write<int>(serverIps.size());
    for(int k=0;k<serverIps.size();k++) bw->WriteString(serverIps[k]);
    bw->WriteString(tileUrl);
}

string CGISMercatorMapProvider::GetTypeName(){
    return "CGISMercatorMapProvider";
}


bool CGISMercatorMapProvider::ReadFromJson(JsonReader*jr,JsonNode node){
    boundingBox->ReadFromJson(jr,node["boundingBox"]);
    globalMaxLevel=node["globalMaxLevel"].asInt();
    JsonNode ipNode=node["serverIps"];
    serverIps.resize(ipNode.size());
    for(int k=0;k<serverIps.size();k++) serverIps[k]= ipNode[k].asString();
    tileUrl=node["tileUrl"].asString();
    return true;
}

string CGISMercatorMapProvider::ToJson(){
    JsonObjectWriter jow;
    jow.AddWritable("boundingBox",boundingBox.get());
    jow.Add<int>("globalMaxLevel",globalMaxLevel);
    JsonArrayWriter jaw;
    for(int k=0;k<serverIps.size();k++) jaw.Add<string>(serverIps[k]);
    jow.AddWriter("serverIps",&jaw);
    jow.Add<string>("tileUrl",tileUrl);
    return jow.GetJsonString();
}

CGISTerrainMapProvider::CGISTerrainMapProvider(){
    name="CGISTerrainMap";
    mp=new MercatorProjection();
    boundingBox=new Envelope(-180,90,180,-90);
    globalMaxLevel=0;
    tileUrl="";
}

CGISTerrainMapProvider::~CGISTerrainMapProvider(){

}

void CGISTerrainMapProvider::AddServerIp(string serverIp){
    serverIps.push_back(serverIp);
}

int CGISTerrainMapProvider::GetServerIpCount(){
    return serverIps.size();
}

string CGISTerrainMapProvider::GetServerIp(int nIndex){
    return serverIps[nIndex];
}

string&CGISTerrainMapProvider::TileUrl(){
    return tileUrl;
}

LONG&CGISTerrainMapProvider::GlobalMaxLevel(){
    return globalMaxLevel;
}

float&CGISTerrainMapProvider::MinDemHeight(){
    return minHeight;
}

float&CGISTerrainMapProvider::MaxDemHeight(){
    return maxHeight;
}

AutoPtr<Envelope>CGISTerrainMapProvider::GetBoundingBox(){
    return boundingBox->Clone();
}

void CGISTerrainMapProvider::SetBoundingBox(Envelope*env){
    boundingBox->CopyFromOther(env);
}

AutoPtr<MapProjection>CGISTerrainMapProvider::GetMapProjection(){
    return mp;
}

string CGISTerrainMapProvider::GetImageUrl(MapTile tile){
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
    }
    return serverIp+"/"+to_string(tile.Level)+"/"+to_string(tile.TileY)+"_"+to_string(tile.TileX)+".terrain";
}

bool CGISTerrainMapProvider::LoadFromXml(string xmlPath){
    AutoPtr<XmlDoc>xmlDoc(new XmlDoc());
    if(!xmlDoc->Load(xmlPath)) return false;
    AutoPtr<XmlNode>rootNode=xmlDoc->GetRootNode();
    if(rootNode==nullptr) return false;
    if(rootNode->GetName()!="Terrain") return false;
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
    string tileUrl="{z}/{x}_{y}.terrain";
    AutoPtr<XmlNode>tileUrlNode=rootNode->GetFirstChildNode("TileUrl");
    if(tileUrlNode!=nullptr){
        tileUrl=tileUrlNode->GetData();
    }
    this->tileUrl=tileUrl;
    this->minHeight=0;
    this->maxHeight=8000;
    AutoPtr<XmlNode>minHNode=rootNode->GetFirstChildNode("MinHeight");
    if(globalNode!=nullptr){
        minHeight=atof(minHNode->GetData().c_str());
    }
    AutoPtr<XmlNode>maxHNode=rootNode->GetFirstChildNode("MaxHeight");
    if(globalNode!=nullptr){
        maxHeight=atof(maxHNode->GetData().c_str());
    }
    return true;
}

AutoPtr<MapProvider>CGISTerrainMapProvider::Clone(){
    CGISTerrainMapProvider*pro=new CGISTerrainMapProvider();
    pro->serverIps.resize(serverIps.size());
    for(int k=0;k<serverIps.size();k++) pro->serverIps[k]=serverIps[k];
    pro->boundingBox=boundingBox->Clone();
    pro->globalMaxLevel=globalMaxLevel;
    pro->tileUrl=tileUrl;
    pro->minHeight=minHeight;
    pro->maxHeight=maxHeight;
    return pro;
}

bool CGISTerrainMapProvider::Read(BufferReader*br){
    boundingBox->Read(br);
    globalMaxLevel=br->Read<LONG>();
    int serverNum=br->Read<int>();
    serverIps.resize(serverNum);
    for(int k=0;k<serverNum;k++) serverIps[k]=br->ReadString();
    tileUrl=br->ReadString();
    minHeight=br->Read<float>();
    maxHeight=br->Read<float>();
    return true;
}
void CGISTerrainMapProvider::Write(BufferWriter*bw){
    boundingBox->Write(bw);
    bw->Write<LONG>(globalMaxLevel);
    bw->Write<int>(serverIps.size());
    for(int k=0;k<serverIps.size();k++) bw->WriteString(serverIps[k]);
    bw->WriteString(tileUrl);
    bw->Write<float>(minHeight);
    bw->Write<float>(maxHeight);
}

string CGISTerrainMapProvider::GetTypeName(){
    return "CGISTerrainMapProvider";
}

bool CGISTerrainMapProvider::ReadFromJson(JsonReader*jr,JsonNode node){
    boundingBox->ReadFromJson(jr,node["boundingBox"]);
    globalMaxLevel=node["globalMaxLevel"].asInt();
    JsonNode ipNode=node["serverIps"];
    serverIps.resize(ipNode.size());
    for(int k=0;k<serverIps.size();k++) serverIps[k]= ipNode[k].asString();
    tileUrl=node["tileUrl"].asInt();
    minHeight=node["minHeight"].asFloat();
    maxHeight=node["maxHeight"].asFloat();
    return true;
}

string CGISTerrainMapProvider::ToJson(){
    JsonObjectWriter jow;
    jow.AddWritable("boundingBox",boundingBox.get());
    jow.Add<int>("globalMaxLevel",globalMaxLevel);
    JsonArrayWriter jaw;
    for(int k=0;k<serverIps.size();k++) jaw.Add<string>(serverIps[k]);
    jow.AddWriter("serverIps",&jaw);
    jow.Add<string>("tileUrl",tileUrl);
    jow.Add<float>("minnHeight",minHeight);
    jow.Add<float>("maxHeight",maxHeight);
    return jow.GetJsonString();
}

MapProviderHelper::MapProviderHelper(){
    pProvider=new TiandituSatelliteMapProvider();
    AutoPtr<MapProjection>proj=pProvider->GetMapProjection();
    psp=proj->GetSpatialReference();
    geosp=new SpatialReference();
    geosp->ImportFromEPSG(4610);
    pTrans=new CoordinateTransformation();
    pTrans->SetFromSpatialReference(geosp);
    pTrans->SetToSpatialReference(psp);
    pTrans->BeginTransform();
    pTransRev=new CoordinateTransformation();
    pTransRev->SetFromSpatialReference(psp);
    pTransRev->SetToSpatialReference(geosp);
    pTransRev->BeginTransform();
}
MapProviderHelper::~MapProviderHelper(){

}
void MapProviderHelper::SetMapProvider(AutoPtr<MapProvider>provider){
    pProvider = provider;
    AutoPtr<MapProjection>proj=pProvider->GetMapProjection();
    psp=proj->GetSpatialReference();
    geosp=new SpatialReference();
    geosp->ImportFromEPSG(4610);
    pTrans=new CoordinateTransformation();
    pTrans->SetFromSpatialReference(geosp);
    pTrans->SetToSpatialReference(psp);
    pTrans->BeginTransform();
    pTransRev=new CoordinateTransformation();
    pTransRev->SetFromSpatialReference(psp);
    pTransRev->SetToSpatialReference(geosp);
    pTransRev->BeginTransform();
}
AutoPtr<SpatialReference>MapProviderHelper::GetSpatialReference(){
    return psp;
}
void MapProviderHelper::GetTileByXY(double x, double y, int level, LONG&tilex, LONG&tiley){
    pTransRev->TransformXY(&x, &y);
    AutoPtr<MapProjection>proj=pProvider->GetMapProjection();
    DOUBLE px, py;
    proj->FromLatLngToPixel(x, y, level, &px, &py);
    proj->FromPixelToTileXY(px, py, &tilex, &tiley);
}
AutoPtr<Envelope> MapProviderHelper::GetTileExtent(int level,LONG tileX, LONG tileY){
    AutoPtr<MapProjection>proj=pProvider->GetMapProjection();
    double tilex, tiley;
    double lon1, lat1, lon2, lat2;
    proj->FromTileXYToPixel(tileX, tileY, &tilex, &tiley);
    proj->FromPixelToLatLng(tilex, tiley, level, &lon1, &lat1);
    pTrans->TransformXY(&lon1, &lat1);
    proj->FromTileXYToPixel(tileX+1, tileY + 1, &tilex, &tiley);
    proj->FromPixelToLatLng(tilex, tiley, level, &lon2, &lat2);
    pTrans->TransformXY(&lon2, &lat2);
    AutoPtr<Envelope>env(new Envelope());
    env->PutCoord(lon1, lat1, lon2, lat2);
    return env;
}
Rect2D<int> MapProviderHelper::GetTileSeries(Envelope*lonlatExtent, int level){
    AutoPtr<MapProjection>proj=pProvider->GetMapProjection();
    DOUBLE x1, y1, x2, y2;
    DOUBLE pixelx, pixely;
    LONG tilex1, tiley1, tilex2, tiley2;
    lonlatExtent->GetCoord(&x1, &y1, &x2, &y2);
    proj->FromLatLngToPixel(x1, y1, level, &pixelx, &pixely);
    proj->FromPixelToTileXY(pixelx, pixely, &tilex1, &tiley1);
    proj->FromLatLngToPixel(x2, y2, level, &pixelx, &pixely);
    proj->FromPixelToTileXY(pixelx, pixely, &tilex2, &tiley2);
    return Rect2D<int>(tilex1, tiley1, tilex2, tiley2);
}
AutoPtr<Envelope> MapProviderHelper::GetTilesExtent(Envelope*lonlatExtent, int level){
    Rect2D<int>tileEnv=GetTileSeries(lonlatExtent,level);
    int tilex1 = tileEnv.Left;
    int tiley1 = tileEnv.Top;
    int tilex2 = tileEnv.Right;
    int tiley2 = tileEnv.Bottom;
    AutoPtr<MapProjection>proj=pProvider->GetMapProjection();
    double tilex, tiley;
    double lon1, lat1, lon2, lat2;
    proj->FromTileXYToPixel(tilex1, tiley1, &tilex, &tiley);
    proj->FromPixelToLatLng(tilex, tiley, level, &lon1, &lat1);
    pTrans->TransformXY(&lon1, &lat1);
    proj->FromTileXYToPixel(tilex2 + 1, tiley2 + 1, &tilex, &tiley);
    proj->FromPixelToLatLng(tilex, tiley, level, &lon2, &lat2);
    pTrans->TransformXY(&lon2, &lat2);
    AutoPtr<Envelope>env(new Envelope());
    env->PutCoord(lon1, lat1, lon2, lat2);
    return env;
}
void MapProviderHelper::GetCellSize(int level, double&xCellSize, double&yCellSize){
    AutoPtr<MapProjection>proj=pProvider->GetMapProjection();
    LONG xnum,ynum;
    xnum=proj->GetTotalXTiles(level);
    ynum=proj->GetTotalYTiles(level);
    LONG cx = xnum / 2;
    LONG cy = ynum / 2;
    AutoPtr<Envelope>env=GetTileExtent(level, cx, cy);
    Point2D<int>ts=proj->GetTileSize();
    DOUBLE x1, y1, x2, y2;
    env->GetCoord(&x1, &y1, &x2, &y2);
    xCellSize = (x2 - x1) / ts.X;
    yCellSize= (y1 - y2) / ts.Y;
}
AutoPtr<CoordinateTransformation> MapProviderHelper::GetTransformFrom(AutoPtr<SpatialReference>lypsp){
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation());
    pTrans->SetFromSpatialReference(lypsp);
    pTrans->SetToSpatialReference(psp);
    pTrans->BeginTransform();
    return pTrans;
}
AutoPtr<CoordinateTransformation> MapProviderHelper::GetTransformTo(AutoPtr<SpatialReference>lypsp){
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation());
    pTrans->SetFromSpatialReference(psp);
    pTrans->SetToSpatialReference(lypsp);
    pTrans->BeginTransform();
    return pTrans;
}

}
