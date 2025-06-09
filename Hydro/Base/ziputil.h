#ifndef ZIP_H
#define ZIP_H
#include "autoptr.h"
#include "BaseArray.h"
#include "CallBack.h"
#include "semaphora.h"
#include "file.h"

namespace SGIS{

class SGIS_EXPORT GZip
{
public:
    static AutoPtr<FixBuffer>CompressByDeflate(AutoPtr<FixBuffer>inbuf,int level=6,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    static AutoPtr<FixBuffer>CompressByGZip(AutoPtr<FixBuffer>inbuf,int level=6,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    static bool CompressFile(const string&inFile,const string&outFile,int level=6,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    static bool UncompressFile(const string&inFile,const string&outFile,int level=6,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    static vector<BYTE>CompressFileToBuffer(const string&inFile,int level=6,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    static AutoPtr<FixBuffer>UncompressByDeflate(AutoPtr<FixBuffer>inbuf,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    static AutoPtr<FixBuffer>UncompressByGZip(AutoPtr<FixBuffer>inbuf,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
};

/*
class SGIS_EXPORT GZipFileCompress{
public:
    GZipFileCompress(string infile,bool gzip=true,int level=6);
    ~GZipFileCompress();
    bool BeginCompress();
    AutoPtr<FixBuffer>GetNextBuffer();
    int GetBufferSizePerBlock();
    bool Reset();
protected:
    bool readBlock();
protected:
    vector<BYTE>dest;
    int outputBytes;
    AutoPtr<FixBuffer>fxb;
    AutoPtr<FixBuffer>cfxb;
    AutoPtr<FixBuffer>ofxb;
    int inputBytes;

    bool gzip;
    int level;
    string infile;
    ULONGLONG fileSize;
    ULONGLONG filePin;
    AutoPtr<BinaryFile>bf;

    void*stream;
    int BUFFER_SIZE_PER_BLOCK;
    int err;
};

class SGIS_EXPORT GZipStreamCompress{
public:
    GZipStreamCompress(AutoPtr<FixBuffer>fxb,bool gzip=true,int level=6);
    ~GZipStreamCompress();
    bool BeginCompress();
    AutoPtr<FixBuffer>GetNextBuffer();
    bool Reset();
    int GetBufferSizePerBlock();
protected:
    bool readBlock();
protected:
    vector<BYTE>dest;
    int outputBytes;
    AutoPtr<FixBuffer>fxb;
    AutoPtr<FixBuffer>cfxb;
    AutoPtr<FixBuffer>ofxb;
    int inputBytes;

    bool gzip;
    int level;
    ULONGLONG fileSize;
    ULONGLONG filePin;
    void*stream;
    int BUFFER_SIZE_PER_BLOCK;
    int err;
};
*/

class SGIS_EXPORT Zip{
public:
    static bool ZipDir(const string&inDir,const string&outFile,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    static bool ZipFiles(const vector<string>&pathnames,const string&outFile,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    static bool UnZipFiles(const string&inFile,const string&outDir,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
};

class SGIS_EXPORT BZip2{
public:
    static AutoPtr<FixBuffer>UnZipFile(const string&pathName,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
    static bool UnZipFile(const string&pathName,const string&outFile,CallBack*callBack=nullptr,Semaphora*signal=nullptr);
};

};

#endif // ZIP_H
