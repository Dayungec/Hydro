#include "shapfilefeatureclass.h"
#include "Base/FilePath.h"
#include "Base/StringHelper.h"
#include "Base/file.h"
#include <fstream>
#include "Base/classfactory.h"
#include "GDAL/geometryfactory.h"
namespace SGIS{

REGISTER(ShapefileFeatureClass)

ShapefileFeatureClass::ShapefileFeatureClass(){
    poDS=nullptr;
    poLayer=nullptr;
    poFeature=nullptr;
    FormerRecord=-1;
    FeatureCount=0;
    is3d=false;
    pVersionManager=nullptr;
    editor=nullptr;
    HistoryCount=500;
    isUtf8=false;
}

ShapefileFeatureClass::~ShapefileFeatureClass(){
    Dispose();
}
string ShapefileFeatureClass::GetDescription(){
    if(shapeType==vstPolygon)
        return "Polygon Shapefile";
    else if(shapeType==vstPolyline)
        return "Polyline Shapefile";
    else if(shapeType==vstPoint)
        return "Point Shapefile";
    return "Unknown Shapefile";
}
string ShapefileFeatureClass::GetFeatureClassName(){
    return fileName;
}
AutoPtr<FeatureDataset>ShapefileFeatureClass::GetFeatureDataset(){
    return AutoPtr<FeatureDataset>(this,true);
}

AutoPtr<Envelope>ShapefileFeatureClass::GetExtent(){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)){
        return editor->GetShapefileEnvelope();
    }
    if(poLayer==nullptr) return nullptr;
    OGREnvelope env;
    poLayer->GetExtent(&env);
    Envelope*pEnv=new Envelope();
    pEnv->PutCoord(env.MinX,env.MaxY,env.MaxX,env.MinY);
    return pEnv;
}

int ShapefileFeatureClass::GetFeatureCount(){
    if((editor==nullptr)||(editor->CurrentEdit==nullptr))
    {
        return FeatureCount;
    }
    else
        return editor->GetRecordNumber();
}
AutoPtr<Geometry>ShapefileFeatureClass::GetFeature(int recordIndex){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr))
    {
        bool bSuc;
        AutoPtr<Geometry>geo=editor->GetGeometry(recordIndex,bSuc);
        if(bSuc) return geo;
        recordIndex=editor->GetFeatureID(recordIndex);
    }
    if(poLayer==nullptr) return nullptr;
    if(!MoveTo(recordIndex)) return nullptr;
    if(poFeature!=nullptr)
    {
        return GeometryFactory::CreateGeometry(poFeature->GetGeometryRef());
    }
    return nullptr;
}
bool ShapefileFeatureClass::GetFeatureEnvelope(int recordIndex,Envelope*pEnv){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr))
    {
        if(editor->GetEnvelope(recordIndex,pEnv)) return true;
        recordIndex=editor->GetFeatureID(recordIndex);
    }
    if(poLayer==nullptr) return false;
    if(!MoveTo(recordIndex)) return false;
    if(poFeature!=NULL)
    {
        OGREnvelope env;
        OGRGeometry*geo=poFeature->GetGeometryRef();
        if(geo==nullptr) return false;
        geo->getEnvelope(&env);
        pEnv->PutCoord(env.MinX,env.MaxY,env.MaxX,env.MinY);
        return true;
    }
    return false;
}
int ShapefileFeatureClass::GetFeatureID(int recordIndex){
    if(editor==nullptr)
        return recordIndex;
    else
        return editor->GetFeatureID(recordIndex);
}
Variant ShapefileFeatureClass::GetFieldValue(int recordIndex,int FieldIndex){
    Variant var;
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)){
        if(editor->GetFieldValue(recordIndex,FieldIndex,var)) return var;
        recordIndex=editor->GetFeatureID(recordIndex);
    }
    if(poLayer==nullptr) return var;
    if(!MoveTo(recordIndex)) return var;
    VectorFieldType fType=pDesc->GetFieldType(FieldIndex);
    switch(fType)
    {
    case vftInteger:
        var=poFeature->GetFieldAsInteger(FieldIndex);
        return var;
    case vftReal:
        var=poFeature->GetFieldAsDouble(FieldIndex);
        return var;
    case vftString:{
        string sValue=poFeature->GetFieldAsString(FieldIndex);
        if(!isUtf8)
            sValue=StringHelper::FromLocalString(sValue);
        var=sValue;
        return var;
    }
    case vftDate:
    {
        string sValue=poFeature->GetFieldAsString(FieldIndex);
        if(!isUtf8)
            sValue=StringHelper::FromLocalString(sValue);
        if((sValue=="1899-12-30")||(sValue=="1899/12/30"))
            sValue= "00:00:00";
        var=sValue;
        return var;
        /*
        int pYear,pMonth,pDay,pHour,pMinute,pSecond;
        poFeature->GetFieldAsDateTime(FieldIndex,&pYear,&pMonth,&pDay,&pHour,&pMinute,&pSecond,nullptr);
        Date dTime(pYear,pMonth,pDay);
        var=dTime;
        return var;
        */
    }
    case vftTime:
    case vftDateTime:
    {
        int pYear,pMonth,pDay,pHour,pMinute,pSecond;
        poFeature->GetFieldAsDateTime(FieldIndex,&pYear,&pMonth,&pDay,&pHour,&pMinute,&pSecond,nullptr);
        DateTime dTime(pYear,pMonth,pDay,pHour,pMinute,pSecond);
        var=dTime;
        return var;
    }
    }
    return var;
}
double ShapefileFeatureClass::GetFieldValueAsDouble(int recordIndex,int FieldIndex){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)){
        double var;
        if(editor->GetFieldValueAsDouble(recordIndex,FieldIndex,var)) return var;
        recordIndex=editor->GetFeatureID(recordIndex);
    }
    if(poLayer==nullptr) return 0;
    if(!MoveTo(recordIndex)) return 0;
    VectorFieldType fType=pDesc->GetFieldType(FieldIndex);
    switch(fType)
    {
    case vftInteger:
    case vftReal:
        return poFeature->GetFieldAsDouble(FieldIndex);
    case vftString:{
        string sValue=poFeature->GetFieldAsString(FieldIndex);
        if(!isUtf8) sValue=StringHelper::FromLocalString(sValue);
        return atof(sValue.c_str());
    }
    case vftDate:
    case vftTime:
    case vftDateTime:
    {
        int pYear,pMonth,pDay,pHour,pMinute,pSecond;
        poFeature->GetFieldAsDateTime(FieldIndex,&pYear,&pMonth,&pDay,&pHour,&pMinute,&pSecond,nullptr);
        DateTime dTime(pYear,pMonth,pDay,pHour,pMinute,pSecond);
        return dTime.ToDouble();
    }
    }
    return 0;
}
string ShapefileFeatureClass::GetFieldValueAsString(int recordIndex,int FieldIndex){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)){
        string var;
        if(editor->GetFieldValueAsString(recordIndex,FieldIndex,var)) return var;
        recordIndex=editor->GetFeatureID(recordIndex);
    }
    if(poLayer==nullptr) return "";
    if(!MoveTo(recordIndex)) return "";
    if(poFeature->IsFieldNull(FieldIndex)) return "";
    VectorFieldType fType=pDesc->GetFieldType(FieldIndex);
    switch(fType)
    {
    case vftInteger:
        return to_string(poFeature->GetFieldAsInteger(FieldIndex));
    case vftReal:
        return to_string(poFeature->GetFieldAsDouble(FieldIndex));
    case vftString:{
        string sValue=poFeature->GetFieldAsString(FieldIndex);
        if(!isUtf8) sValue=StringHelper::FromLocalString(sValue);
        return sValue;
    }
    case vftDate:
    {
        string sValue=poFeature->GetFieldAsString(FieldIndex);
        if(!isUtf8) sValue=StringHelper::FromLocalString(sValue);
        if((sValue=="1899-12-30")||(sValue=="1899/12/30"))
            sValue= "00:00:00";
        return sValue;
        /*
        int pYear,pMonth,pDay,pHour,pMinute,pSecond;
        poFeature->GetFieldAsDateTime(FieldIndex,&pYear,&pMonth,&pDay,&pHour,&pMinute,&pSecond,nullptr);
        if((pYear=1899)&&(pMonth==1)&&(pDay==1)){
             return StringHelper::Format("%02d:%02d:%02d",pHour,pMinute,pSecond);
        }
        else{
             Date dTime(pYear,pMonth,pDay);
             return dTime.ToString();
        }
        */
    }
    case vftTime:
    {
        int pYear,pMonth,pDay,pHour,pMinute,pSecond;
        poFeature->GetFieldAsDateTime(FieldIndex,&pYear,&pMonth,&pDay,&pHour,&pMinute,&pSecond,nullptr);
        return StringHelper::Format("%02d:%02d:%02d",pHour,pMinute,pSecond);
    }
    case vftDateTime:
    {
        int pYear,pMonth,pDay,pHour,pMinute,pSecond;
        poFeature->GetFieldAsDateTime(FieldIndex,&pYear,&pMonth,&pDay,&pHour,&pMinute,&pSecond,nullptr);
        DateTime dTime(pYear,pMonth,pDay,pHour,pMinute,pSecond);
        return dTime.ToString(tstFull);
    }
    }
    return "";
}

bool ShapefileFeatureClass::IsValid(){
    return (poLayer!=nullptr);
}

AutoPtr<FeatureClass>ShapefileFeatureClass::Clone(){
    ShapefileFeatureClass*pfc=new ShapefileFeatureClass();
    pfc->dir=dir;
    pfc->fileName=fileName;
    pfc->name=name;
    pfc->HistoryCount=HistoryCount;
    pfc->isUtf8=isUtf8;
    pfc->ResetReading();
    pfc->pSelectionSet=pSelectionSet->Clone();
    if(editor!=nullptr) pfc->editor=editor->Copy();
    return pfc;
}

void ShapefileFeatureClass::Dispose(){
    if(poDS!=nullptr){
        GDALClose( poDS );
    }
    poDS=nullptr;
    if(poFeature!=nullptr) OGRFeature::DestroyFeature(poFeature);
    poFeature=nullptr;
    poLayer=nullptr;
    editor=nullptr;
}

void ShapefileFeatureClass::FlushCache(){
    if(editor!=nullptr) return;
    ResetReading();
}

bool ShapefileFeatureClass::IsFromSameSource(FeatureClass*other){
    AutoPtr<ShapefileFeatureClass>ofc=AutoPtr<FeatureClass>(other,true);
    if(ofc==nullptr) return false;
    string path1=ofc->GetPathName();
    StringHelper::TrimToLower(path1);
    string path2=this->GetPathName();
    StringHelper::TrimToLower(path2);
    return (path1==path2);
}

//FeatureDataset:
int ShapefileFeatureClass::GetFeatureClassCount(){
    return 1;
}
AutoPtr<FeatureClass>ShapefileFeatureClass::GetFeatureClass(int nIndex){
    if(nIndex!=0) return nullptr;
    return AutoPtr<FeatureClass>(this,true);
}
AutoPtr<FeatureClass>ShapefileFeatureClass::GetDefaultFeatureClass(){
    return AutoPtr<FeatureClass>(this,true);
}
string ShapefileFeatureClass::GetFeatureClassName(int nIndex){
    if(nIndex==0) return fileName;
    return "";
}
int ShapefileFeatureClass::FindFeatureClass(string Name){
   if(StringHelper::EqualsNoCase(fileName,Name)) return 0;
   return -1;
}
//FileDataset:
bool ShapefileFeatureClass::IsDirectory(){
    return false;
}
string ShapefileFeatureClass::GetPathName(){
    return this->dir+"/"+this->fileName+".shp";
}

bool ShapefileFeatureClass::SetSpatialReference(AutoPtr<SpatialReference>psp){
    if(poDS==nullptr) return false;
    string wkt=psp->ExportToWkt();
    if(this->update){
        poDS->SetProjection(wkt.c_str());
    }
    else{
        this->ResetReading(true);
        poDS->SetProjection(wkt.c_str());
        this->ResetReading(false);
    }
    return FeatureClass::SetSpatialReference(psp);
}

bool ShapefileFeatureClass::ResetReading(bool update){
    Dispose();
    pSelectionSet->Clear();
    psp=nullptr;
    string fullPath=dir+"/"+fileName+".shp";
    fullPath=StringHelper::ToLocalString(fullPath);
    if(!update)
        poDS  =  (OGRDataSource*) GDALOpenEx(fullPath.c_str(), GDAL_OF_VECTOR|GDAL_OF_READONLY, NULL, NULL, NULL );
    else
        poDS  =  (OGRDataSource*) GDALOpenEx(fullPath.c_str(), GDAL_OF_VECTOR|GDAL_OF_UPDATE, NULL, NULL, NULL );
    if( poDS == nullptr ) return false;
    string layerName=StringHelper::ToLocalString(fileName);
    poLayer=poDS->GetLayerByName(layerName.c_str());
    if(poLayer==nullptr)
    {
        if(poDS!=NULL) GDALClose(poDS);
        poDS=nullptr;
        return false;
    }
    OGRwkbGeometryType gType=poLayer->GetLayerDefn()->GetGeomType();
    switch(gType)
    {
    case wkbPoint:
    case wkbMultiPoint:
    case wkbPoint25D:
    case wkbMultiPoint25D:
        shapeType=vstPoint;break;
    case wkbLineString:
    case wkbMultiLineString:
    case wkbLineString25D:
    case wkbMultiLineString25D:
        shapeType=vstPolyline;break;
    case wkbPolygon:
    case wkbMultiPolygon:
    case wkbLinearRing:
    case wkbPolygon25D:
    case wkbMultiPolygon25D:
        shapeType=vstPolygon;break;
    case wkbGeometryCollection:
    case wkbGeometryCollection25D:
        shapeType=vstCollection;break;
    }
    switch(gType)
    {
    case wkbPoint25D:
    case wkbMultiPoint25D:
    case wkbLineString25D:
    case wkbMultiLineString25D:
    case wkbPolygon25D:
    case wkbMultiPolygon25D:
    case wkbGeometryCollection25D:
        is3d=true;
        break;
    }
    ClearStatistics();
    poLayer->ResetReading();
    OGRFeatureDefn*poFDefn = poLayer->GetLayerDefn();
    AutoPtr<TableDescCreator>pCreator(new TableDescCreator());
    int Count=poFDefn->GetFieldCount();
    for(int k=0;k<Count;k++)
    {
        OGRFieldDefn*pDef=poFDefn->GetFieldDefn(k);
        string sName=pDef->GetNameRef();
        if(!isUtf8) sName=StringHelper::FromLocalString(sName);
        VectorFieldType fType=FieldTypeHelper::FromOGRFieldType(pDef->GetType());
        int Width=pDef->GetWidth();
        int Pre=pDef->GetPrecision();
        pCreator->AddField(FieldDesp(sName,fType,Width,Pre));
    }
    pDesc=pCreator->CreateTableDesc();
    if(name.empty()){
        name=fileName;
    }
    FormerRecord=-1;
    FeatureCount=poLayer->GetFeatureCount();
    this->update=update;
    if(psp==nullptr){
        psp=new SpatialReference();
        OGRSpatialReference*pSpatial=poLayer->GetSpatialRef();
        if(pSpatial!=nullptr)
        {
            char*pszWKT = NULL;
            OGRErr pErr=pSpatial->exportToWkt(&pszWKT);
            if(pErr==OGRERR_NONE)
            {
                if(pszWKT!=nullptr)
                {
                    psp->ImportFromWkt(pszWKT);
                }
            }
        }
    }
    return true;
}

bool ShapefileFeatureClass::MoveTo(LONG RecordIndex){
    if(poLayer==nullptr) return false;
    if(FormerRecord==RecordIndex) return true;
    if(FormerRecord!=RecordIndex-1)
    {
        poLayer->ResetReading();
        poLayer->SetNextByIndex(RecordIndex);
    }
    FormerRecord=RecordIndex;
    if(poFeature!=nullptr) OGRFeature::DestroyFeature(poFeature);
    poFeature=nullptr;
    poFeature=poLayer->GetNextFeature();
    return true;
}

bool ShapefileFeatureClass::OpenFromFile(string pathName){
    dir=FilePath::GetDir(pathName);
    fileName=FilePath::GetFileNameNoExa(pathName);
    string fullPath=dir+"/"+fileName+".shp";
    bool bExist=FilePath::IsFileExisting(fullPath);
    if(!bExist){
        return false;
    }
    string cpgPath=dir+"/"+fileName+".cpg";
    string text="";
    if(TextFile::Read(cpgPath,text)){
        StringHelper::TrimToLower(text);
        if(text=="utf-8")
            isUtf8=true;
    }
    return ResetReading();
}

AutoPtr<Workspace>ShapefileFeatureClass::GetWorkspace(){
    ShapefileWorkspace*work=new ShapefileWorkspace(dir);
    return work;
}

string ShapefileFeatureClass::GetName(){
    return name;
}
bool ShapefileFeatureClass::TemporaryDataset(){
    return false;
}

//FeatureClassEdit:
bool ShapefileFeatureClass::IsEditing(){
    return (editor!=nullptr);
}
bool ShapefileFeatureClass::StartEditing(){
    if(editor!=nullptr) return false;
    editor=new FeatureClassEditor();
    editor->SetHistoryCount(HistoryCount);
    editor->BeginEdit(this);
    ClearStatistics();
    return true;
}

bool ShapefileFeatureClass::SaveEdit(string PathName,SpatialReference*psp,CallBack*callBack)
{
    string dir=FilePath::GetDir(PathName);
    string fileName=FilePath::GetFileNameNoExa(PathName);
    GDALDataset *poDS;
    OGRLayer *poLayer=NULL;
    OGRFeatureDefn*fDef;
    VectorShapeType ShapeKind=this->GetShapeType();
    string tempFilePath=dir+"/~"+fileName+".shp";
    GDALDriver *poDriver;
    poDriver =  GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    poDS = poDriver->Create(StringHelper::ToLocalString(tempFilePath).c_str(),0, 0, 0, GDT_Unknown, NULL );
    if(poDS == nullptr ) return false;
    OGRSpatialReference sp;
    if(psp!=NULL)
    {
        string bV=psp->ExportToWkt();
        if(!bV.empty())
        {
            sp.importFromWkt(bV.c_str());
        }
    }
    string saveFileName=StringHelper::ToLocalString("~"+fileName+".shp");
    if(ShapeKind==vstPoint)
        poLayer = poDS->CreateLayer(saveFileName.c_str(), &sp, wkbPoint, NULL );
    else if(ShapeKind==vstPolyline)
        poLayer = poDS->CreateLayer(saveFileName.c_str(), &sp, wkbLineString, NULL );
    else if(ShapeKind==vstPolygon)
        poLayer = poDS->CreateLayer(saveFileName.c_str(), &sp, wkbMultiPolygon, NULL );
    if(poLayer==NULL)
    {
        //OGRDataSource::DestroyDataSource( poDS );
        GDALClose( poDS );
        poDS=nullptr;
        return false;
    }
    int FieldCount=pDesc->GetFieldCount();
    for(int k=0;k<FieldCount;k++)
    {
        string bf = pDesc->GetFieldName(k);
        OGRFieldType type;
        switch(pDesc->GetFieldType(k))
        {
        case VectorFieldType::vftInteger:
            type=OFTInteger;break;
        case VectorFieldType::vftReal:
            type=OFTReal;break;
        case VectorFieldType::vftString:
            type=OFTString;break;
        case VectorFieldType::vftDate:
        case VectorFieldType::vftDateTime:
        case VectorFieldType::vftTime:
            type=OFTDate;break;
        default:
            type=OFTInteger;break;
        }
        OGRFieldDefn oField(StringHelper::ToLocalString(bf).c_str(),type);
        oField.SetWidth(pDesc->GetFieldWidth(k));
        oField.SetPrecision(pDesc->GetFieldPrecision(k));
        OGRErr pErr=poLayer->CreateField(&oField);
        if(pErr!=OGRERR_NONE)
        {
            if(poDS!=NULL) GDALClose( poDS );
            poDS=nullptr;
            return false;
        }
    }
    fDef=poLayer->GetLayerDefn();
    LONG Count=GetFeatureCount();
    CallBackHelper::BeginProgress(callBack,"保存编辑");
    for(int k=0;k<Count;k++)
    {
        AutoPtr<Geometry>geo=GetFeature(k);
        if(geo==NULL) continue;
        OGRGeometry*pGeo=GDALGeometryFactory::ConvertGeometry(geo.get());
        if(pGeo==nullptr) continue;
        OGRFeature *poFeature;
        poFeature = OGRFeature::CreateFeature(fDef);
        OGRErr pErr=poFeature->SetGeometryDirectly(pGeo);
        if(pErr!=OGRERR_NONE)
        {
            OGRFeature::DestroyFeature( poFeature );
            continue;
        }
        for(int j=0;j<FieldCount;j++)
        {
            Variant value=GetFieldValue(k,j);
            poFeature->SetField(j,StringHelper::ToLocalString(value.asString()).c_str());
        }
        pErr=poLayer->CreateFeature(poFeature);
        if(pErr!=OGRERR_NONE)
        {
            OGRFeature::DestroyFeature( poFeature );
            continue;
        }
        OGRFeature::DestroyFeature( poFeature );
        CallBackHelper::SendProgress(callBack,(float)k/Count*100);
    }
    if(poDS!=nullptr) GDALClose( poDS );//OGRDataSource::DestroyDataSource( poDS );
    poDS=nullptr;
    Dispose();
    if(!ShapefileWorkspace::RemoveShapefile(PathName))
    {
        CallBackHelper::EndProgress(callBack);
        ResetReading();
        return false;
    }
    string tempFileName=this->dir+"/~"+fileName;
    string targetFileName=this->dir+"/"+fileName;
    FilePath::CopyFile(tempFileName+".shp",targetFileName+".shp");
    FilePath::CopyFile(tempFileName+".dbf",targetFileName+".dbf");
    FilePath::CopyFile(tempFileName+".shx",targetFileName+".shx");
    FilePath::CopyFile(tempFileName+".prj",targetFileName+".prj");
    ShapefileWorkspace::RemoveShapefile(tempFileName+".shp");
    ResetReading();
    CallBackHelper::EndProgress(callBack);
    return true;
}

bool ShapefileFeatureClass::SaveEdit(CallBack*callBack){
    if(editor==nullptr) return false;
    editor->ResetSelectionSet(pSelectionSet.get(),true);
    bool IsOk=SaveEdit(dir+"/"+fileName+".shp",psp.get(),callBack);
    if(IsOk)
    {
        if(editor!=NULL) editor=nullptr;
        editor=nullptr;
        StartEditing();
    }
    ClearStatistics();
}

bool ShapefileFeatureClass::Transaction(CallBack*callBack){
    return SaveEdit(callBack);
}

bool ShapefileFeatureClass::StopEdits(bool bSaveEdit,CallBack*callBack){
    if(editor==nullptr) return false;
    if(bSaveEdit) 
    {
        editor->ResetSelectionSet(pSelectionSet.get(),true);
        //if(psp==nullptr) GetSpatialReference();
        return SaveEdit(dir+"\\"+fileName+".shp",psp.get(),callBack);
    }
    else
    {
        pSelectionSet->Clear();
    }
    if(editor!=nullptr) editor=nullptr;
    ClearStatistics();
    return true;
}
bool ShapefileFeatureClass::SetFeature(LONG RecordIndex,AutoPtr<Geometry>geo){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        if(!update){
            if(!ResetReading(true)) return false;
        }
        OGRFeature*oFeature=poLayer->GetFeature(RecordIndex);
        if(oFeature==nullptr) return false;
        OGRGeometry*oggeo=GDALGeometryFactory::ConvertGeometry(geo.get());
        OGRErr er=oFeature->SetGeometryDirectly(oggeo); 
        if(er==OGRERR_NONE) er=poLayer->SetFeature(oFeature);
        OGRFeature::DestroyFeature(oFeature);
        return (er==OGRERR_NONE);
    }
    editor->SetFeature(RecordIndex,geo->Clone());
    return true;
}
bool ShapefileFeatureClass::SetFieldValue(int RecordIndex,int FieldIndex,Variant newVal){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        if(!update){
            if(!ResetReading(true)) return false;
        }
        OGRFeature*oFeature=poLayer->GetFeature(RecordIndex);
        if(oFeature==nullptr) return false;
        VectorFieldType vType=pDesc->GetFieldType(FieldIndex);
        switch(vType)
        {
        case vftInteger:
        {
            newVal.ChangeType(VRT_I4);
            oFeature->SetField(FieldIndex,(int)newVal.lVal);
            break;
        }
        case vftReal:
        {
            newVal.ChangeType(VRT_R8);
            oFeature->SetField(FieldIndex,newVal.dblVal);
            break;
        }
        case vftString:
        {
            newVal.ChangeType(VRT_BSTR);
            string ls=*newVal.bstrVal;
            if(this->isUtf8)
                 oFeature->SetField(FieldIndex,ls.c_str());
            else
                 oFeature->SetField(FieldIndex,StringHelper::ToLocalString(ls).c_str());
            break;
        }
        case vftDate:
        {
            newVal.ChangeType(VRT_DATE);
            Date dTime(newVal.date);
            oFeature->SetField(FieldIndex,dTime.GetYear(),dTime.GetMonth(),dTime.GetDay());
            break;
        }
        case vftTime:
        case vftDateTime:
        {
            newVal.ChangeType(VRT_DATETIME);
            DateTime dTime(newVal.date);
            oFeature->SetField(FieldIndex,dTime.GetYear(),dTime.GetMonth(),dTime.GetDay(),dTime.GetHour(),dTime.GetMinute(),dTime.GetSecond());
            break;
        }
        }
        OGRErr er=poLayer->SetFeature(oFeature);
        OGRFeature::DestroyFeature(oFeature);
        return (er==OGRERR_NONE);
    }
    return editor->SetFieldValue(RecordIndex,FieldIndex,newVal);
}
bool ShapefileFeatureClass::SetFieldValueWithString(int RecordIndex,int FieldIndex,string newVal){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        if(!update){
            if(!ResetReading(true)) return false;
        }
        OGRFeature*oFeature=poLayer->GetFeature(RecordIndex);
        if(oFeature==nullptr) return false;
        VectorFieldType vType=pDesc->GetFieldType(FieldIndex);
        switch(vType)
        {
        case vftInteger:
        {
            oFeature->SetField(FieldIndex,atoi(newVal.c_str()));
            break;
        }
        case vftReal:
        {
            oFeature->SetField(FieldIndex,atof(newVal.c_str()));
            break;
        }
        case vftString:
        {
            if(this->isUtf8)
                oFeature->SetField(FieldIndex,newVal.c_str());
            else
                oFeature->SetField(FieldIndex,StringHelper::ToLocalString(newVal).c_str());
            break;
        }
        case vftDate:
        {
            Date dTime;
            dTime.FromString(newVal);
            oFeature->SetField(FieldIndex,dTime.GetYear(),dTime.GetMonth(),dTime.GetDay(),0,0,0);
            break;
        }
        case vftTime:
        case vftDateTime:
        {
            DateTime dTime;
            dTime.FromString(newVal);
            oFeature->SetField(FieldIndex,dTime.GetYear(),dTime.GetMonth(),dTime.GetDay(),dTime.GetHour(),dTime.GetMinute(),dTime.GetSecond());
            break;
        }
        }
        OGRErr er=poLayer->SetFeature(oFeature);
        OGRFeature::DestroyFeature(oFeature);
        return (er==OGRERR_NONE);
    }
    return editor->SetFieldValue(RecordIndex,FieldIndex,newVal);
}
bool ShapefileFeatureClass::AddFeature(AutoPtr<Geometry>pGeo){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        if(!update){
            if(!ResetReading(true)) return false;
        }
        OGRFeature *poFeature;
        OGRFeatureDefn*fDef=poLayer->GetLayerDefn();
        poFeature = OGRFeature::CreateFeature(fDef);
        OGRGeometry*geo=GDALGeometryFactory::ConvertGeometry(pGeo.get());
        OGRErr pErr=poFeature->SetGeometryDirectly(geo);
        if(pErr!=OGRERR_NONE)
        {
            OGRFeature::DestroyFeature( poFeature );
            return false;
        }
        OGRErr er=poLayer->CreateFeature(poFeature);
        OGRFeature::DestroyFeature(poFeature);
        ClearStatistics();
        if(er==OGRERR_NONE) FeatureCount++;
        return (er==OGRERR_NONE);
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
bool ShapefileFeatureClass::AddFeatureEx(AutoPtr<Geometry>pGeo,const vector<Variant>&values){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        if(!update){
            if(!ResetReading(true)) return false;
        }
        OGRFeature *poFeature;
        OGRFeatureDefn*fDef=poLayer->GetLayerDefn();
        poFeature = OGRFeature::CreateFeature(fDef);
        OGRGeometry*geo=GDALGeometryFactory::ConvertGeometry(pGeo.get());
        OGRErr pErr=poFeature->SetGeometryDirectly(geo);
        if(pErr!=OGRERR_NONE)
        {
            OGRFeature::DestroyFeature( poFeature );
            return false;
        }
        LONG FieldCount=pDesc->GetFieldCount();
        for(int j=0;j<FieldCount;j++)
        {
            Variant value=values[j];
            VectorFieldType vType=pDesc->GetFieldType(j);
            switch(vType)
            {
            case vftInteger:
            {
                value.ChangeType(VRT_I4);
                poFeature->SetField(j,(int)value.lVal);
                break;
            }
            case vftReal:
            {
                value.ChangeType(VRT_R8);
                poFeature->SetField(j,value.dblVal);
                break;
            }
            case vftString:{
                value.ChangeType(VRT_BSTR);
                string ls=*value.bstrVal;
                if(this->isUtf8)
                    poFeature->SetField(j,ls.c_str());
                else
                    poFeature->SetField(j,StringHelper::ToLocalString(ls).c_str());
                break;
            }
            case vftDate:
            {
                value.ChangeType(VRT_DATE);
                Date dTime(value.date);
                poFeature->SetField(j,dTime.GetYear(),dTime.GetMonth(),dTime.GetDay(),0,0,0);
                break;
            }
            case vftTime:
            case vftDateTime:
            {
                value.ChangeType(VRT_DATETIME);
                DateTime dTime(value.date);
                poFeature->SetField(j,dTime.GetYear(),dTime.GetMonth(),dTime.GetDay(),dTime.GetHour(),dTime.GetMinute(),dTime.GetSecond());
                break;
            }
            }
        }
        OGRErr er=poLayer->CreateFeature(poFeature);
        OGRFeature::DestroyFeature(poFeature);
        ClearStatistics();
        if(er==OGRERR_NONE) FeatureCount++;
        return (er==OGRERR_NONE);
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
bool ShapefileFeatureClass::DeleteFeature(int recordIndex){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        if(!update){
            if(!ResetReading(true)) return false;
        }
        LONG FID=recordIndex;
        pSelectionSet->RemoveFeatureID(FID);
        ClearStatistics();
        bool bSuc=(OGRERR_NONE==poLayer->DeleteField(recordIndex));
        if(bSuc) FeatureCount--;
        FormerRecord=-1;
        if(poFeature!=nullptr) OGRFeature::DestroyFeature(poFeature);
        poFeature=nullptr;
        return bSuc;
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
bool ShapefileFeatureClass::DeleteAllFeatures(int recordIndex){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
    {
        if(!update){
            if(!ResetReading(true)) return false;
        }
        for(int k=poLayer->GetFeatureCount()-1;k>=0;k--) poLayer->DeleteFeature(k);
        pSelectionSet->Clear();
        ClearStatistics();
        FeatureCount=0;
        FormerRecord=-1;
        if(poFeature!=nullptr) OGRFeature::DestroyFeature(poFeature);
        poFeature=nullptr;
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
bool ShapefileFeatureClass::DeleteSelFeatures(CallBack*callBack){
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
bool ShapefileFeatureClass::CalculateField(int TargetFieldIndex,string Expression,CallBack*callBack){
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
bool ShapefileFeatureClass::MoveSelection(DOUBLE difx,DOUBLE dify,CallBack*callBack){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
        return false;
    else
        return editor->MoveSelection(this,difx,dify,callBack);
}
bool ShapefileFeatureClass::MoveSelectionByClientCoord(DOUBLE difx,DOUBLE dify,Display*pDisplay,CallBack*callBack){
    bool noediting=(editor==nullptr);
    if(!noediting) noediting=(editor->CurrentEdit==nullptr);
    if(noediting)
         return false;
    else
        return editor->MoveSelectionByClientCoord(this,difx,dify,pDisplay,callBack);
}
//Version:
int ShapefileFeatureClass::GetHistoryMaxItems(){
    return HistoryCount;
}

void ShapefileFeatureClass::SetHistoryMaxItems(int maxItems){
    if(maxItems<2)
        maxItems=2;
    else if(maxItems>1000)
        maxItems=1000;
    HistoryCount=maxItems;
    if(editor!=NULL) editor->SetHistoryCount(HistoryCount);
}

int ShapefileFeatureClass::GetUsableUndoTimes(){
    if(editor==NULL)
        return 0;
    else
        return editor->UsableUndoTimes();
}
int ShapefileFeatureClass::GetUsableRedoTimes(){
    if(editor==NULL)
        return 0;
    else
        return editor->UsableRedoTimes();
}
bool ShapefileFeatureClass::UndoEdit(){
    if(editor!=nullptr){
        if(editor->UndoEdit()){
            editor->ResetSelectionSet(pSelectionSet.get(),false);
            return true;
        }
    }
    return false;
}
bool ShapefileFeatureClass::RedoEdit(){
    if(editor!=nullptr){
        if(editor->RedoEdit()){
            editor->ResetSelectionSet(pSelectionSet.get(),false);
            return true;
        }
    }
    return false;
}
bool ShapefileFeatureClass::BeginNewVersion(string name,bool EditChanged){
    if(editor!=nullptr){
        editor->RegisterNewVersion(name,EditChanged);
        if(pVersionManager!=NULL) pVersionManager->RegisterVersion(this);
        return true;
    }
    return false;
}
bool ShapefileFeatureClass::GetCurrentEditChanged(){
    if(editor!=nullptr){
        return editor->GetCurrentEditChanged();
    }
    return false;
}
string ShapefileFeatureClass::GetCurrentUndoName(){
    if(editor!=nullptr){
        return editor->GetCurrentUndoName();
    }
    return "";
}
string ShapefileFeatureClass::GetCurrentRedoName(){
    if(editor!=nullptr){
        return editor->GetCurrentRedoName();
    }
    return "";
}
bool ShapefileFeatureClass::SetCurrentUndoName(string name){
    if(editor!=nullptr){
        return editor->SetCurrentUndoName(name);
    }
    return false;
}
bool ShapefileFeatureClass::SetCurrentRedoName(string name){
    if(editor!=nullptr){
        return editor->SetCurrentRedoName(name);
    }
    return false;
}
bool ShapefileFeatureClass::ClipForewardAllRedos(){
    if(editor!=nullptr){
        return editor->ClipForewardAllRedos();
    }
    return false;
}

bool ShapefileFeatureClass::RemoveOneBackwardUndo(){
    if(editor!=nullptr){
        return editor->RemoveOneBackwardUndo();
    }
    return false;
}
VersionManager* ShapefileFeatureClass::GetVersionManager(){
   return pVersionManager;
}
void ShapefileFeatureClass::SetVersionManager(VersionManager*pManager){
    pVersionManager=pManager;
}

bool ShapefileFeatureClass::AddField(string FieldName,VectorFieldType ftype,LONG FieldWidth,LONG DecimalCount,CallBack*callBack){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)) return false;
    LONG fIndex=pDesc->FindField(FieldName);
    if(fIndex>=0) return false;
    if(!update){
        if(!ResetReading(true)) return false;
    }
    CallBackHelper::BeginProgress(callBack,"增加字段");
    OGRFieldType type=(OGRFieldType)FieldTypeHelper::ToOGRFieldType(ftype);
    OGRFieldDefn oField(StringHelper::ToLocalString(FieldName).c_str(),type);
    oField.SetWidth(FieldWidth);
    oField.SetPrecision(DecimalCount);
    OGRErr pErr=poLayer->CreateField(&oField);
    if(pErr!=OGRERR_NONE) return false;
    ResetReading();
    CallBackHelper::EndProgress(callBack);
    return true;
}
bool ShapefileFeatureClass::DeleteField(LONG FieldIndex,CallBack*callBack){
    if((editor!=nullptr)&&(editor->CurrentEdit!=nullptr)) return false;
    if(!update){
        if(!ResetReading(true)) return false;
    }
    CallBackHelper::BeginProgress(callBack,"删除字段");
    OGRErr pErr=poLayer->DeleteField(FieldIndex);
    if(pErr!=OGRERR_NONE) return false;
    ResetReading();
    CallBackHelper::EndProgress(callBack);
    return true;
}

bool ShapefileFeatureClass::Read(BufferReader*br){
    dir=br->ReadString();
    fileName=br->ReadString();
    string pathName=dir+"/"+fileName+".shp";
    FilePath::ValidFilePath(pathName,FilePath::GetDir(br->PathName()));
    this->OpenFromFile(pathName);
    return true;
}
void ShapefileFeatureClass::Write(BufferWriter*bw){
    bw->WriteString(dir);
    bw->WriteString(fileName);
}

string ShapefileFeatureClass::GetTypeName(){
    return "ShapefileFeatureClass";
}
//JsonWritable:
bool ShapefileFeatureClass::ReadFromJson(JsonReader*jr,JsonNode node){
    JsonNode dirNode=node["dir"];
    dir=dirNode.asString();
    JsonNode fileNode=node["filename"];
    fileName=fileNode.asString();
    string pathName=dir+"/"+fileName+".shp";
    FilePath::ValidFilePath(pathName,FilePath::GetDir(jr->PathName()));
    this->OpenFromFile(pathName);
    return true;
}
string ShapefileFeatureClass::ToJson(){
    JsonObjectWriter writer;
    writer.Add<string>("dir",dir);
    writer.Add<string>("filename",fileName);
    return writer.GetJsonString();
}

ShapefileWorkspace::ShapefileWorkspace(string pathName){
    this->pathName=pathName;
}
ShapefileWorkspace::~ShapefileWorkspace(){

}
//FeatureWorkspace:
string ShapefileWorkspace::GetDescription(){
    return "Shapefile工作空间";
}

//FileWorkspace:
string ShapefileWorkspace::GetPathName(){
    return pathName;
}
bool ShapefileWorkspace::IsDirectory(){
    return true;
}
vector<string>ShapefileWorkspace::GetDatasetNames(){
    FileDirectory fd(pathName);
    vector<string>names=fd.FindFileNames("*.shp");
    for(int k=0;k<names.size();k++){
        names[k]=FilePath::GetFileName(names[k]);
    }
    return names;
}
AutoPtr<FileDataset>ShapefileWorkspace::OpenDataset(string name){
    ShapefileFeatureClass*pfc=new ShapefileFeatureClass();
    if(!pfc->OpenFromFile(this->pathName+"/"+name)){
        delete pfc;
        return nullptr;
    }
    return pfc;
}

bool ShapefileWorkspace::IsDatasetFeatureClass(string name){
    return true;
}

AutoPtr<ShapefileFeatureClass>ShapefileWorkspace::CreateFeatureClass(string Name,VectorShapeType type,TableDesc*pDesc,SpatialReference*psp){
    string uName=FilePath::GetFileNameNoExa(Name);
    string sPathName=this->pathName+"/"+uName+".shp";
    uName=StringHelper::ToLocalString(uName);
    RemoveShapefile(sPathName);
    GDALDataset *poDS;
    OGRLayer *poLayer=NULL;
    VectorShapeType ShapeKind=type;
    GDALDriver *poDriver;
    poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    //OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
    poDS = poDriver->Create(StringHelper::ToLocalString(sPathName).c_str(),0, 0, 0, GDT_Unknown, NULL );
    //poDS = poDriver->CreateDataSource(sPathName, NULL );
    if(poDS == NULL )
    {
        return nullptr;
    }
    OGRSpatialReference sp;
    if(psp!=nullptr)
    {
        string bV=psp->ExportToWkt();
        if(!bV.empty())
        {
            sp.importFromWkt(bV.c_str());
        }
    }
    if(ShapeKind==vstPoint)
        poLayer = poDS->CreateLayer((uName+".shp").c_str(), &sp, wkbPoint, NULL );
    else if(ShapeKind==vstPolyline)
        poLayer = poDS->CreateLayer((uName+".shp").c_str(), &sp, wkbLineString, NULL );
    else if(ShapeKind==vstPolygon)
        poLayer = poDS->CreateLayer((uName+".shp").c_str(), &sp, wkbMultiPolygon, NULL );
    else
        return nullptr;
    if(poLayer==nullptr)
    {
        GDALClose(poDS);
        //OGRDataSource::DestroyDataSource( poDS );
        poDS=nullptr;
        return nullptr;
    }
    LONG FieldCount=pDesc->GetFieldCount();
    for(int k=0;k<FieldCount;k++)
    {
        string FieldName=pDesc->GetFieldName(k);
        OGRFieldType type;
        VectorFieldType vType=pDesc->GetFieldType(k);
        type=(OGRFieldType)FieldTypeHelper::ToOGRFieldType(vType);
        OGRFieldDefn oField(StringHelper::ToLocalString(FieldName).c_str(),type);
        LONG FieldWidth=pDesc->GetFieldWidth(k);
        oField.SetWidth(FieldWidth);
        LONG Pre=pDesc->GetFieldPrecision(k);
        oField.SetPrecision(Pre);
        OGRErr pErr=poLayer->CreateField(&oField);
        if(pErr!=OGRERR_NONE)
        {
            if(poDS!=NULL) GDALClose( poDS );
            poDS=nullptr;
            return nullptr;
        }
    }
    GDALClose( poDS );
    return OpenFeatureClass(Name);
}


AutoPtr<ShapefileFeatureClass>ShapefileWorkspace::Create3DFeatureClass(string Name,VectorShapeType type,TableDesc*pDesc,SpatialReference*psp){
    string uName=FilePath::GetFileNameNoExa(Name);
    string sPathName=this->pathName+"/"+uName+".shp";
    uName=StringHelper::ToLocalString(uName);
    RemoveShapefile(sPathName);
    GDALDataset *poDS;
    OGRLayer *poLayer=NULL;
    VectorShapeType ShapeKind=type;
    GDALDriver *poDriver;
    poDriver = GetGDALDriverManager()->GetDriverByName("ESRI Shapefile");
    //OGRSFDriverRegistrar::GetRegistrar()->GetDriverByName("ESRI Shapefile");
    poDS = poDriver->Create(StringHelper::ToLocalString(sPathName).c_str(),0, 0, 0, GDT_Unknown, NULL );
    //poDS = poDriver->CreateDataSource(sPathName, NULL );
    if(poDS == NULL )
    {
        return nullptr;
    }
    OGRSpatialReference sp;
    if(psp!=nullptr)
    {
        string bV=psp->ExportToWkt();
        if(!bV.empty())
        {
            sp.importFromWkt(bV.c_str());
        }
    }
    if(ShapeKind==vstPoint)
        poLayer = poDS->CreateLayer((uName+".shp").c_str(), &sp, wkbPoint25D, NULL );
    else if(ShapeKind==vstPolyline)
        poLayer = poDS->CreateLayer((uName+".shp").c_str(), &sp, wkbLineString25D, NULL );
    else if(ShapeKind==vstPolygon)
        poLayer = poDS->CreateLayer((uName+".shp").c_str(), &sp, wkbMultiPolygon25D, NULL );
    else
        return nullptr;
    if(poLayer==nullptr)
    {
        GDALClose(poDS);
        //OGRDataSource::DestroyDataSource( poDS );
        poDS=nullptr;
        return nullptr;
    }
    LONG FieldCount=pDesc->GetFieldCount();
    for(int k=0;k<FieldCount;k++)
    {
        string FieldName=pDesc->GetFieldName(k);
        OGRFieldType type;
        VectorFieldType vType=pDesc->GetFieldType(k);
        type=(OGRFieldType)FieldTypeHelper::ToOGRFieldType(vType);
        OGRFieldDefn oField(StringHelper::ToLocalString(FieldName).c_str(),type);
        LONG FieldWidth=pDesc->GetFieldWidth(k);
        oField.SetWidth(FieldWidth);
        LONG Pre=pDesc->GetFieldPrecision(k);
        oField.SetPrecision(Pre);
        OGRErr pErr=poLayer->CreateField(&oField);
        if(pErr!=OGRERR_NONE)
        {
            if(poDS!=NULL) GDALClose( poDS );
            poDS=nullptr;
            return nullptr;
        }
    }
    GDALClose( poDS );
    return OpenFeatureClass(Name);
}

AutoPtr<ShapefileFeatureClass>ShapefileWorkspace::OpenFeatureClass(string Name){
    if(-1==Name.find(".")) Name=Name+".shp";
    string uName=FilePath::GetFileNameNoExa(Name);
    string sPathName=this->pathName+"/"+Name;
    if(!FilePath::IsFileExisting(sPathName)){
        return nullptr;
    }
    AutoPtr<ShapefileFeatureClass>pFeatureClass(new ShapefileFeatureClass());
    if(!pFeatureClass->OpenFromFile(sPathName)) return nullptr;
    return pFeatureClass;
}
bool ShapefileWorkspace::RemoveShapefile(string path){
    string dir=FilePath::GetDir(path);
    string fileName=FilePath::GetFileNameNoExa(path);
    vector<string>stocks={"shp","dbf","shx","sbn","sbx","avl","prj","cpg","shp.xml"};
    bool existing=FilePath::IsFileExisting(dir+"/"+fileName+".shp");
    if(!existing) return false;
    for(int k=0;k<stocks.size();k++){
        string fullPath=dir+"/"+fileName+"."+stocks[k];
        if(FilePath::IsFileExisting(fullPath)){
            if(0!=remove(StringHelper::ToLocalString(fullPath).c_str())) return false;
        }
    }
    return true;
}
bool ShapefileWorkspace::CanDeleteDataset(string name){
    if(""==FilePath::GetFileExa(name)) name+=".shp";
    if(!FilePath::IsFileExisting(pathName+"/"+name)) return false;
    return true;
}
bool ShapefileWorkspace::DeleteDataset(string name){
    return RemoveShapefile(pathName+"/"+name);
}

bool ShapefileWorkspace::CanRenameDataset(string name){
    string exa=FilePath::GetFileExa(name);
    StringHelper::TrimToLower(exa);
    if(exa==""){
          if(!FilePath::IsFileExisting(pathName+"/"+name+".shp")) return false;
    }
    else{
        if(!FilePath::IsFileExisting(pathName+"/"+name)) return false;
    }
    return true;
}

bool ShapefileWorkspace::RenameDataset(string oldName,string newName){
    string dir=FilePath::GetDir(oldName);
    if(dir!="") return false;
    string exa=FilePath::GetFileExa(oldName);
    StringHelper::TrimToLower(exa);
    if(exa=="shp")
        oldName=FilePath::GetFileNameNoExa(oldName);
    else if(exa!="")
        return false;
    dir=FilePath::GetDir(newName);
    if(dir!="") return false;
    exa=FilePath::GetFileExa(newName);
    StringHelper::TrimToLower(exa);
    if(exa=="shp"){
        newName=FilePath::GetFileNameNoExa(newName);
    }
    else if(exa!="")
        return false;
    if(!FilePath::IsFileExisting(pathName+"/"+oldName+".shp")) return false;
    vector<string>stocks={"shp","dbf","shx","sbn","sbx","avl","prj","cpg","xml"};
    for(int k=0;k<stocks.size();k++){
        string oldPath=pathName+"/"+oldName+"."+stocks[k];
        string newPath=pathName+"/"+newName+"."+stocks[k];
        if(FilePath::IsFileExisting(oldPath.c_str())){
            if(0!=::rename(StringHelper::ToLocalString(oldPath).c_str(),StringHelper::ToLocalString(newPath).c_str())) return false;
        }
    }
    return true;
}
//ShapfileWorkspace:

ShapefileWorkspaceFactory::ShapefileWorkspaceFactory(){

}
ShapefileWorkspaceFactory::~ShapefileWorkspaceFactory(){

}

//FeatureWorkspaceFactory:
string ShapefileWorkspaceFactory::GetDescription(){
    return "Shapefile";
}
AutoPtr<WorkspaceFactory>ShapefileWorkspaceFactory::Clone(){
   return new ShapefileWorkspaceFactory();
}
//FileWorkspaceFactory:
vector<string>ShapefileWorkspaceFactory::GetWorkspaceNames(string dir){
    FileDirectory fd(dir);
    return fd.FindDirectories();
}
bool ShapefileWorkspaceFactory::IsWorkspaceDirectory(){
   return true;
}
AutoPtr<FileWorkspace>ShapefileWorkspaceFactory::OpenFromFile(string pathName){
   return new ShapefileWorkspace(pathName);
}

}
