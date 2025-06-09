#ifndef FILE_H
#define FILE_H
#include "base_globle.h"
#include "BaseArray.h"
#include "Json/json.h"
#include "autoptr.h"

namespace SGIS{

class SGIS_EXPORT BinaryFile
{
public:
    BinaryFile();
    virtual~BinaryFile();
    static AutoPtr<FixBuffer>Read(const string&pathName);
    static bool Write(const string&pathName,FixBuffer*buffer);
    static bool Write(const string&pathName,char*buffer,ULONGLONG len);
    static ULONGLONG ReadTo(const string&pathName,BYTE*bytes,ULONGLONG len);
    bool OpenForRead(const string&pathName);
    bool OpenForWrite(const string&pathName,bool append);
    ULONGLONG GetFileLength();
    bool Read(char*buffer,int readLen);
    bool Write(char*buffer,int writeLen);
    bool Seek(ULONGLONG pos);
protected:
    std::ifstream*fileReader;
    std::fstream*fileWriter;
};

class SGIS_EXPORT TextFile{
public:
    TextFile();
    virtual~TextFile();
    static bool Read(const string&pathName,string&str,bool autoDetectCode=true);
    static bool Write(const string&pathName,string&str);
    static string GetFileCode(const string&pathName);
    bool OpenForRead(const string&pathName);
    bool OpenForWrite(const string&pathName,bool append);
    bool ReadLine(string&line);
    bool WriteLine(const string&line);
protected:
    static string getEncode(BYTE*buffer,int len,int chinestNum=4);
protected:
    std::ifstream*fileReader;
    std::ofstream*fileWriter;
};
}
#endif // FILE_H
