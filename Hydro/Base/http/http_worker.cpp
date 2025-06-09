#include "http_worker.h"
#include "../file.h"
#include "../ziputil.h"
#include "../StringHelper.h"
#include "../FilePath.h"

namespace SGIS{

SocketPool::SocketPool(net::io_context&ioc, AsyncHttpServer*pServer):
   request_deadline_(ioc, (std::chrono::steady_clock::time_point::max)()){
    this->pServer=pServer;
    request_deadline_.expires_after(std::chrono::seconds(pServer->getTimeoutSeconds()));
    check_dead_line();
}

SocketPool::~SocketPool(){

}

void SocketPool::AddSocket(tcp::socket*socket){
    mutex.Lock();
    for(int k=sels.size()-1;k>=0;k--){
        if(sels[k].socket==socket){
            sels[k].activeTime=DateTime();
            mutex.Unlock();
            return;
        }
    }
    sels.push_back(SocketEl(socket));
    mutex.Unlock();
}

void SocketPool::LabelActive(tcp::socket*socket){
    mutex.Lock();
    for(int k=sels.size()-1;k>=0;k--){
        if(sels[k].socket==socket){
            sels[k].activeTime=DateTime();
            sels[k].waiting=true;
            mutex.Unlock();
            return;
        }
    }
    mutex.Unlock();
}

void SocketPool::LabelWaitingFinished(tcp::socket*socket){
    mutex.Lock();
    for(int k=sels.size()-1;k>=0;k--){
        if(sels[k].socket==socket){
            sels[k].waiting=false;
            mutex.Unlock();
            return;
        }
    }
    mutex.Unlock();
}

void SocketPool::RemoveSocket(tcp::socket*socket){
    mutex.Lock();
    for(int k=sels.size()-1;k>=0;k--){
        if(sels[k].socket==socket){
            if(sels[k].waiting) continue;
            sels.erase(begin(sels)+k);
            mutex.Unlock();
            return;
        }
    }
    mutex.Unlock();
}

void SocketPool::check_dead_line(){
    if (request_deadline_.expiry() <= std::chrono::steady_clock::now())
    {
        DateTime curTime;
        mutex.Lock();
        for(int k=sels.size()-1;k>=0;k--){
            if(sels[k].waiting) continue;
            if((curTime-sels[k].activeTime).TotalSeconds()>=pServer->getTimeoutSeconds()){
                try {
                    boost::system::error_code ec;
                    sels[k].socket->shutdown(tcp::socket::shutdown_both, ec);
                } catch (exception e) {

                }
                sels.erase(begin(sels)+k);
            }
        }
        mutex.Unlock();
        request_deadline_.expires_after(std::chrono::seconds(pServer->getTimeoutSeconds()));
    }
    request_deadline_.async_wait(
         [this](beast::error_code)
    {
        check_dead_line();
    });
}

AsyncHttpServer::AsyncHttpServer(HttpServer*pServer,int threadNum,std::string certChainFile,string privateKeyFile,string verifyfile,string password):
    address(net::ip::make_address(pServer->GetServerIp())),
    port(pServer->GetServerPort()),
    doc_root(std::make_shared<std::string>(pServer->GetWorkingDirectory())),
    threads(threadNum),
    ioc{threads},
    ctx{ssl::context::tlsv12_server},
    socketPool(ioc,this){
    https=true;
    this->pServer=pServer;
    this->mimeType=pServer->GetMimeType();
    this->pathMap=pServer->GetPathMap();
    this->certChainFile = certChainFile;
    this->privateKeyFile = privateKeyFile;
    this->verifyfile=verifyfile;
    this->password=password;
    load_server_certificate();
    threadList.reserve(threads);
}

AsyncHttpServer::AsyncHttpServer(HttpServer*pServer,int threadNum):
    address(net::ip::make_address(pServer->GetServerIp())),
    port(pServer->GetServerPort()),
    doc_root(std::make_shared<std::string>(pServer->GetWorkingDirectory())),
    pServer(pServer),
    threads(threadNum),
    ioc{threads},
    ctx{ssl::context::tlsv12_server},
    socketPool(ioc,this){
    https=false;
    this->mimeType=pServer->GetMimeType();
    this->pathMap=pServer->GetPathMap();
    this->certChainFile = "";
    this->privateKeyFile = "";
    this->verifyfile="";
    this->password="";
    threadList.reserve(threads);
}

AsyncHttpServer::~AsyncHttpServer() {
    stop();
}

bool AsyncHttpServer::on_verify_certificate (bool preverified, boost::asio::ssl::verify_context & ctx)
{
    X509_STORE_CTX * cts = ctx.native_handle ();
    auto error (X509_STORE_CTX_get_error (cts));
    switch (error)
    {
        case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
            //"TLS: Unable to get issuer";
            break;
        case X509_V_ERR_CERT_NOT_YET_VALID:
        case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
            //"TLS: Certificate not yet valid";
            break;
        case X509_V_ERR_CERT_HAS_EXPIRED:
        case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
            //"TLS: Certificate expired";
            break;
        case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
            preverified = true;
            break;
        case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
            //"TLS: Self signed certificate not in the list of trusted certs (forgot to subject-hash certificate filename?)");
            break;
        default:
            break;
    }
    return preverified;
}

bool AsyncHttpServer::load_server_certificate()
{
    ctx.set_options(
                boost::asio::ssl::context::default_workarounds| boost::asio::ssl::context::no_sslv2 | boost::asio::ssl::context::no_sslv3 | boost::asio::ssl::context::single_dh_use);
    //SSL_CTX_set_cipher_list(ctx.native_handle(), tls12CipherSuite.c_str());

    boost::system::error_code ecCert;
    ctx.use_certificate_chain_file(certChainFile, ecCert);

    if (ecCert) return false;
    boost::system::error_code ecKey;
    ctx.use_private_key_file(
                privateKeyFile,
                boost::asio::ssl::context::pem,
                ecKey);
    if (ecKey) return false;
    ctx.set_password_callback (
        [this](std::size_t,
        boost::asio::ssl::context_base::password_purpose) {
            return password;
     });
    if(verifyfile=="")
        ctx.set_verify_mode(ssl::verify_none);
    else{
        ctx.set_verify_mode(ssl::verify_peer);
        ctx.load_verify_file(verifyfile);
        ctx.set_verify_callback ([this](auto preverified, auto & ctx) {
             return this->on_verify_certificate(preverified, ctx);
        });
    }
    return true;
}

void AsyncHttpServer::start()
{
    boost::asio::spawn(
                ioc.get_executor(),
                std::bind(
                    &AsyncHttpServer::do_listen,
                    this,
                    std::placeholders::_1));

    // Run the I/O service on the requested number of threads
    for (auto i = threads; i > 0; --i)
        threadList.emplace_back([this]
        {
            while(1){
                try{
                    ioc.run();
                    break;
                }
                catch(exception){

                }
            }
        }
     );
}

void AsyncHttpServer::stop()
{
    ioc.stop();
    for (auto &item : threadList)
    {
        item.join();
    }
}

int AsyncHttpServer::getTimeoutSeconds(){
    return pServer->GetTimeoutSeconds();
}

void AsyncHttpServer::do_listen(
        boost::asio::yield_context yield)
{
    boost::system::error_code ec;

    // Open the acceptor
    tcp::acceptor acceptor(ioc);
    auto endpoint = tcp::endpoint{address, port};
    acceptor.open(endpoint.protocol(), ec);
    if (ec) return;

    // Allow address reuse
    acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec) return;

    // Bind to the server address
    acceptor.bind(endpoint, ec);
    if (ec) return;

    // Start listening for connections
    acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) return;
    for (;;)
    {
        tcp::socket socket(ioc);

        //net::steady_timer request_deadline_{
                //acceptor.get_executor(), (std::chrono::steady_clock::time_point::max)()};

        acceptor.async_accept(socket, yield[ec]);
        if (!ec){
            boost::asio::spawn(
                        acceptor.get_executor(),
                        std::bind(
                            https?&AsyncHttpServer::do_session:&AsyncHttpServer::do_session_http,
                            this,
                            std::move(socket),
                            std::placeholders::_1));
        }
    }
}

void AsyncHttpServer::do_session_http(
        tcp::socket &socket,
        boost::asio::yield_context yield)
{
    bool close = false;
    boost::system::error_code ec;
    boost::beast::flat_buffer buffer;
    socketPool.AddSocket(&socket);
    send_lambda<tcp::socket&> lambda{socket, close, ec, yield};
    for (;;)
    {
        http::request_parser<http::string_body> req;
        size_t size=1024;
        size=size*10*1024;
        req.body_limit(size);
        http::async_read(socket, buffer, req, yield[ec]);
        if (ec == http::error::end_of_stream)
            break;
        if (ec){
            socketPool.RemoveSocket(&socket);
            return;
        }
        socketPool.LabelActive(&socket);
        string sClientIp = socket.remote_endpoint().address().to_string();
        handle_request(std::move(req.get()), lambda,sClientIp);
        socketPool.LabelWaitingFinished(&socket);
        if (ec){
            socketPool.RemoveSocket(&socket);
            return;
        }
        if (close)
        {
            break;
        }

    }
    socketPool.RemoveSocket(&socket);
    socket.shutdown(tcp::socket::shutdown_both, ec);
}

void AsyncHttpServer::do_session(
        tcp::socket &socket,
        boost::asio::yield_context yield)
{
    bool close = false;
    boost::system::error_code ec;
    ssl::stream<tcp::socket &> stream{socket, ctx};
    stream.async_handshake(ssl::stream_base::server, yield[ec]);
    if (ec) return;
    boost::beast::flat_buffer buffer;
    socketPool.AddSocket(&socket);
    send_lambda<ssl::stream<tcp::socket &>> lambda{stream, close, ec, yield};
    http::request<http::string_body> req;
    for (;;)
    {
        // Read a request
        //http::request<http::string_body> req;
        http::request_parser<http::string_body> req;
        size_t size=1024;
        size=size*10*1024;
        req.body_limit(size);
        http::async_read(stream, buffer, req, yield[ec]);
        if (ec == http::error::end_of_stream)
            break;
        if (ec){
             socketPool.RemoveSocket(&socket);
             return;
        }
        socketPool.LabelActive(&socket);
        string sClientIp = socket.remote_endpoint().address().to_string();
        handle_request(std::move(req.get()), lambda,sClientIp);
        socketPool.LabelWaitingFinished(&socket);
        if (ec){
            socketPool.RemoveSocket(&socket);
            return;
        }
        if (close)
        {
            break;
        }
    }
    socketPool.RemoveSocket(&socket);
    stream.async_shutdown(yield[ec]);
}


template <class Body, class Allocator, class Send>
void AsyncHttpServer::SendFile(const string&path,http::request<Body, http::basic_fields<Allocator>> &req,
                               Send &&send){
    // Returns a not found response

    auto const not_found = [&req](boost::beast::string_view target)
    {
        /*
        bool hasToken=false;
        for(auto citer=req.begin();citer!=req.end();citer++)
        {
            string name=citer->name_string().to_string();
            StringHelper::TrimLower(name);
            if(name=="token"){
               hasToken=true;
               break;
            }
        }
        */
        http::response<http::string_body> res{http::status::not_found, req.version()};
        res.set(http::field::server, "SGIS");
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.set(http::field::access_control_allow_origin,"*");
        //if(hasToken)
            res.set(http::field::access_control_allow_headers,"token,content-type");
        //else
            //res.set(http::field::access_control_allow_headers,"content-type");
        res.set(http::field::access_control_allow_credentials,"true");
            res.body() = "The resource '" + string(target.data()) + "' was not found.";
        res.prepare_payload();
        return res;
    };

    // Returns a server error response
    auto const server_error = [&req](boost::beast::string_view what)
    {
        /*
        bool hasToken=false;
        for(auto citer=req.begin();citer!=req.end();citer++)
        {
            string name=citer->name_string().to_string();
            StringHelper::TrimLower(name);
            if(name=="token"){
               hasToken=true;
               break;
            }
        }
        */
        http::response<http::string_body> res{http::status::internal_server_error, req.version()};
        res.set(http::field::server, "SGIS");
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.set(http::field::access_control_allow_origin,"*");
        //if(hasToken)
            res.set(http::field::access_control_allow_headers,"token,content-type");
        //else
            //res.set(http::field::access_control_allow_headers,"content-type");
        res.set(http::field::access_control_allow_credentials,"true");
        res.body() = "An error occurred: '" +string(what.data()) + "'";
        res.prepare_payload();
        return res;
    };
    /*
    bool hasToken=false;
    for(auto citer=req.begin();citer!=req.end();citer++)
    {
        string name=citer->name_string().to_string();
        StringHelper::TrimLower(name);
        if(name=="token"){
           hasToken=true;
           break;
        }
    }
    */

    string fileExa=FilePath::GetFileExa(path);
    boost::beast::error_code ec;
    if(this->pServer->IsFileTypeInZipList(fileExa))
    {
        string zipFile=FilePath::GetDir(path)+"/"+FilePath::GetFileName(path)+".gz";
        if(FilePath::IsFileExisting(zipFile)){
            http::file_body::value_type body;
            body.open(zipFile.c_str(), boost::beast::file_mode::scan, ec);
            auto const size = body.size();
            if (req.method() == http::verb::head)
            {
                http::response<http::empty_body> res{http::status::ok, req.version()};
                res.set(http::field::server, pServer->ServerName());
                res.set(http::field::content_type, mimeType->GetString(std::string(path)));
                res.content_length(size);
                res.set(http::field::access_control_allow_origin,"*");
                //if(hasToken)
                    res.set(http::field::access_control_allow_headers,"token,content-type");
                //else
                    //res.set(http::field::access_control_allow_headers,"content-type");
                res.set(http::field::access_control_allow_credentials,"true");
                DateTime curTime;
                res.set(http::field::last_modified,curTime.GetHttpTime());
                res.keep_alive(req.keep_alive());
                return send(std::move(res));
            }
            http::response<http::file_body>
                    res{std::piecewise_construct,
                        std::make_tuple(std::move(body)),
                        std::make_tuple(http::status::ok, req.version())};
            res.set(http::field::server, pServer->ServerName());
            res.set(http::field::content_type, mimeType->GetString(std::string(path)));
            res.set(http::field::access_control_allow_origin,"*");
            //if(hasToken)
                res.set(http::field::access_control_allow_headers,"token,content-type");
            //else
                //res.set(http::field::access_control_allow_headers,"content-type");
            res.set(http::field::access_control_allow_credentials,"true");
            res.set(http::field::content_encoding,"gzip");
            DateTime curTime;
            res.set(http::field::last_modified,curTime.GetHttpTime());
            res.content_length(size);
            res.keep_alive(req.keep_alive());
            return send(std::move(res));
        }
        else{
            vector<BYTE>fxb=GZip::CompressFileToBuffer(path);
            if (req.method() == http::verb::head)
            {
                http::response<http::empty_body> res{http::status::ok, req.version()};
                res.set(http::field::server, pServer->ServerName());
                res.set(http::field::content_type, mimeType->GetString(std::string(path)));
                res.content_length(fxb.size());
                res.set(http::field::access_control_allow_origin,"*");
                //if(hasToken)
                    res.set(http::field::access_control_allow_headers,"token,content-type");
                //else
                    //res.set(http::field::access_control_allow_headers,"content-type");
                res.set(http::field::access_control_allow_credentials,"true");
                DateTime curTime;
                res.set(http::field::last_modified,curTime.GetHttpTime());
                res.keep_alive(req.keep_alive());
                return send(std::move(res));
            }
            http::response<http::buffer_body> res{http::status::ok, req.version()};
            res.keep_alive(req.keep_alive());
            res.set(http::field::server, pServer->ServerName());
            res.set(http::field::content_type, mimeType->GetString(std::string(path)));
            res.set(http::field::access_control_allow_origin,"*");
            //if(hasToken)
                res.set(http::field::access_control_allow_headers,"token,content-type");
            //else
                //res.set(http::field::access_control_allow_headers,"content-type");
            res.set(http::field::access_control_allow_credentials,"true");
            res.set(http::field::content_encoding,"gzip");
            DateTime curTime;
            string httpTime=curTime.GetHttpTime();
            res.set(http::field::last_modified,httpTime);
            http::buffer_body::value_type vt;
            vt.more=false;
            vt.data=(char*)fxb.data();
            vt.size=fxb.size();
            res.body()=std::move(vt);
            res.prepare_payload();
            AutoPtr<FixBuffer>fxbb(new FixBuffer());
            fxbb->Attach((char*)fxb.data(),fxb.size());
            BinaryFile::Write(zipFile,fxbb.get());
            return send(std::move(res));
        }
    }
    else{
        http::file_body::value_type body;
        string cpath=path;
        body.open(cpath.c_str(), boost::beast::file_mode::scan, ec);
        if (ec == boost::system::errc::no_such_file_or_directory){
            return send(not_found(req.target()));
        }
        if (ec)
            return send(server_error(ec.message()));
        auto const size = body.size();
        if (req.method() == http::verb::head)
        {
            http::response<http::empty_body> res{http::status::ok, req.version()};
            res.set(http::field::server, pServer->ServerName());
            res.set(http::field::content_type, mimeType->GetString(std::string(path)));
            res.content_length(size);
            res.set(http::field::access_control_allow_origin,"*");
            //if(hasToken)
                res.set(http::field::access_control_allow_headers,"token,content-type");
            //else
                //res.set(http::field::access_control_allow_headers,"content-type");
            res.set(http::field::access_control_allow_credentials,"true");
            DateTime curTime;
            res.set(http::field::last_modified,curTime.GetHttpTime());
            res.keep_alive(req.keep_alive());
            return send(std::move(res));
        }
        http::response<http::file_body>
                res{std::piecewise_construct,
                    std::make_tuple(std::move(body)),
                    std::make_tuple(http::status::ok, req.version())};
        res.set(http::field::server, pServer->ServerName());
        res.set(http::field::content_type, mimeType->GetString(std::string(path)));
        res.set(http::field::access_control_allow_origin,"*");
        //if(hasToken)
            res.set(http::field::access_control_allow_headers,"token,content-type");
        //else
            //res.set(http::field::access_control_allow_headers,"content-type");
        res.set(http::field::access_control_allow_credentials,"true");
        DateTime curTime;
        res.set(http::field::last_modified,curTime.GetHttpTime());
        res.content_length(size);
        res.keep_alive(req.keep_alive());
        return send(std::move(res));
    }
}

template <class Body, class Allocator, class Send>
void AsyncHttpServer::SendStringData(AutoPtr<ResponseData>rsData,http::request<Body, http::basic_fields<Allocator>> &req,
                    Send &&send){
    /*
    bool hasToken=false;
    for(auto citer=req.begin();citer!=req.end();citer++){
        string name=citer->name_string().to_string();
        StringHelper::TrimLower(name);
        if(name=="token"){
           hasToken=true;
           break;
        }
    }
    */
    string mimeType=rsData->MimeType();
    http::response<http::string_body> res{http::status::ok, req.version()};
    res.keep_alive(req.keep_alive());
    res.set(http::field::server, pServer->ServerName());
    res.set(http::field::content_type, mimeType);
    res.set(http::field::access_control_allow_origin,"*");
    //if(hasToken)
        res.set(http::field::access_control_allow_headers,"token,content-type");
    //else
        //res.set(http::field::access_control_allow_headers,"content-type");
    res.set(http::field::access_control_allow_credentials,"true");
    DateTime curTime;
    string httpTime=curTime.GetHttpTime();
    res.set(http::field::last_modified,httpTime);
    res.body()=rsData->GetStringData();
    res.prepare_payload();
    return send(std::move(res));
}

template <class Body, class Allocator, class Send>
void AsyncHttpServer::SendBufferData(AutoPtr<ResponseData>rsData,http::request<Body, http::basic_fields<Allocator>> &req,
                    Send &&send){
    /*
    bool hasToken=false;
    for(auto citer=req.begin();citer!=req.end();citer++)
    {
        string name=citer->name_string().to_string();
        StringHelper::TrimLower(name);
        if(name=="token"){
           hasToken=true;
           break;
        }
    }
    */
    http::response<http::buffer_body> res{http::status::ok, req.version()};
    string mimeType=rsData->MimeType();
    if(rsData->GetResponseDataType()==ResponseDataType::rdtCompressed){
        AutoPtr<FixBuffer>fxb=rsData->GetBufferData();
        http::response<http::buffer_body> res{http::status::ok, req.version()};
        res.keep_alive(req.keep_alive());
        res.set(http::field::server, pServer->ServerName());
        res.set(http::field::content_type, mimeType);
        res.set(http::field::access_control_allow_origin,"*");
        //if(hasToken)
            res.set(http::field::access_control_allow_headers,"token,content-type");
        //else
            //res.set(http::field::access_control_allow_headers,"content-type");
        res.set(http::field::access_control_allow_credentials,"true");
        res.set(http::field::content_encoding,"gzip");
        DateTime curTime;
        string httpTime=curTime.GetHttpTime();
        res.set(http::field::last_modified,httpTime);
        http::buffer_body::value_type vt;
        vt.more=false;
        //vector<char>values(fxb->GetSize());
        //memcpy(values.data(),fxb->GetBuffer(),fxb->GetSize());
        //vt.data=values.data();
        //vt.size=values.size();
        vt.data=(char*)fxb->GetBuffer();
        vt.size=fxb->GetSize();
        res.body()=std::move(vt);
        res.prepare_payload();
        send(std::move(res));
    }else{
        res.keep_alive(req.keep_alive());
        res.set(http::field::server, pServer->ServerName());
        res.set(http::field::content_type, mimeType);
        res.set(http::field::access_control_allow_origin,"*");
        //if(hasToken)
            res.set(http::field::access_control_allow_headers,"token,content-type");
        //else
            //res.set(http::field::access_control_allow_headers,"content-type");
        res.set(http::field::access_control_allow_credentials,"true");
        DateTime curTime;
        string httpTime=curTime.GetHttpTime();
        res.set(http::field::last_modified,httpTime);
        AutoPtr<FixBuffer>fb=rsData->GetBufferData();
        http::buffer_body::value_type vt;
        vt.more=false;
        vt.data=(char*)fb->GetBuffer();
        vt.size=fb->GetSize();
        res.body()=std::move(vt);
        res.prepare_payload();
        send(std::move(res));
    }
}

template <class Body, class Allocator, class Send>
void AsyncHttpServer::handle_request(
        http::request<Body, http::basic_fields<Allocator>> &&req,
        Send &&send,const string&remoteServerIp)
{

    // Returns a bad request response
    auto const bad_request = [&req](boost::beast::string_view why)
    {
        /*
        bool hasToken=false;
        for(auto citer=req.begin();citer!=req.end();citer++)
        {
            string name=citer->name_string().to_string();
            StringHelper::TrimLower(name);
            if(name=="token"){
               hasToken=true;
               break;
            }
        }
        */
        http::response<http::string_body> res{http::status::bad_request, req.version()};
        res.set(http::field::server, "SGIS");
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.set(http::field::access_control_allow_origin,"*");
        //if(hasToken)
            res.set(http::field::access_control_allow_headers,"token,content-type");
        //else
            //res.set(http::field::access_control_allow_headers,"content-type");
        res.set(http::field::access_control_allow_credentials,"true");
        res.body() = string(why.data());
        res.prepare_payload();
        return res;
    };

    auto const send_options=[&req](){
        /*
        bool hasToken=false;
        for(auto citer=req.begin();citer!=req.end();citer++)
        {
            string name=citer->name_string().to_string();
            StringHelper::TrimLower(name);
            if(name=="token"){
               hasToken=true;
               break;
            }
        }
        */
        http::response<http::string_body> res{http::status::ok, req.version()};
        res.set(http::field::server, "SGIS");
        res.set(http::field::content_type, "text/html");
        res.keep_alive(req.keep_alive());
        res.set(http::field::access_control_allow_origin,"*");
        //if(hasToken)
            res.set(http::field::access_control_allow_headers,"token,content-type");
        //else
            //res.set(http::field::access_control_allow_headers,"content-type");
        res.set(http::field::access_control_allow_credentials,"true");
        res.prepare_payload();
        return res;
    };
    // Make sure we can handle the method
    if(req.method()==http::verb::options){
        return send(send_options());
    }
    if (req.method() != http::verb::get && req.method() != http::verb::head && req.method() != http::verb::post)
        return send(bad_request("Unknown HTTP-method"));

    // Request path must be absolute and not contain "..".
    if (req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos)
        return send(bad_request("Illegal request-target"));

    // Build the path to the requested file
    string target="";
    target.resize(req.target().size());
    memcpy(target.data(),req.target().data(),req.target().size());

    std::string path = path_cat(*doc_root, target);
    path=StringHelper::UrlDecode(path);
    path=FilePath::ConvertToStardardPath(path);

    string pageHtml=pServer->GetWorkingDirectory()+"/web";
    pageHtml=FilePath::ConvertToStardardPath(pageHtml);
    StringHelper::MakeLower(pageHtml);
    string tempPath=path;
    StringHelper::MakeLower(tempPath);
    if(tempPath==pageHtml){
        if(FilePath::IsFileExisting(pageHtml+"/index.html")){
            return SendFile(pageHtml+"/index.html",req,send);
        }
    }
    int lpos=path.find('?');
    if(lpos>0){
        path=path.substr(0,lpos);
    }
    if(!FilePath::IsFileExisting(path)){
        string sTarget=path;
        sTarget=pathMap->MapPath(sTarget);
        path=sTarget;
    }

    boost::beast::error_code ec;
    if(FilePath::IsFileExisting(path)){
        return SendFile(path,req,send);
    }
    AutoPtr<FixBuffer>buffer=nullptr;
    if(req.method()==http::verb::post){
        buffer=new FixBuffer();
        buffer->Attach(req.body().data(),req.body().size());
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
    AutoPtr<HttpResponseEventArgs>eventArgs(new HttpResponseEventArgs(rqData));
    if(pServer->ResponseFunc!=nullptr){
        HTTPResponseEvent event(pServer,eventArgs);
        pServer->ResponseFunc(event);
    }
    if(eventArgs->HasHandled()){
        AutoPtr<ResponseData>rsData=eventArgs->GetResponseData();
        if(!rsData->IsBufferData()){
            if(rsData->IsFile()){
                std::string full_path =rsData->GetStringData();
                SendFile(full_path,req,send);
                return;
            }
            else{
                try {
                   SendStringData(rsData,req,send);
                }
                catch (exception e) {

                }
                return;
            }
        }
        else{
            try {
                SendBufferData(rsData,req,send);
            } catch (exception e) {

            }
            return;
        }
        return;
    }
    else
        return send(bad_request("Illegal request"));
}

std::string AsyncHttpServer::path_cat(
        boost::beast::string_view base, boost::beast::string_view path)
{
    if (base.empty())
        return string(path.data());
    std::string result = string(base.data());
    char constexpr path_separator = '/';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    return result;
}


WebSocketPool::WebSocketPool(net::io_context&ioc, AsyncWebSocketServer*pServer){
    this->pServer=pServer;
}

WebSocketPool::~WebSocketPool(){

}

void WebSocketPool::AddSocket(websocket::stream<beast::tcp_stream>*socket,const string&protocol){
    mutex.Lock();
    for(int k=conns.size()-1;k>=0;k--){
        if(conns[k]->socket==socket){
            conns[k]->UpdateActiveTime();
            mutex.Unlock();
            return;
        }
    }
    string pt=protocol;
    StringHelper::TrimLower(pt);
    conns.push_back(new WebSocketConnect(socket,pt));
    mutex.Unlock();
}

void WebSocketPool::LabelActive(websocket::stream<beast::tcp_stream>*socket){
    mutex.Lock();
    for(int k=conns.size()-1;k>=0;k--){
        if(conns[k]->socket==socket){
            conns[k]->UpdateActiveTime();
            conns[k]->recv_waiting=true;
            mutex.Unlock();
            return;
        }
    }
    mutex.Unlock();
}


void WebSocketPool::LabelWaitingFinished(websocket::stream<beast::tcp_stream>*socket){
    mutex.Lock();
    for(int k=conns.size()-1;k>=0;k--){
        if(conns[k]->socket==socket){
            conns[k]->recv_waiting=false;
            mutex.Unlock();
            return;
        }
    }
    mutex.Unlock();
}

void WebSocketPool::RemoveSocket(websocket::stream<beast::tcp_stream>*socket){
    mutex.Lock();
    for(int k=conns.size()-1;k>=0;k--){
        if(conns[k]->socket==socket){
            conns.erase(begin(conns)+k);
            mutex.Unlock();
            return;
        }
    }
    mutex.Unlock();
}

bool WebSocketPool::HasProtocol(const string&protocol){
    mutex.Lock();
    string pt=protocol;
    StringHelper::TrimLower(pt);
    for(int k=conns.size()-1;k>=0;k--){
        if(conns[k]->protocol==pt){
            mutex.Unlock();
            return true;
        }
    }
    mutex.Unlock();
    return false;
}


void WebSocketPool::LabelActive(const string&protocol){
    mutex.Lock();
    string pt=protocol;
    StringHelper::TrimLower(pt);
    for(int k=conns.size()-1;k>=0;k--){
        if(conns[k]->protocol!=pt) continue;
        conns[k]->UpdateActiveTime();
        conns[k]->recv_waiting=true;
    }
    mutex.Unlock();
}

bool WebSocketPool::Send(websocket::stream<beast::tcp_stream>*socket,AutoPtr<FixBuffer>rsData,bool binary,net::yield_context&yield){
    mutex.Lock();
    boost::system::error_code ec;
    for(int k=conns.size()-1;k>=0;k--){
         if(conns[k]->socket==socket){
             conns[k]->socket->binary(binary);
             conns[k]->send_waiting=true;
             conns[k]->socket->async_write(boost::asio::buffer(rsData->GetBuffer(),rsData->GetSize()),yield[ec]);
             if(ec){
                 try {
                    conns[k]->socket->next_layer().close();
                 } catch (exception e) {

                 }
                 conns.erase(begin(conns)+k);
             }
             else{
                 conns[k]->UpdateActiveTime();
                 conns[k]->send_waiting=false;
             }
             mutex.Unlock();
             return true;
         }
    }
    mutex.Unlock();
    return false;
}

void WebSocketPool::Broadcast(AutoPtr<FixBuffer>rsData,bool binary,net::yield_context&yield){
    mutex.Lock();
    boost::system::error_code ec;
    for(int k=conns.size()-1;k>=0;k--){
        conns[k]->socket->binary(binary);
        conns[k]->send_waiting=true;
        conns[k]->socket->async_write(boost::asio::buffer(rsData->GetBuffer(),rsData->GetSize()),yield[ec]);
        if(ec){
            try {
               conns[k]->socket->next_layer().close();
            } catch (exception e) {

            }
            conns.erase(begin(conns)+k);
        }
        else
            conns[k]->UpdateActiveTime();
        conns[k]->send_waiting=false;
    }
    mutex.Unlock();
}

void WebSocketPool::Broadcast(const string&protocol,AutoPtr<FixBuffer>rsData,bool binary,net::yield_context&yield){
    mutex.Lock();
    if(conns.size()==0){
        mutex.Unlock();
        return;
    }
    string pt=protocol;
    StringHelper::TrimLower(pt);
    boost::system::error_code ec;
    for(int k=conns.size()-1;k>=0;k--){
        if(conns[k]->protocol!=pt) continue;
        conns[k]->socket->binary(binary);
        conns[k]->send_waiting=true;
        conns[k]->socket->async_write(boost::asio::buffer(rsData->GetBuffer(),rsData->GetSize()),yield[ec]);
        if(ec){
            try {
               conns[k]->socket->next_layer().close();
            } catch (exception e) {

            }
            conns.erase(begin(conns)+k);
        }
        else
            conns[k]->UpdateActiveTime();
        conns[k]->send_waiting=false;
    }
    mutex.Unlock();
}

void WebSocketPool::check_dead_line(){
    DateTime curTime;
    mutex.Lock();
    for(int k=conns.size()-1;k>=0;k--){
        if((conns[k]->send_waiting)||(conns[k]->recv_waiting)) continue;
        if((curTime-conns[k]->activeTime).TotalSeconds()>=pServer->getTimeoutSeconds()){
            try {
                boost::system::error_code ec;
                conns[k]->socket->next_layer().close();
            } catch (exception e) {

            }
            conns.erase(begin(conns)+k);
        }
    }
    mutex.Unlock();
}

AsyncWebSocketServer::AsyncWebSocketServer(WebSocketServer*pServer,int threadNum):
    address(net::ip::make_address(pServer->GetServerIp())),
    port(pServer->GetServerPort()),
    doc_root(std::make_shared<std::string>(pServer->GetWorkingDirectory())),
    threads(threadNum),
    ioc{threads},
    ctx{ssl::context::tlsv12_server},
    socketPool(ioc,this),
    request_deadline_(ioc, (std::chrono::steady_clock::time_point::max)()){
    //https=false;
    this->pServer=pServer;
    threadList.reserve(threads);
    request_deadline_.expires_after(std::chrono::seconds(pServer->GetTimerSeconds()));
    check_dead_line();
}

AsyncWebSocketServer::~AsyncWebSocketServer() {
   stop();
}

void AsyncWebSocketServer::start()
{
    boost::asio::spawn(
                ioc.get_executor(),
                std::bind(
                    &AsyncWebSocketServer::do_listen,
                    this,
                    std::placeholders::_1));
    // Run the I/O service on the requested number of threads
    for (auto i = threads; i > 0; --i)
        threadList.emplace_back([this]
        {
            while(1){
                try{
                    ioc.run();
                    break;
                }
                catch(exception){

                }
            }
        });
}

void AsyncWebSocketServer::stop()
{
    ioc.stop();
    for (auto &item : threadList)
    {
        item.join();
    }
}

void AsyncWebSocketServer::check_dead_line(){
    if (request_deadline_.expiry() <= std::chrono::steady_clock::now())
    {
        //this->socketPool.check_dead_line();
        if(pServer->TimerFunc!=nullptr){
            WebSocketTimerEvent event(pServer);
            pServer->TimerFunc(event);
        }
        request_deadline_.expires_after(std::chrono::seconds(pServer->GetTimerSeconds()));
    }
    request_deadline_.async_wait(
         [this](beast::error_code)
    {
        check_dead_line();
    });
}

int AsyncWebSocketServer::getTimeoutSeconds(){
    return pServer->GetTimeoutSeconds();
}

int AsyncWebSocketServer::getTimerSeconds(){
    return pServer->GetTimerSeconds();
}

void AsyncWebSocketServer::do_listen(
        boost::asio::yield_context yield)
{
    boost::system::error_code ec;
    tcp::acceptor acceptor(ioc);
    auto endpoint = tcp::endpoint{address, port};
    acceptor.open(endpoint.protocol(), ec);
    if (ec) return;
    acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
    if (ec) return;
    acceptor.bind(endpoint, ec);
    if (ec) return;
    acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
    if (ec) return;
    for (;;)
    {
        tcp::socket socket(ioc);
        acceptor.async_accept(socket, yield[ec]);
        if (!ec){
            beast::flat_buffer buffer;
            http::request<http::string_body> req;
            http::async_read(socket, buffer, req, yield[ec]);
            if(websocket::is_upgrade(req))
            {
                boost::asio::spawn(
                   acceptor.get_executor(),
                   std::bind(&AsyncWebSocketServer::do_session,
                             this,
                             std::ref(socket),
                             std::ref(req),
                             std::placeholders::_1));
            }
        }
    }
}

void AsyncWebSocketServer::do_session(
        tcp::socket&socket,
        http::request<http::string_body> const&req,
        net::yield_context yield)
{
    websocket::stream<beast::tcp_stream>ws=websocket::stream<beast::tcp_stream>(std::move(socket));
    ws.write_buffer_bytes(8192);
    beast::error_code ec;
    std::string protocol;
    protocol.assign(req.target().begin(), req.target().end());
    if((protocol[0]='/')||(protocol[0]='\\')){
        protocol.erase(begin(protocol));
    }
    if (protocol.empty()) return;
    ws.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));
    ws.set_option(websocket::stream_base::decorator(
        [protocol](websocket::response_type& res)
    {
        res.set(http::field::server, protocol);
    }));
    ws.async_accept(req, yield[ec]);
    if(ec) return;
    socketPool.AddSocket(&ws,protocol);
    beast::flat_buffer buffer;
    for(;;)
    {
        ws.binary(true);
        ws.async_read(buffer, yield[ec]);
        if (ec == http::error::end_of_stream)
            break;
        if (ec){
            socketPool.RemoveSocket(&ws);
            return;
        }
        socketPool.LabelActive(&ws);
        auto buf=buffer.data();
        char* p = static_cast<char*>(buf.data());
        AutoPtr<FixBuffer>fixb(new FixBuffer());
        fixb->Attach(p,buffer.size());
        //AutoPtr<FixBuffer>fixb(new FixBuffer(buffer.size()));
        //memcpy(fixb->GetBuffer(),p,buffer.size());
        AutoPtr<WebSocketResponseEventArgs>eventArgs(new WebSocketResponseEventArgs(protocol,&ws,fixb));
        if(pServer->ResponseFunc!=nullptr){
            WebSocketResponseEvent event(pServer,eventArgs);
            pServer->ResponseFunc(event);
        }
        buffer.consume(buffer.size());
        socketPool.LabelWaitingFinished(&ws);
    }
    socketPool.RemoveSocket(&ws);
    ws.next_layer().close();
}

void AsyncWebSocketServer::do_write(
        net::yield_context yield){
    for(;;){
        if(sendingDatas.size()==0){
            return;
        }
        AutoPtr<WebSocketSendData>wsd=sendingDatas.front();
        AutoPtr<FixBuffer>rsData=wsd->data;
        sendingDatas.pop();
        if(wsd->socket!=nullptr){
            socketPool.Send(wsd->socket,wsd->data,wsd->binary,yield);
        }
        else if(wsd->protocol.empty()){
           socketPool.Broadcast(wsd->data,wsd->binary,yield);
        }
        else{
            socketPool.Broadcast(wsd->protocol,wsd->data,wsd->binary,yield);
        }
    }
}

bool AsyncWebSocketServer::HasProtocol(const string&protocol){
    return socketPool.HasProtocol(protocol);
}

void AsyncWebSocketServer::LabelActive(const string&protocol){
    return socketPool.LabelActive(protocol);
}

void AsyncWebSocketServer::Post(websocket::stream<beast::tcp_stream>*socket,AutoPtr<FixBuffer>buffer){
    this->sendingDatas.push(new WebSocketSendData(socket,"",buffer->Clone()));
    boost::asio::spawn(
                ioc.get_executor(),
                std::bind(
                    &AsyncWebSocketServer::do_write,
                    this,
                    std::placeholders::_1));
}

void AsyncWebSocketServer::Post(websocket::stream<beast::tcp_stream>*socket,const string&data){
    this->sendingDatas.push(new WebSocketSendData(socket,"",data));
    boost::asio::spawn(
                ioc.get_executor(),
                std::bind(
                    &AsyncWebSocketServer::do_write,
                    this,
                    std::placeholders::_1));
}


void AsyncWebSocketServer::Broadcast(const string&data){
    this->sendingDatas.push(new WebSocketSendData(nullptr,"",data));
    boost::asio::spawn(
                ioc.get_executor(),
                std::bind(
                    &AsyncWebSocketServer::do_write,
                    this,
                    std::placeholders::_1));
}

void AsyncWebSocketServer::Broadcast(const string&protocol,const string&data){
    this->sendingDatas.push(new WebSocketSendData(nullptr,protocol,data));
    boost::asio::spawn(
                ioc.get_executor(),
                std::bind(
                    &AsyncWebSocketServer::do_write,
                    this,
                    std::placeholders::_1));
}

void AsyncWebSocketServer::Broadcast(AutoPtr<FixBuffer>buffer){
    this->sendingDatas.push(new WebSocketSendData(nullptr,"",buffer->Clone()));
    boost::asio::spawn(
                ioc.get_executor(),
                std::bind(
                    &AsyncWebSocketServer::do_write,
                    this,
                    std::placeholders::_1));
}

void AsyncWebSocketServer::Broadcast(const string&protocol,AutoPtr<FixBuffer>buffer){
    this->sendingDatas.push(new WebSocketSendData(nullptr,protocol,buffer->Clone()));
    boost::asio::spawn(
                ioc.get_executor(),
                std::bind(
                    &AsyncWebSocketServer::do_write,
                    this,
                    std::placeholders::_1));
}

}
