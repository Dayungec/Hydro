#ifndef BUFFERIO_H
#define BUFFERIO_H
#include "base_globle.h"
#include "BaseArray.h"
#include "classfactory.h"
#include "autoptr.h"
namespace SGIS{

class BufferWriter;
class BufferReader;

class SGIS_EXPORT BufferWritable :
   public Object
{
public:
    BufferWritable(){};
    virtual~BufferWritable(){};
    virtual std::string GetTypeName()=0;
    virtual bool Read(BufferReader*br)=0;
    virtual void Write(BufferWriter*bw)=0;
};

class SGIS_EXPORT BufferWriter{
public:
    BufferWriter();
    virtual~BufferWriter();
    template<typename T>
    void Write(T value){
        char*buffer=autobuffer.GetBuffer(currentPin+sizeof(T));
        memcpy(buffer+currentPin,&value,sizeof(T));
        currentPin+=sizeof(T);
    };
    /*
    template <typename T, typename ...Args>
    void Write(T head, Args... rest)
    {
       Write<T>(head);
       Write<Args...>(rest...);
    };
    */
    template<typename T>
    void WriteVector(const vector<T>&values){
        int len=sizeof(T)*values.size();
        Write<int>(values.size());
        char*buffer=autobuffer.GetBuffer(currentPin+len);
        memcpy(buffer+currentPin,values.data(),len);
        currentPin+=len;
    };
    void WriteBuffer(FixBuffer*fixBuffer);
    void WriteString(string str);
    void WriteObject(BufferWritable*obj,bool allowNull);
    char*GetBuffer();
    ULONGLONG GetBufferLength();
    AutoPtr<FixBuffer>GetFixBuffer();
protected:
    ULONGLONG currentPin;
    AutoBuffer autobuffer;
};

class SGIS_EXPORT BufferReader{
public:
    BufferReader(char*buffer,ULONGLONG totalSize=0);
    virtual~BufferReader();
    ULONGLONG GetCurrentPin();
    string&PathName();
    template<typename T>
    T Read(){
        if(totalSize>0){
            if(currentPin+sizeof(T)>totalSize){
                return 0;
            }
        }
        T value;
        memcpy(&value,buffer+currentPin,sizeof(T));
        currentPin+=sizeof(T);
        return value;
    };
    template<typename T>
    void Read(T&value){
        if(totalSize>0){
            if(currentPin+sizeof(T)>totalSize){
                return;
            }
        }
        memcpy(&value,buffer+currentPin,sizeof(T));
        currentPin+=sizeof(T);
    };
    /*
    template <typename T, typename ...Args>
    void Read(T&head, Args&... rest)
    {
       Read<T>(head);
       Read<Args...>(rest...);
    };
    */
    AutoPtr<FixBuffer>ReadBuffer();
    template<typename T>
    void ReadVector(vector<T>&values){
        int size=Read<int>();
         int len=sizeof(T)*size;
        if(totalSize>0){
            if(currentPin+len>totalSize) return;
        }
        values.resize(size);
        memcpy(values.data(),buffer+currentPin,len);
        currentPin+=len;
    };
    void Move(int pos);
    void MoveTo(int pos);
    string ReadString();
    AutoPtr<BufferWritable>ReadObject(bool allowNull);
protected:
    ULONGLONG currentPin;
    char*buffer;
    string lpszPathName;
    ULONGLONG totalSize;
};

class SGIS_EXPORT BufferFile{
public:
    BufferFile();
    virtual~BufferFile();
    static bool SaveFile(string pathName,const vector<AutoPtr<BufferWritable>>bws);
    static vector<AutoPtr<BufferWritable>>ReadFile(string pathName);
};

}

#endif // BUFFERIO_H
