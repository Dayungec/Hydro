#include "base_globle.h"
#include<string.h>
#include "BaseArray.h"

const int ary[8]={1,2,4,8,16,32,64,128};
const int ary4[4]={3,12,48,192};
namespace SGIS{

TwoValueArray::TwoValueArray(int size){
   this->size=0;
   this->bits=nullptr;
   if(size>0) SetSize(size);
   attached=false;
}
TwoValueArray::~TwoValueArray(){
    if(!attached){
        if(bits!=nullptr) delete []bits;
    }
    bits=nullptr;
}

int TwoValueArray::GetSize(){
    return size;
}

bool TwoValueArray::SetSize(int size,bool defaultValue){
    if(this->size==size) return true;
    this->size=size;
    if(bits!=nullptr) delete []bits;
    bits=NULL;
    if(size>0){
        if(size%8==0){
            bits=new unsigned char[size/8];
            if(bits==NULL) {
                size=0;
                return false;
            }
            unsigned char bV=(defaultValue?255:0);
            memset(bits,bV,size/8);
        }
        else{
            bits=new unsigned char[size/8+1];
            if(bits==NULL) {
                size=0;
                return false;
            }
            unsigned char bV=(defaultValue?255:0);
            memset(bits,bV,size/8+1);
        }
    }
    return true;
}

void TwoValueArray::CopyFrom(unsigned char*buffer,int bytelen){
    if(bytelen>size/8) bytelen=size/8;
    if(bytelen==0) return;
    memcpy(bits,buffer,bytelen);
}

void TwoValueArray::Attach(unsigned char*buffer,int bytelen){
    if(!attached){
        if(bits!=nullptr) delete []bits;
    }
    bits=nullptr;
    bits=buffer;
    size=bytelen*8;
    attached=true;
}

int TwoValueArray::GetCombineValue(int nPos,int len){
    int tPos=nPos+len-1;
    int iV=0;
    int nSize=0;
    for(int k=tPos;k>=nPos;k--){
        if(k>=size){
            nSize++;
            continue;
        }
        iV+=((*this)[k]?1:0)<<nSize;
        nSize++;
    }
    return iV;
}

void TwoValueArray::SetCombineValue(int nPos,int len,int value){
    int tPos=nPos+len-1;
    int nSize=0;
    for(int k=tPos;k>=nPos;k--){
        if(k>=size){
            nSize++;
            continue;
        }
        int pV=((value>>nSize)&1);
        SetValue(k,(pV!=0));
        nSize++;
    }
}

void TwoValueArray::SetDefaultValue(bool defaultValue){
    int v;
    if(defaultValue)
        v=255;
    else
        v=0;
    if(size%8==0)
    {
        memset(bits,v,size/8);
    }
    else
    {
        memset(bits,v,size/8+1);
    }
}

bool TwoValueArray::operator[] (int nIndex){
    int Num=(nIndex>>3);
    if ((bits[Num]&ary[7-nIndex+(Num<<3)])>0)
        return true;
    return false;
}

void TwoValueArray::SetValue(int nIndex,bool value){
    unsigned char Value=(value?1:0);
    int Num=nIndex>>3;
    int p=7-nIndex+(Num<<3);
    int iValue=(value?1:0);
    bits[Num]=bits[Num]-(bits[Num]&ary[p])+(iValue<<p);
}

unsigned char*TwoValueArray::GetBits(){
    return bits;
}


FourValueArray::FourValueArray(int size){
   this->size=0;
   this->bits=nullptr;
   if(size>0) SetSize(size);
}
FourValueArray::~FourValueArray(){
    if(bits!=nullptr) delete []bits;
    bits=nullptr;
}

int FourValueArray::GetSize(){
    return size;
}

bool FourValueArray::SetSize(int size,unsigned char defaultValue){
    if(this->size==size) return true;
    this->size=size;
    if(bits!=nullptr) delete []bits;
    bits=NULL;
    bits=NULL;
    if(defaultValue<0)
        defaultValue=0;
    else if(defaultValue>3)
        defaultValue=3;
    unsigned char Value=((unsigned char)defaultValue)*85;
    if(size>0){
        if(size%4==0)
        {
            bits=new unsigned char[size/4];
            if(bits==NULL)
            {
                if(bits==NULL) {
                    size=0;
                    return false;
                }
            }
            memset(bits,Value,size/4);
        }
        else
        {
            bits=new unsigned char[size/4+1];
            if(bits==NULL)
            {
                if(bits==NULL) {
                    size=0;
                    return false;
                }
            }
            memset(bits,Value,size/4+1);
        }
    }
    return true;
}

void FourValueArray::SetDefaultValue(unsigned char defaultValue){
    if(defaultValue<0)
        defaultValue=0;
    else if(defaultValue>3)
        defaultValue=3;
    unsigned char Value=((unsigned char)defaultValue)*85;
    if(size%4==0)
    {
        memset(bits,Value,size/4);
    }
    else
    {
        memset(bits,Value,size/4+1);
    }
}

unsigned char FourValueArray::operator[] (int nIndex){
    int Num=nIndex>>2;
    int p=3-nIndex+(Num<<2);
    return ((bits[Num]&ary4[p])>>(p<<1));
}

void FourValueArray::SetValue(int nIndex,unsigned char value){
    int iValue=value;
    if(iValue<0)
        iValue=0;
    else if(iValue>3)
        iValue=3;
    int Num=nIndex>>2;
    int p=3-nIndex+(Num<<2);
    bits[Num]-=(bits[Num]&ary4[p]);
    bits[Num]+=(((unsigned char)iValue)<<(p<<1));
}

AutoBuffer::AutoBuffer(int initialBlocks,int sizeofBlock){
    int curLen=initialBlocks*sizeofBlock;
    this->sizeofBlock=sizeofBlock;
    this->blocks=initialBlocks;
    buffer.resize(curLen);
}
AutoBuffer::~AutoBuffer(){

}

char*AutoBuffer::GetBuffer(int len){
    int curLen=blocks*sizeofBlock;
    if(len>curLen){
        int totalBlocks=len/sizeofBlock;
        if(totalBlocks*sizeofBlock<len) totalBlocks++;
        blocks=totalBlocks;
        buffer.resize(blocks*sizeofBlock);
    }
    return buffer.data();
}

FixBuffer::FixBuffer(ULONGLONG size){
    if(size>0)
        buffer=new char[size];
    else
        buffer=nullptr;
    this->size=size;
    if(buffer==nullptr) this->size=0;
    needDestroy=true;
}

FixBuffer::FixBuffer(char*buffer,ULONGLONG size){
    this->buffer=buffer;
    this->size=size;
    needDestroy=true;
}

FixBuffer::~FixBuffer(){
    if(needDestroy){
        if(buffer!=nullptr) delete []buffer;
    }
}

void FixBuffer::Attach(FixBuffer*buffer){
    if(needDestroy){
        if(this->buffer!=nullptr) delete []this->buffer;
    }
    this->buffer=buffer->GetBuffer();
    this->size=buffer->GetSize();
    needDestroy=false;
}

void FixBuffer::Attach(char*buffer,int len){
    if(needDestroy){
        if(this->buffer!=nullptr) delete []this->buffer;
    }
    this->buffer=buffer;
    this->size=len;
    needDestroy=false;
}

AutoPtr<FixBuffer>FixBuffer::Clone(){
    FixBuffer*buf=new FixBuffer(size);
    memcpy(buf->GetBuffer(),buffer,size);
    return buf;
}

ULONGLONG FixBuffer::GetSize(){
    return size;
}

char*FixBuffer::GetBuffer(){
    return buffer;
}

string FixBuffer::ConvertString(){
    string str;
    str.resize(size);
    memcpy(str.data(),buffer,size);
    return str;
}

AutoPtr<JsonNode>FixBuffer::ConvertJson(){
    string sJson;
    sJson.resize(size);
    memcpy(sJson.data(),buffer,size);
    AutoPtr<JsonReader>jr(new JsonReader());
    AutoPtr<JsonNode>rootNode=jr->Parse(sJson);
    return rootNode;
}

}

