#ifndef VARIANT_H
#define VARIANT_H
#include "base_globle.h"
#include "datetime.h"

#define VarType unsigned short
#define VRT_EMPTY 0
#define VRT_I2 2
#define VRT_I4 3
#define VRT_R4 4
#define VRT_R8 5
#define VRT_DATE 6
#define VRT_DATETIME 7
#define VRT_BSTR 8
#define VRT_BOOL 11
#define VRT_I1 16
#define VRT_UI1 17
#define VRT_UI2 18
#define VRT_UI4 19
#define VRT_I8 20
#define VRT_UI8 21
#define VRT_PIN 22

namespace SGIS{

struct SGIS_EXPORT Variant
{
    VarType vt;
    union
    {
        ULONGLONG ullVal;
        LONGLONG llVal;
        ULONG ulVal;
        LONG lVal;
        USHORT uiVal;
        SHORT iVal;
        BYTE bVal;
        CHAR cVal;
        FLOAT fltVal;
        DOUBLE dblVal;
        bool boolVal;
        DATE date;
        string*bstrVal;
        void*pData;
    };
    Variant();
    Variant(const ULONGLONG&v);
    Variant(const LONGLONG&v);
    Variant(const ULONG&v);
    Variant(const LONG&v);
    Variant(const int&v);
    Variant(const USHORT&v);
    Variant(const SHORT&v);
    Variant(const BYTE&v);
    Variant(const CHAR&v);
    Variant(const bool&v);
    Variant(const FLOAT&v);
    Variant(const DOUBLE&v);
    Variant(const string&v);
    Variant(Date dt);
    Variant(DateTime dt);
    Variant(void*other);
    Variant(const Variant&other);
    ~Variant();
    void Clear();
    Variant& operator =(const ULONGLONG& other);
    Variant& operator =(const LONGLONG& other);
    Variant& operator =(const ULONG& other);
    Variant& operator =(const LONG& other);
    Variant& operator =(const int& other);
    Variant& operator =(const USHORT& other);
    Variant& operator =(const SHORT& other);
    Variant& operator =(const BYTE& other);
    Variant& operator =(const CHAR& other);
    Variant& operator =(const bool& other);
    Variant& operator =(const FLOAT& other);
    Variant& operator =(const DOUBLE& other);
    Variant& operator =(const string& other);
    Variant& operator =(DateTime other);
    Variant& operator =(Date other);
    Variant& operator =(void*other);
    Variant& operator =(const Variant& other);

    bool asBool();
    char asChar();
    BYTE asByte();
    SHORT asShort();
    SHORT asUShort();
    int asInt();
    UINT asUInt();
    ULONGLONG asULong();
    LONGLONG asLong();
    float asSingle();
    double asDouble();
    Date asDate();
    DateTime asDateTime();
    string asString();
    string ToSqlString();
    bool ChangeType(VarType vt);
};

}

#endif // VARIANT_H
