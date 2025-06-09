#include "databaseconnection.h"
#include "Sqlite/sqlite3.h"
#include "StringHelper.h"
#include "file.h"
#include "FilePath.h"

namespace SGIS{

REGISTER(SqliteDatabaseConnection)
REGISTER(DataConnections)

DatabaseFields::DatabaseFields() {

}
DatabaseFields::~DatabaseFields() {

}
int DatabaseFields::FindField(string name) {
    name = StringHelper::Trim(name);
    StringHelper::MakeLower(name);
    string fname=StringHelper::Trim(name);
    for (int k = fields.size() - 1; k >= 0; k--) {
        string oname = fields[k].fieldName;
        oname = StringHelper::Trim(oname);
        StringHelper::MakeLower(oname);
        if (fname == oname) return k;
    }
    return -1;
}
bool DatabaseFields::AddField(string fieldName,DatabaseFieldType type) {
    if (FindField(fieldName) >= 0) return false;
    fields.push_back(DatabaseField(fieldName,type));
    return true;
}
bool DatabaseFields::AddField(DatabaseField field) {
    if (FindField(field.fieldName) >= 0) return false;
    fields.push_back(field);
    return true;
}
int DatabaseFields::GetSize() {
    return fields.size();
}
void DatabaseFields::Remove(int nIndex) {
    fields.erase(std::begin(fields) + nIndex);
}
DatabaseField DatabaseFields::GetItem(int index) {
    return fields[index];
}

Recordset::Recordset(){

}
Recordset::~Recordset(){

}

string Recordset::ToJson(){
    AutoPtr<DatabaseFields>fields=GetFields();
    vector<string>fieldNames;
    vector<DatabaseFieldType>fieldTypes;
    JsonObjectWriter jow;
    JsonArrayWriter faow;
    int fieldCount = fields->GetSize();
    for (int k = 0; k < fieldCount; k++) {
        JsonObjectWriter fow;
        DatabaseField field = fields->GetItem(k);
        string fieldName = field.fieldName;
        DatabaseFieldType dte = field.fieldType;
        fieldNames.push_back(fieldName);
        fieldTypes.push_back(dte);
        fow.Add<string>("Name",fieldName);
        fow.Add<int>("FieldType",(int)dte);
        faow.AddWriter(&fow);
    }
    jow.AddWriter("fields",&faow);

    JsonArrayWriter pWriter;
    this->ResetReading();
    while(this->MoveNext()){
        JsonObjectWriter recordWriter;
        for (int k = 0; k < fieldCount; k++) {
            switch(fieldTypes[k]){
            case DatabaseFieldType::ftInteger:
                recordWriter.Add<int>(fieldNames[k],this->GetFieldValueAsInteger(k));
                break;
            case DatabaseFieldType::ftNumeric:
                recordWriter.Add<double>(fieldNames[k],this->GetFieldValueAsDouble(k));
                break;
            default:{
                recordWriter.Add<string>(fieldNames[k],this->GetFieldValueAsString(k));
                break;
            }
            }
        }
        pWriter.AddWriter(&recordWriter);
    }
    jow.AddWriter("records",&pWriter);
    return jow.GetJsonString();
}

bool Recordset::ToStream(BufferWriter&pWriter){
    AutoPtr<DatabaseFields>fields=GetFields();
    vector<string>fieldNames;
    vector<DatabaseFieldType>fieldTypes;
    int fieldCount = fields->GetSize();
    pWriter.Write<int>(fieldCount);
    for (int k = 0; k < fieldCount; k++) {
        DatabaseField field = fields->GetItem(k);
        string fieldName = field.fieldName;
        DatabaseFieldType dte = field.fieldType;
        fieldNames.push_back(fieldName);
        fieldTypes.push_back(dte);
    }
    for (int k = 0; k < fieldCount; k++) {
         pWriter.WriteString(fieldNames[k]);
         pWriter.Write<int>((int)fieldTypes[k]);
    }
    int buf=pWriter.GetBufferLength();
    pWriter.Write<int>(0);
    this->ResetReading();
    int recordNum=0;
    while(this->MoveNext()){
        for (int k = 0; k < fieldCount; k++) {
            switch(fieldTypes[k]){
            case DatabaseFieldType::ftInteger:
                pWriter.Write<int>(this->GetFieldValueAsInteger(k));
                break;
            case DatabaseFieldType::ftNumeric:
                pWriter.Write<double>(this->GetFieldValueAsDouble(k));
                break;
            default:{
                pWriter.WriteString(this->GetFieldValueAsString(k));
                break;
            }
            }
        }
        recordNum++;
    }
    char*buffer=pWriter.GetBuffer();
    memcpy(buffer+buf,&recordNum,sizeof(int));
    return true;
}

DataColumn::DataColumn(DatabaseField field)
{
    this->field=field;
    currentPin = 0;
    maxNumPerBlock = 1000;
    blockNum = 1;
    FieldValues.resize(blockNum*maxNumPerBlock);
}
DataColumn::~DataColumn()
{
    Clear();
}

DatabaseField DataColumn::GetField(){
    return field;
}

void DataColumn::ValidValue(Variant&value)
{
    switch (field.fieldType)
    {
    case ftInteger:
    {
        value.ChangeType(VRT_I4);
        break;
    }
    case ftNumeric:
    {
        value.ChangeType(VRT_R8);
        break;
    }
    case ftDate:
    {
        value.ChangeType(VRT_DATE);
        break;
    }
    case ftString:{
        value.ChangeType(VRT_BSTR);
        break;
    }
    case ftTime:
    case ftDateTime:
    {
        value.ChangeType(VRT_DATETIME);
        break;
    }
    }
}

void DataColumn::AddRecord(Variant value)
{
    ValidValue(value);
    currentPin++;
    if (currentPin >= maxNumPerBlock * blockNum)
    {
        FieldValues.resize(currentPin + maxNumPerBlock);
        blockNum++;
    }
    FieldValues[currentPin - 1] = value;
}

void DataColumn::SetValue(int nIndex, Variant value)
{
    ValidValue(value);
    FieldValues[nIndex] = value;
}

void DataColumn::AddRecord(string value)
{
    Variant fv(value);
    ValidValue(fv);
    currentPin++;
    if (currentPin >= maxNumPerBlock * blockNum)
    {
        FieldValues.resize(currentPin + maxNumPerBlock);
        blockNum++;
    }
    FieldValues[currentPin - 1] = fv;
}

void DataColumn::SetValue(int nIndex, string value)
{
    Variant fv(value);
    ValidValue(fv);
    FieldValues[nIndex] = fv;
}

void DataColumn::AddRecord(int value)
{
    Variant fv(value);
    ValidValue(fv);
    currentPin++;
    if (currentPin >= maxNumPerBlock * blockNum)
    {
        FieldValues.resize(currentPin + maxNumPerBlock);
        blockNum++;
    }
    FieldValues[currentPin - 1] = fv;
}

void DataColumn::SetValue(int nIndex, int value)
{
    Variant fv(value);
    ValidValue(fv);
    FieldValues[nIndex] = fv;
}

void DataColumn::AddRecord(double value)
{
    Variant fv(value);
    ValidValue(fv);
    currentPin++;
    if (currentPin >= maxNumPerBlock * blockNum)
    {
        FieldValues.resize(currentPin + maxNumPerBlock);
        blockNum++;
    }
    FieldValues[currentPin - 1] = fv;
}

void DataColumn::SetValue(int nIndex, double value)
{
    Variant fv(value);
    ValidValue(fv);
    FieldValues[nIndex] = fv;
}

void DataColumn::RemoveAt(int nIndex)
{
    FieldValues.erase(std::begin(FieldValues) + nIndex);
    currentPin--;
    blockNum = currentPin / maxNumPerBlock + 1;
    FieldValues.resize(blockNum*maxNumPerBlock);
}

void DataColumn::Clear()
{
    currentPin = 0;
    blockNum = 1;
    FieldValues.resize(blockNum*maxNumPerBlock);
}

string DataColumn::GetFieldValueAsString(int nIndex)
{
    Variant vt=FieldValues[nIndex];
    switch(vt.vt){
    case VRT_I1:
        return to_string(vt.cVal);
    case VRT_I2:
        return to_string(vt.iVal);
    case VRT_I4:
        return to_string(vt.lVal);
    case VRT_UI1:
        return to_string(vt.bVal);
    case VRT_UI2:
        return to_string(vt.uiVal);
    case VRT_UI4:
        return to_string(vt.ulVal);
    case VRT_I8:
        return to_string(vt.llVal);
    case VRT_UI8:
        return to_string(vt.ullVal);
    case VRT_R4:
        return StringHelper::ConvertStringByError(vt.fltVal);
    case VRT_R8:
        return StringHelper::ConvertStringByError(vt.dblVal);
    case VRT_BSTR:
        return vt.asString();
    default:{
        vt.ChangeType(VRT_BSTR);
        return *vt.bstrVal;
    }
    }
}

Variant DataColumn::GetFieldValue(int nIndex)
{
    return FieldValues[nIndex];
}

DOUBLE DataColumn::GetFieldValueAsDouble(int nIndex)
{
    Variant vt=FieldValues[nIndex];
    vt.ChangeType(VRT_R8);
    return vt.dblVal;
}

DateTime DataColumn::GetFieldValueAsDateTime(int nIndex){
    Variant vt=FieldValues[nIndex];
    vt.ChangeType(VRT_DATETIME);
    DateTime dTime(vt.date);
    return dTime;
}

void DataColumn::SetSize(LONG Size)
{
    Clear();
    currentPin = Size;
    blockNum = currentPin / maxNumPerBlock + 1;
    FieldValues.resize(blockNum*maxNumPerBlock);
}

DataTable::DataTable(string name,vector<DatabaseField>columns){
   this->name=name;
   for(int k=0;k<columns.size();k++){
       this->columns.push_back(new DataColumn(columns[k]));
   }
   curpin=-1;
   recordNumber=0;
}

DataTable::DataTable(string name,AutoPtr<DatabaseFields>columns){
   this->name=name;
   for(int k=0;k<columns->GetSize();k++){
       this->columns.push_back(new DataColumn(columns->GetItem(k)));
   }
   curpin=-1;
   recordNumber=0;
}

DataTable::~DataTable(){

}


AutoPtr<DataTable>DataTable::FromRecordSet(Recordset*rs,string name){
    AutoPtr<DatabaseFields>dfs=rs->GetFields();
    vector<DatabaseField>dfss;
    int fieldCount=dfs->GetSize();
    for(int k=0;k<dfs->GetSize();k++){
        dfss.push_back(dfs->GetItem(k));
    }
    AutoPtr<DataTable>dt=new DataTable(name,dfss);
    rs->ResetReading();
    vector<Variant>values;
    values.resize(fieldCount);
    while(rs->MoveNext()){
        for(int k=0;k<fieldCount;k++){
            DatabaseField&df=dfss[k];
            switch(df.fieldType){
            case DatabaseFieldType::ftInteger:
                values[k]=rs->GetFieldValueAsInteger(k);
                break;
            case DatabaseFieldType::ftNumeric:
                values[k]=rs->GetFieldValueAsDouble(k);
                break;
            case DatabaseFieldType::ftString:
                values[k]=rs->GetFieldValueAsString(k);
                break;
            case DatabaseFieldType::ftDate:
            case DatabaseFieldType::ftTime:
                values[k]=rs->GetFieldValueAsDateTime(k);
                break;
            default:
                values[k]=rs->GetFieldValueAsString(k);
                break;
            }
        }
        dt->AddRecord(values);
    }
    return dt;
}

AutoPtr<DataTable>DataTable::LoadFromStream(BufferReader*br){
    int fieldCount=br->Read<int>();
    vector<DatabaseField>dfs;
    for(int k=0;k<fieldCount;k++){
        string name=br->ReadString();
        DatabaseFieldType ft=(DatabaseFieldType)br->Read<int>();
        dfs.push_back(DatabaseField(name,ft));
    }
    AutoPtr<DataTable>dt(new DataTable("br",dfs));
    int recordCount=br->Read<int>();
    vector<Variant>values(fieldCount);
    for(int k=0;k<recordCount;k++){
        for(int j=0;j<fieldCount;j++){
            switch(dfs[j].fieldType){
            case DatabaseFieldType::ftInteger:{
                int iv=br->Read<int>();
                values[j]=iv;
                break;
            }
            case DatabaseFieldType::ftNumeric:{
                double dv=br->Read<double>();
                values[j]=dv;
                break;
            }
            default:{
                string sv=br->ReadString();
                values[j]=sv;
                break;
            }
            }
        }
        dt->AddRecord(values);
    }
    return dt;
}

bool DataTable::ResetReading(){
    if(recordNumber==0) return false;
    curpin=-1;
    return true;
}
string DataTable::GetName(){
    return name;
}
void DataTable::AddRecord(){
    for(int k=0;k<columns.size();k++){
        columns[k]->AddRecord(Variant(string("")));
    }
    recordNumber++;
}
void DataTable::AddRecord(const vector<Variant>&newValues){
    for(int k=0;k<columns.size();k++){
        columns[k]->AddRecord(newValues[k]);
    }
    recordNumber++;
}
bool DataTable::MoveNext(){
   int curNum=curpin+1;
   if(curNum>=recordNumber) return false;
   curpin++;
   return true;
}
string DataTable::GetFieldValueAsString(int fieldIndex){
    return columns[fieldIndex]->GetFieldValueAsString(curpin);
}
float DataTable::GetFieldValueAsSingle(int fieldIndex){
    return columns[fieldIndex]->GetFieldValueAsDouble(curpin);
}
double DataTable::GetFieldValueAsDouble(int fieldIndex){
    return columns[fieldIndex]->GetFieldValueAsDouble(curpin);
}
int DataTable::GetFieldValueAsInteger(int fieldIndex){
    return columns[fieldIndex]->GetFieldValueAsDouble(curpin);
}
DateTime DataTable::GetFieldValueAsDateTime(int fieldIndex){
    return columns[fieldIndex]->GetFieldValueAsDateTime(curpin);
}

string DataTable::GetFieldValueAsString(int recordIndex,int fieldIndex){
    return columns[fieldIndex]->GetFieldValueAsString(recordIndex);
}
Variant DataTable::GetFieldValue(int recordIndex,int fieldIndex){
    return columns[fieldIndex]->GetFieldValue(recordIndex);
}
double DataTable::GetFieldValueAsDouble(int recordIndex,int fieldIndex){
    return columns[fieldIndex]->GetFieldValueAsDouble(recordIndex);
}

void DataTable::SetFieldValueWithString(int recordIndex,int fieldIndex,string value){
    columns[fieldIndex]->SetValue(recordIndex,value);
}
void DataTable::SetFieldValue(int recordIndex,int fieldIndex,Variant value){
    columns[fieldIndex]->SetValue(recordIndex,value);
}

void DataTable::RemoveField(int nIndex){
    columns.erase(begin(columns)+nIndex);
}

AutoPtr<DatabaseFields>DataTable::GetFields(){
    AutoPtr<DatabaseFields>fields(new DatabaseFields());
    for(int k=0;k<columns.size();k++){
        DatabaseField df=columns[k]->GetField();
        fields->AddField(df);
    }
    return fields;
}
//jiang
int DataTable::GetRecordCount(){
    return recordNumber;
}

AutoPtr<DataTable>DataTable::LoadFromCsv(string pathName){
    string content;
    string code=TextFile::GetFileCode(pathName);
    bool localCode=(code=="ANSI");
    TextFile file;
    if(!file.OpenForRead(pathName)) return nullptr;
    int recordNum=-1;
    AutoPtr<DataTable>dt;
    vector<DatabaseField>dfs;
    string dataTableName=FilePath::GetFileNameNoExa(pathName);
    while(file.ReadLine(content)){
        if(localCode) content=StringHelper::FromLocalString(content);
        vector<string>fields=StringHelper::Split(content,',','"');
        if(recordNum==-1){
            if(fields.size()==0) return nullptr;
            for(int k=0;k<fields.size();k++){
                string field=fields[k];
                if(field.length()>=2){
                    if((field[0]=='\"')&&(field[field.length()-1]=='\"')){
                        field=field.substr(1,field.length()-2);
                    }
                    if((field[0]=='\'')&&(field[field.length()-1]=='\'')){
                        field=field.substr(1,field.length()-2);
                    }
                }
                DatabaseField df(field,DatabaseFieldType::ftString);
                dfs.push_back(df);
            }
            dt=new DataTable(dataTableName,dfs);
        }
        else{
            vector<string>values=StringHelper::Split(content,',','"');
            int toSize=values.size();
            if(toSize==1){
                string sV=StringHelper::Trim(values[0]);
                if(sV=="") continue;
            }
            if(toSize>dfs.size()) toSize=dfs.size();
            dt->AddRecord();
            for(int j=0;j<toSize;j++){
                string value=values[j];
                value=StringHelper::Trim(value);
                if(value.length()>=2){
                    if((value[0]=='\"')&&(value[value.length()-1]=='\"')){
                        value=value.substr(1,value.length()-2);
                    }
                    if((value[0]=='\'')&&(value[value.length()-1]=='\'')){
                        value=value.substr(1,value.length()-2);
                    }
                }
                dt->SetFieldValueWithString(recordNum,j,value);
            }
        }
        recordNum++;
    }
    return dt;
}

AutoPtr<DataTable>DataTable::LoadFromCsv(string pathName,int fromRecord,int recordCount){
    string code=TextFile::GetFileCode(pathName);
    bool localCode=(code=="ANSI");
    string content;
    TextFile file;
    if(!file.OpenForRead(pathName)) return nullptr;
    int recordNum=-1;
    AutoPtr<DataTable>dt;
    vector<DatabaseField>dfs;
    string dataTableName=FilePath::GetFileNameNoExa(pathName);
    int curPin=-1;
    int endPin=fromRecord+recordCount-1;
    while(file.ReadLine(content)){
        if(localCode) content=StringHelper::FromLocalString(content);
        vector<string>fields=StringHelper::Split(content,',','"');
        if(recordNum==-1){
            if(fields.size()==0) return nullptr;
            for(int k=0;k<fields.size();k++){
                string field=fields[k];
                if(field.length()>=2){
                    if((field[0]=='\"')&&(field[field.length()-1]=='\"')){
                        field=field.substr(1,field.length()-2);
                    }
                    if((field[0]=='\'')&&(field[field.length()-1]=='\'')){
                        field=field.substr(1,field.length()-2);
                    }
                }
                DatabaseField df(field,DatabaseFieldType::ftString);
                dfs.push_back(df);
            }
            dt=new DataTable(dataTableName,dfs);
        }
        else{
            curPin++;
            if(curPin<fromRecord) continue;
            if(curPin>endPin) break;
            vector<string>values=StringHelper::Split(content,',','"');
            int toSize=values.size();
            if(toSize==1){
                string sV=StringHelper::Trim(values[0]);
                if(sV=="") continue;
            }
            if(toSize>dfs.size()) toSize=dfs.size();
            dt->AddRecord();
            for(int j=0;j<toSize;j++){
                string value=values[j];
                value=StringHelper::Trim(value);
                if(value.length()>=2){
                    if((value[0]=='\"')&&(value[value.length()-1]=='\"')){
                        value=value.substr(1,value.length()-2);
                    }
                    if((value[0]=='\'')&&(value[value.length()-1]=='\'')){
                        value=value.substr(1,value.length()-2);
                    }
                }
                dt->SetFieldValueWithString(recordNum,j,value);
            }
        }
        recordNum++;
    }
    return dt;
}

AutoPtr<DatabaseFields>DataTable::LoadHeaderFromCsv(string pathName){
    string code=TextFile::GetFileCode(pathName);
    bool localCode=(code=="ANSI");
    string content;
    TextFile file;
    if(!file.OpenForRead(pathName)) return nullptr;
    AutoPtr<DataTable>dt;
    AutoPtr<DatabaseFields>dfs=new DatabaseFields();
    string dataTableName=FilePath::GetFileNameNoExa(pathName);
    if(!file.ReadLine(content)) return nullptr;
    if(localCode) content=StringHelper::FromLocalString(content);
    vector<string>fields=StringHelper::Split(content,',','"');
    if(fields.size()==0) return nullptr;
    for(int k=0;k<fields.size();k++){
        string field=fields[k];
        if(field.length()>=2){
            if((field[0]=='\"')&&(field[field.length()-1]=='\"')){
                field=field.substr(1,field.length()-2);
            }
            if((field[0]=='\'')&&(field[field.length()-1]=='\'')){
                field=field.substr(1,field.length()-2);
            }
        }
        DatabaseField df(field,DatabaseFieldType::ftString);
        dfs->AddField(df);
    }
    return dfs;
}

bool DataTable::SaveToCsv(string pathName){
    AutoPtr<DatabaseFields>fields=GetFields();
    int fieldCount=fields->GetSize();
    TextFile tf;
    if(!tf.OpenForWrite(pathName,false)) return false;
    string header="";
    for(int k=0;k<fieldCount;k++){
        if(k==0)
            header="\""+fields->GetItem(k).fieldName+"\"";
        else
            header+=",\""+fields->GetItem(k).fieldName+"\"";
    }
    tf.WriteLine(header);
    int recordCount=this->GetRecordCount();
    for(int k=0;k<recordCount;k++){
        string rowValue="";
        for(int j=0;j<fieldCount;j++){
            if(fields->GetItem(j).IsFieldNumeric()){
                if(j==0)
                    rowValue=GetFieldValueAsString(k,j);
                else
                    rowValue+=","+GetFieldValueAsString(k,j);
            }
            else{
                if(j==0)
                    rowValue="\""+GetFieldValueAsString(k,j)+"\"";
                else
                    rowValue+=",\""+GetFieldValueAsString(k,j)+"\"";
            }
        }
        tf.WriteLine(rowValue);
    }
    return true;
}

bool DataTable::SaveToCsvText(string&content){
    content="";
    AutoPtr<DatabaseFields>fields=GetFields();
    int fieldCount=fields->GetSize();
    string header="";
    for(int k=0;k<fieldCount;k++){
        if(k==0)
            header="\""+fields->GetItem(k).fieldName+"\"";
        else
            header+=",\""+fields->GetItem(k).fieldName+"\"";
    }
    content+=header+"\r\n";
    int recordCount=this->GetRecordCount();
    for(int k=0;k<recordCount;k++){
        string rowValue="";
        for(int j=0;j<fieldCount;j++){
            if(fields->GetItem(j).IsFieldNumeric()){
                if(j==0)
                    rowValue=GetFieldValueAsString(k,j);
                else
                    rowValue+=","+GetFieldValueAsString(k,j);
            }
            else{
                if(j==0)
                    rowValue="\""+GetFieldValueAsString(k,j)+"\"";
                else
                    rowValue+=",\""+GetFieldValueAsString(k,j)+"\"";
            }
        }
        content+=rowValue+"\r\n";
    }
    return true;
}

DataConnection::DataConnection(string name) {
    this->name=name;
}

DataConnection::~DataConnection() {
}

string DataConnection::GetName(){
    return name;
}

DatabaseConnection::DatabaseConnection(string name):
   DataConnection(name){

}

DatabaseConnection::~DatabaseConnection() {
}

bool DatabaseConnection::Exists(string sql){
    AutoPtr<Recordset>rec=Select(sql);
    if (rec == NULL) return false;
    if (!rec->MoveNext()) return false;
    int iV=rec->GetFieldValueAsInteger(0);
    return (iV > 0);
}
bool DatabaseConnection::GetFirstFieldValueOfFirstRecord(string sql,string&value){
    AutoPtr<Recordset>rec=Select(sql);
    if (rec == NULL) return false;
    if (!rec->MoveNext()) return false;
    value = rec->GetFieldValueAsString(0);
    return true;
}

SqliteRecordset::SqliteRecordset(int Rows, int Cols, char**Contents){
    rows = Rows;
     cols = Cols;
    contents = Contents;
    currenPin=-1;
}
SqliteRecordset::~SqliteRecordset(){
    sqlite3_free_table(contents);
}
bool SqliteRecordset::ResetReading(){
    if(rows==0) return false;
    currenPin=-1;
    return true;
}
bool SqliteRecordset::MoveNext(){
   currenPin++;
   if(currenPin>=rows) return false;
   return true;
}
string SqliteRecordset::GetFieldValueAsString(int fieldIndex){
    char*v = contents[fieldIndex + (currenPin + 1)*cols];
    string vu(v);
    return vu;
}
float SqliteRecordset::GetFieldValueAsSingle(int fieldIndex){
    char*v = contents[fieldIndex + (currenPin + 1)*cols];
    return atof(v);
}
double SqliteRecordset::GetFieldValueAsDouble(int fieldIndex){
    char*v = contents[fieldIndex + (currenPin + 1)*cols];
    return atof(v);
}
int SqliteRecordset::GetFieldValueAsInteger(int fieldIndex){
    char*v = contents[fieldIndex + (currenPin + 1)*cols];
    return atoi(v);
}
DateTime SqliteRecordset::GetFieldValueAsDateTime(int fieldIndex){
    char*v = contents[fieldIndex + (currenPin + 1)*cols];
    string sV=string(v);
    DateTime time;
    time.FromString(sV);
    return time;
}
AutoPtr<DatabaseFields>SqliteRecordset::GetFields(){
    int fieldCount = cols;
    DatabaseFields*newfields = new DatabaseFields();
    for (int k = 0; k < fieldCount; k++) {
        string fieldName = contents[k];
        newfields->AddField(fieldName, ftString);
    }
    return newfields;
}

SqliteDatabaseConnection::SqliteDatabaseConnection(string name):
   DatabaseConnection(name){
    db=nullptr;
}

SqliteDatabaseConnection::SqliteDatabaseConnection(string name,string path):
    DatabaseConnection(name){
    this->pathName=path;
    db=nullptr;
}
SqliteDatabaseConnection::~SqliteDatabaseConnection(){
    if(db!=nullptr)  sqlite3_close(db);
}

bool SqliteDatabaseConnection::GetConnected(){
    return (db!=nullptr);
}
string SqliteDatabaseConnection::GetPathName(){
    return pathName;
}
void SqliteDatabaseConnection::Initialize(string pathName){
    if(db!=nullptr)  sqlite3_close(db);
    db=nullptr;
    this->pathName=pathName;
}

bool SqliteDatabaseConnection::Connect(){
    if(db!=nullptr)  sqlite3_close(db);
    db = nullptr;
    string path=pathName;
    int ret = sqlite3_open(path.c_str(), &db);
    if (ret != SQLITE_OK){
        db = nullptr;
        return false;
    }
    return true;
}

void SqliteDatabaseConnection::Disconnect(){
    if(db!=nullptr)  sqlite3_close(db);
    db = nullptr;
}
bool SqliteDatabaseConnection::Execute(string command){
    if (db == 0) return false;
    char * pErrMsg = 0;
    int ret = sqlite3_exec(db, command.c_str(), 0, 0, &pErrMsg);
    if (ret != SQLITE_OK)
    {
        sqlite3_free(pErrMsg);
        return false;
    }
    if (pErrMsg != NULL) sqlite3_free(pErrMsg);
    return true;
}
AutoPtr<Recordset>SqliteDatabaseConnection::Select(string sql){
    char* errMsg;
    int nrow = 0, ncolumn = 0;
    char **azResult;
    int ret = sqlite3_get_table(db, sql.c_str(), &azResult, &nrow, &ncolumn, &errMsg);
    if (ret != SQLITE_OK)
    {
        sqlite3_free(errMsg);
        return nullptr;
    }
    if (errMsg != nullptr) sqlite3_free(errMsg);
    return new SqliteRecordset(nrow, ncolumn, azResult);
}

string SqliteDatabaseConnection::GetTypeName(){
    return "SqliteDatabaseConnection";
}

//BufferWritable:
bool SqliteDatabaseConnection::Read(BufferReader*br){
    name=br->ReadString();
    pathName=br->ReadString();
    return true;
}
void SqliteDatabaseConnection::Write(BufferWriter*bw){
    bw->WriteString(name);
    bw->WriteString(pathName);
}

bool SqliteDatabaseConnection::ReadFromJson(JsonReader*jr,JsonNode node){
    name=node["name"].asString();
    pathName=node["pathName"].asString();
    return true;
}

string SqliteDatabaseConnection::ToJson(){
    JsonObjectWriter jow;
    jow.Add<string>("name",name);
    jow.Add<string>("pathName",pathName);
    return jow.GetJsonString();
}

DataConnections::DataConnections(){

}
DataConnections::~DataConnections(){

}

string DataConnections::GetName(){
    return "数据链接";
}

int DataConnections::GetSize(){
    return dses.size();
}
AutoPtr<DataConnection>DataConnections::GetItem(int nIndex){
    return dses[nIndex];
}
void DataConnections::Remove(int nIndex){
   dses.erase(begin(dses)+nIndex);
}

AutoPtr<DataConnection>DataConnections::Find(string name){
    for(int k=dses.size()-1;k>=0;k--){
        if(name==dses[k]->GetName()) return dses[k];
    }
    return nullptr;
}

bool DataConnections::AddDataConnection(AutoPtr<DataConnection>ds){
    AutoPtr<DataConnection>dds=Find(ds->GetName());
    if(dds!=nullptr) return false;
    dses.push_back(ds);
    return true;
}
void DataConnections::Clear(){
   dses.clear();
}

AutoPtr<DataConnections>DataConnections::Clone(){
    DataConnections*npd=new DataConnections();
    npd->dses.resize(dses.size());
    for(int k=0;k<dses.size();k++) npd->dses[k]=dses[k];
    return npd;
}

bool DataConnections::Read(BufferReader*br){
    int fcount=br->Read<int>();
    dses.clear();
    for(int k=0;k<fcount;k++){
        AutoPtr<DataConnection>ds=br->ReadObject(false);
        dses.push_back(ds);
    }
    return true;
}
void DataConnections::Write(BufferWriter*bw){
    bw->Write<int>(dses.size());
    for(int k=0;k<dses.size();k++){
        bw->WriteObject(dses[k].get(),false);
    }
}

string DataConnections::GetTypeName(){
    return "DataConnections";
}

bool DataConnections::ReadFromJson(JsonReader*jr,JsonNode node){
    int fcount=node.size();
    dses.clear();
    for(int k=0;k<fcount;k++){
        AutoPtr<DataConnection>ds=JsonReader::ReadObject(jr,node[k]);
        if(ds!=nullptr) dses.push_back(ds);
    }
    return true;
}

string DataConnections::ToJson(){
    JsonArrayWriter writer;
    int fcount=dses.size();
    for(int k=0;k<fcount;k++){
       writer.WriteObject(dses[k].get());
    }
    return writer.GetJsonString();
}

string SqlHelper::CreateInsertSql(const vector<Variant>&values){
    string sql="(";
    for(int k=0;k<values.size();k++){
        if(k>0) sql+=",";
        VarType vType=values[k].vt;
        if(vType==VRT_BSTR){
            sql+="'"+(*values[k].bstrVal)+"'";
        }
        else{
            Variant var=values[k];
            sql+=var.ToSqlString();
        }
    }
    sql+=")";
    return sql;
}

string SqlHelper::CreateInsertSql(const string&tableName,const vector<Variant>&values){
    string sql="insert into "+tableName+" values"+CreateInsertSql(values);
    return sql;
}

string SqlHelper::CreateInsertSqlWithFields(const string&tableName,const vector<std::tuple<string,Variant>>&values,string fieldPrefix){
    string sql="insert into "+tableName+"(";
    string sFields="";
    for(int k=0;k<values.size();k++){
        if(k>0) sFields+=",";
        sFields+=fieldPrefix+get<0>(values[k])+fieldPrefix;
    }
    sql+=sFields+") values(";
    for(int k=0;k<values.size();k++){
        if(k>0) sql+=",";
        Variant var=get<1>(values[k]);
        sql+=var.ToSqlString();
    }
    sql+=")";
    return sql;
}

string SqlHelper::CreateUpdateSql(const string&tableName,const vector<std::tuple<string,Variant>>&values,string fieldPrefix){
    string sql="update table "+tableName+" set ";
    for(int k=0;k<values.size();k++){
        if(k>0) sql+=",";
        sql+=fieldPrefix+get<0>(values[k])+fieldPrefix+"=";
        Variant var=get<1>(values[k]);
        sql+=var.ToSqlString();
    }
    return sql;
}

int SqlHelper::GetRecordCount(DatabaseConnection*conn,const vector<string>&tables,const string&whereClouse,bool allTableExists){
    string wlc=whereClouse;
    if(wlc!="") wlc = " where "+whereClouse;
    if(allTableExists){
        string sql="";
        for(int k=0;k<tables.size();k++){
            if(k==0)
               sql="(select count(*) from "+tables[k]+wlc+")";
            else
               sql+=" union (select count(*) from "+tables[k]+wlc+")";
        }
        AutoPtr<Recordset>rs=conn->Select(sql);
        if(rs==nullptr) return 0;
        int totalCount=0;
        while(rs->MoveNext()){
            totalCount+=rs->GetFieldValueAsInteger(0);
        }
        return totalCount;
    }
    else{
        int totalCount=0;
        for(int k=0;k<tables.size();k++){
            string sql="select count(*) from "+tables[k]+wlc;
            AutoPtr<Recordset>rs=conn->Select(sql);
            if(rs!=nullptr){
                rs->MoveNext();
                totalCount+=rs->GetFieldValueAsInteger(0);
            }
        }
        return totalCount;
    }
}

AutoPtr<Recordset>SqlHelper::GetPageRecordset(DatabaseConnection*conn,const vector<string>&tables,const string&fields,const string&whereClouse,const string&otherClause,int fromRecord,int recordNum,bool allTableExists){
    int totalRecords=0;
    return GetPageRecordsetEx(conn,tables,fields,whereClouse,otherClause,totalRecords,fromRecord,recordNum,allTableExists);
}

AutoPtr<Recordset>SqlHelper::GetPageRecordsetEx(DatabaseConnection*conn,const vector<string>&tables,const string&fields,const string&whereClouse,const string&otherClause,int&totalRecords,int fromRecord,int recordNum,bool allTableExists){
    totalRecords=0;
    vector<int>subrecords;
    subrecords.push_back(0);
    string wlc=whereClouse;
    if(wlc!="") wlc = " where "+whereClouse;
    int totalCount=0;
    if(allTableExists){
        string sql="";
        for(int k=0;k<tables.size();k++){
            if(k==0)
               sql="(select count(*) from "+tables[k]+wlc+")";
            else
               sql+=" union (select count(*) from "+tables[k]+wlc+")";
        }
        AutoPtr<Recordset>rs=conn->Select(sql);
        if(rs==nullptr) return nullptr;
        while(rs->MoveNext()){
            totalCount+=rs->GetFieldValueAsInteger(0);
            subrecords.push_back(totalCount);
        }
    }
    else{
        for(int k=0;k<tables.size();k++){
            string sql="select count(*) from "+tables[k]+wlc;


            //TextFile::Write("D:/work/temp.txt",sql);

            AutoPtr<Recordset>rs=conn->Select(sql);
            if(rs!=nullptr){
                rs->MoveNext();
                totalCount+=rs->GetFieldValueAsInteger(0);
            }
            subrecords.push_back(totalCount);
        }
    }
    totalRecords=totalCount;
    if(totalRecords==0){
        string sql="select * from "+tables[0]+wlc+" "+otherClause+ " limit 0,0";
        return conn->Select(sql);
    }
    vector<int>tableIndices;
    vector<int>fromTableRecord;
    vector<int>tableRecords;
    int toRecord=fromRecord+recordNum;

    int fromps=subrecords[0];
    for(int l=1;l<subrecords.size();l++){
        int ps=subrecords[l];
        if((ps<=fromRecord)||(fromps>=toRecord)){
            fromps=ps;
            continue;
        }
        tableIndices.push_back(l-1);
        int fromyr;
        if(fromRecord<=fromps)
            fromyr=0;
        else
            fromyr=fromRecord-fromps;
        fromTableRecord.push_back(fromyr);
        if(toRecord>ps)
            tableRecords.push_back(ps-fromps-fromyr);
        else
            tableRecords.push_back(toRecord-(fromyr+fromps));
        fromps=ps;
    }
    string sql="";
    if(fields.empty()){
        for(int k=0;k<tableIndices.size();k++){
            if(tableRecords[k]==0) continue;
            if(!sql.empty()) sql+=" union ";
            sql+="(select * from "+tables[tableIndices[k]]+wlc+" "+otherClause+ " limit "+to_string(fromTableRecord[k])+","+to_string(tableRecords[k])+")";
        }
    }
    else{
        for(int k=0;k<tableIndices.size();k++){
            if(tableRecords[k]==0) continue;
            if(!sql.empty()) sql+=" union ";
            sql="(select "+fields+" from "+tables[tableIndices[k]]+wlc+" "+otherClause+ " limit "+to_string(fromTableRecord[k])+","+to_string(tableRecords[k])+")";
        }
    }
    return conn->Select(sql);
}

AutoPtr<Recordset>SqlHelper::GetRecordset(DatabaseConnection*conn,const vector<string>&tables,const string&fields,const string&whereClouse,const string&otherClause,bool allTableExists){
    vector<int>subrecords;
    subrecords.push_back(0);
    string wlc=whereClouse;
    if(wlc!="") wlc = " where "+whereClouse;
    int tableSize=tables.size();
    vector<bool>tableValids;
    tableValids.resize(tableSize);
    if(allTableExists){
        for(int k=0;k<tableSize;k++){
           tableValids[k]=true;
        }
    }
    else{
        for(int k=0;k<tableSize;k++){
            string sql="select count(*) from "+tables[k]+wlc;
            AutoPtr<Recordset>rs=conn->Select(sql);
            tableValids[k]=(rs!=nullptr);
        }
    }
    string sql="";
    if(fields.empty()){
        for(int k=0;k<tableSize;k++){
            if(!tableValids[k]) continue;
            if(!sql.empty()) sql+=" union ";
            sql+="(select * from "+tables[k]+wlc+" "+otherClause+")";
        }
    }
    else{
        for(int k=0;k<tableSize;k++){
            if(!tableValids[k]) continue;
            if(!sql.empty()) sql+=" union ";
            sql="(select "+fields+" from "+tables[k]+wlc+" "+otherClause+")";
        }
    }
    return conn->Select(sql);
}


SqlInsertTool::SqlInsertTool(DatabaseConnection*conn,string tableName,const vector<string>&fields,int bufferCount){
    this->conn=conn;
    if(bufferCount>1)
        insertSql="insert ignore into "+tableName+"(";
    else
        insertSql="insert into "+tableName+"(";
    for(int k=0;k<fields.size();k++){
        if(k>0) insertSql+=",";
        insertSql+=fields[k];
    }
    insertSql+=") values";
    sql=insertSql;
    curPin=0;
    this->bufferCount=bufferCount;
}

SqlInsertTool::~SqlInsertTool(){
    if(curPin>0){
        conn->Execute(sql);
    }
}

bool SqlInsertTool::Insert(const vector<Variant>&values){
    string subSql=SqlHelper::CreateInsertSql(values);
    if(curPin==0)
        sql+=subSql;
    else
        sql+=","+subSql;
    bool IsOk=true;
    curPin++;
    if(curPin>=bufferCount){
        IsOk=conn->Execute(sql);
        curPin=0;
        sql=insertSql;
    }
    return IsOk;
}

}
