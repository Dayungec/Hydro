#include "datacoder.h"
namespace SGIS{

Rle2Encoder::Rle2Encoder(unsigned char*bufferData,long bufferDataSize)
    : DataEncoder(bufferData,bufferDataSize)
{
    CoderType=1;
    dataLength=0;
}
Rle2Encoder::~Rle2Encoder()
{

}

LONG Rle2Encoder::ComputeEncodeLength(int fromPos,int Dif)
{
    int RunLength=1;
    int DifNum=1;
    BYTE formerValue=buffer[fromPos];
    long AllLength=0;
    for(int k=fromPos+Dif;k<bufferSize;k+=Dif)
    {
         BYTE bV=buffer[k];
         if(formerValue==bV)
         {
             RunLength++;
             if((DifNum>1)&&(RunLength>2))
             {
                 AllLength+=DifNum;
                 DifNum=1;
             }
             if(RunLength>128)
             {
                 AllLength+=2;
                 RunLength=1;
                 DifNum=1;
             }
         }
         else
         {
             if(RunLength>2)
             {
                 AllLength+=2;
                 DifNum=1;
             }
             else
                 DifNum+=RunLength;
             if(DifNum>129)
             {
                 AllLength+=130;
                 DifNum=DifNum-129;
             }
             RunLength=1;
             formerValue=bV;
         }
    }
    if((DifNum==1)&&(RunLength>2))
        AllLength+=2;
    else
    {
        DifNum=DifNum+RunLength-1;
        if(DifNum<=129)
        {
            AllLength+=DifNum+1;
        }
        else
        {

            DifNum=DifNum-129;
            AllLength+=131+DifNum;
        }
    }
    return AllLength;
}
LONG Rle2Encoder::ComputeEncodeLength(int Dif)
{
    if(bufferSize<=Dif) return 0;
    int AllLength=0;
    for(int k=0;k<Dif;k++) AllLength+=ComputeEncodeLength(k,Dif);
    return AllLength;
}
LONG Rle2Encoder::ComputeEncodeLength()
{
    if(dataLength>0) return dataLength;
    long AllLength=5;
    //前4个字节表示原长度，后面1个字节表示间隔
    long minlen=ComputeEncodeLength(8);
    CurrentDif=8;
    long len=ComputeEncodeLength(16);
    if((minlen>len)||(minlen==0))
    {
        minlen=len;
        CurrentDif=16;
    }
    if(minlen==0)
        return 0;
    else
    {
        LONG dataLength=AllLength+minlen;
        return dataLength;
    }
    return 0;
}

void Rle2Encoder::Encode(int fromPos,int Dif,long&fromPin,BYTE*pNewData)
{
    int RunLength=1;
    int DifNum=1;
    BYTE formerValue=buffer[fromPos];
    int k;
    for(k=fromPos+Dif;k<bufferSize;k+=Dif)
    {
         BYTE bV=buffer[k];
         if(formerValue==bV)
         {
             RunLength++;
             if((DifNum>1)&&(RunLength>2))
             {
                 pNewData[fromPin++]=DifNum-1;
                 int num=DifNum-1;
                 for(int j=num-1;j>=0;j--) pNewData[fromPin++]=buffer[k-RunLength*Dif-j*Dif];
                 DifNum=1;
             }
             if(RunLength>128)
             {
                 pNewData[fromPin++]=255;
                 pNewData[fromPin++]=bV;
                 RunLength=1;
                 DifNum=1;
             }
         }
         else
         {
             if(RunLength>2)
             {
                 pNewData[fromPin++]=127+RunLength;
                 pNewData[fromPin++]=formerValue;
                 DifNum=1;
             }
             else
                 DifNum+=RunLength;
             if(DifNum>129)
             {
                 pNewData[fromPin++]=129;
                 DifNum=DifNum-129;
                 for(int j=128;j>=0;j--) pNewData[fromPin++]=buffer[k-DifNum*Dif-j*Dif];
             }
             RunLength=1;
             formerValue=bV;
         }
    }
    if((DifNum==1)&&(RunLength>2))
    {
        pNewData[fromPin++]=127+RunLength;
        pNewData[fromPin++]=formerValue;
    }
    else
    {
        DifNum=DifNum+RunLength-1;
        k=k-Dif;
        if(DifNum<=129)
        {
            pNewData[fromPin++]=DifNum;
            for(int j=DifNum-1;j>=0;j--) pNewData[fromPin++]=buffer[k-j*Dif];
        }
        else
        {
            DifNum=DifNum-129;
            pNewData[fromPin++]=129;
            for(int j=128;j>=0;j--) pNewData[fromPin++]=buffer[k-DifNum*Dif-j*Dif];
            pNewData[fromPin++]=DifNum;
            for(int j=DifNum-1;j>=0;j--) pNewData[fromPin++]=buffer[k-j*Dif];
        }
    }
}

AutoPtr<FixBuffer>Rle2Encoder::Encode(int PreLen)
{
    long AllLength=ComputeEncodeLength();
    if(AllLength==0){
        BYTE*newBuffer=new BYTE[bufferSize+4];
        if(newBuffer==nullptr) return nullptr;
        memset(newBuffer,0,4);
        memcpy(newBuffer+4,buffer,bufferSize);
        AutoPtr<FixBuffer>result(new FixBuffer((char*)newBuffer,bufferSize+4));
        return result;
    }
    if(AllLength>=bufferSize){
        BYTE*newBuffer=new BYTE[bufferSize+4];
        if(newBuffer==nullptr) return nullptr;
        memset(newBuffer,0,4);
        memcpy(newBuffer+4,buffer,bufferSize);
        AutoPtr<FixBuffer>result(new FixBuffer((char*)newBuffer,bufferSize+4));
        return result;
    }
    AutoPtr<FixBuffer>newData(new FixBuffer(AllLength+PreLen));
    if(newData==nullptr) return nullptr;
    BYTE*pNewData=(BYTE*)newData->GetBuffer();
    memcpy(pNewData+PreLen,&bufferSize,4);
    AllLength=4+PreLen;
    pNewData[AllLength++]=CurrentDif;
    for(int k=0;k<CurrentDif;k++)
    {
        Encode(k,CurrentDif,AllLength,pNewData);
    }
    return newData;
}

bool Rle2Encoder::Encode(int PreLen,BYTE*pNewData)
{
    long AllLength=ComputeEncodeLength();
    if(AllLength==0) return false;
    if((float)AllLength/bufferSize>=0.95) return false;
    memcpy(pNewData+PreLen,&bufferSize,4);
    AllLength=4+PreLen;
    pNewData[AllLength++]=CurrentDif;
    for(int k=0;k<CurrentDif;k++)
    {
        Encode(k,CurrentDif,AllLength,pNewData);
    }
    return true;
}

Rle2Decoder::Rle2Decoder(unsigned char*bufferData,long bufferDataSize)
    :DataDecoder(bufferData,bufferDataSize)
{

}
Rle2Decoder::~Rle2Decoder()
{

}
LONG Rle2Decoder::GetDataLength(int PreLen)
{
    LONG dataLength;
    memcpy(&dataLength,buffer+PreLen,4);
    return dataLength;
}
AutoPtr<FixBuffer>Rle2Decoder::Decode(int PreLen)
{
    long dataLength;
    memcpy(&dataLength,buffer+PreLen,4);
    if(dataLength==0){
        dataLength=this->bufferSize-4;
        AutoPtr<FixBuffer>newData(new FixBuffer(dataLength+PreLen));
        BYTE*pNewData=(BYTE*)newData->GetBuffer();
        if(newData==NULL) return NULL;
        memcpy(pNewData+PreLen,this->buffer+4,dataLength);
        return newData;
    }
    AutoPtr<FixBuffer>newData(new FixBuffer(dataLength+PreLen));
    BYTE*pNewData=(BYTE*)newData->GetBuffer();
    if(newData==NULL) return NULL;
    BYTE curDif=buffer[PreLen+4];
    int k=5+PreLen;
    int CurrentNum;
    unsigned char*bits=pNewData+PreLen;
    for(int p=0;p<curDif;p++)
    {
        CurrentNum=0;
        int num=(dataLength-p)/curDif;
        if(curDif*num<dataLength-p) num++;
        while(CurrentNum<num)
        {
            BYTE bV=buffer[k++];
            if(bV>=130)
            {
                int num2=bV-127;
                for(int j=0;j<num2;j++) bits[curDif*(CurrentNum++)+p]=buffer[k];
                k++;
            }
            else
            {
                int num2=bV;
                for(int j=0;j<num2;j++) bits[curDif*(CurrentNum++)+p]=buffer[k++];
            }
        }
    }
    return newData;
}

bool Rle2Decoder::Decode(int PreLen,BYTE*pNewData)
{
    LONG dataLength=GetDataLength(PreLen);
    BYTE curDif=buffer[PreLen+4];
    int k=5+PreLen;
    int CurrentNum;
    unsigned char*bits=pNewData+PreLen;
    for(int p=0;p<curDif;p++)
    {
        CurrentNum=0;
        int num=(dataLength-p)/curDif;
        if(curDif*num<dataLength-p) num++;
        while(CurrentNum<num)
        {
            BYTE bV=buffer[k++];
            if(bV>=130)//表示相同的
            {
                int num2=bV-127;
                for(int j=0;j<num2;j++) bits[curDif*(CurrentNum++)+p]=buffer[k];
                k++;
            }
            else
            {
                int num2=bV;
                for(int j=0;j<num2;j++) bits[curDif*(CurrentNum++)+p]=buffer[k++];
            }
        }
    }
    return true;
}

const unsigned char Base64Encrypt::Base64EncodeMap[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

unsigned char Base64Decrypt::Base64DecodeMap[256] =
{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F,
    0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,
    0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF,
    0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,
    0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
    0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,
    0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
    0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
    0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,
    0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
};

Base64Encrypt::Base64Encrypt()
    : _groupLength(0) {

}
Base64Encrypt::Base64Encrypt(const void *input, size_t length)
    : Base64Encrypt()
{
    Update(input, length);
}

void Base64Encrypt::Update(const void *input, size_t length)
{
    static const size_t LEN = 3;
    _buf.reserve(_buf.size() + (length - (LEN - _groupLength) + LEN - 1) / LEN * 4 + 1);
    const unsigned char *buff = reinterpret_cast<const unsigned char *>(input);
    unsigned int i;

    for (i = 0; i < length; ++i)
    {
        _group[_groupLength++] = buff[i];
        if (_groupLength == LEN)
        {
            Encode();
        }
    }
}
const unsigned char *Base64Encrypt::CipherText()
{
    Final();
    return _buf.data();
}
std::string Base64Encrypt::GetString()
{
    const char *pstr = (const char *)CipherText();
    size_t length = GetSize();
    return std::string(pstr, length);
}
void Base64Encrypt::Reset()
{
    _buf.clear();
    _groupLength = 0;
    for (unsigned int i = 0; i < sizeof(_group) / sizeof(_group[0]); ++i)
    {
        _group[i] = 0;
    }
}
size_t Base64Encrypt::GetSize()
{
    CipherText();
    return _buf.size();
}

void Base64Encrypt::Encode()
{
    unsigned char index;

    // 0 index byte
    index = _group[0] >> 2;
    _buf.push_back(Base64EncodeMap[index]);
    // 1 index byte
    index = ((_group[0] & 0x03) << 4) | (_group[1] >> 4);
    _buf.push_back(Base64EncodeMap[index]);
    // 2 index byte
    index = ((_group[1] & 0x0F) << 2) | (_group[2] >> 6);
    _buf.push_back(Base64EncodeMap[index]);
    // 3 index byte
    index = _group[2] & 0x3F;
    _buf.push_back(Base64EncodeMap[index]);

    _groupLength = 0;
}
void Base64Encrypt::Final()
{
    unsigned char index;

    if (_groupLength == 1)
    {
        _group[1] = 0;
        // 0 index byte
        index = _group[0] >> 2;
        _buf.push_back(Base64EncodeMap[index]);
        // 1 index byte
        index = ((_group[0] & 0x03) << 4) | (_group[1] >> 4);
        _buf.push_back(Base64EncodeMap[index]);
        // 2 index byte
        _buf.push_back('=');
        // 3 index byte
        _buf.push_back('=');
    }
    else if (_groupLength == 2)
    {
        _group[2] = 0;
        // 0 index byte
        index = _group[0] >> 2;
        _buf.push_back(Base64EncodeMap[index]);
        // 1 index byte
        index = ((_group[0] & 0x03) << 4) | (_group[1] >> 4);
        _buf.push_back(Base64EncodeMap[index]);
        // 2 index byte
        index = ((_group[1] & 0x0F) << 2) | (_group[2] >> 6);
        _buf.push_back(Base64EncodeMap[index]);
        // 3 index byte
        _buf.push_back('=');
    }
    _groupLength = 0;
}

Base64Decrypt::Base64Decrypt()
    : _groupLength(0) {

}
Base64Decrypt::Base64Decrypt(const void *input, size_t length)
    : Base64Decrypt()
{
    Update(input, length);
}

void Base64Decrypt::Update(const void *input, size_t length)
{
    static const size_t LEN = 4;
    _buf.reserve(_buf.size() + (length + (LEN - _groupLength) + LEN - 1) / LEN * 3 + 1);
    const unsigned char *buff = reinterpret_cast<const unsigned char *>(input);
    unsigned int i;

    for (i = 0; i < length; ++i)
    {
        if (Base64DecodeMap[buff[i]] == 0xFF)
        {
            throw std::invalid_argument("ciphertext is illegal");
        }

        _group[_groupLength++] = buff[i];
        if (_groupLength == LEN)
        {
            Decode();
        }
    }
}

const unsigned char *Base64Decrypt::PlainText()
{
    if (_groupLength)
    {
        throw std::invalid_argument("ciphertext's length must be a multiple of 4");
    }
    return _buf.data();
}
void Base64Decrypt::Reset()
{
    _buf.clear();
    _groupLength = 0;
    for (unsigned int i = 0; i < sizeof(_group) / sizeof(_group[0]); ++i)
    {
        _group[i] = 0;
    }
}
size_t Base64Decrypt::GetSize()
{
    PlainText();
    return _buf.size();
}

void Base64Decrypt::Decode()
{
    unsigned char buff[3];
    unsigned int top = 1;
    if (_group[0] == '=' || _group[1] == '=')
    {
        throw std::invalid_argument("ciphertext is illegal");
    }

    buff[0] = (Base64DecodeMap[_group[0]] << 2) | (Base64DecodeMap[_group[1]] >> 4);
    if (_group[2] != '=')
    {
        buff[1] = ((Base64DecodeMap[_group[1]] & 0x0F) << 4) | (Base64DecodeMap[_group[2]] >> 2);
        top = 2;
    }
    if (_group[3] != '=')
    {
        buff[2] = (Base64DecodeMap[_group[2]] << 6) | Base64DecodeMap[_group[3]];
        top = 3;
    }

    for (unsigned int i = 0; i < top; ++i)
    {
        _buf.push_back(buff[i]);
    }

    _groupLength = 0;
}


}
