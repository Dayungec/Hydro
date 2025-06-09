#ifndef FILERASTERDATASET_H
#define FILERASTERDATASET_H
#include "dataset.h"
class GDALRasterBand;
class GDALDataset;
namespace SGIS{

class FileRasterBand;
class FileRasterDataset;
class FileRasterWorkspace;
class FileRasterWorkspaceFactory;

enum RasterCreateFileType
{
    rcftTiff	= 0,
    rcftErdasImagine	= 0x1,
    rcftPCIPix	= 0x2,
    rcftEnviHdr	= 0x3
};

class GDALRasterDataset
{
public:
    GDALRasterDataset();
    virtual~GDALRasterDataset();
    void Dispose();
public:
    GDALDataset  *poDataset;
};

class SGIS_EXPORT FileRasterBand:
        public RasterBand
{
protected:
    FileRasterBand();
public:
    virtual~FileRasterBand();
    string GetDescription();
    AutoPtr<RasterDataset>GetRasterDataset();
    float GetPixelValue(int x,int y);
    bool GetBlockData(int x,int y,int width,int height,int bufferWidth,int bufferHeight,float*data);
    bool SaveBlockData(int x,int y,int width,int height,float*data);
    AutoPtr<ColorTable>GetColorTable();
    bool IsFromSameSource(RasterBand*other);
    void Dispose();
    void FlushCache();
    friend class FileRasterDataset;
protected:
    AutoPtr<GDALRasterDataset>pDataset;
    AutoPtr<FileRasterDataset>father;
    GDALRasterBand *poBand;
};

class SGIS_EXPORT FileRasterDataset:
        public RasterDataset,
        public FileDataset,
        public BufferWritable,
        public JsonWritable
{
public:
    FileRasterDataset();
    virtual~FileRasterDataset();
    vector<string>GetSubDatasets();
    AutoPtr<RasterDataset>GetSubDataset(string subdataset);
//RasterDataset:
    string GetDescription();
    AutoPtr<Workspace>GetWorkspace();
    string GetName();
    bool TemporaryDataset();
    RasterDesp GetRasterDesp();
    void SetLeftTop(double left,double top);
    void SetCellSize(double xCellSize,double yCellSize);
    AutoPtr<SpatialReference>GetSpatialReference();
    bool SetSpatialReference(AutoPtr<SpatialReference>psp);
    int GetBandCount();
    AutoPtr<RasterBand>GetRasterBand(int nIndex);
    vector<string>GetMetaData(string KeyWord);
    bool IsValid();
    void Dispose();
    AutoPtr<RasterDataset>Clone();
//FileDataset:
    bool IsDirectory();
    string GetPathName();
    bool OpenFromFile(string pathName);
//FileRasterDataset:
    bool OpenFromFile(string pathName,bool Update);
    bool IsReadOnly();
//BufferWritable:
    bool Read(BufferReader*br);
    void Write(BufferWriter*bw);
    string GetTypeName();
//JsonWritable:
    bool ReadFromJson(JsonReader*jr,JsonNode node);
    string ToJson();
protected:
    string pathName;
    bool readOnly;
    RasterDesp rasterDesp;
    AutoPtr<GDALRasterDataset>pDataset;
    AutoPtr<SpatialReference>psp;
    int bandCount;
};


class SGIS_EXPORT FileRasterWorkspace:
        public RasterWorkspace,
        public FileWorkspace
{
public:
   FileRasterWorkspace(string pathName);
   virtual~FileRasterWorkspace();
//RasterWorkspace:
   string GetDescription();
//FileWorkspace
   string GetPathName();
   bool IsDirectory();
   vector<string>GetDirectoryDatasetNames();
   vector<string>GetDatasetNames();
   AutoPtr<FileDataset>OpenDataset(string name);
   bool CanDeleteDataset(string name);
   bool DeleteDataset(string name);
   bool CanRenameDataset(string name);
   bool RenameDataset(string oldName,string newName);
//FileRasterWorkspace
   AutoPtr<FileRasterDataset>OpenRasterDataset(string name,bool Update=false);
   AutoPtr<FileRasterDataset>CreateRasterDataset(string Name,RasterDesp rasterDesp,LONG BandCount,RasterDataType dataType,RasterCreateFileType fileType,DOUBLE NoData,SpatialReference*psp);
protected:
   void GetGridDatasetNames(vector<string>&Names);
protected:
   string pathName;
};

class SGIS_EXPORT FileRasterWorkspaceFactory:
        public RasterWorkspaceFactory,
        public FileWorkspaceFactory
{
public:
    FileRasterWorkspaceFactory();
    virtual~FileRasterWorkspaceFactory();
//RasterWorkspaceFactory:
    string GetDescription();
    AutoPtr<WorkspaceFactory>Clone();
//FileWorkspaceFactory:
    vector<string>GetWorkspaceNames(string dir);
    bool IsWorkspaceDirectory();
    AutoPtr<FileWorkspace>OpenFromFile(string pathName);
};

}
#endif // FILERASTERDATASET_H
