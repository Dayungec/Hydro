#ifndef VECTORPROVIDER_H
#define VECTORPROVIDER_H
#include "Base/autoptr.h"
#include "Base/jsonio.h"
#include "Base/bufferio.h"
#include "mapprojection.h"
#include <time.h>

namespace SGIS{

enum VectorTileFormat{
    vtfJson=0,
    vtfStream=1
};

class SGIS_EXPORT VectorProvider:
        public Object,
        public BufferWritable,
        public JsonWritable
{
public:
    VectorProvider();
    virtual~VectorProvider();
    string&Name();
    VectorTileFormat GetVectorTileFormat();
    int&MinLevel();
    int&MaxLevel();
    virtual AutoPtr<VectorProvider>Clone()=0;
    virtual AutoPtr<MapProjection>GetMapProjection()=0;
    virtual string GetVectorUrl(MapTile tile)=0;
protected:
    int GetServerNum(int max);
protected:
    string name;
    int maxLevel;
    int minLevel;
    VectorTileFormat tileFormat;
};


class SGIS_EXPORT MercatorVectorProvider:public VectorProvider{
public:
    MercatorVectorProvider(VectorTileFormat tileFormat=vtfJson);
    virtual~MercatorVectorProvider();
    LONG&ServerNumber();
    string&ServerIp();
    LONG&GlobalMaxLevel();
    AutoPtr<Envelope>GetBoundingBox();
    void SetBoundingBox(Envelope*env);
    AutoPtr<MapProjection>GetMapProjection();
    string GetVectorUrl(MapTile tile);
    AutoPtr<VectorProvider>Clone();
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


class SGIS_EXPORT CGISMercatorVectorProvider:public VectorProvider{
public:
    CGISMercatorVectorProvider();
    virtual~CGISMercatorVectorProvider();
    void AddServerIp(string serverIp);
    int GetServerIpCount();
    string GetServerIp(int nIndex);
    string&TileUrl();
    LONG&GlobalMaxLevel();
    AutoPtr<Envelope>GetBoundingBox();
    void SetBoundingBox(Envelope*env);
    AutoPtr<MapProjection>GetMapProjection();
    string GetVectorUrl(MapTile tile);
    bool LoadFromXml(string xmlPath);
    AutoPtr<VectorProvider>Clone();
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


};
#endif // VECTORPROVIDER_H
