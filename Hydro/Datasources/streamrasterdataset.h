#ifndef STREAMRASTERDATASET_H
#define STREAMRASTERDATASET_H
#include "Base/autoptr.h"
#include "dataset.h"
#include "Base/guid.h"
class GDALDataset;
class GDALRasterBand;
namespace SGIS{
class StreamRasterBand;
class StreamRasterDataset;
class StreamRasterWorkspace;
class StreamRasterWorkspaceFactory;

class InnerStreamRasterDataset
{
public:
    InnerStreamRasterDataset();
    virtual~InnerStreamRasterDataset();
    bool AttachMemData(string name,BYTE*bts,int len,bool update);
    bool CopyMemData(string name,BYTE*bts,int len,bool update);
    void Dispose();
public:
    GDALDataset  *poDataset;
protected:
    BYTE*buffer;
    int dataLen;
    bool destroyData;
    string name;
    string szGUID;
};

class SGIS_EXPORT StreamRasterBand:
        public RasterBand
{
protected:
    StreamRasterBand();
public:
    virtual~StreamRasterBand();
    string GetDescription();
    AutoPtr<RasterDataset>GetRasterDataset();
    float GetPixelValue(int x,int y);
    bool GetBlockData(int x,int y,int width,int height,int bufferWidth,int bufferHeight,float*data);
    bool SaveBlockData(int x,int y,int width,int height,float*data);
    AutoPtr<ColorTable>GetColorTable();
    bool IsFromSameSource(RasterBand*other);
    bool Save(string pathName);
    void Dispose();
    friend class StreamRasterDataset;
protected:
    bool ReadOnly;
    GDALRasterBand *poBand;
    AutoPtr<InnerStreamRasterDataset>pDataset;
    AutoPtr<StreamRasterDataset>father;
    AutoPtr<ColorTable>colorTable;
};

class SGIS_EXPORT StreamRasterDataset:
        public RasterDataset
{
protected:
    StreamRasterDataset();
public:
    virtual~StreamRasterDataset();
//RasterDataset:
    string GetDescription();
    AutoPtr<Workspace>GetWorkspace();
    string GetName();
    string GetTypeName();
    bool TemporaryDataset();
    RasterDesp GetRasterDesp();
    void SetLeftTop(double left,double top);
    void SetCellSize(double xCellSize,double yCellSize);
    AutoPtr<SpatialReference>GetSpatialReference();
    bool SetSpatialReference(AutoPtr<SpatialReference>psp);
    int GetBandCount();
    AutoPtr<RasterBand>GetRasterBand(int nIndex);
    bool Save(string pathName);
    bool IsValid();
    void Dispose();
    AutoPtr<RasterDataset>Clone();
public:
    bool AttachImageData(string Name,BYTE* memImage,LONG datalen,bool Update);
    bool CopyImageData(string Name,BYTE* memImage,LONG datalen,bool Update);
protected:
    friend class StreamRasterWorkspace;
    string name;
    bool ReadOnly;
    RasterDesp rasterDesp;
    AutoPtr<SpatialReference>psp;
    AutoPtr<InnerStreamRasterDataset>pDataset;
    int bandCount;
};


class SGIS_EXPORT StreamRasterWorkspace:
        public RasterWorkspace
{
public:
   StreamRasterWorkspace();
   virtual~StreamRasterWorkspace();
//RasterWorkspace:
   string GetDescription();
//StreamRasterWorkspace
   AutoPtr<StreamRasterDataset>CreateRasterDataset(string Name,BYTE*bits,LONG len,bool Update);
};

class SGIS_EXPORT StreamRasterWorkspaceFactory:
        public RasterWorkspaceFactory
{
public:
    StreamRasterWorkspaceFactory();
    virtual~StreamRasterWorkspaceFactory();
//RasterWorkspaceFactory:
    string GetDescription();
    AutoPtr<WorkspaceFactory>Clone();
//StreamRasterWorkspaceFactory:
    AutoPtr<StreamRasterWorkspace>CreateStreamRasterWorkspace();
};
}
#endif // STREAMRASTERDATASET_H
