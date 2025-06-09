#ifndef MEMDATAARRAY_H
#define MEMDATAARRAY_H
#include "Base/autoptr.h"

namespace SGIS{

enum RasterDataType
{
    rdtUnknown	= 0,
    rdtByte	= 0x1,
    rdtUInt16	= 0x2,
    rdtInt16	= 0x3,
    rdtUInt32	= 0x4,
    rdtInt32	= 0x5,
    rdtFloat32	= 0x6,
    rdtFloat64	= 0x7
};

struct RasterDataTypeHelper{
    template<typename T>
    static RasterDataType GetRasterDataType(){
        int size=sizeof(T);
        if(size==1)
            return rdtByte;
        else if(size==2){
            if(::is_unsigned<T>()) return rdtUInt16;
            return rdtInt16;
        }
        else if(size==4){
            if(::is_unsigned<T>()) return rdtUInt32;
            if(::is_integral<T>()) return rdtInt32;
            return rdtFloat32;
        }
        else if(size==8){
            return rdtFloat64;
        }
        return rdtUnknown;
    };
    static BYTE GetGDALDataType(RasterDataType type){
      switch(type){
      case rdtByte:return 1;
      case rdtUInt16:return 2;
      case rdtInt16:return 3;
      case rdtUInt32:return 4;
      case rdtInt32:return 5;
      case rdtFloat32:return 6;
      case rdtFloat64:return 7;
      }
      return 0;
    };
    static RasterDataType GetRasterDataType(BYTE type){
      switch(type){
      case 1:return rdtByte;
      case 2:return rdtUInt16;
      case 3:return rdtInt16;
      case 4:return rdtUInt32;
      case 5:return rdtInt32;
      case 6:return rdtFloat32;
      case 7:return rdtFloat64;
      }
      return rdtUnknown;
    };
    static int GetPixelSize(RasterDataType type){
        switch(type){
        case rdtByte:
            return 1;
        case rdtUInt16:
        case rdtInt16:
            return 2;
        case rdtUInt32:
        case rdtInt32:
        case rdtFloat32:
            return 4;
        case rdtFloat64:
            return 8;
        }
        return 0;
    };
};

class SGIS_EXPORT MemDataArray
{
public:
    MemDataArray();
    virtual~MemDataArray();
    RasterDataType GetDataType();
    LONG GetSize();
    BYTE*GetValues();
    bool SetDefaultValue(float defaultValue);
    template<typename T>
    T GetValue(int nPos){
        switch(dataType){
        case rdtByte:{
            return values[nPos];
        }
        case rdtUInt16:{
            return ((USHORT*)values)[nPos];
        }
        case rdtInt16:{
            return ((SHORT*)values)[nPos];
        }
        case rdtUInt32:{
            return ((UINT*)values)[nPos];
        }
        case rdtInt32:{
            return ((INT*)values)[nPos];
        }
        case rdtFloat32:{
            return ((FLOAT*)values)[nPos];
        }
        case rdtFloat64:{
            return ((DOUBLE*)values)[nPos];
        }
        }
        return 0;
    };
    template<typename T>
    void GetValues(int fromPos,int count,T*data){
        int toPos=fromPos+count;
        switch(dataType){
        case rdtByte:{
            for(int k=fromPos;k<toPos;k++) data[k-fromPos]=values[k];
            break;
        }
        case rdtUInt16:{
            USHORT*us=(USHORT*)values;
            for(int k=fromPos;k<toPos;k++) data[k-fromPos]=us[k];
            break;
        }
        case rdtInt16:{
            SHORT*us=(SHORT*)values;
            for(int k=fromPos;k<toPos;k++) data[k-fromPos]=us[k];
            break;
        }
        case rdtUInt32:{
            UINT*us=(UINT*)values;
            for(int k=fromPos;k<toPos;k++) data[k-fromPos]=us[k];
            break;
        }
        case rdtInt32:{
            INT*us=(INT*)values;
            for(int k=fromPos;k<toPos;k++) data[k-fromPos]=us[k];
            break;
        }
        case rdtFloat32:{
            FLOAT*us=(FLOAT*)values;
            for(int k=fromPos;k<toPos;k++) data[k-fromPos]=us[k];
            break;
        }
        case rdtFloat64:{
            DOUBLE*us=(DOUBLE*)values;
            for(int k=fromPos;k<toPos;k++) data[k-fromPos]=us[k];
            break;
        }
        }
    };
    template<typename T>
    void SetValues(int fromPos,int count,T*data){
        int toPos=fromPos+count;
        switch(dataType){
        case rdtByte:{
            for(int k=fromPos;k<toPos;k++) values[k]=data[k-fromPos];
            break;
        }
        case rdtUInt16:{
            USHORT*us=(USHORT*)values;
            for(int k=fromPos;k<toPos;k++) us[k]=data[k-fromPos];
            break;
        }
        case rdtInt16:{
            SHORT*us=(SHORT*)values;
            for(int k=fromPos;k<toPos;k++) us[k]=data[k-fromPos];
            break;
        }
        case rdtUInt32:{
            UINT*us=(UINT*)values;
            for(int k=fromPos;k<toPos;k++) us[k]=data[k-fromPos];
            break;
        }
        case rdtInt32:{
            INT*us=(INT*)values;
            for(int k=fromPos;k<toPos;k++) us[k]=data[k-fromPos];
            break;
        }
        case rdtFloat32:{
            FLOAT*us=(FLOAT*)values;
            for(int k=fromPos;k<toPos;k++) us[k]=data[k-fromPos];
            break;
        }
        case rdtFloat64:{
            DOUBLE*us=(DOUBLE*)values;
            for(int k=fromPos;k<toPos;k++) us[k]=data[k-fromPos];
            break;
        }
        }
    };
    template<typename T>
    void GetBlockData(int x,int y,int width,int height,int cols,int rows,T*data){
        if(size<cols*rows) return;
        int toy=y+height;
        int tox=x+width;
        switch(dataType){
        case rdtByte:{
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    data[pos]=values[rPos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtUInt16:{
            USHORT*us=(USHORT*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    data[pos]=us[rPos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtInt16:{
            SHORT*us=(SHORT*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    data[pos]=us[rPos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtUInt32:{
            UINT*us=(UINT*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    data[pos]=us[rPos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtInt32:{
            INT*us=(INT*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    data[pos]=us[rPos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtFloat32:{
            FLOAT*us=(FLOAT*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    data[pos]=us[rPos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtFloat64:{
            DOUBLE*us=(DOUBLE*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    data[pos]=us[rPos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        }
    };
    template<typename T>
    void SaveBlockData(int x,int y,int width,int height,int cols,int rows,T*data){
        if(size<cols*rows) return;
        int toy=y+height;
        int tox=x+width;
        switch(dataType){
        case rdtByte:{
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    values[rPos]=data[pos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtUInt16:{
            USHORT*us=(USHORT*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    us[rPos]=data[pos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtInt16:{
            SHORT*us=(SHORT*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    us[rPos]=data[pos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtUInt32:{
            UINT*us=(UINT*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    us[rPos]=data[pos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtInt32:{
            INT*us=(INT*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    us[rPos]=data[pos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtFloat32:{
            FLOAT*us=(FLOAT*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    us[rPos]=data[pos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        case rdtFloat64:{
            DOUBLE*us=(DOUBLE*)values;
            LONG pos=0;
            for(int i=y;i<toy;i++){
                LONG rPos=i*rows;
                for(int j=x;j<tox;j++){
                    us[rPos]=data[pos];
                    pos++;
                    rPos++;
                }
            }
            break;
        }
        }
    };
    bool Initialize(LONG size,RasterDataType dataType);
    bool Initialize(LONG size,RasterDataType dataType,float defaultValue);
    void Attach(BYTE*values,LONG size,RasterDataType dataType);
protected:
    LONG size;
    BYTE*values;
    RasterDataType dataType;
    bool refData;
};

}

#endif // MEMDATAARRAY_H
