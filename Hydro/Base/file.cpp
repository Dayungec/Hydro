#include "file.h"
#include "StringHelper.h"
#include <fstream>
#include "FilePath.h"
#include <bitset>

#define GetBit(v, n) ((v) & ((UINT)1 << (n)))

namespace SGIS{

BinaryFile::BinaryFile(){
    fileReader=nullptr;
    fileWriter=nullptr;
}
BinaryFile::~BinaryFile(){
    if(fileReader!=nullptr){
        fileReader->close();
        delete fileReader;
    }
    if(fileWriter!=nullptr){
        fileWriter->close();
        delete fileWriter;
    }
}
AutoPtr<FixBuffer>BinaryFile::Read(const string&pathName){
    std::ifstream ifs;
    ifs.open(StringHelper::ToLocalString(pathName), std::ios::binary | ios::in);
    if (!ifs.is_open()) return nullptr;
    ifs.seekg(0, std::ios::end);
    ULONGLONG nFileLen = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    FixBuffer*buffer=new FixBuffer(nFileLen);
    ifs.read(buffer->GetBuffer(), nFileLen);
    ifs.close();
    return buffer;
}

bool BinaryFile::Write(const string&pathName,FixBuffer*buffer){
    ofstream destFile(StringHelper::ToLocalString(pathName).c_str(), std::ios::binary |ios::out);
    if(!destFile.is_open()) return false;
    destFile.write(buffer->GetBuffer(),buffer->GetSize());
    destFile.close();
    return true;
}

bool BinaryFile::Write(const string&pathName,char*buffer,ULONGLONG len){
    ofstream destFile(StringHelper::ToLocalString(pathName).c_str(), std::ios::binary |ios::out);
    if(!destFile.is_open()) return false;
    destFile.write(buffer,len);
    destFile.close();
    return true;
}

ULONGLONG BinaryFile::ReadTo(const string&pathName,BYTE*bytes,ULONGLONG len){
    std::ifstream ifs;
    ifs.open(StringHelper::ToLocalString(pathName), std::ios::binary | ios::in);
    if (!ifs.is_open()) return 0;
    ifs.seekg(0, std::ios::end);
    ULONGLONG nFileLen = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    if(len<nFileLen){
        ifs.close();
        return 0;
    }
    ifs.read((char*)bytes, nFileLen);
    ifs.close();
    return nFileLen;
}

bool BinaryFile::OpenForRead(const string&pathName){
    if(fileReader!=nullptr){
        fileReader->close();
        delete fileReader;
    }
    if(fileWriter!=nullptr){
        fileWriter->close();
        delete fileWriter;
    }
    fileReader=nullptr;
    fileWriter=nullptr;
    fileReader=new ifstream();
    fileReader->open(StringHelper::ToLocalString(pathName),std::ios::binary |ios::in);
    if(!fileReader->is_open()){
        delete fileReader;
        fileReader=nullptr;
        return false;
    }
    return true;
}

bool BinaryFile::OpenForWrite(const string&pathName,bool append){
    if(fileReader!=nullptr){
        fileReader->close();
        delete fileReader;
    }
    if(fileWriter!=nullptr){
        fileWriter->close();
        delete fileWriter;
    }
    fileReader=nullptr;
    fileWriter=nullptr;
    fileWriter=new fstream();
    if(FilePath::IsFileExisting(pathName)){
        if(append)
            fileWriter->open(StringHelper::ToLocalString(pathName),std::ios::binary | ios::in | ios::out);
        else
            fileWriter->open(StringHelper::ToLocalString(pathName),std::ios::binary  |ios::in |ios::out | ios::trunc);
    }
    else{
        if(append)
            fileWriter->open(StringHelper::ToLocalString(pathName),std::ios::binary | ios::out);
        else
            fileWriter->open(StringHelper::ToLocalString(pathName),std::ios::binary  |ios::out | ios::trunc);
    }
    if(!fileWriter->is_open()){
        delete fileWriter;
        fileWriter=nullptr;
        return false;
    }
    return true;
}

ULONGLONG BinaryFile::GetFileLength(){
    if(fileReader!=nullptr){
        if (!fileReader->is_open()) return 0;
        fileReader->seekg(0, std::ios::end);
        ULONGLONG nFileLen = fileReader->tellg();
        fileReader->seekg(0, std::ios::beg);
        return nFileLen;
    }
    else{
        return 0;
    }
}

bool BinaryFile::Read(char*buffer,int readLen){
    if(fileReader==nullptr) return false;
    fileReader->read(buffer,readLen);
    return (!fileReader->eof());
}
bool BinaryFile::Write(char*buffer,int writeLen){
    if(fileWriter==nullptr) return false;
    fileWriter->write(buffer,writeLen);
    return true;
}

bool BinaryFile::Seek(ULONGLONG pos){
    if(fileReader!=nullptr){
        fileReader->clear();
        fileReader->seekg(0,ios::end);
        ULONGLONG fileSize=fileReader->tellg();
        if(fileSize==pos) return true;
        if(pos<fileSize){
            fileReader->clear();
            fileReader->seekg(pos,ios::beg);
        }
        return false;
    }
    else if(fileWriter!=nullptr){
         fileWriter->clear();
         fileWriter->seekp(0,ios::end);
         ULONGLONG fileSize=fileWriter->tellp();
         if(fileSize==pos) return true;
         if(pos<fileSize){
            fileWriter->clear();
            fileWriter->seekp(pos,ios::beg);
         }
         else{
            char temp[1024];
            memset(temp,0,1024);
            for(int k=fileSize;k<pos;k+=1024){
                int pl=1024;
                if(k+pl>pos) pl=pos-k;
                fileWriter->write(temp,pl);
            }
         }
         return true;
    }
    return false;
}

TextFile::TextFile(){
    fileReader=nullptr;
    fileWriter=nullptr;
}
TextFile::~TextFile(){
    if(fileReader!=nullptr){
        fileReader->close();
        delete fileReader;
    }
    if(fileWriter!=nullptr){
        fileWriter->close();
        delete fileWriter;
    }
}

bool TextFile::Read(const string&pathName,string&str,bool autoDetectCode){
    string code="UTF8";
    if(autoDetectCode) code=GetFileCode(pathName);
    std::ifstream t(StringHelper::ToLocalString(pathName));
    if (!t.is_open()) return false;
    std::string strd((std::istreambuf_iterator<char>(t)),
                     std::istreambuf_iterator<char>());
    t.close();
    str=strd;
    if(code=="ANSI") str=StringHelper::FromLocalString(strd);
    return true;
    /*
    std::ifstream ifs;
    ifs.open(pathName, std::ios::binary | ios::in);
    if (!ifs.is_open()) return false;
    ifs.seekg(0, std::ios::end);
    int nFileLen = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    char*buffer=new char[nFileLen];
    ifs.read(buffer, nFileLen);
    str = "";
    str.append(buffer);
    ifs.close();
    delete[]buffer;
    return true;
    */
}

bool getIsbigendian_bit()
{
    char a = 1;
    if (GetBit(a, 0) >> 0)
        return true;
    return false;
}

string TextFile::getEncode(BYTE* data, int size, int ChineseNumber)
{
    bool isbigendian = getIsbigendian_bit();
    int byte = 0;
    int utf8number = 0;
    //如果是大端位序，从右边开始读取位；如果是小端位序，从左边开始读位
    if (isbigendian){
        for (int i = 0; i < size; i++){
            if (GetBit(data[i], 7) >> 7 == 1){
                int byte = 0;
                for (int j = 7; j >= 0; j--)
                {
                    if ((GetBit(data[i], j) >> j) == 1)
                        byte++;
                    else
                        break;
                }
                if (byte ==0)
                    utf8number = 0;
                if (byte > 1){
                    bitset <8> head_bit_match[] = { data[i], data[i + 1], data[i + 2], data[i + 3] };
                    bool isutf8 = true;
                    for (int j = 1; j < byte; j++)
                    {
                        if (!(head_bit_match[j][7] == 1 && head_bit_match[j][6] == 0))
                            isutf8 &= false;
                    }
                    if (isutf8)
                    {
                        utf8number++;
                        i += (byte - 1);
                        if (utf8number >= ChineseNumber)
                            return "UTF8";
                    }
                    else
                        return "ANSI";
                }
                else if (byte == 1)
                    return "ANSI";
            }
        }
    }
    else
    {
        for (int i = 0; i < size; i++){
            if (GetBit(data[i], 0) >> 0 == 1){
                int byte = 0;
                for (int j = 0; j < 8; j++)
                {
                    if ((GetBit(data[i], j) >> j) == 1)
                        byte++;
                    else
                        break;
                }
                //若byte等于0，则非中文，中文数量清零
                if (byte == 0)
                    utf8number = 0;
                if (byte > 1)
                {
                    bitset <8> head_bit_match[] = { data[i], data[i + 1], data[i + 2], data[i + 3] };
                    bool isutf8 = true;
                    for (int j = 1; j < byte; j++)
                    {
                        if (!(head_bit_match[j][0] == 1 && head_bit_match[j][1] == 0))
                            isutf8 &= false;
                    }
                    if (isutf8)
                    {
                        utf8number++;
                        i += (byte - 1);
                        if (utf8number >= ChineseNumber)
                            return "UTF8";
                    }
                    else
                        return "ANSI";
                }
                else if (byte == 1)
                    return "ANSI";
            }
        }
    }
    return "ANSI";
}

string TextFile::GetFileCode(const string&pathName){
    std::ifstream t(StringHelper::ToLocalString(pathName),ios::binary);
    if (!t.is_open()) return "";
    t.seekg(0, std::ios::end);
    ULONGLONG nFileLen = t.tellg();
    t.seekg(0, std::ios::beg);
    if(nFileLen==0) return "";
    unsigned char s1=0,s2=0,s3=0;
    if(nFileLen>0) t.read((char*)&s1, sizeof(s1));
    if(nFileLen>1) t.read((char*)&s2, sizeof(s2));
    if(nFileLen>2) t.read((char*)&s3, sizeof(s3));
    string code="";
    if (s1 == 0xFF && s2 == 0xFE)
        code= "UTF16_LE";
    else if (s1 == 0xFE && s2 == 0xFF)
        code= "UTF16_BE";
    else if (s1 == 0xEF && s2 == 0xBB && s3 == 0xBF)
        code= "UTF8_BOM";
    if(code!=""){
        t.close();
        return code;
    }
    t.seekg(0, std::ios::beg);
    char bytes[1024];
    for(int l=0;l<nFileLen;l+=2014){
        int readLen=1024;
        if(l+readLen>nFileLen) readLen=nFileLen-l;
        t.read(bytes,readLen);
        string icode=getEncode((BYTE*)bytes,readLen);
        if(icode!="ANSI"){
            t.close();
            return icode;
        }
    }
    code="ANSI";
    t.close();
    return code;
}

bool TextFile::Write(const string&pathName,string&str){
    ofstream destFile(StringHelper::ToLocalString(pathName).c_str(), std::ios::binary |ios::out);
   if(!destFile.is_open()) return false;
   const char*buffer=str.data();
   int size=str.length();
   destFile.write(buffer,size);
   destFile.close();
   return true;
}

bool TextFile::OpenForRead(const string&pathName){
    if(fileReader!=nullptr){
        fileReader->close();
        delete fileReader;
    }
    if(fileWriter!=nullptr){
        fileWriter->close();
        delete fileWriter;
    }
    fileReader=nullptr;
    fileWriter=nullptr;
    fileReader=new ifstream();
    fileReader->open(StringHelper::ToLocalString(pathName),ios::in);
    if(!fileReader->is_open()){
        delete fileReader;
        fileReader=nullptr;
        return false;
    }
    return true;
}

bool TextFile::OpenForWrite(const string&pathName,bool append){
    if(fileReader!=nullptr){
        fileReader->close();
        delete fileReader;
    }
    if(fileWriter!=nullptr){
        fileWriter->close();
        delete fileWriter;
    }
    fileReader=nullptr;
    fileWriter=nullptr;
    fileWriter=new ofstream();
    if(append)
        fileWriter->open(StringHelper::ToLocalString(pathName),ios::app);
    else
        fileWriter->open(StringHelper::ToLocalString(pathName),ios::trunc);
    if(!fileWriter->is_open()){
        delete fileWriter;
        fileWriter=nullptr;
        return false;
    }
    return true;
}

bool TextFile::ReadLine(string&line){
    if(fileReader==nullptr) return false;
    if(fileReader->eof()) return false;
    if(!getline(*fileReader,line)) return false;
    return true;
}

bool TextFile::WriteLine(const string&line){
    if(fileWriter==nullptr) return false;
    (*fileWriter)<<line<<"\n";
    return true;
}

}

