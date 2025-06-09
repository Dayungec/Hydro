#include "gdal_priv.h"
#include "ogrsf_frmts.h"
#include "LicenseEngine.h"
#include "FilePath.h"
#include "StringHelper.h"

#include <iostream>

namespace SGIS{

string ASSETS_PATH="";

LicenseEngine::LicenseEngine(){

}

LicenseEngine::~LicenseEngine(){

}

string&LicenseEngine::AssetsPath(){
    return ASSETS_PATH;
}
bool LicenseEngine::StartUsing(string assets_path){
    GDALAllRegister();
    OGRRegisterAll();
    string curDir;
    if(assets_path==""){
         curDir=FilePath::GetCurrentDir()+"/Assets";
    }else
        curDir=assets_path;
    if(curDir!="") curDir=FilePath::ConvertToStardardPath(curDir);
    ASSETS_PATH=curDir;
    curDir+="\\proj";
    curDir=FilePath::ConvertToStardardPath(curDir);
    CPLSetConfigOption("GDAL_DATA", StringHelper::ToLocalString(curDir).c_str());
    CPLSetConfigOption("GDAL_FILENAME_IS_UTF8","NO");
    CPLSetConfigOption("SHAPE_ENCODING","");
    return true;
}


}
