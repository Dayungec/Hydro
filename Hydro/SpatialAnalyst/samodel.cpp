#include "samodel.h"
#include "Datasources/memrasterdataset.h"
#include "Datasources/rasterbandtool.h"
#include "Datasources/featureclasstool.h"
#include "featuretoraster.h"


namespace SGIS{

SAEnvironment::SAEnvironment()
{
    left=top=xCellSize=yCellSize=cols=rows=0;
    psp=new SpatialReference();
    maskBand=nullptr;
}

SAEnvironment::SAEnvironment(RasterDesp desp,SpatialReference*psp){
    left=desp.left;
    top=desp.top;
    cols=desp.cols;
    rows=desp.rows;
    xCellSize=desp.xCellSize;
    yCellSize=desp.yCellSize;
    if(psp!=nullptr)
       this->psp->CopyFromOther(psp);
    else
       this->psp=new SpatialReference();
    maskBand=nullptr;
}

SAEnvironment::~SAEnvironment(){

}

double&SAEnvironment::Left(){
    return left;
}

double&SAEnvironment::Top(){
    return top;
}

double&SAEnvironment::XCellSize(){
    return xCellSize;
}
double&SAEnvironment::YCellSize(){
    return yCellSize;
}

int&SAEnvironment::Cols(){
    return cols;
}
int&SAEnvironment::Rows(){
    return rows;
}

RasterDesp SAEnvironment::GetRasterDesp(){
    return RasterDesp(left,top,cols,rows,xCellSize,yCellSize);
}

AutoPtr<SpatialReference>SAEnvironment::GetSpatialReference(){
    return this->psp->Clone();
}

void SAEnvironment::SetSpatialReference(AutoPtr<SpatialReference>psp){
    if(psp!=nullptr)
       this->psp->CopyFromOther(psp.get());
    else
       this->psp=new SpatialReference();
}

AutoPtr<Point>SAEnvironment::GetLeftTop(){
    return new Point(left,top);
}

double SAEnvironment::GetRight(){
    return left+xCellSize*cols;
}
double SAEnvironment::GetBottom(){
    return top-yCellSize*rows;
}

AutoPtr<RasterBand>SAEnvironment::GetMaskBand(){
    return maskBand;
}

AutoPtr<Envelope>SAEnvironment::GetExtent(){
    return new Envelope(left,top,left+xCellSize*cols,top-yCellSize*rows);
}

void SAEnvironment::CreateFromRasterBand(RasterBand*pBand,bool autoMask){
   RasterDesp desp=pBand->GetRasterDesp();
   left=desp.left;
   top=desp.top;
   cols=desp.cols;
   rows=desp.rows;
   xCellSize=desp.xCellSize;
   yCellSize=desp.yCellSize;
   AutoPtr<SpatialReference>psp=pBand->GetSpatialReference();
   if(psp!=nullptr)
      this->psp->CopyFromOther(psp.get());
   else
      this->psp=new SpatialReference();
   if(autoMask){
       CreateMaskBandByRasterBand(pBand,nullptr);
   }
}

void SAEnvironment::CreateFromFeatureClass(FeatureClass*pfc,double cellSize){
    this->xCellSize=this->yCellSize=cellSize;
    AutoPtr<SelectionSet>selSet=pfc->GetSelectionSet();
    int selCount=selSet->GetSize();
    if((selCount==0)||(selCount==pfc->GetFeatureCount())){
        AutoPtr<Envelope>ext=pfc->GetExtent();
        left=ext->Left();
        top=ext->Top();
        double right=ext->Right();
        double bottom=ext->Bottom();
        cols=(right-left)/cellSize;
        if(cols*cellSize<right-left) cols++;
        rows=(top-bottom)/cellSize;
        if(rows*cellSize<top-bottom) rows++;
    }
    else{
        AutoPtr<Envelope>ext=nullptr;
        AutoPtr<Envelope>subEnv=new Envelope();
        AutoPtr<FeatureClassTool>pTool=new FeatureClassTool(pfc);
        for(int k=0;k<selCount;k++){
            int recordIndex=pTool->FindFeatureID(selSet->GetFeatureID(k));
            if(recordIndex==-1) continue;
            if(!pfc->GetFeatureEnvelope(recordIndex,subEnv.get())) continue;
            if(ext==nullptr)
                ext=subEnv;
            else
                ext->UnionOther(subEnv.get());
        }
        left=ext->Left();
        top=ext->Top();
        double right=ext->Right();
        double bottom=ext->Bottom();
        cols=(right-left)/cellSize;
        if(cols*cellSize<right-left) cols++;
        rows=(top-bottom)/cellSize;
        if(rows*cellSize<top-bottom) rows++;
    }
    AutoPtr<SpatialReference>psp=pfc->GetSpatialReference();
    if(psp!=nullptr)
       this->psp->CopyFromOther(psp.get());
    else
       this->psp=new SpatialReference();

}

void SAEnvironment::CreateFromGeometry(Geometry*geo,SpatialReference*psp,double cellSize,bool autoMask){
    this->xCellSize=this->yCellSize=cellSize;
    AutoPtr<Envelope>ext=geo->GetExtent();
    left=ext->Left();
    top=ext->Top();
    double right=ext->Right();
    double bottom=ext->Bottom();
    cols=(right-left)/cellSize;
    if(cols*cellSize<right-left) cols++;
    rows=(top-bottom)/cellSize;
    if(rows*cellSize<top-bottom) rows++;
    if(psp!=nullptr)
       this->psp->CopyFromOther(psp);
    else
       this->psp=new SpatialReference();
    if(autoMask) CreateMaskBandByGeometry(geo,psp);
}

void SAEnvironment::CreateMaskBandByGeometry(Geometry*geo,SpatialReference*psp){
    this->maskBand=nullptr;
    AutoPtr<FeatureToRaster>ftr=new FeatureToRaster();
    AutoPtr<RasterBand>maskBand=this->maskBand;
    this->maskBand=nullptr;
    ftr->SetEnvironment(AutoPtr<SAEnvironment>(this,true));
    AutoPtr<MemRasterTarget>target(new MemRasterTarget());
    this->maskBand=ftr->ConvertGeometry(geo,psp,target.get());
}

bool SAEnvironment::CreateMaskBandByRasterBand(RasterBand*pBand,CallBack*callBack,Semaphora*signal){
   this->maskBand=nullptr;
   AutoPtr<MemRasterWorkspaceFactory>pFac(new MemRasterWorkspaceFactory());
   AutoPtr<MemRasterWorkspace>pWork=pFac->CreateMemRasterWorkspace();
   AutoPtr<RasterDataset>rd=pWork->CreateRasterDataset("Mask",GetRasterDesp(),1,rdtByte,255,GetSpatialReference().get());
   maskBand=rd->GetRasterBand(0);
   float NoData=pBand->GetNodata();
   AutoPtr<Point>LeftTop=GetLeftTop();
   AutoPtr<SpatialReference>psp=GetSpatialReference();
   DOUBLE left,top;
   LeftTop->GetCoord(&left,&top);
   DOUBLE xCellSize=XCellSize();
   DOUBLE yCellSize=YCellSize();
   LONG DifRows=128;
   AutoPtr<Point>BlockLeftTop(new Point());
   AutoPtr<MemRasterBand>pMaskBand(maskBand);
   BYTE*ayData=pMaskBand->GetRawData()->GetValues();
   CallBackHelper::BeginProgress(callBack,"装载掩膜数据");
   float fValue;
   AutoPtr<RasterBandTool>mTool(new RasterBandTool(pBand));
   LONG formerRows=0;
   FLOAT*fV=nullptr;
   for(int i=0;i<rows;i+=DifRows)
   {
       int dif=DifRows;
       if(i+dif-1>=rows) dif=rows-i;
       if(formerRows!=dif)
       {
           if(fV!=nullptr) delete []fV;
           fV=new FLOAT[dif*cols];
           formerRows=dif;
       }
       BlockLeftTop->PutCoord(left,top-i*yCellSize);
       mTool->GetBlockDataByCoord(BlockLeftTop.get(),xCellSize,yCellSize,cols,dif,fV,psp.get(),NoData,false);
       for(int p=i;p<i+dif;p++)
       {
           long pos=(p-i)*cols;
           for(int m=0;m<cols;m++)
           {
               fValue=fV[pos];
               if(fValue==NoData)
                   ayData[m+p*cols]=0;
               else
                   ayData[m+p*cols]=1;
               pos++;
           }
       }
       if(signal!=nullptr){
           if(signal->Wait(5)){
               if(fV!=nullptr) delete []fV;
               return false;
           }
       }
       CallBackHelper::SendProgress(callBack,(float)i/rows*100);
   }
   if(fV!=nullptr) delete []fV;
   return true;
}

bool SAEnvironment::CreateMaskBandByFeatureClass(FeatureClass*pfc,CallBack*callBack,Semaphora*signal){
    AutoPtr<FeatureToRaster>ftr(new FeatureToRaster());
    AutoPtr<RasterBand>maskBand=this->maskBand;
    ftr->SetEnvironment(AutoPtr<SAEnvironment>(this,true));
    AutoPtr<RasterBand>pBand=ftr->ConvertMask(pfc,signal);
    if(pBand==nullptr) return false;
    this->maskBand=pBand;
    return true;
}


AutoPtr<SAEnvironment>SAEnvironment::Clone(bool withMask){
    SAEnvironment*newr=new SAEnvironment();
    newr->cols=cols;
    newr->rows=rows;
    newr->xCellSize=xCellSize;
    newr->yCellSize=yCellSize;
    newr->left=left;
    newr->top=top;
    newr->psp=((psp==nullptr)?nullptr:psp->Clone());
    if(withMask){
       newr->maskBand=maskBand;
    }
    return newr;
}

bool SAEnvironment::CreateMaskBand(CallBack*callBack,Semaphora*signal){
    if(maskBand!=nullptr) return true;
    return true;
}

SAModel::SAModel(){

}
SAModel::~SAModel(){

}

bool SAModel::CheckValid(){
    if(pEnvi==nullptr) return false;
    if((pEnvi->Cols()==0)||(pEnvi->Rows()==0)||(pEnvi->XCellSize()==0)||(pEnvi->YCellSize()==0)) return false;
    return true;
}
AutoPtr<SAEnvironment>SAModel::GetEnvironment(){
    return pEnvi;
}
void SAModel::SetEnvironment(AutoPtr<SAEnvironment>envi){
    pEnvi=envi;
}

BYTE*SAModel::GetMaskData(CallBack*callBack,Semaphora*siganl,bool&bSuc){
    if(!pEnvi->CreateMaskBand(callBack,siganl)){
        bSuc=false;
        return nullptr;
    }
    bSuc=true;
    AutoPtr<MemRasterBand>pMaskBand=pEnvi->GetMaskBand();
    if(pMaskBand==nullptr) return nullptr;
    return pMaskBand->GetRawData()->GetValues();
}

bool SAModel::DetectExitSignal(CallBack*callBack,Semaphora*signal){
    if(signal==nullptr) return false;
    DateTime curTime;
    if((curTime-detectTime).TotalSeconds()<1) return false;
    detectTime=curTime;
    if(signal->Wait(5)){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return true;
    }
    return false;
}


}
