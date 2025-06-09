#ifndef VIRTUALFIELDS_H
#define VIRTUALFIELDS_H
#include "../base_globle.h"
#include "../variant.h"
#include "../datetime.h"
#include "ExpressionDataType.h"


namespace SGIS{


class SGIS_EXPORT VirtualTable{
public:
    VirtualTable();
    virtual~VirtualTable();
    ExpressionDataType GetFieldType(int nIndex);
    string GetTipName();
    void SetTipName(string tipName);
    int GetFieldCount();
    string GetFieldName(int nIndex);
    string GetFieldAliasName(int nIndex);
    int FindField(string Name);
    int GetUsingFieldCount();
    bool GetFieldUsing(int nIndex);
    int GetUsingFieldIndex(int nIndex);
    void LabelFieldUsing(int nIndex);
    void LabelAllFieldsUnused();
    void Reset();
    bool AddField(string FieldName,ExpressionDataType FieldType);
    bool AddField(string FieldName,ExpressionDataType FieldType,string AliasName);
    void SetFieldValue(int nIndex,Variant value);
    Variant GetFieldValue(int nIndex);
protected:
    vector<Variant>Values;
    vector<ExpressionDataType>Types;
    vector<bool>IsUsing;
    vector<string>Names;
    vector<string>AliasNames;
    string TipName;
};

class SGIS_EXPORT VirtualGrids{
public:
    VirtualGrids();
    virtual~VirtualGrids();
    ExpressionDataType GetGridType(int nIndex);
    FLOAT GetGridValue(int nIndex);
    string GetTipName();
    void SetTipName(string tipName);
    int GetGridCount();
    string GetGridName(int nIndex);
    string GetGridAliasName(int nIndex);
    int FindGrid(string Name);
    int GetUsingGridCount();
    bool GetGridUsing(int nIndex);
    int GetUsingGridIndex(int nIndex);
    void LabelGridUsing(int nIndex);
    void LabelAllGridsUnused();
    void Reset();
    bool AddGrid(string FieldName,bool IsFloat=true);
    bool AddGrid(string FieldName,string AliasName,bool IsFloat=true);
    void SetGridValue(int nIndex,FLOAT value);
protected:
    vector<FLOAT>Values;
    vector<ExpressionDataType>Types;
    vector<bool>IsUsing;
    vector<string>Names;
    vector<string>AliasNames;
    string TipName;
};

class SGIS_EXPORT ConstantValues{
public:
    ConstantValues();
    virtual~ConstantValues();
    int GetCount();
    ExpressionDataType GetConstantType(int nIndex);
    string GetConstantName(int nIndex);
    string GetConstantAliasName(int nIndex);
    int FindConstant(string Name);
    void Reset();
    bool AddConstant(string ConstName,ExpressionDataType ConstantType);
    bool AddConstant(string ConstName,ExpressionDataType ConstantType,string AliasName);
    void SetContantValue(int nIndex,Variant value);
    Variant GetConstantValue(int nIndex);
protected:
    vector<ExpressionDataType>Types;
    vector<string>Names;
    vector<string>AliasNames;
    vector<Variant>Values;
};

}

#endif // VIRTUALFIELDS_H
