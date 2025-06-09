#ifndef HTTP_H
#define HTTP_H

#include "BaseArray.h"
#include "autoptr.h"
#include "eventhelper.h"
#include "datetime.h"
#include "semaphora.h"
#include "xmldoc.h"
#include "threadpool.h"

namespace SGIS{

class Session;

class SGIS_EXPORT MimeType{
public:
    MimeType();
    virtual~MimeType();
    string GetString(string path);
    string GetStringByExa(string exa);
    void AddMimeType(string exa,string type);
protected:
    vector<string>exas;
    vector<string>types;
};

class SGIS_EXPORT PathMap{
public:
    PathMap();
    virtual~PathMap();
    bool ReadFromXml(string pathName);
    bool WriteToXml(string pathName);
    bool AddKey(string key,string path);
    string FindPath(string key);
    string MapPath(string url);
protected:
    vector<string>keys;
    vector<string>paths;
    AutoPtr<Mutex>mutex;
};

class SGIS_EXPORT HeaderKeys{
public:
    HeaderKeys();
    virtual~HeaderKeys();
    bool Add(string key,string value);
    void SetContentTypeHtml();
    void SetContentTypePng();
    void SetContentTypeJpg();
    void SetContentTypeXml();
    void SetContentTypeJson();
    void SetContentTypeStream();
    void SetAcceptCodeGzip();
    void SetAccessControlAllowOrigin();
    void SetConnection(bool keepalive);
    bool FindKey(string key);
    int GetSize();
    string GetKey(int nIndex);
    string GetValue(string key);
    string GetValue(int nIndex);
protected:
    vector<string>keys;
    vector<string>values;
};

class SGIS_EXPORT HostHelper{
public:
    static vector<string>GetIps();
    static int GetNumberOfProcessors();
};

class SGIS_EXPORT HttpClient
{
public:
    HttpClient();
    virtual~HttpClient();
    AutoPtr<FixBuffer>doGet(string url,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    AutoPtr<FixBuffer>doPost(string url,AutoPtr<FixBuffer>data,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    int doGet(string url,BYTE*buffer,int len,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    int doPost(string url,AutoPtr<FixBuffer>data,BYTE*buffer,int len,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    AutoPtr<FixBuffer>doGet(string url,AutoPtr<Semaphora>quitsignal,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    AutoPtr<FixBuffer>doPost(string url,AutoPtr<FixBuffer>data,AutoPtr<Semaphora>quitsignal,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    int doGet(string url,AutoPtr<Semaphora>quitsignal,BYTE*buffer,int len,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    int doPost(string url,AutoPtr<FixBuffer>data,AutoPtr<Semaphora>quitsignal,BYTE*buffer,int len,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    bool downloadFile(string url,string savingPath);
protected:
    bool analyseUrl(string&url,string& host,string& port,string& page,bool&https);
    AutoPtr<FixBuffer>innerGet(string url,AutoPtr<Semaphora>quitsignal,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    int innerGet(string url,AutoPtr<Semaphora>quitsignal,BYTE*buffer,int len,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    AutoPtr<FixBuffer>innerPost(string url,AutoPtr<Semaphora>quitsignal,AutoPtr<FixBuffer>data,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
    int innerPost(string url,AutoPtr<Semaphora>quitsignal,AutoPtr<FixBuffer>data,BYTE*buffer,int len,HeaderKeys*hks=nullptr,int timeoutOfmilliseconds=0);
 protected:
    static void sendfailedFunc(Event& e);
    static void sendSucFunc(Event& e);
 public:
    Event OnSendFailed;
    Event OnReceived;
protected:
    AutoPtr<Semaphora>m_exit;
    AutoPtr<Semaphora>m_event;
    AutoPtr<EventHelper>helper;
};

class SGIS_EXPORT RequestData{
public:
    RequestData(string url,AutoPtr<FixBuffer>data,const string&remoteServerIp);
    virtual~RequestData();
    NamedMap<string>&Headers();
    int GetParametersCount();
    string GetParameter(int nIndex);
    string GetValue(int nIndex);
    string FindValue(string parameter);
    bool FindParameter(string parameter);
    AutoPtr<FixBuffer>Data();
    string GetUrl();
    string GetRemoteServerIp();
protected:
    vector<string>parameters;
    vector<string>values;
    string url;
    AutoPtr<FixBuffer>data;
    string remoteServerIp;
    NamedMap<string>headers;
};

enum ResponseDataType{
    rdtFile=0,
    rdtNoCompressed=1,
    rdtCompressed=2
};

class SGIS_EXPORT ResponseData{
public:
    ResponseData();
    virtual~ResponseData();
    string&MimeType();
    string&GetStringData();
    AutoPtr<FixBuffer>GetBufferData();
    bool IsBufferData();
    bool IsFile();
    ResponseDataType GetResponseDataType();
    void SetStringData(const string&data,bool bCompressed=false);
    void SetBufferData(AutoPtr<FixBuffer>buffer,bool bCompressed=false);
    void SendXml(XmlDoc*xmlDoc,bool bCompressed=false);
    void SendHtml(const string&html,bool bCompressed=false);
    void SendJson(const string&json,bool bCompressed=false);
    void SendText(const string&text,bool bCompressed=false);
    void SendStream(AutoPtr<FixBuffer>buffer,bool bCompressed=false,bool rle2Encode=false);
    void SendStream(char*buffer,int len,bool bCompressed=false,bool rle2Encode=false);
    bool SendImage(AutoPtr<FixBuffer>buffer,string imageExa);
    bool SendImage(char*buffer,int len,string imageExa);
    bool SendXmlFile(string pathName);
    bool SendHtmlFile(string pathName);
    bool SendJsonFile(string pathName);
    bool SendImageFile(string pathName);
    bool SendStreamFile(string pathName);
    bool SendFile(string pathName);
    AutoPtr<ResponseData>Clone();
protected:
    string string_data;
    AutoPtr<FixBuffer>buffer_data;
    string mimetype;
    ResponseDataType rsdType;
};

class SGIS_EXPORT HttpResponseEventArgs :public EventArgs{
public:
    HttpResponseEventArgs(AutoPtr<RequestData>rqdata,AutoPtr<Semaphora>signal=nullptr);
    virtual~HttpResponseEventArgs();
    void FinishResponse();
    AutoPtr<RequestData>GetRequestData();
    AutoPtr<ResponseData>GetResponseData();
    AutoPtr<Semaphora>GetSignal();
    bool HasHandled();
protected:
    bool handled;
    AutoPtr<RequestData>rqdata;
    AutoPtr<ResponseData>rsdata;
    AutoPtr<Semaphora>signal;
};

struct HTTPResponseEvent{
    Sender sender;
    AutoPtr<HttpResponseEventArgs>args;
    HTTPResponseEvent(Sender sender,AutoPtr<HttpResponseEventArgs>args){
       this->sender=sender;
       this->args=args;
    };
};

typedef void(*HTTPRESPONSEEVENTFUN) (HTTPResponseEvent&);

struct HttpsCertificate{
    string certificate_chain_file;
    string private_key_file;
    string password;
    string verify_file;
    HttpsCertificate(){
        certificate_chain_file="";
        private_key_file="";
        password="";
        verify_file="";
    };
    HttpsCertificate(const string&certificate_chain_file,const string&private_key_file,const string&verify_file="",const string&password=""){
        this->certificate_chain_file=certificate_chain_file;
        this->private_key_file=private_key_file;
        this->password=password;
        this->verify_file=verify_file;
    };
    HttpsCertificate(const HttpsCertificate&other){
        this->certificate_chain_file=other.certificate_chain_file;
        this->private_key_file=other.private_key_file;
        this->password=other.password;
        this->verify_file=other.verify_file;
    };
    HttpsCertificate&operator=(const HttpsCertificate&other){
        this->certificate_chain_file=other.certificate_chain_file;
        this->private_key_file=other.private_key_file;
        this->password=other.password;
        this->verify_file=other.verify_file;
        return *this;
    };
};

class SGIS_EXPORT HttpServer{
public:
    HttpServer();
    virtual~HttpServer();
    AutoPtr<MimeType>GetMimeType();
    AutoPtr<PathMap>GetPathMap();
    string GetWorkingDirectory();
    int GetNumberWorkers();
    bool RunServer(const string&serverIp,USHORT port,const string&workDir,int num_threads=0,bool workerPerThread=false);
    bool RunServer(const string&serverIp,USHORT port,const string&workDir,const HttpsCertificate&cert,int num_threads=0);
    void Stop();
    string GetServerIp();
    USHORT GetServerPort();
    string&ServerName();
    int GetTimeoutSeconds();
    void SetTimeoutSeconds(int seconds);
    void AddZipFileType(string fileExa);
    bool IsFileTypeInZipList(string fileExa);
    bool IsMimeTypeInZipList(string mimeTye);
public:
    Event OnStartup;
    HTTPRESPONSEEVENTFUN ResponseFunc;
protected:
    AutoPtr<MimeType>mimeType;
    AutoPtr<PathMap>pathMap;
    string workDir;
    int num_workers;
    string serverIp;
    USHORT port;
    string serverName;
    string password;
    int timeoutSeconds;

    void*server;
    bool workerPerThread;
    vector<string>gzips;
    vector<string>gzipMimes;
};

enum WebSocketReceiveState{
    wsrsPostPackage=0,
    wsrsSendPackage=1,
    wsrsTimedoutSendPackage=2
};

class SGIS_EXPORT WebSocketClient
{
public:
    WebSocketClient();
    virtual~WebSocketClient();
    bool GetConnected();
    void Close();
    bool Connect(string url,int timeoutOfSeconds=30);
    AutoPtr<FixBuffer>GetSendingPackage();
    bool Post(const string&data);
    bool Post(AutoPtr<FixBuffer>data);
    AutoPtr<FixBuffer>Send(const string&data,int timeoutOfSeconds=30,Semaphora*quitsignal=nullptr);
    AutoPtr<FixBuffer>Send(AutoPtr<FixBuffer>data,int timeoutOfSeconds=30,Semaphora*quitsignal=nullptr);
protected:
    bool analyseUrl(string&url,string& host,string& port,string& protocol);
    static void connectedFunc(Event& e);
    static void connectFailedFunc(Event& e);
    static void stoppedFunc(Event& e);
    static void sendReceivedFunc(Event& e);
public:
    Event OnConnected;
    Event OnConnetFailed;
    Event OnAccept;
    Event OnSendReceived;
    Event OnQueryReceiveState;
    Event OnStopped;
protected:
    AutoPtr<Semaphora>m_exit;
    AutoPtr<Semaphora>m_event;
    AutoPtr<Semaphora>m_sevent;
    AutoPtr<EventHelper>helper;
    bool bConnected;
    int timeoutOfSeconds;
    string url;
    thread sessionThread;
    AutoPtr<FixBuffer>sendingPack;
    AutoPtr<FixBuffer>acceptedBuffer;
    void*session;
};

class SGIS_EXPORT WebSocketResponseEventArgs :public EventArgs{
public:
    WebSocketResponseEventArgs(string protol,void*socket,AutoPtr<FixBuffer>rqdata,AutoPtr<Semaphora>signal=nullptr);
    virtual~WebSocketResponseEventArgs();
    string GetProtocol();
    void*GetSocket();
    AutoPtr<FixBuffer>GetRequestData();
    AutoPtr<Semaphora>GetSignal();
protected:
    void*socket;
    AutoPtr<FixBuffer>rqdata;
    string protocol;
    AutoPtr<Semaphora>signal;
};

struct WebSocketResponseEvent{
    Sender sender;
    AutoPtr<WebSocketResponseEventArgs>args;
    WebSocketResponseEvent(Sender sender,AutoPtr<WebSocketResponseEventArgs>args){
       this->sender=sender;
       this->args=args;
    };
};

typedef void(*WEBSOCKETRESPONSEEVENTFUN) (WebSocketResponseEvent&);

struct WebSocketTimerEvent{
    Sender sender;
    WebSocketTimerEvent(Sender sender){
       this->sender=sender;
    };
};

typedef void(*WEBSOCKETTIMEREVENTFUN) (WebSocketTimerEvent&);

class SGIS_EXPORT WebSocketServer{
public:
    WebSocketServer();
    virtual~WebSocketServer();
    bool RunServer(const string&serverIp,USHORT port,const string&workDir,int num_threads=0,bool workerPerThread=false);
    string GetServerIp();
    USHORT GetServerPort();
    int GetNumberWorkers();
    string GetWorkingDirectory();
    AutoPtr<PathMap>GetPathMap();
    string&ServerName();
    int GetTimeoutSeconds();
    void SetTimeoutSeconds(int seconds);
    int GetTimerSeconds();
    void SetTimerSeconds(int seconds);
    bool HasProtocol(const string&protocol);
    void LabelActive(const string&protocol);
    bool Post(void*socket,const string&data);
    bool Post(void*socket,AutoPtr<FixBuffer>buffer);
    void Broadcast(const string&data);
    void Broadcast(const string&protocol,const string&data);
    void Broadcast(AutoPtr<FixBuffer>buffer);
    void Broadcast(const string&protocol,AutoPtr<FixBuffer>buffer);
public:
    Event OnStartup;
    WEBSOCKETRESPONSEEVENTFUN ResponseFunc;
    WEBSOCKETTIMEREVENTFUN TimerFunc;
protected:
    AutoPtr<PathMap>pathMap;
    string workDir;
    int num_workers;
    string serverIp;
    USHORT port;
    string serverName;
    string password;
    int timeoutSeconds;
    bool workerPerThread;
    int timerSeconds;
    void*server;
};


};
#endif // HTTP_H
