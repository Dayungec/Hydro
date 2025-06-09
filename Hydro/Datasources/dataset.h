#ifndef DATASET_H
#define DATASET_H

#include "Base/autoptr.h"
#include "Base/classfactory.h"
#include "Base/jsonio.h"
#include  "Base/bufferio.h"
#include  "Base/CallBack.h"
#include "colortable.h"
#include "tabledesc.h"
#include "Base/variant.h"
#include "selectionset.h"
#include "display.h"
#include "histogram.h"
#include "queryfilter.h"
#include "memdataarray.h"
#include "Geometry/geometrybordercapture.h"
#include "featurecursor.h"
#include "statishistory.h"
#include "mapprojection.h"

namespace SGIS{

class WorkspaceFactory;
class FileWorkspaceFactory;
class FeatureWorkspaceFactory;
class RasterWorkspaceFactory;
class Workspace;
class FileWorkspace;
class FeatureWorkspace;
class RasterWorkspace;
class Dataset;
class FileDataset;
class FeatureDataset;
class RasterDataset;
class RasterBand;
class FeatureClass;

struct RasterDesp{
    int cols;
    int rows;
    double xCellSize;
    double yCellSize;
    double left;
    double top;
    RasterDesp(){
        cols=rows=0;
        xCellSize=yCellSize=0;
        left=top=0;
    };
    RasterDesp(double left,double top,int cols,int rows,double xCellSize,double yCellSize){
        this->left=left;
        this->top=top;
        this->cols=cols;
        this->rows=rows;
        this->xCellSize=xCellSize;
        this->yCellSize=yCellSize;
    };
    void Reset(){
        cols=rows=0;
        xCellSize=yCellSize=0;
        left=top=0;
    };
    RasterDesp(const RasterDesp&other){
        this->left=other.left;
        this->top=other.top;
        this->cols=other.cols;
        this->rows=other.rows;
        this->xCellSize=other.xCellSize;
        this->yCellSize=other.yCellSize;
    };
    double GetRight(){
        return left+xCellSize*cols;
    };
    double GetBottom(){
        return top-yCellSize*rows;
    };
    double GetCenterX(){
        return left+xCellSize*cols/2;
    };
    double GetCenterY(){
        return top-yCellSize*rows/2;
    };
    AutoPtr<Envelope>Extent(){
        return new Envelope(left,top,left+xCellSize*cols,top-yCellSize*rows);
    };
    void MapRectToPixel(Envelope*env,bool normalize=false){
        double l,t,r,b;
        env->GetCoord(&l,&t,&r,&b);
        env->PutCoord((l-left)/xCellSize,(top-t)/yCellSize,(r-left)/xCellSize,(top-b)/yCellSize);
        if(normalize){
            env->GetCoord(&l,&t,&r,&b);
            if(l<0) l=0;
            if(r>=cols) r=cols-1;
            if(t<0) t=0;
            if(b>=rows) b=rows-1;
            env->PutCoord(l,t,r,b);
        }
    };
    void PixelRectToMap(Envelope*env,bool parseInt=true){
        double l,t,r,b;
        env->GetCoord(&l,&t,&r,&b);
        if(parseInt){
           l=(int)l;
           t=(int)t;
           r=(int)r;
           b=(int)b;
           env->PutCoord(left+l*xCellSize,top-t*yCellSize,left+(r+1)*xCellSize,top-(b+1)*yCellSize);
        }
        else{
            env->PutCoord(left+l*xCellSize,top-t*yCellSize,left+r*xCellSize,top-b*yCellSize);
        }
    };
    bool IsSameAs(const RasterDesp&other){
        if(cols!=other.cols) return false;
        if(rows!=other.rows) return false;
        if(xCellSize!=other.xCellSize) return false;
        if(yCellSize!=other.yCellSize) return false;
        if(left!=other.left) return false;
        if(top!=other.top) return false;
        return true;
    };
};

enum VectorShapeType
{
    vstUnknown	= 0,
    vstPoint	= 0x1,
    vstPolyline	= 0x2,
    vstPolygon	= 0x3,
    vstCollection	= 0x4
};

enum SelectionSelMode
{
    ssmNew	= 0,
    ssmAdd	= 0x1,
    ssmRemove	= 0x2
};

class SGIS_EXPORT WorkspaceFactory{
public:
    WorkspaceFactory();
    virtual~WorkspaceFactory();
    virtual string GetDescription()=0;
    virtual AutoPtr<WorkspaceFactory>Clone()=0;
};

class SGIS_EXPORT FileWorkspaceFactory
{
public:
    FileWorkspaceFactory();
    virtual~FileWorkspaceFactory();
    virtual vector<string>GetWorkspaceNames(string dir)=0;
    virtual bool IsWorkspaceDirectory()=0;
    virtual AutoPtr<FileWorkspace>OpenFromFile(string pathName)=0;
};

class SGIS_EXPORT FeatureWorkspaceFactory:
        public WorkspaceFactory{
public:
    FeatureWorkspaceFactory();
    virtual~FeatureWorkspaceFactory();
};

class SGIS_EXPORT RasterWorkspaceFactory:
        public WorkspaceFactory{
public:
    RasterWorkspaceFactory();
    virtual~RasterWorkspaceFactory();
};

class SGIS_EXPORT Workspace{
public:
    Workspace();
    virtual~Workspace();
    virtual string GetDescription()=0;
};

class SGIS_EXPORT FileWorkspace
{
public:
    FileWorkspace();
    virtual~FileWorkspace();
    virtual vector<string>GetDirectoryDatasetNames();
    virtual string GetPathName()=0;
    virtual bool IsDirectory()=0;
    virtual vector<string>GetDatasetNames()=0;
    virtual AutoPtr<FileDataset>OpenDataset(string name)=0;
    virtual bool CanDeleteDataset(string name)=0;
    virtual bool DeleteDataset(string name)=0;
    virtual bool CanRenameDataset(string name)=0;
    virtual bool RenameDataset(string oldName,string newName)=0;
};

class SGIS_EXPORT FeatureWorkspace:
        public Workspace{
public:
    FeatureWorkspace();
    virtual~FeatureWorkspace();
    virtual bool IsDatasetFeatureClass(string name)=0;
};
class SGIS_EXPORT RasterWorkspace:
        public Workspace{
public:
    RasterWorkspace();
    virtual~RasterWorkspace();
};

class SGIS_EXPORT Dataset
        :public Object
{
public:
    Dataset();
    virtual~Dataset();
    virtual string GetDescription()=0;
    virtual AutoPtr<Workspace>GetWorkspace()=0;
    virtual string GetName()=0;
    virtual bool TemporaryDataset()=0;
};

class SGIS_EXPORT FileDataset
{
public:
    FileDataset();
    virtual~FileDataset();
    virtual bool IsDirectory()=0;
    virtual string GetPathName()=0;
    virtual bool OpenFromFile(string pathName)=0;
};

class SGIS_EXPORT FeatureDataset:
        public Dataset{
public:
    FeatureDataset();
    virtual~FeatureDataset();
    virtual int GetFeatureClassCount()=0;
    virtual AutoPtr<FeatureClass>GetFeatureClass(int nIndex)=0;
    virtual AutoPtr<FeatureClass>GetDefaultFeatureClass()=0;
    virtual string GetFeatureClassName(int nIndex)=0;
    virtual int FindFeatureClass(string Name)=0;
    virtual void Dispose()=0;
};


class SGIS_EXPORT RasterBand{
public:
    RasterBand();
    virtual~RasterBand();
    string&Name();
    int GetBandIndex();
    RasterDesp GetRasterDesp();
    int GetCols();
    int GetRows();
    AutoPtr<SpatialReference>GetSpatialReference();
    void SetLeftTop(double left,double top);
    void SetCellSize(double xCellSize,double yCellSize);
    void SetSpatialReference(AutoPtr<SpatialReference>psp);
    double GetNodata();
    void SetNodata(double nodata);
    RasterDataType GetDataType();
    void ClearStatistics();
    bool IsNodata(double value);
    StatisHistory&GetStatisHistory();
    virtual bool Save(string pathName);
    bool SaveAsCompressedPng(string pathName);
    float GetPixelValue(double x,double y,AutoPtr<SpatialReference>psp);
    static AutoPtr<RasterBand>LoadFromCompressedPng(string pathName);
    virtual void FlushCache();
public:
    virtual string GetDescription()=0;
    virtual AutoPtr<RasterDataset>GetRasterDataset()=0;
    virtual float GetPixelValue(int x,int y)=0;
    virtual bool GetBlockData(int x,int y,int width,int height,int bufferWidth,int bufferHeight,float*data)=0;
    virtual bool SaveBlockData(int x,int y,int width,int height,float*data)=0;
    virtual AutoPtr<ColorTable>GetColorTable()=0;
    virtual bool IsFromSameSource(RasterBand*other)=0;
    virtual void Dispose()=0;
protected:
    string name;
    AutoPtr<SpatialReference>psp;
    StatisHistory pStaHist;
    RasterDesp rasterDesp;
    int bandIndex;
    double nodata;
    RasterDataType dataType;
};

class SGIS_EXPORT RasterDataset:
        public Dataset
{
public:
    RasterDataset();
    virtual~RasterDataset();
    virtual RasterDesp GetRasterDesp()=0;
    virtual void SetLeftTop(double left,double top)=0;
    virtual void SetCellSize(double xCellSize,double yCellSize)=0;
    virtual AutoPtr<SpatialReference>GetSpatialReference()=0;
    virtual bool SetSpatialReference(AutoPtr<SpatialReference>psp)=0;
    virtual int GetBandCount()=0;
    virtual AutoPtr<RasterBand>GetRasterBand(int nIndex)=0;
    virtual bool IsValid()=0;
    virtual void Dispose()=0;
    virtual AutoPtr<RasterDataset>Clone()=0;
    int GetCols();
    int GetRows();
};


class SGIS_EXPORT FeatureClassEdit{
public:
    FeatureClassEdit();
    virtual~FeatureClassEdit();
    virtual bool IsEditing()=0;
    virtual bool StartEditing()=0;
    virtual bool SaveEdit(CallBack*callBack=nullptr)=0;
    virtual bool Transaction(CallBack*callBack=nullptr)=0;
    virtual bool StopEdits(bool SaveEdit,CallBack*callBack=nullptr)=0;
    virtual bool SetFeature(LONG RecordIndex,AutoPtr<Geometry>geo)=0;
    virtual bool SetFieldValue(int RecordIndex,int FieldIndex,Variant newVal)=0;
    virtual bool SetFieldValueWithString(int RecordIndex,int FieldIndex,string newVal)=0;
    virtual bool AddFeature(AutoPtr<Geometry>geo)=0;
    virtual bool AddFeatureEx(AutoPtr<Geometry>geo,const vector<Variant>&values)=0;
    virtual bool DeleteFeature(int recordIndex)=0;
    virtual bool DeleteAllFeatures(int recordIndex)=0;
    virtual bool DeleteSelFeatures(CallBack*callBack)=0;
    virtual bool CalculateField(int TargetFieldIndex,string Expression,CallBack*callBack=nullptr)=0;
    virtual bool MoveSelection(DOUBLE difx,DOUBLE dify,CallBack*callBack=nullptr)=0;
    virtual bool MoveSelectionByClientCoord(DOUBLE difx,DOUBLE dify,Display*pDisplay,CallBack*callBack=nullptr)=0;
};

class SGIS_EXPORT FeatureClass{
public:
    FeatureClass();
    virtual~FeatureClass();
    string&Name();
    AutoPtr<TableDesc>GetTableDesc();
    VectorShapeType GetShapeType();
    AutoPtr<SelectionSet>GetSelectionSet();
    AutoPtr<SpatialReference>GetSpatialReference();
    bool Is3DFeatureClass();
    virtual bool SetSpatialReference(AutoPtr<SpatialReference>psp);
    StatisHistory&GetStatisHistory();
    void ClearStatistics();
    virtual void FlushCache();
    bool Output(string lyPath,SpatialReference*psp,bool onlySel);
    virtual bool AddField(string FieldName,VectorFieldType type,LONG FieldWidth,LONG DecimalCount,CallBack*callBack=nullptr)=0;
    virtual bool DeleteField(LONG FieldIndex,CallBack*callBack=nullptr)=0;
public:
    virtual string GetDescription()=0;
    virtual string GetFeatureClassName()=0;
    virtual AutoPtr<FeatureDataset>GetFeatureDataset()=0;
    virtual AutoPtr<Envelope>GetExtent()=0;
    virtual int GetFeatureCount()=0;
    virtual AutoPtr<Geometry>GetFeature(int recordIndex)=0;
    virtual bool GetFeatureEnvelope(int recordIndex,Envelope*env)=0;
    virtual int GetFeatureID(int recordIndex)=0;
    virtual Variant GetFieldValue(int recordIndex,int FieldIndex)=0;
    virtual double GetFieldValueAsDouble(int recordIndex,int FieldIndex)=0;
    virtual string GetFieldValueAsString(int recordIndex,int FieldIndex)=0;
    virtual bool IsFromSameSource(FeatureClass*other)=0;
    virtual bool IsValid()=0;
    virtual AutoPtr<FeatureClass>Clone()=0;
    virtual void Dispose()=0;
protected:
    string name;
    bool is3d;
    AutoPtr<TableDesc>pDesc;
    AutoPtr<SpatialReference>psp;
    AutoPtr<SelectionSet>pSelectionSet;
    VectorShapeType shapeType;
    StatisHistory pStaHist;
};


}

#endif // DATASET_H
