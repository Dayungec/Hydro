#include "memrasterdataset.h"
#include <QString>
#include "Base/FilePath.h"
#include "Base/StringHelper.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "Base/guid.h"
namespace SGIS{

MemRasterBand::MemRasterBand(){
    bandData=nullptr;
    father=nullptr;
}
MemRasterBand::~MemRasterBand(){

}
string MemRasterBand::GetDescription(){
   return "内存栅格波段";
}
AutoPtr<RasterDataset>MemRasterBand::GetRasterDataset(){
   return this->father;
}
float MemRasterBand::GetPixelValue(int x,int y){
    return bandData->GetValue<float>(x+y*rasterDesp.cols);
}

bool MemRasterBand::innerGetBlockData(LONG x1,LONG y1,LONG Width,LONG Height,LONG buffx,LONG buffy,float*data){
    LONG buffer=buffx*buffy;
    if(buffer<=0) return false;
    int cols=rasterDesp.cols;
    int rows=rasterDesp.rows;
    RasterDataType dType=bandData->GetDataType();
    if((Width==buffx)&&(Height==buffy))
    {
        int y2=y1+buffy-1;
        if(dType==rdtFloat32){
            BYTE*bData=bandData->GetValues();
            for(int i=y1;i<=y2;i++)
            {
                if(i>=rows) break;
                if(buffx+x1>=cols) buffx=cols-x1;
                memcpy(((BYTE*)data)+(i-y1)*buffx*sizeof(FLOAT),bData+(i*cols+x1)*sizeof(FLOAT),buffx*sizeof(FLOAT));
            }
        }
        else{
            float*fVs=new float[Width];
            for(int i=y1;i<=y2;i++)
            {
                if(i>=rows) break;
                if(buffx+x1>=cols) buffx=cols-x1;
                bandData->GetValues<float>(x1+i*cols,Width,fVs);
                memcpy(((BYTE*)data)+(i-y1)*buffx*sizeof(FLOAT),fVs,buffx*sizeof(FLOAT));
            }
            delete []fVs;
        }
    }
    else
    {
        float difx=(float)Width/buffx;
        float dify=(float)Height/buffy;
        float*fVs=new float[Width];
        int CurrentRow=y1-1;
        int x2=x1+Width-1;
        int y2=y1+Height-1;
        long pos=0;
        int iCol,iRow;
        int nCol=0,nRow=0;
        for(float row=y1+dify/2;row<y2+1;row+=dify)
        {
            iRow=(int)row;
            if((iRow>y2)||(nRow>=buffy)) break;
            if(CurrentRow!=iRow)
            {
                bandData->GetValues<float>(x1+iRow*cols,Width,fVs);
            }
            pos=buffx*nRow;
            nCol=0;
            for(float col=x1+difx/2;col<x2+1;col+=difx)
            {
                iCol=(int)col;
                if((iCol>x2)||(nCol>=buffx)) break;
                data[pos]=fVs[iCol-x1];
                pos++;
                nCol++;
            }
            CurrentRow=row;
            nRow++;
        }
        delete []fVs;
    }
    return true;
}

bool MemRasterBand::GetBlockData(int x,int y,int width,int height,int bufferWidth,int bufferHeight,float*data){
    if((bandData==nullptr)||(width<=0)||(height==0)) return false;
    return innerGetBlockData(x, y, width, height,bufferWidth, bufferHeight,data);
}

bool MemRasterBand::SaveBlockData(int x,int y,int width,int height,float*data){
    if((bandData==nullptr)||(width<=0)||(height==0)) return false;
    if((x==0)&&(y==0)&&(width==this->rasterDesp.cols)&&(height==this->rasterDesp.rows)){
        if(bandData->GetDataType()==rdtFloat32)
            memcpy(bandData->GetValues(),data,width*height*sizeof(FLOAT));
        else
            bandData->SetValues<float>(0,width*height,data);
        return true;
    }
    else if(height==1){
        bandData->SetValues<float>(y*rasterDesp.cols+x,width,data);
        return true;
    }
    else{
        int y2=y+height-1;
        float*pData=new float[width];
        for(int i=y;i<=y2;i++)
        {
            memcpy(pData,((BYTE*)data)+(i-y)*width*sizeof(float),width*sizeof(float));
            bandData->SetValues<float>(i*rasterDesp.cols+x,width,pData);
        }
        delete []pData;
        ClearStatistics();
        return true;
    }
}

void MemRasterBand::SetColorTable(AutoPtr<ColorTable>ct){
    if(ct==nullptr)
        this->colorTable=nullptr;
    else
        this->colorTable=ct->Clone();
    father->colorTables[bandIndex]=this->colorTable;
}
AutoPtr<MemDataArray>MemRasterBand::GetRawData(){
    return bandData;
}
AutoPtr<ColorTable>MemRasterBand::GetColorTable(){
    return this->colorTable;
}
bool MemRasterBand::IsFromSameSource(RasterBand*other){
    AutoPtr<MemRasterBand>po=AutoPtr<RasterBand>(other,true);
    if(po==nullptr) return false;
    if(po->bandData!=bandData) return false;
    return true;
}
void MemRasterBand::Dispose(){
   bandData=nullptr;
   bandIndex=-1;
   psp=nullptr;
}

bool MemRasterBand::Save(string savingPath){
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
    AutoPtr<ColorTable>table=GetColorTable();
    bool rgbds=false;
    if((table!=nullptr)&&((sType=="PNG")||(sType=="JPEG"))){
        int crCount=table->GetColors();
        for(int k=0;k<crCount;k++){
            BYTE r,g,b,a;
            table->GetColor(k,r,g,b,a);
            if(a<255) rgbds=true;
        }
    }
    GDALDataType outType=(GDALDataType)RasterDataTypeHelper::GetGDALDataType(GetDataType());
    if((sType=="PNG")||(sType=="JPEG")){
        outType=GDALDataType::GDT_Byte;
    }
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
        pBand->RasterIO(GF_Write, 0,0, desp.cols,desp.rows,bandData->GetValues(), desp.cols,desp.rows, (GDALDataType)RasterDataTypeHelper::GetGDALDataType(GetDataType()),0, 0 );
        if(table!=nullptr){
            int crCount=table->GetColors();
            GDALColorTable ct;
            GDALColorEntry gce;
            gce.c1=0;gce.c2=0;gce.c3=0;
            GDALColorEntry gce2;
            gce2.c1=255;gce2.c2=255;gce2.c3=255;
            ct.CreateColorRamp(0,&gce,crCount-1,&gce2);
            for(int k=0;k<crCount;k++){
                GDALColorEntry gce;
                BYTE r,g,b,a;
                table->GetColor(k,r,g,b,a);
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
        AutoPtr<FixBuffer>fxb(new FixBuffer());
        fxb=new FixBuffer(desp.cols*desp.rows);
        BYTE*nvalues=(BYTE*)fxb->GetBuffer();
        for(int b=0;b<4;b++){
            GDALRasterBand *pBand=pMemDataSet->GetRasterBand(b+1);
            //pBand->SetNoDataValue(nodata);
            BYTE*bValues=(BYTE*)bandData->GetValues();
            int nPos=0;
            int crCount=table->GetColors();
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
    GDALDataset * pDataSet = pDstDriver->CreateCopy(StringHelper::ToLocalString(savingPath).c_str(),pMemDataSet,FALSE, NULL, NULL, NULL);
    if (pDataSet == nullptr)
    {
        GDALClose(pMemDataSet);
        return false;
    }
    GDALClose(pMemDataSet);
    GDALClose(pDataSet);
    return true;
}

AutoPtr<FixBuffer>MemRasterBand::SaveAsBuffer(string exa){
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

    if (pDstDriver == nullptr) return nullptr;
    RasterDesp desp=GetRasterDesp();
    GDALDriver *pMemDriver = nullptr;

    pMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");
    if( pMemDriver == nullptr ) return nullptr;
    AutoPtr<ColorTable>table=GetColorTable();
    bool rgbds=false;
    if((table!=nullptr)&&((sType=="PNG")||(sType=="JPEG"))){
        int crCount=table->GetColors();
        for(int k=0;k<crCount;k++){
            BYTE r,g,b,a;
            table->GetColor(k,r,g,b,a);
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
    if(pMemDataSet==nullptr) return nullptr;
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
        pBand->RasterIO(GF_Write, 0,0, desp.cols,desp.rows,bandData->GetValues(), desp.cols,desp.rows, (GDALDataType)RasterDataTypeHelper::GetGDALDataType(GetDataType()),0, 0 );
        if(table!=nullptr){
            int crCount=table->GetColors();
            GDALColorTable ct;
            GDALColorEntry gce;
            gce.c1=0;gce.c2=0;gce.c3=0;
            GDALColorEntry gce2;
            gce2.c1=255;gce2.c2=255;gce2.c3=255;
            ct.CreateColorRamp(0,&gce,crCount-1,&gce2);
            for(int k=0;k<crCount;k++){
                GDALColorEntry gce;
                BYTE r,g,b,a;
                table->GetColor(k,r,g,b,a);
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
        AutoPtr<FixBuffer>fxb(new FixBuffer());
        fxb=new FixBuffer(desp.cols*desp.rows);
        BYTE*nvalues=(BYTE*)fxb->GetBuffer();
        for(int b=0;b<4;b++){
            GDALRasterBand *pBand=pMemDataSet->GetRasterBand(b+1);
            //pBand->SetNoDataValue(nodata);
            BYTE*bValues=(BYTE*)bandData->GetValues();
            int nPos=0;
            int crCount=table->GetColors();
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
    string uuid=Guid::CreateGuid();
    string savingPath= "/vsimem/mem_"+uuid+"."+exa;
    GDALDataset * pDataSet = pDstDriver->CreateCopy(savingPath.c_str(),pMemDataSet,FALSE, NULL, NULL, NULL);
    if (pDataSet == nullptr)
    {
        GDALClose(pMemDataSet);
        return nullptr;
    }
    vsi_l_offset outDataLength;
    int bUnlinkAndSeize = FALSE;
    GByte * binData = VSIGetMemFileBuffer(savingPath.c_str(), &outDataLength, bUnlinkAndSeize);
    char*newdata=new char[outDataLength];
    memcpy(newdata,(char*)binData,outDataLength);
    FixBuffer*buffer=new FixBuffer(newdata,outDataLength);
    GDALClose(pMemDataSet);
    GDALClose(pDataSet);
    VSIUnlink(savingPath.c_str());
    return buffer;
}
MemRasterDataset::MemRasterDataset()
{
    name="";
    psp=nullptr;
}

MemRasterDataset::~MemRasterDataset()
{

}

string MemRasterDataset::GetDescription(){
   return "内存栅格数据集";
}
AutoPtr<Workspace>MemRasterDataset::GetWorkspace(){
   return new MemRasterWorkspace();
}

string MemRasterDataset::GetName(){
   return name;
}

string MemRasterDataset::GetTypeName(){
    return "MemRasterDataset";
}

bool MemRasterDataset::TemporaryDataset(){
   return true;
}

RasterDesp MemRasterDataset::GetRasterDesp(){
   return rasterDesp;
}

void MemRasterDataset::SetLeftTop(double left,double top){
   rasterDesp.left=left;
   rasterDesp.top=top;
}
void MemRasterDataset::SetCellSize(double xCellSize,double yCellSize){
    rasterDesp.xCellSize=xCellSize;
    rasterDesp.yCellSize=yCellSize;
}
AutoPtr<SpatialReference>MemRasterDataset::GetSpatialReference(){
    if(psp==nullptr) return new SpatialReference();
    return psp->Clone();
}
bool MemRasterDataset::SetSpatialReference(AutoPtr<SpatialReference>psp){
    if(psp==nullptr)
        this->psp=nullptr;
    else
        this->psp=psp->Clone();
    return true;
}
int MemRasterDataset::GetBandCount(){
    return bands.size();
}
AutoPtr<RasterBand>MemRasterDataset::GetRasterBand(int nIndex){
    AutoPtr<MemRasterBand>pBand(new MemRasterBand);
    pBand->bandData=bands[nIndex];
    pBand->rasterDesp=rasterDesp;
    pBand->bandIndex=nIndex;
    if(psp!=nullptr) pBand->psp=this->psp->Clone();
    pBand->nodata=nodata;
    if(bands.size()==1)
        pBand->name=name;
    else
        pBand->name=name+"_"+to_string(nIndex+1);
    pBand->dataType=this->bands[nIndex]->GetDataType();
    pBand->father=new MemRasterDataset();
    pBand->father->name=this->name;
    pBand->rasterDesp=this->rasterDesp;
    pBand->father->psp=psp;
    pBand->father->rasterDesp=this->rasterDesp;
    pBand->father->bands.resize(bands.size());
    pBand->father->colorTables.resize(colorTables.size());
    for(int k=0;k<bands.size();k++){
        pBand->father->bands[k]=bands[k];
        pBand->father->colorTables[k]=colorTables[k];
    }
    pBand->father->nodata=nodata;
    pBand->colorTable=colorTables[nIndex];
    return pBand;
}
bool MemRasterDataset::Save(string savingPath){
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
    else if(bandCount==1){
        if((sType=="PNG")||(sType=="JPEG")){
            bool hasTrans=false;
            AutoPtr<MemRasterBand>band=this->GetRasterBand(0);
            AutoPtr<ColorTable>table=band->GetColorTable();
            if(table!=nullptr){
                int crCount=table->GetColors();
                BYTE r,g,b,a;
                for(int k=0;k<crCount;k++){
                    table->GetColor(k,r,g,b,a);
                    if(a<255){
                        hasTrans=true;
                        break;
                    }
                }
            }
            if(hasTrans){
                createBand=4;
            }
        }
    }
    else if(bandCount==2){
        if((sType=="PNG")||(sType=="JPEG")) createBand=1;
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
    if(createBand<=bandCount){
        for(int k=0;k<bandCount;k++){
            GDALRasterBand *pBand=pMemDataSet->GetRasterBand(k+1);
            pBand->SetNoDataValue(nodata);
            AutoPtr<MemRasterBand>band=this->GetRasterBand(k);
            pBand->RasterIO(GF_Write, 0,0, desp.cols,desp.rows,band->GetRawData()->GetValues(), desp.cols,desp.rows, (GDALDataType)RasterDataTypeHelper::GetGDALDataType(firstBand->GetDataType()),0, 0 );
        }
    }
    else{
        AutoPtr<MemRasterBand>band=this->GetRasterBand(0);
        AutoPtr<ColorTable>table=band->GetColorTable();
        BYTE*values=(BYTE*)band->GetRawData()->GetValues();
        BYTE*rvalues=new BYTE[desp.cols*desp.rows];
        BYTE r,g,b,a;
        int crCount=table->GetColors();
        for(int k=0;k<4;k++){
            GDALRasterBand *pBand=pMemDataSet->GetRasterBand(k+1);
            int nPos=0;
            for(int i=0;i<desp.rows;i++){
                for(int j=0;j<desp.cols;j++){
                    int colorIndex=values[nPos];
                    if(colorIndex>=crCount)
                        r=g=b=a=0;
                    else{
                        table->GetColor(colorIndex,r,g,b,a);
                    }
                    if(k==0)
                       rvalues[nPos]=r;
                    else if(k==1)
                       rvalues[nPos]=g;
                    else if(k==2)
                       rvalues[nPos]=b;
                    else if(k==3)
                       rvalues[nPos]=a;
                    nPos++;
                }
            }
            pBand->RasterIO(GF_Write, 0,0, desp.cols,desp.rows,rvalues, desp.cols,desp.rows, GDALDataType::GDT_Byte,0, 0 ); 
        }
        delete []rvalues;
    }
    if(createBand==1){
        AutoPtr<MemRasterBand>band=this->GetRasterBand(0);
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
                BYTE r,g,b,a;
                table->GetColor(k,r,g,b,a);
                gce.c1=r;
                gce.c2=g;
                gce.c3=b;
                gce.c4=a;
                ct.SetColorEntry(k,&gce);
            }
            pBand->SetColorTable(&ct);
        }
    }
    GDALDataset * pDataSet = pDstDriver->CreateCopy(StringHelper::ToLocalString(savingPath).c_str(),pMemDataSet,FALSE, NULL, NULL, NULL);
    if (pDataSet == nullptr)
    {
        GDALClose(pMemDataSet);
        return false;
    }
    GDALClose(pMemDataSet);
    GDALClose(pDataSet);
    return true;
}
AutoPtr<FixBuffer>MemRasterDataset::SaveAsBuffer(string exa){
    int bandCount=this->GetBandCount();
    if(bandCount==0) return nullptr;
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
    if (pDstDriver == nullptr) return nullptr;
    RasterDesp desp=this->GetRasterDesp();
    GDALDriver *pMemDriver = nullptr;
    pMemDriver = GetGDALDriverManager()->GetDriverByName("MEM");
    if( pMemDriver == nullptr ) return nullptr;
    AutoPtr<RasterBand>firstBand=this->GetRasterBand(0);
    GDALDataset * pMemDataSet = pMemDriver->Create("",desp.cols,desp.rows,createBand,(GDALDataType)RasterDataTypeHelper::GetGDALDataType(firstBand->GetDataType()),nullptr);
    if(pMemDataSet==nullptr) return nullptr;
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
    for(int k=0;k<bandCount;k++){
       GDALRasterBand *pBand=pMemDataSet->GetRasterBand(k+1);
       pBand->SetNoDataValue(nodata);
       AutoPtr<MemRasterBand>band=this->GetRasterBand(k);
       pBand->RasterIO(GF_Write, 0,0, desp.cols,desp.rows,band->GetRawData()->GetValues(), desp.cols,desp.rows, (GDALDataType)RasterDataTypeHelper::GetGDALDataType(firstBand->GetDataType()),0, 0 );
    }
    if(bandCount==1){
        AutoPtr<MemRasterBand>band=this->GetRasterBand(0);
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

    string uuid=Guid::CreateGuid();
    string savingPath= "/vsimem/mem_"+uuid+"."+exa;
    GDALDataset * pDataSet = pDstDriver->CreateCopy(savingPath.c_str(),pMemDataSet,FALSE, NULL, NULL, NULL);
    if (pDataSet == nullptr)
    {
        GDALClose(pMemDataSet);
        return nullptr;
    }
    vsi_l_offset outDataLength;
    int bUnlinkAndSeize = FALSE;
    GByte * binData = VSIGetMemFileBuffer(savingPath.c_str(), &outDataLength, bUnlinkAndSeize);
    char*newdata=new char[outDataLength];
    memcpy(newdata,(char*)binData,outDataLength);
    FixBuffer*buffer=new FixBuffer(newdata,outDataLength);
    GDALClose(pMemDataSet);
    GDALClose(pDataSet);
    VSIUnlink(savingPath.c_str());
    return buffer;
}
bool MemRasterDataset::IsValid(){
    return (bands.size()>0);
}
void MemRasterDataset::Dispose(){
    bands.clear();
    psp=nullptr;
    name="";
}

AutoPtr<RasterDataset>MemRasterDataset::Clone(){
    AutoPtr<MemRasterDataset>newds=new MemRasterDataset();
    newds->name=name;
    newds->rasterDesp=rasterDesp;
    newds->psp=psp->Clone();
    newds->bands.resize(bands.size());
    for(int k=0;k<bands.size();k++){
        newds->bands[k]=bands[k];
    }
    newds->colorTables.resize(colorTables.size());
    for(int k=0;k<colorTables.size();k++){
        if(colorTables[k]==nullptr)
            newds->colorTables[k]=nullptr;
        else
            newds->colorTables[k]=colorTables[k]->Clone();
    }
    newds->nodata=nodata;
    return newds;
}

MemRasterWorkspace::MemRasterWorkspace(){

}
MemRasterWorkspace::~MemRasterWorkspace(){

}
//RasterWorkspace:
string MemRasterWorkspace::GetDescription(){
    return "内存栅格数据工作空间";
}

AutoPtr<MemRasterDataset>MemRasterWorkspace::CreateRasterDataset(string Name,RasterDesp rasterDesp,LONG BandCount,RasterDataType dataType,DOUBLE NoData,SpatialReference*psp){
    if(dataType==rdtUnknown) return nullptr;
    LONG Width=rasterDesp.cols;
    LONG Height=rasterDesp.rows;
    if((Width<=0)||(Height<=0)) return nullptr;
    DOUBLE xCellSize=fabs(rasterDesp.xCellSize);
    DOUBLE yCellSize=fabs(rasterDesp.yCellSize);
    if((xCellSize==0)||(yCellSize==0)) return nullptr;
    AutoPtr<MemRasterDataset>pRaster(new MemRasterDataset);
    pRaster->rasterDesp=rasterDesp;
    pRaster->name=Name;
    pRaster->psp=nullptr;
    pRaster->nodata=NoData;
    if(psp!=nullptr) pRaster->psp=psp->Clone();
    for(int k=0;k<BandCount;k++){
        AutoPtr<MemDataArray>banddata(new MemDataArray());
        if(!banddata->Initialize(rasterDesp.cols*rasterDesp.rows,dataType,NoData)) return nullptr;
        pRaster->bands.push_back(banddata);
        pRaster->colorTables.push_back(nullptr);
    }
    return pRaster;
}

MemRasterWorkspaceFactory::MemRasterWorkspaceFactory(){

}
MemRasterWorkspaceFactory::~MemRasterWorkspaceFactory(){

}
//RasterWorkspaceFactory:
string MemRasterWorkspaceFactory::GetDescription(){
   return "内存栅格";
}
AutoPtr<WorkspaceFactory>MemRasterWorkspaceFactory::Clone(){
   return new MemRasterWorkspaceFactory();
}

AutoPtr<MemRasterWorkspace>MemRasterWorkspaceFactory::CreateMemRasterWorkspace(){
    return new MemRasterWorkspace();
}

}
