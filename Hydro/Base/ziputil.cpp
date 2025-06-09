#include "ziputil.h"
#include "zlib/zlib.h"
#include "file.h"
#include "FilePath.h"
#include "zlib/zip.h"
#include "zlib/unzip.h"
#include "StringHelper.h"
#include "bzib2/bzlib.h"

namespace SGIS{

AutoPtr<FixBuffer>GZip::CompressByDeflate(AutoPtr<FixBuffer>inbuf,int level,CallBack*callBack,Semaphora*signal){
    unsigned long sourceLen=inbuf->GetSize();
    unsigned long destLen=sourceLen;
    BYTE*source=(BYTE*)inbuf->GetBuffer();
    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong left;

    left = destLen;
    destLen = 0;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = deflateInit(&stream, level);
    if (err != Z_OK) return nullptr;
    int BUFFER_SIZE_PER_BLOCK=10240;
    vector<BYTE>dest(BUFFER_SIZE_PER_BLOCK);
    stream.next_out = dest.data();
    stream.avail_out = BUFFER_SIZE_PER_BLOCK;
    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    double percentage=0;
    do {
        if (stream.avail_out == 0) {
            dest.resize(dest.size()+BUFFER_SIZE_PER_BLOCK);
            stream.next_out = dest.data()+(dest.size()-BUFFER_SIZE_PER_BLOCK);
            stream.avail_out = BUFFER_SIZE_PER_BLOCK;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (uLong)max ? max : (uInt)sourceLen;
            sourceLen -= stream.avail_in;
            percentage=1-sourceLen*100.0/inbuf->GetSize();
            CallBackHelper::SendProgress(callBack,percentage);
            if(signal!=nullptr){
                if(signal->Wait(10)){
                    return nullptr;
                }
            }
        }
        err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);
    if(err != Z_STREAM_END ){
        return nullptr;
    }
    destLen = stream.total_out;
    deflateEnd(&stream);
    AutoPtr<FixBuffer>fxb(new FixBuffer(destLen));
    memcpy(fxb->GetBuffer(),dest.data(),destLen);
    return fxb;
}

AutoPtr<FixBuffer>GZip::CompressByGZip(AutoPtr<FixBuffer>inbuf,int level,CallBack*callBack,Semaphora*signal){
    unsigned long sourceLen=inbuf->GetSize();
    unsigned long destLen=sourceLen;
    BYTE*source=(BYTE*)inbuf->GetBuffer();
    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong left;

    left = destLen;
    destLen = 0;

    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;
    err = deflateInit2_(&stream,level,Z_DEFLATED, 31, 8,
                        Z_DEFAULT_STRATEGY, ZLIB_VERSION, (int)sizeof(z_stream));
    if (err != Z_OK){
        return nullptr;
    }
    int BUFFER_SIZE_PER_BLOCK=10240;
    vector<BYTE>dest(BUFFER_SIZE_PER_BLOCK);
    stream.next_out = dest.data();
    stream.avail_out = BUFFER_SIZE_PER_BLOCK;
    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    double percentage=0;
    do {
        if (stream.avail_out == 0) {
            dest.resize(dest.size()+BUFFER_SIZE_PER_BLOCK);
            stream.next_out = dest.data()+(dest.size()-BUFFER_SIZE_PER_BLOCK);
            stream.avail_out = BUFFER_SIZE_PER_BLOCK;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = sourceLen > (uLong)max ? max : (uInt)sourceLen;
            sourceLen -= stream.avail_in;
            percentage=1-sourceLen*100.0/inbuf->GetSize();
            CallBackHelper::SendProgress(callBack,percentage);
            if(signal!=nullptr){
                if(signal->Wait(10)){
                    return nullptr;
                }
            }
        }
        err = deflate(&stream, sourceLen ? Z_NO_FLUSH : Z_FINISH);
    } while (err == Z_OK);
    if(err != Z_STREAM_END ){
        return nullptr;
    }
    destLen = stream.total_out;
    deflateEnd(&stream);
    AutoPtr<FixBuffer>fxb(new FixBuffer(destLen));
    memcpy(fxb->GetBuffer(),dest.data(),destLen);
    return fxb;
}

bool GZip::CompressFile(const string&inFile,const string&outFile,int level,CallBack*callBack,Semaphora*signal){
    AutoPtr<BinaryFile>bf(new BinaryFile());
    AutoPtr<BinaryFile>bw(new BinaryFile());
    if(!bf->OpenForRead(inFile)) return false;
    ULONGLONG fileSize=bf->GetFileLength();
    bf->Seek(0);
    int BUFFER_SIZE_PER_BLOCK=10240;
    if(!bw->OpenForWrite(outFile,false)) return false;
    AutoPtr<FixBuffer>cfxb(new FixBuffer(BUFFER_SIZE_PER_BLOCK));

    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;
    err = deflateInit2_(&stream,level,Z_DEFLATED, 31, 8,
                        Z_DEFAULT_STRATEGY, ZLIB_VERSION, (int)sizeof(z_stream));
    if (err != Z_OK) return false;
    uLong rlen=0;
    /*
    gz_header _gzipHead;
    string fileName=FilePath::GetFileName(inFile);
    memset(&_gzipHead, sizeof(gz_header), 0);
    _gzipHead.name =(BYTE*)fileName.c_str();
    _gzipHead.name_max = fileName.size();
    err=deflateSetHeader(&stream, &_gzipHead);
    */

    double percentage=0;
    vector<BYTE>dest(BUFFER_SIZE_PER_BLOCK);
    dest.resize(BUFFER_SIZE_PER_BLOCK);
    stream.next_out = dest.data();
    stream.avail_out = BUFFER_SIZE_PER_BLOCK;
    stream.next_in = nullptr;
    stream.avail_in = 0;
    bool isEof=false;
    do {
        if(err==Z_STREAM_END) break;
        isEof=(rlen>=fileSize);
        if (stream.avail_in == 0) {
            if(signal!=nullptr){
                if(signal->Wait(10)){
                    return false;
                }
            }
            if(rlen<fileSize){
                int curSize=BUFFER_SIZE_PER_BLOCK;
                if(rlen+curSize>=fileSize){
                    curSize=fileSize-rlen;
                }
                bf->Read(cfxb->GetBuffer(),curSize);
                rlen+=curSize;
                stream.avail_in = curSize;
                stream.next_in=(z_const Bytef *)cfxb->GetBuffer();
                percentage=rlen*100.0/fileSize;
                CallBackHelper::SendProgress(callBack,percentage);
            }
        }
        err = deflate(&stream, isEof?Z_FINISH:Z_NO_FLUSH);
        if(!isEof){
            if(err != Z_OK) return false;
        }else{
            if (err != Z_OK && err != Z_STREAM_END) return false;
        }
        if (stream.avail_out == 0) {
            bw->Write((char*)dest.data(),BUFFER_SIZE_PER_BLOCK);
            stream.next_out = dest.data();
            stream.avail_out = BUFFER_SIZE_PER_BLOCK;
        }
    } while (err == Z_OK);
    if(err==Z_STREAM_END){
        if(BUFFER_SIZE_PER_BLOCK-stream.avail_out>0){
            bw->Write((char*)dest.data(),BUFFER_SIZE_PER_BLOCK-stream.avail_out);
        }
    }
    deflateEnd(&stream);
    if(err != Z_STREAM_END ){
        return false;
    }
    return true;
}

bool GZip::UncompressFile(const string&inFile,const string&outFile,int level,CallBack*callBack,Semaphora*signal){
    AutoPtr<BinaryFile>bf(new BinaryFile());
    AutoPtr<BinaryFile>bw(new BinaryFile());
    if(!bf->OpenForRead(inFile)) return false;
    ULONGLONG fileSize=bf->GetFileLength();
    bf->Seek(0);
    int BUFFER_SIZE_PER_BLOCK=10240;
    if(!bw->OpenForWrite(outFile,false)) return false;
    AutoPtr<FixBuffer>cfxb(new FixBuffer(BUFFER_SIZE_PER_BLOCK));

    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err=inflateInit2_(&stream, 31, ZLIB_VERSION, (int)sizeof(z_stream));
    if (err != Z_OK) return false;
    uLong rlen=0;
    /*
    gz_header _gzipHead;
    string fileName=FilePath::GetFileName(inFile);
    memset(&_gzipHead, sizeof(gz_header), 0);
    _gzipHead.name =(BYTE*)fileName.c_str();
    _gzipHead.name_max = fileName.size();
    err=deflateSetHeader(&stream, &_gzipHead);
    */

    double percentage=0;
    vector<BYTE>dest(BUFFER_SIZE_PER_BLOCK);
    dest.resize(BUFFER_SIZE_PER_BLOCK);
    stream.next_out = dest.data();
    stream.avail_out = BUFFER_SIZE_PER_BLOCK;
    stream.next_in = nullptr;
    stream.avail_in = 0;
    bool isEof=false;
    do {
        if(err==Z_STREAM_END) break;
        isEof=(rlen>=fileSize);
        if (stream.avail_in == 0) {
            if(signal!=nullptr){
                if(signal->Wait(10)){
                    return false;
                }
            }
            if(rlen<fileSize){
                int curSize=BUFFER_SIZE_PER_BLOCK;
                if(rlen+curSize>=fileSize){
                    curSize=fileSize-rlen;
                }
                bf->Read(cfxb->GetBuffer(),curSize);
                rlen+=curSize;
                stream.avail_in = curSize;
                stream.next_in=(z_const Bytef *)cfxb->GetBuffer();
                percentage=rlen*100.0/fileSize;
                CallBackHelper::SendProgress(callBack,percentage);
            }
        }
        err = inflate(&stream, isEof?Z_FINISH:Z_NO_FLUSH);
        if(!isEof){
            if(err != Z_OK) return false;
        }else{
            if (err != Z_OK && err != Z_STREAM_END) return false;
        }
        if (stream.avail_out == 0) {
            bw->Write((char*)dest.data(),BUFFER_SIZE_PER_BLOCK);
            stream.next_out = dest.data();
            stream.avail_out = BUFFER_SIZE_PER_BLOCK;
        }
    } while (err == Z_OK);
    if(err==Z_STREAM_END){
        if(BUFFER_SIZE_PER_BLOCK-stream.avail_out>0){
            bw->Write((char*)dest.data(),BUFFER_SIZE_PER_BLOCK-stream.avail_out);
        }
    }
    inflateEnd(&stream);
    if(err != Z_STREAM_END ){
        return false;
    }
    return true;
}

vector<BYTE>GZip::CompressFileToBuffer(const string&inFile,int level,CallBack*callBack,Semaphora*signal){
    AutoPtr<BinaryFile>bf(new BinaryFile());
    if(!bf->OpenForRead(inFile)) return {};
    ULONGLONG fileSize=bf->GetFileLength();
    bf->Seek(0);
    int BUFFER_SIZE_PER_BLOCK=10240;
    AutoPtr<FixBuffer>cfxb(new FixBuffer(BUFFER_SIZE_PER_BLOCK));

    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;
    err = deflateInit2_(&stream,level,Z_DEFLATED, 31, 8,
                        Z_DEFAULT_STRATEGY, ZLIB_VERSION, (int)sizeof(z_stream));
    if (err != Z_OK) return {};
    uLong rlen=0;
    /*
    gz_header _gzipHead;
    string fileName=FilePath::GetFileName(inFile);
    memset(&_gzipHead, sizeof(gz_header), 0);
    _gzipHead.name =(BYTE*)fileName.c_str();
    _gzipHead.name_max = fileName.size();
    err=deflateSetHeader(&stream, &_gzipHead);
    */
    vector<BYTE>result;
    double percentage=0;
    vector<BYTE>dest(BUFFER_SIZE_PER_BLOCK);
    dest.resize(BUFFER_SIZE_PER_BLOCK);
    stream.next_out = dest.data();
    stream.avail_out = BUFFER_SIZE_PER_BLOCK;
    stream.next_in = nullptr;
    stream.avail_in = 0;
    bool isEof=false;
    do {
        if(err==Z_STREAM_END) break;
        isEof=(rlen>=fileSize);
        if (stream.avail_in == 0) {
            if(signal!=nullptr){
                if(signal->Wait(10)){
                    return {};
                }
            }
            if(rlen<fileSize){
                int curSize=BUFFER_SIZE_PER_BLOCK;
                if(rlen+curSize>=fileSize){
                    curSize=fileSize-rlen;
                }
                bf->Read(cfxb->GetBuffer(),curSize);
                rlen+=curSize;
                stream.avail_in = curSize;
                stream.next_in=(z_const Bytef *)cfxb->GetBuffer();
                percentage=rlen*100.0/fileSize;
                CallBackHelper::SendProgress(callBack,percentage);
            }
        }
        err = deflate(&stream, isEof?Z_FINISH:Z_NO_FLUSH);
        if(!isEof){
            if(err != Z_OK) return {};
        }else{
            if (err != Z_OK && err != Z_STREAM_END) return {};
        }
        if (stream.avail_out == 0) {
            result.resize(result.size()+BUFFER_SIZE_PER_BLOCK);
            memcpy(result.data()+result.size()-BUFFER_SIZE_PER_BLOCK,dest.data(),BUFFER_SIZE_PER_BLOCK);
            stream.next_out = dest.data();
            stream.avail_out = BUFFER_SIZE_PER_BLOCK;
        }
    } while (err == Z_OK);
    if(err==Z_STREAM_END){
        if(BUFFER_SIZE_PER_BLOCK-stream.avail_out>0){
            int obys=BUFFER_SIZE_PER_BLOCK-stream.avail_out;
            result.resize(result.size()+obys);
            memcpy(result.data()+result.size()-obys,dest.data(),obys);
        }
    }
    deflateEnd(&stream);
    if(err != Z_STREAM_END ){
        return {};
    }
    return result;
}

AutoPtr<FixBuffer>GZip::UncompressByDeflate(AutoPtr<FixBuffer>inbuf,CallBack*callBack,Semaphora*signal){
    const Bytef *source=(BYTE*)inbuf->GetBuffer();
    uLong sourceLen=inbuf->GetSize();
    int BUFFER_SIZE_PER_BLOCK=10240;
    vector<BYTE>dest(BUFFER_SIZE_PER_BLOCK);
    uLong destLen = 0;

    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong len, left;
    len = sourceLen;
    left = destLen;

    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err = inflateInit(&stream);
    if (err != Z_OK) return nullptr;

    stream.next_out = dest.data();
    stream.avail_out = BUFFER_SIZE_PER_BLOCK;
    double percentage;
    do {
        if (stream.avail_out == 0) {
            dest.resize(dest.size()+BUFFER_SIZE_PER_BLOCK);
            stream.next_out = dest.data()+(dest.size()-BUFFER_SIZE_PER_BLOCK);
            stream.avail_out = BUFFER_SIZE_PER_BLOCK;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = len > (uLong)max ? max : (uInt)len;
            len -= stream.avail_in;
            percentage=1-len*100.0/inbuf->GetSize();
            CallBackHelper::SendProgress(callBack,percentage);
            if(signal!=nullptr){
                if(signal->Wait(10)){
                    return nullptr;
                }
            }
        }
        err = inflate(&stream, Z_NO_FLUSH);
    } while (err == Z_OK);

    sourceLen -= len + stream.avail_in;
    destLen = stream.total_out;
    inflateEnd(&stream);
    if(err != Z_STREAM_END ){
        return nullptr;
    }
    AutoPtr<FixBuffer>fxb(new FixBuffer(destLen));
    memcpy(fxb->GetBuffer(),dest.data(),destLen);
    return fxb;
}

AutoPtr<FixBuffer>GZip::UncompressByGZip(AutoPtr<FixBuffer>inbuf,CallBack*callBack,Semaphora*signal){
    const Bytef *source=(BYTE*)inbuf->GetBuffer();
    uLong sourceLen=inbuf->GetSize();
    int BUFFER_SIZE_PER_BLOCK=10240;
    vector<BYTE>dest(BUFFER_SIZE_PER_BLOCK);
    uLong destLen = 0;

    z_stream stream;
    int err;
    const uInt max = (uInt)-1;
    uLong len, left;
    len = sourceLen;
    left = destLen;

    stream.next_in = (z_const Bytef *)source;
    stream.avail_in = 0;
    stream.zalloc = (alloc_func)0;
    stream.zfree = (free_func)0;
    stream.opaque = (voidpf)0;

    err=inflateInit2_(&stream, 31, ZLIB_VERSION, (int)sizeof(z_stream));
    if (err != Z_OK) return nullptr;

    stream.next_out = dest.data();
    stream.avail_out = BUFFER_SIZE_PER_BLOCK;
    double percentage;
    do {
        if (stream.avail_out == 0) {
            dest.resize(dest.size()+BUFFER_SIZE_PER_BLOCK);
            stream.next_out = dest.data()+(dest.size()-BUFFER_SIZE_PER_BLOCK);
            stream.avail_out = BUFFER_SIZE_PER_BLOCK;
            left -= stream.avail_out;
        }
        if (stream.avail_in == 0) {
            stream.avail_in = len > (uLong)max ? max : (uInt)len;
            len -= stream.avail_in;
            percentage=1-len*100.0/inbuf->GetSize();
            CallBackHelper::SendProgress(callBack,percentage);
            if(signal!=nullptr){
                if(signal->Wait(10)){
                    return nullptr;
                }
            }
        }
        err = inflate(&stream, Z_NO_FLUSH);
    } while (err == Z_OK);

    sourceLen -= len + stream.avail_in;
    destLen = stream.total_out;
    inflateEnd(&stream);
    if(err != Z_STREAM_END ){
        return nullptr;
    }
    AutoPtr<FixBuffer>fxb(new FixBuffer(destLen));
    memcpy(fxb->GetBuffer(),dest.data(),destLen);
    return fxb;
}


/*
GZipFileCompress::GZipFileCompress(string infile,bool gzip,int level){
     BUFFER_SIZE_PER_BLOCK=10240;
     bf=new BinaryFile();
     fxb=new FixBuffer(BUFFER_SIZE_PER_BLOCK);
     cfxb=new FixBuffer();
     ofxb=new FixBuffer();
     this->infile=infile;
     z_stream*sstream=new z_stream();
     stream=sstream;
     this->level=level;
     this->gzip=gzip;
     dest.resize(BUFFER_SIZE_PER_BLOCK);
}

GZipFileCompress::~GZipFileCompress(){
    z_stream*sstream=(z_stream*)stream;
    delete sstream;
}

bool GZipFileCompress::BeginCompress(){
    if(!bf->OpenForRead(infile)) return false;
    fileSize=bf->GetFileLength();
    bf->Seek(0);
    z_stream*sstream=(z_stream*)stream;
    sstream->zalloc = (alloc_func)0;
    sstream->zfree = (free_func)0;
    sstream->opaque = (voidpf)0;
    err = deflateInit2_(sstream,level,Z_DEFLATED, 15+(gzip?16:0), 8,
                        Z_DEFAULT_STRATEGY, ZLIB_VERSION, (int)sizeof(z_stream));
    if (err != Z_OK) return false;
    sstream->next_out = dest.data();
    sstream->avail_out = BUFFER_SIZE_PER_BLOCK;
    filePin=0;
    outputBytes=0;
    inputBytes=0;
    sstream->next_in = (z_const Bytef *)fxb->GetBuffer();
    sstream->avail_in = 0;
    err=0;
    return true;
}

bool GZipFileCompress::readBlock(){
    int curSize=BUFFER_SIZE_PER_BLOCK;
    if(filePin<fileSize){
        if(filePin+curSize>fileSize) curSize=fileSize-filePin;
        cfxb->Attach(fxb->GetBuffer(),curSize);
        bf->Read(cfxb->GetBuffer(),curSize);
        inputBytes=curSize;
        filePin+=curSize;
        return true;
    }
    return false;
}

bool GZipFileCompress::Reset(){
    if(stream==nullptr) return false;
    bf->Seek(0);
    z_stream*sstream=(z_stream*)stream;
    delete sstream;
    sstream=new z_stream();
    stream=sstream;
    sstream->zalloc = (alloc_func)0;
    sstream->zfree = (free_func)0;
    sstream->opaque = (voidpf)0;
    err = deflateInit2_(sstream,level,Z_DEFLATED, 15+(gzip?16:0), 8,
                        Z_DEFAULT_STRATEGY, ZLIB_VERSION, (int)sizeof(z_stream));
    if (err != Z_OK) return false;
    sstream->next_out = dest.data();
    sstream->avail_out = BUFFER_SIZE_PER_BLOCK;
    filePin=0;
    outputBytes=0;
    inputBytes=0;
    sstream->next_in = (z_const Bytef *)fxb->GetBuffer();
    sstream->avail_in = 0;
    err=0;
    return true;
}

AutoPtr<FixBuffer>GZipFileCompress::GetNextBuffer(){
    if (err != Z_OK && err != Z_STREAM_END) return nullptr;
    z_stream*sstream=(z_stream*)stream;
    bool isEof=false;
    do {
        if(err==Z_STREAM_END) break;
        if (sstream->avail_in == 0) {
            isEof=(!readBlock());
            if(!isEof){
                sstream->avail_in = inputBytes;
                sstream->next_in = (z_const Bytef *)fxb->GetBuffer();
            }
        }
        err = deflate(sstream, isEof?Z_FINISH:Z_NO_FLUSH);
        if(!isEof){
            if(err != Z_OK) return nullptr;
        }else{
            if (err != Z_OK && err != Z_STREAM_END) return nullptr;
        }
        if (sstream->avail_out == 0) {
            outputBytes=BUFFER_SIZE_PER_BLOCK;
            sstream->next_out = dest.data();
            sstream->avail_out = BUFFER_SIZE_PER_BLOCK;
            ofxb->Attach((char*)dest.data(),outputBytes);
            return ofxb;
        }
    }while (err == Z_OK);
    if(BUFFER_SIZE_PER_BLOCK-sstream->avail_out>0){
        outputBytes=BUFFER_SIZE_PER_BLOCK-sstream->avail_out;
        sstream->avail_out=BUFFER_SIZE_PER_BLOCK;
        ofxb->Attach((char*)dest.data(),outputBytes);
        return ofxb;
    }
    return nullptr;
}

int GZipFileCompress::GetBufferSizePerBlock(){
    return BUFFER_SIZE_PER_BLOCK;
}

GZipStreamCompress::GZipStreamCompress(AutoPtr<FixBuffer>ffxb,bool gzip,int level){
     BUFFER_SIZE_PER_BLOCK=10240;
     this->fxb=ffxb;
     cfxb=new FixBuffer();
     z_stream*sstream=new z_stream();
     stream=sstream;
     this->level=level;
     this->gzip=gzip;
     dest.resize(BUFFER_SIZE_PER_BLOCK);
}

GZipStreamCompress::~GZipStreamCompress(){
    z_stream*sstream=(z_stream*)stream;
    delete sstream;
}

bool GZipStreamCompress::BeginCompress(){
    fileSize=fxb->GetSize();
    z_stream*sstream=(z_stream*)stream;
    sstream->zalloc = (alloc_func)0;
    sstream->zfree = (free_func)0;
    sstream->opaque = (voidpf)0;
    err = deflateInit2_(sstream,level,Z_DEFLATED, 15+(gzip?16:0), 8,
                        Z_DEFAULT_STRATEGY, ZLIB_VERSION, (int)sizeof(z_stream));
    if (err != Z_OK) return false;
    sstream->next_out = dest.data();
    sstream->avail_out = BUFFER_SIZE_PER_BLOCK;
    filePin=0;
    outputBytes=0;
    inputBytes=0;
    sstream->next_in = (z_const Bytef *)fxb->GetBuffer();
    sstream->avail_in = 0;
    err=0;
    return true;
}

bool GZipStreamCompress::readBlock(){
    int curSize=BUFFER_SIZE_PER_BLOCK;
    if(filePin<fileSize){
        if(filePin+curSize>fileSize) curSize=fileSize-filePin;
        cfxb->Attach(fxb->GetBuffer()+filePin,curSize);
        inputBytes=curSize;
        filePin+=curSize;
        return true;
    }
    return false;
}

int GZipStreamCompress::GetBufferSizePerBlock(){
    return BUFFER_SIZE_PER_BLOCK;
}

bool GZipStreamCompress::Reset(){
    if(stream==nullptr) return false;
    z_stream*sstream=(z_stream*)stream;
    delete sstream;
    sstream=new z_stream();
    stream=sstream;
    sstream->zalloc = (alloc_func)0;
    sstream->zfree = (free_func)0;
    sstream->opaque = (voidpf)0;
    err = deflateInit2_(sstream,level,Z_DEFLATED, 15+(gzip?16:0), 8,
                        Z_DEFAULT_STRATEGY, ZLIB_VERSION, (int)sizeof(z_stream));
    if (err != Z_OK) return false;
    sstream->next_out = dest.data();
    sstream->avail_out = BUFFER_SIZE_PER_BLOCK;
    filePin=0;
    outputBytes=0;
    inputBytes=0;
    sstream->next_in = (z_const Bytef *)fxb->GetBuffer();
    sstream->avail_in = 0;
    err=0;
    return true;
}

AutoPtr<FixBuffer>GZipStreamCompress::GetNextBuffer(){
    if (err != Z_OK && err != Z_STREAM_END) return nullptr;
    z_stream*sstream=(z_stream*)stream;
    bool isEof=false;
    do {
        if(err==Z_STREAM_END) break;
        if (sstream->avail_in == 0) {
            isEof=(!readBlock());
            if(!isEof){
                sstream->avail_in = inputBytes;
                sstream->next_in = (z_const Bytef *)cfxb->GetBuffer();
            }
        }
        err = deflate(sstream, isEof?Z_FINISH:Z_NO_FLUSH);
        if(!isEof){
            if(err != Z_OK) return nullptr;
        }else{
            if (err != Z_OK && err != Z_STREAM_END) return nullptr;
        }
        if (sstream->avail_out == 0) {
            outputBytes=BUFFER_SIZE_PER_BLOCK;
            sstream->next_out = dest.data();
            sstream->avail_out = BUFFER_SIZE_PER_BLOCK;
            ofxb->Attach((char*)dest.data(),outputBytes);
            return ofxb;
        }
    }while (err == Z_OK);
    if(BUFFER_SIZE_PER_BLOCK-sstream->avail_out>0){
        outputBytes=BUFFER_SIZE_PER_BLOCK-sstream->avail_out;
        sstream->avail_out=BUFFER_SIZE_PER_BLOCK;
        ofxb->Attach((char*)dest.data(),outputBytes);
        return ofxb;
    }
    return nullptr;
}
*/

bool AddDirToZip(zipFile zf, const std::string& relative){
    zip_fileinfo zi{ 0 };
    memset(&zi, 0, sizeof(zip_fileinfo));
    int ret{ ZIP_ERRNO };
    std::string newRelative { relative + "/" };
    ret = zipOpenNewFileInZip(zf, newRelative.c_str(), &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    if (ZIP_OK != ret)
    {
        return false;
    }
    ret = zipCloseFileInZip(zf);
    return ret == ZIP_OK;
}

bool InnerWriteFileToZip(zipFile zf, const std::string& path)
{
    FILE* fps = fopen(path.c_str(), "rb");
    if (NULL == fps)
    {
        return false;
    }
    int err{ ZIP_ERRNO };
    do
    {
        enum {MAX_BUFFER = 40960 };
        char buf[MAX_BUFFER];
        size_t nRead{ 0 };
        while (!feof(fps))
        {
            nRead = fread(buf, 1, sizeof(buf), fps);
            err = zipWriteInFileInZip(zf, buf, nRead);
            if (ZIP_OK != err)
            {
                break;
            }
            if (ferror(fps))
            {
                err = ZIP_ERRNO;
                break;
            }
        }
    } while (0);
    fclose(fps);
    return ZIP_OK == err;
}

bool AddFileToZip(zipFile zf, const std::string& relativeInZip, const std::string& sourcePath)
{
    FILE* fps{ NULL };
    int err{ ZIP_ERRNO };
    bool ret{ false };
    zip_fileinfo zi { {0} };
    memset(&zi, 0, sizeof(zip_fileinfo));
    std::string newFileName{ relativeInZip };
    err = zipOpenNewFileInZip(zf, newFileName.c_str(), &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);
    if (ZIP_OK != err){
        return false;
    }
    ret = InnerWriteFileToZip(zf, sourcePath);
    err = zipCloseFileInZip(zf);
    return ret && (ZIP_OK == err);
}

bool innerZipDir(zipFile zf,const string&dir,const std::string& relative,CallBack*callBack,Semaphora*signal){
    string fullPath=dir+"/"+relative;
    AddDirToZip(zf,relative);
    FileDirectory fd(fullPath);
    vector<string>dirs=fd.FindDirectories();
    for(int k=0;k<dirs.size();k++){
        if(!innerZipDir(zf,dir,relative+"/"+FilePath::GetFileName(dirs[k]),callBack,signal)) return false;
    }
    vector<string>files=fd.FindFiles("*");
    for(int k=0;k<files.size();k++){
        CallBackHelper::SendMessageA(callBack,"压缩"+files[k]);
        if(!AddFileToZip(zf,relative+"/"+FilePath::GetFileName(files[k]),files[k])) return false;
        if(signal!=nullptr){
            if(signal->Wait(10)){
                return false;
            }
        }
    }
    return true;
}

bool Zip::ZipDir(const string&InDir,const string&OutFile,CallBack*callBack,Semaphora*signal){
    string inDir=FilePath::ConvertToStardardPath(StringHelper::ToLocalString(InDir));
    string outFile=FilePath::ConvertToStardardPath(StringHelper::ToLocalString(OutFile));
    if (!FilePath::IsDirExisting(inDir)){
        return false;
    }
    int ret { ZIP_ERRNO };
    zipFile zf = zipOpen(outFile.c_str(), APPEND_STATUS_CREATE);
    if ( zf == NULL )  return false;
    string relative=FilePath::GetFileName(inDir);
    innerZipDir(zf,FilePath::GetDir(inDir),relative,callBack,signal);
    ret = zipClose(zf, NULL);
    return ZIP_OK == ret;
}

bool Zip::ZipFiles(const vector<string>&pathnames,const string&OutFile,CallBack*callBack,Semaphora*signal){
    string outFile=FilePath::ConvertToStardardPath(StringHelper::ToLocalString(OutFile));
    int ret { ZIP_ERRNO };
    zipFile zf = zipOpen(outFile.c_str(), APPEND_STATUS_CREATE);
    if ( zf == NULL )  return false;
    for(int k=0;k<pathnames.size();k++){
        CallBackHelper::SendMessageA(callBack,"压缩"+pathnames[k]);
        string fileName=StringHelper::ToLocalString(pathnames[k]);
        fileName=FilePath::ConvertToStardardPath(fileName);
        if(!AddFileToZip(zf,FilePath::GetFileName(fileName),fileName)) return false;
        CallBackHelper::SendProgress(callBack,k*100.0/pathnames.size());
        if(signal!=nullptr){
            if(signal->Wait(10)){
                zipClose(zf,NULL);
                return false;
            }
        }
    }
    zipClose(zf,NULL);
    return true;
}

bool Zip::UnZipFiles(const string&inFile,const string&OutDir,CallBack*callBack,Semaphora*signal){
    string inputFile=FilePath::ConvertToStardardPath(StringHelper::ToLocalString(inFile));
    string outDir=FilePath::ConvertToStardardPath(StringHelper::ToLocalString(OutDir));
    unzFile zipfile = unzOpen((const char*)inputFile.c_str());
    if ( zipfile == NULL )  return false;
    unz_global_info global_info;
    if ( unzGetGlobalInfo( zipfile, &global_info ) != UNZ_OK )
    {
        unzClose( zipfile );
        return false;
    }
    const int READ_SIZE=10240;
    const int MAX_FILENAME=512;
    char read_buffer[ READ_SIZE ];
    uLong i;
    for ( i = 0; i < global_info.number_entry; ++i ){
        unz_file_info file_info;
        char filename[ MAX_FILENAME ];
        if ( unzGetCurrentFileInfo(
                 zipfile,
                 &file_info,
                 filename,
                 MAX_FILENAME,
                 NULL, 0, NULL, 0 ) != UNZ_OK ){
            unzClose( zipfile );
            return false;
        }
        const size_t filename_length = strlen( filename );
        if (filename[ filename_length-1 ] == '/'){
            string sFileName=filename;
            sFileName=sFileName.substr(0,filename_length-1);
            FileDirectory fd(outDir+"/"+sFileName);
            fd.CreateFolder();
        }
        else{
            if ( unzOpenCurrentFile( zipfile ) != UNZ_OK ){
                unzClose( zipfile );
                return false;
            }
            string sFileName=filename;
            sFileName=outDir+"/"+sFileName;
            CallBackHelper::SendMessageA(callBack,"解压"+sFileName);
            FILE *out = fopen( sFileName.c_str(), "wb" );
            if ( out == NULL ){
                unzCloseCurrentFile( zipfile );
                unzClose( zipfile );
                return false;
            }
            int error = UNZ_OK;
            do    {
                error = unzReadCurrentFile( zipfile, read_buffer, READ_SIZE );
                if ( error < 0 ){
                    unzCloseCurrentFile( zipfile );
                    unzClose( zipfile );
                    return false;
                }
                if ( error > 0 ){
                    fwrite( read_buffer, error, 1, out );
                }
                if(signal!=nullptr){
                    if(signal->Wait(10)){
                        return false;
                    }
                }
            } while ( error > 0 );
            fclose( out );
        }
        unzCloseCurrentFile( zipfile );
        if ( ( i+1 ) < global_info.number_entry ){
            if ( unzGoToNextFile( zipfile ) != UNZ_OK ){
                unzClose( zipfile );
                return -1;
            }
        }
        CallBackHelper::SendProgress(callBack,i*100.0/global_info.number_entry);
    }
    unzClose( zipfile );
    return true;
}

AutoPtr<FixBuffer> BZip2::UnZipFile(const string&pathName,CallBack*callBack,Semaphora*signal){
    int bzerror;
    string pathname=StringHelper::ToLocalString(pathName);
    //string outfile=StringHelper::ToLocalString(outFile);
    FILE *fp = fopen(pathname.c_str(), "rb");
    if (!fp) {
        CallBackHelper::SendMessageA(callBack,"打开文件失败");
        return nullptr;
    }
    BZFILE *bzFp = BZ2_bzReadOpen(&bzerror, fp, 4, 0, nullptr, 0);
    if (bzerror != BZ_OK) {
        BZ2_bzReadClose(&bzerror, bzFp);
        fclose(fp);
        CallBackHelper::SendMessageA(callBack,"打开文件失败");
        return nullptr;
    }
    bool bSuc=true;
    vector<char>orginVec;
    char buffer[1024];
    while (bzerror == BZ_OK) {
        short orginData = 0;
        int bitTrans=BZ2_bzRead(&bzerror, bzFp, buffer, 1024);
        if (bzerror == BZ_OK) {
            int formerSize=orginVec.size();
            orginVec.resize(formerSize+bitTrans);
            memcpy(orginVec.data()+formerSize,buffer,bitTrans);
        } else if (bzerror == BZ_STREAM_END) {
            break;
        } else {
            if (bzerror == BZ_DATA_ERROR) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败");
                bSuc=false;
                break;
            } else if (bzerror == BZ_PARAM_ERROR) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败，参数错误");
                bSuc=false;
                break;
            } else if (bzerror == BZ_SEQUENCE_ERROR) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败，序列化错误");
                bSuc=false;
                break;
            } else if (bzerror == BZ_IO_ERROR) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败，IO错误");
                bSuc=false;
                break;
            } else if (bzerror == BZ_UNEXPECTED_EOF) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败，意外的文件尾");
                bSuc=false;
                break;
            } else if (bzerror == BZ_DATA_ERROR_MAGIC) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败，数据异常");
                bSuc=false;
                break;
            } else if (bzerror == BZ_MEM_ERROR) {
                CallBackHelper::SendMessageA(callBack,"内存错误");
                bSuc=false;
                break;
            }
        }
        if(signal!=nullptr){
            if(signal->Wait(1)){
                CallBackHelper::SendMessageA(callBack,"用户取消");
                bSuc=false;
                break;
            }
        }
    }
    BZ2_bzReadClose(&bzerror, bzFp);
    fclose(fp);
    if(!bSuc) return nullptr;
    AutoPtr<FixBuffer>fxb=new FixBuffer(orginVec.size());
    memcpy(fxb->GetBuffer(),orginVec.data(),orginVec.size());
    return fxb;
}


bool BZip2::UnZipFile(const string&pathName,const string&outFile,CallBack*callBack,Semaphora*signal){
    int bzerror;
    string pathname=StringHelper::ToLocalString(pathName);
    string outfile=StringHelper::ToLocalString(outFile);
    FILE *fp = fopen(pathname.c_str(), "rb");
    if (!fp) {
        CallBackHelper::SendMessageA(callBack,"打开文件失败");
        return false;
    }
    BZFILE *bzFp = BZ2_bzReadOpen(&bzerror, fp, 4, 0, nullptr, 0);
    if (bzerror != BZ_OK) {
        BZ2_bzReadClose(&bzerror, bzFp);
        fclose(fp);
        CallBackHelper::SendMessageA(callBack,"打开文件失败");
        return false;
    }
    bool bSuc=true;
    AutoPtr<BinaryFile>bf(new BinaryFile());
    if(!bf->OpenForWrite(outfile,false)){
        CallBackHelper::SendMessageA(callBack,"新建解压文件失败");
        return false;
    }
    char buffer[1024];
    while (bzerror == BZ_OK) {
        int bitTrans=BZ2_bzRead(&bzerror, bzFp, buffer, 1024);
        if (bzerror == BZ_OK) {
            bf->Write(buffer,bitTrans);
        } else if (bzerror == BZ_STREAM_END) {
            if(bitTrans>0) bf->Write(buffer,bitTrans);
            break;
        } else {
            if (bzerror == BZ_DATA_ERROR) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败");
                bSuc=false;
                break;
            } else if (bzerror == BZ_PARAM_ERROR) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败，参数错误");
                bSuc=false;
                break;
            } else if (bzerror == BZ_SEQUENCE_ERROR) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败，序列化错误");
                bSuc=false;
                break;
            } else if (bzerror == BZ_IO_ERROR) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败，IO错误");
                bSuc=false;
                break;
            } else if (bzerror == BZ_UNEXPECTED_EOF) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败，意外的文件尾");
                bSuc=false;
                break;
            } else if (bzerror == BZ_DATA_ERROR_MAGIC) {
                CallBackHelper::SendMessageA(callBack,"读取数据失败，数据异常");
                bSuc=false;
                break;
            } else if (bzerror == BZ_MEM_ERROR) {
                CallBackHelper::SendMessageA(callBack,"内存错误");
                bSuc=false;
                break;
            }
        }
        if(signal!=nullptr){
            if(signal->Wait(1)){
                CallBackHelper::SendMessageA(callBack,"用户取消");
                bSuc=false;
                break;
            }
        }
    }
    BZ2_bzReadClose(&bzerror, bzFp);
    fclose(fp);
    if(!bSuc) return false;
    return true;
}



};
