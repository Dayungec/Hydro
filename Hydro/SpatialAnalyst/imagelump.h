#ifndef IMAGELUMP_H
#define IMAGELUMP_H

#define INT_NO_DATA (-2147483647 - 1)
#include "Base/autoptr.h"
#include "rastertofeature.h"
#include "rastertarget.h"

namespace SGIS{

class SGIS_EXPORT Erode
{
public:
    Erode(BYTE*img, LONG imgWidth, LONG imgHeight, LONG imgCode,bool excludeBorder=true);
    virtual~Erode();
    void Compute(BYTE*output,LONG times);
    void Compute(LONG times);
    bool ComputeOneTime();
protected:
    BYTE*img;
    LONG imgWidth;
    LONG imgHeight;
    LONG imgCode;
    bool excludeBorder;
};

class SGIS_EXPORT Expode
{
public:
    Expode(BYTE*img, LONG imgWidth, LONG imgHeight, LONG imgCode, bool excludeBorder = true);
    virtual~Expode();
    void Compute(BYTE*output, LONG times,BYTE outerCode=1);
    void Compute(LONG times,BYTE outerCode=1);
protected:
    BYTE*img;
    LONG imgWidth;
    LONG imgHeight;
    LONG imgCode;
    bool excludeBorder;
};

class SGIS_EXPORT InnerExpode
{
public:
    InnerExpode(BYTE*img, LONG imgWidth, LONG imgHeight, LONG imgCode);
    virtual~InnerExpode();
    void Compute(BYTE*output);
    void Compute();
protected:
    BYTE*img;
    LONG imgWidth;
    LONG imgHeight;
    LONG imgCode;
};

class SGIS_EXPORT ImageBorder{
public:
    ImageBorder(BYTE*img, LONG imgWidth, LONG imgHeight, LONG imgCode);
    virtual~ImageBorder();
    void Compute(BYTE*output);
    void Compute();
protected:
    BYTE*img;
    LONG imgWidth;
    LONG imgHeight;
    LONG imgCode;
};

class SGIS_EXPORT Thin
{
public:
    Thin(BYTE*img,LONG imgWidth,LONG imgHeight,LONG imgCode, bool excludeBorder = true);
    virtual~Thin();
    void Compute(BYTE*output);
    void Compute(BYTE*output,BYTE*mask);
    void ComputeWithReservedPoints(BYTE*output,vector<Point2D<int>>&reservedPoints);
    void Compute();
    void ComputeWithMask(BYTE*mask);
    void ComputeWithReservedPoints(vector<Point2D<int>>&reservedPoints);
protected:
    bool IsSpecialOfThin(BYTE*output, int x, int y);
    bool CanThined(BYTE*output, int x, int y);
    void ThinArray(BYTE*output);
    void ThinArray(BYTE*output,BYTE*mask);
    void ThinArray(BYTE*output,vector<Point2D<int>>&reservedPoints);
protected:
    int bi[4];
    int around[8];
    BYTE*img;
    LONG imgWidth;
    LONG imgHeight;
    LONG imgCode;
    bool excludeBorder;
};

class SGIS_EXPORT ImageLump {
public:
    ImageLump();
    virtual~ImageLump();
    void AllocStack(LONG cols, LONG rows);
    void EmptyStack();
    LONG*GetStack();
    bool PushBack(int x, int y);
    bool PushBack(LONG pos);
    LONG PopUp();
    void Reset();
    bool IntersectBorder();
    LONG GetCount();
    LONG GetLeft();
    LONG GetTop();
    LONG GetLumpWidth();
    LONG GetLumpHeight();
    LONG GetCols();
    LONG GetRows();
    void Expand(int w);
    double GetDensity();
    LONG GetPixel(int nIndex);
    AutoPtr<ImageLump>CreateCompactLump();
protected:
    LONG*stack;
    int stackCount;
    int minx;
    int miny;
    int maxx;
    int maxy;
    int cols;
    int rows;
    int pin;
};

class SGIS_EXPORT ImageLumpBlock {
public:
    ImageLumpBlock();
    virtual~ImageLumpBlock();
    bool CreateImageBlock(int cols,int rows);
    bool CreateImageBlock(ImageLump*lump);
    bool UpdateImageBlock(AutoArray<Point2D<int>>&pointArray);
    bool UpdateImageBlock(vector<Point2D<int>>&pointArray);
    bool CopyFrom(ImageLumpBlock*other);
    int GetMaxValue();
    bool MakeForeGroundOne();
    void ExpodeOnly(int times,bool labelDiffer);
    void ErodeAndExpand(int times);
    void ExpandAndErode(int times);
    void ThinImage();
    void ThinImage(BYTE*mask);
    void ThinImage(vector<Point2D<int>>&reservedPoints);
    void ThinImage(float*dems,float minHeight,vector<Point2D<int>>&reservedPoints);
    void ExtractBorder();
    void UpdateImageLump(ImageLump*lump);
    BYTE*GetBuffer();
    LONG GetLeft();
    LONG GetTop();
    LONG GetLumpWidth();
    LONG GetLumpHeight();
    LONG GetCols();
    LONG GetRows();
    AutoPtr<RasterDataset>CreateRasterDataset(RasterTarget*target);
protected:
    BYTE*buffer;
    int cols;
    int rows;
    int lumpWidth;
    int lumpHeight;
    int left;
    int top;
};

class SGIS_EXPORT ImageLumpPolygon {
public:
    ImageLumpPolygon();
    virtual~ImageLumpPolygon();
    vector<AutoPtr<Geometry>> ExtractPolygons(ImageLump*lump, DOUBLE Left, DOUBLE Top, DOUBLE xCellSize,DOUBLE yCellSize, LONG maxIslands);
    AutoPtr<FeatureClass>CreatePolygonFeatureClass(FeatureClassTarget*target);
protected:
    void InitialBorder();
    int GetStateOfBorderPixel(int x, int y);
    void GetNextPoint(int Deri, int&Posi, int&x, int&y, int ri);
    bool PursueRing(int x, int y);
    void RegisterRings();
    void RegisterPolygons();
    AutoPtr<Polygon>get_Polygon(LONG index);
protected:
    ImageLump*lump;
    ImageLumpBlock imageLumpBlock;
    BYTE*data;
    LONG Cols, Rows;
    DOUBLE Left; DOUBLE Top;
    DOUBLE xCellSize;
    DOUBLE yCellSize;
    BYTE*b0;
    BYTE*b1;
    LONG imageCols, imageRows;
    vector<RPolygon*>polys;
    vector<RRing*>rings;
};

class SGIS_EXPORT ImageLumpCircle
{
public:
    ImageLumpCircle(int r);
    virtual~ImageLumpCircle();
    void CreateCircle();
    void BuildCircle(ImageLumpBlock*lumpBlock, int x, int y);
    template<typename T>
    void BuildCircle(T*buffer,int cols,int rows,int x,int y){
        int lumpw= cols;
        int lumph = rows;
        int cpCount = GetPixelCount();
        LONG pos;
        int px, py;
        bool state = false;
        BeginCircle();
        for (int k = 0; k < cpCount; k++)
        {
            px = poses[k].X + x;
            py = poses[k].Y + y;
            if ((px < 0) || (px >= lumpw) || (py < 0) || (py >= lumph))
                state = false;
            else
            {
                pos = px + py * lumpw;
                state = (buffer[pos]==1);
            }
            SetNextCirclePixel(state);
        }
        EndCircle();
    };
    template<typename T>
    void BuildCircle(T*buffer,int cols,int rows,int x,int y,T fromV,T toV,float nodata){
        int lumpw= cols;
        int lumph = rows;
        int cpCount = GetPixelCount();
        LONG pos;
        int px, py;
        bool state = false;
        BeginCircle();
        for (int k = 0; k < cpCount; k++)
        {
            px = poses[k].X + x;
            py = poses[k].Y + y;
            if ((px < 0) || (px >= lumpw) || (py < 0) || (py >= lumph))
                state = false;
            else
            {
                pos = px + py * lumpw;
                if((float)buffer[pos]==nodata)
                    state=false;
                else
                    state = ((buffer[pos]>=fromV)&&(buffer[pos]<=toV));
            }
            SetNextCirclePixel(state);
        }
        EndCircle();
    };
    template<typename T>
    T GetBestThrash(T*buffer,int cols,int rows,int x,int y,float percentage,float nodata){
        int lumpw= cols;
        int lumph = rows;
        int cpCount = GetPixelCount();
        int px, py;
        T delta;
        T maxdelta=0;
        T centerValue=buffer[x+y*cols];
        if((FLOAT)centerValue==nodata) return nodata;
        int pos;
        for (int k = 0; k < cpCount; k++)
        {
            px = poses[k].X + x;
            py = poses[k].Y + y;
            if ((px < 0) || (px >= lumpw) || (py < 0) || (py >= lumph)) continue;
            pos = px + py * lumpw;
            delta=fabs(centerValue-buffer[pos]);
            if(delta>maxdelta) maxdelta=delta;
        }
        int difNum=20;
        vector<int>counts;
        counts.resize(difNum);
        T ldif=maxdelta/difNum;
        int idelta;
        for (int k = 0; k < cpCount; k++)
        {
            px = poses[k].X + x;
            py = poses[k].Y + y;
            if ((px < 0) || (px >= lumpw) || (py < 0) || (py >= lumph)) continue;
            pos = px + py * lumpw;
            delta=fabs(centerValue-buffer[pos]);
            idelta=delta/ldif;
            if(idelta>=difNum) idelta=difNum-1;
            counts[idelta]++;
        }
        int totalNum=0;
        int numth=percentage*cpCount;
        int bestIndex=0;
        for(int k=0;k<difNum;k++){
            totalNum+=counts[k];
            if(totalNum>numth) break;
            bestIndex=k;
        }
        return ldif*(bestIndex+1);
    };
    int GetRadius();
    void RemoveIntersects(double maxanglerange);
    //获取圆上的像素点总数
    int GetPixelCount();
    //获取圆上的像素点
    Point2D<int> GetPixelPoint(int nIndex);
    //获取圆上的交点数量
    int GetIntersectNum();
    //获取圆上的交点的起始角度
    double GetIntersectFromAngle(int nIndex);
    //获取圆上的交点的终止角度
    double GetIntersectToAngle(int nIndex);
    //获取圆上的交点的角度范围
    double GetIntersectAngleRange(int nIndex);
    //获取圆上的交点的中心角度
    double GetIntersectAngle(int nIndex);
    //获取圆上的交点是否包含指定角度
    bool IsIntersectIn(int nIndex, double ang);
    //获取圆是否在线段内
    bool IsCircleInner();
    //获取圆上的交点的中心坐标
    Point2D<int> GetCenterPoint(int nIndex);
    //获取圆上的交点的左边缘坐标
    Point2D<int> GetLeftPoint(int nIndex);
    //获取圆上的交点的右边缘坐标
    Point2D<int> GetRightPoint(int nIndex);
    double GetTotalInnerAngleRange();
    double GetMaxInnerAngleRange();
protected:
    int round(double x);
    void BeginCircle();
    void SetNextCirclePixel(bool s);
    void EndCircle();
public:
    vector<Point2D<int>>poses;
    vector<double>angs;
    int radius;
    bool state;
    int pixelIndex;
    vector<int>breakPos;
    bool allIn;
};

class SGIS_EXPORT CircleLineSwitch{
public:
    CircleLineSwitch(double ang,double angrange,int radius);
    virtual~CircleLineSwitch();
    void AddChild(AutoPtr<CircleLineSwitch>child);
    double GetAngDif(double ang,double angrange,double angtoler);
    int GetLevels();
    double GetAngle();
    double GetAngleRange();
    int GetRadius();
    AutoPtr<CircleLineSwitch>GetChild();
    CircleLineSwitch*GetFather();
    CircleLineSwitch*GetLeafNode();
protected:
    CircleLineSwitch*father;
    AutoPtr<CircleLineSwitch>child;
    double ang;
    double angrange;
    int radius;
    CircleLineSwitch*leafNode;
};

class SGIS_EXPORT CircleLineSwitches{
public:
    CircleLineSwitches();
    virtual~CircleLineSwitches();
    void Add(int switchIndex,double ang,double angrange,int radius);
    int GetNearest(double ang,double angrange,int radius,double angtoler,double&angdif);
    int GetSwitchNum(int minlevels);
    int GetSize();
    void Remove(int nIndex);
    AutoPtr<CircleLineSwitch>GetItem(int nIndex);
protected:
    vector<AutoPtr<CircleLineSwitch>>lses;
};

class SGIS_EXPORT ImageLumpCircles
{
public:
    ImageLumpCircles(int fromRadius=4, int circleCount=8);
    virtual~ImageLumpCircles();
    void CreateCircles();
    int GetCircleCount();
    int GetFromRadius();
    AutoPtr<ImageLumpCircle>GetCircle(int nIndex);
    void BuildCircles(ImageLumpBlock*lumpBlock, int x, int y);
    template<typename T>
    void BuildCircles(T*buffer,int cols,int rows,int x,int y){
        for (int k = 0; k < cps.size(); k++) {
            AutoPtr<ImageLumpCircle>cp = cps[k];
            cp->BuildCircle<T>(buffer,cols,rows, x, y);
        }
    };
    template<typename T>
    void BuildCircle(T*buffer,int cols,int rows,int x,int y,T fromV,T toV,float nodata){
        for (int k = 0; k < cps.size(); k++) {
            AutoPtr<ImageLumpCircle>cp = cps[k];
            cp->BuildCircle<T>(buffer,cols,rows, x, y,fromV,toV,nodata);
        }
    };
    AutoPtr<CircleLineSwitches>GetLineSwitches(int maxLineWidthInPixels=10);
    template<typename T>
    void ValidLineSwitches(T*buffer,int cols,int rows,int x,int y,AutoPtr<CircleLineSwitches>lses,int fromRadius,int minLevels){
        for(int k=lses->GetSize()-1;k>=0;k--){
            AutoPtr<CircleLineSwitch>ls=lses->GetItem(k);
            if(ls->GetLevels()<minLevels){
                lses->Remove(k);
                continue;
            }
            CircleLineSwitch*leafNode=ls->GetLeafNode();
            if(leafNode->GetRadius()>fromRadius+2){
                lses->Remove(k);
                continue;
            }

            double sina=sin(leafNode->GetAngle());
            double cosa=cos(leafNode->GetAngle());
            int radius=leafNode->GetRadius();
            int bc=0;
            for(int r=radius-1;r>=1;r--){
                int ix=x+cosa*r;
                int iy=y+sina*r;
                if(buffer[ix+iy*cols]==1) bc++;
            }
            double per=bc*1.0/(radius-1);
            if(per<0.95){
                lses->Remove(k);
                continue;
            }

        }
    };
    template<typename T>
    void ValidLineSwitches(T*buffer,int cols,int rows,int x,int y,T fromV,T toV,AutoPtr<CircleLineSwitches>lses,int fromRadius,int minLevels){
        for(int k=lses->GetSize()-1;k>=0;k--){
            AutoPtr<CircleLineSwitch>ls=lses->GetItem(k);
            if(ls->GetLevels()<minLevels){
                lses->Remove(k);
                continue;
            }
            CircleLineSwitch*leafNode=ls->GetLeafNode();
            double sina=sin(leafNode->GetAngle());
            double cosa=cos(leafNode->GetAngle());
            int radius=leafNode->GetRadius();
            int bc=0;
            for(int r=radius-1;r>=1;r--){
                int ix=x+cosa*r;
                int iy=y+sina*r;
                if((buffer[ix+iy*cols]>=fromV)&&(buffer[ix+iy*cols]<=toV)) bc++;
            }
            double per=bc*1.0/(radius-1);
            if(per<0.95){
                lses->Remove(k);
                continue;
            }
        }
    };
protected:
    vector<AutoPtr<ImageLumpCircle>>cps;
    int fromRadius;
};

};
#endif // IMAGELUMP_H
