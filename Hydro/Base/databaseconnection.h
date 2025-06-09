#ifndef DATABASECONNECTION_H
#define DATABASECONNECTION_H
#include "autoptr.h"
#include "StringHelper.h"
#include "datetime.h"
#include "variant.h"
#include "jsonio.h"
#include "bufferio.h"
#include "classfactory.h"

using namespace std;
class sqlite3;

namespace SGIS{

enum DatabaseFieldType {
    ftInteger=0,
    ftNumeric=1,
    ftString=2,
    ftBinary=3,
    ftDate=4,
    ftTime=5,
    ftDateTime=6,
    ftUnknown=7
};

template<typename T>
class SGIS_EXPORT DatabaseFieldTypeHelper{
public:
    DatabaseFieldTypeHelper(){};
    virtual~DatabaseFieldTypeHelper(){};
    static DatabaseFieldType DataType(){
        if (is_same<T, char>()) return ftInteger;
        if (is_same < T, unsigned char>()) return ftInteger;
        if (is_same < T, short>()) return ftInteger;
        if (is_same < T, unsigned short>()) return ftInteger;
        if ((is_same < T, int>()) || (is_same < T, long>())) return ftInteger;
        if ((is_same < T, unsigned long>()) || (is_same < T, unsigned long>())) return ftInteger;
        if (is_same < T, long long>()) return ftInteger;
        if (is_same < T, unsigned long long>()) return ftInteger;
        if (is_same < T, float>()) return ftNumeric;
        if (is_same < T, double>()) return ftNumeric;
        if (is_same < T, string>()) return ftString;
        return ftString;
    };
};

struct DatabaseField {
    string fieldName;
    DatabaseFieldType fieldType;
    DatabaseField() {
        fieldName = "";
        fieldType = ftUnknown;
    };
    DatabaseField(string name, DatabaseFieldType type) {
        this->fieldName = name;
        this->fieldType = type;
    };
    DatabaseField(const DatabaseField&other){
        this->fieldName = other.fieldName;
        this->fieldType = other.fieldType;
    };
    DatabaseField& operator =(const DatabaseField& other)
    {
        this->fieldName = other.fieldName;
        this->fieldType = other.fieldType;
        return *this;
    }
    bool IsFieldNumeric() {
        return ((fieldType == ftNumeric) || (fieldType == ftInteger));
    }
    bool IsFieldNumber() {
        return (fieldType == ftInteger);
    }
};

class SGIS_EXPORT DatabaseFields {
public:
    DatabaseFields();
    virtual~DatabaseFields();
    int FindField(string name);
    bool AddField(string fieldName,DatabaseFieldType type);
    bool AddField(DatabaseField field);
    int GetSize();
    void Remove(int nIndex);
    DatabaseField GetItem(int index);
protected:
    vector<DatabaseField>fields;
};

class SGIS_EXPORT Recordset {
public:
    Recordset();
    virtual~Recordset();
    virtual bool ResetReading()=0;
    virtual bool MoveNext() = 0;
    virtual string GetFieldValueAsString(int fieldIndex) = 0;
    virtual float GetFieldValueAsSingle(int fieldIndex) = 0;
    virtual double GetFieldValueAsDouble(int fieldIndex) = 0;
    virtual int GetFieldValueAsInteger(int fieldIndex) = 0;
    virtual DateTime GetFieldValueAsDateTime(int fieldIndex) = 0;
    virtual AutoPtr<DatabaseFields>GetFields() = 0;
    string ToJson();
    bool ToStream(BufferWriter&pWriter);
};

class SGIS_EXPORT DataColumn
{
public:
    DataColumn(DatabaseField type);
    virtual~DataColumn();
    DatabaseField GetField();
    void AddRecord(Variant value);
    void SetValue(int nIndex, Variant value);
    void AddRecord(string value);
    void SetValue(int nIndex, string value);
    void AddRecord(int value);
    void SetValue(int nIndex, int value);
    void AddRecord(double value);
    void SetValue(int nIndex, double value);
    void RemoveAt(int nIndex);
    void Clear();
    string GetFieldValueAsString(int nIndex);
    Variant GetFieldValue(int nIndex);
    DOUBLE GetFieldValueAsDouble(int nIndex);
    DateTime GetFieldValueAsDateTime(int nIndex);
    void SetSize(LONG Size);
protected:
    void ValidValue(Variant&value);
protected:
    DatabaseField field;
    vector<Variant>FieldValues;
    int currentPin;
    int maxNumPerBlock;
    int blockNum;
};

class SGIS_EXPORT DataTable :public Recordset{
public:
    DataTable(string name,vector<DatabaseField>columns);
    DataTable(string name,AutoPtr<DatabaseFields>columns);
    virtual~DataTable();
    static AutoPtr<DataTable>FromRecordSet(Recordset*rs,string name);
    static AutoPtr<DataTable>LoadFromStream(BufferReader*br);
    string GetName(); 
    void AddRecord();
    void AddRecord(const vector<Variant>&newValues);
    string GetFieldValueAsString(int recordIndex,int fieldIndex);
    double GetFieldValueAsDouble(int recordIndex,int fieldIndex);
    Variant GetFieldValue(int recordIndex,int fieldIndex);
    void SetFieldValueWithString(int recordIndex,int fieldIndex,string value);
    void SetFieldValue(int recordIndex,int fieldIndex,Variant value);
    void RemoveField(int nIndex);
    AutoPtr<DatabaseFields>GetFields();
    int GetRecordCount();
    bool SaveToCsv(string pathName);
    bool SaveToCsvText(string&content);
    static AutoPtr<DataTable>LoadFromCsv(string pathName);
    static AutoPtr<DataTable>LoadFromCsv(string pathName,int fromRecord,int recordCount);
    static AutoPtr<DatabaseFields>LoadHeaderFromCsv(string pathName);
//Recordset:
    bool ResetReading();
    bool MoveNext();
    string GetFieldValueAsString(int fieldIndex);
    float GetFieldValueAsSingle(int fieldIndex);
    double GetFieldValueAsDouble(int fieldIndex);
    int GetFieldValueAsInteger(int fieldIndex);
    DateTime GetFieldValueAsDateTime(int fieldIndex);
protected:
    vector<AutoPtr<DataColumn>>columns;
    int recordNumber;
    string name;
    int curpin;
};


class SGIS_EXPORT DataConnection:
        public BufferWritable,
        public JsonWritable{
public:
    DataConnection(string name="");
    virtual~DataConnection();
    string GetName();
    virtual bool GetConnected()=0;
    virtual bool Connect() = 0;
    virtual void Disconnect()=0;
protected:
    string name;
};

class SGIS_EXPORT DatabaseConnection :
   public DataConnection
{
public:
    DatabaseConnection(string name="");
    virtual~DatabaseConnection();
    virtual bool Execute(string command) = 0;
    virtual AutoPtr<Recordset>Select(string sql)=0;
    bool Exists(string sql);
    bool GetFirstFieldValueOfFirstRecord(string sql,string&value);
};

class SGIS_EXPORT SqliteRecordset :public Recordset {
protected:
    SqliteRecordset(int Rows, int Cols, char**Contents);
public:
    virtual~SqliteRecordset();
    bool ResetReading();
    bool MoveNext();
    string GetFieldValueAsString(int fieldIndex);
    float GetFieldValueAsSingle(int fieldIndex);
    double GetFieldValueAsDouble(int fieldIndex);
    int GetFieldValueAsInteger(int fieldIndex);
    DateTime GetFieldValueAsDateTime(int fieldIndex);
    AutoPtr<DatabaseFields>GetFields();
    friend class SqliteDatabaseConnection;
protected:
    int rows;
    int cols;
    char**contents;
    int currenPin;
};

class SGIS_EXPORT SqliteDatabaseConnection :public DatabaseConnection{
public:
    SqliteDatabaseConnection(string name="");
    SqliteDatabaseConnection(string name,string path);
    virtual~SqliteDatabaseConnection();
    bool GetConnected();
    string GetPathName();
    void Initialize(string pathName);
    bool Connect();
    void Disconnect();
    bool Execute(string command);
    AutoPtr<Recordset>Select(string sql);
    string GetTypeName();
//BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    string pathName;
    sqlite3 * db;
};

class SGIS_EXPORT DataConnections:
        public BufferWritable,
        public JsonWritable{
public:
    DataConnections();
    virtual~DataConnections();
    string GetName();
    int GetSize();
    AutoPtr<DataConnection>GetItem(int nIndex);
    template<typename T>
    AutoPtr<T>GetDataConnection(){
        for(int k=0;k<dses.size();k++){
            AutoPtr<T>ds=dses[k];
            if(ds!=nullptr) return ds;
        }
        return nullptr;
    };
    void Remove(int nIndex);
    bool AddDataConnection(AutoPtr<DataConnection>ds);
    AutoPtr<DataConnection>Find(string name);
    void Clear();
    AutoPtr<DataConnections>Clone();
//BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    vector<AutoPtr<DataConnection>>dses;
};

class SGIS_EXPORT SqlHelper{
public:
    static string CreateInsertSql(const vector<Variant>&values);
    static string CreateInsertSql(const string&tableName,const vector<Variant>&values);
    static string CreateInsertSqlWithFields(const string&tableName,const vector<std::tuple<string,Variant>>&values,string fieldPrefix="");
    static string CreateUpdateSql(const string&tableName,const vector<std::tuple<string,Variant>>&values,string fieldPrefix="");
    static int GetRecordCount(DatabaseConnection*conn,const vector<string>&tables,const string&whereClouse,bool allTableExists=false);
    static AutoPtr<Recordset>GetPageRecordset(DatabaseConnection*conn,const vector<string>&tables,const string&fields,const string&whereClouse,const string&otherClause,int fromRecord,int recordNum,bool allTableExists=false);
    static AutoPtr<Recordset>GetPageRecordsetEx(DatabaseConnection*conn,const vector<string>&tables,const string&fields,const string&whereClouse,const string&otherClause,int&totalRecords,int fromRecord,int recordNum,bool allTableExists=false);
    static AutoPtr<Recordset>GetRecordset(DatabaseConnection*conn,const vector<string>&tables,const string&fields,const string&whereClouse,const string&otherClause,bool allTableExists=false);
};

class SGIS_EXPORT SqlInsertTool{
public:
     SqlInsertTool(DatabaseConnection*conn,string tableName,const vector<string>&fields,int bufferCount=50);
     ~SqlInsertTool();
     bool Insert(const vector<Variant>&values);
protected:
     string insertSql;
     string sql;
     string fields;
     int bufferCount;
     DatabaseConnection*conn;
     int curPin;
};


};
#endif // DATABASECONNECTION_H
