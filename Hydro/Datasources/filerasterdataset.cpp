#include "filerasterdataset.h"
#include "Base/FilePath.h"
#include "Base/StringHelper.h"
#include <fstream>
#include <QString>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "Base/classfactory.h"

namespace SGIS{

REGISTER(FileRasterDataset)


GDALRasterDataset::GDALRasterDataset()
{
    poDataset=nullptr;
}
GDALRasterDataset::~GDALRasterDataset()
{
    Dispose();
}
void GDALRasterDataset::Dispose()
{
    if(poDataset!=nullptr){
       GDALClose(poDataset);
    }
    poDataset=nullptr;
}

FileRasterBand::FileRasterBand(){
    pDataset=nullptr;
    father=nullptr;
    poBand=nullptr;
}
FileRasterBand::~FileRasterBand(){

}
string FileRasterBand::GetDescription(){
   return "栅格波段";
}
AutoPtr<RasterDataset>FileRasterBand::GetRasterDataset(){
   return this->father;
}
float FileRasterBand::GetPixelValue(int x,int y){
    float data=0;
    poBand->RasterIO(GF_Read, x, y, 1, 1,&data,1, 1, GDT_Float32,0, 0 );
    return data;
}

bool FileRasterBand::GetBlockData(int x, int y, int width, int height, int bufferWidth, int bufferHeight, float *data){
    if((poBand==nullptr)||(rasterDesp.cols<=0)||(rasterDesp.rows==0)) return false;
    OGRErr pErr=poBand->RasterIO(GF_Read, x, y, width, height,data,bufferWidth, bufferHeight, GDT_Float32,0, 0 );
    return (pErr==OGRERR_NONE);
}

bool FileRasterBand::SaveBlockData(int x,int y,int width,int height,float*data){
    if((poBand==nullptr)||(rasterDesp.cols<=0)||(rasterDesp.rows==0)) return false;
    if(father->IsReadOnly()) return false;
    bool bSuc=(poBand->RasterIO(GF_Write, x, y, width,height,data,width, height, GDT_Float32,0, 0 )==CE_None);
    ClearStatistics();
    return bSuc;
}

AutoPtr<ColorTable>FileRasterBand::GetColorTable(){
    if(poBand==nullptr) return nullptr;
    GDALColorTable*gct=poBand->GetColorTable();
    if(gct==NULL) return nullptr;
    int Size=gct->GetColorEntryCount();
    AutoPtr<ColorTable>pTable(new ColorTable);
    pTable->SetColors(Size);
    for(long k=0;k<Size;k++)
    {
        GDALColorEntry gce;
        gct->GetColorEntryAsRGB(k,&gce);
        BYTE r,g,b;
        r=gce.c1;
        g=gce.c2;
        b=gce.c3;
        pTable->SetColor(k,r,g,b);
    }
    return pTable;
}
bool FileRasterBand::IsFromSameSource(RasterBand*other){
    AutoPtr<FileRasterBand>po=AutoPtr<RasterBand>(other,true);
    if(po==nullptr) return false;
    if((po->father==nullptr)||(father==nullptr)) return false;
    if(!StringHelper::EqualsNoCase(father->GetPathName(),po->father->GetPathName())) return false;
    return (bandIndex==po->bandIndex);
}
void FileRasterBand::Dispose(){
   pDataset=nullptr;
}

void FileRasterBand::FlushCache(){
   if(poBand==nullptr) return;
   poBand->FlushCache();
}

FileRasterDataset::FileRasterDataset()
{
    pathName="";
    readOnly=true;
    psp=nullptr;
    bandCount=0;
}

FileRasterDataset::~FileRasterDataset()
{

}

string FileRasterDataset::GetDescription(){
   return "栅格数据集";
}
AutoPtr<Workspace>FileRasterDataset::GetWorkspace(){
   return new FileRasterWorkspace(FilePath::GetDir(pathName));
}

string FileRasterDataset::GetName(){
   return FilePath::GetFileName(pathName);
}

bool FileRasterDataset::TemporaryDataset(){
   return false;
}

RasterDesp FileRasterDataset::GetRasterDesp(){
   return rasterDesp;
}

void FileRasterDataset::SetLeftTop(double left,double top){
   rasterDesp.left=left;
   rasterDesp.top=top;
}
void FileRasterDataset::SetCellSize(double xCellSize,double yCellSize){
    rasterDesp.xCellSize=xCellSize;
    rasterDesp.yCellSize=yCellSize;
}
AutoPtr<SpatialReference>FileRasterDataset::GetSpatialReference(){
    if(psp==nullptr) return new SpatialReference();
    return psp->Clone();
}
bool FileRasterDataset::SetSpatialReference(AutoPtr<SpatialReference>psp){
    if(pDataset==nullptr) return false;
    if(psp==nullptr)
        this->psp=nullptr;
    else
        this->psp=psp->Clone();
    string wkt=this->psp->ExportToWkt();
    if(this->IsReadOnly()){
        AutoPtr<FileRasterDataset>of(new FileRasterDataset());
        if(of->OpenFromFile(this->pathName,true)){
            of->pDataset->poDataset->SetProjection(wkt.c_str());
        }
    }
    else{
        pDataset->poDataset->SetProjection(wkt.c_str());
    }
    return true;
}

int FileRasterDataset::GetBandCount(){
    return bandCount;
}

AutoPtr<RasterBand>FileRasterDataset::GetRasterBand(int nIndex){
    AutoPtr<FileRasterBand>pBand(new FileRasterBand);
    pBand->father=new FileRasterDataset();
    pBand->father->pathName=this->pathName;
    pBand->father->readOnly=this->readOnly;
    pBand->father->rasterDesp=this->rasterDesp;
    pBand->father->pDataset=this->pDataset;
    pBand->father->psp=this->psp;
    pBand->father->bandCount=this->bandCount;
    pBand->pDataset=this->pDataset;
    pBand->rasterDesp=rasterDesp;
    pBand->bandIndex=nIndex;
    if(psp!=nullptr) pBand->psp=this->psp->Clone();
    pBand->poBand=pDataset->poDataset->GetRasterBand(nIndex+1);
    pBand->nodata=pBand->poBand->GetNoDataValue();
    GDALDataType dType=pBand->poBand->GetRasterDataType();
    pBand->dataType=RasterDataTypeHelper::GetRasterDataType(dType);
    if(bandCount==1)
        pBand->name=FilePath::GetFileNameNoExa(pathName);
    else
        pBand->name=FilePath::GetFileNameNoExa(pathName)+"_"+to_string(nIndex+1);
    return pBand;
}

vector<string>FileRasterDataset::GetMetaData(string KeyWord){
    if(pDataset==NULL) return {};
    if(pDataset->poDataset==nullptr) return {};
    char ** SUBDATASETS;
    bool IsSub=false;
    if(!KeyWord.empty()) IsSub=true;
    if(KeyWord.empty())
        SUBDATASETS= GDALGetMetadata( (GDALDatasetH)pDataset->poDataset,NULL);
    else
    {
        SUBDATASETS= GDALGetMetadata( (GDALDatasetH)pDataset->poDataset, KeyWord.c_str() );
    }
    if(CSLCount(SUBDATASETS) == 0) return {};
    vector<string>metadatas;
    for(int  i = 0; SUBDATASETS[i] != NULL; i++ )
    {
        string tmpstr=string(SUBDATASETS[i]);
        int nPos=tmpstr.find("=");
        if((IsSub)&&(nPos>0)) tmpstr=tmpstr.substr(nPos+1,tmpstr.length()-nPos-1);
        metadatas.push_back(tmpstr);
    }
    return metadatas;
}

bool FileRasterDataset::IsValid(){
    return (pDataset!=nullptr);
}

void FileRasterDataset::Dispose(){
    if(pDataset!=nullptr) pDataset->Dispose();
    rasterDesp.Reset();
    this->pathName="";
    readOnly=true;
    psp=nullptr;
    bandCount=0;
    pDataset=nullptr;
    psp=nullptr;
}

AutoPtr<RasterDataset>FileRasterDataset::Clone(){
    AutoPtr<FileRasterDataset>newds=new FileRasterDataset();
    newds->pathName=pathName;
    newds->OpenFromFile(pathName,!readOnly);
    return newds;
}
//FileDataset:
bool FileRasterDataset::IsDirectory(){
    if(pathName=="") return false;
    if(FilePath::IsDirExisting(pathName)) return true;
    return false;
}
string FileRasterDataset::GetPathName(){
    return pathName;
}
bool FileRasterDataset::OpenFromFile(string pathName){
    return OpenFromFile(pathName,false);
}

bool FileRasterDataset::OpenFromFile(string pathName,bool Update){
    if(pDataset!=nullptr) pDataset->Dispose();
    rasterDesp.Reset();
    pDataset=nullptr;
    this->pathName="";
    readOnly=true;
    psp=nullptr;
    bandCount=0;
    pDataset=nullptr;
    psp=nullptr;
    this->pathName=pathName;
    string exa=FilePath::GetFileExa(pathName);
    string fileName=FilePath::GetFileNameNoExa(pathName);
    string dir=FilePath::GetDir(pathName);
    string path=pathName;
    if(exa.length()>0)
    {
        StringHelper::MakeUpper(exa);
        if(exa=="HDR")
        {
            path=dir+"/"+fileName;
        }
    }
    path=StringHelper::ToLocalString(path);
    pDataset=new GDALRasterDataset();
    if(!Update)
        pDataset->poDataset=(GDALDataset*)GDALOpen(path.c_str(),GA_ReadOnly);
    else
        pDataset->poDataset=(GDALDataset*)GDALOpen(path.c_str(),GA_Update);
    readOnly=(!Update);
    if(pDataset->poDataset==nullptr)
    {
        pDataset=nullptr;
        return false;
    }
    rasterDesp.rows=pDataset->poDataset->GetRasterYSize();
    rasterDesp.cols=pDataset->poDataset->GetRasterXSize();
    double adfGeoTransform[6];
    pDataset->poDataset->GetGeoTransform( adfGeoTransform );
    if(adfGeoTransform[5]>0)
    {
        //adfGeoTransform[3]=adfGeoTransform[3]+adfGeoTransform[5]*rows;
        adfGeoTransform[5]=-adfGeoTransform[5];
    }
    rasterDesp.left=adfGeoTransform[0];
    rasterDesp.top=adfGeoTransform[3];
    rasterDesp.xCellSize=fabs(adfGeoTransform[1]);
    rasterDesp.yCellSize=fabs(adfGeoTransform[5]);
    bandCount=pDataset->poDataset->GetRasterCount();
    if(psp==nullptr){
        const char*info=pDataset->poDataset->GetProjectionRef();
        psp=new SpatialReference();
        if(info!=nullptr)
        {
            string wkt(info);
            psp->ImportFromWkt(wkt);
        }
    }
    return true;
}

vector<string>FileRasterDataset::GetSubDatasets(){
    if(pDataset==nullptr) return {};
    char ** papszSUBDATASETS = GDALGetMetadata( (GDALDatasetH)pDataset->poDataset, "SUBDATASETS");
    int cout=CSLCount(papszSUBDATASETS);
    vector<string>sds;
    if(cout==0) return sds;
    for(int i=0; papszSUBDATASETS[i] != NULL; i++ ){
        sds.push_back(StringHelper::FromLocalString(papszSUBDATASETS[i]));
    }
    return sds;
}

AutoPtr<RasterDataset>FileRasterDataset::GetSubDataset(string subdataset){
    int nPos=subdataset.find('=');
    if(nPos>0){
        subdataset=subdataset.substr(nPos+1,subdataset.length()-nPos-1);
    }
    AutoPtr<FileRasterDataset>fds(new FileRasterDataset());
    if(fds->OpenFromFile(StringHelper::ToLocalString(subdataset))) return fds;
    return nullptr;
}

bool FileRasterDataset::IsReadOnly(){
    return readOnly;
}


bool FileRasterDataset::Read(BufferReader*br){
    pathName=br->ReadString();
    FilePath::ValidFilePath(pathName,FilePath::GetDir(br->PathName()));
    this->OpenFromFile(pathName);
    return true;
}
void FileRasterDataset::Write(BufferWriter*bw){
    bw->WriteString(pathName);
}

string FileRasterDataset::GetTypeName(){
    return "FileRasterDataset";
}
//JsonWritable:
bool FileRasterDataset::ReadFromJson(JsonReader*jr,JsonNode node){
    JsonNode dirNode=node["pathname"];
    pathName=dirNode.asString();
    FilePath::ValidFilePath(pathName,FilePath::GetDir(jr->PathName()));
    this->OpenFromFile(pathName);
    return true;
}
string FileRasterDataset::ToJson(){
    JsonObjectWriter writer;
    writer.Add<string>("pathname",pathName);
    return writer.GetJsonString();
}


FileRasterWorkspace::FileRasterWorkspace(string pathName){
   this->pathName=pathName;
}
FileRasterWorkspace::~FileRasterWorkspace(){

}
//RasterWorkspace:
string FileRasterWorkspace::GetDescription(){
    return "栅格数据集工作空间";
}
//FileWorkspace
string FileRasterWorkspace::GetPathName(){
    return pathName;
}
bool FileRasterWorkspace::IsDirectory(){
    return true;
}
void FileRasterWorkspace::GetGridDatasetNames(vector<string>&Names){
    string arcPath=this->pathName+"/Info/arc.dir";
    if(!FilePath::IsFileExisting(arcPath)) return;
    std::ifstream ifs;
    string localPath=arcPath;
    ifs.open(StringHelper::ToLocalString(localPath), std::ios::binary | ios::in);
    if (!ifs.is_open()) return;
    ifs.seekg(0, std::ios::end);
    int nFileLen = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    char stru[380];
    int Pos=0;
    char bName[32];
    IndexedArray<string>FullFileNames;
     IndexedArray<string>FileNames;
    while(Pos+380<=nFileLen)
    {
        ifs.read(stru, 380);
        memcpy(bName,stru,32);
        string sName="";
        sName.resize(32);
        memcpy((char*)sName.c_str(),bName,32);
        sName=StringHelper::FromLocalString(sName);
        sName=StringHelper::Trim(sName);
        FullFileNames.Add(sName);
        FileNames.Add(FilePath::GetFileNameNoExa(sName));
        Pos+=380;
    }
    ifs.close();
    LONG Count=FileNames.GetSize();
    string Name;
    LONG nIndex;
    for(int k=0;k<Count;k++)
    {
        Name=FileNames[k];
        string PatName=Name+".pat";
        nIndex=FullFileNames.FindValueIndex(PatName);
        if(nIndex==-1)
        {
            string AatName=Name+".aat";
            nIndex=FullFileNames.FindValueIndex(AatName);
        }
        if(nIndex==-1)
        {
            if(!FilePath::IsDirExisting(this->pathName+"/"+Name)) continue;
            Names.push_back(Name);
        }
    }
}

vector<string>FileRasterWorkspace::GetDirectoryDatasetNames(){
    vector<string>Names;
    GetGridDatasetNames(Names);
    return Names;
}

vector<string>FileRasterWorkspace::GetDatasetNames(){
    vector<string>Names;
    GetGridDatasetNames(Names);
    FileDirectory fd(this->pathName);
    vector<string>results=fd.FindFiles("*.hdr;*.tif;*.tiff;*.img;*.pix;*.bmp;*.jpg;*.gif;*.png");
    for(int k=0;k<results.size();k++)
    {
        Names.push_back(FilePath::GetFileName(results[k]));
    }
    return Names;
}

AutoPtr<FileRasterDataset>FileRasterWorkspace::OpenRasterDataset(string name,bool Update){
    string fullPath=this->pathName+"/"+name;
    bool bDir;
    if(!FilePath::IsFileOrDirExisting(fullPath,bDir)) return nullptr;
    AutoPtr<FileRasterDataset>pDataset(new FileRasterDataset());
    if(!pDataset->OpenFromFile(fullPath,Update)){
        return nullptr;
    }
    return pDataset;
}

AutoPtr<FileRasterDataset>FileRasterWorkspace::CreateRasterDataset(string Name,RasterDesp rasterDesp,LONG BandCount,RasterDataType dataType,RasterCreateFileType fileType,DOUBLE NoData,SpatialReference*psp){
    double xCellSize=fabs(rasterDesp.xCellSize);
    double yCellSize=fabs(rasterDesp.yCellSize);
    if((xCellSize==0)||(yCellSize==0)) return nullptr;
    RasterDataType dt=dataType;
    RasterCreateFileType ft=fileType;
    string sType="";
    string sExa="";
    if(ft==rcftErdasImagine)//Eadas
    {
        sType="HFA";
        sExa="img";
    }
    else if(ft==rcftTiff)//Geotif
    {
        sType="GTiff";
        sExa="tif";
    }
    else if(ft==rcftPCIPix)//PCI
    {
        sType="PCIDSK";
        sExa="pix";
    }
    else if(ft==rcftEnviHdr)//Envi
    {
        sExa="hdr";
        sType="EHdr";
    }
    string sName=Name;
    string uName=FilePath::GetFileNameNoExa(sName);
    GDALDriver *poDriver;
    poDriver = GetGDALDriverManager()->GetDriverByName(sType.c_str());
    if( poDriver == nullptr ) return nullptr;
    GDALDataset *poDstDS;
    char **papszOptions = NULL;
    GDALDataType gType=(GDALDataType)RasterDataTypeHelper::GetGDALDataType(dt);
    string sPathName=this->pathName+"/"+uName+"."+sExa;
    sPathName=StringHelper::ToLocalString(sPathName);
    poDstDS = poDriver->Create(sPathName.c_str(), rasterDesp.cols, rasterDesp.rows, BandCount, gType,papszOptions );
    if(poDstDS==nullptr)
    {
        return nullptr;
    }
    double adfGeoTransform[6];

    adfGeoTransform[0]=rasterDesp.left;
    adfGeoTransform[1]=rasterDesp.xCellSize;
    adfGeoTransform[2]=0;
    adfGeoTransform[3]=rasterDesp.top;
    adfGeoTransform[4]=0;
    adfGeoTransform[5]=-rasterDesp.yCellSize;
    poDstDS->SetGeoTransform( adfGeoTransform );
    if(psp!=nullptr)
    {
        string wkt=psp->ExportToWkt();
        if(wkt!="")
        {
            poDstDS->SetProjection(wkt.c_str());
        }
    }
    for(int k=0;k<BandCount;k++) poDstDS->GetRasterBand(k+1)->SetNoDataValue(NoData);
    GDALClose(poDstDS);
    return this->OpenRasterDataset(Name,true);
}

AutoPtr<FileDataset>FileRasterWorkspace::OpenDataset(string name){
    return OpenRasterDataset(name,false);
}

bool FileRasterWorkspace::CanDeleteDataset(string name){
    string fullPath=this->pathName+"/"+name;
    if(FilePath::IsDirExisting(fullPath)) return false;
    if(!FilePath::IsFileExisting(fullPath)) return false;
    return true;
}

bool FileRasterWorkspace::DeleteDataset(string name){
    string fullPath=this->pathName+"/"+name;
    if(FilePath::IsDirExisting(fullPath)) return false;
    if(!FilePath::IsFileExisting(fullPath)) return false;
    return FilePath::RemoveFile(fullPath);
}

bool FileRasterWorkspace::CanRenameDataset(string name){
    string dir=FilePath::GetDir(name);
    if(dir!="") return false;
    string fullPath=this->pathName+"/"+name;
    if(FilePath::IsDirExisting(fullPath)) return false;
    if(!FilePath::IsFileExisting(fullPath)) return false;
    return true;
}
bool FileRasterWorkspace::RenameDataset(string oldName,string newName){
    string dir=FilePath::GetDir(newName);
    if(dir!="") return false;
    string fullPath=this->pathName+"/"+oldName;
    if(FilePath::IsDirExisting(fullPath)) return false;
    if(!FilePath::IsFileExisting(fullPath)) return false;
    string exa1=FilePath::GetFileExa(oldName);
    string exa2=FilePath::GetFileExa(newName);
    StringHelper::TrimToLower(exa1);
    StringHelper::TrimToLower(exa2);
    if(exa1!=exa2) return false;

    return (0==::rename((StringHelper::ToLocalString(pathName+"/"+oldName)).c_str(),(StringHelper::ToLocalString(pathName+"/"+newName)).c_str()));
}

FileRasterWorkspaceFactory::FileRasterWorkspaceFactory(){

}
FileRasterWorkspaceFactory::~FileRasterWorkspaceFactory(){

}
//RasterWorkspaceFactory:
string FileRasterWorkspaceFactory::GetDescription(){
   return "栅格文件";
}
AutoPtr<WorkspaceFactory>FileRasterWorkspaceFactory::Clone(){
   return new FileRasterWorkspaceFactory();
}
//FileWorkspaceFactory:
vector<string>FileRasterWorkspaceFactory::GetWorkspaceNames(string dir){
    FileDirectory fd(dir);
    return fd.FindDirectories();
}
bool FileRasterWorkspaceFactory::IsWorkspaceDirectory(){
   return true;
}
AutoPtr<FileWorkspace>FileRasterWorkspaceFactory::OpenFromFile(string pathName){
    return new FileRasterWorkspace(pathName);
}



}
