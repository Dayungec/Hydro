#include "memdataarray.h"
#include "gdal_priv.h"
#include "ogrsf_frmts.h"

namespace SGIS{
MemDataArray::MemDataArray(){
    values=nullptr;
    size=0;
    dataType=RasterDataType::rdtUnknown;
    refData=false;
}
MemDataArray::~MemDataArray(){
    if(!refData){
         if(values!=nullptr) delete []values;
    }
}

RasterDataType MemDataArray::GetDataType(){
    return dataType;
}

LONG MemDataArray::GetSize(){
    return size;
}

BYTE*MemDataArray::GetValues(){
   return values;
}

bool MemDataArray::SetDefaultValue(float defaultValue){
    switch(dataType){
    case rdtByte:{
        for(int k=0;k<size;k++) values[k]=defaultValue;
        break;
    }
    case rdtUInt16:{
        USHORT*us=(USHORT*)values;
        for(int k=0;k<size;k++) us[k]=defaultValue;
        break;
    }
    case rdtInt16:{
        SHORT*us=(SHORT*)values;
        for(int k=0;k<size;k++) us[k]=defaultValue;
        break;
    }
    case rdtUInt32:{
        UINT*us=(UINT*)values;
        for(int k=0;k<size;k++) us[k]=defaultValue;
        break;
    }
    case rdtInt32:{
        INT*us=(INT*)values;
        for(int k=0;k<size;k++) us[k]=defaultValue;
        break;
    }
    case rdtFloat32:{
        FLOAT*us=(FLOAT*)values;
        for(int k=0;k<size;k++) us[k]=defaultValue;
        break;
    }
    case rdtFloat64:{
        DOUBLE*us=(DOUBLE*)values;
        for(int k=0;k<size;k++) us[k]=defaultValue;
        break;
    }
    }
    return true;
}

bool MemDataArray::Initialize(LONG size,RasterDataType dataType){
    int formerSize=RasterDataTypeHelper::GetPixelSize(this->dataType)*this->size;
    int curSize=RasterDataTypeHelper::GetPixelSize(dataType)*size;
    if(!refData){
        if(formerSize==curSize){
            this->dataType=dataType;
            return true;
        }
        if(values!=nullptr) delete []values;
    }
    this->size=0;
    this->dataType=rdtUnknown;
    values=new BYTE[curSize];
    if(values==nullptr) return false;
    this->dataType=dataType;
    this->size=size;
    refData=false;
    return true;
}

bool MemDataArray::Initialize(LONG size,RasterDataType dataType,float defaultValue){
    if(!Initialize(size,dataType)) return false;
    SetDefaultValue(defaultValue);
    return true;
}

void MemDataArray::Attach(BYTE*values,LONG size,RasterDataType dataType){
    if(!refData){
         if(this->values!=nullptr) delete []this->values;
    }
    this->values=values;
    this->dataType=dataType;
    this->size=size;
    refData=true;
}


}
