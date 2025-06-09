#include "jsonio.h"
#include "file.h"
#include "classfactory.h"
#include "StringHelper.h"

namespace SGIS{

JsonNode::JsonNode(Json::Value jsnode):
    node(jsnode)
{
    lpszPathName="";
}


JsonNode::JsonNode(const JsonNode&other):
    node(other.node){
    lpszPathName="";
}


JsonNode::~JsonNode(){

}

string&JsonNode::PathName(){
    return lpszPathName;
}

vector<string>JsonNode::getMemberNames(){
    return node.getMemberNames();
}

JsonNode&JsonNode::operator =(const JsonNode&other){
    this->node=other.node;
    this->lpszPathName=other.lpszPathName;
    return (*this);
}
UINT JsonNode::asUInt(){
    return node.asUInt();
}

bool JsonNode::Empty(){
    return node.empty();
}

bool JsonNode::asBool(){
    if(node.empty()) return false;
    return node.asBool();
}

int JsonNode::asInt(){
    if(node.empty()) return 0;
    return node.asInt();
}

int JsonNode::asInt64(){
    if(node.empty()) return 0;
    return node.asInt64();
}

ULONGLONG JsonNode::asUint64(){
    if(node.empty()) return 0;
    return node.asUInt64();
}

string JsonNode::asString(){
    if(node.empty()) return "";
    return node.asString();
}

float JsonNode::asFloat(){
    if(node.empty()) return 0;
    return node.asFloat();
}

double JsonNode::asDouble(){
    if(node.empty()) return 0;
    return node.asDouble();
}

int JsonNode::size(){
    return node.size();
}

JsonValueType JsonNode::ValueType(){
    return (JsonValueType)node.type();
}

JsonNode JsonNode::operator [](int nIndex){
    JsonNode childNode=JsonNode(node[nIndex]);
    childNode.PathName()=lpszPathName;
    return childNode;
}

JsonNode JsonNode::operator [](string name){
    JsonNode childNode=JsonNode(node[name]);
    childNode.PathName()=lpszPathName;
    return childNode;
}


map<string,AutoPtr<JsonNode>>JsonNode::Children(){
    Json::ValueIterator iter;
    map<string,AutoPtr<JsonNode>>children;
    for(iter=node.begin();iter!=node.end();++iter){
        string name=iter.name();
        Json::Value value=iter.key();
        AutoPtr<JsonNode>jn(new JsonNode(node[name]));
        children[name]=jn;
    }
    return children;
}

JsonReader::JsonReader()
{

}
JsonReader::~JsonReader()
{

}

string&JsonReader::PathName(){
    return pathName;
}

AutoPtr<JsonNode>JsonReader::Parse(const string&jsonStr){
    Json::Value rv;
    if(!jsonReader.parse(jsonStr, rv)) return nullptr;
    AutoPtr<JsonNode>root(new JsonNode(rv));
    return root;
}

AutoPtr<JsonNode>JsonReader::Load(const string&jsonFile){
    string sJson;
    if(!TextFile::Read(jsonFile,sJson)) return nullptr;
    return Parse(sJson);
}

AutoPtr<JsonWritable>JsonReader::ReadObject(JsonReader*jr,JsonNode node){
    if(node.Empty()){
        return nullptr;
    }
    else{
        string typeName=node["typename"].asString();
        AutoPtr<JsonWritable>jwr=ClassFactory::getInstance(typeName);
        jwr->ReadFromJson(jr,node["object"]);
        return jwr;
    }
}

bool JsonReader::GetBoolValue(JsonNode node){
    if(node.Empty()) return false;
    string value=node.asString();
    StringHelper::TrimToLower(value);
    if((value=="true")||(value=="t")||(value=="yes")||(value=="y")||(value=="1")) return true;
    return false;
}

JsonWriter::JsonWriter(){
    json="";
}
JsonWriter::~JsonWriter(){

}

string JsonWriter::ToString(string value){
   return "\""+value+"\"";
}
string JsonWriter::ToString(DateTime value){
   return "\""+value.ToString(tstFull)+"\"";
}
string JsonWriter::ToString(int value){
   return to_string(value);
}
string JsonWriter::ToString(double value){
   return to_string(value);
}
string JsonWriter::ToString(bool value){
   return (value?"true":"false");
}

JsonArrayWriter::JsonArrayWriter(){

}

JsonArrayWriter::JsonArrayWriter(const vector<Variant>&values){
    for(int k=0;k<values.size();k++){
        VarType vt=values[k].vt;
        switch(vt){
        case VRT_R8:
            this->Add<double>(values[k].dblVal);
            break;
        case VRT_R4:
            this->Add<float>(values[k].fltVal);
            break;
        case VRT_BSTR:
            this->Add<string>(*values[k].bstrVal);
            break;
        case VRT_I4:
            this->Add<int>(values[k].lVal);
            break;
        case VRT_UI4:
            this->Add<int>(values[k].ulVal);
            break;
        case VRT_I1:
            this->Add<char>(values[k].cVal);
            break;
        case VRT_UI1:
            this->Add<BYTE>(values[k].bVal);
            break;
        case VRT_I2:
            this->Add<short>(values[k].iVal);
            break;
        case VRT_UI2:
            this->Add<USHORT>(values[k].uiVal);
            break;
        case VRT_DATETIME:
        case VRT_DATE:{
            DateTime dt(values[k].date);
            this->Add<string>(dt.ToFullString(tstFull));
            break;
        }
        }
    }
}


JsonArrayWriter::~JsonArrayWriter(){

}

string JsonArrayWriter::GetJsonString(){
   return "["+json+"]";
}

void JsonArrayWriter::AddWriter(JsonWriter*el){
    if(el==nullptr) return;
    if(json!="") json+=",";
    json+=el->GetJsonString();
}
void JsonArrayWriter::AddWritable(JsonWritable*el){
    if(el==nullptr) return;
    if(json!="") json+=",";
    json+=el->ToJson();
}

void JsonArrayWriter::AddWriter(string property,JsonWriter*el){
    if(el==nullptr) return;
    if(json!="") json+=",";
    json+="{\""+property+"\":"+el->GetJsonString()+"}";
}
void JsonArrayWriter::AddWritable(string property,JsonWritable*el){
    if(el==nullptr) return;
    if(json!="") json+=",";
    json+="{\""+property+"\":"+el->ToJson()+"}";
}

void JsonArrayWriter::AddProperty(string property,string json){
    if(json=="") return;
    if(json!="") json+=",";
    json+="{\""+property+"\":"+json+"}";
}

void JsonArrayWriter::WriteObject(JsonWritable*jwr){
    if(jwr==nullptr) return;
    AutoPtr<JsonObjectWriter>jow=new JsonObjectWriter();
    jow->Add<string>("typename",jwr->GetTypeName());
    jow->AddWritable("object",jwr);
    if(json!="") json+=",";
    json+=jow->GetJsonString();
}

JsonObjectWriter::JsonObjectWriter(string baseJson){
    json=baseJson;
}


JsonObjectWriter::JsonObjectWriter(const vector<tuple<string,Variant>>&values,string baseJson){
    json=baseJson;
    for(int k=0;k<values.size();k++){
        string props=get<0>(values[k]);
        Variant vr=get<1>(values[k]);
        VarType vt=vr.vt;
        switch(vt){
        case VRT_R8:
            this->Add<double>(props,vr.dblVal);
            break;
        case VRT_R4:
            this->Add<float>(props,vr.fltVal);
            break;
        case VRT_BSTR:
            this->Add<string>(props,*vr.bstrVal);
            break;
        case VRT_I4:
            this->Add<int>(props,vr.lVal);
            break;
        case VRT_UI4:
            this->Add<int>(props,vr.ulVal);
            break;
        case VRT_I1:
            this->Add<char>(props,vr.cVal);
            break;
        case VRT_UI1:
            this->Add<BYTE>(props,vr.bVal);
            break;
        case VRT_I2:
            this->Add<short>(props,vr.iVal);
            break;
        case VRT_UI2:
            this->Add<USHORT>(props,vr.uiVal);
            break;
        case VRT_DATETIME:
        case VRT_DATE:{
            DateTime dt(vr.date);
            this->Add<string>(props,dt.ToFullString(tstFull));
            break;
        }
        }
    }
}


JsonObjectWriter::~JsonObjectWriter(){

}

void JsonObjectWriter::AddWriter(string property,JsonWriter*el){
    if(el==nullptr) return;
    if(json!="") json+=",";
    json+="\""+property+"\":"+el->GetJsonString();
}
void JsonObjectWriter::AddWritable(string property,JsonWritable*el){
    if(el==nullptr) return;
    if(json!="") json+=",";
    json+="\""+property+"\":"+el->ToJson();
}

void JsonObjectWriter::WriteObject(string property,JsonWritable*jwr){
    if(jwr==nullptr) return;
    AutoPtr<JsonObjectWriter>jow=new JsonObjectWriter();
    jow->Add<string>("typename",jwr->GetTypeName());
    jow->AddWritable("object",jwr);
    if(json!="") json+=",";
    json+="\""+property+"\":"+jow->GetJsonString();
}

void JsonObjectWriter::AddProperty(string property,string json){
    if(json=="") return;
    if(this->json!="") this->json+=",";
    this->json+="\""+property+"\":"+json;
}


string JsonObjectWriter::GetInnerJsonString(){
    return json;
}
string JsonObjectWriter::GetJsonString(){
   return "{"+json+"}";
}

JsonFile::JsonFile(){

}

JsonFile::~JsonFile(){

}

bool JsonFile::SaveFile(string pathName,const vector<AutoPtr<JsonWritable>>bws){
    AutoPtr<JsonArrayWriter>bw(new JsonArrayWriter());
    for(int k=0;k<bws.size();k++){
        bw->WriteObject(bws[k].get());
    }
    string sJson=bw->GetJsonString();
    return TextFile::Write(pathName,sJson);
}

vector<AutoPtr<JsonWritable>>JsonFile::ReadFile(string pathName){
    AutoPtr<JsonReader>jr(new JsonReader());
    AutoPtr<JsonNode>rootNode=jr->Load(pathName);
    if(rootNode==nullptr) return {};
    if(rootNode->ValueType()!=jvtArray) return {};
    vector<AutoPtr<JsonWritable>>jws;
    for(int k=0;k<rootNode->size();k++){
        JsonNode childNode=(*rootNode)[k];
        AutoPtr<JsonWritable>jw=JsonReader::ReadObject(jr.get(),childNode);
        jws.push_back(jw);
    }
    return jws;
}

}
