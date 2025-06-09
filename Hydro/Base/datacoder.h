#ifndef DATACODER_H
#define DATACODER_H

#include "autoptr.h"
#include "bufferio.h"

namespace SGIS{


class SGIS_EXPORT DataEncoder
{
public:
    DataEncoder(unsigned char*bufferData,long bufferDataSize)
    {
         buffer=bufferData;
         bufferSize=bufferDataSize;
    };
    virtual~DataEncoder()
    {
    };
    int GetCoderType()
    {
        return CoderType;
    };
    virtual long ComputeEncodeLength()=0;
    virtual AutoPtr<FixBuffer>Encode(int PreLen)=0;
protected:
    unsigned char*buffer;
    long bufferSize;
    int CoderType;
};

class SGIS_EXPORT DataDecoder
{
public:
    DataDecoder(unsigned char*bufferData,long bufferDataSize)
    {
        buffer=bufferData;
         bufferSize=bufferDataSize;
    };
    virtual~DataDecoder()
    {
    };
    int GetCoderType()
    {
        return CoderType;
    };
    virtual AutoPtr<FixBuffer>Decode(int PreLen)=0;
protected:
    unsigned char*buffer;
    long bufferSize;
    int CoderType;
};
// DataPackage.h : CRle2DataPackageCoder 的声明
// CRle2DataPackageCoder
class SGIS_EXPORT Rle2Encoder :public DataEncoder
{
public:
    Rle2Encoder(unsigned char*bufferData,long bufferDataSize);
    virtual~Rle2Encoder();
    long ComputeEncodeLength();
    AutoPtr<FixBuffer>Encode(int PreLen);
    bool Encode(int PreLen,BYTE*newData);
protected:
    LONG ComputeEncodeLength(int fromPos,int Dif);
    LONG ComputeEncodeLength(int Dif);
    void Encode(int fromPos,int Dif,long&fromPin,BYTE*buffer);
protected:
    LONG CurrentDif;
    LONG dataLength;
};

class SGIS_EXPORT Rle2Decoder :public DataDecoder
{
public:
    Rle2Decoder(unsigned char*bufferData,long bufferDataSize);
    virtual~Rle2Decoder();
    LONG GetDataLength(int PreLen);
    AutoPtr<FixBuffer>Decode(int PreLen);
    bool Decode(int PreLen,BYTE*newData);
};

class SGIS_EXPORT Base64Encrypt
{
public:
    Base64Encrypt();
    Base64Encrypt(const void *input, size_t length);
    void Update(const void *input, size_t length);
    const unsigned char *CipherText();
    std::string GetString();
    void Reset();
    size_t GetSize();
private:
    Base64Encrypt(const Base64Encrypt &) = delete;
    Base64Encrypt & operator = (const Base64Encrypt &) = delete;
    void Encode();
    void Final();
private:
    std::vector<unsigned char> _buf;
    unsigned char _group[3];
    int _groupLength;
    static const unsigned char Base64EncodeMap[64];
};

class SGIS_EXPORT Base64Decrypt
{
public:
    Base64Decrypt();
    Base64Decrypt(const void *input, size_t length);
    void Update(const void *input, size_t length);
    const unsigned char *PlainText();
    void Reset();
    size_t GetSize();
private:
    Base64Decrypt(const Base64Decrypt &) = delete;
    Base64Decrypt & operator = (const Base64Decrypt &) = delete;
    void Decode();
private:
    std::vector<unsigned char> _buf;
    unsigned char _group[4];
    int _groupLength;
    static unsigned char Base64DecodeMap[256];
};


};

#endif // DATACODER_H
