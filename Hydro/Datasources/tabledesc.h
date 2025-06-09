#ifndef TABLEDESC_H
#define TABLEDESC_H

#include "Base/autoptr.h"
#include "Base/jsonio.h"
#include  "Base/bufferio.h"
#include "Base/variant.h"
#include "Base/Exp/ExpressionDataType.h"
#include "Base/classfactory.h"

namespace SGIS{

enum VectorFieldType
{
    vftInteger	= 0x0,
    vftReal	= 0x1,
    vftString	= 0x2,
    vftBinary	= 0x3,
    vftDate	= 0x4,
    vftTime	= 0x5,
    vftDateTime	= 0x6
};

struct FieldDesp{
    string name;
    enum VectorFieldType type;
    int width;
    int precision;
    FieldDesp(){
        name="";
        width=0;
        precision=0;
        type=vftInteger;
    };
    FieldDesp(string name, VectorFieldType type,int width,int precision){
        this->name=name;
        this->type=type;
        this->width=width;
        this->precision=precision;
    };
    FieldDesp(const FieldDesp&other){
        this->name=other.name;
        this->type=other.type;
        this->width=other.width;
        this->precision=other.precision;
    };
    FieldDesp&operator=(const FieldDesp&other){
        this->name=other.name;
        this->type=other.type;
        this->width=other.width;
        this->precision=other.precision;
        return *this;
    };
    void MakeInterger(string name,int width){
        this->name=name;
        this->type=vftInteger;
        this->width=width;
        this->precision=0;
    };
    void MakeReal(string name,int width,int precision){
        this->name=name;
        this->type=vftReal;
        this->width=width;
        this->precision=precision;
    };
    void MakeString(string name,int width){
        this->name=name;
        this->type=vftString;
        this->width=width;
        this->precision=0;
    };
};

class SGIS_EXPORT FieldTypeHelper{
public:
    static BYTE ToOGRFieldType(VectorFieldType fieldType);
    static VectorFieldType FromOGRFieldType(BYTE fieldType);
    static ExpressionDataType ToExpressionDataType(VectorFieldType fieldType);
    static VectorFieldType FromExpressionDataType(ExpressionDataType eType);
    static string GetFieldTypeString(VectorFieldType fieldType);
};

class SGIS_EXPORT TableDesc:
        public Object,
        public BufferWritable,
        public JsonWritable
{
public:
    TableDesc();
    virtual~TableDesc();
    int GetFieldCount();
    FieldDesp operator[](int nIndex);
    string GetFieldName(int nIndex);
    VectorFieldType GetFieldType(int nIndex);
    bool IsFieldNumeric(int nIndex);
    int GetFieldWidth(int nIndex);
    int GetFieldPrecision(int nIndex);
    AutoPtr<TableDesc>Clone();
    int FindField(string fieldName);
    void ValidFieldValue(int fieldIndex,Variant&value);
    string GetFirstNumericField();
    string GetBestStringField();
 //BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
 //JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    vector<FieldDesp>desps;
    friend class TableDescCreator;
    IndexedArray<string>pArray;
};

class SGIS_EXPORT TableDescCreator{
public:
    TableDescCreator();
    virtual~TableDescCreator();
    bool AddField(FieldDesp field);
    void Remove(int nIndex);
    int FindField(string fieldName);
    string GetUniqueFieldName(string fieldName);
    void FromTableDesc(TableDesc*pDesc);
    AutoPtr<TableDesc>CreateTableDesc();
    FieldDesp&GetField(int nIndex);
    void Clear();
    int GetFieldCount();
    bool MoveFieldUp(int nIndex);
    bool MoveFieldDown(int nIndex);
protected:
    vector<FieldDesp>desps;
};

}
#endif // TABLEDESC_H
