#ifndef JSONREADER_H
#define JSONREADER_H
#include "base_globle.h"
#include "autoptr.h"
#include "Json/json.h"
#include "classfactory.h"
#include "datetime.h"
#include "variant.h"

namespace SGIS{

enum JsonValueType {
  jvtNull = 0,
  jvtInt,
  jvtUint,
  jvtReal,
  jvtString,
  jvtBool,
  jvtArray,
  jvtObj
};

class JsonNode;
class JsonWritable;
class JsonReader;
class JsonWriter;
class JsonArrayWriter;
class JsonObjectWriter;

class SGIS_EXPORT JsonNode{
public:
    JsonNode(Json::Value node);
    JsonNode(const JsonNode&other);
    virtual~JsonNode();
    string&PathName();
    vector<string>getMemberNames();
    JsonNode&operator =(const JsonNode&other);
    bool Empty();
    UINT asUInt();
    bool asBool();
    int asInt();
    int asInt64();
    ULONGLONG asUint64();
    float asFloat();
    double asDouble();
    string asString();
    int size();
    JsonValueType ValueType();
    JsonNode operator [](int nIndex);
    JsonNode operator [](string name);
    map<string,AutoPtr<JsonNode>>Children();
protected:
    Json::Value node;
    string lpszPathName;
};

class SGIS_EXPORT JsonWritable:
    public Object{
public:
    JsonWritable(){};
    virtual~JsonWritable(){};
    virtual bool ReadFromJson(JsonReader*jr,JsonNode node)=0;
    virtual string ToJson()=0;
    virtual std::string GetTypeName()=0;
};

class SGIS_EXPORT JsonReader
{
public:
    JsonReader();
    virtual~JsonReader();
    string&PathName();
    AutoPtr<JsonNode>Parse(const string&jsonStr);
    AutoPtr<JsonNode>Load(const string&jsonFile);
    static AutoPtr<JsonWritable>ReadObject(JsonReader*jr,JsonNode node);
    static bool GetBoolValue(JsonNode node);
protected:
    Json::Reader jsonReader;
    string pathName;
};

class SGIS_EXPORT JsonWriter{
public:
    JsonWriter();
    virtual~JsonWriter();
    virtual void AddWriter(string property,JsonWriter*el)=0;
    virtual void AddWritable(string property,JsonWritable*el)=0;
    virtual string GetJsonString()=0;
protected:
    static string ToString(string value);
    static string ToString(DateTime value);
    static string ToString(int value);
    static string ToString(double value);
    static string ToString(bool value);
protected:
    string json;
};

class SGIS_EXPORT JsonArrayWriter:public JsonWriter{
public:
    JsonArrayWriter();
    JsonArrayWriter(const vector<Variant>&values);
    virtual~JsonArrayWriter();
    template<typename T>
    void Add(T value){
        if(json!="") json+=",";
        json+=ToString(value);
    };
    template<typename T>
    void Add(string property,T value){
        if(json!="") json+=",";
        json+="{\""+property+"\":"+ToString(value)+"}";
    };
    template<typename T>
    void Add(const vector<T>&values){
        for(int k=0;k<values.size();k++){
            if(json!="") json+=",";
            json+=ToString(values[k]);
        }
    };
    template<typename T>
    void Add(string property,const vector<T>&values){
        for(int k=0;k<values.size();k++){
            if(json!="") json+=",";
            json+="{\""+property+"\":"+ToString(values[k])+"}";
        }
    };
    void AddWriter(JsonWriter*el);
    void AddWritable(JsonWritable*el);
    void AddWriter(string property,JsonWriter*el);
    void AddWritable(string property,JsonWritable*el);
    void AddProperty(string property,string json);
    void WriteObject(JsonWritable*el);
    string GetJsonString();
};

class SGIS_EXPORT JsonObjectWriter :public JsonWriter{
public:
    JsonObjectWriter(string baseJson="");
    JsonObjectWriter(const vector<tuple<string,Variant>>&values,string baseJson="");
    virtual~JsonObjectWriter();
    template<typename T>
    void Add(string property,T value){
        if(json!="") json+=",";
        json+="\""+property+"\":"+ToString(value);
    };
    template<typename T>
    void Add(string property,const vector<T>&values){
        if(json!="") json+=",";
        json+="\""+property+"\":[";
        for(int k=0;k<values.size();k++){
            if(k==0)
               json+=ToString(values[k]);
             else
               json+=","+ToString(values[k]);
        }
        json+="]";
    };
    void AddWriter(string property,JsonWriter*el);
    void AddWritable(string property,JsonWritable*el);
    void WriteObject(string property,JsonWritable*el);
    void AddProperty(string property,string json);
    string GetJsonString();
    string GetInnerJsonString();
};


class SGIS_EXPORT JsonFile{
public:
    JsonFile();
    virtual~JsonFile();
    static bool SaveFile(string pathName,const vector<AutoPtr<JsonWritable>>bws);
    static vector<AutoPtr<JsonWritable>>ReadFile(string pathName);
};

}

#endif // JSONREADER_H
