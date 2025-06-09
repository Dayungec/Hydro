#include "memfeatureclass.h"
#include "Base/StringHelper.h"
#include "Base/FilePath.h"
#include "shapfilefeatureclass.h"

namespace SGIS{


MemField::MemField(string Name,VectorFieldType type)
{
    FieldName=Name;
    FieldType=type;
}
MemField::~MemField()
{
    Clear();
}

void MemField::ValidValue(Variant&value)
{
    switch(FieldType)
    {
    case vftInteger:
        {
            value.ChangeType(VRT_I4);
            break;
        }
    case vftReal:
        {
            value.ChangeType(VRT_R8);
            break;
        }
    case vftDate:
        {
            value.ChangeType(VRT_DATE);
            break;
        }
    case vftTime:
    case vftDateTime:
        {
            value.ChangeType(VRT_DATETIME);
            break;
        }
    }
}

void MemField::AddRecord(Variant value)
{
    ValidValue(value);
    FieldValues.push_back(value);
}
void MemField::SetValue(int nIndex,Variant value)
{
    ValidValue(value);
    FieldValues[nIndex]=value;
}
void MemField::Remove(int nIndex)
{
    FieldValues.erase(begin(FieldValues)+nIndex);
}
void MemField::Clear()
{
    FieldValues.clear();
}
string MemField::GetFieldValueAsString(int nIndex)
{
    return FieldValues[nIndex].asString();
}
Variant MemField::GetFieldValue(int nIndex)
{
    return FieldValues[nIndex];
}
DOUBLE MemField::GetFieldValueAsDouble(int nIndex)
{
    return FieldValues[nIndex].asDouble();
}
void MemField::SetSize(LONG Size)
{
    Clear();
    FieldValues.resize(Size);
}

MemVectors::MemVectors(AutoPtr<TableDesc>desc,VectorShapeType type)
{
    pDesc=desc;
    LONG Count=pDesc->GetFieldCount();
    for(int k=0;k<Count;k++)
    {
        string FieldName=pDesc->GetFieldName(k);
        VectorFieldType ft=pDesc->GetFieldType(k);
        AutoPtr<MemField>newf(new MemField(FieldName,ft));
        fields.push_back(newf);
    }
    RecordNumber=0;
    Extent=new Envelope();
    ShapeType=type;
}
MemVectors::~MemVectors()
{
    fields.clear();
    geos.clear();
}
void MemVectors::DeleteField(int index)
{
    fields.erase(begin(fields)+index);
    AutoPtr<TableDescCreator>pCreator(new TableDescCreator);
    LONG Count=pDesc->GetFieldCount();
    for(int k=0;k<Count;k++)
    {
        FieldDesp fd=(*pDesc)[k];
        if(k!=index) pCreator->AddField(fd);
    }
    pDesc=pCreator->CreateTableDesc();
}
int MemVectors::GetFeatureID(int nIndex){
    if(ids.size()==0) return nIndex;
    return ids[nIndex];
}
int MemVectors::FindField(string FieldName)
{
    return pDesc->FindField(FieldName);
}
bool MemVectors::AddField(string FieldName,VectorFieldType type,LONG FieldWidth,LONG Precision)
{
     int index=FindField(FieldName);
     if(index>=0) return false;
     AutoPtr<MemField>fld(new MemField(FieldName,type));
     fields.push_back(fld);
     fld->SetSize(RecordNumber);
     AutoPtr<TableDescCreator>pCreator(new TableDescCreator);
     pCreator->FromTableDesc(pDesc.get());
     pCreator->AddField(FieldDesp(FieldName,type,FieldWidth,Precision));
     pDesc=pCreator->CreateTableDesc();
     return true;
}
bool MemVectors::AddRecord(AutoPtr<Geometry>geo,const vector<Variant>&newValues)
{
    GeometryType gType=geo->GetType();
    if(ShapeType==vstPoint)
    {
        if(gType!=gtPoint) return false;
    }
    if(ShapeType==vstPolyline)
    {
        if(gType!=gtPolyline) return false;
    }
    if(ShapeType==vstPolygon)
    {
        if((gType!=gtPolygon)&&(gType!=gtMultiPolygon)) return false;
    }
    int FieldCount=fields.size();
    for(int k=0;k<FieldCount;k++)
    {
        fields[k]->AddRecord(newValues[k]);
    }
    AutoPtr<Geometry>newGeo=geo->Clone();
    RecordNumber++;
    geos.push_back(newGeo);
    if(RecordNumber==1)
    {
        Extent=geo->GetExtent();
    }
    else
    {
        Extent->UnionOther(geo->GetExtent().get());
    }
    if(ids.size()>0) ids.push_back(ids.size());
    return true;
}
bool MemVectors::AddRecord(AutoPtr<Geometry>geo)
{
    GeometryType gType=geo->GetType();
    if(ShapeType==vstPoint)
    {
        if(gType!=gtPoint) return false;
    }
    if(ShapeType==vstPolyline)
    {
        if(gType!=gtPolyline) return false;
    }
    if(ShapeType==vstPolygon)
    {
        if((gType!=gtPolygon)&&(gType!=gtMultiPolygon)) return false;
    }
    int FieldCount=fields.size();
    for(int k=0;k<FieldCount;k++)
    {
        VectorFieldType vft=pDesc->GetFieldType(k);
        switch(vft)
        {
        case vftString:
        case vftBinary:{
            Variant vt(string(""));
            fields[k]->AddRecord(vt);
            break;
        }
        default:
            fields[k]->AddRecord(0);
            break;
        }
    }
    AutoPtr<Geometry>newGep=geo->Clone();
    RecordNumber++;
    geos.push_back(geo->Clone());
    if(RecordNumber==1)
    {
        Extent=geo->GetExtent();
    }
    else
    {
        Extent->UnionOther(geo->GetExtent().get());
    }
    if(ids.size()>0) ids.push_back(ids.size());
    return true;
}

bool MemVectors::AddRecord(int fid,AutoPtr<Geometry>geo,const vector<Variant>&newValues){
    if(ids.size()==0){
        ids.resize(this->geos.size());
        for(int k=0;k<this->geos.size();k++) ids[k]=k;
    }
    if(!AddRecord(geo,newValues)) return false;
    if(ids.size()==this->geos.size())
        ids[ids.size()-1]=fid;
    else
        ids.push_back(fid);
    return true;
}

bool MemVectors::AddRecord(int fid,AutoPtr<Geometry>geo){
    if(ids.size()==0){
        ids.resize(this->geos.size());
        for(int k=0;k<this->geos.size();k++) ids[k]=k;
    }
    if(!AddRecord(geo)) return false;
    if(ids.size()==this->geos.size())
        ids[ids.size()-1]=fid;
    else
        ids.push_back(fid);
    return true;
}

void MemVectors::DeleteRecord(int index)
{
    int FieldCount=fields.size();
    for(int k=0;k<FieldCount;k++)
    {
        fields[k]->Remove(index);
    }
    geos.erase(begin(geos)+index);
    if(ids.size()>0){
        ids.erase(begin(ids)+index);
    }
    RecordNumber--;
}
void MemVectors::RecomputeEnvelope()
{
    AutoPtr<Envelope>env;
    for(int k=0;k<RecordNumber;k++)
    {
        AutoPtr<Geometry>geo=geos[k];
        if(k==0)
        {
            Extent=geo->GetExtent();
        }
        else
        {
            Extent->UnionOther(geo->GetExtent().get());
        }
    }
}
AutoPtr<TableDesc>MemVectors::GetTableDesc()
{
    return pDesc;
}
LONG MemVectors::GetRecordNumber()
{
    return RecordNumber;
}
AutoPtr<Geometry>MemVectors::GetGeometry(int index)
{
    return geos[index]->Clone();
}

AutoPtr<Geometry>MemVectors::GetGeometryRef(int index){
    return geos[index];
}

bool MemVectors::GetEnvelope(int index,Envelope*env)
{
    AutoPtr<Envelope>pEnv=geos[index]->GetExtent();
    env->CopyFromOther(pEnv.get());
    return true;
}
void MemVectors::SetGeometry(int index,AutoPtr<Geometry>geo)
{
    geos[index]=geo->Clone();
    RecomputeEnvelope();
}
Variant MemVectors::GetFieldValue(int index,int findex)
{
    return fields[findex]->GetFieldValue(index);
}
string MemVectors::GetFieldValueAsString(int index,int findex)
{
    return fields[findex]->GetFieldValueAsString(index);
}
double MemVectors::GetFieldValueAsDouble(int index,int findex)
{
    return fields[findex]->GetFieldValueAsDouble(index);
}
void MemVectors::SetFieldValue(int index,int findex,Variant Value)
{
    fields[findex]->SetValue(index,Value);
}
AutoPtr<Envelope>MemVectors::GetShapeEnvelope()
{
    return Extent->Clone();
}
void MemVectors::RemoveAllRecords()
{
    fields.clear();
    geos.clear();
    ids.clear();
}
VectorShapeType MemVectors::GetShapeType()
{
    return ShapeType;
}
AutoPtr<MemVectors>MemVectors::Copy()
{
    AutoPtr<MemVectors>newVectors(new MemVectors(pDesc,ShapeType));
    newVectors->RecordNumber=RecordNumber;
    LONG size=fields.size();
    for(int k=0;k<size;k++)
    {
        newVectors->fields.push_back(fields[k]);
    }
    size=geos.size();
    newVectors->geos.resize(size);
    for(int k=0;k<RecordNumber;k++)
    {
        newVectors->geos[k]=geos[k];
    }
    if(ids.size()>0){
        newVectors->ids.resize(ids.size());
        for(int k=ids.size()-1;k>=0;k--) newVectors->ids[k]=ids[k];
    }
    newVectors->Extent=Extent;
    return newVectors;
}

bool MemVectors::HasFeatureIds(){
    return (ids.size()>0);
}

MemFeatureClass::MemFeatureClass()
{
   editor=nullptr;
   pVectors=nullptr;
   pVersionManager=nullptr;
   HistoryCount=500;
}

MemFeatureClass::~MemFeatureClass()
{
   editor=nullptr;
   pVectors=nullptr;
   pVersionManager=nullptr;
}

AutoPtr<MemVectors>MemFeatureClass::GetMemVectors(){
   return pVectors;
}

bool MemFeatureClass::HasFeatureIds(){
    if(pVectors==nullptr) return false;
    return pVectors->HasFeatureIds();
}

void MemFeatureClass::InitialData(string name,VectorShapeType shapeType,TableDesc*desc,SpatialReference*pSpatial,bool is3d)
{
    editor=nullptr;
    CurrentPin=-1;
    pVectors=new MemVectors(desc->Clone(),shapeType);
    this->name=name;
    this->is3d=is3d;
    psp=pSpatial->Clone();
    pSelectionSet->Clear();
    this->shapeType=shapeType;
    this->pDesc=desc->Clone();
}

string MemFeatureClass::GetDescription(){
    if(shapeType==vstPolygon)
        return "Polygon Memory FeatureClass";
    else if(shapeType==vstPolyline)
        return "Polyline Memory FeatureClass";
    else if(shapeType==vstPoint)
        return "Point Memory FeatureClass";
    return "Unknown Memory FeatureClass";
}
string MemFeatureClass::GetFeatureClassName(){
    return name;
}

AutoPtr<FeatureDataset>MemFeatureClass::GetFeatureDataset(){
    return AutoPtr<FeatureDataset>(this,true);
}

AutoPtr<Envelope>MemFeatureClass::GetExtent(){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)){
        return editor->GetShapefileEnvelope();
    }
    if(pVectors==nullptr) return nullptr;
    return pVectors->GetShapeEnvelope();
}

int MemFeatureClass::GetFeatureCount(){
    if((editor==nullptr)||(editor->CurrentEdit==nullptr))
    {
        return pVectors->GetRecordNumber();
    }
    else
        return editor->GetRecordNumber();
}
AutoPtr<Geometry>MemFeatureClass::GetFeature(int recordIndex){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr))
    {
        bool bSuc;
        AutoPtr<Geometry>geo=editor->GetGeometry(recordIndex,bSuc);
        if(bSuc) return geo;
        recordIndex=editor->GetFeatureID(recordIndex);
    }
    if(pVectors==nullptr) return nullptr;
    return pVectors->GetGeometry(recordIndex);
}
bool MemFeatureClass::GetFeatureEnvelope(int recordIndex,Envelope*pEnv){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr))
    {
        if(editor->GetEnvelope(recordIndex,pEnv)) return true;
        recordIndex=editor->GetFeatureID(recordIndex);
    }
    if(pVectors==nullptr) return false;
    return pVectors->GetEnvelope(recordIndex,pEnv);
}
int MemFeatureClass::GetFeatureID(int recordIndex){
    if(editor==nullptr)
        return pVectors->GetFeatureID(recordIndex);
    else
        return editor->GetFeatureID(recordIndex);
}
Variant MemFeatureClass::GetFieldValue(int recordIndex,int FieldIndex){
    Variant var;
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)){
        if(editor->GetFieldValue(recordIndex,FieldIndex,var)) return var;
        recordIndex=editor->GetFeatureID(recordIndex);
    }
    if(pVectors==nullptr) return var;
    return pVectors->GetFieldValue(recordIndex,FieldIndex);
}
double MemFeatureClass::GetFieldValueAsDouble(int recordIndex,int FieldIndex){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)){
        double var;
        if(editor->GetFieldValueAsDouble(recordIndex,FieldIndex,var)) return var;
        recordIndex=editor->GetFeatureID(recordIndex);
    }
    if(pVectors==nullptr) return 0;
    return pVectors->GetFieldValueAsDouble(recordIndex,FieldIndex);
}
string MemFeatureClass::GetFieldValueAsString(int recordIndex,int FieldIndex){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)){
        string var;
        if(editor->GetFieldValueAsString(recordIndex,FieldIndex,var)) return var;
        recordIndex=editor->GetFeatureID(recordIndex);
    }
    if(pVectors==nullptr) return "";
    return pVectors->GetFieldValueAsString(recordIndex,FieldIndex);
}

bool MemFeatureClass::IsValid(){
    return (pVectors!=nullptr);
}

AutoPtr<FeatureClass>MemFeatureClass::Clone(){
    MemFeatureClass*pfc=new MemFeatureClass();
    pfc->name=name;
    pfc->HistoryCount=HistoryCount;
    pfc->pSelectionSet=pSelectionSet->Clone();
    pfc->is3d=is3d;
    if(editor!=nullptr) pfc->editor=editor->Copy();
    pfc->pVectors=pVectors;
    return pfc;
}
void MemFeatureClass::Dispose(){
    editor=nullptr;
    pVectors=nullptr;
    pVersionManager=nullptr;
}

//FeatureDataset:
string MemFeatureClass::GetName(){
    return name;
}
string MemFeatureClass::GetTypeName(){
    return "MemFeatureClass";
}
AutoPtr<Workspace>MemFeatureClass::GetWorkspace(){
   return new MemFeatureWorkspace();
}
int MemFeatureClass::GetFeatureClassCount(){
    return 1;
}
AutoPtr<FeatureClass>MemFeatureClass::GetFeatureClass(int nIndex){
    if(nIndex!=0) return nullptr;
    return AutoPtr<FeatureClass>(this,true);
}
AutoPtr<FeatureClass>MemFeatureClass::GetDefaultFeatureClass(){
    return AutoPtr<FeatureClass>(this,true);
}
string MemFeatureClass::GetFeatureClassName(int nIndex){
    if(nIndex==0) return name;
    return "";
}
int MemFeatureClass::FindFeatureClass(string Name){
    if(StringHelper::EqualsNoCase(this->name,Name)) return 0;
    return -1;
}

bool MemFeatureClass::TemporaryDataset(){
    return true;
}

//FeatureClassEdit:
bool MemFeatureClass::IsEditing(){
    return (editor!=nullptr);
}
bool MemFeatureClass::StartEditing(){
    if(editor!=nullptr) return false;
    editor=new FeatureClassEditor();
    editor->SetHistoryCount(HistoryCount);
    editor->BeginEdit(this);
    ClearStatistics();
    return true;
}

bool MemFeatureClass::SaveEdit(CallBack*callBack)
{
    if(editor==nullptr) return false;
    editor->ResetSelectionSet(pSelectionSet.get(),true);
    bool IsOk=editor->SaveEdit(this,callBack);
    if(IsOk)
    {
        editor=nullptr;
        StartEditing();
    }
    ClearStatistics();
    return true;
}

bool MemFeatureClass::Transaction(CallBack*callBack){
    return SaveEdit(callBack);
}

bool MemFeatureClass::StopEdits(bool bSaveEdit,CallBack*callBack){
    if(editor==nullptr) return false;
    if(bSaveEdit)
    {
        editor->ResetSelectionSet(pSelectionSet.get(),true);
        //if(psp==nullptr) GetSpatialReference();
        return SaveEdit(callBack);
    }
    else
    {
        pSelectionSet->Clear();
    }
    if(editor!=nullptr) editor=nullptr;
    ClearStatistics();
    return true;
}
bool MemFeatureClass::SetFeature(LONG RecordIndex,AutoPtr<Geometry>geo){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        pVectors->SetGeometry(RecordIndex,geo);
        return true;
    }
    editor->SetFeature(RecordIndex,geo->Clone());
    return true;
}
bool MemFeatureClass::SetFieldValue(int RecordIndex,int FieldIndex,Variant newVal){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        pVectors->SetFieldValue(RecordIndex,FieldIndex,newVal);
        return true;
    }
    return editor->SetFieldValue(RecordIndex,FieldIndex,newVal);
}
bool MemFeatureClass::SetFieldValueWithString(int RecordIndex,int FieldIndex,string newVal){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        pVectors->SetFieldValue(RecordIndex,FieldIndex,newVal);
        return true;
    }
    return editor->SetFieldValue(RecordIndex,FieldIndex,newVal);
}
bool MemFeatureClass::AddFeature(AutoPtr<Geometry>pGeo){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        return pVectors->AddRecord(pGeo);
    }
    else
    {
        if(editor->AddFeature(pGeo)){
            ClearStatistics();
            return true;
        }
        return false;
    }
}
bool MemFeatureClass::AddFeatureEx(AutoPtr<Geometry>pGeo,const vector<Variant>&values){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        return pVectors->AddRecord(pGeo,values);
    }
    else
    {
        if(editor->AddFeature(pGeo,values)){
            ClearStatistics();
            return true;
        }
        return false;
    }
}

bool MemFeatureClass::AddFeature(int fid,AutoPtr<Geometry>pGeo){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        return pVectors->AddRecord(fid,pGeo);
    }
    else
    {
        return false;
    }
}
bool MemFeatureClass::AddFeatureEx(int fid,AutoPtr<Geometry>pGeo,const vector<Variant>&values){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        return pVectors->AddRecord(fid,pGeo,values);
    }
    else
    {
        return false;
    }
}

bool MemFeatureClass::Save(string pathName){
    AutoPtr<TableDesc>desc=this->GetTableDesc();
    AutoPtr<ShapefileWorkspaceFactory>pFac(new ShapefileWorkspaceFactory());
    AutoPtr<ShapefileWorkspace>pWork=pFac->OpenFromFile(FilePath::GetDir(pathName));
    if(pWork==nullptr) return false;
    AutoPtr<ShapefileFeatureClass>newfc=pWork->CreateFeatureClass(FilePath::GetFileName(pathName),this->GetShapeType(),desc.get(),this->psp.get());
    if(newfc==nullptr) return false;
    int featureCount=this->GetFeatureCount();
    AutoPtr<FeatureClassEdit>pEdit(newfc);
    int fieldCount=desc->GetFieldCount();
    vector<Variant>values;
    values.resize(fieldCount);
    for(int k=0;k<featureCount;k++){
        AutoPtr<Geometry>geo=this->GetFeature(k);
        for(int j=0;j<fieldCount;j++) values[j]=this->GetFieldValue(k,j);
        pEdit->AddFeatureEx(geo,values);
    }
    newfc->Dispose();
    return true;
}

bool MemFeatureClass::IsFromSameSource(FeatureClass*other){
    return (this==other);
}

bool MemFeatureClass::DeleteFeature(int recordIndex){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        pVectors->DeleteRecord(recordIndex);
        return true;
    }
    else
    {
        LONG FID=editor->GetFeatureID(recordIndex);
        pSelectionSet->RemoveFeatureID(FID);
        bool bSuc=editor->DeleteFeature(recordIndex);
        if(bSuc) ClearStatistics();
        return bSuc;
    }
    return false;
}
bool MemFeatureClass::DeleteAllFeatures(int recordIndex){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        pVectors->RemoveAllRecords();
        return true;
    }
    else
    {
        bool bSuc=editor->DeleteAllFeatures();
        pSelectionSet->Clear();
        ClearStatistics();
        return bSuc;
    }
}
bool MemFeatureClass::DeleteSelFeatures(CallBack*callBack){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
        return false;
    else
    {
        if(editor->DeleteSelFeatures(this,callBack)){
            pSelectionSet->Clear();
            ClearStatistics();
            return true;
        }
        return false;
    }
}
bool MemFeatureClass::CalculateField(int TargetFieldIndex,string Expression,CallBack*callBack){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
        return false;
    else
    {
        if(editor->CalculateField(this,TargetFieldIndex,Expression,callBack)){
           pStaHist.RemoveStatis(pDesc->GetFieldName(TargetFieldIndex));
           ClearStatistics();
           return true;
        }
        return false;
    }
}
bool MemFeatureClass::MoveSelection(DOUBLE difx,DOUBLE dify,CallBack*callBack){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
        return false;
    else
        return editor->MoveSelection(this,difx,dify,callBack);
}
bool MemFeatureClass::MoveSelectionByClientCoord(DOUBLE difx,DOUBLE dify,Display*pDisplay,CallBack*callBack){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
         return false;
    else
        return editor->MoveSelectionByClientCoord(this,difx,dify,pDisplay,callBack);
}
//Version:
int MemFeatureClass::GetHistoryMaxItems(){
    return HistoryCount;
}

void MemFeatureClass::SetHistoryMaxItems(int maxItems){
    if(maxItems<2)
        maxItems=2;
    else if(maxItems>1000)
        maxItems=1000;
    HistoryCount=maxItems;
    if(editor!=NULL) editor->SetHistoryCount(HistoryCount);
}

int MemFeatureClass::GetUsableUndoTimes(){
    if(editor==NULL)
        return 0;
    else
        return editor->UsableUndoTimes();
}
int MemFeatureClass::GetUsableRedoTimes(){
    if(editor==NULL)
        return 0;
    else
        return editor->UsableRedoTimes();
}
bool MemFeatureClass::UndoEdit(){
    if(editor!=nullptr){
        if(editor->UndoEdit()){
            editor->ResetSelectionSet(pSelectionSet.get(),false);
            return true;
        }
    }
    return false;
}
bool MemFeatureClass::RedoEdit(){
    if(editor!=nullptr){
        if(editor->RedoEdit()){
            editor->ResetSelectionSet(pSelectionSet.get(),false);
            return true;
        }
    }
    return false;
}
bool MemFeatureClass::BeginNewVersion(string name,bool EditChanged){
    if(editor!=nullptr){
        editor->RegisterNewVersion(name,EditChanged);
        if(pVersionManager!=NULL) pVersionManager->RegisterVersion(this);
        return true;
    }
    return false;
}
bool MemFeatureClass::GetCurrentEditChanged(){
    if(editor!=nullptr){
        return editor->GetCurrentEditChanged();
    }
    return false;
}
string MemFeatureClass::GetCurrentUndoName(){
    if(editor!=nullptr){
        return editor->GetCurrentUndoName();
    }
    return "";
}
string MemFeatureClass::GetCurrentRedoName(){
    if(editor!=nullptr){
        return editor->GetCurrentRedoName();
    }
    return "";
}

bool MemFeatureClass::SetCurrentUndoName(string name){
    if(editor!=nullptr){
        return editor->SetCurrentUndoName(name);
    }
    return false;
}
bool MemFeatureClass::SetCurrentRedoName(string name){
    if(editor!=nullptr){
        return editor->SetCurrentRedoName(name);
    }
    return false;
}
bool MemFeatureClass::ClipForewardAllRedos(){
    if(editor!=nullptr){
        return editor->ClipForewardAllRedos();
    }
    return false;
}
bool MemFeatureClass::RemoveOneBackwardUndo(){
    if(editor!=nullptr){
        return editor->RemoveOneBackwardUndo();
    }
    return false;
}
VersionManager* MemFeatureClass::GetVersionManager(){
   return pVersionManager;
}
void MemFeatureClass::SetVersionManager(VersionManager*pManager){
    pVersionManager=pManager;
}

bool MemFeatureClass::AddField(string FieldName,VectorFieldType ftype,LONG FieldWidth,LONG DecimalCount,CallBack*callBack){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)) return false;
    LONG fIndex=pDesc->FindField(FieldName);
    if(fIndex>=0) return false;
    CallBackHelper::BeginProgress(callBack,"增加字段");
    pVectors->AddField(FieldName,ftype,FieldWidth,DecimalCount);
    CallBackHelper::EndProgress(callBack);
    return true;
}
bool MemFeatureClass::DeleteField(LONG FieldIndex,CallBack*callBack){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)) return false;
    CallBackHelper::BeginProgress(callBack,"删除字段");
    pVectors->DeleteField(FieldIndex);
    CallBackHelper::EndProgress(callBack);
    return true;
}

MemFeatureWorkspace::MemFeatureWorkspace(){

}
MemFeatureWorkspace::~MemFeatureWorkspace(){

}
//FeatureWorkspace:
string MemFeatureWorkspace::GetDescription(){
    return "内存矢量工作空间";
}

bool MemFeatureWorkspace::IsDatasetFeatureClass(string name){
    return true;
}

AutoPtr<MemFeatureClass>MemFeatureWorkspace::CreateFeatureClass(string name,VectorShapeType type,TableDesc*pDesc,SpatialReference*psp){
    if(type==vstUnknown) return nullptr;
    MemFeatureClass*pFeatureClass=new MemFeatureClass();
    pFeatureClass->InitialData(name,type,pDesc,psp,false);
    return pFeatureClass;
}

AutoPtr<MemFeatureClass>MemFeatureWorkspace::Create3DFeatureClass(string name,VectorShapeType type,TableDesc*pDesc,SpatialReference*psp){
    if(type==vstUnknown) return nullptr;
    MemFeatureClass*pFeatureClass=new MemFeatureClass();
    pFeatureClass->InitialData(name,type,pDesc,psp,true);
    return pFeatureClass;
}

MemFeatureWorkspaceFactory::MemFeatureWorkspaceFactory(){

}
MemFeatureWorkspaceFactory::~MemFeatureWorkspaceFactory(){

}

//FeatureWorkspaceFactory:
string MemFeatureWorkspaceFactory::GetDescription(){
    return "Memory Feature";
}
AutoPtr<WorkspaceFactory>MemFeatureWorkspaceFactory::Clone(){
   return new MemFeatureWorkspaceFactory();
}

AutoPtr<MemFeatureWorkspace>MemFeatureWorkspaceFactory::CreateMemFeatureWorkspace(){
    return new MemFeatureWorkspace();
}

}
