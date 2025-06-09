#include "http.h"
#include "StringHelper.h"
#include "FilePath.h"
#include "xmldoc.h"
#include "http/http_worker.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include "file.h"
#include "ziputil.h"

#include "datacoder.h"

/*
#ifdef Q_OS_LINUX
  #include "unistd.h"
#else
  #include "Windows.h"
#endif
*/

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>



namespace SGIS{

MimeType::MimeType(){

}
MimeType::~MimeType(){

}

string MimeType::GetStringByExa(string exa){
    StringHelper::MakeLower(exa);
    if(exa=="htm") return "text/html";
    if(exa=="html") return "text/html";
    if(exa=="php") return "text/html";
    if(exa=="css") return "text/css";
    if((exa=="txt")||(exa=="csv")) return "text/plain";
    if(exa=="js") return "application/javascript";
    if(exa=="json") return "application/json";
    if(exa=="xml") return "application/xml";
    if(exa=="swf") return "application/x-shockwave-flash";
    if(exa=="flv") return "video/x-flv";
    if(exa=="png") return "image/png";
    if(exa=="jpe") return "image/jpeg";
    if(exa=="jpg") return "image/jpeg";
    if(exa=="jpeg") return "image/jpeg";
    if(exa=="gif") return "image/gif";
    if(exa=="bmp") return "image/bmp";
    if(exa=="ico") return "image/vnd.microsoft.icon";
    if(exa=="tiff") return "image/tiff";
    if(exa=="tif") return "image/tiff";
    if(exa=="svg") return "image/svg+xml";
    if(exa=="svgz") return "image/svg+xml";
    for(int k=exas.size()-1;k>=0;k--){
        if(exas[k]==exa) return types[k];
    }
    return "application/octet-stream;charset=UTF-8";
}

string MimeType::GetString(string path){
    string exa=FilePath::GetFileExa(path);
    return GetStringByExa(exa);
}

void MimeType::AddMimeType(string exa,string type){
    StringHelper::TrimToLower(exa);
    exas.push_back(exa);
    types.push_back(type);
}

PathMap::PathMap(){
    mutex=new Mutex();
}
PathMap::~PathMap(){

}

bool PathMap::ReadFromXml(string pathName){
    keys.clear();
    paths.clear();
    AutoPtr<XmlDoc>xmlDoc(new XmlDoc());
    if(!xmlDoc->Load(pathName)) return false;
    AutoPtr<XmlNode>rootNode=xmlDoc->GetRootNode();
    if(rootNode->GetName()!="Pathes") return false;
    AutoPtr<XmlNode>childNode=rootNode->GetFirstChildNode();
    while(childNode!=nullptr){
        if(childNode->GetName()=="Path"){
            string key=childNode->GetAttribute("Key");
            string path=childNode->GetData();
            keys.push_back(key);
            paths.push_back(path);
        }
        childNode=childNode->GetNextSiblingElement();
    }
    return true;
}

bool PathMap::WriteToXml(string pathName){
    AutoPtr<XmlDoc>xmlDoc(new XmlDoc());
    AutoPtr<XmlNode>node=xmlDoc->AppendChild("Pathes");
    for(int k=0;k<paths.size();k++){
        AutoPtr<XmlNode>childNode=node->AppendChild("Path",paths[k]);
        childNode->SetAttribute("Key",keys[k]);
    }
    return xmlDoc->Save(pathName);
}

bool PathMap::AddKey(string key,string path){
    mutex->Lock();
    StringHelper::TrimToLower(key);
    for(int k=keys.size()-1;k>=0;k--)
    {
        string skey=keys[k];
        StringHelper::TrimToLower(skey);
        if(skey==key){
            mutex->Unlock();
            return false;
        }
    }
    keys.push_back(key);
    paths.push_back(path);
    mutex->Unlock();
    return true;
}

string PathMap::FindPath(string key){
    mutex->Lock();
    StringHelper::TrimToLower(key);
    for(int k=keys.size()-1;k>=0;k--)
    {
        string skey=keys[k];
        StringHelper::TrimToLower(skey);
        if(skey==key){
            mutex->Unlock();
            return paths[k];
        }
    }
    mutex->Unlock();
    return "";
}

string PathMap::MapPath(string sUrl){
    int nPos=sUrl.find("/");
    if(nPos==0){
        sUrl=sUrl.substr(1,sUrl.size()-1);
    }
    nPos=sUrl.find("/");
    if(nPos<0) {
        return "";
    }
    string key=sUrl.substr(0,nPos);
    string otherV=sUrl.substr(nPos+1,sUrl.size()-nPos-1);
    string sPath=FindPath(key);
    if(sPath==""){
        return "";
    }
    sPath+="\\"+otherV;
    sPath=FilePath::ConvertToStardardPath(sPath);
    return sPath;
}

HeaderKeys::HeaderKeys(){

}
HeaderKeys::~HeaderKeys(){

}

bool HeaderKeys::Add(string key,string value){
    if(FindKey(key)) return false;
    keys.push_back(key);
    values.push_back(value);
    return true;
}

void HeaderKeys::SetContentTypeHtml(){
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey=="content-type"){
            values[k]="text/html;charset=UTF-8";
            return;
        }
    }
    keys.push_back("Content-Type");
    values.push_back("text/html;charset=UTF-8");
}

void HeaderKeys::SetContentTypePng(){
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey=="content-type"){
            values[k]="image/png";
            return;
        }
    }
    keys.push_back("Content-Type");
    values.push_back("image/png");
}

void HeaderKeys::SetContentTypeJpg(){
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey=="content-type"){
            values[k]="image/jpeg";
            return;
        }
    }
    keys.push_back("Content-Type");
    values.push_back("image/jpeg");
}

void HeaderKeys::SetContentTypeXml(){
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey=="content-type"){
            values[k]="application/xml;charset=UTF-8";
            return;
        }
    }
    keys.push_back("Content-Type");
    values.push_back("application/xml;charset=UTF-8");
}

void HeaderKeys::SetContentTypeJson(){
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey=="content-type"){
            values[k]="application/json;charset=UTF-8";
            return;
        }
    }
    keys.push_back("Content-Type");
    values.push_back("application/json;charset=UTF-8");
}

void HeaderKeys::SetContentTypeStream(){
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey=="content-type"){
            values[k]="application/octet-stream;charset=UTF-8";
            return;
        }
    }
    keys.push_back("Content-Type");
    values.push_back("application/octet-stream;charset=UTF-8");
}
void HeaderKeys::SetAcceptCodeGzip(){
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey=="accept-encoding"){
            values[k]="gzip,deflate";
            return;
        }
    }
    keys.push_back("Accept-Encoding");
    values.push_back("gzip,deflate");
}

void HeaderKeys::SetAccessControlAllowOrigin(){
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey=="access-control-allow-origin"){
            values[k]="*";
            return;
        }
    }
    keys.push_back("Access-Control-Allow-Origin");
    values.push_back("*");
}

void HeaderKeys::SetConnection(bool keepalive){
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey=="connection"){
            values[k]=(keepalive?"keep-alive":"close");
            return;
        }
    }
    keys.push_back("Connection");
    values.push_back(keepalive?"keep-alive":"close");
}

bool HeaderKeys::FindKey(string key){
    StringHelper::TrimToLower(key);
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey==key) return true;
    }
    return false;
}
int HeaderKeys::GetSize(){
    return keys.size();
}
string HeaderKeys::GetKey(int nIndex){
    return keys[nIndex];
}

string HeaderKeys::GetValue(string key){
    StringHelper::TrimToLower(key);
    for(int k=keys.size()-1;k>=0;k--){
        string subkey=keys[k];
        StringHelper::TrimToLower(subkey);
        if(subkey==key) return values[k];
    }
    return "";
}
string HeaderKeys::GetValue(int nIndex){
    return values[nIndex];
}

vector<string>HostHelper::GetIps(){
    boost::asio::io_service io_service;
    tcp::resolver resolver(io_service);
    tcp::resolver::query query(boost::asio::ip::host_name(), "");
    try{
        tcp::resolver::iterator iter = resolver.resolve(query);
        tcp::resolver::iterator end;
        vector<string>ips;
        while (iter != end)
        {
            tcp::endpoint ep = *iter++;
            string sIp=ep.address().to_string();
            int nPos=sIp.find(':');
            if(nPos>=0) continue;
            ips.push_back(sIp);
        }
        return ips;
    }
    catch(boost::system::system_error e){
        return vector<string>{"127.0.0.1"};
    }
}

int HostHelper::GetNumberOfProcessors(){
#ifdef Q_OS_LINUX
   return sysconf( _SC_NPROCESSORS_CONF);
#else
    SYSTEM_INFO sysInfo;
    GetSystemInfo( &sysInfo );
    return sysInfo.dwNumberOfProcessors;
#endif
}

class Session : public std::enable_shared_from_this<Session>
{
public:
    explicit Session(net::io_context& ioc,HttpClient*client,int timed_out=30)
        : resolver_(net::make_strand(ioc))
        , stream_(net::make_strand(ioc))
    {
        receivedBuffer=nullptr;
        this->client=client;
        this->timed_out=timed_out;
        res_.eager(true);
        size_t size=1024;
        size*=1024*1024;
        //res_.get().body().data=wb;
        //res_.get().body().size=512;
        res_.body_limit(size);
    }
    void get(char const* host,char const* port,char const* target,int version,HeaderKeys*hks)
    {
        receivedBuffer=nullptr;
        bGzip=false;
        req_get_.version(version);
        req_get_.method(http::verb::get);
        req_get_.target(target);
        req_get_.set(http::field::host, host);
        req_get_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        if(hks!=nullptr){
            string accept=hks->GetValue("Accept");
            if(accept=="") accept="*/*";
            req_get_.set(http::field::accept,accept);
            string contType=hks->GetValue("Content-Type");
            if(contType=="") contType="application/x-www-form-urlencoded";
            req_get_.set(http::field::content_type,contType);
            string conType=hks->GetValue("Connection");
            if(conType=="") conType="close";
            req_get_.set(http::field::connection,conType);
            int hkCount=hks->GetSize();
            for(int k=0;k<hkCount;k++){
                string key=hks->GetKey(k);
                string key2=key;
                StringHelper::TrimToLower(key2);
                if((key2=="accept")||(key2=="content-type")||(key2=="connection")) continue;
                req_get_.set(key,hks->GetValue(k));
            }
        }
        else{
            req_get_.set(http::field::accept,"*/*");
            req_get_.set(http::field::content_type,"application/x-www-form-urlencoded");
            req_get_.set(http::field::connection,"close");
        }
        request_type=0;
        resolver_.async_resolve(host,port,beast::bind_front_handler(&Session::on_resolve,shared_from_this()));
    };

    void post(char const* host,char const* port,char const* target,char const* data,int len,int version,HeaderKeys*hks)
    {
        receivedBuffer=nullptr;
        bGzip=false;
        req_post_.version(version);
        req_post_.method(http::verb::post);
        req_post_.target(target);
        req_post_.set(http::field::host, host);
        req_post_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        if(hks!=nullptr){
            string accept=hks->GetValue("Accept");
            if(accept=="") accept="*/*";
            req_post_.set(http::field::accept,accept);
            string contType=hks->GetValue("Content-Type");
            if(contType=="") contType="application/x-www-form-urlencoded";
            req_post_.set(http::field::content_type,contType);
            string conType=hks->GetValue("Connection");
            if(conType=="") conType="close";
            req_post_.set(http::field::connection,conType);
            int hkCount=hks->GetSize();
            for(int k=0;k<hkCount;k++){
                string key=hks->GetKey(k);
                string key2=key;
                StringHelper::TrimToLower(key2);
                if((key2=="accept")||(key2=="content-type")||(key2=="connection")) continue;
                req_post_.set(key,hks->GetValue(k));
            }
        }
        else{
            req_post_.set(http::field::accept,"*/*");
            req_post_.set(http::field::content_type,"application/x-www-form-urlencoded");
            req_post_.set(http::field::connection,"close");
        }
        if(data!=nullptr){
            req_post_.content_length(len);
            http::buffer_body::value_type body;
            body.data=(void*)data;
            body.size=len;
            body.more=false;
            req_post_.body()=std::move(body);
            req_post_.prepare_payload();
        }else{
            req_post_.content_length(0);
            http::buffer_body::value_type body;
            body.data=nullptr;
            body.size=0;
            body.more=false;
            req_post_.body()=std::move(body);
            req_post_.prepare_payload();
        }
        request_type=1;
        resolver_.async_resolve(host,port,beast::bind_front_handler(&Session::on_resolve,shared_from_this()));
    };

    void on_resolve(beast::error_code ec,tcp::resolver::results_type results)
    {
        if(ec){
            client->OnSendFailed.RaiseEvent(client);
            return;
        }
        if(timed_out>0) stream_.expires_after(std::chrono::seconds(timed_out));
        stream_.async_connect(results,beast::bind_front_handler(&Session::on_connect,shared_from_this()));
    };

    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type)
    {
        if(ec){
            client->OnSendFailed.RaiseEvent(client);
            return;
        }
        if(timed_out>0) stream_.expires_after(std::chrono::seconds(timed_out));
        if(request_type==0)
           http::async_write(stream_, req_get_,beast::bind_front_handler(&Session::on_write,shared_from_this()));
        else if(request_type==1)
           http::async_write(stream_, req_post_,beast::bind_front_handler(&Session::on_write,shared_from_this()));
    };

    void on_write(beast::error_code ec,std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        if(ec){
            client->OnSendFailed.RaiseEvent(client);
            return;
        }
        http::async_read(stream_, buffer_, res_,beast::bind_front_handler(&Session::on_read,shared_from_this()));
    };

    void on_read(beast::error_code ec,std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        if(ec){
            client->OnSendFailed.RaiseEvent(client);
            return;
        }
        receivedBuffer=new FixBuffer(bytes_transferred);
        boost::beast::basic_multi_buffer<std::allocator<char>>::mutable_buffers_type data=res_.get().body().data();
        char*m_buffer=receivedBuffer->GetBuffer();
        int size=0;
        for(auto const buffer : buffers_range_ref(data)){
            memcpy(m_buffer+size,static_cast<char const*>(buffer.data()),buffer.size());
            size+=buffer.size();
        }
        client->OnReceived.RaiseEvent(client);
        stream_.socket().shutdown(tcp::socket::shutdown_both, ec);
    };
    AutoPtr<FixBuffer>GetBody(){
        return receivedBuffer;
    };
    bool GetGZip(){
        return bGzip;
    };
    void Reset(){
        receivedBuffer=nullptr;
    };
public:
    tcp::resolver resolver_;
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;

    http::request<http::empty_body> req_get_;
    http::request<http::buffer_body> req_post_;
    int request_type;

    AutoPtr<FixBuffer>receivedBuffer;
    bool bGzip;
    http::response_parser<http::dynamic_body> res_;
    HttpClient*client;
    int timed_out;
};

class HttpsSession : public std::enable_shared_from_this<HttpsSession>
{
public:
    explicit HttpsSession(ssl::context&ctx,net::io_context& ioc,HttpClient*client)
        : resolver_(net::make_strand(ioc))
        , stream_(net::make_strand(ioc), ctx)
    {
        this->client=client;
        receivedBuffer=nullptr;
        res_.eager(true);
        size_t size=1024;
        size*=1024*1024;
        res_.body_limit(size);
    }
    void get(char const* host,char const* port,char const* target,int version,HeaderKeys*hks)
    {
        receivedBuffer=nullptr;
        bGzip=false;
        req_get_.version(version);
        req_get_.method(http::verb::get);
        req_get_.target(target);
        req_get_.set(http::field::host, host);
        req_get_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        if(hks!=nullptr){
            string accept=hks->GetValue("Accept");
            if(accept=="") accept="*/*";
            req_get_.set(http::field::accept,accept);
            string contType=hks->GetValue("Content-Type");
            if(contType=="") contType="application/x-www-form-urlencoded";
            req_get_.set(http::field::content_type,contType);
            string conType=hks->GetValue("Connection");
            if(conType=="") conType="close";
            req_get_.set(http::field::connection,conType);
            int hkCount=hks->GetSize();
            for(int k=0;k<hkCount;k++){
                string key=hks->GetKey(k);
                string key2=key;
                StringHelper::TrimToLower(key2);
                if((key2=="accept")||(key2=="content-type")||(key2=="connection")) continue;
                req_get_.set(key,hks->GetValue(k));
            }
        }
        else{
             req_get_.set(http::field::accept,"*/*");
             req_get_.set(http::field::content_type,"application/x-www-form-urlencoded");
             req_get_.set(http::field::connection,"close");
        }
        request_type=0;
        resolver_.async_resolve(host,port,beast::bind_front_handler(&HttpsSession::on_resolve,shared_from_this()));
    };

    void post(char const* host,char const* port,char const* target,char const* data,int len,int version,HeaderKeys*hks)
    {
        receivedBuffer=nullptr;
        bGzip=false;
        req_post_.version(version);
        req_post_.method(http::verb::post);
        req_post_.target(target);
        req_post_.set(http::field::host, host);
        req_post_.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
        //http::request_parser<http::string_body> req;
        if(hks!=nullptr){
            string accept=hks->GetValue("Accept");
            if(accept=="") accept="*/*";
            req_post_.set(http::field::accept,accept);
            string contType=hks->GetValue("Content-Type");
            if(contType=="") contType="application/x-www-form-urlencoded";
            req_post_.set(http::field::content_type,contType);
            string conType=hks->GetValue("Connection");
            if(conType=="") conType="close";
            req_post_.set(http::field::connection,conType);
            int hkCount=hks->GetSize();
            for(int k=0;k<hkCount;k++){
                string key=hks->GetKey(k);
                string key2=key;
                StringHelper::TrimToLower(key2);
                if((key2=="accept")||(key2=="content-type")||(key2=="connection")) continue;
                req_post_.set(key,hks->GetValue(k));
            }
        }
        else{
             req_post_.set(http::field::accept,"*/*");
             req_post_.set(http::field::content_type,"application/x-www-form-urlencoded");
             req_post_.set(http::field::connection,"close");
        }
        if(data!=nullptr){
             req_post_.content_length(len);
             http::buffer_body::value_type body;
             body.data=(void*)data;
             body.size=len;
             body.more=false;
             req_post_.body()=std::move(body);
             req_post_.prepare_payload();
        }
        request_type=1;
        resolver_.async_resolve(host,port,beast::bind_front_handler(&HttpsSession::on_resolve,shared_from_this()));
    };

    void on_resolve(beast::error_code ec,tcp::resolver::results_type results)
    {
        if(ec){
            client->OnSendFailed.RaiseEvent(client);
            return;
        }
        stream_.lowest_layer().async_connect(results->endpoint(),beast::bind_front_handler(&HttpsSession::on_connect,shared_from_this()));
    };

    void on_connect(beast::error_code ec)
    {
        if(ec){
            client->OnSendFailed.RaiseEvent(client);
            return;
        }
        stream_.async_handshake(ssl::stream_base::client,beast::bind_front_handler(&HttpsSession::on_handshake,shared_from_this()));
    };

    void on_handshake(beast::error_code ec)
    {
        if(ec){
            client->OnSendFailed.RaiseEvent(client);
            return;
        }
        if(request_type==0)
            http::async_write(stream_, req_get_,beast::bind_front_handler(&HttpsSession::on_write,shared_from_this()));
        else
            http::async_write(stream_, req_post_,beast::bind_front_handler(&HttpsSession::on_write,shared_from_this()));
    };

    void on_write(beast::error_code ec,std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        if(ec){
            client->OnSendFailed.RaiseEvent(client);
            return;
        }
        //http::async_read_some(stream_, buffer_, res_,beast::bind_front_handler(&HttpsSession::on_read,shared_from_this()));
        http::async_read(stream_, buffer_, res_,beast::bind_front_handler(&HttpsSession::on_read,shared_from_this()));
    };

    void on_read(beast::error_code ec,std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);
        if(ec){
            client->OnSendFailed.RaiseEvent(client);
            return;
        }
        receivedBuffer=new FixBuffer(bytes_transferred);
        boost::beast::basic_multi_buffer<std::allocator<char>>::mutable_buffers_type data=res_.get().body().data();
        char*m_buffer=receivedBuffer->GetBuffer();
        int size=0;
        for(auto const buffer : buffers_range_ref(data)){
            memcpy(m_buffer+size,static_cast<char const*>(buffer.data()),buffer.size());
            size+=buffer.size();
        }
        client->OnReceived.RaiseEvent(client);
        stream_.lowest_layer().shutdown(tcp::socket::shutdown_both, ec);
    };

    AutoPtr<FixBuffer>GetBody(){
        return receivedBuffer;
    };
    bool GetGZip(){
        return bGzip;
    };
    void Reset(){
        receivedBuffer=nullptr;
    };
public:
    tcp::resolver resolver_;
    ssl::stream<tcp::socket>stream_;
    beast::flat_buffer buffer_;
    http::request<http::empty_body> req_get_;
    http::request<http::buffer_body> req_post_;
    int request_type;
    AutoPtr<FixBuffer>receivedBuffer;
    bool bGzip;
    http::response_parser<http::dynamic_body> res_;
    HttpClient*client;
};

HttpClient::HttpClient()
{

}

HttpClient::~HttpClient()
{
    if(m_exit!=nullptr) m_exit->SetSignal();
}

void HttpClient::sendfailedFunc(Event& e){
    HttpClient*client=(HttpClient*)e.GetSender();
    client->m_event->SetSignal();
}

void HttpClient::sendSucFunc(Event& e){
    HttpClient*client=(HttpClient*)e.GetSender();
    client->m_event->SetSignal();
}

bool HttpClient::analyseUrl(string&url,string& host,string& port,string& page,bool&https){
    https=false;
    string Url=StringHelper::Trim(url);
    int nPos=Url.find("http://");
    if(nPos==0){
        Url=Url.substr(7,Url.size()-7);
    }
    else{
       nPos=Url.find("https://");
       if(nPos==0){
           https=true;
           Url=Url.substr(8,Url.size()-8);
       }
    }
    nPos=Url.find("/");
    if(nPos>0){
       host=Url.substr(0,nPos);
       page=Url.substr(nPos,Url.size()-nPos);
    }
    else{
        host=Url;
        page="/";
    }
    port=(https?"443":"80");
    nPos=host.find(":");
    if(nPos>0){
        port=host.substr(nPos+1,host.size()-nPos-1);
        host=host.substr(0,nPos);
    }
    return true;
}

AutoPtr<FixBuffer>HttpClient::innerGet(string url,AutoPtr<Semaphora>quitsignal,HeaderKeys*hks,int timeoutOfmilliseconds){
    if(helper==nullptr){
        m_exit=new Semaphora(false,false);
        m_event=new Semaphora(true,false);
        helper=new EventHelper();
        helper->Add(&this->OnSendFailed,sendfailedFunc);
        helper->Add(&this->OnReceived,sendSucFunc);
    }
    if(!m_event->Wait(1000)) return nullptr;
    string host,port,page;
    bool https;
    if(!analyseUrl(url,host,port,page,https)) return nullptr;
    if(!https){
        AutoPtr<Semaphoras>sps(new Semaphoras());
        sps->Add(m_exit);
        sps->Add(m_event);
        if(quitsignal!=nullptr) sps->Add(quitsignal);
        m_event->ResetSignal();
        net::io_context ioc;
        shared_ptr<Session>session(new Session(ioc,this,timeoutOfmilliseconds/1000));
        session->get(host.c_str(), port.c_str(), page.c_str(), 11,hks);
        if(timeoutOfmilliseconds==0){
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0) continue;
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return nullptr;
                }
                break;
            }
        }
        else{
            boost::posix_time::ptime initialTime = boost::posix_time::microsec_clock::local_time();
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0){
                    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
                    boost::posix_time::time_duration duration = currentTime - initialTime;
                    if(duration.ticks()/1000.0>=timeoutOfmilliseconds){
                        m_event->SetSignal();
                        return nullptr;
                    }
                    continue;
                }
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return nullptr;
                }
                break;
            }
        }
        AutoPtr<FixBuffer>body=session->GetBody();
        session->Reset();
        if(body==nullptr) return nullptr;
        if(session->GetGZip()){
            AutoPtr<FixBuffer>newbuffer=GZip::UncompressByGZip(body);
            return newbuffer;
        }
        else{
            return body;
        }
    }
    else{
        AutoPtr<Semaphoras>sps(new Semaphoras());
        sps->Add(m_exit);
        sps->Add(m_event);
        if(quitsignal!=nullptr) sps->Add(quitsignal);
        m_event->ResetSignal();
        net::io_context ioc;
        ssl::context ctx(ssl::context::sslv23_client);
        ctx.set_options(
              boost::asio::ssl::context::default_workarounds
              | boost::asio::ssl::context::no_sslv2
              | boost::asio::ssl::context::no_sslv3);
        ctx.set_default_verify_paths();

        shared_ptr<HttpsSession>session(new HttpsSession(ctx,ioc,this));
        session->get(host.c_str(), port.c_str(), page.c_str(), 11,hks);
        if(timeoutOfmilliseconds==0){
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0) continue;
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return nullptr;
                }
                break;
            }
        }
        else{
            boost::posix_time::ptime initialTime = boost::posix_time::microsec_clock::local_time();
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0){
                    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
                    boost::posix_time::time_duration duration = currentTime - initialTime;
                    if(duration.ticks()/1000.0>=timeoutOfmilliseconds){
                        m_event->SetSignal();
                        return nullptr;
                    }
                    continue;
                }
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return nullptr;
                }
                break;
            }
        }
        AutoPtr<FixBuffer>body=session->GetBody();
        session->Reset();
        if(body==nullptr) return nullptr;
        if(session->GetGZip()){
            AutoPtr<FixBuffer>newbuffer=GZip::UncompressByGZip(body);
            return newbuffer;
        }
        else{
            return body;
        }
    }
}

int HttpClient::innerGet(string url,AutoPtr<Semaphora>quitsignal,BYTE*buffer,int len,HeaderKeys*hks,int timeoutOfmilliseconds){
    if(helper==nullptr){
        m_exit=new Semaphora(false,false);
        m_event=new Semaphora(true,false);
        helper=new EventHelper();
        helper->Add(&this->OnSendFailed,sendfailedFunc);
        helper->Add(&this->OnReceived,sendSucFunc);
    }
    if(!m_event->Wait(1000)) return 0;
    string host,port,page;
    bool https;
    if(!analyseUrl(url,host,port,page,https)) return 0;
    if(!https){
        AutoPtr<Semaphoras>sps(new Semaphoras());
        sps->Add(m_exit);
        sps->Add(m_event);
        if(quitsignal!=nullptr) sps->Add(quitsignal);
        m_event->ResetSignal();
        net::io_context ioc;
        shared_ptr<Session>session(new Session(ioc,this,timeoutOfmilliseconds/1000));
        session->get(host.c_str(), port.c_str(), page.c_str(), 11,hks);
        if(timeoutOfmilliseconds==0){
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0) continue;
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return 0;
                }
                break;
            }
        }
        else{
            boost::posix_time::ptime initialTime = boost::posix_time::microsec_clock::local_time();
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0){
                    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
                    boost::posix_time::time_duration duration = currentTime - initialTime;
                    if(duration.ticks()/1000.0>=timeoutOfmilliseconds){
                        m_event->SetSignal();
                        return 0;
                    }
                    continue;
                }
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return 0;
                }
                break;
            }
        }
        AutoPtr<FixBuffer>body=session->GetBody();
        session->Reset();
        if(body==nullptr) return 0;
        if(session->GetGZip()){
            AutoPtr<FixBuffer>newbuffer=GZip::UncompressByGZip(body);
            if(newbuffer==nullptr){
                return 0;
            }
            else{
                int slen=newbuffer->GetSize();
                if(slen>len) slen=len;
                if(slen>0) memcpy(buffer,newbuffer->GetBuffer(),slen);
                return slen;
            }
        }
        else{
            int slen=body->GetSize();
            if(slen>len) slen=len;
            if(slen>0) memcpy(buffer,body->GetBuffer(),slen);
            return slen;
        }
    }
    else{
        AutoPtr<Semaphoras>sps(new Semaphoras());
        sps->Add(m_exit);
        sps->Add(m_event);
        if(quitsignal!=nullptr) sps->Add(quitsignal);
        m_event->ResetSignal();
        net::io_context ioc;
        //net::io_service io_service;;
        ssl::context ctx(ssl::context::sslv23_client);
        ctx.set_options(
              boost::asio::ssl::context::default_workarounds
              | boost::asio::ssl::context::no_sslv2
              | boost::asio::ssl::context::no_sslv3);
        ctx.set_default_verify_paths();
        shared_ptr<HttpsSession>session(new HttpsSession(ctx,ioc,this));
        session->get(host.c_str(), port.c_str(), page.c_str(), 11,hks);
        if(timeoutOfmilliseconds==0){
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0) continue;
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return 0;
                }
                break;
            }
        }
        else{
            boost::posix_time::ptime initialTime = boost::posix_time::microsec_clock::local_time();
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0){
                    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
                    boost::posix_time::time_duration duration = currentTime - initialTime;
                    if(duration.ticks()/1000.0>=timeoutOfmilliseconds){
                        m_event->SetSignal();
                        return 0;
                    }
                    continue;
                }
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return 0;
                }
                break;
            }
        }
        AutoPtr<FixBuffer>body=session->GetBody();
        session->Reset();
        if(body==nullptr) return 0;
        if(session->GetGZip()){
            AutoPtr<FixBuffer>newbuffer=GZip::UncompressByGZip(body);
            if(newbuffer==nullptr){
                return 0;
            }
            else{
                int slen=newbuffer->GetSize();
                if(slen>len) slen=len;
                if(slen>0) memcpy(buffer,newbuffer->GetBuffer(),slen);
                return slen;
            }
        }
        else{
            int slen=body->GetSize();
            if(slen>len) slen=len;
            if(slen>0) memcpy(buffer,body->GetBuffer(),slen);
            return slen;
        }
    }
}

AutoPtr<FixBuffer>HttpClient::innerPost(string url,AutoPtr<Semaphora>quitsignal,AutoPtr<FixBuffer>data,HeaderKeys*hks,int timeoutOfmilliseconds){
    if(helper==nullptr){
        m_exit=new Semaphora(false,false);
        m_event=new Semaphora(true,false);
        helper=new EventHelper();
        helper->Add(&this->OnSendFailed,sendfailedFunc);
        helper->Add(&this->OnReceived,sendSucFunc);
    }
    if(!m_event->Wait(1000)) return nullptr;
    string host,port,page;
    bool https;
    if(!analyseUrl(url,host,port,page,https)) return nullptr;
    if(!https){
        AutoPtr<Semaphoras>sps(new Semaphoras());
        sps->Add(m_exit);
        sps->Add(m_event);
        if(quitsignal!=nullptr) sps->Add(quitsignal);
        m_event->ResetSignal();
        net::io_context ioc;
        shared_ptr<Session>session(new Session(ioc,this,timeoutOfmilliseconds/1000));
        if(data==nullptr)
            session->post(host.c_str(), port.c_str(), page.c_str(),nullptr,0, 11,hks);
        else
            session->post(host.c_str(), port.c_str(), page.c_str(),data->GetBuffer(),data->GetSize(),11,hks);
        if(timeoutOfmilliseconds==0){
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0) continue;
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return nullptr;
                }
                break;
            }
        }
        else{
            boost::posix_time::ptime initialTime = boost::posix_time::microsec_clock::local_time();
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0){
                    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
                    boost::posix_time::time_duration duration = currentTime - initialTime;
                    if(duration.ticks()/1000.0>=timeoutOfmilliseconds){
                        m_event->SetSignal();
                        return nullptr;
                    }
                    continue;
                }
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return nullptr;
                }
                break;
            }
        }
        AutoPtr<FixBuffer>body=session->GetBody();
        session->Reset();
        if(body==nullptr) return nullptr;
        if(session->GetGZip()){
            AutoPtr<FixBuffer>newbuffer=GZip::UncompressByGZip(body);
            return newbuffer;
        }
        else{
            return body;
        }
    }
    else{
        AutoPtr<Semaphoras>sps(new Semaphoras());
        sps->Add(m_exit);
        sps->Add(m_event);
        if(quitsignal!=nullptr) sps->Add(quitsignal);
        m_event->ResetSignal();
        net::io_context ioc;
        ssl::context ctx(ssl::context::sslv23_client);
        ctx.set_options(
            boost::asio::ssl::context::default_workarounds
            | boost::asio::ssl::context::no_sslv2
            | boost::asio::ssl::context::no_sslv3);
        ctx.set_default_verify_paths();
        shared_ptr<HttpsSession>session(new HttpsSession(ctx,ioc,this));
        if(data==nullptr)
            session->post(host.c_str(), port.c_str(), page.c_str(),nullptr,0, 11,hks);
        else
            session->post(host.c_str(), port.c_str(), page.c_str(),data->GetBuffer(),data->GetSize(),11,hks);
        if(timeoutOfmilliseconds==0){
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0) continue;
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return nullptr;
                }
                break;
            }
        }
        else{
            boost::posix_time::ptime initialTime = boost::posix_time::microsec_clock::local_time();
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0){
                    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
                    boost::posix_time::time_duration duration = currentTime - initialTime;
                    if(duration.ticks()/1000.0>=timeoutOfmilliseconds){
                        m_event->SetSignal();
                        return nullptr;
                    }
                    continue;
                }
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return nullptr;
                }
                break;
            }
        }
        AutoPtr<FixBuffer>body=session->GetBody();
        session->Reset();
        if(body==nullptr) return nullptr;
        if(session->GetGZip()){
            AutoPtr<FixBuffer>newbuffer=GZip::UncompressByGZip(body);
            return newbuffer;
        }
        else{
            return body;
        }
    }
}

int HttpClient::innerPost(string url,AutoPtr<Semaphora>quitsignal,AutoPtr<FixBuffer>data,BYTE*buffer,int len,HeaderKeys*hks,int timeoutOfmilliseconds){
    if(helper==nullptr){
        m_exit=new Semaphora(false,false);
        m_event=new Semaphora(true,false);
        helper=new EventHelper();
        helper->Add(&this->OnSendFailed,sendfailedFunc);
        helper->Add(&this->OnReceived,sendSucFunc);
    }
    if(!m_event->Wait(1000)) return 0;
    string host,port,page;
    bool https;
    if(!analyseUrl(url,host,port,page,https)) return 0;
    if(!https){
        AutoPtr<Semaphoras>sps(new Semaphoras());
        sps->Add(m_exit);
        sps->Add(m_event);
        if(quitsignal!=nullptr) sps->Add(quitsignal);
        m_event->ResetSignal();
        net::io_context ioc;
        shared_ptr<Session>session(new Session(ioc,this,timeoutOfmilliseconds/1000));
        if(data==nullptr)
            session->post(host.c_str(), port.c_str(), page.c_str(),nullptr,0, 11,hks);
        else
            session->post(host.c_str(), port.c_str(), page.c_str(),data->GetBuffer(),data->GetSize(),11,hks);
        if(timeoutOfmilliseconds==0){
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0) continue;
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return 0;
                }
                break;
            }
        }
        else{
            boost::posix_time::ptime initialTime = boost::posix_time::microsec_clock::local_time();
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0){
                    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
                    boost::posix_time::time_duration duration = currentTime - initialTime;
                    if(duration.ticks()/1000.0>=timeoutOfmilliseconds){
                        m_event->SetSignal();
                        return 0;
                    }
                    continue;
                }
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return 0;
                }
                break;
            }
        }
        AutoPtr<FixBuffer>body=session->GetBody();
        session->Reset();
        if(body==nullptr) return 0;
        if(session->GetGZip()){
            AutoPtr<FixBuffer>newbuffer=GZip::UncompressByGZip(body);
            if(newbuffer==nullptr){
                return 0;
            }
            else{
                int slen=newbuffer->GetSize();
                if(slen>len) slen=len;
                if(slen>0) memcpy(buffer,newbuffer->GetBuffer(),slen);
                return slen;
            }
        }
        else{
            int slen=body->GetSize();
            if(slen>len) slen=len;
            if(slen>0) memcpy(buffer,body->GetBuffer(),slen);
            return slen;
        }
    }
    else{
        AutoPtr<Semaphoras>sps(new Semaphoras());
        sps->Add(m_exit);
        sps->Add(m_event);
        if(quitsignal!=nullptr) sps->Add(quitsignal);
        m_event->ResetSignal();
        net::io_context ioc;
        //net::io_service io_service;;
        ssl::context ctx(ssl::context::sslv23_client);
        ctx.set_options(
            boost::asio::ssl::context::default_workarounds
            | boost::asio::ssl::context::no_sslv2
            | boost::asio::ssl::context::no_sslv3);
        ctx.set_default_verify_paths();
        shared_ptr<HttpsSession>session(new HttpsSession(ctx,ioc,this));
        if(data==nullptr)
            session->post(host.c_str(), port.c_str(), page.c_str(), nullptr,0,11,hks);
        else
            session->post(host.c_str(), port.c_str(), page.c_str(),data->GetBuffer(),data->GetSize(),11,hks);
        if(timeoutOfmilliseconds==0){
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0) continue;
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return 0;
                }
                break;
            }
        }
        else{
            boost::posix_time::ptime initialTime = boost::posix_time::microsec_clock::local_time();
            while(true){
                ioc.run_for(chrono::microseconds(100));
                int state=sps->PeepForAny();
                if(state<0){
                    boost::posix_time::ptime currentTime = boost::posix_time::microsec_clock::local_time();
                    boost::posix_time::time_duration duration = currentTime - initialTime;
                    if(duration.ticks()/1000.0>=timeoutOfmilliseconds){
                        m_event->SetSignal();
                        return 0;
                    }
                    continue;
                }
                if((state==0)||(state==2)){
                    m_event->SetSignal();
                    return 0;
                }
                break;
            }
        }
        AutoPtr<FixBuffer>body=session->GetBody();
        session->Reset();
        if(body==nullptr) return 0;
        if(session->GetGZip()){
            AutoPtr<FixBuffer>newbuffer=GZip::UncompressByGZip(body);
            if(newbuffer==nullptr){
                return 0;
            }
            else{
                int slen=newbuffer->GetSize();
                if(slen>len) slen=len;
                if(slen>0) memcpy(buffer,newbuffer->GetBuffer(),slen);
                return slen;
            }
        }
        else{
            int slen=body->GetSize();
            if(slen>len) slen=len;
            if(slen>0) memcpy(buffer,body->GetBuffer(),slen);
            return slen;
        }
    }
}


AutoPtr<FixBuffer>HttpClient::doGet(string url,HeaderKeys*hks,int timeoutOfmilliseconds){
    return innerGet(url,nullptr,hks,timeoutOfmilliseconds);
}

AutoPtr<FixBuffer>HttpClient::doPost(string url,AutoPtr<FixBuffer>data,HeaderKeys*hks,int timeoutOfmilliseconds){
    return innerPost(url,nullptr,data,hks,timeoutOfmilliseconds);
}

int HttpClient::doGet(string url,BYTE*buffer,int len,HeaderKeys*hks,int timeoutOfmilliseconds){
    return innerGet(url,nullptr,buffer,len,hks,timeoutOfmilliseconds);
}

int HttpClient::doPost(string url,AutoPtr<FixBuffer>data,BYTE*buffer,int len,HeaderKeys*hks,int timeoutOfmilliseconds){
    return innerPost(url,nullptr,data,buffer,len,hks,timeoutOfmilliseconds);
}

AutoPtr<FixBuffer>HttpClient::doGet(string url,AutoPtr<Semaphora>signal,HeaderKeys*hks,int timeoutOfmilliseconds){
    return innerGet(url,signal,hks,timeoutOfmilliseconds);
}

AutoPtr<FixBuffer>HttpClient::doPost(string url,AutoPtr<FixBuffer>data,AutoPtr<Semaphora>signal,HeaderKeys*hks,int timeoutOfmilliseconds){
    return innerPost(url,signal,data,hks,timeoutOfmilliseconds);
}

int HttpClient::doGet(string url,AutoPtr<Semaphora>signal,BYTE*buffer,int len,HeaderKeys*hks,int timeoutOfmilliseconds){
    return innerGet(url,signal,buffer,len,hks,timeoutOfmilliseconds);
}

int HttpClient::doPost(string url,AutoPtr<FixBuffer>data,AutoPtr<Semaphora>signal,BYTE*buffer,int len,HeaderKeys*hks,int timeoutOfmilliseconds){
    return innerPost(url,signal,data,buffer,len,hks,timeoutOfmilliseconds);
}

bool HttpClient::downloadFile(string url,string savingPath){
    AutoPtr<FixBuffer>fs=this->doGet(url);
    if(fs==nullptr) return false;
    if(fs->GetSize()==0) return false;
    string sResponse=fs->GetBuffer();
    BinaryFile bFile;
    const string path=savingPath;
    return bFile.Write(path,fs.get());
}

RequestData::RequestData(string url,AutoPtr<FixBuffer>data,const string&remoteServerIp){
    url=StringHelper::UrlDecode(url);
    this->data=data;
    this->remoteServerIp=remoteServerIp;
    this->parameters.clear();
    this->values.clear();
    int nPos=url.find('?');
    if(nPos==-1){
        this->url=url.substr(1,nPos-1);
        return;
    }
    this->url=url.substr(1,nPos-1);
    string paraStr=url.substr(nPos+1,url.size()-nPos-1);
    vector<string>values=StringHelper::Split(paraStr,"&");
    for(int k=0;k<values.size();k++){
        int pos=values[k].find('=');
        if(pos==-1) continue;
        string para=StringHelper::UrlDecode(values[k].substr(0,pos));
        string value=StringHelper::UrlDecode(values[k].substr(pos+1,values[k].size()-pos-1));
        this->parameters.push_back(para);
        this->values.push_back(value);
    }
}

RequestData::~RequestData(){

}

NamedMap<string>&RequestData::Headers(){
    return headers;
}

int RequestData::GetParametersCount(){
    return parameters.size();
}
string RequestData::GetParameter(int nIndex){
    return parameters[nIndex];
}
string RequestData::GetValue(int nIndex){
    return values[nIndex];
}
bool RequestData::FindParameter(string parameter){
    StringHelper::TrimToLower(parameter);
    for(int k=parameters.size()-1;k>=0;k--){
        string spara= parameters[k];
        StringHelper::TrimToLower(spara);
        if(spara==parameter) return true;
    }
    return false;
}

string RequestData::FindValue(string parameter){
    StringHelper::TrimToLower(parameter);
    for(int k=parameters.size()-1;k>=0;k--){
        string spara= parameters[k];
        StringHelper::TrimToLower(spara);
        if(spara==parameter) return values[k];
    }
    return "";
}

AutoPtr<FixBuffer>RequestData::Data(){
    return data;
}

string RequestData::GetUrl(){
    return url;
}

string RequestData::GetRemoteServerIp(){
    return remoteServerIp;
}

ResponseData::ResponseData(){
    string_data="";
    buffer_data=nullptr;
    mimetype="";
    rsdType=ResponseDataType::rdtNoCompressed;
}

ResponseData::~ResponseData(){

}
string&ResponseData::MimeType(){
    return mimetype;
}

string&ResponseData::GetStringData(){
    return string_data;
}

AutoPtr<FixBuffer>ResponseData::GetBufferData(){
    return buffer_data;
}

bool ResponseData::IsBufferData(){
    return (buffer_data!=nullptr);
}

bool ResponseData::IsFile(){
    return (rsdType==ResponseDataType::rdtFile);
}

ResponseDataType ResponseData::GetResponseDataType(){
    return rsdType;
}

void ResponseData::SetStringData(const string&data,bool bCompressed){
    if(bCompressed){
        AutoPtr<FixBuffer>buffer(new FixBuffer());
        buffer->Attach((char*)data.c_str(),data.length());
        AutoPtr<FixBuffer>fxb=GZip::CompressByGZip(buffer);
        if(fxb!=nullptr){
            buffer_data=fxb;
            string_data="";
            rsdType=ResponseDataType::rdtCompressed;
            return;
        }
    }
    string_data=data;
    buffer_data=nullptr;
    rsdType=ResponseDataType::rdtNoCompressed;
}
void ResponseData::SetBufferData(AutoPtr<FixBuffer>buffer,bool bCompressed){
    if(bCompressed){
        AutoPtr<FixBuffer>fxb=GZip::CompressByGZip(buffer);
        if(fxb!=nullptr){
            buffer_data=fxb;
            string_data="";
            rsdType=ResponseDataType::rdtCompressed;
            return;
        }
    }
    buffer_data=buffer;
    string_data="";
    rsdType=ResponseDataType::rdtNoCompressed;
}

void ResponseData::SendXml(XmlDoc*xmlDoc,bool bCompressed){
    this->mimetype="text/xml;charset=utf-8";
    SetStringData(xmlDoc->GetContent(),bCompressed);
}

void ResponseData::SendHtml(const string&html,bool bCompressed){
    this->mimetype="text/html;charset=utf-8";
    SetStringData(html,bCompressed);
}

void ResponseData::SendJson(const string&json,bool bCompressed){
    this->mimetype="application/json;charset=UTF-8";
    SetStringData(json,bCompressed);
}

void ResponseData::SendText(const string&text,bool bCompressed){
    this->mimetype="text/plain;charset=UTF-8";
    SetStringData(text,bCompressed);
}

void ResponseData::SendStream(AutoPtr<FixBuffer>buffer,bool bCompress,bool rle2Encode){
    this->mimetype="application/octet-stream;charset=UTF-8";
    if(rle2Encode){
        LONG bufferLength=buffer->GetSize();
        Rle2Encoder encoder((BYTE*)buffer->GetBuffer(),buffer->GetSize());
        LONG size=encoder.ComputeEncodeLength();
        if((size>0)&&(size<bufferLength*0.95))
        {
            BYTE*codsa=new BYTE[size];
            encoder.Encode(0,codsa);
            AutoPtr<FixBuffer>fb(new FixBuffer((char*)codsa,size));     
            SetBufferData(fb,bCompress);
        }
        else
        {
            BYTE*codsa=new BYTE[bufferLength+4];
            memset(codsa,0,4);
            memcpy(codsa+4,buffer->GetBuffer(),bufferLength);
            AutoPtr<FixBuffer>fb(new FixBuffer((char*)codsa,bufferLength+4));
            SetBufferData(fb,bCompress);
        }
    }
    else
    {
        SetBufferData(buffer,bCompress);
    }
}

void ResponseData::SendStream(char*buffer,int bufferLength,bool bCompress,bool rle2Encode){
    this->mimetype="application/octet-stream;charset=UTF-8";
    if(rle2Encode){
        Rle2Encoder encoder((BYTE*)buffer,bufferLength);
        LONG size=encoder.ComputeEncodeLength();
        if((size>0)&&(size<bufferLength*0.95))
        {
            BYTE*codsa=new BYTE[size];
            encoder.Encode(0,codsa);
            AutoPtr<FixBuffer>fb(new FixBuffer((char*)codsa,size));
            SetBufferData(fb,bCompress);
        }
        else
        {
            BYTE*codsa=new BYTE[bufferLength+4];
            memset(codsa,0,4);
            memcpy(codsa+4,buffer,bufferLength);
            AutoPtr<FixBuffer>fb(new FixBuffer((char*)codsa,bufferLength+4));
            SetBufferData(fb,bCompress);
        }
    }
    else if(bCompress)
    {
        AutoPtr<FixBuffer>fb(new FixBuffer());
        fb->Attach((char*)buffer,bufferLength);
        SetBufferData(fb,bCompress);
    }
    else
    {
        AutoPtr<FixBuffer>fb(new FixBuffer());
        fb->Attach((char*)buffer,bufferLength);
        SetBufferData(fb);
    }
}

bool ResponseData::SendImage(AutoPtr<FixBuffer>buffer,string imageExa){
    StringHelper::MakeLower(imageExa);
    if(imageExa=="bmp")
        this->mimetype="image/bmp";
    else if(imageExa=="jpg")
        this->mimetype="image/jpeg";
    else if(imageExa=="jpeg")
        this->mimetype="image/jpeg";
    else if(imageExa=="png")
        this->mimetype="image/png";
    else if((imageExa=="tif")||(imageExa=="titf"))
        this->mimetype="image/tif";
    else
        return false;
    SetBufferData(buffer,false);
    return true;
}

bool ResponseData::SendImage(char*buffer,int len,string imageExa){
    StringHelper::MakeLower(imageExa);
    if(imageExa=="bmp")
        this->mimetype="image/bmp";
    else if(imageExa=="jpg")
        this->mimetype="image/jpeg";
    else if(imageExa=="jpeg")
        this->mimetype="image/jpeg";
    else if(imageExa=="png")
        this->mimetype="image/png";
    else if((imageExa=="tif")||(imageExa=="titf"))
        this->mimetype="image/tif";
    else
        return false;
    AutoPtr<FixBuffer>fb(new FixBuffer());
    fb->Attach((char*)buffer,len);
    SetBufferData(fb->Clone());
    return true;
}

bool ResponseData::SendXmlFile(string pathName){
    if(!FilePath::IsFileExisting(pathName)) return false;
    this->mimetype="text/xml;charset=utf-8";
    SetStringData(pathName);
    rsdType=ResponseDataType::rdtFile;
    return true;
}
bool ResponseData::SendHtmlFile(string pathName){
    if(!FilePath::IsFileExisting(pathName)) return false;
    this->mimetype="text/html;charset=utf-8";
    SetStringData(pathName);
    rsdType=ResponseDataType::rdtFile;
    return true;
}
bool ResponseData::SendJsonFile(string pathName){
    if(!FilePath::IsFileExisting(pathName)) return false;
    this->mimetype="application/json;charset=UTF-8";
    SetStringData(pathName);
    rsdType=ResponseDataType::rdtFile;
    return true;
}

bool ResponseData::SendImageFile(string pathName){
    if(!FilePath::IsFileExisting(pathName)) return false;
    string imageExa=FilePath::GetFileExa(pathName);
    StringHelper::MakeLower(imageExa);
    if(imageExa=="bmp")
        this->mimetype="image/bmp";
    else if(imageExa=="jpg")
        this->mimetype="image/jpeg";
    else if(imageExa=="jpeg")
        this->mimetype="image/jpeg";
    else if(imageExa=="png")
        this->mimetype="image/png";
    else if((imageExa=="tif")||(imageExa=="titf"))
        this->mimetype="image/tiff";
    else
        return false;
    SetStringData(pathName);
    rsdType=ResponseDataType::rdtFile;
    return true;
}
bool ResponseData::SendStreamFile(string pathName){
    if(!FilePath::IsFileExisting(pathName)) return false;
    this->mimetype="application/octet-stream;charset=UTF-8";
    SetStringData(pathName);
    rsdType=ResponseDataType::rdtFile;
    return true;
}

bool ResponseData::SendFile(string pathName){
    if(!FilePath::IsFileExisting(pathName)) return false;
    this->mimetype="";
    SetStringData(pathName);
    rsdType=ResponseDataType::rdtFile;
    return true;
}

AutoPtr<ResponseData>ResponseData::Clone(){
    ResponseData*rsp=new ResponseData();
    rsp->string_data=string_data;
    if(buffer_data!=nullptr) rsp->buffer_data=buffer_data->Clone();
    rsp->mimetype=mimetype;
    rsp->rsdType=rsdType;
    return rsp;
}

HttpResponseEventArgs::HttpResponseEventArgs(AutoPtr<RequestData>rqdata,AutoPtr<Semaphora>signal){
    this->rqdata=rqdata;
    this->handled=false;
    this->signal=signal;
    rsdata=new ResponseData();
}
HttpResponseEventArgs::~HttpResponseEventArgs(){

}
void HttpResponseEventArgs::FinishResponse(){
    this->handled=true;
}

bool HttpResponseEventArgs::HasHandled(){
    return handled;
}
AutoPtr<RequestData>HttpResponseEventArgs::GetRequestData(){
    return rqdata;
}
AutoPtr<ResponseData>HttpResponseEventArgs::GetResponseData(){
    return rsdata;
}

AutoPtr<Semaphora>HttpResponseEventArgs::GetSignal(){
    return this->signal;
}

HttpServer::HttpServer(){
    mimeType=new MimeType();
    pathMap=new PathMap();
    workDir="";
    serverName="SGIS";
    ResponseFunc=nullptr;
    server=nullptr;
    timeoutSeconds=60;
    //gzips.push_back("js");
}

HttpServer::~HttpServer(){
    Stop();
}

AutoPtr<MimeType>HttpServer::GetMimeType(){
    return mimeType;
}
AutoPtr<PathMap>HttpServer::GetPathMap(){
    return pathMap;
}
string HttpServer::GetWorkingDirectory(){
    return workDir;
}

int HttpServer::GetNumberWorkers(){
    return num_workers;
}

bool HttpServer::RunServer(const string&serverIp,USHORT port,const string&workDir,int num_threads,bool workerPerThread){
    this->serverIp=serverIp;
    this->port=port;
    this->workDir=workDir;
    if(num_threads<=0)
        num_workers=HostHelper::GetNumberOfProcessors()*2;
    else
        num_workers=num_threads;
    Stop();
    this->workerPerThread=workerPerThread;
    if(workerPerThread){
        http_workers*workers=new http_workers();
        workers->runserver(this);
        this->server=workers;
        if(FilePath::IsFileExisting(workDir+"/path.xml")) pathMap->ReadFromXml(workDir+"/path.xml");
        OnStartup.RaiseEvent(this);
    }
    else{
        AsyncHttpServer*server = new AsyncHttpServer(this,num_workers);
        server->start();
        this->server=server;
        if(FilePath::IsFileExisting(workDir+"/path.xml")) pathMap->ReadFromXml(workDir+"/path.xml");
        OnStartup.RaiseEvent(this);
    }
    return true;
}

bool HttpServer::RunServer(const string&serverIp,USHORT port,const string&workDir,const HttpsCertificate&cert,int num_threads){
    this->serverIp=serverIp;
    this->port=port;
    this->workDir=workDir;
    if(num_threads<=0)
        num_workers=HostHelper::GetNumberOfProcessors()*2;
    else
        num_workers=num_threads;
    Stop();
    this->workerPerThread=false;
    AsyncHttpServer*server = new AsyncHttpServer(this,num_workers,cert.certificate_chain_file,cert.private_key_file,cert.verify_file,cert.password);
    server->start();
    this->server=server;
    if(FilePath::IsFileExisting(workDir+"/path.xml")) pathMap->ReadFromXml(workDir+"/path.xml");
    OnStartup.RaiseEvent(this);
    return true;
}

void HttpServer::Stop(){
    if(this->server!=nullptr){
        if(workerPerThread){
            http_workers*workers=(http_workers*)this->server;
            workers->stopserver();
            delete workers;
        }else{
            AsyncHttpServer*Server=(AsyncHttpServer*)this->server;
            Server->stop();
            delete (AsyncHttpServer*)this->server;
        }
        this->server=nullptr;
    }
}

string HttpServer::GetServerIp(){
    return serverIp;
}
USHORT HttpServer::GetServerPort(){
    return port;
}

string&HttpServer::ServerName(){
    return serverName;
}

int HttpServer::GetTimeoutSeconds(){
    return timeoutSeconds;
}

void HttpServer::SetTimeoutSeconds(int seconds){
    timeoutSeconds=seconds;
}

void HttpServer::AddZipFileType(string fileExa){
    StringHelper::TrimToLower(fileExa);
    gzips.push_back(fileExa);
    gzipMimes.push_back(mimeType->GetStringByExa(fileExa));
}

bool HttpServer::IsFileTypeInZipList(string fileExa){
    StringHelper::TrimToLower(fileExa);
    for(int k=0;k<gzips.size();k++){
        if(gzips[k]==fileExa) return true;
    }
    return false;
}

bool HttpServer::IsMimeTypeInZipList(string mimeTye){
    StringHelper::TrimToLower(mimeTye);
    for(int k=0;k<gzipMimes.size();k++){
        if(gzipMimes[k]==mimeTye) return true;
    }
    return false;
}

class WebSocketSession
{
public:
  explicit WebSocketSession(WebSocketClient*client,int timed_out=30)
    : resolver_(ioc), ws_(ioc)
  {
       this->client=client;
       this->timed_out=timed_out;
       m_exit=new Semaphora(false,false);
  };
  void initialize(char const* host, char const* port,char const*protocol)
  {
      this->host_=host;
      this->port_=port;
      this->protocol_=protocol;
      boost::beast::websocket::stream_base::timeout timeOut=boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client);
      if(timed_out>0) {
          timeOut.handshake_timeout=std::chrono::seconds(timed_out);
      }
      else{
          timeOut.handshake_timeout=boost::beast::websocket::stream_base::none();
      }
      ws_.set_option(timeOut);
      ws_.binary(true);
      ws_.write_buffer_bytes(8192);
  }
  void do_session(boost::asio::yield_context yield){
      boost::system::error_code ec;
      tcp::resolver::results_type results=resolver_.resolve(host_.c_str(), port_.c_str(),ec);
      if(ec){
          client->OnConnetFailed.RaiseEvent(client,EventArgs());
          return;
      }
      boost::asio::async_connect(ws_.next_layer(), results.begin(), results.end(),yield[ec]);
      if(ec){
          client->OnConnetFailed.RaiseEvent(client,EventArgs());
          return;
      }
      ws_.async_handshake(host_.c_str(), protocol_.c_str(),yield[ec]);
      if(ec){
          client->OnConnetFailed.RaiseEvent(client,EventArgs());
          return;
      }
      client->OnConnected.RaiseEvent(client,EventArgs());
      for(;;){
          ws_.async_read(buffer_,yield[ec]);
          if (ec){
              client->OnStopped.RaiseEvent(client,EventArgs());
              return;
          }
          AutoPtr<FixBuffer>fixb(new FixBuffer(buffer_.size()));
          auto buf=buffer_.data();
          char* p = static_cast<char*>(buf.data());
          AutoPtr<FixBuffer>fxb=new FixBuffer();
          fxb->Attach(p,buf.size());
          WebSocketReceiveState sendState=WebSocketReceiveState::wsrsSendPackage;
          client->OnQueryReceiveState.RaiseEvent(client,EventArgs({{"sendingPack",client->GetSendingPackage().get()},{"receivedPack",fxb.get()},{"receivedState",&sendState}}));
          if(sendState==WebSocketReceiveState::wsrsSendPackage)
              client->OnSendReceived.RaiseEvent(client,EventArgs({{"data",fxb.get()}}));
          else if(sendState==WebSocketReceiveState::wsrsPostPackage)
              client->OnAccept.RaiseEvent(client,EventArgs({{"data",fxb.get()}}));
          buffer_.consume(buffer_.size());
      }
  };

  void do_write(boost::asio::yield_context yield){
      boost::system::error_code ec;
      for(;;){
          if(buffers.size()==0){
              return;
          }
          AutoPtr<FixBuffer>rsData=buffers.front();
          buffers.pop();
          ws_.binary(true);
          ws_.async_write(boost::asio::buffer(rsData->GetBuffer(),rsData->GetSize()),yield[ec]);
          if(ec){
              return;
          }
      }
  };
  void Send(AutoPtr<FixBuffer>fxb){
      buffers.push(fxb->Clone());
      boost::asio::spawn(
                  ioc.get_executor(),
                  std::bind(
                      &WebSocketSession::do_write,
                      this,
                      std::placeholders::_1));
  };
public:
    boost::asio::io_context ioc;
    tcp::resolver resolver_;
    websocket::stream<tcp::socket> ws_;
    beast::flat_buffer buffer_;
    WebSocketClient*client;
    string host_;
    string port_;
    string protocol_;
    int timed_out;
    AutoPtr<Semaphora>m_exit;
    queue<AutoPtr<FixBuffer>>buffers;
};

WebSocketClient::WebSocketClient(){
    bConnected=false;
    session=nullptr;
    acceptedBuffer=nullptr;
    sendingPack=nullptr;
}

WebSocketClient::~WebSocketClient(){
   Close();
}

bool WebSocketClient::GetConnected(){
    return bConnected;
}

void WebSocketClient::Close(){
    if(session!=nullptr){
        WebSocketSession*ss=(WebSocketSession*)session;
        ss->ws_.next_layer().close();
        ss->m_exit->SetSignal();
        ss->ioc.stop();
    }
    m_exit->SetSignal();
    if(sessionThread.joinable())
       sessionThread.join();
    if(session!=nullptr){
        WebSocketSession*ss=(WebSocketSession*)session;
        delete ss;
    }
    session=nullptr;
}

bool WebSocketClient::analyseUrl(string&url,string& host,string& port,string& protocol){
    string Url=StringHelper::Trim(url);
    int nPos=Url.find("ws://");
    if(nPos==0){
        Url=Url.substr(5,Url.size()-5);
    }
    nPos=Url.find("/");
    if(nPos>0){
       host=Url.substr(0,nPos);
       protocol=Url.substr(nPos,Url.size()-nPos);
    }
    else{
        host=Url;
        protocol="/";
    }
    port="0";
    nPos=host.find(":");
    if(nPos>0){
        port=host.substr(nPos+1,host.size()-nPos-1);
        host=host.substr(0,nPos);
    }
    return true;
}

void WebSocketClient::connectedFunc(Event& e){
    WebSocketClient*client=(WebSocketClient*)e.GetSender();
    client->bConnected=true;
    client->m_event->SetSignal();
}

void WebSocketClient::connectFailedFunc(Event& e){
    WebSocketClient*client=(WebSocketClient*)e.GetSender();
    client->bConnected=false;
    client->m_event->SetSignal();
}

void WebSocketClient::stoppedFunc(Event& e){
    WebSocketClient*client=(WebSocketClient*)e.GetSender();
    client->bConnected=false;
    client->m_event->SetSignal();
    client->m_sevent->SetSignal();
}


void WebSocketClient::sendReceivedFunc(Event& e){
    WebSocketClient*client=(WebSocketClient*)e.GetSender();
    FixBuffer*fxb=(FixBuffer*)e["data"].pData;
    client->acceptedBuffer=new FixBuffer(fxb->GetSize());
    memcpy(client->acceptedBuffer->GetBuffer(),fxb->GetBuffer(),fxb->GetSize());
    client->m_sevent->SetSignal();
}

bool WebSocketClient::Connect(string url,int timeoutOfSeconds){
    this->timeoutOfSeconds=timeoutOfSeconds;
    this->url=url;
    if(bConnected) Close();
    bConnected=false;
    if(helper==nullptr){
        m_exit=new Semaphora(false,false);
        m_event=new Semaphora(true,false);
        m_sevent=new Semaphora(true,false);
        helper=new EventHelper();
        helper->Add(&this->OnConnected,connectedFunc);
        helper->Add(&this->OnConnetFailed,connectFailedFunc);
        helper->Add(&this->OnSendReceived,sendReceivedFunc);
        helper->Add(&this->OnStopped,stoppedFunc);
    }
    WebSocketSession*ss=new WebSocketSession(this,timeoutOfSeconds);
    session=ss;
    string host,port,page;
    if(!analyseUrl(url,host,port,page)) return false;
    ss->initialize(host.c_str(),port.c_str(),page.c_str());
    boost::asio::spawn(
                ss->ioc.get_executor(),
                std::bind(
                    &WebSocketSession::do_session,
                    ss,
                    std::placeholders::_1));
    sessionThread=thread([this]{
        WebSocketSession*ss=(WebSocketSession*)this->session;
        ss->ioc.run();
    });
    AutoPtr<Semaphoras>sps(new Semaphoras());
    sps->Add(m_exit);
    sps->Add(m_event);
    m_event->ResetSignal();
    while(true){
        int waitId=sps->PeepForAny();
        if(waitId>0){
            return bConnected;
        }
    }
    return false;
}

AutoPtr<FixBuffer>WebSocketClient::GetSendingPackage(){
    return sendingPack;
}

bool WebSocketClient::Post(const string&data){
    if(!bConnected) return false;
    if(session==nullptr) return false;
    AutoPtr<FixBuffer>fxb=new FixBuffer();
    fxb->Attach((char*)data.data(),data.length());
    WebSocketSession*ss=(WebSocketSession*)this->session;
    ss->Send(fxb);
    return true;
}

bool WebSocketClient::Post(AutoPtr<FixBuffer>data){
    if(!bConnected) return false;
    if(session==nullptr) return false;
    WebSocketSession*ss=(WebSocketSession*)this->session;
    ss->Send(data);
    return true;
}

AutoPtr<FixBuffer>WebSocketClient::Send(const string&data,int timeoutOfSeconds,Semaphora*quitsignal){
    if(!bConnected) return nullptr;
    if(session==nullptr) return nullptr;
    sendingPack=nullptr;
    AutoPtr<FixBuffer>fxb=new FixBuffer();
    fxb->Attach((char*)data.data(),data.length());
    AutoPtr<Semaphoras>sps(new Semaphoras());
    sps->Add(m_exit);
    sps->Add(m_sevent);
    if(quitsignal!=nullptr){
        sps->Add(AutoPtr<Semaphora>(quitsignal,true));
    }
    acceptedBuffer=nullptr;
    m_sevent->ResetSignal();
    WebSocketSession*ss=(WebSocketSession*)this->session;
    sendingPack=fxb;
    ss->Send(fxb);
    DateTime lastTime;
    while(true){
        if(timeoutOfSeconds>0){
            if((DateTime::Now()-lastTime).TotalSeconds()>timeoutOfSeconds){
                sendingPack=nullptr;
                return nullptr;
            }
        }
        int waitId=sps->PeepForAny();
        if(waitId==1){
            sendingPack=nullptr;
            return acceptedBuffer;
        }
        else if(waitId==2){
            sendingPack=nullptr;
            return nullptr;
        }
    }
}
AutoPtr<FixBuffer>WebSocketClient::Send(AutoPtr<FixBuffer>data,int timeoutOfSeconds,Semaphora*quitsignal){
    if(!bConnected) return nullptr;
    if(session==nullptr) return nullptr;
    sendingPack=nullptr;
    AutoPtr<Semaphoras>sps(new Semaphoras());
    sps->Add(m_exit);
    sps->Add(m_sevent);
    if(quitsignal!=nullptr){
        sps->Add(AutoPtr<Semaphora>(quitsignal,true));
    }
    acceptedBuffer=nullptr;
    m_sevent->ResetSignal();
    WebSocketSession*ss=(WebSocketSession*)this->session;
    sendingPack=data;
    ss->Send(data);
    DateTime lastTime;
    while(true){
        if(timeoutOfSeconds>0){
            if((DateTime::Now()-lastTime).TotalSeconds()>timeoutOfSeconds){
                sendingPack=nullptr;
                return nullptr;
            }
        }
        int waitId=sps->PeepForAny();
        if(waitId==1){
            sendingPack=nullptr;
            return acceptedBuffer;
        }
        else if(waitId==2){
            sendingPack=nullptr;
            return nullptr;
        }
    }
}

WebSocketResponseEventArgs::WebSocketResponseEventArgs(string protocol,void*socket,AutoPtr<FixBuffer>rqdata,AutoPtr<Semaphora>signal){
    this->protocol=protocol;
    this->socket=socket;
    this->rqdata=rqdata;
    this->signal=signal;
}

WebSocketResponseEventArgs::~WebSocketResponseEventArgs(){

}

string WebSocketResponseEventArgs::GetProtocol(){
    return protocol;
}

void*WebSocketResponseEventArgs::GetSocket(){
    return socket;
}

AutoPtr<FixBuffer>WebSocketResponseEventArgs::GetRequestData(){
    return rqdata;
}

AutoPtr<Semaphora>WebSocketResponseEventArgs::GetSignal(){
    return signal;
}

WebSocketServer::WebSocketServer(){
    workDir="";
    serverName="SGIS";
    ResponseFunc=nullptr;
    TimerFunc=nullptr;
    server=nullptr;
    timeoutSeconds=60;
    timerSeconds=2;
    pathMap=new PathMap();
}

WebSocketServer::~WebSocketServer(){
    if(this->server!=nullptr){
        if(this->workerPerThread){
            websocket_workers*workers=(websocket_workers*)this->server;
            workers->stopserver();
            delete workers;
        }
        else{
            AsyncWebSocketServer*Server=(AsyncWebSocketServer*)this->server;
            Server->stop();
            delete (AsyncWebSocketServer*)this->server;
        }
    }
}

string WebSocketServer::GetWorkingDirectory(){
    return workDir;
}

AutoPtr<PathMap>WebSocketServer::GetPathMap(){
    return pathMap;
}

bool WebSocketServer::RunServer(const string&serverIp,USHORT port,const string&workDir,int num_threads,bool workerPerThread){
    this->serverIp=serverIp;
    this->port=port;
    this->workDir=workDir;
    if(num_threads<=0)
        num_workers=HostHelper::GetNumberOfProcessors()*2;
    else
        num_workers=num_threads;
    if(this->server!=nullptr){
        if(this->workerPerThread){
            websocket_workers*workers=(websocket_workers*)this->server;
            workers->stopserver();
            delete workers;
        }
        else{
            AsyncWebSocketServer*Server=(AsyncWebSocketServer*)this->server;
            Server->stop();
            delete (AsyncWebSocketServer*)this->server;
        }
    }
    this->workerPerThread=workerPerThread;
    if(workerPerThread){
        websocket_workers*workers=new websocket_workers();
        workers->runserver(this);
        this->server=workers;
    }
    else{
        AsyncWebSocketServer*server = new AsyncWebSocketServer(this,num_workers);
        server->start();
        this->server=server;
    }
    if(FilePath::IsFileExisting(workDir+"/path.xml")) pathMap->ReadFromXml(workDir+"/path.xml");
    OnStartup.RaiseEvent(this);
    return true;
}

string WebSocketServer::GetServerIp(){
    return serverIp;
}
USHORT WebSocketServer::GetServerPort(){
    return port;
}

string&WebSocketServer::ServerName(){
    return serverName;
}

int WebSocketServer::GetTimeoutSeconds(){
    return timeoutSeconds;
}
void WebSocketServer::SetTimeoutSeconds(int seconds){
    timeoutSeconds=seconds;
}

int WebSocketServer::GetNumberWorkers(){
    return num_workers;
}

int WebSocketServer::GetTimerSeconds(){
    return timerSeconds;
}
void WebSocketServer::SetTimerSeconds(int seconds){
    timerSeconds=seconds;
}

bool WebSocketServer::HasProtocol(const string&protocol){
    if(this->server!=nullptr){
        if(workerPerThread){
           websocket_workers*workers=(websocket_workers*)this->server;
           return workers->HasProtocol(protocol);
        }else{
           AsyncWebSocketServer*Server=(AsyncWebSocketServer*)this->server;
           return Server->HasProtocol(protocol);
        }
    }
    return false;
}


void WebSocketServer::LabelActive(const string&protocol){
    if(this->server!=nullptr){
        if(workerPerThread){
           websocket_workers*workers=(websocket_workers*)this->server;
           workers->LabelActive(protocol);
        }
        else{
           AsyncWebSocketServer*Server=(AsyncWebSocketServer*)this->server;
           Server->LabelActive(protocol);
        }
    }
}

bool WebSocketServer::Post(void*socket,const string&data){
    if(this->server!=nullptr){
        if(workerPerThread){
           websocket_workers*workers=(websocket_workers*)this->server;
           websocket::stream<beast::tcp_stream>*ws=(websocket::stream<beast::tcp_stream>*)socket;
           workers->Post(ws,data);
        }
        else{
           AsyncWebSocketServer*Server=(AsyncWebSocketServer*)this->server;
           websocket::stream<beast::tcp_stream>*ws=(websocket::stream<beast::tcp_stream>*)socket;
           Server->Post(ws,data);
        }
        return true;
    }
    return false;
}

bool WebSocketServer::Post(void*socket,AutoPtr<FixBuffer>buffer){
    if(this->server!=nullptr){
        if(workerPerThread){
           websocket_workers*workers=(websocket_workers*)this->server;
           websocket::stream<beast::tcp_stream>*ws=(websocket::stream<beast::tcp_stream>*)socket;
           workers->Post(ws,buffer);
        }
        else{
            AsyncWebSocketServer*Server=(AsyncWebSocketServer*)this->server;
            websocket::stream<beast::tcp_stream>*ws=(websocket::stream<beast::tcp_stream>*)socket;
            Server->Post(ws,buffer);
        }
        return true;
    }
    return false;
}


void WebSocketServer::Broadcast(const string&data){
    if(this->server!=nullptr){
        if(workerPerThread){
           websocket_workers*workers=(websocket_workers*)this->server;
           workers->Broadcast(data);
        }
        else{
           AsyncWebSocketServer*Server=(AsyncWebSocketServer*)this->server;
           Server->Broadcast(data);
        }
    }
}

void WebSocketServer::Broadcast(const string&protocol,const string&data){
    if(this->server!=nullptr){
        if(workerPerThread){
           websocket_workers*workers=(websocket_workers*)this->server;
           workers->Broadcast(protocol,data);
        }
        else{
           AsyncWebSocketServer*Server=(AsyncWebSocketServer*)this->server;
           Server->Broadcast(protocol,data);
        }
    }
}

void WebSocketServer::Broadcast(AutoPtr<FixBuffer>buffer){
    if(this->server!=nullptr){
        if(workerPerThread){
           websocket_workers*workers=(websocket_workers*)this->server;
           workers->Broadcast(buffer);
        }
        else{
           AsyncWebSocketServer*Server=(AsyncWebSocketServer*)this->server;
           Server->Broadcast(buffer);
        }
    }
}

void WebSocketServer::Broadcast(const string&protocol,AutoPtr<FixBuffer>buffer){
    if(this->server!=nullptr){
        if(workerPerThread){
           websocket_workers*workers=(websocket_workers*)this->server;
           workers->Broadcast(protocol,buffer);
        }
        else{
           AsyncWebSocketServer*Server=(AsyncWebSocketServer*)this->server;
           Server->Broadcast(protocol,buffer);
        }
    }
}

}
