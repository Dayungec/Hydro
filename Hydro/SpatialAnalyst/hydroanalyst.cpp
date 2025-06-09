#include "hydroanalyst.h"
#include "Base/FilePath.h"
#include "Base/classfactory.h"
#include "Base/variant.h"
#include "Datasources/rasterbandtool.h"
//#include "MapDisplay/featurelegend.h"
//#include "analysemodel.h"

namespace SGIS{

REGISTER(HydroDemFill)
REGISTER(HydroLake)
REGISTER(HydroLakeTable)
REGISTER(HydroFlowDirection)
REGISTER(HydroFlowAccumulation)
REGISTER(HydroFlowLength)
REGISTER(HydroLakePouringLine)
REGISTER(HydroDig)
REGISTER(HydroLakePouringLineDig)

HydroAnalyst::HydroAnalyst(){
    nodata=-32768;
}
HydroAnalyst::~HydroAnalyst(){

}
string HydroAnalyst::GetCatagory(){
    return "空间分析/水文分析";
}

FLOAT&HydroAnalyst::OutputNoData(){
    return nodata;
}

HydroDemFill::HydroDemFill(){

}
HydroDemFill::~HydroDemFill(){

}
string HydroDemFill::GetTypeName(){
    return "HydroDemFill";
}

void HydroDemFill::FillCell(AutoArray<Point2D<int>>&pointArray,FourValueArray&fvArray,FLOAT*pData,int Cols,int Rows, float&fillDem){
    Point2D<int>firstPoint=pointArray.GetItem(0);
    float Dem=pData[firstPoint.X+firstPoint.Y*Cols];
    fillDem = Dem;
    while (true)
    {
        bool HasFindBig = false;//是否找到更高的地形
        float damDem = Dem;//更高的地形
        bool IsInBorder = false;//是否找到更底的地形
        int pointCount = pointArray.GetSize();
        for (int k = 0; k < pointCount; k++)
        {
            Point2D<int> point = pointArray.GetItem(k);
            int i = point.Y;
            int j = point.X;
            for (int m = 0; m < 8; m++)
            {
                int ci = i + aroundy8[m];
                int cj = j + aroundx8[m];
                if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                    IsInBorder = true;
                    break;
                }
                if (fvArray[ci*Cols + cj] == 3) continue;
                float fV = pData[ci*Cols + cj];
                if (nodata == fV) {
                    IsInBorder = true;
                    break;
                }
                if (fV == Dem) {
                    pointArray.Add(Point2D<int>(cj, ci));
                    fvArray.SetValue(ci*Cols + cj, 3);
                    pointCount++;
                }
                else if (fV < Dem) {
                    IsInBorder = true;
                }
                else if (fV > Dem) {
                    if (!HasFindBig)
                        damDem = fV;
                    else if (fV < damDem)
                        damDem = fV;
                    HasFindBig = true;
                }
            }
        }
        if(IsInBorder) break;
        Dem = damDem;
        fillDem = Dem;
        if(!HasFindBig) break;
    }
}

void HydroDemFill::FillArea(AutoArray<Point2D<int>>&pointArray,FourValueArray&fvArray,FLOAT*pData,int Cols,int Rows, float fillDem){
    int pointCount = pointArray.GetSize();
    LONG pos;
    for (int k = 0; k < pointCount; k++)
    {
        Point2D<int>point = pointArray.GetItem(k);
        pos = point.Y*Cols + point.X;
        pData[pos] = fillDem;
        fvArray.SetValue(pos, 2);
    }
}

AutoPtr<RasterBand>HydroDemFill::Execute(RasterBand*demBand,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    LONG Cols=desp.cols;
    LONG Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    CallBackHelper::SendMessage(callBack,"装载数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(demBand));
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0) pData[Pos]=nodata;
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    CallBackHelper::SendMessage(callBack,"填充局部洼地");
    FourValueArray fvArray(Cols*Rows);
    for (int i = 1; i < Rows - 1; i++)
    {
        LONG Pos = i * Cols + 1;
        for (int j = 1; j < Cols - 1; j++)
        {
            float cV = pData[Pos];
            if (nodata == (LONG)cV)
            {
                Pos++;
                continue;
            }
            float minV = cV;
            int nPos = -1;
            for (int m = 0; m < 8; m++)
            {
                int ci = i + aroundy8[m];
                int cj = j + aroundx8[m];
                if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                    minV=cV-1;
                    break;
                }
                float nV = pData[ci*Cols + cj];
                if (nodata == nV){
                    minV=cV-1;
                    continue;
                }
                if (nPos == -1)
                {
                    nPos = 0;
                    minV = nV;
                }
                else if (nV < minV)
                {
                    minV = nV;
                }
            }
            if (nPos == 0)
            {
                if (minV >= cV)
                {
                    pData[Pos] = minV;
                    fvArray.SetValue(Pos, 1);
                }
            }
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
              return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    AutoArray<Point2D<int>>pointArray;
    //1:表示初始洼地；2--表示已填充洼地；3--表示正填充洼地
    CallBackHelper::SendMessage(callBack,"局部洼地微调升高");
    FLOAT fillDem;
    for (int i = 0; i < Rows; i++)
    {
        LONG Pos = i * Cols;
        for (int j = 0; j < Cols; j++)
        {
            if (fvArray[Pos] != 1) {
                Pos++;
                continue;
            }
            fvArray.SetValue(Pos, 3);
            pointArray.Clear();
            pointArray.Add(Point2D<int>(j, i));
            FillCell(pointArray, fvArray, pData, Cols, Rows, fillDem);
            FillArea(pointArray, fvArray, pData, Cols, Rows, fillDem);
            //if(smooth) SmoothArea(pointArray, pData, Cols, Rows);
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
              return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    RasterDataType dType=RasterDataType::rdtFloat32;
    //if(smooth)
        //dType=RasterDataType::rdtFloat32;
    //else
    dType=demBand->GetDataType();
    AutoPtr<RasterDataset>pDataset=target->CreateRasterDataset(pEnvi.get(),dType,1,nodata);
    if(pDataset==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建栅格数据失败");
        return nullptr;
    }
    AutoPtr<RasterBand>tb=pDataset->GetRasterBand(0);
    tb->SaveBlockData(0,0,Cols,Rows,pData);
    CallBackHelper::SendMessage(callBack,"填充洼地完成");
    tb->FlushCache();
    return tb;
}

HydroLake::HydroLake(){

}
HydroLake::~HydroLake(){

}
string HydroLake::GetTypeName(){
    return "HydroLake";
}

AutoPtr<RasterBand>HydroLake::Execute(RasterBand*fillBand,int lakeMinRadius,bool demFill,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    LONG Cols=desp.cols;
    LONG Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    AutoPtr<FixBuffer>buffer2=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer2->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*resultData=(float*)buffer2->GetBuffer();
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    CallBackHelper::SendMessage(callBack,"装载地形数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(fillBand));
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                resultData[Pos]=nodata;
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0){
                    pData[Pos]=nodata;
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }else{
        CallBackHelper::SendMessage(callBack,"初始化输出");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                resultData[Pos]=nodata;
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    AutoArray<Point2D<int>>pointArray;
    CallBackHelper::SendMessage(callBack,"提取洼地");
    FourValueArray fvArray(Cols*Rows);
    fvArray.SetDefaultValue(0);
    ImageLumpBlock imageLumpBlock;
    imageLumpBlock.CreateImageBlock(Cols,Rows);
    OrderArray<int>accuOrders(true);
    vector<Point2D<int>>sinkPoses;
    RasterDataType dt=fillBand->GetDataType();
    if(demFill) dt=RasterDataType::rdtInt32;
    AutoPtr<RasterDataset>resultds=target->CreateRasterDataset(pEnvi.get(),dt,1,nodata);
    if(resultds==nullptr){
        CallBackHelper::SendMessage(callBack,"创建洼地图层失败");
        return nullptr;
    }
    AutoPtr<RasterBand>resultBand=resultds->GetRasterBand(0);
    int indx=1;
    for (int i = 1; i < Rows - 1; i++){
        LONG Pos = i * Cols + 1;
        for (int j = 1; j < Cols - 1; j++)
        {
            float cV = pData[Pos];
            if (nodata == (LONG)cV)
            {
                Pos++;
                continue;
            }
            if(fvArray[Pos]!=0){
                Pos++;
                continue;
            }
            pointArray.Clear();
            pointArray.Add(Point2D<int>(j,i));
            fvArray.SetValue(Pos,2);
            for(int k=0;k<pointArray.GetSize();k++){
                Point2D<int>cp=pointArray[k];
                int ni=cp.Y;
                int nj=cp.X;
                for (int m = 0; m < 8; m++){
                    int ci = ni + aroundy8[m];
                    int cj = nj + aroundx8[m];
                    if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                        continue;
                    }
                    int nPos=ci*Cols + cj;
                    float nV = pData[nPos];
                    if (nodata == nV){
                        continue;
                    }
                    if(fvArray[nPos]!=0) continue;
                    if(nV==cV){
                        pointArray.Add(Point2D<int>(cj,ci));
                        fvArray.SetValue(nPos,2);
                    }
                }
            }
            int ptSize=pointArray.GetSize();
            if(ptSize<4){
                Pos++;
                continue;
            }
            imageLumpBlock.UpdateImageBlock(pointArray);
            imageLumpBlock.ExtractBorder();
            if(imageLumpBlock.GetMaxValue()<lakeMinRadius){
                Pos++;
                continue;
            }
            for(int l=0;l<ptSize;l++){
                Point2D<int>cp=pointArray[l];
                if(demFill)
                   resultData[cp.X+cp.Y*Cols]=cV;
                else
                   resultData[cp.X+cp.Y*Cols]=indx;
            }
            indx++;
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    CallBackHelper::SendMessage(callBack,"洼地提取完成");
    resultBand->SaveBlockData(0,0,Cols,Rows,resultData);
    resultBand->FlushCache();
    return resultBand;
}

HydroLakeTable::HydroLakeTable(){

}
HydroLakeTable::~HydroLakeTable(){

}
string HydroLakeTable::GetTypeName(){
    return "HydroLakeTable";
}

AutoPtr<DataTable>HydroLakeTable::Execute(RasterBand*demBand,RasterBand*fillBand,int lakeMinRadius,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    LONG Cols=desp.cols;
    LONG Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    CallBackHelper::SendMessage(callBack,"装载地形数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(fillBand));
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);

    AutoPtr<FixBuffer>buffer2=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer2->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*oldData=(float*)buffer2->GetBuffer();
    CallBackHelper::SendMessage(callBack,"装载原始地形数据");
    AutoPtr<RasterBandTool>pTool2(new RasterBandTool(demBand));
    pTool2->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,oldData,psp.get(),nodata,false);
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0){
                    pData[Pos]=nodata;
                    oldData[Pos]=nodata;
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    AutoArray<Point2D<int>>pointArray;
    CallBackHelper::SendMessage(callBack,"提取洼地");
    FourValueArray fvArray(Cols*Rows);
    fvArray.SetDefaultValue(0);
    ImageLumpBlock imageLumpBlock;
    imageLumpBlock.CreateImageBlock(Cols,Rows);
    OrderArray<int>accuOrders(true);
    vector<Point2D<int>>sinkPoses;
    int indx=1;
    AutoPtr<DataTable>table=new DataTable("DT",{
        DatabaseField("ID",DatabaseFieldType::ftInteger),
        DatabaseField("Count",DatabaseFieldType::ftInteger),
        DatabaseField("FillH",DatabaseFieldType::ftNumeric),
        DatabaseField("MinH",DatabaseFieldType::ftNumeric),
        DatabaseField("DeltaH",DatabaseFieldType::ftNumeric),
        DatabaseField("Radius",DatabaseFieldType::ftNumeric)
    });
    vector<Variant>values(table->GetFields()->GetSize());
    for (int i = 1; i < Rows - 1; i++){
        LONG Pos = i * Cols + 1;
        for (int j = 1; j < Cols - 1; j++)
        {
            float cV = pData[Pos];
            if (nodata == (LONG)cV)
            {
                Pos++;
                continue;
            }
            if(fvArray[Pos]!=0){
                Pos++;
                continue;
            }
            pointArray.Clear();
            pointArray.Add(Point2D<int>(j,i));
            fvArray.SetValue(Pos,2);
            float minh=-9999,maxh=-9999;
            for(int k=0;k<pointArray.GetSize();k++){
                Point2D<int>cp=pointArray[k];
                int ni=cp.Y;
                int nj=cp.X;
                for (int m = 0; m < 8; m++){
                    int ci = ni + aroundy8[m];
                    int cj = nj + aroundx8[m];
                    if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                        continue;
                    }
                    int nPos=ci*Cols + cj;
                    float nV = pData[nPos];
                    if (nodata == nV){
                        continue;
                    }
                    if(fvArray[nPos]!=0) continue;
                    if(nV==cV){
                        pointArray.Add(Point2D<int>(cj,ci));
                        fvArray.SetValue(nPos,2);
                        float dV = oldData[nPos];
                        if (nodata != dV){
                            if(minh==-9999){
                                minh=maxh=dV;
                            }
                            else{
                                if(dV<minh) minh=dV;
                                if(dV>maxh) maxh=dV;
                            }
                        }
                    }
                }
            }
            int ptSize=pointArray.GetSize();
            if(ptSize<4){
                Pos++;
                continue;
            }
            imageLumpBlock.UpdateImageBlock(pointArray);
            imageLumpBlock.ExtractBorder();
            if(imageLumpBlock.GetMaxValue()<lakeMinRadius){
                Pos++;
                continue;
            }
            int radius=imageLumpBlock.GetMaxValue();
            values[0]=indx;
            values[1]=ptSize;
            values[2]=cV;
            values[3]=minh;
            values[4]=maxh-minh;
            values[5]=radius;
            table->AddRecord(values);
            indx++;
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    CallBackHelper::SendMessage(callBack,"洼地提取完成");
    return table;
}

HydroFlowDirection::HydroFlowDirection(){
    Code[0]=32;
    Code[1]=64;
    Code[2]=128;
    Code[3]=1;
    Code[4]=2;
    Code[5]=4;
    Code[6]=8;
    Code[7]=16;
    Code2[0]=2;
    Code2[1]=4;
    Code2[2]=8;
    Code2[3]=16;
    Code2[4]=32;
    Code2[5]=64;
    Code2[6]=128;
    Code2[7]=1;
    nodata=255;
}

HydroFlowDirection::~HydroFlowDirection(){

}

string HydroFlowDirection::GetTypeName(){
    return "HydroFlowDirection";
}

float HydroFlowDirection::GetDirection(float*fVs,int x,int y)
{
    float nodata=-32768;
    double minV=nodata;
    int dire=-1;
    int validCount=0;
    bool bSplit=false;
    LONG pos=x+y*Cols;
    int nodataCount=0;
    for(int m=0;m<8;m++){
        int ni=aroundy8[m]+y;
        int nj=aroundx8[m]+x;
        float curV;
        if((ni<0)||(ni>=Rows)||(nj<0)||(nj>=Cols))
            curV=fVs[pos];
        else
            curV=fVs[nj+ni*Cols];
        if(nodata==curV){
            nodataCount++;
            continue;
        }
        if(validCount==0){
            minV=curV;
            validCount=1;
            dire=m;
        }
        else if(curV<minV){
            minV=curV;
            validCount=1;
            dire=m;
            bSplit=false;
        }
        else if(curV==minV)
            validCount++;
        else
            bSplit=true;
    }
    if(minV>fVs[pos]) validCount=0;
    if(validCount==0){
        if(nodataCount==0)
            return 0;
        int fromPos=0;
        for(int m=0;m<8;m++){
            int ni=aroundy8[m]+y;
            int nj=aroundx8[m]+x;
            float curV;
            if((ni<0)||(ni>=Rows)||(nj<0)||(nj>=Cols))
                curV=fVs[pos];
            else
                curV=fVs[nj+ni*Cols];
            if(nodata!=curV){
                fromPos=m;
                break;
            }
        }
        for(int m=7;m>=0;m--)
        {
            int pm=m+fromPos;
            if(pm>=8) pm-=8;
            int ni=aroundy8[pm]+y;
            int nj=aroundx8[pm]+x;
            float curV;
            if((ni<0)||(ni>=Rows)||(nj<0)||(nj>=Cols))
                curV=fVs[pos];
            else
                curV=fVs[nj+ni*Cols];
            if(curV!=nodata)
                break;
            else
                fromPos=pm;
        }
        int runLength=0;
        int maxrunLength=0;
        int maxFromPos=fromPos;
        int curFromPos=fromPos;
        for(int m=0;m<8;m++){
            int pm=m+fromPos;
            if(pm>=8) pm-=8;
            int ni=aroundy8[pm]+y;
            int nj=aroundx8[pm]+x;
            float curV;
            if((ni<0)||(ni>=Rows)||(nj<0)||(nj>=Cols))
                curV=fVs[pos];
            else
                curV=fVs[nj+ni*Cols];
            if(curV==nodata){
                if(runLength==0) curFromPos=pm;
                runLength++;
            }
            else{
                if(runLength>maxrunLength){
                    maxrunLength=runLength;
                    maxFromPos=curFromPos;
                }
                runLength=0;
            }
        }
        int dire2=maxFromPos+maxrunLength;
        dire=(maxFromPos+dire2)/2;
        dire=dire%8;
        return Code[dire];
    }
    bool IsReverse=(minV==fVs[pos]);
    if(minV>fVs[pos])
        return 0;
    else if(validCount==8){
        return (IsReverse?-1:1);
    }
    if(!bSplit){
        int dire2=dire+validCount;
        dire=(dire+dire2)/2;
        if(dire<0) dire+=8;
        dire=dire%8;
        return (IsReverse?-Code[dire]:Code[dire]);
    }
    else
    {
        int fromPos=dire;
        for(int m=7;m>=0;m--)
        {
            int pm=m+fromPos;
            if(pm>=8) pm-=8;
            int ni=aroundy8[pm]+y;
            int nj=aroundx8[pm]+x;
            float curV;
            if((ni<0)||(ni>=Rows)||(nj<0)||(nj>=Cols))
                curV=fVs[pos];
            else
                curV=fVs[nj+ni*Cols];
            if(curV!=minV)
                break;
            else
                fromPos=pm;
        }
        int runLength=0;
        int maxrunLength=0;
        int maxFromPos=fromPos;
        int curFromPos=fromPos;
        for(int m=0;m<8;m++){
            int pm=m+fromPos;
            if(pm>=8) pm-=8;
            int ni=aroundy8[pm]+y;
            int nj=aroundx8[pm]+x;
            float curV;
            if((ni<0)||(ni>=Rows)||(nj<0)||(nj>=Cols))
                curV=fVs[pos];
            else
                curV=fVs[nj+ni*Cols];
            if(curV==minV){
                if(runLength==0) curFromPos=pm;
                runLength++;
            }
            else{
                if(runLength>maxrunLength){
                    maxrunLength=runLength;
                    maxFromPos=curFromPos;
                }
                runLength=0;
            }
        }
        int dire2=maxFromPos+maxrunLength;
        dire=(maxFromPos+dire2)/2;
        dire=dire%8;
        return (IsReverse?-Code[dire]:Code[dire]);
    }
    return 0;
}

bool HydroFlowDirection::ComputeFlatDir(int x,int y,FourValueArray&fvArray,FLOAT*pData,FLOAT*pDatar)
{
    float nodata=-32768;
    float Dem=pData[x+y*Cols];
    AutoArray<Point2D<int>>pointArray;
    pointArray.Add(Point2D<int>(x,y));
    fvArray.SetValue(x+y*Cols,3);
    for(int k=0;k<pointArray.GetSize();k++){
        Point2D<int>point=pointArray.GetItem(k);
        int i=point.Y;
        int j=point.X;
        for(int m=0;m<8;m++){
            int ci=i+aroundy8[m];
            int cj=j+aroundx8[m];
            if((ci<0)||(ci>=Rows)||(cj<0)||(cj>=Cols)) continue;
            if(fvArray[ci*Cols+cj]==3) continue;
            float fV=pData[ci*Cols+cj];
            if(nodata==(LONG)fV) continue;
            if(fV==Dem){
                pointArray.Add(Point2D<int>(cj,ci));
                fvArray.SetValue(ci*Cols+cj,3);
            }
        }
    }
    int pointCount=pointArray.GetSize();
    float minV=Dem;
    int nPos;
    Point2D<int>minPos;
    bool HasFindSmall=false;
    Point2D<int>nodataMinPos(-1,-1);
    float minNoDataDem=nodata;
    int firstDir=0;
    int nodataFirstDir=0;
    for(int k=0;k<pointCount;k++){
        Point2D<int>point=pointArray.GetItem(k);
        int i=point.Y;
        int j=point.X;
        bool hasNoData=false;
        int nodataPos=0;
        for(int m=0;m<8;m++){
            int ci=i+aroundy8[m];
            int cj=j+aroundx8[m];
            if((ci<0)||(ci>=Rows)||(cj<0)||(cj>=Cols)){
                hasNoData=true;
                nodataPos=m;
                continue;
            }
            nPos=ci*Cols+cj;
            if(fvArray[nPos]==3) continue;
            float fV=pData[nPos];
            if(nodata==fV){
                hasNoData=true;
                nodataPos=m;
                continue;
            }
            if(fV==Dem) continue;
            if(fV<minV)
            {
                minV=fV;
                minPos=Point2D<int>(j,i);
                HasFindSmall=true;
                firstDir=Code[m];
            }
        }
        if(!HasFindSmall){
            if(hasNoData){
                float mind=0;
                int mincount=0;
                for(int m=0;m<8;m++){
                    int ci=i+aroundy8[m];
                    int cj=j+aroundx8[m];
                    if((ci<0)||(ci>=Rows)||(cj<0)||(cj>=Cols)) continue;
                    float fV=pData[ci*Cols+cj];
                    if(nodata==fV) continue;
                    mind+=fV;
                    mincount++;
                }
                if(mincount>0){
                    mind/=mincount;
                    if(nodataMinPos.X==-1){
                        minNoDataDem=mind;
                        nodataMinPos.X=j;
                        nodataMinPos.Y=i;
                        nodataFirstDir=Code[nodataPos];
                    }
                    else if(mind<minNoDataDem){
                        minNoDataDem=mind;
                        nodataMinPos.X=j;
                        nodataMinPos.Y=i;
                        nodataFirstDir=Code[nodataPos];
                    }
                }
            }
        }
    }
    if(!HasFindSmall){
        if(nodataMinPos.X==-1) return false;
        minPos=nodataMinPos;
        firstDir=nodataFirstDir;
    }
    pointArray.Clear();
    pointArray.Add(minPos);
    for(int k=0;k<pointArray.GetSize();k++){
        Point2D<int>point=pointArray.GetItem(k);
        int i=point.Y;
        int j=point.X;
        for(int m=0;m<8;m++){
            int ci=i+aroundy8[m];
            int cj=j+aroundx8[m];
            if((ci<0)||(ci>=Rows)||(cj<0)||(cj>=Cols)) continue;
            if(fvArray[ci*Cols+cj]!=3) continue;
            float fV=pData[ci*Cols+cj];
            if(nodata==(LONG)fV) continue;
            if(fV!=Dem) continue;
            pointArray.Add(Point2D<int>(cj,ci));
            nPos=ci*Cols+cj;
            fvArray.SetValue(nPos,2);
            pDatar[nPos]=Code2[m];
        }
    }
    nPos=minPos.Y*Cols+minPos.X;
    pDatar[nPos]=firstDir;
    return true;
}

bool HydroFlowDirection::ComputeFlatDir2(int x,int y,FourValueArray&fvArray,FLOAT*pData,FLOAT*pDatar,FLOAT*oldData){
    float nodata=-32768;
    float Dem=pData[x+y*Cols];
    AutoArray<Point2D<int>>pointArray;
    pointArray.Add(Point2D<int>(x,y));
    fvArray.SetValue(x+y*Cols,3);
    OrderArray<float>dems(true);
    vector<int>indices;
    int nPos;
    for(int k=0;k<pointArray.GetSize();k++){
        Point2D<int>point=pointArray.GetItem(k);
        int i=point.Y;
        int j=point.X;
        for(int m=0;m<8;m++){
            int ci=i+aroundy8[m];
            int cj=j+aroundx8[m];
            if((ci<0)||(ci>=Rows)||(cj<0)||(cj>=Cols)) continue;
            nPos=ci*Cols+cj;
            if(fvArray[nPos]==3) continue;
            float fV=pData[nPos];
            if(nodata==(LONG)fV) continue;
            if(fV==Dem){
                pointArray.Add(Point2D<int>(cj,ci));
                fvArray.SetValue(ci*Cols+cj,3);
            }
        }
    }
    int pointCount=pointArray.GetSize();
    float minV=Dem;
    Point2D<int>minPos;
    bool HasFindSmall=false;
    Point2D<int>nodataMinPos(-1,-1);
    float minNoDataDem=nodata;
    int firstDir=0;
    int nodataFirstDir=0;
    for(int k=0;k<pointCount;k++){
        Point2D<int>point=pointArray.GetItem(k);
        int i=point.Y;
        int j=point.X;
        bool hasNoData=false;
        int nodataPos=0;
        for(int m=0;m<8;m++){
            int ci=i+aroundy8[m];
            int cj=j+aroundx8[m];
            if((ci<0)||(ci>=Rows)||(cj<0)||(cj>=Cols)){
                hasNoData=true;
                nodataPos=m;
                continue;
            }
            nPos=ci*Cols+cj;
            if(fvArray[nPos]==3) continue;
            float fV=pData[nPos];
            if(nodata==fV){
                hasNoData=true;
                nodataPos=m;
                continue;
            }
            if(fV==Dem) continue;
            if(fV<minV)
            {
                minV=fV;
                minPos=Point2D<int>(j,i);
                HasFindSmall=true;
                firstDir=Code[m];
            }
        }
        if(!HasFindSmall){
            if(hasNoData){
                float mind=0;
                int mincount=0;
                for(int m=0;m<8;m++){
                    int ci=i+aroundy8[m];
                    int cj=j+aroundx8[m];
                    if((ci<0)||(ci>=Rows)||(cj<0)||(cj>=Cols)) continue;
                    float fV=pData[ci*Cols+cj];
                    if(nodata==fV) continue;
                    mind+=fV;
                    mincount++;
                }
                if(mincount>0){
                    mind/=mincount;
                    if(nodataMinPos.X==-1){
                        minNoDataDem=mind;
                        nodataMinPos.X=j;
                        nodataMinPos.Y=i;
                        nodataFirstDir=Code[nodataPos];
                    }
                    else if(mind<minNoDataDem){
                        minNoDataDem=mind;
                        nodataMinPos.X=j;
                        nodataMinPos.Y=i;
                        nodataFirstDir=Code[nodataPos];
                    }
                }
            }
        }
    }
    if(!HasFindSmall){
        if(nodataMinPos.X==-1) return false;
        minPos=nodataMinPos;
        firstDir=nodataFirstDir;
    }
    pointArray.Clear();
    pointArray.Add(minPos);
    for(int k=0;k<pointArray.GetSize();k++){
        Point2D<int>point=pointArray.GetItem(k);
        int i=point.Y;
        int j=point.X;
        dems.Clear();
        indices.clear();
        for(int m=0;m<8;m++){
            int ci=i+aroundy8[m];
            int cj=j+aroundx8[m];
            if((ci<0)||(ci>=Rows)||(cj<0)||(cj>=Cols)) continue;
            nPos=ci*Cols+cj;
            if(fvArray[nPos]!=3) continue;
            float fV=pData[nPos];
            if(nodata==(LONG)fV) continue;
            if(fV!=Dem) continue;
            int lPos=dems.Add(oldData[nPos]);
            indices.insert(begin(indices)+lPos,m);
        }
        for(int l=0;l<indices.size();l++){
            int ci=i+aroundy8[indices[l]];
            int cj=j+aroundx8[indices[l]];
            nPos=ci*Cols+cj;
            pointArray.Add(Point2D<int>(cj,ci));
            fvArray.SetValue(nPos,2);
            pDatar[nPos]=Code2[indices[l]];
        }
    }
    nPos=minPos.Y*Cols+minPos.X;
    pDatar[nPos]=firstDir;
    return true;
}

AutoPtr<RasterBand>HydroFlowDirection::Execute(RasterBand*fillBand,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    Cols=desp.cols;
    Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    AutoPtr<FixBuffer>buffer2=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer2->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pDatar=(float*)buffer2->GetBuffer();
    CallBackHelper::SendMessage(callBack,"装载数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(fillBand));
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    float nodata=-32768;
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0) pData[Pos]=nodata;
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    CallBackHelper::SendMessage(callBack,"填充局部洼地");
    LONG pos=0;
    CallBackHelper::SendMessage(callBack,"初始化");
    for(int i=0;i<Rows;i++)
    {
        for(int j=0;j<Cols;j++)
        {
            pDatar[pos++]=this->nodata;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    CallBackHelper::SendMessage(callBack,"计算水流方向");
    LONG Pos=0;
    FourValueArray fourArray(Cols*Rows);
    for(int i=0;i<Rows;i++)
    {
        for(int j=0;j<Cols;j++)
        {
            float dem=pData[Pos];
            if(nodata==dem)
            {
                Pos++;
                continue;
            }
            if(this->nodata!=pDatar[Pos])
            {
                Pos++;
                continue;
            }
            float dir=GetDirection(pData,j,i);
            if(dir<0)
            {
                if(!ComputeFlatDir(j,i,fourArray,pData,pDatar))
                    pDatar[Pos]=-dir;
            }
            else
                pDatar[Pos]=dir;
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    RasterDataType dType=rdtByte;
    if((this->nodata<0)||(this->nodata>255)) dType=rdtInt16;
    AutoPtr<RasterDataset>pDataset=target->CreateRasterDataset(pEnvi.get(),dType,1,this->nodata);
    if(pDataset==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建栅格数据失败");
        return nullptr;
    }
    AutoPtr<RasterBand>tb=pDataset->GetRasterBand(0);
    tb->SaveBlockData(0,0,Cols,Rows,pDatar);
    CallBackHelper::SendMessage(callBack,"水流方向计算完成");
    tb->FlushCache();
    return tb;
}

AutoPtr<RasterBand>HydroFlowDirection::Execute(RasterBand*demBand,RasterBand*fillBand,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    Cols=desp.cols;
    Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    AutoPtr<FixBuffer>buffer2=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer2->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pDatar=(float*)buffer2->GetBuffer();
    AutoPtr<FixBuffer>buffer3=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer3->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pOldData=(float*)buffer3->GetBuffer();
    CallBackHelper::SendMessage(callBack,"装载数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(fillBand));
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    float nodata=-32768;
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    AutoPtr<RasterBandTool>pTool2(new RasterBandTool(demBand));
    pTool2->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pOldData,psp.get(),nodata,false);
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0) pData[Pos]=nodata;
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    CallBackHelper::SendMessage(callBack,"填充局部洼地");
    LONG pos=0;
    CallBackHelper::SendMessage(callBack,"初始化");
    for(int i=0;i<Rows;i++)
    {
        for(int j=0;j<Cols;j++)
        {
            pDatar[pos++]=this->nodata;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    CallBackHelper::SendMessage(callBack,"计算水流方向");
    LONG Pos=0;
    FourValueArray fourArray(Cols*Rows);
    for(int i=0;i<Rows;i++)
    {
        for(int j=0;j<Cols;j++)
        {
            float dem=pData[Pos];
            if(nodata==dem)
            {
                Pos++;
                continue;
            }
            if(this->nodata!=pDatar[Pos])
            {
                Pos++;
                continue;
            }
            float dir=GetDirection(pData,j,i);
            if(dir<0)
            {
                if(!ComputeFlatDir2(j,i,fourArray,pData,pDatar,pOldData))
                    pDatar[Pos]=-dir;
            }
            else
                pDatar[Pos]=dir;
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    RasterDataType dType=rdtByte;
    if((this->nodata<0)||(this->nodata>255)) dType=rdtInt16;
    AutoPtr<RasterDataset>pDataset=target->CreateRasterDataset(pEnvi.get(),dType,1,this->nodata);
    if(pDataset==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建栅格数据失败");
        return nullptr;
    }
    AutoPtr<RasterBand>tb=pDataset->GetRasterBand(0);
    tb->SaveBlockData(0,0,Cols,Rows,pDatar);
    CallBackHelper::SendMessage(callBack,"水流方向计算完成");
    tb->FlushCache();
    return tb;
}

HydroFlowAccumulation::HydroFlowAccumulation(){

}

HydroFlowAccumulation::~HydroFlowAccumulation(){

}

string HydroFlowAccumulation::GetTypeName(){
    return "HydroFlowAccumulation";
}

AutoPtr<RasterBand>HydroFlowAccumulation::Execute(RasterBand*fldBand,RasterBand*weightBand,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    BYTE iCode[256];
    for(int k=0;k<256;k++){
        iCode[k]=8;
    }
    iCode[1]=3;
    iCode[2]=4;
    iCode[4]=5;
    iCode[8]=6;
    iCode[16]=7;
    iCode[32]=0;
    iCode[64]=1;
    iCode[128]=2;
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    LONG Cols=desp.cols;
    LONG Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    AutoPtr<FixBuffer>buffer2=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer2->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pDataw=(float*)buffer2->GetBuffer();
    AutoPtr<FixBuffer>buffer3=new FixBuffer(Cols*Rows);
    if(buffer3->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    BYTE*sixtArray=(BYTE*)buffer3->GetBuffer();
    CallBackHelper::SendMessage(callBack,"装载数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(fldBand));
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    if(weightBand!=nullptr){
        AutoPtr<RasterBandTool>pTool2(new RasterBandTool(weightBand));
        pTool2->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pDataw,psp.get(),nodata,false);
    }
    else{
        for(int k=Cols*Rows-1;k>=0;k--) pDataw[k]=1;
    }
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0){
                    pData[Pos]=nodata;
                    pDataw[Pos]=nodata;
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    CallBackHelper::SendMessage(callBack,"初始化水流方向");
    long regionxnum,regionynum;
    regionxnum=Cols/25;
    regionynum=Rows/25;
    if(regionxnum*25<Cols) regionxnum++;
    if(regionynum*25<Rows) regionynum++;
    if(regionxnum<1) regionxnum=1;
    if(regionynum<1) regionynum=1;
    int difx,dify;
    difx=25;
    dify=25;
    long allnum=regionxnum*regionynum;
    long*ptnum=new long[allnum];
    for(int k=allnum-1;k>=0;k--) ptnum[k]=0;
    int tempcurrentx,tempcurrenty;
    for(int i=0;i<Rows;i++)
    {
        LONG Pos=i*Cols;
        tempcurrenty=i/dify;
        if(tempcurrenty>=regionynum) tempcurrenty=regionynum-1;
        for(int j=0;j<Cols;j++)
        {
            int fV=pData[Pos];
            if((fV==nodata)||(pDataw[Pos]==nodata))
            {
                pData[Pos]=nodata;
                pDataw[Pos]=nodata;
                Pos++;
                continue;
            }
            pData[Pos]=0;
            if((fV<=0)||(fV>128))
            {
                sixtArray[Pos]=8;
                Pos++;
                continue;
            }
            sixtArray[Pos]=iCode[fV];
            int fld=sixtArray[Pos];
            float w=pDataw[Pos];
            if((w==nodata)||(w==0)||(fld==8))
            {
                Pos++;
                continue;
            }
            tempcurrentx=j/difx;
            if(tempcurrentx>=regionxnum) tempcurrentx=regionxnum-1;
            ptnum[regionxnum*tempcurrenty+tempcurrentx]++;
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            delete []ptnum;
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    //通过上述循环，pData用于存储流水累积量；pDataw为权重栅格
    CallBackHelper::SendMessage(callBack,"汇流计算");
    //现在开始汇流计算
    bool needContinue=true;
    LONG Pos=0;
    LONG oPos;
    int times=0;
    long currentx,currenty;
    long fromi,fromj,toj,toi;
    int tempPosi;
    int maxTimes=max(Cols,Rows);
    while(needContinue)
    {
        needContinue=false;
        int fld;
        float w=0;
        bool IsValid;
        for(currenty=0;currenty<regionynum;currenty++)
        {
            fromi=currenty*dify;
            if(currenty<regionynum-1)
                toi=fromi+dify;
            else
                toi=Rows;
            int currentindex=currenty*regionxnum;
            for(currentx=0;currentx<regionxnum;currentx++)
            {
                if(ptnum[currentindex]==0)
                {
                    currentindex++;
                    continue;
                }
                fromj=currentx*difx;
                if(currentx<regionxnum-1)
                    toj=fromj+difx;
                else
                    toj=Cols;
                for(int i=fromi;i<toi;i++)
                {
                    Pos=fromj+i*Cols;
                    for(int j=fromj;j<toj;j++)
                    {
                        fld=sixtArray[Pos];
                        w=pDataw[Pos];
                        if((w==nodata)||(w==0)||(fld==8))
                        {
                            Pos++;
                            continue;
                        }
                        int ni=aroundy8[fld];
                        int nj=aroundx8[fld];
                        IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                        oPos=Pos+ni*Cols+nj;
                        if(IsValid) IsValid=(nodata!=(LONG)pDataw[oPos]);
                        if(IsValid)
                        {
                            if(pDataw[oPos]==0)
                            {
                                if((ni+i<fromi)||(ni+i>=toi)||(nj+j<fromj)||(nj+j>=toj))
                                {
                                    tempPosi=currentindex;
                                    if(ni+i<fromi)
                                        tempPosi-=regionxnum;
                                    else if(ni+i>=toi)
                                        tempPosi+=regionxnum;
                                    if(nj+j<fromj)
                                        tempPosi--;
                                    else if(nj+j>=toj)
                                        tempPosi++;
                                    ptnum[tempPosi]++;
                                }
                                else
                                    ptnum[currentindex]++;
                            }
                            pDataw[oPos]+=w;
                            pDataw[Pos]=0;
                            pData[oPos]+=w;
                            needContinue=true;
                            ptnum[currentindex]--;
                        }
                        else
                        {
                            pDataw[Pos]=0;
                            ptnum[currentindex]--;
                        }
                        Pos++;
                    }
                }
                currentindex++;
            }
            if(DetectExitSignal(callBack,signal)){
                delete []ptnum;
                return nullptr;
            }
        }
        for(currenty=0;currenty<regionynum;currenty++)
        {
            fromi=currenty*dify;
            if(currenty<regionynum-1)
                toi=fromi+dify;
            else
                toi=Rows;
            int currentindex=currenty*regionxnum+regionxnum-1;
            for(currentx=regionxnum-1;currentx>=0;currentx--)
            {
                if(ptnum[currentindex]==0)
                {
                    currentindex--;
                    continue;
                }
                fromj=currentx*difx;
                if(currentx<regionxnum-1)
                    toj=fromj+difx;
                else
                    toj=Cols;
                for(int i=fromi;i<toi;i++)
                {
                    Pos=toj-1+i*Cols;
                    for(int j=toj-1;j>=fromj;j--)
                    {
                        fld=sixtArray[Pos];
                        w=pDataw[Pos];
                        if((w==nodata)||(w==0)||(fld==8))
                        {
                            Pos--;
                            continue;
                        }
                        int ni=aroundy8[fld];
                        int nj=aroundx8[fld];
                        IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                        oPos=Pos+ni*Cols+nj;
                        if(IsValid) IsValid=(nodata!=(LONG)pDataw[oPos]);
                        if(IsValid)
                        {
                            if(pDataw[oPos]==0)
                            {
                                if((ni+i<fromi)||(ni+i>=toi)||(nj+j<fromj)||(nj+j>=toj))
                                {
                                    tempPosi=currentindex;
                                    if(ni+i<fromi)
                                        tempPosi-=regionxnum;
                                    else if(ni+i>=toi)
                                        tempPosi+=regionxnum;
                                    if(nj+j<fromj)
                                        tempPosi--;
                                    else if(nj+j>=toj)
                                        tempPosi++;
                                    ptnum[tempPosi]++;
                                }
                                else
                                    ptnum[currentindex]++;
                            }
                            pDataw[oPos]+=w;
                            pDataw[Pos]=0;
                            pData[oPos]+=w;
                            needContinue=true;
                            ptnum[currentindex]--;
                        }
                        else
                        {
                            pDataw[Pos]=0;
                            ptnum[currentindex]--;
                        }
                        Pos--;
                    }
                }
                currentindex--;
            }
            if(DetectExitSignal(callBack,signal)){
                delete []ptnum;
                return nullptr;
            }
        }
        for(currenty=regionynum-1;currenty>=0;currenty--)
        {
            fromi=currenty*dify;
            if(currenty<regionynum-1)
                toi=fromi+dify;
            else
                toi=Rows;
            int currentindex=currenty*regionxnum;
            for(currentx=0;currentx<regionxnum;currentx++)
            {
                if(ptnum[currentindex]==0)
                {
                    currentindex++;
                    continue;
                }
                fromj=currentx*difx;
                if(currentx<regionxnum-1)
                    toj=fromj+difx;
                else
                    toj=Cols;
                for(int i=toi-1;i>=fromi;i--)
                {
                    Pos=fromj+i*Cols;
                    for(int j=fromj;j<toj;j++)
                    {
                        fld=sixtArray[Pos];
                        w=pDataw[Pos];
                        if((w==nodata)||(w==0)||(fld==8))
                        {
                            Pos++;
                            continue;
                        }
                        int ni=aroundy8[fld];
                        int nj=aroundx8[fld];
                        IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                        oPos=Pos+ni*Cols+nj;
                        if(IsValid) IsValid=(nodata!=(LONG)pDataw[oPos]);
                        if(IsValid)
                        {
                            if(pDataw[oPos]==0)
                            {
                                if((ni+i<fromi)||(ni+i>=toi)||(nj+j<fromj)||(nj+j>=toj))
                                {
                                    tempPosi=currentindex;
                                    if(ni+i<fromi)
                                        tempPosi-=regionxnum;
                                    else if(ni+i>=toi)
                                        tempPosi+=regionxnum;
                                    if(nj+j<fromj)
                                        tempPosi--;
                                    else if(nj+j>=toj)
                                        tempPosi++;
                                    ptnum[tempPosi]++;
                                }
                                else
                                    ptnum[currentindex]++;
                            }
                            pDataw[oPos]+=w;
                            pDataw[Pos]=0;
                            pData[oPos]+=w;
                            needContinue=true;
                            ptnum[currentindex]--;
                        }
                        else
                        {
                            pDataw[Pos]=0;
                            ptnum[currentindex]--;
                        }
                        Pos++;
                    }
                }
                currentindex++;
            }
            if(DetectExitSignal(callBack,signal)){
                delete []ptnum;
                return nullptr;
            }
        }
        for(currenty=regionynum-1;currenty>=0;currenty--)
        {
            fromi=currenty*dify;
            if(currenty<regionynum-1)
                toi=fromi+dify;
            else
                toi=Rows;
            int currentindex=currenty*regionxnum+regionxnum-1;
            for(currentx=regionxnum-1;currentx>=0;currentx--)
            {
                if(ptnum[currentindex]==0)
                {
                    currentindex--;
                    continue;
                }
                fromj=currentx*difx;
                if(currentx<regionxnum-1)
                    toj=fromj+difx;
                else
                    toj=Cols;
                for(int i=toi-1;i>=fromi;i--)
                {
                    Pos=toj-1+i*Cols;
                    for(int j=toj-1;j>=fromj;j--)
                    {
                        fld=sixtArray[Pos];
                        w=pDataw[Pos];
                        if((w==nodata)||(w==0)||(fld==8))
                        {
                            Pos--;
                            continue;
                        }
                        int ni=aroundy8[fld];
                        int nj=aroundx8[fld];
                        IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                        oPos=Pos+ni*Cols+nj;
                        if(IsValid) IsValid=(nodata!=(LONG)pDataw[oPos]);
                        if(IsValid)
                        {
                            if(pDataw[oPos]==0)
                            {
                                if((ni+i<fromi)||(ni+i>=toi)||(nj+j<fromj)||(nj+j>=toj))
                                {
                                    tempPosi=currentindex;
                                    if(ni+i<fromi)
                                        tempPosi-=regionxnum;
                                    else if(ni+i>=toi)
                                        tempPosi+=regionxnum;
                                    if(nj+j<fromj)
                                        tempPosi--;
                                    else if(nj+j>=toj)
                                        tempPosi++;
                                    ptnum[tempPosi]++;
                                }
                                else
                                    ptnum[currentindex]++;
                            }
                            pDataw[oPos]+=w;
                            pDataw[Pos]=0;
                            pData[oPos]+=w;
                            needContinue=true;
                            ptnum[currentindex]--;
                        }
                        else
                        {
                            pDataw[Pos]=0;
                            ptnum[currentindex]--;
                        }
                        Pos--;
                    }
                }
                currentindex--;
            }
            if(DetectExitSignal(callBack,signal)){
                delete []ptnum;
                return nullptr;
            }
        }
        times++;
        float perCentage=fmod(times*5.0,100);
        CallBackHelper::SendProgress(callBack,perCentage);
        if(times>maxTimes) break;
    }
    delete []ptnum;
    //汇流计算结束
    AutoPtr<RasterDataset>pDataset=target->CreateRasterDataset(pEnvi.get(),rdtFloat32,1,nodata);
    if(pDataset==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建数据集失败");
        return nullptr;
    }
    AutoPtr<RasterBand>tb=pDataset->GetRasterBand(0);
    tb->SaveBlockData(0,0,Cols,Rows,pData);
    CallBackHelper::SendMessage(callBack,"流水累积量计算完成");
    tb->FlushCache();
    return tb;
}

HydroFlowLength::HydroFlowLength(){

}

HydroFlowLength::~HydroFlowLength(){

}

string HydroFlowLength::GetTypeName(){
    return "HydroFlowLength";
}

AutoPtr<RasterBand>HydroFlowLength::FlowLengthUpstream(RasterBand*fldBand,RasterBand*weightBand,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    BYTE iCode[256];
    for(int k=0;k<256;k++)
    {
        iCode[k]=8;
    }
    iCode[1]=3;
    iCode[2]=4;
    iCode[4]=5;
    iCode[8]=6;
    iCode[16]=7;
    iCode[32]=0;
    iCode[64]=1;
    iCode[128]=2;
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    DOUBLE dCellSize=sqrt(xCellSize*xCellSize+yCellSize*yCellSize);
    LONG Cols=desp.cols;
    LONG Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    float*pDataw=nullptr;
    AutoPtr<FixBuffer>buffer2;
    if(weightBand!=nullptr){
        buffer2=new FixBuffer(Cols*Rows*sizeof(FLOAT));
        if(buffer2->GetBuffer()==nullptr)
        {
            CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
            return nullptr;
        }
        pDataw=(float*)buffer2->GetBuffer();
    }
    AutoPtr<FixBuffer>buffer3=new FixBuffer(Cols*Rows);
    if(buffer3->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    BYTE*sixtArray=(BYTE*)buffer3->GetBuffer();
    CallBackHelper::SendMessage(callBack,"装载数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(fldBand));
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    if(weightBand!=nullptr){
        AutoPtr<RasterBandTool>pTool2(new RasterBandTool(weightBand));
        pTool2->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pDataw,psp.get(),nodata,false);
    }
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0){
                    pData[Pos]=nodata;
                    if(pDataw!=nullptr) pDataw[Pos]=nodata;
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    CallBackHelper::SendMessage(callBack,"初始化水流方向");
    long regionxnum,regionynum;
    regionxnum=Cols/25;
    regionynum=Rows/25;
    if(regionxnum*25<Cols) regionxnum++;
    if(regionynum*25<Rows) regionynum++;
    if(regionxnum<1) regionxnum=1;
    if(regionynum<1) regionynum=1;
    int difx,dify;
    difx=25;
    dify=25;
    long allnum=regionxnum*regionynum;
    long*ptnum=new long[allnum];
    for(int k=allnum-1;k>=0;k--) ptnum[k]=0;
    int tempcurrentx,tempcurrenty;
    for(int i=0;i<Rows;i++)
    {
        LONG Pos=i*Cols;
        tempcurrenty=i/dify;
        if(tempcurrenty>=regionynum) tempcurrenty=regionynum-1;
        for(int j=0;j<Cols;j++)
        {
            int fV=pData[Pos];
            if(fV==nodata)
            {
                if(pDataw!=nullptr) pDataw[Pos]=nodata;
                Pos++;
                continue;
            }
            if(pDataw!=nullptr)
            {
                if(pDataw[Pos]==nodata)
                {
                    pData[Pos]=nodata;
                    Pos++;
                    continue;
                }
            }
            pData[Pos]=0;
            if((fV<=0)||(fV>128))
            {
                sixtArray[Pos]=8;
                Pos++;
                continue;
            }
            sixtArray[Pos]=iCode[fV];
            int fld=sixtArray[Pos];
            if(fld==8)
            {
                Pos++;
                continue;
            }
            tempcurrentx=j/difx;
            if(tempcurrentx>=regionxnum) tempcurrentx=regionxnum-1;
            ptnum[regionxnum*tempcurrenty+tempcurrentx]++;
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            delete []ptnum;
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    //通过上述循环，pData用于存储水流长度；pDataw为权重栅格
    CallBackHelper::SendMessage(callBack,"水流长度计算");
    //现在开始汇流计算
    bool needContinue=true;
    LONG Pos=0;
    LONG oPos=0;
    int times=0;
    long currentx,currenty;
    long fromi,fromj,toj,toi;
    int tempPosi;
    int maxTimes=max(Cols,Rows)/2;
    if(maxTimes<10) maxTimes=10;
    float ak;
    while(needContinue)
    {
        needContinue=false;
        int fld;
        float w=0;
        bool IsValid;
        for(currenty=0;currenty<regionynum;currenty++)
        {
            fromi=currenty*dify;
            if(currenty<regionynum-1)
                toi=fromi+dify;
            else
                toi=Rows;
            int currentindex=currenty*regionxnum;
            for(currentx=0;currentx<regionxnum;currentx++)
            {
                if(ptnum[currentindex]==0)
                {
                    currentindex++;
                    continue;
                }
                fromj=currentx*difx;
                if(currentx<regionxnum-1)
                    toj=fromj+difx;
                else
                    toj=Cols;
                for(int i=fromi;i<toi;i++)
                {
                    Pos=fromj+i*Cols;
                    for(int j=fromj;j<toj;j++)
                    {
                        fld=sixtArray[Pos];
                        if((pData[Pos]==nodata)||(fld==8))
                        {
                            Pos++;
                            continue;
                        }
                        int ni=aroundy8[fld];
                        int nj=aroundx8[fld];
                        IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                        oPos=Pos+ni*Cols+nj;
                        w=1;
                        if((IsValid)&&(pDataw!=NULL))
                        {
                            IsValid=((nodata!=(LONG)pDataw[Pos])&&(nodata!=(LONG)pDataw[oPos]));
                            w=(pDataw[Pos]+pDataw[oPos])/2.0;
                        }
                        if(IsValid)
                        {
                            float newlen=0;
                            if(fld%2==0)
                                newlen=pData[Pos]+dCellSize*w;
                            else if((fld==1)||(fld==5))
                                newlen=pData[Pos]+yCellSize*w;
                            else
                                newlen=pData[Pos]+xCellSize*w;
                            if(pData[oPos]<newlen)
                            {
                                pData[oPos]=newlen;
                                needContinue=true;
                                if((ni+i<fromi)||(ni+i>=toi)||(nj+j<fromj)||(nj+j>=toj))
                                {
                                    tempPosi=currentindex;
                                    if(ni+i<fromi)
                                        tempPosi-=regionxnum;
                                    else if(ni+i>=toi)
                                        tempPosi+=regionxnum;
                                    if(nj+j<fromj)
                                        tempPosi--;
                                    else if(nj+j>=toj)
                                        tempPosi++;
                                    ptnum[tempPosi]++;
                                }
                                else
                                    ptnum[currentindex]++;
                            }
                            ptnum[currentindex]--;
                        }
                        else
                        {
                            ptnum[currentindex]--;
                        }
                        Pos++;
                    }
                }
                currentindex++;
            }
            if(DetectExitSignal(callBack,signal)){
                delete []ptnum;
                return nullptr;
            }
        }
        for(currenty=0;currenty<regionynum;currenty++)
        {
            fromi=currenty*dify;
            if(currenty<regionynum-1)
                toi=fromi+dify;
            else
                toi=Rows;
            int currentindex=currenty*regionxnum+regionxnum-1;
            for(currentx=regionxnum-1;currentx>=0;currentx--)
            {
                if(ptnum[currentindex]==0)
                {
                    currentindex--;
                    continue;
                }
                fromj=currentx*difx;
                if(currentx<regionxnum-1)
                    toj=fromj+difx;
                else
                    toj=Cols;
                for(int i=fromi;i<toi;i++)
                {
                    Pos=toj-1+i*Cols;
                    for(int j=toj-1;j>=fromj;j--)
                    {
                        fld=sixtArray[Pos];
                        if((pData[Pos]==nodata)||(fld==8))
                        {
                            Pos--;
                            continue;
                        }
                        int ni=aroundy8[fld];
                        int nj=aroundx8[fld];
                        IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                        oPos=Pos+ni*Cols+nj;
                        w=1;
                        if((IsValid)&&(pDataw!=NULL))
                        {
                            IsValid=((nodata!=(LONG)pDataw[Pos])&&(nodata!=(LONG)pDataw[oPos]));
                            w=(pDataw[Pos]+pDataw[oPos])/2.0;
                        }
                        if(IsValid)
                        {
                            float newlen=0;
                            if(fld%2==0)
                                newlen=pData[Pos]+dCellSize*w;
                            else if((fld==1)||(fld==5))
                                newlen=pData[Pos]+yCellSize*w;
                            else
                                newlen=pData[Pos]+xCellSize*w;
                            if(pData[oPos]<newlen)
                            {
                                pData[oPos]=newlen;
                                needContinue=true;
                                if((ni+i<fromi)||(ni+i>=toi)||(nj+j<fromj)||(nj+j>=toj))
                                {
                                    tempPosi=currentindex;
                                    if(ni+i<fromi)
                                        tempPosi-=regionxnum;
                                    else if(ni+i>=toi)
                                        tempPosi+=regionxnum;
                                    if(nj+j<fromj)
                                        tempPosi--;
                                    else if(nj+j>=toj)
                                        tempPosi++;
                                    ptnum[tempPosi]++;
                                }
                                else
                                    ptnum[currentindex]++;
                            }
                            ptnum[currentindex]--;
                        }
                        else
                        {
                            ptnum[currentindex]--;
                        }
                        Pos--;
                    }
                }
                currentindex--;
            }
            if(DetectExitSignal(callBack,signal)){
                delete []ptnum;
                return nullptr;
            }
        }
        for(currenty=regionynum-1;currenty>=0;currenty--)
        {
            fromi=currenty*dify;
            if(currenty<regionynum-1)
                toi=fromi+dify;
            else
                toi=Rows;
            int currentindex=currenty*regionxnum;
            for(currentx=0;currentx<regionxnum;currentx++)
            {
                if(ptnum[currentindex]==0)
                {
                    currentindex++;
                    continue;
                }
                fromj=currentx*difx;
                if(currentx<regionxnum-1)
                    toj=fromj+difx;
                else
                    toj=Cols;
                for(int i=toi-1;i>=fromi;i--)
                {
                    Pos=fromj+i*Cols;
                    for(int j=fromj;j<toj;j++)
                    {
                        fld=sixtArray[Pos];
                        if((pData[Pos]==nodata)||(fld==8))
                        {
                            Pos++;
                            continue;
                        }
                        int ni=aroundy8[fld];
                        int nj=aroundx8[fld];
                        IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                        oPos=Pos+ni*Cols+nj;
                        w=1;
                        if((IsValid)&&(pDataw!=NULL))
                        {
                            IsValid=((nodata!=(LONG)pDataw[Pos])&&(nodata!=(LONG)pDataw[oPos]));
                            w=(pDataw[Pos]+pDataw[oPos])/2.0;
                        }
                        if(IsValid)
                        {
                            float newlen=0;
                            if(fld%2==0)
                                newlen=pData[Pos]+dCellSize*w;
                            else if((fld==1)||(fld==5))
                                newlen=pData[Pos]+yCellSize*w;
                            else
                                newlen=pData[Pos]+xCellSize*w;
                            if(pData[oPos]<newlen)
                            {
                                pData[oPos]=newlen;
                                needContinue=true;
                                if((ni+i<fromi)||(ni+i>=toi)||(nj+j<fromj)||(nj+j>=toj))
                                {
                                    tempPosi=currentindex;
                                    if(ni+i<fromi)
                                        tempPosi-=regionxnum;
                                    else if(ni+i>=toi)
                                        tempPosi+=regionxnum;
                                    if(nj+j<fromj)
                                        tempPosi--;
                                    else if(nj+j>=toj)
                                        tempPosi++;
                                    ptnum[tempPosi]++;
                                }
                                else
                                    ptnum[currentindex]++;
                            }
                            ptnum[currentindex]--;
                        }
                        else
                        {
                            ptnum[currentindex]--;
                        }
                        Pos++;
                    }
                }
                currentindex++;
            }
            if(DetectExitSignal(callBack,signal)){
                delete []ptnum;
                return nullptr;
            }
        }
        for(currenty=regionynum-1;currenty>=0;currenty--)
        {
            fromi=currenty*dify;
            if(currenty<regionynum-1)
                toi=fromi+dify;
            else
                toi=Rows;
            int currentindex=currenty*regionxnum+regionxnum-1;
            for(currentx=regionxnum-1;currentx>=0;currentx--)
            {
                if(ptnum[currentindex]==0)
                {
                    currentindex--;
                    continue;
                }
                fromj=currentx*difx;
                if(currentx<regionxnum-1)
                    toj=fromj+difx;
                else
                    toj=Cols;
                for(int i=toi-1;i>=fromi;i--)
                {
                    Pos=toj-1+i*Cols;
                    for(int j=toj-1;j>=fromj;j--)
                    {
                        fld=sixtArray[Pos];
                        if((pData[Pos]==nodata)||(fld==8))
                        {
                            Pos--;
                            continue;
                        }
                        int ni=aroundy8[fld];
                        int nj=aroundx8[fld];
                        IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                        oPos=Pos+ni*Cols+nj;
                        w=1;
                        if((IsValid)&&(pDataw!=NULL))
                        {
                            IsValid=((nodata!=(LONG)pDataw[Pos])&&(nodata!=(LONG)pDataw[oPos]));
                            w=(pDataw[Pos]+pDataw[oPos])/2.0;
                        }
                        if(IsValid)
                        {
                            float newlen=0;
                            if(fld%2==0)
                                newlen=pData[Pos]+dCellSize*w;
                            else if((fld==1)||(fld==5))
                                newlen=pData[Pos]+yCellSize*w;
                            else
                                newlen=pData[Pos]+xCellSize*w;
                            if(pData[oPos]<newlen)
                            {
                                pData[oPos]=newlen;
                                needContinue=true;
                                if((ni+i<fromi)||(ni+i>=toi)||(nj+j<fromj)||(nj+j>=toj))
                                {
                                    tempPosi=currentindex;
                                    if(ni+i<fromi)
                                        tempPosi-=regionxnum;
                                    else if(ni+i>=toi)
                                        tempPosi+=regionxnum;
                                    if(nj+j<fromj)
                                        tempPosi--;
                                    else if(nj+j>=toj)
                                        tempPosi++;
                                    ptnum[tempPosi]++;
                                }
                                else
                                    ptnum[currentindex]++;
                            }
                            ptnum[currentindex]--;
                        }
                        else
                        {
                            ptnum[currentindex]--;
                        }
                        Pos--;
                    }
                }
                currentindex--;
            }
            if(DetectExitSignal(callBack,signal)){
                delete []ptnum;
                return nullptr;
            }
        }
        times++;
        CallBackHelper::SendProgress(callBack,(FLOAT)times*100.0/maxTimes);
        if(times>maxTimes) break;
    }
    delete []ptnum;
    //汇流计算结束
    AutoPtr<RasterDataset>pDataset=target->CreateRasterDataset(pEnvi.get(),rdtFloat32,1,nodata);
    if(pDataset==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建数据集失败");
        return nullptr;
    }
    AutoPtr<RasterBand>tb=pDataset->GetRasterBand(0);
    tb->SaveBlockData(0,0,Cols,Rows,pData);
    CallBackHelper::SendMessage(callBack,"水流长度计算完成");
    tb->FlushCache();
    return tb;
}

AutoPtr<RasterBand>HydroFlowLength::FlowLengthDownstream(RasterBand*fldBand,RasterBand*weightBand,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    BYTE iCode[256];
    for(int k=0;k<256;k++)
    {
        iCode[k]=8;
    }
    iCode[1]=3;
    iCode[2]=4;
    iCode[4]=5;
    iCode[8]=6;
    iCode[16]=7;
    iCode[32]=0;
    iCode[64]=1;
    iCode[128]=2;
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    DOUBLE dCellSize=sqrt(xCellSize*xCellSize+yCellSize*yCellSize);
    LONG Cols=desp.cols;
    LONG Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    float*pDataw=nullptr;
    AutoPtr<FixBuffer>buffer2;
    if(weightBand!=nullptr){
        buffer2=new FixBuffer(Cols*Rows*sizeof(FLOAT));
        if(buffer2->GetBuffer()==nullptr)
        {
            CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
            return nullptr;
        }
        pDataw=(float*)buffer2->GetBuffer();
    }
    AutoPtr<FixBuffer>buffer3=new FixBuffer(Cols*Rows);
    if(buffer3->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    BYTE*sixtArray=(BYTE*)buffer3->GetBuffer();
    CallBackHelper::SendMessage(callBack,"装载数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(fldBand));
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    if(weightBand!=nullptr){
        AutoPtr<RasterBandTool>pTool2(new RasterBandTool(weightBand));
        pTool2->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pDataw,psp.get(),nodata,false);
    }
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0){
                    pData[Pos]=nodata;
                    if(pDataw!=nullptr) pDataw[Pos]=nodata;
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    CallBackHelper::SendMessage(callBack,"初始化水流方向");
    for(int i=0;i<Rows;i++)
    {
        LONG Pos=i*Cols;
        for(int j=0;j<Cols;j++)
        {
            int fV=pData[Pos];
            if(fV==nodata)
            {
                if(pDataw!=nullptr) pDataw[Pos]=nodata;
                Pos++;
                continue;
            }
            if(pDataw!=NULL)
            {
                if(pDataw[Pos]==nodata)
                {
                    pData[Pos]=nodata;
                    Pos++;
                    continue;
                }
            }
            pData[Pos]=0;
            if((fV<=0)||(fV>128))
            {
                sixtArray[Pos]=8;
                Pos++;
                continue;
            }
            sixtArray[Pos]=iCode[fV];
            int fld=sixtArray[Pos];
            if(fld==8)
            {
                Pos++;
                continue;
            }
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    CallBackHelper::SendMessage(callBack,"计算水流长度");
    //现在开始汇流计算
    bool needContinue=true;
    LONG Pos=0;
    LONG oPos=0;
    int times=0;
    int maxTimes=max(Cols,Rows)/2;
    if(maxTimes<10) maxTimes=10;
    while(needContinue)
    {
        needContinue=false;
        int fld;
        float w=0;
        bool IsValid;
        for(int i=0;i<Rows;i++)
        {
            Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                fld=sixtArray[Pos];
                if((pData[Pos]==nodata)||(fld==8))
                {
                    Pos++;
                    continue;
                }
                int ni=aroundy8[fld];
                int nj=aroundx8[fld];
                IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                oPos=Pos+ni*Cols+nj;
                w=1;
                if((IsValid)&&(pDataw!=NULL))
                {
                    IsValid=((nodata!=(LONG)pDataw[Pos])&&(nodata!=(LONG)pDataw[oPos]));
                    w=(pDataw[Pos]+pDataw[oPos])/2.0;
                }
                if(IsValid)
                {
                    float newlen=0;
                    if(fld%2==0)
                        newlen=pData[oPos]+dCellSize*w;
                    else if((fld==1)||(fld==5))
                        newlen=pData[oPos]+yCellSize*w;
                    else
                        newlen=pData[oPos]+xCellSize*w;
                    if(pData[Pos]<newlen)
                    {
                        pData[Pos]=newlen;
                        needContinue=true;
                    }
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
        }

        for(int i=0;i<Rows;i++)
        {
            Pos=Cols-1+i*Cols;
            for(int j=Cols-1;j>=0;j--)
            {
                fld=sixtArray[Pos];
                if((pData[Pos]==nodata)||(fld==8))
                {
                    Pos--;
                    continue;
                }
                int ni=aroundy8[fld];
                int nj=aroundx8[fld];
                IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                oPos=Pos+ni*Cols+nj;
                w=1;
                if((IsValid)&&(pDataw!=NULL))
                {
                    IsValid=((nodata!=(LONG)pDataw[Pos])&&(nodata!=(LONG)pDataw[oPos]));
                    w=(pDataw[Pos]+pDataw[oPos])/2.0;
                }
                if(IsValid)
                {
                    float newlen=0;
                    if(fld%2==0)
                        newlen=pData[oPos]+dCellSize*w;
                    else if((fld==1)||(fld==5))
                        newlen=pData[oPos]+yCellSize*w;
                    else
                        newlen=pData[oPos]+xCellSize*w;
                    if(pData[Pos]<newlen)
                    {
                        pData[Pos]=newlen;
                        needContinue=true;
                    }
                }
                Pos--;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
        }

        for(int i=Rows-1;i>=0;i--)
        {
            Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                fld=sixtArray[Pos];
                if(((LONG)pData[Pos]==nodata)||(fld==8))
                {
                    Pos++;
                    continue;
                }
                int ni=aroundy8[fld];
                int nj=aroundx8[fld];
                IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                oPos=Pos+ni*Cols+nj;
                w=1;
                if((IsValid)&&(pDataw!=NULL))
                {
                    IsValid=((nodata!=(LONG)pDataw[Pos])&&(nodata!=(LONG)pDataw[oPos]));
                    w=(pDataw[Pos]+pDataw[oPos])/2.0;
                }
                if(IsValid)
                {
                    float newlen=0;
                    if(fld%2==0)
                        newlen=pData[oPos]+dCellSize*w;
                    else if((fld==1)||(fld==5))
                        newlen=pData[oPos]+yCellSize*w;
                    else
                        newlen=pData[oPos]+xCellSize*w;
                    if(pData[Pos]<newlen)
                    {
                        pData[Pos]=newlen;
                        needContinue=true;
                    }
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
        }

        for(int i=Rows-1;i>=0;i--)
        {
            Pos=Cols-1+i*Cols;
            for(int j=Cols-1;j>=0;j--)
            {
                fld=sixtArray[Pos];
                if(((LONG)pData[Pos]==nodata)||(fld==8))
                {
                    Pos--;
                    continue;
                }
                int ni=aroundy8[fld];
                int nj=aroundx8[fld];
                IsValid=((ni+i>=0)&&(ni+i<Rows)&&(nj+j>=0)&&(nj+j<Cols));
                oPos=Pos+ni*Cols+nj;
                w=1;
                if((IsValid)&&(pDataw!=NULL))
                {
                    IsValid=((nodata!=(LONG)pDataw[Pos])&&(nodata!=(LONG)pDataw[oPos]));
                    w=(pDataw[Pos]+pDataw[oPos])/2.0;
                }
                if(IsValid)
                {
                    float newlen=0;
                    if(fld%2==0)
                        newlen=pData[oPos]+dCellSize*w;
                    else if((fld==1)||(fld==5))
                        newlen=pData[oPos]+yCellSize*w;
                    else
                        newlen=pData[oPos]+xCellSize*w;
                    if(pData[Pos]<newlen)
                    {
                        pData[Pos]=newlen;
                        needContinue=true;
                    }
                }
                Pos--;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
        }
        times++;
        CallBackHelper::SendProgress(callBack,(FLOAT)times*100.0/maxTimes);
        if(times>maxTimes) break;
    }
    //汇流计算结束
    AutoPtr<RasterDataset>pDataset=target->CreateRasterDataset(pEnvi.get(),rdtFloat32,1,nodata);
    if(pDataset==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建数据集失败");
        return nullptr;
    }
    AutoPtr<RasterBand>tb=pDataset->GetRasterBand(0);
    tb->SaveBlockData(0,0,Cols,Rows,pData);
    CallBackHelper::SendMessage(callBack,"水流长度计算完成");
    tb->FlushCache();
    return tb;
}

AutoPtr<RasterBand>HydroFlowLength::Execute(RasterBand*fldBand,RasterBand*weightBand,bool upsteam,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    if(upsteam){
        return FlowLengthUpstream(fldBand,weightBand,target,callBack,signal);
    }
    else{
        return FlowLengthDownstream(fldBand,weightBand,target,callBack,signal);
    }
}

PourLineRoute::PourLineRoute(){
    state=1;
}

PourLineRoute::~PourLineRoute(){

}

void PourLineRoute::Add(int x,int y){
    routes.push_back(Point2D<int>(x,y));
}
void PourLineRoute::LabelState(int state){
    this->state=state;
}

int PourLineRoute::GetState(){
    return state;
}

void PourLineRoute::CopyFrom(PourLineRoute*other){
    routes.resize(other->routes.size());
    memcpy(routes.data(),other->routes.data(),routes.size()*sizeof(int)*2);
}

int PourLineRoute::GetSize(){
    return routes.size();
}

Point2D<int>PourLineRoute::GetNode(int nIndex){
    return routes[nIndex];
}

Point2D<int>PourLineRoute::GetBegin(){
    return Point2D<int>(beginX,beginY);
}

void PourLineRoute::SetBegin(int x,int y){
    beginX=x;
    beginY=y;
}

HydroLakePouringLine::HydroLakePouringLine(){

}
HydroLakePouringLine::~HydroLakePouringLine(){

}
string HydroLakePouringLine::GetTypeName(){
    return "HydroLakePouringLine";
}

void HydroLakePouringLine::PersuitSwitch(AutoPtr<PourLineRoute>route,ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer){
    BYTE*buffer=imageLumpBlock.GetBuffer();
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    int lumpHeight=imageLumpBlock.GetLumpHeight();
    int nj,ni;
    Point2D<int>begin=route->GetBegin();
    int bX=begin.X;
    int bY=begin.Y;
    while(true){
        int uppers=0;
        int upperPos=0;
        for (int m = 0; m < 8; m++)
        {
            ni = bY + aroundy8[m];
            nj = bX + aroundx8[m];
            if ((ni < 0) || (ni >= lumpHeight) || (nj < 0) || (nj >= lumpWidth)) continue;
            if(buffer[nj+ni*lumpWidth]==1){
                upperPos=m;
                uppers++;
            }
            if(maskBuffer[nj+ni*lumpWidth]==1){
                route->Add(nj,ni);
                buffer[nj+ni*lumpWidth]=2;
                route->LabelState(2);
                return;
            }
        }
        if(uppers==1){
            ni = bY + aroundy8[upperPos];
            nj = bX + aroundx8[upperPos];
            route->Add(nj,ni);
            buffer[nj+ni*lumpWidth]=2;
            bX=nj;
            bY=ni;
        }
        else if(uppers==0){
            route->LabelState(0);
            return;
        }
        else{
            route->SetBegin(bX,bY);
            return;
        }
    }
}

vector<AutoPtr<PourLineRoute>>HydroLakePouringLine::PersuitSwitchs(AutoPtr<PourLineRoute>route,ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer,FLOAT*oldData){
    BYTE*buffer=imageLumpBlock.GetBuffer();
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    int lumpHeight=imageLumpBlock.GetLumpHeight();
    int Cols=imageLumpBlock.GetCols();
    int Rows=imageLumpBlock.GetRows();
    int nj,ni;
    Point2D<int>begin=route->GetBegin();
    int bX=begin.X;
    int bY=begin.Y;
    int upperCount=0;
    int upperX,upperY;
    for (int m = 0; m < 8; m++)
    {
        ni = bY + aroundy8[m];
        nj = bX + aroundx8[m];
        if ((ni < 0) || (ni >= lumpHeight) || (nj < 0) || (nj >= lumpWidth)) continue;
        if(buffer[nj+ni*lumpWidth]==1){
            upperCount++;
            upperX=nj;
            upperY=ni;
        }
    }
    if(upperCount==1){
        route->SetBegin(upperX,upperY);
        route->Add(upperX,upperY);
        buffer[upperX+upperY*lumpWidth]=2;
        PersuitSwitch(route,imageLumpBlock,maskBuffer);
        if(route->GetState()==0) return {};
        return {route};
    }
    vector<AutoPtr<PourLineRoute>>resultRoutes;
    for (int m = 0; m < 8; m++)
    {
        ni = bY + aroundy8[m];
        nj = bX + aroundx8[m];
        if ((ni < 0) || (ni >= lumpHeight) || (nj < 0) || (nj >= lumpWidth)) continue;
        if(buffer[nj+ni*lumpWidth]==1){
            buffer[nj+ni*lumpWidth]=2;
            AutoPtr<PourLineRoute>nrt=new PourLineRoute();
            nrt->CopyFrom(route.get());
            nrt->SetBegin(nj,ni);
            nrt->Add(nj,ni);
            buffer[nj+ni*lumpWidth]=2;
            PersuitSwitch(nrt,imageLumpBlock,maskBuffer);
            if(nrt->GetState()==0) continue;
            resultRoutes.push_back(nrt);
        }
    }
    return resultRoutes;
}

AutoPtr<PourLineRoute>HydroLakePouringLine::PersuitRoute(ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer,FLOAT*oldData,int bX,int bY){
    //int nej=bX;
    //int nei=bY;
    vector<AutoPtr<PourLineRoute>>routes;
    AutoPtr<PourLineRoute>route=new PourLineRoute();
    route->Add(bX,bY);
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    BYTE*buffer=imageLumpBlock.GetBuffer();
    buffer[bX+bY*lumpWidth]=2;
    route->SetBegin(bX,bY);
    routes.push_back(route);
    AutoPtr<PourLineRoute>rRoute=nullptr;
    while(true){
        for(int k=routes.size()-1;k>=0;k--){
            int state=routes[k]->GetState();
            if(state==0){
                routes.erase(begin(routes)+k);
            }
            else if(state==2){
                if(rRoute==nullptr)
                    rRoute=routes[k];
                else if(routes[k]->GetSize()<rRoute->GetSize())
                    rRoute=routes[k];
                routes.erase(begin(routes)+k);
            }
        }
        int minLength=0;
        int minIndex=-1;
        for(int k=routes.size()-1;k>=0;k--){
            if(minIndex==-1){
                minLength=routes[k]->GetSize();
                minIndex=k;
            }
            else if(routes[k]->GetSize()<minLength){
                minLength=routes[k]->GetSize();
                minIndex=k;
            }
        }
        if(minIndex==-1) break;
        vector<AutoPtr<PourLineRoute>>nextRoutes=PersuitSwitchs(routes[minIndex],imageLumpBlock,maskBuffer,oldData);
        routes.erase(begin(routes)+minIndex);
        for(int k=0;k<nextRoutes.size();k++){
            routes.push_back(nextRoutes[k]);
        }
    }
    return rRoute;
}

vector<AutoArray<Point2D<int>>>HydroLakePouringLine::ClearUpPond(ImageLumpBlock&imageLumpBlock2){
    int lumpWidth=imageLumpBlock2.GetLumpWidth();
    int lumpHeight=imageLumpBlock2.GetLumpHeight();
    BYTE*buffer2=imageLumpBlock2.GetBuffer();
    imageLumpBlock2.MakeForeGroundOne();
    vector<AutoArray<Point2D<int>>>dpts;
    int nPos=0;
    for(int i=0;i<lumpHeight;i++){
        for(int j=0;j<lumpWidth;j++){
            if(buffer2[nPos]!=1){
                nPos++;
                continue;
            }
            AutoArray<Point2D<int>>subpts;
            subpts.Add(Point2D<int>(j,i));
            buffer2[nPos]=2;
            for(int k=0;k<subpts.GetSize();k++){
                Point2D<int>fpt=subpts.GetItem(k);
                for (int m = 0; m < 8; m++)
                {
                    int ni = fpt.Y + aroundy8[m];
                    int nj = fpt.X + aroundx8[m];
                    if ((ni < 0) || (ni >= lumpHeight) || (nj < 0) || (nj >= lumpWidth)){
                        continue;
                    }
                    if(buffer2[nj+ni*lumpWidth]==1){
                        buffer2[nj+ni*lumpWidth]=2;
                        subpts.Add(Point2D<int>(nj,ni));
                    }
                }
            }
            dpts.push_back(subpts);
            nPos++;
        }
    }
    vector<int>widths(dpts.size());
    int maxWidth=0;
    for(int l=0;l<dpts.size();l++){
        AutoArray<Point2D<int>>&subpts=dpts[l];
        for(int p=lumpWidth*lumpHeight-1;p>=0;p--) buffer2[p]=0;
        for(int k=0;k<subpts.GetSize();k++){
            Point2D<int>pt=subpts[k];
            buffer2[pt.X+pt.Y*lumpWidth]=1;
        }
        imageLumpBlock2.ExtractBorder();
        int mv=imageLumpBlock2.GetMaxValue();
        widths[l]=mv;
        if(mv>maxWidth) maxWidth=mv;
    }
    for(int p=lumpWidth*lumpHeight-1;p>=0;p--) buffer2[p]=0;
    for(int l=dpts.size()-1;l>=0;l--){
        AutoArray<Point2D<int>>&subpts=dpts[l];
        if(widths[l]==maxWidth){
            for(int k=0;k<subpts.GetSize();k++){
                Point2D<int>pt=subpts[k];
                buffer2[pt.X+pt.Y*lumpWidth]=1;
            }
        }
        else{
            dpts.erase(begin(dpts)+l);
        }
    }
    return dpts;
}

bool HydroLakePouringLine::ComputeUpPond(ImageLumpBlock&imageLumpBlock,ImageLumpBlock&imageLumpBlock2,FLOAT*oldData,int lakeMinRadius,float&innerHeight){
    int left=imageLumpBlock.GetLeft();
    int top=imageLumpBlock.GetTop();
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    int lumpHeight=imageLumpBlock.GetLumpHeight();
    BYTE*buffer=imageLumpBlock.GetBuffer();
    BYTE*buffer2=imageLumpBlock2.GetBuffer();
    int nPos=0;
    int Cols=pEnvi->Cols();
    OrderArray<float>dems;
    for(int i=0;i<lumpHeight;i++){
        int ni=i+top;
        for(int j=0;j<lumpWidth;j++){
            int nj=j+left;
            int lPos=nj+ni*Cols;
            if(buffer[nPos]==1){
                if(oldData[lPos]==nodata){
                    nPos++;
                    continue;
                }
                dems.Add(oldData[lPos]);
            }
            nPos++;
        }
    }
    int totalNum=dems.GetSize();
    if(totalNum<2) return false;
    int fPos=0;
    int tPos=totalNum-2;
    int cPos=(tPos+fPos)/2;
    int formerPos=-1;
    float fDem=dems[0];
    while(true){
        float curDem=dems[cPos];
        nPos=0;
        for(int i=0;i<lumpHeight;i++){
            int ni=i+top;
            for(int j=0;j<lumpWidth;j++){
                int nj=j+left;
                int lPos=nj+ni*Cols;
                if(buffer[nPos]==0){
                    nPos++;
                    continue;
                }
                float dem=oldData[lPos];
                if(dem<=curDem)
                    buffer2[nPos]=1;
                else
                    buffer2[nPos]=0;
                nPos++;
            }
        }
        imageLumpBlock2.ExtractBorder();
        int maxBorder=imageLumpBlock2.GetMaxValue();
        if(maxBorder==1){
            if(maxBorder>lakeMinRadius){
                tPos=cPos;
                cPos=(tPos+fPos)/2;
            }
            else if(maxBorder<=lakeMinRadius){
                fPos=cPos;
                cPos=(tPos+fPos)/2;
                if(fDem<curDem) fDem=curDem;
            }
        }
        else{
            if(maxBorder>=lakeMinRadius){
                tPos=cPos;
                cPos=(tPos+fPos)/2;
            }
            else if(maxBorder<lakeMinRadius){
                fPos=cPos;
                cPos=(tPos+fPos)/2;
                if(fDem<curDem) fDem=curDem;
            }
        }
        if(formerPos==cPos){
            break;
        }
        formerPos=cPos;
    }
    innerHeight=fDem;
    return true;
}

void HydroLakePouringLine::DigDem(FLOAT*oldData,vector<Point2D<int>>&route,float innerHeight,int Cols,int Rows){
    int cellCount=route.size();
    vector<float>dems(cellCount);
    for(int c=0;c<cellCount;c++){
        Point2D<int>cell=route[c];
        float dem=oldData[cell.X+cell.Y*Cols];
        dems[c]=dem;
    }
    dems[0]=innerHeight;
    float beginDem=innerHeight;
    float endDem=dems[cellCount-1];
    int lPos=cellCount-1;
    if(endDem==nodata){
        for(int c=cellCount-2;c>=0;c--){
            if(dems[c]!=nodata){
                endDem=dems[c];
                lPos=c;
                break;
            }
        }
    }
    if(endDem==nodata) return;
    if(beginDem<endDem) endDem=beginDem;
    for(int c=lPos;c<cellCount;c++) dems[c]=endDem;
    for(int c=0;c<cellCount;c++){
        if(dems[c]==nodata) dems[c]=innerHeight;
        if(dems[c]<endDem) dems[c]=endDem;
    }
    float curDem=beginDem;
    for(int c=1;c<cellCount-1;c++){
        if(dems[c]>curDem)
            dems[c]=curDem;
        else if(dems[c]<curDem)
            curDem=dems[c];
    }
    for(int c=0;c<cellCount;c++){
        Point2D<int>cell=route[c];
        float dem=oldData[cell.X+cell.Y*Cols];
        if(dem==nodata) continue;
        if(oldData[cell.X+cell.Y*Cols]>dems[c])
            oldData[cell.X+cell.Y*Cols]=dems[c];
    }
}

void HydroLakePouringLine::UpdateLocalHydro(FLOAT*fillData,FLOAT*oldData,ImageLumpBlock&imageLumpBlock,BYTE*flowDir){
    int Cols=imageLumpBlock.GetCols();
    int Rows=imageLumpBlock.GetRows();
    int left=imageLumpBlock.GetLeft();
    int top=imageLumpBlock.GetTop();
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    int lumpHeight=imageLumpBlock.GetLumpHeight();
    int rl,rt,rr,rb;
    rl=left-2;
    if(rl<0) rl=0;
    rt=top-2;
    if(rt<0) rt=0;
    rr=left+lumpWidth+1;
    if(rr>=Cols) rr=Cols-1;
    rb=top+lumpHeight+1;
    if(rb>=Rows) rb=Rows-1;
    int rw=rr-rl+1;
    int rh=rb-rt+1;
    int difx=left-rl;
    int dify=top-rt;
    RasterDesp desp;
    desp.left=0;
    desp.top=0;
    desp.xCellSize=desp.yCellSize=1.0;
    desp.cols=rw;
    desp.rows=rh;
    AutoPtr<SpatialReference>psp(new SpatialReference());
    AutoPtr<MemRasterWorkspaceFactory>pFac(new MemRasterWorkspaceFactory());
    AutoPtr<MemRasterWorkspace>pWork=pFac->CreateMemRasterWorkspace();
    AutoPtr<MemRasterDataset>mds=pWork->CreateRasterDataset("Mem",desp,1,rdtFloat32,nodata,psp.get());
    AutoPtr<MemRasterBand>mBand=mds->GetRasterBand(0);
    float*dem=(float*)mBand->GetRawData()->GetValues();
    BYTE*buffer=imageLumpBlock.GetBuffer();
    int Pos;
    int pos=0;
    for(int i=rt;i<=rb;i++){
        Pos=i*Cols+rl;
        for(int j=rl;j<=rr;j++){
            dem[pos]=oldData[Pos];
            Pos++;
            pos++;
        }
    }
    AutoPtr<SAEnvironment>pEnvi(new SAEnvironment());
    pEnvi->Cols()=rw;
    pEnvi->Rows()=rh;
    pEnvi->Left()=0;
    pEnvi->Top()=0;
    pEnvi->XCellSize()=1;
    pEnvi->YCellSize()=1;
    pEnvi->SetSpatialReference(psp);
    AutoPtr<HydroDemFill>fill(new HydroDemFill());
    fill->SetEnvironment(pEnvi);
    AutoPtr<MemRasterTarget>target(new MemRasterTarget());
    target->Name()="Mem";
    AutoPtr<MemRasterBand>fill_dem=fill->Execute(mBand.get(),target.get());
    if(fill_dem==nullptr) return;
    AutoPtr<HydroFlowDirection>flow=new HydroFlowDirection();
    flow->SetEnvironment(pEnvi);
    AutoPtr<MemRasterBand>flow_band=flow->Execute(mBand.get(),fill_dem.get(),target.get());
    if(flow_band==nullptr) return;
    float*m_dem=(float*)fill_dem->GetRawData()->GetValues();
    BYTE*m_dir=(BYTE*)flow_band->GetRawData()->GetValues();
    pos=0;
    BYTE iCode[256];
    for(int k=0;k<256;k++)
    {
        iCode[k]=8;
    }
    iCode[1]=3;
    iCode[2]=4;
    iCode[4]=5;
    iCode[8]=6;
    iCode[16]=7;
    iCode[32]=0;
    iCode[64]=1;
    iCode[128]=2;
    int lpos=0;
    for(int i=0;i<lumpHeight;i++){
        Pos=(i+top)*Cols+left;
        pos=difx+(i+dify)*rw;
        for(int j=0;j<lumpWidth;j++){
            if(buffer[lpos]==0){
                Pos++;
                pos++;
                lpos++;
                continue;
            }
            int dire=m_dir[pos];
            if((dire<0)||(dire>255))
                flowDir[Pos]=8;
            else
                flowDir[Pos]=iCode[dire];
            Pos++;
            pos++;
            lpos++;
        }
    }
}

void HydroLakePouringLine::ExtractPouringLine(AutoArray<Point2D<int>>&pointArray,FourValueArray&fvArray,FLOAT*pData,FLOAT*oldData,BYTE*sixArray,int Cols,int Rows,float innerHeight,ImageLumpBlock&imageLumpBlock,ImageLumpBlock&imageLumpBlock2,AutoPtr<FeatureClassEdit>pEdit){
    int pointCount = pointArray.GetSize();
    if(pointCount==0) return;
    float innermindem=innerHeight;
    float fillDem=pData[pointArray[0].X+pointArray[0].Y*Cols];
    vector<Point2D<int>>pourpts;
    int maxDownStreams=max(Cols,Rows);
    //traced downstream
    int minDownStreams=15;
    for (int k = 0; k < pointCount; k++){
        Point2D<int>point = pointArray.GetItem(k);
        int i = point.Y;
        int j = point.X;
        int dir=sixArray[j+i*Cols];
        if((dir<0)||(dir>=8)) continue;
        int cci=i+aroundy8[dir];
        int ccj=j+aroundx8[dir];
        if ((cci < 0) || (cci >= Rows) || (ccj < 0) || (ccj >= Cols)){
            pourpts.push_back(Point2D<int>(j,i));
            continue;
        }
        int state=fvArray[ccj+cci*Cols];
        if(state==3){
            continue;
        }
        if(oldData[ccj+cci*Cols]==nodata){
            pourpts.push_back(Point2D<int>(j,i));
            continue;
        }
        int curX=j;
        int curY=i;
        float curd=oldData[curX+curY*Cols];
        vector<int>downs;
        vector<int>downstates;
        int downNum=0;
        bool adjacent=false;
        int curDownStreams=0;
        float formerdem=innermindem;
        while(true){
            int dir=sixArray[curX+curY*Cols];
            if((dir<0)||(dir>=8)) break;
            int ci=curY+aroundy8[dir];
            int cj=curX+aroundx8[dir];
            if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                break;
            }
            int pPos=ci*Cols + cj;
            if (fvArray[pPos] == 3){
                adjacent=true;
                break;
            }
            if (fvArray[pPos] == 1){
                break;
            }
            curd=oldData[pPos];
            curX=cj;
            curY=ci;
            downs.push_back(pPos);
            downstates.push_back(fvArray[pPos]);
            fvArray.SetValue(pPos,1);
            downNum++;
            //相对于洼地最低积水线低
            if(curd<=innermindem){
                if(curDownStreams==0) curDownStreams=1;
                if(curDownStreams>=minDownStreams){
                    if((curd<=formerdem)||(curDownStreams>=minDownStreams*2)) break;
                }
                //break;
            }
            if(downNum>maxDownStreams) break;
            if(curDownStreams>0) curDownStreams++;
            formerdem=curd;
        }
        for(int d=0;d<downNum;d++){
            fvArray.SetValue(downs[d],downstates[d]);
        }
        if(adjacent) continue;
        bool bSame=false;
        for(int l=0;l<pourpts.size();l++){
            if((pourpts[l].X==j)&&(pourpts[l].Y==i)){
                bSame=true;
                break;
            }
        }
        if(!bSame){
            pourpts.push_back(Point2D<int>(j,i));
        }
    }
    vector<vector<Point2D<int>>>pourRoutes;
    for(int p=0;p<pourpts.size();p++){
        int curX=pourpts[p].X;
        int curY=pourpts[p].Y;
        float curd=oldData[curX+curY*Cols];
        int dir=sixArray[curX+curY*Cols];
        if((dir<0)||(dir>=8)) break;
        vector<int>downs;
        vector<int>downstates;
        vector<Point2D<int>>routes;
        routes.push_back(Point2D<int>(curX,curY));
        int downNum=0;
        int curDownStreams=0;
        float formerdem=innermindem;
        while(true){
            int dir=sixArray[curX+curY*Cols];
            if((dir<0)||(dir>=8)) break;
            int ci=curY+aroundy8[dir];
            int cj=curX+aroundx8[dir];
            if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                break;
            }
            if(oldData[cj+ci*Cols]==nodata){
                break;
            }
            int pPos=ci*Cols + cj;
            if ((fvArray[pPos] == 3)||(fvArray[pPos] == 1)) break;
            curd=oldData[pPos];
            curX=cj;
            curY=ci;
            downs.push_back(pPos);
            downstates.push_back(fvArray[pPos]);
            routes.push_back(Point2D<int>(cj,ci));
            fvArray.SetValue(pPos,1);
            downNum++;
            if(curd<=innermindem){
                if(curDownStreams==0) curDownStreams=1;
                if(curDownStreams>=minDownStreams){
                    if((curd<=formerdem)||(curDownStreams>=minDownStreams*2)) break;
                }
                //break;
            }
            if(downNum>maxDownStreams) break;
            if(curDownStreams>0) curDownStreams++;
            formerdem=curd;
        }
        pourRoutes.push_back(routes);
        //if(downs.size()==0) continue;
        //float mind=oldData[downs[downs.size()-1]];
        for(int k=0;k<downs.size();k++){
            fvArray.SetValue(downs[k],downstates[k]);
        }
    }
    int routeCount=pourRoutes.size();
    vector<bool>valids(routeCount);
    for(int k=0;k<routeCount;k++){
        valids[k]=true;
    }
    for(int i=0;i<routeCount-1;i++){
        vector<Point2D<int>>&r1=pourRoutes[i];
        if(!valids[i]) continue;
        int pts1=r1.size();
        for(int j=i+1;j<routeCount;j++){
            vector<Point2D<int>>&r2=pourRoutes[j];
            if(!valids[j]) continue;
            int pts2=r2.size();
            bool bSame=false;
            for(int m=0;m<pts1;m++){
                Point2D<int>&pt1=r1[m];
                for(int n=0;n<pts2;n++){
                    Point2D<int>&pt2=r2[n];
                    if((pt1.X==pt2.X)&&(pt1.Y==pt2.Y)){
                        bSame=true;
                        break;
                    }
                }
                if(bSame) break;
            }
            if(bSame){
                if(r1.size()>=r2.size()){
                    valids[j]=false;
                }
                else{
                    valids[i]=false;
                    break;
                }
            }
        }
    }
    for(int k=routeCount-1;k>=0;k--){
        if(!valids[k]){
            pourRoutes.erase(begin(pourRoutes)+k);
            pourpts.erase(begin(pourpts)+k);
        }
    }
    AutoPtr<FeatureClass>pfc=pEdit;
    vector<Variant>values(3);
    int id=pfc->GetFeatureCount();
    RasterDesp desp=pEnvi->GetRasterDesp();
    int lumpLeft=imageLumpBlock.GetLeft();
    int lumpTop=imageLumpBlock.GetTop();
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    int lumpHeight=imageLumpBlock.GetLumpHeight();
    BYTE*maskBuffer=imageLumpBlock2.GetBuffer();
    BYTE*buffer=imageLumpBlock.GetBuffer();

    OrderArray<double>dems;
    vector<int>demCounts;
    int totalCount=0;
    int left=imageLumpBlock.GetLeft();
    int top=imageLumpBlock.GetTop();
    int nPos=0;
    for(int i=0;i<lumpHeight;i++){
        int ni=i+top;
        for(int j=0;j<lumpWidth;j++){
            int nj=j+left;
            int lPos=nj+ni*Cols;
            if(buffer[nPos]==1){
                if(oldData[lPos]==nodata){
                    nPos++;
                    continue;
                }
                totalCount++;
                int fpos=dems.Find(oldData[lPos]);
                if(fpos<0){
                    fpos=dems.Add(oldData[lPos]);
                    demCounts.insert(begin(demCounts)+fpos,1);
                }
                else{
                    demCounts[fpos]++;
                }
            }
            nPos++;
        }
    }
    vector<float>referDems;
    int curCount=0;
    float formerPer=0;
    for(int k=dems.GetSize()-1;k>=0;k--){
        if(dems[k]<=innerHeight) break;
        curCount+=demCounts[k];
        float per=curCount*1.0/totalCount;
        if((per-formerPer>=0.05)&&(k<dems.GetSize()-1)){
            formerPer=per;
            referDems.push_back(dems[k]);
        }
    }
    AutoPtr<FixBuffer>fxb=new FixBuffer(lumpWidth*lumpHeight);
    BYTE*mbs=(BYTE*)fxb->GetBuffer();
    for(int r=0;r<pourRoutes.size();r++){
        //traced upstream
        imageLumpBlock.MakeForeGroundOne();
        vector<Point2D<int>>&route=pourRoutes[r];
        vector<Point2D<int>>tails;
        tails.push_back(Point2D<int>(route[0].X,route[0].Y));
        imageLumpBlock2.CopyFrom(&imageLumpBlock);
        for(int l=0;l<referDems.size();l++){
            int nPos=0;
            for(int i=0;i<lumpHeight;i++){
                int ni=i+top;
                for(int j=0;j<lumpWidth;j++){
                    int nj=j+left;
                    int lPos=nj+ni*Cols;
                    if(buffer[nPos]==1){
                        if(oldData[lPos]==nodata){
                            mbs[nPos]=0;
                            nPos++;
                            continue;
                        }
                        if(oldData[lPos]<=referDems[l])
                            mbs[nPos]=1;
                        else
                            mbs[nPos]=0;
                    }
                    else
                        mbs[nPos]=0;
                    nPos++;
                }
            }
            mbs[route[0].X-lumpLeft+(route[0].Y-lumpTop)*lumpWidth]=1;
            imageLumpBlock2.ThinImage(mbs);
        }
        imageLumpBlock2.ThinImage(tails);
        int state=maskBuffer[route[0].X-lumpLeft+(route[0].Y-lumpTop)*lumpWidth];
        if(state!=1) continue;
        imageLumpBlock.CopyFrom(&imageLumpBlock2);
        for (int k = 0; k < pointCount; k++){
            Point2D<int>pt=pointArray.GetItem(k);
            int iX=pt.X-imageLumpBlock.GetLeft();
            int iY=pt.Y-imageLumpBlock.GetTop();
            int ni=iY+top;
            int nj=iX+left;
            int lPos=nj+ni*Cols;
            if(oldData[lPos]<=innerHeight)
                maskBuffer[iX+iY*imageLumpBlock.GetLumpWidth()]=1;
            else
                maskBuffer[iX+iY*imageLumpBlock.GetLumpWidth()]=0;
        }
        vector<AutoArray<Point2D<int>>>dpts=ClearUpPond(imageLumpBlock2);
        int bX=route[0].X-lumpLeft;
        int bY=route[0].Y-lumpTop;
        for(int d=0;d<dpts.size();d++){
            AutoArray<Point2D<int>>&subpts=dpts[d];
            for(int p=lumpWidth*lumpHeight-1;p>=0;p--) maskBuffer[p]=0;
            for(int k=0;k<subpts.GetSize();k++){
                Point2D<int>pt=subpts[k];
                maskBuffer[pt.X+pt.Y*lumpWidth]=1;
            }
            AutoPtr<PourLineRoute>upperRoute=PersuitRoute(imageLumpBlock,maskBuffer,oldData,bX,bY);
            if(upperRoute!=nullptr){
                vector<Point2D<int>>nroutes(upperRoute->GetSize()+route.size()-1);
                AutoPtr<Polyline>poly=new Polyline();
                AutoPtr<Points>dpts(new Points());
                int pIndex=0;
                for(int n=upperRoute->GetSize()-1;n>=1;n--){
                    Point2D<int>pt=upperRoute->GetNode(n);
                    pt.X+=lumpLeft;
                    pt.Y+=lumpTop;
                    nroutes[pIndex++]=pt;
                    double X=pt.X*desp.xCellSize+desp.left+desp.xCellSize/2;
                    double Y=desp.top-pt.Y*desp.yCellSize-desp.yCellSize/2;
                    dpts->Add(X,Y);
                }
                for(int l=0;l<route.size();l++){
                    Point2D<int>&pt=route[l];
                    nroutes[pIndex++]=pt;
                    double X=pt.X*desp.xCellSize+desp.left+desp.xCellSize/2;
                    double Y=desp.top-pt.Y*desp.yCellSize-desp.yCellSize/2;
                    dpts->Add(X,Y);
                }
                poly->Add(dpts);
                DigDem(oldData,nroutes,innerHeight,Cols,Rows);
                values[0]=id++;
                values[1]=fillDem;
                values[2]=innerHeight;
                pEdit->AddFeatureEx(poly,values);
            }
            for(int p=lumpWidth*lumpHeight-1;p>=0;p--){
                if(buffer[p]>0) buffer[p]=1;
            }
        }
    }
    for (int k = 0; k < pointCount; k++){
        Point2D<int>point = pointArray.GetItem(k);
        fvArray.SetValue(point.X+point.Y*Cols, 2);
    }
    imageLumpBlock.UpdateImageBlock(pointArray);
    UpdateLocalHydro(pData,oldData,imageLumpBlock,sixArray);
}

AutoPtr<FeatureClass>HydroLakePouringLine::Execute(RasterBand*demBand,RasterBand*fillBand,RasterBand*fldBand,RasterBand*accuBand,int lakeMinRadius,FeatureClassTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    LONG Cols=desp.cols;
    LONG Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    AutoPtr<FixBuffer>buffer2=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer2->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*oldData=(float*)buffer2->GetBuffer();
    AutoPtr<FixBuffer>buffer3=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer3->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*accuData=(float*)buffer3->GetBuffer();
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    AutoPtr<RasterBandTool>pfTool(new RasterBandTool(fldBand));
    pfTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    CallBackHelper::SendMessage(callBack,"初始化水流方向");
    AutoPtr<FixBuffer>buffer4=new FixBuffer(Cols*Rows);
    if(buffer4->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    BYTE iCode[256];
    for(int k=0;k<256;k++)
    {
        iCode[k]=8;
    }
    iCode[1]=3;
    iCode[2]=4;
    iCode[4]=5;
    iCode[8]=6;
    iCode[16]=7;
    iCode[32]=0;
    iCode[64]=1;
    iCode[128]=2;
    BYTE*sixtArray=(BYTE*)buffer4->GetBuffer();
    for(int i=0;i<Rows;i++){
        LONG Pos=i*Cols;
        for(int j=0;j<Cols;j++){
            if(pData[Pos]==nodata){
                sixtArray[Pos]=8;
                Pos++;
                continue;
            }
            int dire=pData[Pos];
            if((dire<0)||(dire>255))
                sixtArray[Pos]=8;
            else
                sixtArray[Pos]=iCode[dire];
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    CallBackHelper::SendMessage(callBack,"装载地形数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(demBand));
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,oldData,psp.get(),nodata,false);
    AutoPtr<RasterBandTool>pTool2(new RasterBandTool(fillBand));
    pTool2->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    AutoPtr<RasterBandTool>pTool3(new RasterBandTool(accuBand));
    pTool3->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,accuData,psp.get(),nodata,false);
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0){
                    pData[Pos]=nodata;
                    oldData[Pos]=nodata;
                    accuData[Pos]=nodata;
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    AutoArray<Point2D<int>>pointArray;
    CallBackHelper::SendMessage(callBack,"提取洼地");
    FourValueArray fvArray(Cols*Rows);
    fvArray.SetDefaultValue(0);
    ImageLumpBlock imageLumpBlock;
    imageLumpBlock.CreateImageBlock(Cols,Rows);
    ImageLumpBlock imageLumpBlock2;
    imageLumpBlock2.CreateImageBlock(Cols,Rows);
    OrderArray<int>accuOrders(true);
    vector<Point2D<int>>sinkPoses;
    for (int i = 1; i < Rows - 1; i++){
        LONG Pos = i * Cols + 1;
        for (int j = 1; j < Cols - 1; j++)
        {
            float cV = pData[Pos];
            if (nodata == (LONG)cV)
            {
                Pos++;
                continue;
            }
            if(fvArray[Pos]!=0){
                Pos++;
                continue;
            }
            pointArray.Clear();
            pointArray.Add(Point2D<int>(j,i));
            fvArray.SetValue(Pos,2);
            for(int k=0;k<pointArray.GetSize();k++){
                Point2D<int>cp=pointArray[k];
                int ni=cp.Y;
                int nj=cp.X;
                for (int m = 0; m < 8; m++){
                    int ci = ni + aroundy8[m];
                    int cj = nj + aroundx8[m];
                    if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                        continue;
                    }
                    int nPos=ci*Cols + cj;
                    float nV = pData[nPos];
                    if (nodata == nV){
                        continue;
                    }
                    if(fvArray[nPos]!=0) continue;
                    if(nV==cV){
                        pointArray.Add(Point2D<int>(cj,ci));
                        fvArray.SetValue(nPos,2);
                    }
                }
            }
            if(pointArray.GetSize()<4){
                Pos++;
                continue;
            }
            imageLumpBlock.UpdateImageBlock(pointArray);
            imageLumpBlock.ExtractBorder();
            if(imageLumpBlock.GetMaxValue()<lakeMinRadius){
                Pos++;
                continue;
            }
            float maxaccu=accuData[pointArray[0].X+pointArray[0].Y*Cols];
            for(int l=pointArray.GetSize()-1;l>=1;l--){
                Point2D<int>cp=pointArray[l];
                float accu=accuData[cp.X+cp.Y*Cols];
                if(accu>maxaccu) maxaccu=accu;
            }
            int nPos=accuOrders.Add(maxaccu);
            sinkPoses.insert(begin(sinkPoses)+nPos,Point2D<int>(j,i));
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    AutoPtr<TableDescCreator>pCreator(new TableDescCreator());
    pCreator->AddField(FieldDesp("ID",vftInteger,10,0));
    pCreator->AddField(FieldDesp("LakeHeight",vftReal,12,4));
    pCreator->AddField(FieldDesp("DigHeight",vftReal,12,4));
    AutoPtr<TableDesc>desc=pCreator->CreateTableDesc();
    AutoPtr<FeatureClass>pfc=target->CreateFeatureClass(pEnvi->GetSpatialReference().get(),vstPolyline,desc.get());
    if(pfc==nullptr)
    {
        CallBackHelper::SendMessage(callBack,"创建洼地倾泻线失败");
        return nullptr;
    }
    AutoPtr<FeatureClassEdit>pEdit(pfc);
    CallBackHelper::SendMessage(callBack,"洼地倾泻线提取");
    int sinkNum=sinkPoses.size();
    for(int k=sinkNum-1;k>=0;k--){
        Point2D<int>fromPos=sinkPoses[k];
        int Pos=fromPos.X+fromPos.Y*Cols;
        float cV = pData[Pos];
        if (nodata == (LONG)cV) continue;
        if(fvArray[Pos]!=2) continue;
        pointArray.Clear();
        pointArray.Add(Point2D<int>(fromPos.X,fromPos.Y));
        fvArray.SetValue(Pos,3);
        for(int k=0;k<pointArray.GetSize();k++){
            Point2D<int>cp=pointArray[k];
            int ni=cp.Y;
            int nj=cp.X;
            for (int m = 0; m < 8; m++){
                int ci = ni + aroundy8[m];
                int cj = nj + aroundx8[m];
                if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                    continue;
                }
                int nPos=ci*Cols + cj;
                float nV = pData[nPos];
                if (nodata == nV){
                    continue;
                }
                if(fvArray[nPos]!=2) continue;
                if(nV==cV){
                    pointArray.Add(Point2D<int>(cj,ci));
                    fvArray.SetValue(nPos,3);
                }
            }
        }
        imageLumpBlock.UpdateImageBlock(pointArray);
        imageLumpBlock2.UpdateImageBlock(pointArray);
        float innerHeight;
        if(!ComputeUpPond(imageLumpBlock,imageLumpBlock2,oldData,2,innerHeight)){
            for (int k = 0; k < pointArray.GetSize(); k++){
                Point2D<int>point = pointArray.GetItem(k);
                fvArray.SetValue(point.X+point.Y*Cols, 2);
            }
            continue;
        }
        ExtractPouringLine(pointArray,fvArray,pData,oldData,sixtArray,Cols,Rows,innerHeight,imageLumpBlock,imageLumpBlock2,pEdit);
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)((sinkNum-k)*100.0/sinkNum));
    }
    CallBackHelper::SendMessage(callBack,"洼地倾泻线提取完成");
    pfc->FlushCache();
    return pfc;
}

HydroDig::HydroDig(){

}
HydroDig::~HydroDig(){

}
string HydroDig::GetTypeName(){
    return "HydroDig";
}

void HydroDig::BreachingHeading(int spillX,int spillY,int Cols,int Rows,FLOAT*oldData,BYTE*flowDir){
    int nPos=spillX+spillY*Cols;
    float centerDem=oldData[nPos];
    float upperDem=centerDem;
    int validNum=0;
    int ii,jj,ccj,cci,dir;
    int nnPos;
    for(int m=0;m<8;m++){
        ii=spillY+aroundy8[m];
        jj=spillX+aroundx8[m];
        if ((ii < 0) || (ii >= Rows) || (jj < 0) || (jj >= Cols)) continue;
        nnPos=jj+ii*Cols;
        if(oldData[nnPos]==nodata) continue;
        dir=flowDir[nnPos];
        if((dir<0)||(dir>=8)) continue;
        cci=ii+aroundy8[dir];
        ccj=jj+aroundx8[dir];
        if((cci!=spillY)||(ccj!=spillX)) continue;
        if(validNum==0){
            upperDem=oldData[nnPos];
        }
        else if(oldData[nnPos]<upperDem)
            upperDem=oldData[nnPos];
        validNum++;
    }
    if(validNum==0) return;
    if(centerDem>upperDem)
        oldData[nPos]=upperDem;
    else
        upperDem=centerDem;
    /*
    dir=flowDir[spillX+spillY*Cols];
    if((dir<0)||(dir>=8)) return;
    cci=spillY+aroundy8[dir];
    ccj=spillX+aroundx8[dir];
    if ((cci < 0) || (cci >= Rows) || (ccj < 0) || (ccj >= Cols)) return;
    nnPos=ccj+cci*Cols;
    if(oldData[nnPos]==nodata) return;
    float nextDem=oldData[nnPos];
    if(nextDem>upperDem) oldData[nnPos]=upperDem;
    */
}

void HydroDig::BreachingSink(AutoArray<Point2D<int>>&pointArray,FLOAT*oldData,BYTE*flowDir,ImageLumpBlock&imageLumpBlock){
    vector<Point2D<int>>pourpts;
    int pointCount = pointArray.GetSize();
    if(pointCount==0) return;
    int Cols=imageLumpBlock.GetCols();
    int Rows=imageLumpBlock.GetRows();
    int left=imageLumpBlock.GetLeft();
    int top=imageLumpBlock.GetTop();
    BYTE*buffer=imageLumpBlock.GetBuffer();
    int lw=imageLumpBlock.GetLumpWidth();
    int lh=imageLumpBlock.GetLumpHeight();
    for (int k = 0; k < pointCount; k++){
        Point2D<int>point = pointArray.GetItem(k);
        int i = point.Y;
        int j = point.X;
        int dir=flowDir[j+i*Cols];
        if((dir<0)||(dir>=8)) continue;
        int cci=i+aroundy8[dir];
        int ccj=j+aroundx8[dir];
        if ((cci < 0) || (cci >= Rows) || (ccj < 0) || (ccj >= Cols)){
            pourpts.push_back(Point2D<int>(j,i));
            continue;
        }
        if(oldData[ccj+cci*Cols]==nodata){
            pourpts.push_back(Point2D<int>(j,i));
            continue;
        }
        int ci=cci-top;
        int cj=ccj-left;
        if ((ci < 0) || (ci >= lh) || (cj < 0) || (cj >= lw)){
            pourpts.push_back(Point2D<int>(j,i));
            continue;
        }
        int state=buffer[cj+ci*lw];
        if(state==1) continue;
        pourpts.push_back(Point2D<int>(j,i));
    }
    for(int k=0;k<pourpts.size();k++){
        Point2D<int>&pt=pourpts[k];
        BreachingHeading(pt.X,pt.Y,Cols,Rows,oldData,flowDir);
    }
}

void HydroDig::DigFeature(Polyline*polyline,float*pData,float*resultData,float digh){
    RasterDesp desp=pEnvi->GetRasterDesp();
    if(polyline->GetSize()==0) return;
    AutoPtr<Path>pPath=polyline->GetItem(0);
    AutoPtr<Points>oldPoints=pPath;
    int oldSize=oldPoints->GetSize();
    if(oldSize==0) return;
    double cellSize=desp.xCellSize;
    if(cellSize>desp.yCellSize) cellSize=desp.yCellSize;
    AutoPtr<Points>dpts=pPath->ReturnPoints(0,cellSize);
    int ptsSize=dpts->GetSize();
    if(ptsSize==0) return;
    AutoArray<Point2D<int>>cells;
    double X,Y;
    int iX,iY;
    int fX,fY;
    dpts->GetItem(0,X,Y);
    iX=(X-desp.left)/desp.xCellSize;
    iY=(desp.top-Y)/desp.yCellSize;
    fX=iX;
    fY=iY;
    cells.Add(Point2D<int>(iX,iY));
    for(int k=0;k<ptsSize;k++){
        dpts->GetItem(k,X,Y);
        iX=(X-desp.left)/desp.xCellSize;
        iY=(desp.top-Y)/desp.yCellSize;
        if((iX==fX)&&(iY==fY)) continue;
        cells.Add(Point2D<int>(iX,iY));
        fX=iX;
        fY=iY;
    }
    oldPoints->GetItem(oldSize-1,X,Y);
    iX=(X-desp.left)/desp.xCellSize;
    iY=(desp.top-Y)/desp.yCellSize;
    if((iX!=fX)||(iY!=fY)){
        cells.Add(Point2D<int>(iX,iY));
    }
    int cellCount=cells.GetSize();
    vector<float>dems(cellCount);
    bool bInitial=false;
    for(int c=0;c<cellCount;c++){
        Point2D<int>cell=cells[c];
        float dem=pData[cell.X+cell.Y*desp.cols];
        dems[c]=dem;
    }
    dems[0]=digh;


    //vector<float>odelms(cellCount);
    //for(int c=0;c<cellCount;c++){
        //odelms[c]=dems[c];
    //}


    float beginDem=digh;
    float endDem=dems[cellCount-1];
    int lPos=cellCount-1;
    if(endDem==nodata){
        for(int c=cellCount-2;c>=0;c--){
            if(dems[c]!=nodata){
                endDem=dems[c];
                lPos=c;
                break;
            }
        }
    }
    if(endDem==nodata) return;
    if(beginDem<endDem) endDem=beginDem;
    for(int c=lPos;c<cellCount;c++) dems[c]=endDem;
    for(int c=0;c<cellCount;c++){
        if(dems[c]==nodata) dems[c]=digh;
        if(dems[c]<endDem) dems[c]=endDem;
    }
    float curDem=beginDem;
    for(int c=1;c<cellCount-1;c++){
        if(dems[c]>curDem)
            dems[c]=curDem;
        else if(dems[c]<curDem)
            curDem=dems[c];
    }

    /*
    vector<DatabaseField>dfs;
    dfs.push_back(DatabaseField("DIS",DatabaseFieldType::ftNumeric));
    dfs.push_back(DatabaseField("OldDEM",DatabaseFieldType::ftNumeric));
    dfs.push_back(DatabaseField("NewDEM",DatabaseFieldType::ftNumeric));
    AutoPtr<DataTable>dt=new DataTable("DT",dfs);
    vector<Variant>values(3);
    double dist=0;
    for(int l=0;l<dems.size();l++){
        if(l>0) dist+=sqrt(cells[l].DistanceTo(cells[l-1]));
        values[0]=dist;
        values[1]=odelms[l];
        values[2]=dems[l];
        dt->AddRecord(values);
    }
    dt->SaveToCsv("D:/work/curve.csv");
    */



    for(int c=0;c<cellCount;c++){
        Point2D<int>cell=cells[c];
        float dem=pData[cell.X+cell.Y*desp.cols];
        if(dem==nodata) continue;
        if(resultData[cell.X+cell.Y*desp.cols]>dems[c])
            resultData[cell.X+cell.Y*desp.cols]=dems[c];
    }
}

AutoPtr<RasterBand>HydroDig::Execute(RasterBand*demBand,FeatureClass*pouringLine,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    if(pouringLine->GetShapeType()!=vstPolyline){
        CallBackHelper::SendMessage(callBack,"洼地倾泻线必须为矢量线图层");
        return nullptr;
    }
    int digField=pouringLine->GetTableDesc()->FindField("DigHeight");
    if(digField==-1){
        CallBackHelper::SendMessage(callBack,"洼地倾泻线缺少DigHeight字段");
        return nullptr;
    }
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    LONG Cols=desp.cols;
    LONG Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    AutoPtr<FixBuffer>buffer2=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer2->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*resultData=(float*)buffer2->GetBuffer();
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    CallBackHelper::SendMessage(callBack,"装载地形数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(demBand));
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                resultData[Pos]=pData[Pos];
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0){
                    pData[Pos]=nodata;
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }else{
        CallBackHelper::SendMessage(callBack,"初始化输出");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                resultData[Pos]=pData[Pos];
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    CallBackHelper::SendMessage(callBack,"依据倾泻线下挖地形");
    AutoPtr<RasterDataset>resultds=target->CreateRasterDataset(pEnvi.get(),demBand->GetDataType(),1,nodata);
    if(resultds==nullptr){
        CallBackHelper::SendMessage(callBack,"创建修正地形图层失败");
        return nullptr;
    }
    AutoPtr<RasterBand>resultBand=resultds->GetRasterBand(0);
    int featureCount=pouringLine->GetFeatureCount();
    AutoPtr<SelectionSet>selSet=pouringLine->GetSelectionSet();
    int selCount=selSet->GetSize();
    AutoPtr<SpatialReference>psp2=pouringLine->GetSpatialReference();
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation(psp2,psp));
    pTrans->BeginTransform();
    for(int k=0;k<featureCount;k++){
        if(selCount>0){
            int fid=pouringLine->GetFeatureID(k);
            if(!selSet->FindFeatureID(fid)) continue;
        }
        AutoPtr<Polyline>polyline=pouringLine->GetFeature(k);
        if(polyline==nullptr) continue;
        float digh=pouringLine->GetFieldValueAsDouble(k,digField);
        polyline->Project(pTrans.get());
        DigFeature(polyline.get(),pData,resultData,digh);
        if(k%100==0){
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)k/featureCount*100.0);
        }
    }
    CallBackHelper::SendMessage(callBack,"依据倾泻线下挖地形完成");
    resultBand->SaveBlockData(0,0,Cols,Rows,resultData);
    resultBand->FlushCache();
    return resultBand;
}


AutoPtr<RasterBand>HydroDig::Execute(RasterBand*demBand,RasterBand*fillBand,RasterBand*fldBand,FeatureClass*pouringLine,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    if(pouringLine->GetShapeType()!=vstPolyline){
        CallBackHelper::SendMessage(callBack,"洼地倾泻线必须为矢量线图层");
        return nullptr;
    }
    int digField=pouringLine->GetTableDesc()->FindField("DigHeight");
    if(digField==-1){
        CallBackHelper::SendMessage(callBack,"洼地倾泻线缺少DigHeight字段");
        return nullptr;
    }
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    LONG Cols=desp.cols;
    LONG Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*oldData=(float*)buffer->GetBuffer();
    AutoPtr<FixBuffer>buffer3=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer3->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer3->GetBuffer();
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    AutoPtr<RasterBandTool>pfTool(new RasterBandTool(fldBand));
    pfTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    CallBackHelper::SendMessage(callBack,"初始化水流方向");
    AutoPtr<FixBuffer>buffer4=new FixBuffer(Cols*Rows);
    if(buffer4->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    BYTE iCode[256];
    for(int k=0;k<256;k++)
    {
        iCode[k]=8;
    }
    iCode[1]=3;
    iCode[2]=4;
    iCode[4]=5;
    iCode[8]=6;
    iCode[16]=7;
    iCode[32]=0;
    iCode[64]=1;
    iCode[128]=2;
    BYTE*sixtArray=(BYTE*)buffer4->GetBuffer();
    for(int i=0;i<Rows;i++){
        LONG Pos=i*Cols;
        for(int j=0;j<Cols;j++){
            if(pData[Pos]==nodata){
                sixtArray[Pos]=8;
                Pos++;
                continue;
            }
            int dire=pData[Pos];
            if((dire<0)||(dire>255))
                sixtArray[Pos]=8;
            else
                sixtArray[Pos]=iCode[dire];
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    CallBackHelper::SendMessage(callBack,"装载地形数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(demBand));
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,oldData,psp.get(),nodata,false);
    AutoPtr<RasterBandTool>pTool2(new RasterBandTool(fillBand));
    pTool2->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                float cV=oldData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0){
                    oldData[Pos]=pData[Pos]=nodata;
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    AutoArray<Point2D<int>>pointArray;
    CallBackHelper::SendMessage(callBack,"针对像素宽度小于3的小洼地倾泻点实施简单洼地突破");
    FourValueArray fvArray(Cols*Rows);
    fvArray.SetDefaultValue(0);
    ImageLumpBlock imageLumpBlock;
    imageLumpBlock.CreateImageBlock(Cols,Rows);
    for (int i = 1; i < Rows - 1; i++){
        LONG Pos = i * Cols + 1;
        for (int j = 1; j < Cols - 1; j++)
        {
            float cV = pData[Pos];
            if (nodata == (LONG)cV)
            {
                Pos++;
                continue;
            }
            if(fvArray[Pos]!=0){
                Pos++;
                continue;
            }
            pointArray.Clear();
            pointArray.Add(Point2D<int>(j,i));
            fvArray.SetValue(Pos,2);
            for(int k=0;k<pointArray.GetSize();k++){
                Point2D<int>cp=pointArray[k];
                int ni=cp.Y;
                int nj=cp.X;
                for (int m = 0; m < 8; m++){
                    int ci = ni + aroundy8[m];
                    int cj = nj + aroundx8[m];
                    if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                        continue;
                    }
                    int nPos=ci*Cols + cj;
                    float nV = pData[nPos];
                    if (nodata == nV){
                        continue;
                    }
                    if(fvArray[nPos]!=0) continue;
                    if(nV==cV){
                        pointArray.Add(Point2D<int>(cj,ci));
                        fvArray.SetValue(nPos,2);
                    }
                }
            }
            if(pointArray.GetSize()<4){
                Pos++;
                continue;
            }
            imageLumpBlock.UpdateImageBlock(pointArray);
            imageLumpBlock.ExtractBorder();
            if(imageLumpBlock.GetMaxValue()<3){
                if(pointArray.GetSize()<40){
                    imageLumpBlock.UpdateImageBlock(pointArray);
                    BreachingSink(pointArray,oldData,sixtArray,imageLumpBlock);
                }
            }
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    for(int i=Cols*Rows-1;i>=0;i--){
        pData[i]=oldData[i];
    }
    CallBackHelper::SendMessage(callBack,"依据倾泻线下挖地形");
    AutoPtr<RasterDataset>resultds=target->CreateRasterDataset(pEnvi.get(),demBand->GetDataType(),1,nodata);
    if(resultds==nullptr){
        CallBackHelper::SendMessage(callBack,"创建修正地形图层失败");
        return nullptr;
    }
    AutoPtr<RasterBand>resultBand=resultds->GetRasterBand(0);
    int featureCount=pouringLine->GetFeatureCount();
    AutoPtr<SelectionSet>selSet=pouringLine->GetSelectionSet();
    int selCount=selSet->GetSize();
    AutoPtr<SpatialReference>psp2=pouringLine->GetSpatialReference();
    AutoPtr<CoordinateTransformation>pTrans(new CoordinateTransformation(psp2,psp));
    pTrans->BeginTransform();
    for(int k=0;k<featureCount;k++){
        if(selCount>0){
            int fid=pouringLine->GetFeatureID(k);
            if(!selSet->FindFeatureID(fid)) continue;
        }
        AutoPtr<Polyline>polyline=pouringLine->GetFeature(k);
        if(polyline==nullptr) continue;
        float digh=pouringLine->GetFieldValueAsDouble(k,digField);
        polyline->Project(pTrans.get());
        DigFeature(polyline.get(),pData,oldData,digh);
        if(k%100==0){
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)k/featureCount*100.0);
        }
    }
    CallBackHelper::SendMessage(callBack,"依据倾泻线下挖地形完成");
    resultBand->SaveBlockData(0,0,Cols,Rows,oldData);
    resultBand->FlushCache();
    return resultBand;
}

HydroLakePouringLineDig::HydroLakePouringLineDig(){

}
HydroLakePouringLineDig::~HydroLakePouringLineDig(){

}
string HydroLakePouringLineDig::GetTypeName(){
    return "HydroLakePouringLineDig";
}

void HydroLakePouringLineDig::PersuitSwitch(AutoPtr<PourLineRoute>route,ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer){
    BYTE*buffer=imageLumpBlock.GetBuffer();
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    int lumpHeight=imageLumpBlock.GetLumpHeight();
    int nj,ni;
    Point2D<int>begin=route->GetBegin();
    int bX=begin.X;
    int bY=begin.Y;
    while(true){
        int uppers=0;
        int upperPos=0;
        for (int m = 0; m < 8; m++)
        {
            ni = bY + aroundy8[m];
            nj = bX + aroundx8[m];
            if ((ni < 0) || (ni >= lumpHeight) || (nj < 0) || (nj >= lumpWidth)) continue;
            if(buffer[nj+ni*lumpWidth]==1){
                upperPos=m;
                uppers++;
            }
            if(maskBuffer[nj+ni*lumpWidth]==1){
                route->Add(nj,ni);
                buffer[nj+ni*lumpWidth]=2;
                route->LabelState(2);
                return;
            }
        }
        if(uppers==1){
            ni = bY + aroundy8[upperPos];
            nj = bX + aroundx8[upperPos];
            route->Add(nj,ni);
            buffer[nj+ni*lumpWidth]=2;
            bX=nj;
            bY=ni;
        }
        else if(uppers==0){
            route->LabelState(0);
            return;
        }
        else{
            route->SetBegin(bX,bY);
            return;
        }
    }
}

vector<AutoPtr<PourLineRoute>>HydroLakePouringLineDig::PersuitSwitchs(AutoPtr<PourLineRoute>route,ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer,FLOAT*oldData){
    BYTE*buffer=imageLumpBlock.GetBuffer();
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    int lumpHeight=imageLumpBlock.GetLumpHeight();
    int Cols=imageLumpBlock.GetCols();
    int Rows=imageLumpBlock.GetRows();
    int nj,ni;
    Point2D<int>begin=route->GetBegin();
    int bX=begin.X;
    int bY=begin.Y;
    int upperCount=0;
    int upperX,upperY;
    for (int m = 0; m < 8; m++)
    {
        ni = bY + aroundy8[m];
        nj = bX + aroundx8[m];
        if ((ni < 0) || (ni >= lumpHeight) || (nj < 0) || (nj >= lumpWidth)) continue;
        if(buffer[nj+ni*lumpWidth]==1){
            upperCount++;
            upperX=nj;
            upperY=ni;
        }
    }
    if(upperCount==1){
        route->SetBegin(upperX,upperY);
        route->Add(upperX,upperY);
        buffer[upperX+upperY*lumpWidth]=2;
        PersuitSwitch(route,imageLumpBlock,maskBuffer);
        if(route->GetState()==0) return {};
        return {route};
    }
    vector<AutoPtr<PourLineRoute>>resultRoutes;
    for (int m = 0; m < 8; m++)
    {
        ni = bY + aroundy8[m];
        nj = bX + aroundx8[m];
        if ((ni < 0) || (ni >= lumpHeight) || (nj < 0) || (nj >= lumpWidth)) continue;
        if(buffer[nj+ni*lumpWidth]==1){
            buffer[nj+ni*lumpWidth]=2;
            AutoPtr<PourLineRoute>nrt=new PourLineRoute();
            nrt->CopyFrom(route.get());
            nrt->SetBegin(nj,ni);
            nrt->Add(nj,ni);
            buffer[nj+ni*lumpWidth]=2;
            PersuitSwitch(nrt,imageLumpBlock,maskBuffer);
            if(nrt->GetState()==0) continue;
            resultRoutes.push_back(nrt);
        }
    }
    return resultRoutes;
}

AutoPtr<PourLineRoute>HydroLakePouringLineDig::PersuitRoute(ImageLumpBlock&imageLumpBlock,BYTE*maskBuffer,FLOAT*oldData,int bX,int bY){
    //int nej=bX;
    //int nei=bY;
    vector<AutoPtr<PourLineRoute>>routes;
    AutoPtr<PourLineRoute>route=new PourLineRoute();
    route->Add(bX,bY);
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    BYTE*buffer=imageLumpBlock.GetBuffer();
    buffer[bX+bY*lumpWidth]=2;
    route->SetBegin(bX,bY);
    routes.push_back(route);
    AutoPtr<PourLineRoute>rRoute=nullptr;
    while(true){
        for(int k=routes.size()-1;k>=0;k--){
            int state=routes[k]->GetState();
            if(state==0){
                routes.erase(begin(routes)+k);
            }
            else if(state==2){
                if(rRoute==nullptr)
                    rRoute=routes[k];
                else if(routes[k]->GetSize()<rRoute->GetSize())
                    rRoute=routes[k];
                routes.erase(begin(routes)+k);
            }
        }
        int minLength=0;
        int minIndex=-1;
        for(int k=routes.size()-1;k>=0;k--){
            if(minIndex==-1){
                minLength=routes[k]->GetSize();
                minIndex=k;
            }
            else if(routes[k]->GetSize()<minLength){
                minLength=routes[k]->GetSize();
                minIndex=k;
            }
        }
        if(minIndex==-1) break;
        vector<AutoPtr<PourLineRoute>>nextRoutes=PersuitSwitchs(routes[minIndex],imageLumpBlock,maskBuffer,oldData);
        routes.erase(begin(routes)+minIndex);
        for(int k=0;k<nextRoutes.size();k++){
            routes.push_back(nextRoutes[k]);
        }
    }
    return rRoute;
}

vector<AutoArray<Point2D<int>>>HydroLakePouringLineDig::ClearUpPond(ImageLumpBlock&imageLumpBlock2){
    int lumpWidth=imageLumpBlock2.GetLumpWidth();
    int lumpHeight=imageLumpBlock2.GetLumpHeight();
    BYTE*buffer2=imageLumpBlock2.GetBuffer();
    imageLumpBlock2.MakeForeGroundOne();
    vector<AutoArray<Point2D<int>>>dpts;
    int nPos=0;
    for(int i=0;i<lumpHeight;i++){
        for(int j=0;j<lumpWidth;j++){
            if(buffer2[nPos]!=1){
                nPos++;
                continue;
            }
            AutoArray<Point2D<int>>subpts;
            subpts.Add(Point2D<int>(j,i));
            buffer2[nPos]=2;
            for(int k=0;k<subpts.GetSize();k++){
                Point2D<int>fpt=subpts.GetItem(k);
                for (int m = 0; m < 8; m++)
                {
                    int ni = fpt.Y + aroundy8[m];
                    int nj = fpt.X + aroundx8[m];
                    if ((ni < 0) || (ni >= lumpHeight) || (nj < 0) || (nj >= lumpWidth)){
                        continue;
                    }
                    if(buffer2[nj+ni*lumpWidth]==1){
                        buffer2[nj+ni*lumpWidth]=2;
                        subpts.Add(Point2D<int>(nj,ni));
                    }
                }
            }
            dpts.push_back(subpts);
            nPos++;
        }
    }
    vector<int>widths(dpts.size());
    int maxWidth=0;
    for(int l=0;l<dpts.size();l++){
        AutoArray<Point2D<int>>&subpts=dpts[l];
        for(int p=lumpWidth*lumpHeight-1;p>=0;p--) buffer2[p]=0;
        for(int k=0;k<subpts.GetSize();k++){
            Point2D<int>pt=subpts[k];
            buffer2[pt.X+pt.Y*lumpWidth]=1;
        }
        imageLumpBlock2.ExtractBorder();
        int mv=imageLumpBlock2.GetMaxValue();
        widths[l]=mv;
        if(mv>maxWidth) maxWidth=mv;
    }
    for(int p=lumpWidth*lumpHeight-1;p>=0;p--) buffer2[p]=0;
    for(int l=dpts.size()-1;l>=0;l--){
        AutoArray<Point2D<int>>&subpts=dpts[l];
        if(widths[l]==maxWidth){
            for(int k=0;k<subpts.GetSize();k++){
                Point2D<int>pt=subpts[k];
                buffer2[pt.X+pt.Y*lumpWidth]=1;
            }
        }else{
            dpts.erase(begin(dpts)+l);
        }
    }
    return dpts;
}

bool HydroLakePouringLineDig::ComputeUpPond(ImageLumpBlock&imageLumpBlock,ImageLumpBlock&imageLumpBlock2,FLOAT*oldData,int lakeMinRadius,float&innerHeight){
    int left=imageLumpBlock.GetLeft();
    int top=imageLumpBlock.GetTop();
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    int lumpHeight=imageLumpBlock.GetLumpHeight();
    BYTE*buffer=imageLumpBlock.GetBuffer();
    BYTE*buffer2=imageLumpBlock2.GetBuffer();
    int nPos=0;
    int Cols=pEnvi->Cols();
    OrderArray<float>dems;
    for(int i=0;i<lumpHeight;i++){
        int ni=i+top;
        for(int j=0;j<lumpWidth;j++){
            int nj=j+left;
            int lPos=nj+ni*Cols;
            if(buffer[nPos]==1){
                if(oldData[lPos]==nodata){
                    nPos++;
                    continue;
                }
                dems.Add(oldData[lPos]);
            }
            nPos++;
        }
    }
    int totalNum=dems.GetSize();
    if(totalNum<2) return false;
    int fPos=0;
    int tPos=totalNum-2;
    int cPos=(tPos+fPos)/2;
    int formerPos=-1;
    float fDem=dems[0];
    while(true){
        float curDem=dems[cPos];
        nPos=0;
        for(int i=0;i<lumpHeight;i++){
            int ni=i+top;
            for(int j=0;j<lumpWidth;j++){
                int nj=j+left;
                int lPos=nj+ni*Cols;
                if(buffer[nPos]==0){
                    nPos++;
                    continue;
                }
                float dem=oldData[lPos];
                if(dem<=curDem)
                    buffer2[nPos]=1;
                else
                    buffer2[nPos]=0;
                nPos++;
            }
        }
        imageLumpBlock2.ExtractBorder();
        int maxBorder=imageLumpBlock2.GetMaxValue();
        if(maxBorder==1){
            if(maxBorder>lakeMinRadius){
                tPos=cPos;
                cPos=(tPos+fPos)/2;
            }
            else if(maxBorder<=lakeMinRadius){
                fPos=cPos;
                cPos=(tPos+fPos)/2;
                if(fDem<curDem) fDem=curDem;
            }
        }
        else{
            if(maxBorder>=lakeMinRadius){
                tPos=cPos;
                cPos=(tPos+fPos)/2;
            }
            else if(maxBorder<lakeMinRadius){
                fPos=cPos;
                cPos=(tPos+fPos)/2;
                if(fDem<curDem) fDem=curDem;
            }
        }
        if(formerPos==cPos){
            break;
        }
        formerPos=cPos;
    }
    innerHeight=fDem;
    return true;
}

void HydroLakePouringLineDig::BreachingHeading(int spillX,int spillY,int Cols,int Rows,FLOAT*oldData,BYTE*flowDir){
    int nPos=spillX+spillY*Cols;
    float centerDem=oldData[nPos];
    float upperDem=centerDem;
    int validNum=0;
    int ii,jj,ccj,cci,dir;
    int nnPos;
    for(int m=0;m<8;m++){
        ii=spillY+aroundy8[m];
        jj=spillX+aroundx8[m];
        if ((ii < 0) || (ii >= Rows) || (jj < 0) || (jj >= Cols)) continue;
        nnPos=jj+ii*Cols;
        if(oldData[nnPos]==nodata) continue;
        dir=flowDir[nnPos];
        if((dir<0)||(dir>=8)) continue;
        cci=ii+aroundy8[dir];
        ccj=jj+aroundx8[dir];
        if((cci!=spillY)||(ccj!=spillX)) continue;
        if(validNum==0){
            upperDem=oldData[nnPos];
        }
        else if(oldData[nnPos]<upperDem)
            upperDem=oldData[nnPos];
        validNum++;
    }
    if(validNum==0) return;
    if(centerDem>upperDem)
        oldData[nPos]=upperDem;
    else
        upperDem=centerDem;
    /*
    dir=flowDir[spillX+spillY*Cols];
    if((dir<0)||(dir>=8)) return;
    cci=spillY+aroundy8[dir];
    ccj=spillX+aroundx8[dir];
    if ((cci < 0) || (cci >= Rows) || (ccj < 0) || (ccj >= Cols)) return;
    nnPos=ccj+cci*Cols;
    if(oldData[nnPos]==nodata) return;
    float nextDem=oldData[nnPos];
    if(nextDem>upperDem) oldData[nnPos]=upperDem;
    */
}

void HydroLakePouringLineDig::BreachingSink(AutoArray<Point2D<int>>&pointArray,FLOAT*oldData,BYTE*flowDir,ImageLumpBlock&imageLumpBlock){
    vector<Point2D<int>>pourpts;
    int pointCount = pointArray.GetSize();
    if(pointCount==0) return;
    int Cols=imageLumpBlock.GetCols();
    int Rows=imageLumpBlock.GetRows();
    int left=imageLumpBlock.GetLeft();
    int top=imageLumpBlock.GetTop();
    BYTE*buffer=imageLumpBlock.GetBuffer();
    int lw=imageLumpBlock.GetLumpWidth();
    int lh=imageLumpBlock.GetLumpHeight();
    for (int k = 0; k < pointCount; k++){
        Point2D<int>point = pointArray.GetItem(k);
        int i = point.Y;
        int j = point.X;
        int dir=flowDir[j+i*Cols];
        if((dir<0)||(dir>=8)) continue;
        int cci=i+aroundy8[dir];
        int ccj=j+aroundx8[dir];
        if ((cci < 0) || (cci >= Rows) || (ccj < 0) || (ccj >= Cols)){
            pourpts.push_back(Point2D<int>(j,i));
            continue;
        }
        if(oldData[ccj+cci*Cols]==nodata){
            pourpts.push_back(Point2D<int>(j,i));
            continue;
        }
        int ci=cci-top;
        int cj=ccj-left;
        if ((ci < 0) || (ci >= lh) || (cj < 0) || (cj >= lw)){
            pourpts.push_back(Point2D<int>(j,i));
            continue;
        }
        int state=buffer[cj+ci*lw];
        if(state==1) continue;
        pourpts.push_back(Point2D<int>(j,i));
    }
    for(int k=0;k<pourpts.size();k++){
        Point2D<int>&pt=pourpts[k];
        BreachingHeading(pt.X,pt.Y,Cols,Rows,oldData,flowDir);
    }
}

void HydroLakePouringLineDig::DigDem(FLOAT*oldData,vector<Point2D<int>>&route,float innerHeight,int Cols,int Rows){
    int cellCount=route.size();
    vector<float>dems(cellCount);
    for(int c=0;c<cellCount;c++){
        Point2D<int>cell=route[c];
        float dem=oldData[cell.X+cell.Y*Cols];
        dems[c]=dem;
    }
    dems[0]=innerHeight;
    float beginDem=innerHeight;
    float endDem=dems[cellCount-1];
    int lPos=cellCount-1;
    if(endDem==nodata){
        for(int c=cellCount-2;c>=0;c--){
            if(dems[c]!=nodata){
                endDem=dems[c];
                lPos=c;
                break;
            }
        }
    }
    if(endDem==nodata) return;
    if(beginDem<endDem) endDem=beginDem;
    for(int c=lPos;c<cellCount;c++) dems[c]=endDem;
    for(int c=0;c<cellCount;c++){
        if(dems[c]==nodata) dems[c]=innerHeight;
        if(dems[c]<endDem) dems[c]=endDem;
    }
    float curDem=beginDem;
    for(int c=1;c<cellCount-1;c++){
        if(dems[c]>curDem)
            dems[c]=curDem;
        else if(dems[c]<curDem)
            curDem=dems[c];
    }
    for(int c=0;c<cellCount;c++){
        Point2D<int>cell=route[c];
        float dem=oldData[cell.X+cell.Y*Cols];
        if(dem==nodata) continue;
        if(oldData[cell.X+cell.Y*Cols]>dems[c])
            oldData[cell.X+cell.Y*Cols]=dems[c];
    }
}

void HydroLakePouringLineDig::UpdateLocalHydro(FLOAT*fillData,FLOAT*oldData,ImageLumpBlock&imageLumpBlock,BYTE*flowDir){
    int Cols=imageLumpBlock.GetCols();
    int Rows=imageLumpBlock.GetRows();
    int left=imageLumpBlock.GetLeft();
    int top=imageLumpBlock.GetTop();
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    int lumpHeight=imageLumpBlock.GetLumpHeight();
    int rl,rt,rr,rb;
    rl=left-10;
    if(rl<0) rl=0;
    rt=top-10;
    if(rt<0) rt=0;
    rr=left+lumpWidth+9;
    if(rr>=Cols) rr=Cols-1;
    rb=top+lumpHeight+9;
    if(rb>=Rows) rb=Rows-1;
    int rw=rr-rl+1;
    int rh=rb-rt+1;
    int difx=left-rl;
    int dify=top-rt;
    RasterDesp desp;
    desp.left=0;
    desp.top=0;
    desp.xCellSize=desp.yCellSize=1.0;
    desp.cols=rw;
    desp.rows=rh;
    AutoPtr<SpatialReference>psp(new SpatialReference());
    AutoPtr<MemRasterWorkspaceFactory>pFac(new MemRasterWorkspaceFactory());
    AutoPtr<MemRasterWorkspace>pWork=pFac->CreateMemRasterWorkspace();
    AutoPtr<MemRasterDataset>mds=pWork->CreateRasterDataset("Mem",desp,1,rdtFloat32,nodata,psp.get());
    AutoPtr<MemRasterBand>mBand=mds->GetRasterBand(0);
    float*dem=(float*)mBand->GetRawData()->GetValues();
    BYTE*buffer=imageLumpBlock.GetBuffer();
    int Pos;
    int pos=0;
    for(int i=rt;i<=rb;i++){
        Pos=i*Cols+rl;
        for(int j=rl;j<=rr;j++){
            dem[pos]=oldData[Pos];
            Pos++;
            pos++;
        }
    }
    AutoPtr<SAEnvironment>pEnvi(new SAEnvironment());
    pEnvi->Cols()=rw;
    pEnvi->Rows()=rh;
    pEnvi->Left()=0;
    pEnvi->Top()=0;
    pEnvi->XCellSize()=1;
    pEnvi->YCellSize()=1;
    pEnvi->SetSpatialReference(psp);
    AutoPtr<HydroDemFill>fill(new HydroDemFill());
    fill->SetEnvironment(pEnvi);
    AutoPtr<HydroFlowDirection>flow=new HydroFlowDirection();
    flow->SetEnvironment(pEnvi);
    AutoPtr<HydroFlowAccumulation>accu=new HydroFlowAccumulation();
    accu->SetEnvironment(pEnvi);
    AutoPtr<MemRasterTarget>target(new MemRasterTarget());
    target->Name()="Mem";
    BYTE iCode[256];
    for(int k=0;k<256;k++)
    {
        iCode[k]=8;
    }
    iCode[1]=3;
    iCode[2]=4;
    iCode[4]=5;
    iCode[8]=6;
    iCode[16]=7;
    iCode[32]=0;
    iCode[64]=1;
    iCode[128]=2;
    AutoPtr<MemRasterBand>fill_dem=fill->Execute(mBand.get(),target.get());
    if(fill_dem==nullptr) return;
    AutoPtr<MemRasterBand>flow_band=flow->Execute(mBand.get(),fill_dem.get(),target.get());
    if(flow_band==nullptr) return;
    BYTE*m_dir=(BYTE*)flow_band->GetRawData()->GetValues();
    pos=0;
    int lpos=0;
    for(int i=0;i<lumpHeight;i++){
        Pos=(i+top)*Cols+left;
        pos=difx+(i+dify)*rw;
        for(int j=0;j<lumpWidth;j++){
            if(buffer[lpos]==0){
                Pos++;
                pos++;
                lpos++;
                continue;
            }
            int dire=m_dir[pos];
            if((dire<0)||(dire>255))
                flowDir[Pos]=8;
            else
                flowDir[Pos]=iCode[dire];
            Pos++;
            pos++;
            lpos++;
        }
    }
}

void HydroLakePouringLineDig::DigByPouringLine(AutoArray<Point2D<int>>&pointArray,FourValueArray&fvArray,FLOAT*pData,FLOAT*oldData,BYTE*sixArray,int Cols,int Rows,float innerHeight,ImageLumpBlock&imageLumpBlock,ImageLumpBlock&imageLumpBlock2){
    int pointCount = pointArray.GetSize();
    if(pointCount==0) return;
    float innermindem=innerHeight;
    float fillDem=pData[pointArray[0].X+pointArray[0].Y*Cols];
    int maxDownStreams=max(Cols,Rows);
    int minDownStreams=15;
    //首先，将pointArray转换为ImageLump
    //然后找到淹没区的倾泻点,按照水流方向，外部的倾泻点必须是不能再流入到淹没区的
    vector<Point2D<int>>pourpts;
    //向下游追踪
    for (int k = 0; k < pointCount; k++){
        Point2D<int>point = pointArray.GetItem(k);
        int i = point.Y;
        int j = point.X;
        int dir=sixArray[j+i*Cols];
        if((dir<0)||(dir>=8)) continue;
        int cci=i+aroundy8[dir];
        int ccj=j+aroundx8[dir];
        if ((cci < 0) || (cci >= Rows) || (ccj < 0) || (ccj >= Cols)){
            pourpts.push_back(Point2D<int>(j,i));
            continue;
        }
        int state=fvArray[ccj+cci*Cols];
        if(state==3){
            continue;
        }
        if(oldData[ccj+cci*Cols]==nodata){
            pourpts.push_back(Point2D<int>(j,i));
            continue;
        }
        int curX=j;
        int curY=i;
        float curd=oldData[curX+curY*Cols];
        vector<int>downs;
        vector<int>downstates;
        int downNum=0;
        bool adjacent=false;
        int curDownStreams=0;
        float formerdem=innermindem;
        while(true){
            int dir=sixArray[curX+curY*Cols];
            if((dir<0)||(dir>=8)) break;
            int ci=curY+aroundy8[dir];
            int cj=curX+aroundx8[dir];
            if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                break;
            }
            int pPos=ci*Cols + cj;
            if (fvArray[pPos] == 3){
                adjacent=true;
                break;
            }
            if (fvArray[pPos] == 1){
                break;
            }
            curd=oldData[pPos];
            curX=cj;
            curY=ci;
            downs.push_back(pPos);
            downstates.push_back(fvArray[pPos]);
            fvArray.SetValue(pPos,1);
            downNum++;
            //相对于洼地最低积水线低
            if(curd<=innermindem){
                if(curDownStreams==0) curDownStreams=1;
                if(curDownStreams>=minDownStreams){
                    if((curd<=formerdem)||(curDownStreams>=minDownStreams*2)) break;
                }
                //break;
            }
            if(downNum>maxDownStreams) break;
            if(curDownStreams>0) curDownStreams++;
            formerdem=curd;
        }
        for(int d=0;d<downNum;d++){
            fvArray.SetValue(downs[d],downstates[d]);
        }
        if(adjacent) continue;
        //if(downNum==0) continue;
        bool bSame=false;
        for(int l=0;l<pourpts.size();l++){
            if((pourpts[l].X==j)&&(pourpts[l].Y==i)){
                bSame=true;
                break;
            }
        }
        if(!bSame){
            pourpts.push_back(Point2D<int>(j,i));
        }
    }
    vector<vector<Point2D<int>>>pourRoutes;
    for(int p=0;p<pourpts.size();p++){
        int curX=pourpts[p].X;
        int curY=pourpts[p].Y;
        float curd=oldData[curX+curY*Cols];
        int dir=sixArray[curX+curY*Cols];
        if((dir<0)||(dir>=8)) break;
        vector<int>downs;
        vector<int>downstates;
        vector<Point2D<int>>routes;
        routes.push_back(Point2D<int>(curX,curY));
        int downNum=0;
        int curDownStreams=0;
        float formerdem=innermindem;
        while(true){
            int dir=sixArray[curX+curY*Cols];
            if((dir<0)||(dir>=8)) break;
            int ci=curY+aroundy8[dir];
            int cj=curX+aroundx8[dir];
            if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                break;
            }
            if(oldData[cj+ci*Cols]==nodata){
                break;
            }
            int pPos=ci*Cols + cj;
            if ((fvArray[pPos] == 3)||(fvArray[pPos] == 1)) break;
            curd=oldData[pPos];
            curX=cj;
            curY=ci;
            downs.push_back(pPos);
            downstates.push_back(fvArray[pPos]);
            routes.push_back(Point2D<int>(cj,ci));
            fvArray.SetValue(pPos,1);
            downNum++;
            //相对于洼地最低积水线低
            if(curd<=innermindem){
                if(curDownStreams==0) curDownStreams=1;
                if(curDownStreams>=minDownStreams){
                    if((curd<=formerdem)||(curDownStreams>=minDownStreams*2)) break;
                }
                //break;
            }
            if(downNum>maxDownStreams) break;
            if(curDownStreams>0) curDownStreams++;
            formerdem=curd;
        }
        pourRoutes.push_back(routes);
        for(int k=0;k<downs.size();k++){
            fvArray.SetValue(downs[k],downstates[k]);
        }
        //if(mind<globalMinDem) globalMinDem=mind;
    }
    //依据路径，去除至少包含一个相同位置的
    int routeCount=pourRoutes.size();
    vector<bool>valids(routeCount);
    for(int k=0;k<routeCount;k++){
        valids[k]=true;
    }
    for(int i=0;i<routeCount-1;i++){
        vector<Point2D<int>>&r1=pourRoutes[i];
        if(!valids[i]) continue;
        int pts1=r1.size();
        for(int j=i+1;j<routeCount;j++){
            vector<Point2D<int>>&r2=pourRoutes[j];
            if(!valids[j]) continue;
            int pts2=r2.size();
            bool bSame=false;
            for(int m=0;m<pts1;m++){
                 Point2D<int>&pt1=r1[m];
                 for(int n=0;n<pts2;n++){
                    Point2D<int>&pt2=r2[n];
                    if((pt1.X==pt2.X)&&(pt1.Y==pt2.Y)){
                        bSame=true;
                        break;
                    }
                 }
                 if(bSame) break;
            }
            if(bSame){
                 if(r1.size()>=r2.size()){
                    valids[j]=false;
                 }
                 else{
                    valids[i]=false;
                    break;
                 }
            }
        }
    }
    for(int k=routeCount-1;k>=0;k--){
        if(!valids[k]){
            pourRoutes.erase(begin(pourRoutes)+k);
            pourpts.erase(begin(pourpts)+k);
        }
    }
    RasterDesp desp=pEnvi->GetRasterDesp();
    int lumpLeft=imageLumpBlock.GetLeft();
    int lumpTop=imageLumpBlock.GetTop();
    int lumpWidth=imageLumpBlock.GetLumpWidth();
    int lumpHeight=imageLumpBlock.GetLumpHeight();
    BYTE*maskBuffer=imageLumpBlock2.GetBuffer();
    BYTE*buffer=imageLumpBlock.GetBuffer();
    OrderArray<double>dems;
    vector<int>demCounts;
    int totalCount=0;
    int left=imageLumpBlock.GetLeft();
    int top=imageLumpBlock.GetTop();
    int nPos=0;
    for(int i=0;i<lumpHeight;i++){
        int ni=i+top;
        for(int j=0;j<lumpWidth;j++){
            int nj=j+left;
            int lPos=nj+ni*Cols;
            if(buffer[nPos]==1){
                 if(oldData[lPos]==nodata){
                    nPos++;
                    continue;
                 }
                 totalCount++;
                 int fpos=dems.Find(oldData[lPos]);
                 if(fpos<0){
                    fpos=dems.Add(oldData[lPos]);
                    demCounts.insert(begin(demCounts)+fpos,1);
                 }
                 else{
                    demCounts[fpos]++;
                 }
            }
            nPos++;
        }
    }
    vector<float>referDems;
    int curCount=0;
    float formerPer=0;
    for(int k=dems.GetSize()-1;k>=0;k--){
        if(dems[k]<=innerHeight) break;
        curCount+=demCounts[k];
        float per=curCount*1.0/totalCount;
        if((per-formerPer>=0.05)&&(k<dems.GetSize()-1)){
            formerPer=per;
            referDems.push_back(dems[k]);
        }
    }
    AutoPtr<FixBuffer>fxb=new FixBuffer(lumpWidth*lumpHeight);
    BYTE*mbs=(BYTE*)fxb->GetBuffer();
    for(int r=0;r<pourRoutes.size();r++){
        //向上游溯源
        imageLumpBlock.MakeForeGroundOne();
        vector<Point2D<int>>&route=pourRoutes[r];
        vector<Point2D<int>>tails;
        tails.push_back(Point2D<int>(route[0].X,route[0].Y));
        imageLumpBlock2.CopyFrom(&imageLumpBlock);
        for(int l=0;l<referDems.size();l++){
            int nPos=0;
            for(int i=0;i<lumpHeight;i++){
                 int ni=i+top;
                 for(int j=0;j<lumpWidth;j++){
                    int nj=j+left;
                    int lPos=nj+ni*Cols;
                    if(buffer[nPos]==1){
                        if(oldData[lPos]==nodata){
                            mbs[nPos]=0;
                            nPos++;
                            continue;
                        }
                        if(oldData[lPos]<=referDems[l])
                            mbs[nPos]=1;
                        else
                            mbs[nPos]=0;
                    }
                    else
                        mbs[nPos]=0;
                    nPos++;
                 }
            }
            mbs[route[0].X-lumpLeft+(route[0].Y-lumpTop)*lumpWidth]=1;
            imageLumpBlock2.ThinImage(mbs);
        }
        imageLumpBlock2.ThinImage(tails);
        int state=buffer[route[0].X-lumpLeft+(route[0].Y-lumpTop)*lumpWidth];
        if(state!=1){
            continue;
        }
        imageLumpBlock.CopyFrom(&imageLumpBlock2);
        for (int k = 0; k < pointCount; k++){
            Point2D<int>pt=pointArray.GetItem(k);
            int iX=pt.X-imageLumpBlock.GetLeft();
            int iY=pt.Y-imageLumpBlock.GetTop();
            int ni=iY+top;
            int nj=iX+left;
            int lPos=nj+ni*Cols;
            if(oldData[lPos]<=innerHeight)
                 maskBuffer[iX+iY*imageLumpBlock.GetLumpWidth()]=1;
            else
                 maskBuffer[iX+iY*imageLumpBlock.GetLumpWidth()]=0;
        }
        vector<AutoArray<Point2D<int>>>dpts=ClearUpPond(imageLumpBlock2);
        int bX=route[0].X-lumpLeft;
        int bY=route[0].Y-lumpTop;
        for(int d=0;d<dpts.size();d++){
            AutoArray<Point2D<int>>&subpts=dpts[d];
            for(int p=lumpWidth*lumpHeight-1;p>=0;p--) maskBuffer[p]=0;
            for(int k=0;k<subpts.GetSize();k++){
                 Point2D<int>pt=subpts[k];
                 maskBuffer[pt.X+pt.Y*lumpWidth]=1;
            }
            AutoPtr<PourLineRoute>upperRoute=PersuitRoute(imageLumpBlock,maskBuffer,oldData,bX,bY);
            if(upperRoute!=nullptr){
                 vector<Point2D<int>>nroutes(upperRoute->GetSize()+route.size()-1);
                 int pIndex=0;
                 for(int n=upperRoute->GetSize()-1;n>=1;n--){
                    Point2D<int>pt=upperRoute->GetNode(n);
                    pt.X+=lumpLeft;
                    pt.Y+=lumpTop;
                    nroutes[pIndex++]=pt;
                 }
                 for(int l=0;l<route.size();l++){
                    Point2D<int>&pt=route[l];
                    nroutes[pIndex++]=pt;
                 }
                 DigDem(oldData,nroutes,innerHeight,Cols,Rows);
            }
            for(int p=lumpWidth*lumpHeight-1;p>=0;p--){
                 if(buffer[p]>0) buffer[p]=1;
            }
        }
    }
    for (int k = 0; k < pointCount; k++){
        Point2D<int>point = pointArray.GetItem(k);
        fvArray.SetValue(point.X+point.Y*Cols, 2);
    }
    imageLumpBlock.UpdateImageBlock(pointArray);
    UpdateLocalHydro(pData,oldData,imageLumpBlock,sixArray);
}

AutoPtr<RasterBand>HydroLakePouringLineDig::innerExecute(RasterBand*demBand,RasterBand*fillBand,RasterBand*fldBand,RasterBand*accuBand,int lakeMinRadius,bool breachingHeader,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    RasterDesp desp=pEnvi->GetRasterDesp();
    DOUBLE Left=desp.left,Top=desp.top;
    DOUBLE xCellSize=desp.xCellSize,yCellSize=desp.yCellSize;
    LONG Cols=desp.cols;
    LONG Rows=desp.rows;
    AutoPtr<SpatialReference>psp=pEnvi->GetSpatialReference();
    AutoPtr<FixBuffer>buffer=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*pData=(float*)buffer->GetBuffer();
    AutoPtr<FixBuffer>buffer2=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer2->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*oldData=(float*)buffer2->GetBuffer();
    AutoPtr<FixBuffer>buffer3=new FixBuffer(Cols*Rows*sizeof(FLOAT));
    if(buffer3->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    float*accuData=(float*)buffer3->GetBuffer();
    AutoPtr<Point>LeftTop(new Point(Left,Top));
    AutoPtr<RasterBandTool>pfTool(new RasterBandTool(fldBand));
    pfTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    CallBackHelper::SendMessage(callBack,"初始化水流方向");
    AutoPtr<FixBuffer>buffer4=new FixBuffer(Cols*Rows);
    if(buffer4->GetBuffer()==nullptr){
        CallBackHelper::SendMessage(callBack,"创建临时数据失败，请确认内存是否足够");
        return nullptr;
    }
    BYTE iCode[256];
    for(int k=0;k<256;k++)
    {
        iCode[k]=8;
    }
    iCode[1]=3;
    iCode[2]=4;
    iCode[4]=5;
    iCode[8]=6;
    iCode[16]=7;
    iCode[32]=0;
    iCode[64]=1;
    iCode[128]=2;
    BYTE*sixtArray=(BYTE*)buffer4->GetBuffer();
    for(int i=0;i<Rows;i++){
        LONG Pos=i*Cols;
        for(int j=0;j<Cols;j++){
            if(pData[Pos]==nodata){
                sixtArray[Pos]=8;
                Pos++;
                continue;
            }
            int dire=pData[Pos];
            if((dire<0)||(dire>255))
                sixtArray[Pos]=8;
            else
                sixtArray[Pos]=iCode[dire];
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    CallBackHelper::SendMessage(callBack,"装载地形数据");
    AutoPtr<RasterBandTool>pTool(new RasterBandTool(demBand));
    pTool->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,oldData,psp.get(),nodata,false);
    AutoPtr<RasterBandTool>pTool2(new RasterBandTool(fillBand));
    pTool2->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,pData,psp.get(),nodata,false);
    AutoPtr<RasterBandTool>pTool3(new RasterBandTool(accuBand));
    pTool3->GetBlockDataByCoord(LeftTop.get(),xCellSize,yCellSize,Cols,Rows,accuData,psp.get(),nodata,false);
    bool bSuc;
    BYTE*pArray=GetMaskData(callBack,signal,bSuc);
    if(!bSuc){
        CallBackHelper::SendMessage(callBack,"任务取消");
        return nullptr;
    }
    if(pArray!=nullptr){
        CallBackHelper::SendMessage(callBack,"定义掩膜");
        for(int i=0;i<Rows;i++)
        {
            LONG Pos=i*Cols;
            for(int j=0;j<Cols;j++)
            {
                float cV=pData[Pos];
                if(nodata==cV)
                {
                    Pos++;
                    continue;
                }
                if(pArray[Pos]==0){
                    pData[Pos]=nodata;
                    oldData[Pos]=nodata;
                    accuData[Pos]=nodata;
                }
                Pos++;
            }
            if(DetectExitSignal(callBack,signal)){
                return nullptr;
            }
            CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
        }
    }
    AutoArray<Point2D<int>>pointArray;
    CallBackHelper::SendMessage(callBack,"提取洼地");
    FourValueArray fvArray(Cols*Rows);
    fvArray.SetDefaultValue(0);
    ImageLumpBlock imageLumpBlock;
    imageLumpBlock.CreateImageBlock(Cols,Rows);
    ImageLumpBlock imageLumpBlock2;
    imageLumpBlock2.CreateImageBlock(Cols,Rows);
    OrderArray<int>accuOrders(true);
    vector<Point2D<int>>sinkPoses;
    for (int i = 1; i < Rows - 1; i++){
        LONG Pos = i * Cols + 1;
        for (int j = 1; j < Cols - 1; j++)
        {
            float cV = pData[Pos];
            if (nodata == (LONG)cV)
            {
                Pos++;
                continue;
            }
            if(fvArray[Pos]!=0){
                Pos++;
                continue;
            }
            pointArray.Clear();
            pointArray.Add(Point2D<int>(j,i));
            fvArray.SetValue(Pos,2);
            for(int k=0;k<pointArray.GetSize();k++){
                Point2D<int>cp=pointArray[k];
                int ni=cp.Y;
                int nj=cp.X;
                for (int m = 0; m < 8; m++){
                    int ci = ni + aroundy8[m];
                    int cj = nj + aroundx8[m];
                    if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                        continue;
                    }
                    int nPos=ci*Cols + cj;
                    float nV = pData[nPos];
                    if (nodata == nV){
                        continue;
                    }
                    if(fvArray[nPos]!=0) continue;
                    if(nV==cV){
                        pointArray.Add(Point2D<int>(cj,ci));
                        fvArray.SetValue(nPos,2);
                    }
                }
            }
            if(pointArray.GetSize()<4){
                Pos++;
                continue;
            }
            imageLumpBlock.UpdateImageBlock(pointArray);
            imageLumpBlock.ExtractBorder();
            if(imageLumpBlock.GetMaxValue()<lakeMinRadius){
                if((breachingHeader)&&(pointArray.GetSize()<40)){
                    imageLumpBlock.UpdateImageBlock(pointArray);
                    BreachingSink(pointArray,oldData,sixtArray,imageLumpBlock);
                }
                Pos++;
                continue;
            }
            float maxaccu=accuData[pointArray[0].X+pointArray[0].Y*Cols];
            for(int l=pointArray.GetSize()-1;l>=1;l--){
                Point2D<int>cp=pointArray[l];
                float accu=accuData[cp.X+cp.Y*Cols];
                if(accu>maxaccu) maxaccu=accu;
            }
            int nPos=accuOrders.Add(maxaccu);
            sinkPoses.insert(begin(sinkPoses)+nPos,Point2D<int>(j,i));
            Pos++;
        }
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)i/Rows*100.0);
    }
    AutoPtr<RasterDataset>newds=target->CreateRasterDataset(pEnvi.get(),rdtFloat32,1,nodata);
    if(newds==nullptr){
        CallBackHelper::SendMessage(callBack,"生成结果图层失败");
        return nullptr;
    }
    AutoPtr<RasterBand>newBand=newds->GetRasterBand(0);
    CallBackHelper::SendMessage(callBack,"基于洼地倾泻线的地形下挖");
    int sinkNum=sinkPoses.size();
    for(int k=sinkNum-1;k>=0;k--){
        Point2D<int>fromPos=sinkPoses[k];
        int Pos=fromPos.X+fromPos.Y*Cols;
        float cV = pData[Pos];
        if (nodata == (LONG)cV) continue;
        if(fvArray[Pos]!=2) continue;
        pointArray.Clear();
        pointArray.Add(Point2D<int>(fromPos.X,fromPos.Y));
        fvArray.SetValue(Pos,3);
        for(int k=0;k<pointArray.GetSize();k++){
            Point2D<int>cp=pointArray[k];
            int ni=cp.Y;
            int nj=cp.X;
            for (int m = 0; m < 8; m++){
                int ci = ni + aroundy8[m];
                int cj = nj + aroundx8[m];
                if ((ci < 0) || (ci >= Rows) || (cj < 0) || (cj >= Cols)){
                    continue;
                }
                int nPos=ci*Cols + cj;
                float nV = pData[nPos];
                if (nodata == nV){
                    continue;
                }
                if(fvArray[nPos]!=2) continue;
                if(nV==cV){
                    pointArray.Add(Point2D<int>(cj,ci));
                    fvArray.SetValue(nPos,3);
                }
            }
        }
        imageLumpBlock.UpdateImageBlock(pointArray);
        imageLumpBlock2.UpdateImageBlock(pointArray);
        float innerHeight;
        if(!ComputeUpPond(imageLumpBlock,imageLumpBlock2,oldData,2,innerHeight)){
            for (int k = 0; k < pointArray.GetSize(); k++){
                Point2D<int>point = pointArray.GetItem(k);
                fvArray.SetValue(point.X+point.Y*Cols, 2);
            }
            continue;
        }
        DigByPouringLine(pointArray,fvArray,pData,oldData,sixtArray,Cols,Rows,innerHeight,imageLumpBlock,imageLumpBlock2);
        if(DetectExitSignal(callBack,signal)){
            return nullptr;
        }
        CallBackHelper::SendProgress(callBack,(FLOAT)((sinkNum-k)*100.0/sinkNum));
    }
    newBand->SaveBlockData(0,0,Cols,Rows,oldData);
    CallBackHelper::SendMessage(callBack,"洼地下挖完成");
    newBand->FlushCache();
    return newBand;
}

AutoPtr<RasterBand>HydroLakePouringLineDig::Execute(RasterBand*demBand,int lakeMinRadius,bool breachingHeader,int iters,RasterTarget*target,CallBack*callBack,Semaphora*signal){
    if(!CheckValid()) return nullptr;
    AutoPtr<RasterBand>dem=AutoPtr<RasterBand>(demBand,true);
    AutoPtr<HydroDemFill>hydro_fill(new HydroDemFill());
    hydro_fill->SetEnvironment(pEnvi);
    AutoPtr<HydroFlowDirection>hydro_flow(new HydroFlowDirection());
    hydro_flow->SetEnvironment(pEnvi);
    AutoPtr<HydroFlowAccumulation>hydro_accu(new HydroFlowAccumulation());
    hydro_accu->SetEnvironment(pEnvi);
    AutoPtr<MemRasterTarget>mTarget(new MemRasterTarget());
    AutoPtr<RasterBand>resultBand=nullptr;
    for(int k=0;k<iters;k++){
        CallBackHelper::SendMessage(callBack,"第"+to_string(k+1)+"次下挖");
        AutoPtr<RasterBand>fill_dem=hydro_fill->Execute(dem.get(),mTarget.get(),callBack,signal);
        if(fill_dem==nullptr) return nullptr;
        AutoPtr<RasterBand>flowdir=hydro_flow->Execute(dem.get(),fill_dem.get(),mTarget.get(),callBack,signal);
        if(flowdir==nullptr) return nullptr;
        AutoPtr<RasterBand>accu=hydro_accu->Execute(flowdir.get(),nullptr,mTarget.get(),callBack,signal);
        if(accu==nullptr) return nullptr;
        AutoPtr<RasterBand>outputBand;
        if(k==iters-1)
            resultBand=innerExecute(dem.get(),fill_dem.get(),flowdir.get(),accu.get(),lakeMinRadius,breachingHeader,target,callBack,signal);
        else{
            outputBand=innerExecute(dem.get(),fill_dem.get(),flowdir.get(),accu.get(),lakeMinRadius,breachingHeader,mTarget.get(),callBack,signal);
            dem=outputBand;
        }
        CallBackHelper::SendMessage(callBack,"已完成第"+to_string(k+1)+"次下挖,还剩"+to_string(iters-k-1)+"次");
    }
    return resultBand;
}

}


