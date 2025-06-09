#include "rastertarget.h"
#include "Base/FilePath.h"
namespace SGIS{
RasterTarget::RasterTarget()
{

}

RasterTarget::~RasterTarget()
{

}

FileRasterTarget::FileRasterTarget(){
    fileType=rcftTiff;
}
FileRasterTarget::~FileRasterTarget(){

}
string&FileRasterTarget::PathName(){
    return pathName;
}

RasterCreateFileType&FileRasterTarget::FileType(){
    return fileType;
}

AutoPtr<RasterDataset>FileRasterTarget::CreateRasterDataset(SAEnvironment*envi,RasterDataType dType,int bandNumber,double nodata){
    AutoPtr<FileRasterWorkspaceFactory>pFac(new FileRasterWorkspaceFactory());
    string dir=FilePath::GetDir(pathName);
    string fileName=FilePath::GetFileName(pathName);
    AutoPtr<FileRasterWorkspace>pWork=pFac->OpenFromFile(dir);
    AutoPtr<SpatialReference>psp=envi->GetSpatialReference();
    AutoPtr<RasterDataset>newrd=pWork->CreateRasterDataset(fileName,envi->GetRasterDesp(), bandNumber,dType,fileType,nodata, psp.get());
    return newrd;
}

MemRasterTarget::MemRasterTarget(){

}
MemRasterTarget::~MemRasterTarget(){

}
string&MemRasterTarget::Name(){
    return name;
}

AutoPtr<RasterDataset>MemRasterTarget::CreateRasterDataset(SAEnvironment*envi,RasterDataType dType,int bandNumber,double nodata){
    AutoPtr<MemRasterWorkspaceFactory>pFac(new MemRasterWorkspaceFactory());
    AutoPtr<MemRasterWorkspace>pWork=pFac->CreateMemRasterWorkspace();
    AutoPtr<SpatialReference>psp=envi->GetSpatialReference();
    AutoPtr<RasterDataset>newrd=pWork->CreateRasterDataset(name,envi->GetRasterDesp(), bandNumber,dType,nodata, psp.get());
    return newrd;
}

}
