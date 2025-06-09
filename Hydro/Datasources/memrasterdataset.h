#ifndef MEMRASTERDATASET_H
#define MEMRASTERDATASET_H
#include "Base/autoptr.h"
#include "dataset.h"
#include "memdataarray.h"

namespace SGIS{
class MemDataArray;
class MemRasterBand;
class MemRasterDataset;
class MemRasterWorkspace;
class MemRasterWorkspaceFactory;

class SGIS_EXPORT MemRasterBand:
        public RasterBand
{
protected:
    MemRasterBand();
public:
    virtual~MemRasterBand();
    string GetDescription();
    AutoPtr<RasterDataset>GetRasterDataset();
    float GetPixelValue(int x,int y);
    bool GetBlockData(int x,int y,int width,int height,int bufferWidth,int bufferHeight,float*data);
    bool SaveBlockData(int x,int y,int width,int height,float*data);
    AutoPtr<ColorTable>GetColorTable();
    bool IsFromSameSource(RasterBand*other);
    void Dispose();
    bool Save(string pathName);
    AutoPtr<FixBuffer>SaveAsBuffer(string exa);
    friend class MemRasterDataset;
public:
    void SetColorTable(AutoPtr<ColorTable>ct);
    AutoPtr<MemDataArray>GetRawData();
protected:
    bool innerGetBlockData(LONG x1,LONG y1,LONG Width,LONG Height,LONG buffx,LONG buffy,float*data);
protected:
    AutoPtr<MemRasterDataset>father;
    AutoPtr<MemDataArray>bandData;
    AutoPtr<ColorTable>colorTable;
};

class SGIS_EXPORT MemRasterDataset:
        public RasterDataset
{
protected:
    MemRasterDataset();
public:
    virtual~MemRasterDataset();
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
    bool IsValid();
    bool Save(string pathName);
    AutoPtr<FixBuffer>SaveAsBuffer(string exa);
    void Dispose();
    AutoPtr<RasterDataset>Clone();
protected:
    friend class MemRasterBand;
    friend class MemRasterWorkspace;
    string name;
    RasterDesp rasterDesp;
    AutoPtr<SpatialReference>psp;
    vector<AutoPtr<MemDataArray>>bands;
    vector<AutoPtr<ColorTable>>colorTables;
    double nodata;
};


class SGIS_EXPORT MemRasterWorkspace:
        public RasterWorkspace
{
public:
   MemRasterWorkspace();
   virtual~MemRasterWorkspace();
//RasterWorkspace:
   string GetDescription();
//MemRasterWorkspace
   AutoPtr<MemRasterDataset>CreateRasterDataset(string Name,RasterDesp rasterDesp,LONG BandCount,RasterDataType dataType,DOUBLE NoData,SpatialReference*psp);
};

class SGIS_EXPORT MemRasterWorkspaceFactory:
        public RasterWorkspaceFactory
{
public:
    MemRasterWorkspaceFactory();
    virtual~MemRasterWorkspaceFactory();
//RasterWorkspaceFactory:
    string GetDescription();
    AutoPtr<WorkspaceFactory>Clone();
//MemRasterWorkspaceFactory:
    AutoPtr<MemRasterWorkspace>CreateMemRasterWorkspace();
};

}
#endif // MEMRASTERDATASET_H
