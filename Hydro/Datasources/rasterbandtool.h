#ifndef RASTERBANDTOOL_H
#define RASTERBANDTOOL_H
#include "Base/autoptr.h"
#include "dataset.h"
#include "Geometry/geometry2d.h"

namespace SGIS{

class SGIS_EXPORT RasterBandTool
{
public:
    RasterBandTool(RasterBand*pBand);
    virtual~RasterBandTool();
    void Attach(RasterBand*pBand);
    bool GetBlockData(LONG x,LONG y,LONG Width,LONG Height,LONG buffx,LONG buffy,float*data,bool bInterpolated);
    bool GetBlockDataByCoord(Point*leftTop,DOUBLE xCellSize,DOUBLE yCellSize,LONG Width, LONG Height,float*data,SpatialReference*psp,FLOAT nodata,bool bInterpolated);
    bool GetBlockDataByCoordEx(Point*leftTop,DOUBLE xCellSize,DOUBLE yCellSize,LONG Width, LONG Height,float*data,SpatialReference*psp,FLOAT nodata,FLOAT borderNodata,bool bInterpolated);
    bool GetBlockDataByCoord(const RasterDesp&desp,float*data,SpatialReference*psp,FLOAT nodata,bool bInterpolated);
    template<typename T>
    OrderArray<T>GetUniqueValues(LONG x,LONG y,LONG Width,LONG Height,LONG buffx,LONG buffy,LONG maxSize){
        OrderArray<T>uvs;
        if(pBand==nullptr) return uvs;
        float*data=new float[buffx*buffy];
        pBand->GetBlockData(x,y,Width,Height,buffx,buffy,data);
        float iNodata=NoData;
        float formerV=NoData;
        bool IsByte=false;
        RasterDataType type=pBand->GetDataType();
        if(type==rdtByte) IsByte=true;
        LONG Count=0;
        for(int j=buffx*buffy-1;j>=0;j--)
        {
            if(iNodata==data[j]) continue;
            if(formerV==data[j]) continue;
            formerV=data[j];
            LONG index=uvs.Add(data[j]);
            if(index>=0)
            {
                Count++;
                if((IsByte)&&(Count>255)) break;
                if((maxSize>0)&&(Count>=maxSize)) break;
            }
        }
        delete []data;
        return uvs;
    };
    template<typename T>
    OrderArray<int>GetUniqueValuesEx(LONG x,LONG y,LONG Width,LONG Height,LONG buffx,LONG buffy,LONG maxSize,IndexedArray<T>&excludeValues){
        OrderArray<T>uvs;
        if(pBand==nullptr) return uvs;
        float*data=new float[buffx*buffy];
        pBand->GetBlockData(x,y,Width,Height,buffx,buffy,data);
        float iNodata=NoData;
        float formerV=NoData;
        bool IsByte=false;
        RasterDataType type=pBand->GetDataType();
        if(type==rdtByte) IsByte=true;
        LONG Count=0;
        for(int j=buffx*buffy-1;j>=0;j--)
        {
            if(iNodata==data[j]) continue;
            if(formerV==data[j]) continue;
            if(excludeValues.FindValueIndex(data[j])) continue;
            formerV=data[j];
            LONG index=uvs.Add(data[j]);
            if(index>=0)
            {
                Count++;
                if((IsByte)&&(Count>255)) break;
                if((maxSize>0)&&(Count>=maxSize)) break;
            }
        }
        delete []data;
        return uvs;
    };
    AutoPtr<Histogram>ComputeHistogram(bool bApproxOK,DOUBLE minValue,DOUBLE maxValue,LONG ColumnsNum);
    AutoPtr<Histogram>ComputeHistogramEx(bool bApproxOK,DOUBLE minValue,DOUBLE maxValue,LONG ColumnsNum,IndexedArray<int>&excludeValues);
    bool ComputeStatistics(bool bApproxOK,DOUBLE*minValue,DOUBLE*maxValue,DOUBLE*meanValue,DOUBLE*stdValue);
    bool ComputeStatisticsEx(bool bApproxOK,DOUBLE*minValue,DOUBLE*maxValue,DOUBLE*meanValue,DOUBLE*stdValue,IndexedArray<int>&excludeValues);
    AutoPtr<Histogram>ComputeExtentHistogram(bool bApproxOK,Envelope*mapExtent,DOUBLE minValue,DOUBLE maxValue,LONG ColumnsNum);
    AutoPtr<Histogram>ComputeExtentHistogramEx(bool bApproxOK,Envelope*mapExtent,DOUBLE minValue,DOUBLE maxValue,LONG ColumnsNum,IndexedArray<int>&excludeValues);
    bool ComputeExtentStatistics(bool bApproxOK,Envelope*mapExtent,DOUBLE*minValue,DOUBLE*maxValue,DOUBLE*meanValue,DOUBLE*stdValue);
    bool ComputeExtentStatisticsEx(bool bApproxOK,Envelope*mapExtent,DOUBLE*minValue,DOUBLE*maxValue,DOUBLE*meanValue,DOUBLE*stdValue,IndexedArray<int>&excludeValues);
protected:
    Rect2D<double> MapToPixelCoord(Rect2D<double>&MapExtent);
    Rect2D<double> PixelToMapCoord(Rect2D<double>&PixelExtent);
    bool innerGetInterpolatedBlock(LONG x1,LONG y1,LONG x2,LONG y2,LONG BuffX,LONG BuffY,float* data);
    bool innerGetBlockDataByCoord(Point* LeftTop,DOUBLE xCellSize,DOUBLE yCellSize,LONG Width,LONG Height,FLOAT NoData,FLOAT borderNodata,SpatialReference*pSpatial,float* data);
    bool innerGetProjBlockDataByCoord(Point* LeftTop,DOUBLE xCellSize,DOUBLE yCellSize,LONG Width,LONG Height,CoordinateTransformation*pTrans,CoordinateTransformation*pTransRev,FLOAT NoData,float borderNodata,float* data);
    bool innerGetInterpolatedDataBlock(Point* LeftTop,DOUBLE xCellSize,DOUBLE yCellSize,int Width,int Height,float NoData,float borderNodata,SpatialReference*pSpatial,float* data);
    bool innerGetInterpolatedDataBlockWithProj(Point* LeftTop,DOUBLE xCellSize,DOUBLE yCellSize,int Width,int Height,CoordinateTransformation*pTrans,CoordinateTransformation*pTransRev,float NoData,float borderNodata,float* data);
protected:
    RasterBand*pBand;
    LONG rows;
    LONG cols;
    DOUBLE XCellSize;
    DOUBLE YCellSize;
    DOUBLE Left;
    DOUBLE Top;
    DOUBLE NoData;
    AutoPtr<SpatialReference>psp;
    AutoPtr<BiCoordinateTransformation>biTrans;
};

class SGIS_EXPORT RasterBandCacheBlocks{
public:
    RasterBandCacheBlocks(AutoPtr<RasterBand>rasterBand,int blockSize=512);
    virtual~RasterBandCacheBlocks();
    float GetPixel(int iX,int iY);
    AutoPtr<RasterBand>GetRasterBand();
protected:
    AutoPtr<RasterBand>rasterBand;
    int blockSize;
    RasterDesp desp;
    vector<AutoPtr<FixBuffer>>blocks;
    int blockXNum;
    int blockYNum;
};

}
#endif // RASTERBANDTOOL_H
