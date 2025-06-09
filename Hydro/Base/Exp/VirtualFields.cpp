#include "VirtualFields.h"
#include "../StringHelper.h"

namespace SGIS{

VirtualTable::VirtualTable(){
    TipName="字段";
}

VirtualTable::~VirtualTable(){

}
ExpressionDataType VirtualTable::GetFieldType(int nIndex){
    return Types[nIndex];
}

string VirtualTable::GetTipName(){
    return TipName;
}
void VirtualTable::SetTipName(string tipName){
    TipName=tipName;
}

int VirtualTable::GetFieldCount(){
    return Names.size();
}


string VirtualTable::GetFieldName(int nIndex){
    return Names[nIndex];
}

string VirtualTable::GetFieldAliasName(int nIndex){
    return AliasNames[nIndex];
}


int VirtualTable::FindField(string sName){
    StringHelper::MakeUpper(sName);
    sName=StringHelper::Trim(sName);
    for(int k=Names.size()-1;k>=0;k--)
    {
        string name=Names[k];
        StringHelper::MakeUpper(name);
        name=StringHelper::Trim(name);
        if(sName==name)
        {
            return k;
        }
    }
    return -1;
}

int VirtualTable::GetUsingFieldCount(){
    int Num=0;
    for(int k=0;k<IsUsing.size();k++)
    {
        if(IsUsing[k]) Num++;
    }
    return Num;
}

bool VirtualTable::GetFieldUsing(int nIndex){
    return IsUsing[nIndex];
}

int VirtualTable::GetUsingFieldIndex(int nIndex){
    int Num=0;
    for(int k=0;k<IsUsing.size();k++)
    {
        if(IsUsing[k])
        {
            if(nIndex==Num)
            {
                return k;
            }
            Num++;
        }
    }
    return -1;
}

void VirtualTable::LabelFieldUsing(int nIndex){
    IsUsing[nIndex]=true;
}

void VirtualTable::LabelAllFieldsUnused(){
    for(int k=IsUsing.size()-1;k>=0;k--)
    {
        IsUsing[k]=false;
    }
}

void VirtualTable::Reset(){
    Types.clear();
    Names.clear();
    AliasNames.clear();
    IsUsing.clear();
    Values.clear();
}


bool VirtualTable::AddField(string FieldName,ExpressionDataType FieldType){
    return AddField(FieldName,FieldType,FieldName);
}

bool VirtualTable::AddField(string FieldName,ExpressionDataType FieldType,string AliasName){
    string Name=StringHelper::Trim(FieldName);
    if(Name.empty()) return false;
    int otherIndex=FindField(FieldName);
    if(otherIndex!=-1) return false;
    Types.push_back(FieldType);
    Names.push_back(Name);
    Variant oV;
    oV.vt=VRT_R8;
    oV.dblVal=0;
    switch(FieldType)
    {
    case exInt:
        oV.ChangeType(VRT_I4);
        break;
    case exNum:
        oV.ChangeType(VRT_R4);
        break;
    case exLog:
        oV.ChangeType(VRT_BOOL);
        break;
    case exStr:
        oV.ChangeType(VRT_BSTR);
        break;
    case exDat:
        oV.ChangeType(VRT_DATETIME);
        break;
    }
    Values.push_back(oV);
    AliasNames.push_back(AliasName);
    IsUsing.push_back(false);
    return true;
}

void VirtualTable::SetFieldValue(int nIndex,Variant value){
    Values[nIndex]=value;
}

Variant VirtualTable::GetFieldValue(int nIndex){
    return Values[nIndex];
}

VirtualGrids::VirtualGrids(){
    TipName="栅格";
}
VirtualGrids::~VirtualGrids(){

}

ExpressionDataType VirtualGrids::GetGridType(int nIndex){
    return exNum;
}

string VirtualGrids::GetTipName(){
    return TipName;
}
void VirtualGrids::SetTipName(string tipName){
    TipName=tipName;
}

int VirtualGrids::GetGridCount(){
    return Names.size();
}


string VirtualGrids::GetGridName(int nIndex){
    return Names[nIndex];
}

string VirtualGrids::GetGridAliasName(int nIndex){
    return AliasNames[nIndex];
}


int VirtualGrids::FindGrid(string sName){
    StringHelper::MakeUpper(sName);
    sName=StringHelper::Trim(sName);
    for(int k=Names.size()-1;k>=0;k--)
    {
        string name=Names[k];
        StringHelper::MakeUpper(name);
        name=StringHelper::Trim(name);
        if(sName==name)
        {
            return k;
        }
    }
    return -1;
}

int VirtualGrids::GetUsingGridCount(){
    int Num=0;
    for(int k=0;k<IsUsing.size();k++)
    {
        if(IsUsing[k]) Num++;
    }
    return Num;
}

bool VirtualGrids::GetGridUsing(int nIndex){
    return IsUsing[nIndex];
}

int VirtualGrids::GetUsingGridIndex(int nIndex){
    int Num=0;
    for(int k=0;k<IsUsing.size();k++)
    {
        if(IsUsing[k])
        {
            if(nIndex==Num)
            {
                return k;
            }
            Num++;
        }
    }
    return -1;
}

void VirtualGrids::LabelGridUsing(int nIndex){
    IsUsing[nIndex]=true;
}

void VirtualGrids::LabelAllGridsUnused(){
    for(int k=IsUsing.size()-1;k>=0;k--)
    {
        IsUsing[k]=false;
    }
}

bool VirtualGrids::AddGrid(string FieldName,bool IsFloat){
    string Name=StringHelper::Trim(FieldName);
    if(Name.empty()) return false;
    int otherIndex=FindGrid(FieldName);
    if(otherIndex!=-1) return false;
    Types.push_back((IsFloat?exNum:exInt));
    Names.push_back(Name);
    Values.push_back(0);
    AliasNames.push_back(Name);
    IsUsing.push_back(false);
    return true;
}

bool VirtualGrids::AddGrid(string FieldName,string AliasName,bool IsFloat){
    string Name=StringHelper::Trim(FieldName);
    if(Name.empty()) return false;
    int otherIndex=FindGrid(FieldName);
    if(otherIndex!=-1) return false;
    Types.push_back((IsFloat?exNum:exInt));
    Names.push_back(Name);
    Values.push_back(0);
    AliasNames.push_back(AliasName);
    IsUsing.push_back(false);
    return true;
}

void VirtualGrids::SetGridValue(int nIndex,FLOAT value){
    Values[nIndex]=value;
}

FLOAT VirtualGrids::GetGridValue(int nIndex){
    return Values[nIndex];
}


ConstantValues::ConstantValues(){

}

ConstantValues::~ConstantValues(){

}

int ConstantValues::GetCount(){
    return Names.size();
}
ExpressionDataType ConstantValues::GetConstantType(int nIndex){
    return Types[nIndex];
}
string ConstantValues::GetConstantName(int nIndex){
    return Names[nIndex];
}
string ConstantValues::GetConstantAliasName(int nIndex){

    return AliasNames[nIndex];
}
int ConstantValues::FindConstant(string Name){
    StringHelper::MakeUpper(Name);
    Name=StringHelper::Trim(Name);
    for(int k=Names.size()-1;k>=0;k--)
    {
        string name=Names[k];
        StringHelper::MakeUpper(name);
        name=StringHelper::Trim(name);
        if(Name==name)
        {
            return k;
        }
    }
    return -1;
}
void ConstantValues::Reset(){
    Types.clear();
    Names.clear();
    AliasNames.clear();
    Values.clear();
}
bool ConstantValues::AddConstant(string ConstName,ExpressionDataType ConstantType){
    return AddConstant(ConstName,ConstantType,ConstName);
}
bool ConstantValues::AddConstant(string ConstName,ExpressionDataType ConstantType,string AliasName){
    string Name=StringHelper::Trim(ConstName);
    if(Name.empty()) return false;
    int otherIndex=FindConstant(ConstName);
    if(otherIndex!=-1) return false;
    Types.push_back(ConstantType);
    Names.push_back(Name);
    Variant oV;
    oV.vt=VRT_R8;
    oV.dblVal=0;
    switch(ConstantType)
    {
    case exInt:
        oV.ChangeType(VRT_I4);
        break;
    case exNum:
        oV.ChangeType(VRT_R4);
        break;
    case exLog:
        oV.ChangeType(VRT_BOOL);
        break;
    case exStr:
        oV.ChangeType(VRT_BSTR);
        break;
    case exDat:
        oV.ChangeType(VRT_DATETIME);
        break;
    }
    Values.push_back(oV);
    AliasNames.push_back(AliasName);
    return true;
}
void ConstantValues::SetContantValue(int nIndex,Variant value){
    Values[nIndex]=value;
}
Variant ConstantValues::GetConstantValue(int nIndex){
    return Values[nIndex];
}

}
