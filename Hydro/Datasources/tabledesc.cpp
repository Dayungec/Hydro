#include "tabledesc.h"
#include "Base/StringHelper.h"

namespace SGIS{

REGISTER(TableDesc)

BYTE FieldTypeHelper::ToOGRFieldType(VectorFieldType fieldType){
    switch(fieldType){
    case vftInteger:return 0;
    case vftReal:return 2;
    case vftString:return 4;
    case vftBinary:return 8;
    case vftDate:return 9;
    case vftTime:return 10;
    case vftDateTime:return 11;
    }
    return 0;
}

VectorFieldType FieldTypeHelper::FromOGRFieldType(BYTE fieldType){
   switch(fieldType){
    case 0:
    case 12:
    case 1:
    case 13:
       return VectorFieldType::vftInteger;
   case 2:
   case 3:
       return VectorFieldType::vftReal;
   case 4:
   case 5:
   case 6:
   case 7:
       return VectorFieldType::vftString;
   case 8:
       return VectorFieldType::vftBinary;
   case 11:
       return VectorFieldType::vftDateTime;
   case 9:
       return VectorFieldType::vftDate;
   case 10:
       return VectorFieldType::vftTime;
   }
   return VectorFieldType::vftInteger;
}

ExpressionDataType FieldTypeHelper::ToExpressionDataType(VectorFieldType fieldType){
    ExpressionDataType dt=ExpressionDataType::exNul;
    switch(fieldType)
    {
    case vftInteger:
        dt=exInt;
        break;
    case vftReal:
        dt=exNum;
        break;
    case vftString:
    case vftBinary:
        dt=exStr;
        break;
    case vftDate:
    case vftTime:
    case vftDateTime:
        dt=exDat;
        break;
    }
    return dt;
}
VectorFieldType FieldTypeHelper::FromExpressionDataType(ExpressionDataType eType){
    switch(eType){
     case ExpressionDataType::exInt:
        return VectorFieldType::vftInteger;
    case ExpressionDataType::exNum:
        return VectorFieldType::vftReal;
    case ExpressionDataType::exStr:
        return VectorFieldType::vftString;
    case ExpressionDataType::exDat:
        return VectorFieldType::vftDateTime;
    }
    return VectorFieldType::vftInteger;
}

string FieldTypeHelper::GetFieldTypeString(VectorFieldType fieldType){
    switch(fieldType)
    {
    case vftInteger:
        return "Integer";
    case vftReal:
        return "Real";
    case vftString:
        return "String";
    case vftBinary:
        return "Binary";
    case vftDate:
        return "Date";
    case vftTime:
        return "Time";
    case vftDateTime:
        return "DateTime";
    }
    return "";
}

TableDesc::TableDesc()
{

}
TableDesc::~TableDesc()
{

}

FieldDesp TableDesc::operator[](int nIndex){
    return desps[nIndex];
}
int TableDesc::GetFieldCount(){
    return desps.size();
}

string TableDesc::GetFieldName(int nIndex){
    return desps[nIndex].name;
}

VectorFieldType TableDesc::GetFieldType(int nIndex){
    return desps[nIndex].type;
}

bool TableDesc::IsFieldNumeric(int nIndex){
    VectorFieldType vft=desps[nIndex].type;
    return ((vft==VectorFieldType::vftInteger)||(vft==VectorFieldType::vftReal));
}

int TableDesc::GetFieldWidth(int nIndex){
    return desps[nIndex].width;
}

int TableDesc::GetFieldPrecision(int nIndex){
    return desps[nIndex].precision;
}

AutoPtr<TableDesc>TableDesc::Clone(){
    TableDesc*desc=new TableDesc();
    int fcount=desps.size();
    desc->desps.resize(fcount);
    for(int k=0;k<fcount;k++){
        desc->desps[k]=desps[k];
        desc->pArray.Add(pArray[k]);
    }
    return desc;
}

int TableDesc::FindField(string fieldName){
    StringHelper::TrimToLower(fieldName);
    LONG nIndex=pArray.FindValueIndex(fieldName);
    return nIndex;
}

void TableDesc::ValidFieldValue(int fieldIndex,Variant&value){
    VectorFieldType fType=desps[fieldIndex].type;
    switch(fType)
    {
    case vftInteger:
        {
            value.ChangeType(VRT_I4);
            break;
        }
    case vftReal:
        {
            value.ChangeType(VRT_R4);
            break;
        }
    case vftString:
        {
            value.ChangeType(VRT_BSTR);
            LONG w=desps[fieldIndex].width;
            int l=value.bstrVal->length();
            if(l>w)
            {
                *value.bstrVal=value.bstrVal->substr(0,w);
            }
            else if(w==0)
            {
                *value.bstrVal="";
            }
            break;
        }
    case vftDate:
        value.ChangeType(VRT_DATE);
        break;
    case vftTime:
    case vftDateTime:
        value.ChangeType(VRT_DATETIME);
        break;
    }
}

string TableDesc::GetFirstNumericField(){
    for(int k=0;k<desps.size();k++){
        if((desps[k].type==VectorFieldType::vftInteger)||(desps[k].type==VectorFieldType::vftReal)){
            return desps[k].name;
        }
    }
    return "";
}

string TableDesc::GetBestStringField(){
    for(int k=0;k<desps.size();k++){
        if(desps[k].type==VectorFieldType::vftString){
            return desps[k].name;
        }
    }
    if(desps.size()==0) return "";
    return desps[0].name;
}

bool TableDesc::Read(BufferReader*br){
    int fcount=br->Read<int>();
    desps.clear();
    pArray.Clear();
    for(int k=0;k<fcount;k++){
        FieldDesp desp;
        desp.name=br->ReadString();
        desp.type=(VectorFieldType)br->Read<int>();
        desp.width=br->Read<int>();
        desp.precision=br->Read<int>();
        if(desp.name.empty()) continue;
        if(FindField(desp.name)>=0) continue;
        desps.push_back(desp);
        string name=desp.name;
        StringHelper::TrimToLower(name);
        pArray.Add(name);
    }
    return true;
}
void TableDesc::Write(BufferWriter*bw){
    bw->Write<int>(desps.size());
    for(int k=0;k<desps.size();k++){
        FieldDesp desp=desps[k];
        bw->WriteString(desp.name);
        bw->Write<int>((int)desp.type);
        bw->Write<int>(desp.width);
        bw->Write<int>(desp.precision);
    }
}

string TableDesc::GetTypeName(){
    return "TableDesc";
}

bool TableDesc::ReadFromJson(JsonReader*jr,JsonNode node){
    int fcount=node.size();
    desps.clear();
    pArray.Clear();
    for(int k=0;k<fcount;k++){
        JsonNode childNode=node[k];
        FieldDesp desp;
        desp.name=childNode["name"].asString();
        desp.type=(VectorFieldType)childNode["type"].asInt();
        desp.width=childNode["width"].asInt();
        desp.precision=childNode["precision"].asInt();
        if(desp.name.empty()) continue;
        if(FindField(desp.name)>=0) continue;
        desps.push_back(desp);
        string name=desp.name;
        StringHelper::TrimToLower(name);
        pArray.Add(name);
    }
    return true;
}
string TableDesc::ToJson(){
    JsonArrayWriter writer;
    int fcount=desps.size();
    for(int k=0;k<fcount;k++){
       FieldDesp desp=desps[k];
       JsonObjectWriter subw;
       subw.Add("name",desp.name);
       subw.Add("type",(int)desp.type);
       subw.Add("width",desp.width);
       subw.Add("precision",desp.precision);
       writer.AddWriter(&subw);
    }
    return writer.GetJsonString();
}

TableDescCreator::TableDescCreator(){

}
TableDescCreator::~TableDescCreator(){

}

bool TableDescCreator::AddField(FieldDesp field){
    string name=field.name;
    StringHelper::TrimToLower(name);
    if(name.empty()) return false;
    for(int k=0;k<desps.size();k++){
        string fieldName=desps[k].name;
        StringHelper::TrimToLower(fieldName);
        if(name==fieldName) return false;
    }
    desps.push_back(field);
    return true;
}

int TableDescCreator::FindField(string fieldName){
    StringHelper::TrimToLower(fieldName);
    for(int k=0;k<desps.size();k++){
        string fieldName2=desps[k].name;
        StringHelper::TrimToLower(fieldName2);
        if(fieldName==fieldName2) return k;
    }
    return -1;
}

string TableDescCreator::GetUniqueFieldName(string fieldName){
    string fn=fieldName;
    int num=0;
    while(true){
        if(FindField(fn)<0) return fn;
        num++;
        fn=fieldName+"_"+to_string(num);
    }
    return "";
}

int TableDescCreator::GetFieldCount(){
    return desps.size();
}

bool TableDescCreator::MoveFieldUp(int nIndex){
    if(nIndex<1) return false;
    FieldDesp tempDesp=desps[nIndex];
    desps[nIndex]=desps[nIndex-1];
    desps[nIndex-1]=tempDesp;
    return true;
}
bool TableDescCreator::MoveFieldDown(int nIndex){
    if(nIndex>=desps.size()-1) return false;
    FieldDesp tempDesp=desps[nIndex];
    desps[nIndex]=desps[nIndex+1];
    desps[nIndex+1]=tempDesp;
    return true;
}

void TableDescCreator::FromTableDesc(TableDesc*pDesc){
    desps.clear();
    LONG Count=pDesc->GetFieldCount();
    for(int k=0;k<Count;k++)
    {
        FieldDesp fd=(*pDesc)[k];
        AddField(fd);
    }
}
void TableDescCreator::Remove(int nIndex){
    desps.erase(begin(desps)+nIndex);
}
AutoPtr<TableDesc>TableDescCreator::CreateTableDesc(){
    TableDesc*desc=new TableDesc();
    desc->desps.resize(desps.size());
    for(int k=0;k<desps.size();k++){
        desc->desps[k]=desps[k];
        string fieldName2=desps[k].name;
        StringHelper::TrimToLower(fieldName2);
        desc->pArray.Add(fieldName2);
    }
    return desc;
}

FieldDesp&TableDescCreator::GetField(int nIndex){
    return desps[nIndex];
}

void TableDescCreator::Clear(){
    desps.clear();
}

}
