#include "streamrasterdataset.h"
#include <QString>
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "Base/FilePath.h"
#include "Base/StringHelper.h"

namespace SGIS{

InnerStreamRasterDataset::InnerStreamRasterDataset()
{
    buffer=nullptr;
    dataLen=0;
    poDataset=nullptr;
    destroyData=false;
    szGUID=Guid::CreateGuid();
}
InnerStreamRasterDataset::~InnerStreamRasterDataset()
{
    if((buffer!=nullptr)&&(destroyData))
    {
        delete []buffer;
        buffer=nullptr;
        dataLen=0;
        destroyData=false;
    }
    if(poDataset!=nullptr)
    {
        delete poDataset;
        poDataset=nullptr;
        //VSIUnlink("/vsimem/"+name+szGUID);
        poDataset=nullptr;
    }
}

bool InnerStreamRasterDataset::AttachMemData(string name,BYTE*bts,int len,bool update){
    Dispose();
    string uniqueName="/vsimem/"+name+szGUID;
    VSIFCloseL( VSIFileFromMemBuffer(uniqueName.c_str(), bts, len, FALSE ));
    GDALDataset  *pDataset =(GDALDataset  *) GDALOpen( uniqueName.c_str(), (update?GA_Update:GA_ReadOnly));
    if(pDataset==NULL)
    {
        VSIUnlink(uniqueName.c_str());
        return false;
    }
    this->name=name;
    buffer=bts;
    dataLen=len;
    destroyData=false;
    poDataset=pDataset;
    VSIUnlink(uniqueName.c_str());
    return true;
}

bool InnerStreamRasterDataset::CopyMemData(string name,BYTE*bts,int len,bool update)
{
    Dispose();
    buffer=new BYTE[len];
    memcpy(buffer,bts,len);
    string uniqueName="/vsimem/"+name+szGUID;
    VSIFCloseL( VSIFileFromMemBuffer(uniqueName.c_str(), buffer, len, FALSE ));
    GDALDataset  *pDataset =(GDALDataset  *) GDALOpen(uniqueName.c_str(), (update?GA_Update:GA_ReadOnly));
    if(pDataset==NULL)
    {
       VSIUnlink(uniqueName.c_str());
       delete []buffer;
       buffer=NULL;
       return false;
    }
    this->name=name;
    dataLen=len;
    destroyData=true;
    poDataset=pDataset;
    VSIUnlink(uniqueName.c_str());
    return true;
}

void InnerStreamRasterDataset::Dispose()
{
    if((buffer!=nullptr)&&(destroyData))
    {
        delete []buffer;
        buffer=nullptr;
        dataLen=0;
        destroyData=false;
    }
    if(poDataset!=nullptr)
    {
        delete poDataset;
        poDataset=NULL;
        //VSIUnlink("/vsimem/"+name+szGUID);
        poDataset=NULL;
    }
}

StreamRasterBand::StreamRasterBand(){
    pDataset=nullptr;
    poBand=nullptr;
    father=nullptr;
}
StreamRasterBand::~StreamRasterBand(){

}
string StreamRasterBand::GetDescription(){
   return "数据流栅格波段";
}
AutoPtr<RasterDataset>StreamRasterBand::GetRasterDataset(){
   return this->father;
}

float StreamRasterBand::GetPixelValue(int x,int y){
    float data=0;
    poBand->RasterIO(GF_Read, x, y, 1, 1,&data,1, 1, GDT_Float32,0, 0 );
    return data;
}

bool StreamRasterBand::GetBlockData(int x,int y,int width,int height,int bufferWidth,int bufferHeight,float*data){
    if((poBand==nullptr)||(rasterDesp.cols<=0)||(rasterDesp.rows==0)) return false;
    OGRErr pErr=poBand->RasterIO(GF_Read, x, y, width, height,data,bufferWidth, bufferHeight, GDT_Float32,0, 0 );
    return (pErr==OGRERR_NONE);
}

bool StreamRasterBand::SaveBlockData(int x,int y,int width,int height,float*data){
    if(ReadOnly) return false;
    if((poBand==nullptr)||(rasterDesp.cols<=0)||(rasterDesp.rows==0)) return false;
    bool bSuc=(poBand->RasterIO(GF_Write, x, y, width,height,data,width, height, GDT_Float32,0, 0 )==CE_None);
    ClearStatistics();
    return bSuc;
}

AutoPtr<ColorTable>StreamRasterBand::GetColorTable(){
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
bool StreamRasterBand::IsFromSameSource(RasterBand*other){
    AutoPtr<StreamRasterBand>po=AutoPtr<RasterBand>(other,true);
    if(po==nullptr) return false;
    if(po->father!=father) return false;
    return (bandIndex==po->bandIndex);
}


bool StreamRasterBand::Save(string savingPath){
    string exa=FilePath::GetFileExa(savingPath);
    StringHelper::MakeUpper(exa);
    GDALDriver *pDstDriver = nullptr;
    string sType=exa;
    if(exa=="IMG")//Eadas
        sType="HFA";
    else if(exa=="TIF")//Geotif
        sType="GTiff";
    else if(exa=="PIX")//PCI
        sType="PCIDSK";
    else if(exa=="HDR")//Envi
       sType="EHdr";
    pDstDriver = (GDALDriver *)GDALGetDriverByName(sType.c_str());
    if (pDstDriver == nullptr) return false;
    RasterDesp desp=this->GetRasterDesp();
    GDALDriver *pMemDriver = nullptr;
    pMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");
    if( pMemDriver == nullptr ) return false;
    GDALDataset * pMemDataSet = pMemDriver->Create("",desp.cols,desp.rows,1,(GDALDataType)RasterDataTypeHelper::GetGDALDataType(GetDataType()),nullptr);
    if(pMemDataSet==nullptr) return false;
    double adfGeoTransform[6];
    adfGeoTransform[0]=desp.left;
    adfGeoTransform[1]=desp.xCellSize;
    adfGeoTransform[2]=0;
    adfGeoTransform[3]=desp.top;
    adfGeoTransform[4]=0;
    adfGeoTransform[5]=-desp.yCellSize;
    pMemDataSet->SetGeoTransform( adfGeoTransform );
    AutoPtr<SpatialReference>psp=GetSpatialReference();
    if(psp!=nullptr)
    {
        string wkt=psp->ExportToWkt();
        if(wkt!="") pMemDataSet->SetProjection(wkt.c_str());
    }
    GDALRasterBand *pBand=pMemDataSet->GetRasterBand(1);
    FLOAT*fVs=new FLOAT[desp.cols*desp.rows];
    this->GetBlockData(0,0,desp.cols,desp.rows,desp.cols,desp.rows,fVs);
    pBand->RasterIO(GF_Write, 0,0, desp.cols,desp.rows,fVs, desp.cols,desp.rows, GDALDataType::GDT_Float32,0, 0 );
    delete []fVs;
    GDALDataset * pDataSet = pDstDriver->CreateCopy(savingPath.c_str(),pMemDataSet,FALSE, NULL, NULL, NULL);
    if (pDataSet == nullptr)
    {
        delete pMemDataSet;
        return false;
    }
    delete pMemDataSet;
    delete pDataSet;
    return true;
}


void StreamRasterBand::Dispose(){
   pDataset=nullptr;
   bandIndex=-1;
   psp=nullptr;
}

StreamRasterDataset::StreamRasterDataset()
{
    name="";
    psp=nullptr;
    ReadOnly=true;
    pDataset=nullptr;
    bandCount=0;
}

StreamRasterDataset::~StreamRasterDataset()
{

}

string StreamRasterDataset::GetDescription(){
   return "数据流栅格数据集";
}
AutoPtr<Workspace>StreamRasterDataset::GetWorkspace(){
   return new StreamRasterWorkspace();
}

string StreamRasterDataset::GetName(){
   return name;
}

string StreamRasterDataset::GetTypeName(){
    return "StreamRasterDataset";
}

bool StreamRasterDataset::TemporaryDataset(){
   return true;
}

RasterDesp StreamRasterDataset::GetRasterDesp(){
   return rasterDesp;
}

void StreamRasterDataset::SetLeftTop(double left,double top){
   rasterDesp.left=left;
   rasterDesp.top=top;
}
void StreamRasterDataset::SetCellSize(double xCellSize,double yCellSize){
    rasterDesp.xCellSize=xCellSize;
    rasterDesp.yCellSize=yCellSize;
}
AutoPtr<SpatialReference>StreamRasterDataset::GetSpatialReference(){
    if(psp==nullptr) return new SpatialReference();
    return psp->Clone();
}
bool StreamRasterDataset::SetSpatialReference(AutoPtr<SpatialReference>psp){
    if(psp==nullptr)
        this->psp=nullptr;
    else
        this->psp=psp->Clone();
    return true;
}
int StreamRasterDataset::GetBandCount(){
    return bandCount;
}
AutoPtr<RasterBand>StreamRasterDataset::GetRasterBand(int nIndex){
    if(pDataset==nullptr) return nullptr;
    AutoPtr<StreamRasterBand>pBand(new StreamRasterBand);
    pBand->pDataset=this->pDataset;
    pBand->poBand=pDataset->poDataset->GetRasterBand(nIndex+1);
    pBand->father=AutoPtr<StreamRasterDataset>(this,true);
    pBand->rasterDesp=rasterDesp;
    pBand->bandIndex=nIndex;
    pBand->psp=((this->psp==nullptr)?nullptr:this->psp->Clone());
    pBand->poBand=pDataset->poDataset->GetRasterBand(nIndex+1);
    pBand->nodata=pBand->poBand->GetNoDataValue();
    GDALDataType dType=pBand->poBand->GetRasterDataType();
    pBand->dataType=RasterDataTypeHelper::GetRasterDataType(dType);
    if(bandCount==1)
        pBand->name=name;
    else
        pBand->name=name+"_"+to_string(nIndex+1);
    return pBand;
}

bool StreamRasterDataset::Save(string savingPath){
    int bandCount=this->GetBandCount();
    if(bandCount==0) return false;
    string exa=FilePath::GetFileExa(savingPath);
    StringHelper::MakeUpper(exa);
    GDALDriver *pDstDriver = nullptr;
    string sType=exa;
    if(exa=="IMG")//Eadas
        sType="HFA";
    else if(exa=="TIF")//Geotif
        sType="GTiff";
    else if(exa=="PIX")//PCI
        sType="PCIDSK";
    else if(exa=="HDR")//Envi
       sType="EHdr";
    else if(exa=="PNG")
       sType="PNG";
    else if(exa=="JPG")
       sType="JPEG";
    int createBand=bandCount;
    if(bandCount>4){
        if((sType=="PNG")||(sType=="JPEG")){
            bandCount=4;
            createBand=4;
        }
    }
    else if(bandCount==2){
        if((sType=="PNG")||(sType=="JPEG")) createBand=3;
    }
    pDstDriver = (GDALDriver *)GDALGetDriverByName(sType.c_str());
    if (pDstDriver == nullptr) return false;
    RasterDesp desp=this->GetRasterDesp();
    GDALDriver *pMemDriver = nullptr;
    pMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");
    if( pMemDriver == nullptr ) return false;
    AutoPtr<RasterBand>firstBand=this->GetRasterBand(0);
    GDALDataset * pMemDataSet = pMemDriver->Create("",desp.cols,desp.rows,createBand,(GDALDataType)RasterDataTypeHelper::GetGDALDataType(firstBand->GetDataType()),nullptr);
    if(pMemDataSet==nullptr) return false;
    double adfGeoTransform[6];
    adfGeoTransform[0]=desp.left;
    adfGeoTransform[1]=desp.xCellSize;
    adfGeoTransform[2]=0;
    adfGeoTransform[3]=desp.top;
    adfGeoTransform[4]=0;
    adfGeoTransform[5]=-desp.yCellSize;
    pMemDataSet->SetGeoTransform( adfGeoTransform );
    AutoPtr<SpatialReference>psp=GetSpatialReference();
    if(psp!=nullptr)
    {
        string wkt=psp->ExportToWkt();
        if(wkt!="") pMemDataSet->SetProjection(wkt.c_str());
    }
    FLOAT*fVs=new FLOAT[desp.cols*desp.rows];
    for(int k=0;k<bandCount;k++){
       GDALRasterBand *pBand=pMemDataSet->GetRasterBand(k+1);
       AutoPtr<RasterBand>band=this->GetRasterBand(k);
       band->GetBlockData(0,0,desp.cols,desp.rows,desp.cols,desp.rows,fVs);
       pBand->RasterIO(GF_Write, 0,0, desp.cols,desp.rows,fVs, desp.cols,desp.rows, GDALDataType::GDT_Float32,0, 0 );
    }
    delete []fVs;
    if(bandCount==1){
        AutoPtr<RasterBand>band=this->GetRasterBand(0);
        AutoPtr<ColorTable>table=band->GetColorTable();
        if(table!=nullptr){
            GDALRasterBand *pBand=pMemDataSet->GetRasterBand(1);
            int crCount=table->GetColors();
            GDALColorTable ct;
            GDALColorEntry gce;
            gce.c1=0;gce.c2=0;gce.c3=0;
            GDALColorEntry gce2;
            gce2.c1=255;gce2.c2=255;gce2.c3=255;
            ct.CreateColorRamp(0,&gce,crCount-1,&gce2);
            for(int k=0;k<crCount;k++){
                GDALColorEntry gce;
                BYTE r,g,b;
                table->GetColor(k,r,g,b);
                gce.c1=r;
                gce.c2=g;
                gce.c3=b;
                gce.c4=255;
                ct.SetColorEntry(k,&gce);
            }
            pBand->SetColorTable(&ct);
        }
    }
    GDALDataset * pDataSet = pDstDriver->CreateCopy(savingPath.c_str(),pMemDataSet,FALSE, NULL, NULL, NULL);
    if (pDataSet == nullptr)
    {
        delete pMemDataSet;
        return false;
    }
    delete pMemDataSet;
    delete pDataSet;
    return true;
}

bool StreamRasterDataset::IsValid(){
    return (pDataset!=nullptr);
}
void StreamRasterDataset::Dispose(){
    pDataset->Dispose();
    pDataset=nullptr;
    psp=nullptr;
    name="";
}

AutoPtr<RasterDataset>StreamRasterDataset::Clone(){
    AutoPtr<StreamRasterDataset>newds=new StreamRasterDataset();
    newds->name=name;
    newds->ReadOnly=ReadOnly;
    newds->rasterDesp=rasterDesp;
    newds->psp=(psp==nullptr?nullptr:psp->Clone());
    newds->pDataset=pDataset;
    newds->bandCount=bandCount;
    return newds;
}

bool StreamRasterDataset::AttachImageData(string Name,BYTE* memImage,LONG datalen,bool Update){
    rasterDesp.Reset();
    if(pDataset!=nullptr) pDataset=nullptr;
    psp=nullptr;
    name="";
    psp=nullptr;
    ReadOnly=true;
    bandCount=0;
    pDataset=new InnerStreamRasterDataset();
    if(!pDataset->AttachMemData(Name,memImage,datalen,Update))
    {
        pDataset=nullptr;
        return false;
    }
    ReadOnly=(!Update);
    rasterDesp.rows=pDataset->poDataset->GetRasterYSize();
    rasterDesp.cols=pDataset->poDataset->GetRasterXSize();
    double adfGeoTransform[6];
    pDataset->poDataset->GetGeoTransform( adfGeoTransform );
    if(adfGeoTransform[5]>0)
    {
        //adfGeoTransform[3]=adfGeoTransform[3]+adfGeoTransform[5]*rasterDesp.rows;
        adfGeoTransform[5]=-adfGeoTransform[5];
    }
    rasterDesp.left=adfGeoTransform[0];
    rasterDesp.top=adfGeoTransform[3];
    rasterDesp.xCellSize=fabs(adfGeoTransform[1]);
    rasterDesp.yCellSize=fabs(adfGeoTransform[5]);
    bandCount=pDataset->poDataset->GetRasterCount();
    return true;
}
bool StreamRasterDataset::CopyImageData(string Name,BYTE* memImage,LONG datalen,bool Update){
    rasterDesp.Reset();
    if(pDataset!=nullptr) pDataset=nullptr;
    psp=nullptr;
    name="";
    psp=nullptr;
    ReadOnly=true;
    bandCount=0;
    pDataset=new InnerStreamRasterDataset();
    if(!pDataset->CopyMemData(Name,memImage,datalen,Update))
    {
        pDataset=nullptr;
        return false;
    }
    ReadOnly=(!Update);
    rasterDesp.rows=pDataset->poDataset->GetRasterYSize();
    rasterDesp.cols=pDataset->poDataset->GetRasterXSize();
    double adfGeoTransform[6];
    pDataset->poDataset->GetGeoTransform( adfGeoTransform );
    if(adfGeoTransform[5]>0)
    {
        //adfGeoTransform[3]=adfGeoTransform[3]+adfGeoTransform[5]*rasterDesp.rows;
        adfGeoTransform[5]=-adfGeoTransform[5];
    }
    rasterDesp.left=adfGeoTransform[0];
    rasterDesp.top=adfGeoTransform[3];
    rasterDesp.xCellSize=fabs(adfGeoTransform[1]);
    rasterDesp.yCellSize=fabs(adfGeoTransform[5]);
    bandCount=pDataset->poDataset->GetRasterCount();
    return true;
}

StreamRasterWorkspace::StreamRasterWorkspace(){

}
StreamRasterWorkspace::~StreamRasterWorkspace(){

}
//RasterWorkspace:
string StreamRasterWorkspace::GetDescription(){
    return "数据流栅格工作空间";
}

AutoPtr<StreamRasterDataset>StreamRasterWorkspace::CreateRasterDataset(string Name,BYTE*bits,LONG len,bool Update){
    AutoPtr<StreamRasterDataset>pDataset(new StreamRasterDataset);
    if(!pDataset->CopyImageData(Name,bits,len,Update))
    {
       return nullptr;
    }
    if(pDataset->GetBandCount()==0){
        return nullptr;
    }
    return pDataset;
}

StreamRasterWorkspaceFactory::StreamRasterWorkspaceFactory(){

}
StreamRasterWorkspaceFactory::~StreamRasterWorkspaceFactory(){

}
//RasterWorkspaceFactory:
string StreamRasterWorkspaceFactory::GetDescription(){
   return "数据流栅格";
}
AutoPtr<WorkspaceFactory>StreamRasterWorkspaceFactory::Clone(){
   return new StreamRasterWorkspaceFactory();
}

AutoPtr<StreamRasterWorkspace>StreamRasterWorkspaceFactory::CreateStreamRasterWorkspace(){
    return new StreamRasterWorkspace();
}

}
