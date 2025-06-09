#ifndef HTTP_WORKER_H
#define HTTP_WORKER_H

#endif // HTTP_WORKER_H
#include "../autoptr.h"
#include "../semaphora.h"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <string>
#include "../http.h"
#include "../FilePath.h"
#include "../StringHelper.h"
#include "../file.h"
#include "../ziputil.h"
#include <boost/bind.hpp>

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <thread>
#include <vector>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/strand.hpp>

#include <boost/logic/tribool.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>

namespace ssl = boost::asio::ssl;
namespace websocket = boost::beast::websocket;

namespace SGIS{

class AsyncHttpServer;

struct SocketEl{
    tcp::socket*socket;
    DateTime activeTime;
    bool waiting;
    SocketEl(tcp::socket*socket){
        this->socket=socket;
        activeTime=DateTime::Now();
        waiting=false;
    };
    SocketEl(const SocketEl&other){
        this->socket=other.socket;
        activeTime=other.activeTime;
        waiting=other.waiting;
    };
    SocketEl&operator=(const SocketEl&other){
        this->socket=other.socket;
        activeTime=other.activeTime;
        waiting=other.waiting;
        return *this;
    };
};

class SocketPool{
public:
    SocketPool(net::io_context&ioc, AsyncHttpServer*pServer);
    virtual~SocketPool();
    void AddSocket(tcp::socket*socket);
    void LabelActive(tcp::socket*socket);
    void LabelWaitingFinished(tcp::socket*socket);
    void RemoveSocket(tcp::socket*socket);
protected:
    void check_dead_line();
protected:
    net::steady_timer request_deadline_;
    vector<SocketEl>sels;
    AsyncHttpServer*pServer;
    Mutex mutex;
};


struct static_pool
{
    std::size_t size_;
    std::size_t refs_ = 1;
    std::size_t count_ = 0;
    char* p_;

    char*
    end()
    {
        return reinterpret_cast<char*>(this + 1) + size_;
    }

    explicit
        static_pool(std::size_t size)
        : size_(size)
        , p_(reinterpret_cast<char*>(this + 1))
    {
    }

public:
    static
        static_pool&
        construct(std::size_t size)
    {
        auto p = new char[sizeof(static_pool) + size];
        return *(::new(p) static_pool{size});
    }

    static_pool&
    share()
    {
        ++refs_;
        return *this;
    }

    void
    destroy()
    {
        if(refs_--)
            return;
        this->~static_pool();
        delete[] reinterpret_cast<char*>(this);
    }

    void*
    alloc(std::size_t n)
    {
        auto last = p_ + n;
        if(last >= end())
            BOOST_THROW_EXCEPTION(std::bad_alloc{});
        ++count_;
        auto p = p_;
        p_ = last;
        return p;
    }

    void
    dealloc()
    {
        if(--count_)
            return;
        p_ = reinterpret_cast<char*>(this + 1);
    }
};

template<class T>
struct fields_alloc
{
    static_pool* pool_;

public:
    using value_type = T;
    using is_always_equal = std::false_type;
    using pointer = T*;
    using reference = T&;
    using const_pointer = T const*;
    using const_reference = T const&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template<class U>
    struct rebind
    {
        using other = fields_alloc<U>;
    };

#if defined(_GLIBCXX_USE_CXX11_ABI) && (_GLIBCXX_USE_CXX11_ABI == 0)
    // Workaround for g++
    // basic_string assumes that allocators are default-constructible
    // See: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=56437
    fields_alloc() = default;
#endif

    explicit
        fields_alloc(std::size_t size)
        : pool_(&static_pool::construct(size))
    {
    }

    fields_alloc(fields_alloc const& other)
        : pool_(&other.pool_->share())
    {
    }

    template<class U>
    fields_alloc(fields_alloc<U> const& other)
        : pool_(&other.pool_->share())
    {
    }

    ~fields_alloc()
    {
        pool_->destroy();
    }

    value_type*
    allocate(size_type n)
    {
        return static_cast<value_type*>(
            pool_->alloc(n * sizeof(T)));
    }

    void
    deallocate(value_type*, size_type)
    {
        pool_->dealloc();
    }

#if defined(BOOST_LIBSTDCXX_VERSION) && BOOST_LIBSTDCXX_VERSION < 60000
    template<class U, class... Args>
    void
    construct(U* ptr, Args&&... args)
    {
        ::new(static_cast<void*>(ptr)) U(
            std::forward<Args>(args)...);
    }

    template<class U>
    void
    destroy(U* ptr)
    {
        ptr->~U();
    }
#endif

    template<class U>
    friend
        bool
        operator==(
            fields_alloc const& lhs,
            fields_alloc<U> const& rhs)
    {
        return &lhs.pool_ == &rhs.pool_;
    }

    template<class U>
    friend
        bool
        operator!=(
            fields_alloc const& lhs,
            fields_alloc<U> const& rhs)
    {
        return ! (lhs == rhs);
    }
};

class http_worker
{
private:
    using alloc_t = fields_alloc<char>;
    //using request_body_t = http::basic_dynamic_body<beast::flat_static_buffer<1024 * 1024>>;
    using request_body_t = http::string_body;
    // The path to the root of the document directory.
    std::string doc_root_;

    // The socket for the currently connected client.
    AutoPtr<tcp::socket>socket_;

    // The buffer for performing reads
    beast::flat_static_buffer<8192> buffer_;

    // The allocator used for the fields in the request and reply.
    alloc_t alloc_{8192};

    // The parser for reading the requests
    boost::optional<http::request_parser<request_body_t, alloc_t>> parser_;

    http::response<http::empty_body> empty_response_;
    AutoPtr<http::response_serializer<http::empty_body>>empty_serializer_=nullptr;
    http::response<http::string_body> string_response_;
    AutoPtr<http::response_serializer<http::string_body>>string_serializer_=nullptr;
    http::response<http::file_body> file_response_;
    AutoPtr<http::response_serializer<http::file_body>>file_serializer_=nullptr;
    http::response<http::buffer_body> buffer_response_;
    AutoPtr<http::response_serializer<http::buffer_body>>buffer_serializer_=nullptr;
    vector<BYTE>sendingCompressedData;
    AutoPtr<FixBuffer>sendingBuffer;
    string sendingContent;
    AutoPtr<MimeType>mimeType;
    AutoPtr<PathMap>pathMap;
    HttpServer*pServer;
    AutoPtr<Semaphora>signal;
    AutoPtr<ResourceIndex>lock;
    DateTime lastTime;
private:
    void read_request()
    {
        buffer_.consume(buffer_.size());
        parser_.emplace(
            std::piecewise_construct,
            std::make_tuple(),
            std::make_tuple(alloc_));
        size_t size=1024;
        size=size*10*1024;
        parser_.get().body_limit(size);
        http::async_read(
            *socket_,
            buffer_,
            *parser_,
            [this](beast::error_code ec, std::size_t)
            {
                if (!ec){
                    string sClientIp = socket_->remote_endpoint().address().to_string();
                    process_request(parser_->get(),sClientIp);
                }
                else{
                    this->signal->SetSignal();
                    this->lock=nullptr;
                }
            });
    }
    void process_request(http::request<request_body_t, http::basic_fields<alloc_t>> const& req,const string&remoteServerIp)
    {
        if(req.method()==http::verb::options){
            empty_response_.result(http::status::ok);
            empty_response_.keep_alive(false);
            empty_response_.set(http::field::server, pServer->ServerName());
            empty_response_.set(http::field::content_type,"text/html" );
            empty_response_.set(http::field::keep_alive,req.keep_alive()?"true":"false");
            empty_response_.set(http::field::access_control_allow_origin,"*");
            empty_response_.set(http::field::access_control_allow_headers,"token,content-type");
            empty_response_.set(http::field::access_control_allow_credentials,"true");
            //DateTime curTime;
            //empty_response_.set(http::field::last_modified,curTime.GetHttpTime());
            //empty_response_.prepare_payload();
            empty_serializer_=new http::response_serializer<http::empty_body>(empty_response_);
            boost::beast::http::async_write(
                *socket_,
                *empty_serializer_,
                [this](beast::error_code ec, std::size_t)
                {
                    socket_->shutdown(tcp::socket::shutdown_send, ec);
                    empty_serializer_=nullptr;
                    empty_response_.clear();
                    this->signal->SetSignal();
                    this->lock=nullptr;
                });
            return;
        }
        string target=string(req.target());
        target=StringHelper::UrlDecode(target);
        //target=StringHelper::ToLocalString(target);
        std::string full_path = doc_root_;
        full_path.append(target.data(),target.size());
        string pageHtml=pServer->GetWorkingDirectory()+"/web";
        pageHtml=FilePath::ConvertToStardardPath(pageHtml);
        StringHelper::MakeLower(pageHtml);
        string tempPath=full_path;
        StringHelper::MakeLower(tempPath);
        if(tempPath==pageHtml){
            if(FilePath::IsFileExisting(pageHtml+"/index.html")){
                return send_file(req,pageHtml+"/index.html");
            }
        }
        int lpos=full_path.find('?');
        if(lpos>0){
            full_path=full_path.substr(0,lpos);
        }
        if(!FilePath::IsFileExisting(full_path)){
            string sTarget=pathMap->MapPath(target);
            full_path=sTarget;
        }
        if(FilePath::IsFileExisting(full_path)){
            return send_file(req,full_path);
        }
        AutoPtr<FixBuffer>buffer=nullptr;
        if(req.method()==http::verb::post){
            buffer=new FixBuffer();
            buffer->Attach((char*)req.body().data(),req.body().size());
        }
        AutoPtr<RequestData>rqData(new RequestData(target,buffer,remoteServerIp));
        NamedMap<string>&headers=rqData->Headers();
        for(auto citer=req.begin();citer!=req.end();citer++)
        {
            string name="";
            int nsize=citer->name_string().size();
            name.resize(nsize);
            memcpy(name.data(),citer->name_string().data(),nsize);
            string iv="";
            nsize=citer->value().size();
            iv.resize(nsize);
            memcpy(iv.data(),citer->value().data(),nsize);
            headers.Add(NamedItem<string>(name,iv));
        }
        this->signal->ResetSignal();
        AutoPtr<HttpResponseEventArgs>eventArgs(new HttpResponseEventArgs(rqData,this->signal));
        if(pServer->ResponseFunc!=nullptr){
            HTTPResponseEvent event(pServer,eventArgs);
            pServer->ResponseFunc(event);
        }
        if(eventArgs->HasHandled()){
            AutoPtr<ResponseData>rsData=eventArgs->GetResponseData();
            if(!rsData->IsBufferData()){
                if(rsData->IsFile()){
                    std::string full_path =rsData->GetStringData();
                    send_file(req,full_path);
                    return;
                }
                else{
                    try {
                        return send_string(req,rsData->GetStringData(),rsData->MimeType());
                    }
                    catch (exception e) {

                    }
                }
            }
            else{
                try {
                    AutoPtr<FixBuffer>fxb=rsData->GetBufferData();
                    return send_buffer(req,fxb,rsData->MimeType(),rsData->GetResponseDataType());
                } catch (exception e) {

                }
            }
            this->lock=nullptr;
        }
        else
            return send_bad_response(http::status::not_found,"Illegal request");
    };
public:
    void send_bad_response(
        http::status status,
        std::string const& error)
    {
        string_response_.result(status);
        string_response_.keep_alive(false);
        string_response_.set(http::field::server, pServer->ServerName());
        string_response_.set(http::field::content_type, "text/plain");
        string_response_.set(http::field::access_control_allow_origin,"*");
        DateTime curTime;
        string httpTime=curTime.GetHttpTime();
        string_response_.set(http::field::last_modified,httpTime);
        string_response_.body()=error;
        string_response_.prepare_payload();
        string_serializer_=new http::response_serializer<http::string_body>(string_response_);
        boost::beast::http::async_write(
            *socket_,
            *string_serializer_,
            [this](beast::error_code ec, std::size_t)
            {
                socket_->shutdown(tcp::socket::shutdown_send, ec);
                string_serializer_=nullptr;
                string_response_.clear();
                this->signal->SetSignal();
                this->lock=nullptr;
        });
    };
    void send_file(http::request<request_body_t, http::basic_fields<alloc_t>> const& req,string full_path)
    {
        string fileExa=FilePath::GetFileExa(full_path);
        if(this->pServer->IsFileTypeInZipList(fileExa)){
            string zipFile=FilePath::GetDir(full_path)+"/"+FilePath::GetFileName(full_path)+".gz";
            if(FilePath::IsFileExisting(zipFile)){
                if(req.method()==http::verb::head){
                    BinaryFile bf;
                    bf.OpenForRead(zipFile);
                    int len=bf.GetFileLength();
                    empty_response_.result(http::status::ok);
                    empty_response_.keep_alive(false);
                    empty_response_.set(http::field::server, pServer->ServerName());
                    empty_response_.set(http::field::content_type, mimeType->GetString(std::string(full_path)));
                    empty_response_.set(http::field::content_length,to_string(len));
                    empty_response_.set(http::field::access_control_allow_origin,"*");
                    empty_response_.set(http::field::access_control_allow_headers,"token,content-type");
                    empty_response_.set(http::field::access_control_allow_credentials,"true");
                    DateTime curTime;
                    empty_response_.set(http::field::last_modified,curTime.GetHttpTime());
                    empty_response_.prepare_payload();
                    empty_serializer_=new http::response_serializer<http::empty_body>(empty_response_);
                    boost::beast::http::async_write(
                        *socket_,
                        *empty_serializer_,
                        [this](beast::error_code ec, std::size_t)
                        {
                            socket_->shutdown(tcp::socket::shutdown_send, ec);
                            empty_serializer_=nullptr;
                            empty_response_.clear();
                            this->signal->SetSignal();
                            this->lock=nullptr;
                        });
                }
                else{
                    http::file_body::value_type file;
                    beast::error_code ec;
                    file.open(
                        zipFile.c_str(),
                        beast::file_mode::read,
                        ec);
                    if(ec)
                    {
                        send_bad_response(
                            http::status::not_found,
                            "File not found\r\n");
                        return;
                    }
                    file_response_.result(http::status::ok);
                    file_response_.keep_alive(false);
                    file_response_.set(http::field::server, pServer->ServerName());
                    file_response_.set(http::field::content_type, mimeType->GetString(std::string(full_path)));
                    file_response_.set(http::field::access_control_allow_origin,"*");
                    file_response_.set(http::field::access_control_allow_headers,"token,content-type");
                    file_response_.set(http::field::content_encoding,"gzip");
                    DateTime curTime;
                    file_response_.set(http::field::last_modified,curTime.GetHttpTime());
                    file_response_.body()=std::move(file);
                    file_response_.prepare_payload();
                    file_serializer_=new http::response_serializer<http::file_body>(file_response_);
                    boost::beast::http::async_write(
                        *socket_,
                        *file_serializer_,
                        [this](beast::error_code ec, std::size_t)
                        {
                            socket_->shutdown(tcp::socket::shutdown_send, ec);
                            file_serializer_=nullptr;
                            file_response_.body().file().close(ec);
                            file_response_.clear();
                            this->signal->SetSignal();
                            this->lock=nullptr;
                        });
                }
            }
            else{
                sendingCompressedData=GZip::CompressFileToBuffer(full_path);
                if(req.method()==http::verb::head)
                {
                    empty_response_.result(http::status::ok);
                    empty_response_.keep_alive(false);
                    empty_response_.set(http::field::server, pServer->ServerName());
                    empty_response_.set(http::field::content_type, mimeType->GetString(std::string(full_path)));
                    empty_response_.set(http::field::content_length,to_string(sendingCompressedData.size()));
                    empty_response_.set(http::field::access_control_allow_origin,"*");
                    empty_response_.set(http::field::access_control_allow_headers,"token,content-type");
                    empty_response_.set(http::field::access_control_allow_credentials,"true");
                    DateTime curTime;
                    empty_response_.set(http::field::last_modified,curTime.GetHttpTime());
                    empty_response_.prepare_payload();
                    empty_serializer_=new http::response_serializer<http::empty_body>(empty_response_);
                    boost::beast::http::async_write(
                        *socket_,
                        *empty_serializer_,
                        [this](beast::error_code ec, std::size_t)
                        {
                            socket_->shutdown(tcp::socket::shutdown_send, ec);
                            empty_serializer_=nullptr;
                            empty_response_.clear();
                            this->signal->SetSignal();
                            this->lock=nullptr;
                        });
                }
                else{
                    buffer_response_.result(http::status::ok);
                    buffer_response_.keep_alive(false);
                    buffer_response_.set(http::field::server, pServer->ServerName());
                    buffer_response_.set(http::field::content_type, mimeType->GetString(std::string(full_path)));
                    buffer_response_.set(http::field::content_length,to_string(sendingCompressedData.size()));
                    buffer_response_.set(http::field::access_control_allow_origin,"*");
                    buffer_response_.set(http::field::access_control_allow_headers,"token,content-type");
                    buffer_response_.set(http::field::access_control_allow_credentials,"true");
                    http::buffer_body::value_type data;
                    data.data=sendingCompressedData.data();
                    data.size=sendingCompressedData.size();
                    data.more=false;
                    buffer_response_.body()=std::move(data);
                    buffer_response_.prepare_payload();
                    DateTime curTime;
                    buffer_response_.set(http::field::last_modified,curTime.GetHttpTime());
                    buffer_serializer_=new http::response_serializer<http::buffer_body>(buffer_response_);
                    boost::beast::http::async_write(
                        *socket_,
                        *buffer_serializer_,
                        [this](beast::error_code ec, std::size_t)
                        {
                            socket_->shutdown(tcp::socket::shutdown_send, ec);
                            buffer_serializer_=nullptr;
                            buffer_response_.clear();
                            sendingCompressedData.clear();
                            this->signal->SetSignal();
                            this->lock=nullptr;
                        });
                }
            }
        }
        else{
            if(req.method()==http::verb::head){
                BinaryFile bf;
                bf.OpenForRead(full_path);
                int len=bf.GetFileLength();
                empty_response_.result(http::status::ok);
                empty_response_.keep_alive(false);
                empty_response_.set(http::field::server, pServer->ServerName());
                empty_response_.set(http::field::content_type, mimeType->GetString(std::string(full_path)));
                empty_response_.set(http::field::content_length,to_string(len));
                empty_response_.set(http::field::access_control_allow_origin,"*");
                empty_response_.set(http::field::access_control_allow_headers,"token,content-type");
                empty_response_.set(http::field::access_control_allow_credentials,"true");
                DateTime curTime;
                empty_response_.set(http::field::last_modified,curTime.GetHttpTime());
                empty_response_.prepare_payload();
                empty_serializer_=new http::response_serializer<http::empty_body>(empty_response_);
                boost::beast::http::async_write(
                    *socket_,
                    *empty_serializer_,
                    [this](beast::error_code ec, std::size_t)
                    {
                        socket_->shutdown(tcp::socket::shutdown_send, ec);
                        empty_serializer_=nullptr;
                        empty_response_.clear();
                        this->signal->SetSignal();
                        this->lock=nullptr;
                    });
            }
            else{
                http::file_body::value_type file;
                beast::error_code ec;
                file.open(
                    full_path.c_str(),
                    beast::file_mode::read,
                    ec);
                if(ec)
                {
                    send_bad_response(
                        http::status::not_found,
                        "File not found\r\n");
                    return;
                }
                file_response_.result(http::status::ok);
                file_response_.keep_alive(false);
                file_response_.set(http::field::server, pServer->ServerName());
                file_response_.set(http::field::content_type, mimeType->GetString(std::string(full_path)));
                file_response_.set(http::field::access_control_allow_origin,"*");
                file_response_.set(http::field::access_control_allow_headers,"token,content-type");
                DateTime curTime;
                file_response_.set(http::field::last_modified,curTime.GetHttpTime());
                file_response_.body()=std::move(file);
                file_response_.prepare_payload();
                file_serializer_=new http::response_serializer<http::file_body>(file_response_);
                boost::beast::http::async_write(
                    *socket_,
                    *file_serializer_,
                    [this](beast::error_code ec, std::size_t)
                    {
                        socket_->shutdown(tcp::socket::shutdown_send, ec);
                        file_serializer_=nullptr;
                        file_response_.body().file().close(ec);
                        file_response_.clear();
                        this->signal->SetSignal();
                        this->lock=nullptr;
                    });
            }
        }
    };

    void send_string(http::request<request_body_t, http::basic_fields<alloc_t>> const& req,const string&content,string mimeType){
        if(req.method()==http::verb::head){
            empty_response_.result(http::status::ok);
            empty_response_.keep_alive(false);
            empty_response_.set(http::field::server, pServer->ServerName());
            empty_response_.set(http::field::content_type, mimeType);
            empty_response_.set(http::field::content_length,to_string(content.length()));
            empty_response_.set(http::field::access_control_allow_origin,"*");
            empty_response_.set(http::field::access_control_allow_headers,"token,content-type");
            empty_response_.set(http::field::access_control_allow_credentials,"true");
            DateTime curTime;
            empty_response_.set(http::field::last_modified,curTime.GetHttpTime());
            empty_response_.prepare_payload();
            empty_serializer_=new http::response_serializer<http::empty_body>(empty_response_);
            boost::beast::http::async_write(
                *socket_,
                *empty_serializer_,
                [this](beast::error_code ec, std::size_t)
                {
                    socket_->shutdown(tcp::socket::shutdown_send, ec);
                    empty_serializer_=nullptr;
                    empty_response_.clear();
                    this->signal->SetSignal();
                    this->lock=nullptr;
                });
        }
        else{
            sendingContent=content;
            string_response_.result(http::status::ok);
            string_response_.keep_alive(false);
            string_response_.set(http::field::server, pServer->ServerName());
            string_response_.set(http::field::content_type, mimeType);
            string_response_.set(http::field::access_control_allow_origin,"*");
            string_response_.set(http::field::access_control_allow_headers,"token,content-type");
            DateTime curTime;
            string_response_.set(http::field::last_modified,curTime.GetHttpTime());
            string_response_.body()=sendingContent;
            string_response_.prepare_payload();
            string_serializer_=new http::response_serializer<http::string_body>(string_response_);
            boost::beast::http::async_write(
                *socket_,
                *string_serializer_,
                [this](beast::error_code ec, std::size_t)
                {
                    socket_->shutdown(tcp::socket::shutdown_send, ec);
                    string_serializer_=nullptr;
                    string_response_.clear();
                    sendingContent="";
                    this->signal->SetSignal();
                    this->lock=nullptr;
                });
        }
    };
    void send_buffer(http::request<request_body_t, http::basic_fields<alloc_t>> const& req,AutoPtr<FixBuffer>buffer,string mimeType,ResponseDataType rdt){
        if(req.method()==http::verb::head)
        {
            empty_response_.result(http::status::ok);
            empty_response_.keep_alive(false);
            empty_response_.set(http::field::server, pServer->ServerName());
            empty_response_.set(http::field::content_type, mimeType);
            empty_response_.set(http::field::content_length,to_string(buffer->GetSize()));
            empty_response_.set(http::field::access_control_allow_origin,"*");
            empty_response_.set(http::field::access_control_allow_headers,"token,content-type");
            empty_response_.set(http::field::access_control_allow_credentials,"true");
            DateTime curTime;
            empty_response_.set(http::field::last_modified,curTime.GetHttpTime());
            empty_response_.prepare_payload();
            empty_serializer_=new http::response_serializer<http::empty_body>(empty_response_);
            boost::beast::http::async_write(
                *socket_,
                *empty_serializer_,
                [this](beast::error_code ec, std::size_t)
                {
                    socket_->shutdown(tcp::socket::shutdown_send, ec);
                    empty_serializer_=nullptr;
                    empty_response_.clear();
                    this->signal->SetSignal();
                    this->lock=nullptr;
                });
        }
        else{
            buffer_response_.result(http::status::ok);
            buffer_response_.keep_alive(false);
            buffer_response_.set(http::field::server, pServer->ServerName());
            buffer_response_.set(http::field::content_type, mimeType);
            buffer_response_.set(http::field::content_length,to_string(buffer->GetSize()));
            buffer_response_.set(http::field::access_control_allow_origin,"*");
            buffer_response_.set(http::field::access_control_allow_headers,"token,content-type");
            buffer_response_.set(http::field::access_control_allow_credentials,"true");
            if(rdt==ResponseDataType::rdtCompressed) buffer_response_.set(http::field::content_encoding,"gzip");
            sendingBuffer=buffer;
            http::buffer_body::value_type data;
            data.data=(void*)sendingBuffer->GetBuffer();
            data.size=sendingBuffer->GetSize();
            data.more=false;
            buffer_response_.body()=std::move(data);
            buffer_response_.prepare_payload();
            DateTime curTime;
            buffer_response_.set(http::field::last_modified,curTime.GetHttpTime());
            buffer_serializer_=new http::response_serializer<http::buffer_body>(buffer_response_);
            boost::beast::http::async_write(
                *socket_,
                *buffer_serializer_,
                [this](beast::error_code ec, std::size_t)
                {
                    socket_->shutdown(tcp::socket::shutdown_send, ec);
                    buffer_serializer_=nullptr;
                    buffer_response_.clear();
                    sendingBuffer=nullptr;
                    this->signal->SetSignal();
                    this->lock=nullptr;
                });
        }
    };
public:
    http_worker(http_worker const&) = delete;

    http_worker& operator=(http_worker const&) = delete;

    http_worker()
    {
        this->signal=new Semaphora(false,false);
    }

    void start(AutoPtr<ResourceIndex>lock,HttpServer*pServer,AutoPtr<tcp::socket>socket)
    {
        this->socket_=socket;
        this->pServer=pServer;
        doc_root_=pServer->GetWorkingDirectory();
        this->mimeType=pServer->GetMimeType();
        this->pathMap=pServer->GetPathMap();
        this->lock=lock;
        lastTime=DateTime::Now();
        read_request();
    }
    void checkTimeOut(){
        if(lock==nullptr) return;
        if(((DateTime::Now()-lastTime).TotalSeconds()>=pServer->GetTimeoutSeconds())){
            beast::error_code ec;
            socket_->shutdown(tcp::socket::shutdown_send, ec);
            this->signal->SetSignal();
            this->lock=nullptr;
        }
    }
};

class http_workers{
public:
    http_workers()
    {

    };
    ~http_workers(){
        stopserver();
    };

    void runserver(HttpServer*pServer){
        this->pServer=pServer;
        this->serverIp=pServer->GetServerIp();
        this->port=pServer->GetServerPort();
        this->workDir=pServer->GetWorkingDirectory();
        this->num_threads=pServer->GetNumberWorkers();
        int num_workers=0;
        if(num_threads>0)
            num_workers=num_threads;
        else
            num_workers=HostHelper::GetNumberOfProcessors();
        int pool_size=num_workers*4;
        httpworks.resize(pool_size);
        for(int k=0;k<pool_size;k++){
            httpworks[k]=new http_worker();
        }
        workers_pool=new ResourcePool(pool_size);
        auto const address = net::ip::make_address(serverIp);
        acceptor=new  tcp::acceptor(io_context_, {address, port});
        //acceptor->set_option(net::socket_base::reuse_address(true));
        start_accept();
        for (int i = 0; i < num_workers; ++i) {
            threads.emplace_back([this] {
                while(1){
                    try{
                        io_context_.run();
                        break;
                    }
                    catch (const std::exception& e)
                    {

                    }
                }
            });
        }
        request_deadline_.expires_after(std::chrono::seconds(5));
        check_dead_line();
    }

    void stopserver(){
        request_deadline_.expires_after(std::chrono::seconds::max());
        io_context_.stop();
            // 等待所有线程结束
        for (auto& t : threads) {
            t.join();
        }
    };
    void check_dead_line(){
        if (request_deadline_.expiry() <= std::chrono::steady_clock::now())
        {
            DateTime curTime;
            for(int k=httpworks.size()-1;k>=0;k--){
                httpworks[k]->checkTimeOut();
            }
            request_deadline_.expires_after(std::chrono::seconds(5));
        }
        request_deadline_.async_wait(
            [this](beast::error_code)
            {
                check_dead_line();
        });
    }
protected:
    void start_accept() {
        acceptor->async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    AutoPtr<ResourceIndex>rsi=workers_pool->LockResourceEx();
                    if(rsi!=nullptr){
                        AutoPtr<http_worker>worker=httpworks[rsi->GetResourceId()];
                        AutoPtr<tcp::socket>t_socket=new tcp::socket(std::move(socket));
                        worker->start(rsi,pServer,t_socket);
                    }
                }
                start_accept(); // 继续接受新连
            });
    };
protected:
    string serverIp;
    USHORT port;
    string workDir;
    int num_threads;
    HttpServer*pServer;
    vector<thread>threads;
    net::io_context io_context_;
    AutoPtr<tcp::acceptor>acceptor;
    AutoPtr<ResourcePool>workers_pool;
    vector<AutoPtr<http_worker>>httpworks;
    net::steady_timer request_deadline_{io_context_, (std::chrono::steady_clock::time_point::max)()};
};

template <class Stream>
struct send_lambda
{
    Stream &stream_;
    bool &close_;
    boost::system::error_code &ec_;
    boost::asio::yield_context yield_;

    explicit send_lambda(
            Stream &stream,
            bool &close,
            boost::system::error_code &ec,
            boost::asio::yield_context yield)
        : stream_(stream), close_(close), ec_(ec), yield_(yield)
    {
    }

    template <bool isRequest, class Body, class Fields>
    void operator()(http::message<isRequest, Body, Fields> &&msg) const
    {
        // Determine if we should close the connection after
        close_ = msg.need_eof();

        // We need the serializer here because the serializer requires
        // a non-const file_body, and the message oriented version of
        // http::write only works with const messages.
        http::serializer<isRequest, Body, Fields> sr{msg};
        http::async_write(stream_, sr, yield_[ec_]);
    }
};

class AsyncHttpServer
{
public:
    AsyncHttpServer(HttpServer*pServer,int threadNum);
    AsyncHttpServer(HttpServer*pServer,int threadNum,std::string certChainFile,string privateKeyFile,string verifyfile="",string password="");
    ~AsyncHttpServer();
    void start();
    void stop();
    int getTimeoutSeconds();
private:
    bool load_server_certificate();
    void do_listen(
            boost::asio::yield_context yield);
    void do_session(
            tcp::socket &socket,
            boost::asio::yield_context yield);
    void do_session_http(
            tcp::socket &socket,
            boost::asio::yield_context yield);

    template <class Body, class Allocator, class Send>
    void SendFile(const string&path,http::request<Body, http::basic_fields<Allocator>> &req,
                                   Send &&send);

    template <class Body, class Allocator, class Send>
    void SendStringData(AutoPtr<ResponseData>rqdata,http::request<Body, http::basic_fields<Allocator>> &req,
                                   Send &&send);

    template <class Body, class Allocator, class Send>
    void SendBufferData(AutoPtr<ResponseData>rqdata,http::request<Body, http::basic_fields<Allocator>> &req,
                                   Send &&send);

    template <class Body, class Allocator, class Send>
    void handle_request(
            http::request<Body, http::basic_fields<Allocator>> &&req,
            Send &&send,const string&remoteServerIp);
protected:
    std::string path_cat(boost::beast::string_view base, boost::beast::string_view path);
    bool on_verify_certificate (bool preverified, boost::asio::ssl::verify_context & ctx);

    net::ip::address address;
    unsigned short port;
    std::shared_ptr<std::string> doc_root;
    unsigned short threads;
    net::io_context ioc;
    ssl::context ctx;
    //std::string tls12CipherSuite;
    std::string certChainFile;
    std::string privateKeyFile;
    std::string password;
    std::string verifyfile;
    std::vector<std::thread> threadList;

    AutoPtr<MimeType>mimeType;
    AutoPtr<PathMap>pathMap;
    HttpServer*pServer;
    bool https;
    SocketPool socketPool;
};

class AsyncWebSocketServer;

struct WebSocketConnect{
    websocket::stream<beast::tcp_stream>*socket;
    string protocol;
    DateTime activeTime;
    bool send_waiting;
    bool recv_waiting;
    WebSocketConnect(websocket::stream<beast::tcp_stream>*socket,string protocol)
    {
        this->socket=socket;
        this->protocol=protocol;
        this->activeTime=DateTime::Now();
        send_waiting=false;
        recv_waiting=false;
    };
    WebSocketConnect(const WebSocketConnect&other){
        this->socket=other.socket;
        this->protocol=other.protocol;
        this->activeTime=other.activeTime;
        this->send_waiting=other.send_waiting;
        this->recv_waiting=other.recv_waiting;
    };
    void UpdateActiveTime(){
        this->activeTime=DateTime::Now();
    };
};

struct WebSocketSendData{
    websocket::stream<beast::tcp_stream>*socket=nullptr;
    string protocol="";
    AutoPtr<FixBuffer>data;
    bool binary;
    WebSocketSendData(websocket::stream<beast::tcp_stream>*socket,const string&protocol,AutoPtr<FixBuffer>data){
        this->socket=socket;
        this->protocol=protocol;
        this->data=data;
        binary=true;
    };
    WebSocketSendData(websocket::stream<beast::tcp_stream>*socket,const string&protocol,const string&data){
        this->socket=socket;
        this->protocol=protocol;
        this->data=new FixBuffer(data.length());
        memcpy(this->data->GetBuffer(),data.data(),data.length());
        binary=false;
    };
};

class WebSocketPool{
public:
    WebSocketPool(net::io_context&ioc, AsyncWebSocketServer*pServer);
    virtual~WebSocketPool();
    void AddSocket(websocket::stream<beast::tcp_stream>*socket,const string&protocol);
    void LabelActive(websocket::stream<beast::tcp_stream>*socket);
    void LabelWaitingFinished(websocket::stream<beast::tcp_stream>*socket);
    void RemoveSocket(websocket::stream<beast::tcp_stream>*socket);
    bool HasProtocol(const string&protocol);
    void LabelActive(const string&protocol);
    bool Send(websocket::stream<beast::tcp_stream>*socket,AutoPtr<FixBuffer>buffer,bool binary,net::yield_context&yield);
    void Broadcast(AutoPtr<FixBuffer>buffer,bool binary,net::yield_context&yield);
    void Broadcast(const string&protocol,AutoPtr<FixBuffer>buffer,bool binary,net::yield_context&yield);
    void check_dead_line();
protected:
    vector<AutoPtr<WebSocketConnect>>conns;
    AsyncWebSocketServer*pServer;
    Mutex mutex;
    Mutex smutex;
};

class AsyncWebSocketServer{
public:
    AsyncWebSocketServer(WebSocketServer*pServer,int threadNum);
    ~AsyncWebSocketServer();
    void start();
    void stop();
    int getTimeoutSeconds();
    int getTimerSeconds();
    void setTimerSeconds(int seconds);
    bool HasProtocol(const string&protocol);
    void LabelActive(const string&protocol);
    void Post(websocket::stream<beast::tcp_stream>*socket,const string&data);
    void Post(websocket::stream<beast::tcp_stream>*socket,AutoPtr<FixBuffer>buffer);
    void Broadcast(const string&data);
    void Broadcast(const string&protocol,const string&data);
    void Broadcast(AutoPtr<FixBuffer>buffer);
    void Broadcast(const string&protocol,AutoPtr<FixBuffer>buffer);
private:
    void do_listen(
            boost::asio::yield_context yield);
    void do_session(
         tcp::socket&socket,
         http::request<http::string_body> const& req,
         net::yield_context yield);
    void do_write(net::yield_context yield);
protected:
    void check_dead_line();
protected:
    net::ip::address address;
    unsigned short port;
    std::shared_ptr<std::string> doc_root;
    unsigned short threads;
    net::io_context ioc;
    ssl::context ctx;
    std::vector<std::thread> threadList;
    WebSocketServer*pServer;
    WebSocketPool socketPool;
    queue<AutoPtr<WebSocketSendData>>sendingDatas;
    net::steady_timer request_deadline_;
};

class websocket_worker
{
private:
    using alloc_t = fields_alloc<char>;
    using request_body_t = http::basic_dynamic_body<beast::flat_static_buffer<1024 * 1024>>;
    //using request_body_t = http::string_body;
    // The acceptor used to listen for incoming connections.
    AutoPtr<tcp::socket>socket_;
    AutoPtr<websocket::stream<beast::tcp_stream>>ws;
    // The buffer for performing reads
    beast::flat_static_buffer<8192> buffer_;
    // The allocator used for the fields in the request and reply.
    alloc_t alloc_{8192};
    // The parser for reading the requests
    boost::optional<http::request_parser<request_body_t, alloc_t>> parser_;
    // The timer putting a time limit on requests.
    beast::flat_buffer buffer;
    WebSocketServer*pServer;
    string protocol="";
    queue<AutoPtr<FixBuffer>>sendingBufers;
    queue<bool>binarys;
    bool isSending=false;
    AutoPtr<Semaphora>signal;
    AutoPtr<ResourceIndex>lock;
    DateTime lastTime;
private:
    void read_request()
    {
        parser_.emplace(
            std::piecewise_construct,
            std::make_tuple(),
            std::make_tuple(alloc_));
        size_t size=1024;
        size=size*10*1024;
        parser_.get().body_limit(size);
        http::async_read(
            *socket_,
            buffer_,
            *parser_,
            [this](beast::error_code ec, std::size_t)
            {
                if (!ec){
                    http::request<request_body_t, http::basic_fields<alloc_t>>const& req=parser_->get();
                    if(websocket::is_upgrade(req)){
                        protocol.assign(req.target().begin(), req.target().end());
                        if((protocol[0]='/')||(protocol[0]='\\')){
                            protocol.erase(begin(protocol));
                        }
                        if (protocol.empty()){
                            protocol="";
                            this->signal->SetSignal();
                            sendingBufers=queue<AutoPtr<FixBuffer>>();
                            binarys=queue<bool>();
                            this->lock=nullptr;
                            return;
                        }
                        ws=new websocket::stream<beast::tcp_stream>(std::move(*socket_));
                        ws->write_buffer_bytes(8192);
                        string protocol=this->protocol;
                        ws->set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
                        ws->set_option(websocket::stream_base::decorator(
                            [protocol](websocket::response_type& res)
                            {
                                res.set(http::field::server, protocol);
                            }));
                        web_socket_accept();
                    }
                }
            });
    }

    void web_socket_accept(){
        http::request<request_body_t, http::basic_fields<alloc_t>>const& req=parser_->get();
        ws->async_accept(
            req,
            [this](beast::error_code ec)
        {
            if (ec)
            {
                protocol="";
                this->signal->SetSignal();
                sendingBufers=queue<AutoPtr<FixBuffer>>();
                binarys=queue<bool>();
                this->lock=nullptr;
            }
            else
            {
                web_socket_read();
            }
        });
    }

    void web_socket_read(){
        ws->binary(true);
        ws->async_read(
           buffer,
           [this](beast::error_code ec, std::size_t){
            if (ec)
            {
                protocol="";
                this->signal->SetSignal();
                sendingBufers=queue<AutoPtr<FixBuffer>>();
                binarys=queue<bool>();
                this->lock=nullptr;
            }
            else{
                auto buf=buffer.data();
                char* p = static_cast<char*>(buf.data());
                AutoPtr<FixBuffer>fixb(new FixBuffer());
                fixb->Attach(p,buffer.size());
                signal->ResetSignal();
                AutoPtr<WebSocketResponseEventArgs>eventArgs(new WebSocketResponseEventArgs(protocol,ws.get(),fixb,signal));
                if(pServer->ResponseFunc!=nullptr){
                    WebSocketResponseEvent event(pServer,eventArgs);
                    pServer->ResponseFunc(event);
                }
                buffer.consume(buffer.size());
            }
        });
    };
    void web_socket_write(){
        if(sendingBufers.size()==0){
            isSending=false;
            return;
        }
        AutoPtr<FixBuffer>wsd=sendingBufers.front();
        bool binary=binarys.front();
        ws->binary(binary);
        sendingBufers.pop();
        binarys.pop();
        LabelActive();
        ws->async_write(
            boost::asio::buffer(wsd->GetBuffer(),wsd->GetSize()),
            [this](beast::error_code ec, std::size_t){
            if (!ec)
                web_socket_write();
            else{
                this->signal->SetSignal();
                sendingBufers=queue<AutoPtr<FixBuffer>>();
                binarys=queue<bool>();
                this->lock=nullptr;
            }
        });
    };
public:
    websocket_worker(websocket_worker const&) = delete;

    websocket_worker& operator=(websocket_worker const&) = delete;

    websocket_worker(){
        signal=new Semaphora(false,false);
    }
    void start(AutoPtr<ResourceIndex>lock,WebSocketServer*pServer,AutoPtr<tcp::socket>socket)
    {
        this->socket_=socket;
        this->pServer=pServer;
        this->lock=lock;
        lastTime=DateTime::Now();
        read_request();
    }
    websocket::stream<beast::tcp_stream>*getWebSocket(){
        return ws.get();
    };
    bool isValid(){
        return (this->lock!=nullptr);
    }
    string getProtocol(){
        return protocol;
    };
    void write(AutoPtr<FixBuffer>rsData,bool binary){
        if(protocol.empty()) return;
        if(ws==nullptr) return;
        sendingBufers.push(rsData);
        binarys.push(binary);
        LabelActive();
        if(!isSending){
           isSending=true;
           web_socket_write();
        }
    };
    void checkTimeOut(){
        if(lock==nullptr) return;
        if(((DateTime::Now()-lastTime).TotalSeconds()>=pServer->GetTimeoutSeconds())){
           beast::error_code ec;
           ws=nullptr;
           socket_->shutdown(tcp::socket::shutdown_send, ec);
           protocol="";
           this->signal->SetSignal();
           sendingBufers=queue<AutoPtr<FixBuffer>>();
           binarys=queue<bool>();
           this->lock=nullptr;
        }
    }
    void LabelActive(){
        if(lock==nullptr) return;
        lastTime=DateTime::Now();
    };

};

class websocket_workers{
public:
    websocket_workers()
    {

    };
    ~websocket_workers(){
        stopserver();
    };

    void runserver(WebSocketServer*pServer){
        this->pServer=pServer;
        this->serverIp=pServer->GetServerIp();
        this->port=pServer->GetServerPort();
        this->workDir=pServer->GetWorkingDirectory();
        this->num_threads=pServer->GetNumberWorkers();
        int num_workers=0;
        if(num_threads>0)
           num_workers=num_threads;
        else
           num_workers=HostHelper::GetNumberOfProcessors();
        int pool_size=num_workers*2;
        webworks.resize(pool_size);
        for(int k=0;k<pool_size;k++){
           webworks[k]=new websocket_worker();
        }
        workers_pool=new ResourcePool(pool_size);
        auto const address = net::ip::make_address(serverIp);
        acceptor=new  tcp::acceptor(io_context_, {address, port});
        //acceptor->set_option(net::socket_base::reuse_address(true));
        start_accept();
        for (int i = 0; i < num_workers; ++i) {
           threads.emplace_back([this] {
               while(1){
                   try{
                       io_context_.run();
                       break;
                   }
                   catch (const std::exception& e)
                   {

                   }
               }
           });
        }
        request_deadline_.expires_after(std::chrono::seconds(2));
        check_dead_line();
    }

    void stopserver(){
        request_deadline_.expires_after(std::chrono::seconds::max());
        io_context_.stop();
            // 等待所有线程结束
        for (auto& t : threads) {
           t.join();
        }
    };
    void check_dead_line(){
        if (request_deadline_.expiry() <= std::chrono::steady_clock::now())
        {
           int validNum=0;
           DateTime curTime;
           for(int k=webworks.size()-1;k>=0;k--){
                if(webworks[k]->isValid()) validNum++;
                webworks[k]->checkTimeOut();
           }
           if(validNum>0){
               if(pServer->TimerFunc!=nullptr){
                    WebSocketTimerEvent event(pServer);
                    pServer->TimerFunc(event);
               }
           }
           request_deadline_.expires_after(std::chrono::seconds(2));
        }
        request_deadline_.async_wait(
            [this](beast::error_code)
            {
                check_dead_line();
            });
    };
    int getTimeoutSeconds(){
        return pServer->GetTimeoutSeconds();
    };
    int getTimerSeconds(){
        return pServer->GetTimerSeconds();
    };
    bool HasProtocol(const string&protocol){
        for(int k=0;k<webworks.size();k++){
           if(!webworks[k]->isValid()) continue;
           if(webworks[k]->getProtocol()==protocol) return true;
        }
        return false;
    };
    void LabelActive(const string&protocol){
        for(int k=0;k<webworks.size();k++){
           webworks[k]->LabelActive();
        }
    };
    void Post(websocket::stream<beast::tcp_stream>*socket,const string&data){
        AutoPtr<FixBuffer>fxb=new FixBuffer(data.length());
        memcpy(fxb->GetBuffer(),data.data(),data.length());
        for(int k=webworks.size()-1;k>=0;k--){
           if(!webworks[k]->isValid()) continue;
           if(webworks[k]->getWebSocket()==socket) webworks[k]->write(fxb,false);
        }
    };
    void Post(websocket::stream<beast::tcp_stream>*socket,AutoPtr<FixBuffer>buffer){
        for(int k=webworks.size()-1;k>=0;k--){
           if(!webworks[k]->isValid()) continue;
           if(webworks[k]->getWebSocket()==socket) webworks[k]->write(buffer,true);
        }
    };

    void Broadcast(const string&data){
        AutoPtr<FixBuffer>fxb=new FixBuffer(data.length());
        memcpy(fxb->GetBuffer(),data.data(),data.length());
        for(int k=webworks.size()-1;k>=0;k--){
           if(!webworks[k]->isValid()) continue;
           webworks[k]->write(fxb,false);
        }
    };
    void Broadcast(const string&protocol,const string&data){
        AutoPtr<FixBuffer>fxb=new FixBuffer(data.length());
        memcpy(fxb->GetBuffer(),data.data(),data.length());
        for(int k=webworks.size()-1;k>=0;k--){
           if(!webworks[k]->isValid()) continue;
           if(webworks[k]->getProtocol()==protocol) webworks[k]->write(fxb,false);
        }
    };
    void Broadcast(AutoPtr<FixBuffer>buffer){
        for(int k=webworks.size()-1;k>=0;k--){
           if(!webworks[k]->isValid()) continue;
           webworks[k]->write(buffer,true);
        }
    };
    void Broadcast(const string&protocol,AutoPtr<FixBuffer>buffer){
        for(int k=webworks.size()-1;k>=0;k--){
           if(!webworks[k]->isValid()) continue;
           if(webworks[k]->getProtocol()==protocol) webworks[k]->write(buffer,true);
        }
    };
protected:
    void start_accept() {
        acceptor->async_accept(
            [this](boost::system::error_code ec, tcp::socket socket) {
                if (!ec) {
                    AutoPtr<ResourceIndex>rsi=workers_pool->LockResourceEx();
                    if(rsi!=nullptr){
                        AutoPtr<websocket_worker>worker=webworks[rsi->GetResourceId()];
                        AutoPtr<tcp::socket>t_socket=new tcp::socket(std::move(socket));
                        worker->start(rsi,pServer,t_socket);
                    }
                }
                start_accept(); // 继续接受新连
            });
    };
protected:
    string serverIp;
    USHORT port;
    string workDir;
    int num_threads;
    WebSocketServer*pServer;
    vector<thread>threads;
    net::io_context io_context_;
    AutoPtr<tcp::acceptor>acceptor;
    AutoPtr<ResourcePool>workers_pool;
    vector<AutoPtr<websocket_worker>>webworks;
    net::steady_timer request_deadline_{io_context_, (std::chrono::steady_clock::time_point::max)()};
};

};
