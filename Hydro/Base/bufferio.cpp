#include "bufferio.h"
#include "classfactory.h"
#include "file.h"
#include "StringHelper.h"

namespace SGIS{

BufferWriter::BufferWriter(){
    currentPin=0;
}

BufferWriter::~BufferWriter(){

}

void BufferWriter::WriteBuffer(FixBuffer*fixBuffer){
    Write<int>(fixBuffer->GetSize());
    char*buffer=autobuffer.GetBuffer(currentPin+fixBuffer->GetSize());
    memcpy(buffer+currentPin,fixBuffer->GetBuffer(),fixBuffer->GetSize());
    currentPin+=fixBuffer->GetSize();
}

void BufferWriter::WriteString(string str){
    int bytes=str.length();
    Write<int>(bytes);
    char*buffer=autobuffer.GetBuffer(currentPin+bytes);
    if(bytes>0){
        const char*vs=str.data();
        memcpy(buffer+currentPin,vs,bytes);
        currentPin+=bytes;
    }
}

void BufferWriter::WriteObject(BufferWritable*obj,bool allowNull){
    if(allowNull){
        Write<BYTE>((obj==nullptr)?0:1);
        if(obj==nullptr) return;
    }
    string typeName=obj->GetTypeName();
    this->WriteString(typeName);
    obj->Write(this);
}

char*BufferWriter::GetBuffer(){
    return autobuffer.GetBuffer(0);
}

ULONGLONG BufferWriter::GetBufferLength(){
    return currentPin;
}

AutoPtr<FixBuffer>BufferWriter::GetFixBuffer(){
    FixBuffer*fxb=new FixBuffer(this->GetBufferLength());
    memcpy(fxb->GetBuffer(),this->GetBuffer(),this->GetBufferLength());
    return fxb;
}

BufferReader::BufferReader(char*buffer,ULONGLONG totalSize){
    this->buffer=buffer;
    currentPin=0;
    lpszPathName="";
    this->totalSize=totalSize;
}
BufferReader::~BufferReader(){

}

string&BufferReader::PathName(){
    return lpszPathName;
}

ULONGLONG BufferReader::GetCurrentPin(){
    return currentPin;
}

string BufferReader::ReadString(){
    int bytes=Read<int>();
    if(bytes==0) return "";
    if(totalSize>0){
        if(currentPin+bytes>totalSize){
            return "";
        }
    }
    string value;
    value.resize(bytes);
    const char*szbuf=value.data();
    memcpy((char*)szbuf,buffer+currentPin,bytes);
    currentPin+=bytes;
    return value;
}

AutoPtr<FixBuffer>BufferReader::ReadBuffer(){
    int size=Read<int>();
    int len=size;
    if(totalSize>0){
        if(currentPin+len>totalSize) return nullptr;
    }
    FixBuffer*fxb=new FixBuffer(size);
    memcpy(fxb->GetBuffer(),buffer+currentPin,len);
    currentPin+=len;
    return fxb;
}

void BufferReader::Move(int pos){
    currentPin+=pos;
}

void BufferReader::MoveTo(int pos){
    currentPin=pos;
}

AutoPtr<BufferWritable>BufferReader::ReadObject(bool allowNull){
    if(allowNull){
        bool isNull=(Read<BYTE>()==0);
        if(isNull) return nullptr;
    }
    string objName=ReadString();
    if(objName=="") return nullptr;
    AutoPtr<BufferWritable>obj=ClassFactory::getInstance(objName);
    if(obj==nullptr) return nullptr;
    if(!obj->Read(this)) return nullptr;
    return obj;
}

BufferFile::BufferFile(){

}
BufferFile::~BufferFile(){

}
bool BufferFile::SaveFile(string pathName,const vector<AutoPtr<BufferWritable>>bws){
    AutoPtr<BufferWriter>bw(new BufferWriter());
    string key="BUFFERFILE";
    vector<string>keys=StringHelper::LetterSplit(key);
    for(int k=0;k<keys.size();k++){
        bw->Write<unsigned char>(keys[k][0]);
    }
    bw->Write<int>(bws.size());
    for(int k=0;k<bws.size();k++){
        bw->WriteObject(bws[k].get(),true);
    }
    int bufferLen=bw->GetBufferLength();
    char*buffer=bw->GetBuffer();
    return BinaryFile::Write(pathName,buffer,bufferLen);
}

vector<AutoPtr<BufferWritable>>BufferFile::ReadFile(string pathName){
    AutoPtr<FixBuffer>fb=BinaryFile::Read(pathName);
    if(fb==nullptr) return {};
    string key="BUFFERFILE";
    vector<string>keys=StringHelper::LetterSplit(key);
    if(fb->GetSize()<=keys.size()) return {};
    BufferReader br(fb->GetBuffer());
    for(int k=0;k<keys.size();k++){
        BYTE bV=br.Read<BYTE>();
        if(bV!=keys[k][0]) return {};
    }
    br.PathName()=pathName;
    vector<AutoPtr<BufferWritable>>bws;
    int size=br.Read<int>();
    for(int k=0;k<size;k++){
        bws.push_back(br.ReadObject(true));
    }
    return bws;
}

}
