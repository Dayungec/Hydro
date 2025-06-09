#ifndef MAPPROVIDER_H
#define MAPPROVIDER_H
#include "Base/autoptr.h"
#include "mapprojection.h"

namespace SGIS{

class SGIS_EXPORT MapProvider:
        public Object,
        public BufferWritable,
        public JsonWritable
{
public:
    MapProvider();
    virtual~MapProvider();
    string&Name();
    int&MinLevel();
    int&MaxLevel();
    virtual AutoPtr<MapProvider>Clone()=0;
    virtual AutoPtr<MapProjection>GetMapProjection()=0;
    virtual string GetImageUrl(MapTile tile)=0;
protected:
    int GetServerNum(int max);
protected:
    string name;
    int maxLevel;
    int minLevel;
};

class SGIS_EXPORT MercatorMapProvider:public MapProvider{
public:
    MercatorMapProvider();
    virtual~MercatorMapProvider();
    LONG&ServerNumber();
    string&ServerIp();
    LONG&GlobalMaxLevel();
    AutoPtr<Envelope>GetBoundingBox();
    void SetBoundingBox(Envelope*env);
    AutoPtr<MapProjection>GetMapProjection();
    string GetImageUrl(MapTile tile);
    void SetAsTiandituSatProvider(string userKey);
    AutoPtr<MapProvider>Clone();
public:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    AutoPtr<MercatorProjection>mp;
    AutoPtr<Envelope>boundingBox;
    LONG globalMaxLevel;
    LONG serverNumber;
    string serverIp;
};

class SGIS_EXPORT TiandituMapProvider :public MapProvider{
public:
    TiandituMapProvider(string userKey="");
    virtual~TiandituMapProvider();
    string&UserKey();
    string&ServerIp();
    bool&Labels();
    AutoPtr<MapProjection>GetMapProjection();
    string GetImageUrl(MapTile tile);
    AutoPtr<MapProvider>Clone();
public:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    AutoPtr<MercatorProjection>mp;
    string userKey;
    string serverIp;
    bool labels;
};

class SGIS_EXPORT TiandituSatelliteMapProvider :public MapProvider{
public:
    TiandituSatelliteMapProvider(string userKey="");
    virtual~TiandituSatelliteMapProvider();
    string&UserKey();
    string&ServerIp();
    AutoPtr<MapProjection>GetMapProjection();
    string GetImageUrl(MapTile tile);
    AutoPtr<MapProvider>Clone();
public:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    AutoPtr<MercatorProjection>mp;
    string userKey;
    string serverIp;
};

class SGIS_EXPORT TiandituLabelSatelliteMapProvider :public MapProvider{
public:
    TiandituLabelSatelliteMapProvider(string userKey="");
    virtual~TiandituLabelSatelliteMapProvider();
    string&UserKey();
    string&ServerIp();
    AutoPtr<MapProjection>GetMapProjection();
    string GetImageUrl(MapTile tile);
    AutoPtr<MapProvider>Clone();
public:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
   bool ReadFromJson(JsonReader*jr,JsonNode node);
   string ToJson();
protected:
    AutoPtr<MercatorProjection>mp;
    string userKey;
    string serverIp;
};

class SGIS_EXPORT BingSatelliteMapProvider :public MapProvider{
public:
    BingSatelliteMapProvider();
    virtual~BingSatelliteMapProvider();
    string&ServerIp();
    AutoPtr<MapProjection>GetMapProjection();
    string GetImageUrl(MapTile tile);
    AutoPtr<MapProvider>Clone();
public:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    string TileXYToQuadKey(int tileX, int tileY, int levelOfDetail);
protected:
    AutoPtr<MercatorProjection>mp;
    string serverIp;
};

class SGIS_EXPORT CGISMercatorMapProvider:public MapProvider{
public:
    CGISMercatorMapProvider();
    virtual~CGISMercatorMapProvider();
    void AddServerIp(string serverIp);
    int GetServerIpCount();
    string GetServerIp(int nIndex);
    string&TileUrl();
    LONG&GlobalMaxLevel();
    AutoPtr<Envelope>GetBoundingBox();
    void SetBoundingBox(Envelope*env);
    AutoPtr<MapProjection>GetMapProjection();
    string GetImageUrl(MapTile tile);
    bool LoadFromXml(string xmlPath);
    AutoPtr<MapProvider>Clone();
public:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    AutoPtr<MercatorProjection>mp;
    AutoPtr<Envelope>boundingBox;
    LONG globalMaxLevel;
    string tileUrl;
    vector<string>serverIps;
};

class SGIS_EXPORT CGISTerrainMapProvider:public MapProvider{
public:
    CGISTerrainMapProvider();
    virtual~CGISTerrainMapProvider();
    void AddServerIp(string serverIp);
    int GetServerIpCount();
    string GetServerIp(int nIndex);
    string&TileUrl();
    LONG&GlobalMaxLevel();
    float&MinDemHeight();
    float&MaxDemHeight();
    AutoPtr<Envelope>GetBoundingBox();
    void SetBoundingBox(Envelope*env);
    AutoPtr<MapProjection>GetMapProjection();
    string GetImageUrl(MapTile tile);
    bool LoadFromXml(string xmlPath);
    AutoPtr<MapProvider>Clone();
public:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
    //JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    AutoPtr<MercatorProjection>mp;
    AutoPtr<Envelope>boundingBox;
    LONG globalMaxLevel;
    string tileUrl;
    vector<string>serverIps;
    float minHeight=0;
    float maxHeight=8000;
};

class SGIS_EXPORT MapProviderHelper{
public:
    MapProviderHelper();
    virtual~MapProviderHelper();
    void SetMapProvider(AutoPtr<MapProvider>provider);
    AutoPtr<SpatialReference> GetSpatialReference();
    void GetTileByXY(double x, double y, int level, LONG&tilex, LONG&tiley);
    AutoPtr<Envelope> GetTileExtent(int level,LONG tileX, LONG tileY);
    Rect2D<int> GetTileSeries(Envelope*lonlatExtent, int level);
    AutoPtr<Envelope> GetTilesExtent(Envelope*lonlatExtent, int level);
    void GetCellSize(int level, double&xCellSize, double&yCellSize);
    AutoPtr<CoordinateTransformation> GetTransformFrom(AutoPtr<SpatialReference>lypsp);
    AutoPtr<CoordinateTransformation> GetTransformTo(AutoPtr<SpatialReference>lypsp);
protected:
    AutoPtr<MapProvider>pProvider;
    AutoPtr<SpatialReference>psp;
    AutoPtr<SpatialReference>geosp;
    AutoPtr<CoordinateTransformation>pTrans;
    AutoPtr<CoordinateTransformation>pTransRev;
};

};
#endif // MAPPROVIDER_H
