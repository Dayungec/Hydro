#include "featureclasstarget.h"
#include "Base/FilePath.h"

namespace SGIS{

FeatureClassTarget::FeatureClassTarget()
{

}

FeatureClassTarget::~FeatureClassTarget()
{

}

ShapefileTarget::ShapefileTarget(){

}
ShapefileTarget::~ShapefileTarget(){

}
string&ShapefileTarget::PathName(){
    return pathName;
}

AutoPtr<FeatureClass>ShapefileTarget::CreateFeatureClass(SpatialReference*psp,VectorShapeType shpeType,TableDesc*desc){
    AutoPtr<ShapefileWorkspaceFactory>pFac(new ShapefileWorkspaceFactory());
    string dir=FilePath::GetDir(pathName);
    string fileName=FilePath::GetFileName(pathName);
    AutoPtr<ShapefileWorkspace>pWork=pFac->OpenFromFile(dir);
    AutoPtr<FeatureClass>newfc=pWork->CreateFeatureClass(fileName,shpeType, desc,psp);
    return newfc;
}

AutoPtr<FeatureClass>ShapefileTarget::Create3DFeatureClass(SpatialReference*psp,VectorShapeType shpeType,TableDesc*desc){
    AutoPtr<ShapefileWorkspaceFactory>pFac(new ShapefileWorkspaceFactory());
    string dir=FilePath::GetDir(pathName);
    string fileName=FilePath::GetFileName(pathName);
    AutoPtr<ShapefileWorkspace>pWork=pFac->OpenFromFile(dir);
    AutoPtr<FeatureClass>newfc=pWork->Create3DFeatureClass(fileName,shpeType, desc,psp);
    return newfc;
}

MemFeatureClassTarget::MemFeatureClassTarget(){

}
MemFeatureClassTarget::~MemFeatureClassTarget(){

}
string&MemFeatureClassTarget::Name(){
    return name;
}

AutoPtr<FeatureClass>MemFeatureClassTarget::CreateFeatureClass(SpatialReference*psp,VectorShapeType shpeType,TableDesc*desc){
    AutoPtr<MemFeatureWorkspaceFactory>pFac(new MemFeatureWorkspaceFactory());
    AutoPtr<MemFeatureWorkspace>pWork=pFac->CreateMemFeatureWorkspace();
    AutoPtr<FeatureClass>newfc=pWork->CreateFeatureClass(name,shpeType, desc,psp);
    return newfc;
}


AutoPtr<FeatureClass>MemFeatureClassTarget::Create3DFeatureClass(SpatialReference*psp,VectorShapeType shpeType,TableDesc*desc){
    AutoPtr<MemFeatureWorkspaceFactory>pFac(new MemFeatureWorkspaceFactory());
    AutoPtr<MemFeatureWorkspace>pWork=pFac->CreateMemFeatureWorkspace();
    AutoPtr<FeatureClass>newfc=pWork->Create3DFeatureClass(name,shpeType, desc,psp);
    return newfc;
}

}


