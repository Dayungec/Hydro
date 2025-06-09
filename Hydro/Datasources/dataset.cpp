#include "dataset.h"
#include "Base/stringinterpreter.h"
#include "Base/FilePath.h"
#include "Geometry/geometrytopology.h"
#include "shapfilefeatureclass.h"
#include "featureclasstool.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "filerasterdataset.h"
#include "memrasterdataset.h"

namespace SGIS{

WorkspaceFactory::WorkspaceFactory(){

}
WorkspaceFactory::~WorkspaceFactory(){

}

FileWorkspaceFactory::FileWorkspaceFactory(){

}
FileWorkspaceFactory::~FileWorkspaceFactory(){

}

FeatureWorkspaceFactory::FeatureWorkspaceFactory(){

}
FeatureWorkspaceFactory::~FeatureWorkspaceFactory(){

}

RasterWorkspaceFactory::RasterWorkspaceFactory(){

}
RasterWorkspaceFactory::~RasterWorkspaceFactory(){

}

Workspace::Workspace(){

}
Workspace::~Workspace(){

}

FileWorkspace::FileWorkspace(){

}
FileWorkspace::~FileWorkspace(){

}
vector<string>FileWorkspace::GetDirectoryDatasetNames(){
    return {};
}

FeatureWorkspace::FeatureWorkspace(){

}
FeatureWorkspace::~FeatureWorkspace(){

}

RasterWorkspace::RasterWorkspace(){

}
RasterWorkspace::~RasterWorkspace(){

}

Dataset::Dataset(){

}
Dataset::~Dataset(){

}

FileDataset::FileDataset(){

}
FileDataset::~FileDataset(){

}

FeatureDataset::FeatureDataset(){

}
FeatureDataset::~FeatureDataset(){

}

RasterBand::RasterBand(){
    name="";
    psp=nullptr;
    bandIndex=-1;
    dataType=RasterDataType::rdtUnknown;
    nodata=-32768;
}

RasterBand::~RasterBand(){
    psp=nullptr;
}

string&RasterBand::Name(){
   return name;
}

int RasterBand::GetBandIndex(){
    return bandIndex;
}

RasterDesp RasterBand::GetRasterDesp(){
    return rasterDesp;
}

int RasterBand::GetCols(){
    return rasterDesp.cols;
}
int RasterBand::GetRows(){
    return rasterDesp.rows;
}

AutoPtr<SpatialReference>RasterBand::GetSpatialReference(){
    if(psp==nullptr) return nullptr;
    return psp->Clone();
}

void RasterBand::SetLeftTop(double left,double top){
    rasterDesp.left=left;
    rasterDesp.top=top;
}
void RasterBand::SetCellSize(double xCellSize,double yCellSize){
    rasterDesp.xCellSize=xCellSize;
    rasterDesp.yCellSize=yCellSize;
}
void RasterBand::SetSpatialReference(AutoPtr<SpatialReference>psp){
    if(psp==nullptr)
       this->psp=nullptr;
    else
       this->psp=psp->Clone();
}
double RasterBand::GetNodata(){
    return nodata;
}
void RasterBand::SetNodata(double nodata){
    this->nodata=nodata;
}

RasterDataType RasterBand::GetDataType(){
    return dataType;
}

void RasterBand::ClearStatistics(){
    pStaHist.Clear();
}

bool RasterBand::IsNodata(double value){
    return ((float)nodata==(float)value);
}

StatisHistory&RasterBand::GetStatisHistory(){
    return pStaHist;
}


bool RasterBand::Save(string savingPath){
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
    pDstDriver = (GDALDriver *)GDALGetDriverByName(sType.c_str());
    if (pDstDriver == nullptr) return false;
    RasterDesp desp=this->GetRasterDesp();
    GDALDriver *pMemDriver = nullptr;
    pMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");
    if( pMemDriver == nullptr ) return false;
    AutoPtr<ColorTable>colorTable=GetColorTable();
    bool rgbds=false;
    if((colorTable!=nullptr)&&((sType=="PNG")||(sType=="JPEG"))){
        int crCount=colorTable->GetColors();
        for(int k=0;k<crCount;k++){
            BYTE r,g,b,a;
            colorTable->GetColor(k,r,g,b,a);
            if(a<255) rgbds=true;
        }
    }
    GDALDataType outType=(GDALDataType)RasterDataTypeHelper::GetGDALDataType(GetDataType());
    int outBand=1;
    if(rgbds){
        outBand=4;
        outType=GDALDataType::GDT_Byte;
    }
    GDALDataset * pMemDataSet = pMemDriver->Create("",desp.cols,desp.rows,outBand,outType,nullptr);
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
    if(outBand==1){
        GDALRasterBand *pBand=pMemDataSet->GetRasterBand(1);
        pBand->SetNoDataValue(nodata);
        if(desp.cols*desp.rows<=2000*2000){
            AutoPtr<FixBuffer>fxb(new FixBuffer(desp.cols*desp.rows*sizeof(float)));
            float*fVs=(float*)fxb->GetBuffer();
            this->GetBlockData(0,0,desp.cols,desp.rows,desp.cols,desp.rows,fVs);
            pBand->RasterIO(GF_Write, 0,0, desp.cols,desp.rows,fVs, desp.cols,desp.rows, GDALDataType::GDT_Float32,0, 0 );
        }else{
            AutoPtr<FixBuffer>fxb(new FixBuffer(desp.cols*sizeof(float)));
            float*fVs=(float*)fxb->GetBuffer();
            for(int i=0;i<desp.rows;i++){
                this->GetBlockData(0,i,desp.cols,1,desp.cols,1,fVs);
                pBand->RasterIO(GF_Write, 0,i, desp.cols,1,fVs, desp.cols,1, GDALDataType::GDT_Float32,0, 0 );
            }
        }
        if(colorTable!=nullptr){
            int crCount=colorTable->GetColors();
            GDALColorTable ct;
            GDALColorEntry gce;
            gce.c1=0;gce.c2=0;gce.c3=0;
            GDALColorEntry gce2;
            gce2.c1=255;gce2.c2=255;gce2.c3=255;
            ct.CreateColorRamp(0,&gce,crCount-1,&gce2);
            for(int k=0;k<crCount;k++){
                GDALColorEntry gce;
                BYTE r,g,b,a;
                colorTable->GetColor(k,r,g,b,a);
                gce.c1=r;
                gce.c2=g;
                gce.c3=b;
                gce.c4=a;
                ct.SetColorEntry(k,&gce);
            }
            pBand->SetColorTable(&ct);
        }
    }
    else{
        if(desp.cols*desp.rows<=2000*2000){
            AutoPtr<FixBuffer>fxb(new FixBuffer());
            fxb=new FixBuffer(desp.cols*desp.rows);
            BYTE*nvalues=(BYTE*)fxb->GetBuffer();
            AutoPtr<FixBuffer>bfxb(new FixBuffer(desp.cols*desp.rows*sizeof(float)));
            float*fVs=(float*)bfxb->GetBuffer();
            this->GetBlockData(0,0,desp.cols,desp.rows,desp.cols,desp.rows,fVs);
            for(int b=0;b<4;b++){
                GDALRasterBand *pBand=pMemDataSet->GetRasterBand(b+1);
                float*bValues=fVs;
                int nPos=0;
                int crCount=colorTable->GetColors();
                BYTE r,g,bv,a;
                for(int i=0;i<desp.rows;i++){
                    for(int j=0;j<desp.cols;j++){
                        int colorIndex=bValues[nPos];
                        if(colorIndex>=crCount) {
                            r=g=bv=a=0;
                        }
                        else{
                            colorTable->GetColor(colorIndex,r,g,bv,a);
                        }
                        if(b==0)
                            nvalues[nPos]=r;
                        else if(b==1)
                            nvalues[nPos]=g;
                        else if(b==2)
                            nvalues[nPos]=bv;
                        else
                            nvalues[nPos]=a;
                        nPos++;
                    }
                }
                pBand->RasterIO(GF_Write, 0,0, desp.cols,desp.rows,nvalues,desp.cols,desp.rows,GDALDataType::GDT_Byte ,0, 0 );
            }
        }
        else{
            AutoPtr<FixBuffer>fxb(new FixBuffer());
            fxb=new FixBuffer(desp.cols);
            BYTE*nvalues=(BYTE*)fxb->GetBuffer();
            AutoPtr<FixBuffer>bfxb(new FixBuffer(desp.cols*sizeof(float)));
            float*fVs=(float*)bfxb->GetBuffer();
            for(int i=0;i<desp.rows;i++){
                this->GetBlockData(0,i,desp.cols,1,desp.cols,1,fVs);
                for(int b=0;b<4;b++){
                    GDALRasterBand *pBand=pMemDataSet->GetRasterBand(b+1);
                    float*bValues=fVs;
                    int crCount=colorTable->GetColors();
                    BYTE r,g,bv,a;
                    for(int j=0;j<desp.cols;j++){
                        int colorIndex=bValues[j];
                        if(colorIndex>=crCount) {
                            r=g=bv=a=0;
                        }
                        else{
                            colorTable->GetColor(colorIndex,r,g,bv,a);
                        }
                        if(b==0)
                            nvalues[j]=r;
                        else if(b==1)
                            nvalues[j]=g;
                        else if(b==2)
                            nvalues[j]=bv;
                        else
                            nvalues[j]=a;
                    }
                    pBand->RasterIO(GF_Write, 0,i, desp.cols,1,nvalues,desp.cols,1,GDALDataType::GDT_Byte ,0, 0 );
                }
            }
        }
    }
    GDALDataset * pDataSet = pDstDriver->CreateCopy(savingPath.c_str(),pMemDataSet,FALSE, NULL, NULL, NULL);
    if (pDataSet == nullptr)
    {
        GDALClose(pMemDataSet);
        return false;
    }
    GDALClose(pMemDataSet);
    GDALClose(pDataSet);
    return true;
}


float RasterBand::GetPixelValue(double x,double y,AutoPtr<SpatialReference>psp){
    if(psp!=nullptr){
        AutoPtr<SpatialReference>tpsp=this->GetSpatialReference();
        AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation(psp,tpsp));
        pTrans->BeginTransform();
        pTrans->TransformXY(&x,&y);
    }
    RasterDesp desp=this->GetRasterDesp();
    if((x<desp.left)||(x>desp.left+desp.cols*desp.xCellSize)||(y>desp.top)||(y<desp.top-desp.rows*desp.yCellSize)){
        return this->GetNodata();
    }
    int iX=(x-desp.left)/desp.xCellSize;
    int iY=(desp.top-y)/desp.yCellSize;
    if(iX<0) iX=0;
    if(iX>=desp.cols) iX=desp.cols-1;
    if(iY<0) iY=0;
    if(iY>=desp.rows) iY=desp.rows-1;
    return this->GetPixelValue(iX,iY);
}


bool RasterBand::SaveAsCompressedPng(string pathName){
    string sType="PNG";
    GDALDriver *pDstDriver = (GDALDriver *)GDALGetDriverByName(sType.c_str());
    if (pDstDriver == nullptr) return false;
    RasterDesp desp=this->GetRasterDesp();
    GDALDriver *pMemDriver = nullptr;
    pMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");
    if( pMemDriver == nullptr ) return false;
    GDALDataset * pMemDataSet = pMemDriver->Create("",desp.cols,desp.rows,4,GDALDataType::GDT_Byte,nullptr);
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
    AutoPtr<FixBuffer>fxb(new FixBuffer());
    fxb=new FixBuffer(desp.cols*desp.rows);
    BYTE*nvalues=(BYTE*)fxb->GetBuffer();
    AutoPtr<FixBuffer>bfxb(new FixBuffer(desp.cols*desp.rows*sizeof(float)));
    float*fVs=(float*)bfxb->GetBuffer();
    this->GetBlockData(0,0,desp.cols,desp.rows,desp.cols,desp.rows,fVs);
    for(int b=0;b<4;b++){
        GDALRasterBand *pBand=pMemDataSet->GetRasterBand(b+1);
        int nPos=0;
        BYTE r,g,bv,a;
        for(int i=0;i<desp.rows;i++){
            for(int j=0;j<desp.cols;j++){
                float fv=fVs[nPos];
                int ifv=fv;
                float frac=0;
                BYTE bValue;
                if(fv>=0){
                    for(int l=0;l<=b;l++){
                        frac=fv-ifv;
                        fv=fv/255.0;
                        ifv=fv;
                    }
                    bValue=(BYTE)(frac*255);
                    if(b==3){
                        if(bValue>127) bValue=127;
                    }
                }
                else{
                    fv=-fv;
                    for(int l=0;l<=b;l++){
                        frac=fv-ifv;
                        fv=fv/255.0;
                        ifv=fv;
                    }
                    bValue=(BYTE)(frac*255);
                    if(b==3){
                        if(bValue>127) bValue=127;
                    }
                    bValue+=128;
                }
                nvalues[nPos]=bValue;
                nPos++;
            }
        }
        pBand->RasterIO(GF_Write, 0,0, desp.cols,desp.rows,nvalues,desp.cols,desp.rows,GDALDataType::GDT_Byte ,0, 0 );
    }
    GDALDataset * pDataSet = pDstDriver->CreateCopy(pathName.c_str(),pMemDataSet,FALSE, NULL, NULL, NULL);
    if (pDataSet == nullptr){
        GDALClose(pMemDataSet);
        return false;
    }
    GDALClose(pMemDataSet);
    GDALClose(pDataSet);
    return true;
}

AutoPtr<RasterBand>RasterBand::LoadFromCompressedPng(string pathName){
    AutoPtr<FileRasterDataset>fds(new FileRasterDataset());
    if(!fds->OpenFromFile(pathName)) return nullptr;
    if(fds->GetBandCount()!=4) return nullptr;
    RasterDesp desp=fds->GetRasterDesp();
    AutoPtr<MemRasterWorkspaceFactory>pFac(new MemRasterWorkspaceFactory());
    AutoPtr<MemRasterWorkspace>pWork=pFac->CreateMemRasterWorkspace();
    AutoPtr<SpatialReference>psp=fds->GetSpatialReference();
    AutoPtr<MemRasterDataset>mds=pWork->CreateRasterDataset("Band",desp,1,RasterDataType::rdtFloat32,-32768,psp.get());
    if(mds==nullptr) return nullptr;
    AutoPtr<MemRasterBand>mBand=mds->GetRasterBand(0);
    AutoPtr<FixBuffer>fxb(new FixBuffer(desp.cols*desp.rows*sizeof(float)));
    float*fVs=(float*)fxb->GetBuffer();
    float*oVs=(float*)mBand->GetRawData()->GetValues();
    for(int b=0;b<4;b++){
        fds->GetRasterBand(b)->GetBlockData(0,0,desp.cols,desp.rows,desp.cols,desp.rows,fVs);
        for(int k=desp.cols*desp.rows-1;k>=0;k--){
            int value=fVs[k];
            float fac=1;
            if(b==3){
                if(value>=128){
                    value-=128;
                    fac=-1;
                }
            }
            float fValue;
            if(b==0)
               fValue=value/255.0;
            else if(b==1)
               fValue=value;
            else if(b==2)
               fValue=value*255;
            else if(b==3)
               fValue=value*65025;
            if(b==0)
                oVs[k]=fValue;
            else
                oVs[k]+=fValue;
            if((b==3)&&(fac==-1)) oVs[k]=-oVs[k];
        }
    }
    return mBand;
}

void RasterBand::FlushCache(){

}

RasterDataset::RasterDataset(){

}
RasterDataset::~RasterDataset(){

}
int RasterDataset::GetCols(){
    return GetRasterDesp().cols;
}
int RasterDataset::GetRows(){
    return GetRasterDesp().rows;
}

FeatureClassEdit::FeatureClassEdit(){

}
FeatureClassEdit::~FeatureClassEdit(){

}

FeatureClass::FeatureClass(){
    name="";
    pDesc=nullptr;
    psp=nullptr;
    pSelectionSet=new SelectionSet();
    shapeType=vstUnknown;
    is3d=false;
}

FeatureClass::~FeatureClass(){

}

string&FeatureClass::Name(){
    return name;
}

AutoPtr<TableDesc>FeatureClass::GetTableDesc(){
    return pDesc;
}

VectorShapeType FeatureClass::GetShapeType(){
    return shapeType;
}

AutoPtr<SelectionSet>FeatureClass::GetSelectionSet(){
    return pSelectionSet;
}

AutoPtr<SpatialReference>FeatureClass::GetSpatialReference(){
    if(psp==nullptr) return nullptr;
    return psp->Clone();
}

bool FeatureClass::Is3DFeatureClass(){
    return is3d;
}

bool FeatureClass::SetSpatialReference(AutoPtr<SpatialReference>psp){
    if(this->psp==nullptr) this->psp=new SpatialReference();
    this->psp->CopyFromOther(psp.get());
    return true;
}

StatisHistory&FeatureClass::GetStatisHistory(){
    return pStaHist;
}

void FeatureClass::ClearStatistics(){
    pStaHist.Clear();
}

void FeatureClass::FlushCache(){

}

bool FeatureClass::Output(string lyPath,SpatialReference*psp,bool onlySel){
    AutoPtr<ShapefileWorkspaceFactory>pFac(new ShapefileWorkspaceFactory());
    AutoPtr<ShapefileWorkspace>pWork=pFac->OpenFromFile(FilePath::GetDir(lyPath));
    AutoPtr<ShapefileFeatureClass>pfc=pWork->CreateFeatureClass(FilePath::GetFileName(lyPath),this->GetShapeType(),pDesc.get(),psp);
    if(pfc==nullptr) return false;
    int featureCount=GetFeatureCount();
    AutoPtr<SelectionSet>pSelSet=GetSelectionSet();
    LONG fieldCount=pDesc->GetFieldCount();
    AutoPtr<FeatureClassEdit>pEdit(pfc);
    vector<Variant>values;
    values.resize(fieldCount);
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation(this->GetSpatialReference(),AutoPtr<SpatialReference>(psp,true)));
    pTrans->BeginTransform();
    if(onlySel){
        LONG selCount=pSelSet->GetSize();
        for(int k=0;k<selCount;k++){
           int fid=pSelSet->GetFeatureID(k);
           int recordIndex=FeatureClassTool::FindFeatureID(this,fid);
           if(recordIndex==-1) continue;
           for(int j=0;j<fieldCount;j++){
               values[j]=this->GetFieldValue(recordIndex,j);
           }
           AutoPtr<Geometry>geo=GetFeature(recordIndex);
           pTrans->TransformGeometry(geo.get());
           pEdit->AddFeatureEx(geo,values);
        }
    }
    else{
        for(int k=0;k<featureCount;k++){
           for(int j=0;j<fieldCount;j++){
               values[j]=this->GetFieldValue(k,j);
           }
           AutoPtr<Geometry>geo=GetFeature(k);
           pTrans->TransformGeometry(geo.get());
           pEdit->AddFeatureEx(geo,values);
        }
    }
    pfc->Dispose();
    return true;
}

}
