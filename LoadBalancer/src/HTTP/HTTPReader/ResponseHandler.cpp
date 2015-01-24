/////////////////////////////////////////////////////////////////////
//  ResponseHandler.cpp - implementation of ResponseHandler class     
//  ver 1.0                                                        
//  Language:      standard C++ 11                              
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../../include/HTTP/HTTPReader/HTTPReader.h"


//-------------------------------------------------------------------
// Constructor
// Initialize url, version and header_queue. Method is not initialized,
// and will be passed by an argument in handleHeader() function.
//-------------------------------------------------------------------
ResponseHandler::ResponseHandler(const std::string& url,
                const std::string& version,
                const std::queue<std::string>& header_queue)
    :url_(url), version_(version), header_queue_(header_queue)
{
    initHeaderHandlerTable();
}

//-------------------------------------------------------------------
// Initialize different method handlers' header handler table.
//-------------------------------------------------------------------
void ResponseHandler::initHeaderHandlerTable()
{
    // GET method handler can handle "Accept", "Host", "Source-IP" and "Source-Port".
    get_handler_table_.insert({ "Accept", handleAccept });
    get_handler_table_.insert({ "Host", handleHost });
    get_handler_table_.insert({ "Source-IP", handleSourceIP });
    get_handler_table_.insert({ "Source-Port", handleSourcePort });

    // HEAD method handler can handle "Accept", "Host", "Source-IP" and "Source-Port".
    head_handler_table_.insert({ "Accept", handleAccept });
    head_handler_table_.insert({ "Host", handleHost });
    head_handler_table_.insert({ "Source-IP", handleSourceIP });
    head_handler_table_.insert({ "Source-Port", handleSourcePort });

    // PUT method handler can handle "Host", "Content-Type", "Content-Length", 
    // "Source-IP" and "Source-Port".
    put_handler_table_.insert({ "Host", handleHost });
    put_handler_table_.insert({ "Content-Type", handleContentType });
    put_handler_table_.insert({ "Content-Length", handleContentLength });
    put_handler_table_.insert({ "Source-IP", handleSourceIP });
    put_handler_table_.insert({ "Source-Port", handleSourcePort });

    // POST method handler can handle "Host", "Content-Type", "Content-Length", 
    // "Source-IP" and "Source-Port".
    post_handler_table_.insert({ "Host", handleHost });
    post_handler_table_.insert({ "Content-Type", handleContentType });
    post_handler_table_.insert({ "Content-Length", handleContentLength });
    post_handler_table_.insert({ "Source-IP", handleSourceIP });
    post_handler_table_.insert({ "Source-Port", handleSourcePort });

    // TRACE method handler can handle "Accept", "Host", "Source-IP" and "Source-Port".
    trace_handler_table_.insert({ "Accept", handleAccept });
    trace_handler_table_.insert({ "Host", handleHost });
    trace_handler_table_.insert({ "Source-IP", handleSourceIP });
    trace_handler_table_.insert({ "Source-Port", handleSourcePort });

    // OPTIONS method handler can handle "Accept", "Host", "Source-IP" and "Source-Port".
    options_handler_table_.insert({ "Accept", handleAccept });
    options_handler_table_.insert({ "Host", handleHost });
    options_handler_table_.insert({ "Source-IP", handleSourceIP });
    options_handler_table_.insert({ "Source-Port", handleSourcePort });

    // DELETE method handler can handle "Host", "Source-IP" and "Source-Port".
    delete_handler_table_.insert({ "Host", handleHost });
    delete_handler_table_.insert({ "Source-IP", handleSourceIP });
    delete_handler_table_.insert({ "Source-Port", handleSourcePort });

    // SERVERCHECK method handler can handle "Host", "Source-IP" and "Source-Port".
    server_check_handler_table_.insert({ "Host", handleHost });
    server_check_handler_table_.insert({ "Source-IP", handleSourceIP });
    server_check_handler_table_.insert({ "Source-Port", handleSourcePort });

    // Error handler can handle "Accept", "Host", "Content-Type", 
    // "Content-Length", "Source-IP" and "Source-Port".
    error_handler_table_.insert({ "Accept", handleAccept });
    error_handler_table_.insert({ "Host", handleHost });
    error_handler_table_.insert({ "Content-Type", handleContentType });
    error_handler_table_.insert({ "Content-Length", handleContentLength });
    error_handler_table_.insert({ "Source-IP", handleSourceIP });
    error_handler_table_.insert({ "Source-Port", handleSourcePort });

    // header handler table stores different methods as keys, and 
    // corresponding handler table as values.
    header_handler_table_.insert({ "GET", get_handler_table_ });
    header_handler_table_.insert({ "HEAD", head_handler_table_ });
    header_handler_table_.insert({ "PUT", put_handler_table_ });
    header_handler_table_.insert({ "POST", post_handler_table_ });
    header_handler_table_.insert({ "TRACE", trace_handler_table_ });
    header_handler_table_.insert({ "OPTIONS", options_handler_table_ });
    header_handler_table_.insert({ "DELETE", delete_handler_table_ });
    header_handler_table_.insert({ "SERVERCHECK", server_check_handler_table_ });
    header_handler_table_.insert({ "ERROR", error_handler_table_ });

}

//-------------------------------------------------------------------
// Handler Host header
//-------------------------------------------------------------------
void ResponseHandler::handleHost(ResponseHandler* rh)
{
    DebugCode(std::cout << "host: " << rh->content_ << std::endl;)
}

//-------------------------------------------------------------------
// Handle Accept header
//-------------------------------------------------------------------
void ResponseHandler::handleAccept(ResponseHandler* rh)
{
    DebugCode(std::cout << "accept: " << rh->content_ << std::endl;)
}

//-------------------------------------------------------------------
// Handle Source-IP header
//-------------------------------------------------------------------
void ResponseHandler::handleSourceIP(ResponseHandler* rh)
{
    DebugCode(std::cout << "source ip: " << rh->content_ << std::endl;)
    rh->target_ip_ = rh->content_;
}

//-------------------------------------------------------------------
// Handle Source-Port header
//-------------------------------------------------------------------
void ResponseHandler::handleSourcePort(ResponseHandler* rh)
{
    DebugCode(std::cout << "source port: " << rh->content_ << std::endl;)
    rh->target_port_ = rh->content_;
}

//-------------------------------------------------------------------
// Handle Content-Type header
//-------------------------------------------------------------------
void ResponseHandler::handleContentType(ResponseHandler* rh)
{
    DebugCode(std::cout << "content type: " << rh->content_ << std::endl;)
}

//-------------------------------------------------------------------
// Handle Content-Length header
//-------------------------------------------------------------------
void ResponseHandler::handleContentLength(ResponseHandler* rh)
{
    DebugCode(std::cout << "content length: " << rh->content_ << std::endl;)
}

//-------------------------------------------------------------------
// Get information of each header title, marked by ':'
//-------------------------------------------------------------------
void ResponseHandler::getHeaderInfo()
{
    msg_ = header_queue_.front();
    header_queue_.pop();

    for (unsigned i = 0; i < msg_.size(); i++)
    {
        if (msg_.at(i) == ':')
        {
            header_ = msg_.substr(0, i);

            // There should be a whitespace after ':', otherwise
            // sbustr(i + 2, msg_size() - i)
            content_ = msg_.substr(i + 2, msg_.size() - i - 1);
            break;
        }
    }
}

//-------------------------------------------------------------------
// According to the method, invoke method handler, find corresponding
// header handler in each method's header handler table
// return  0: success
//        -1: error
//-------------------------------------------------------------------
int ResponseHandler::handleHeaders(const std::string& method)
{
    while (header_queue_.size() > 0)
    {
        getHeaderInfo();

        HandlerTable handler = header_handler_table_.at(method);
        if (handler.find(header_) != handler.end())
            handler.at(header_)(this);
        else
        {
            std::cout << "Unknown Header: " << header_ << std::endl;
            error_code_ = StatusCode::ClientErrorStatusCode::HEAD400;
        }
    } 

    if (error_code_.size() > 0) 
    {
        ErrorMessage em("HTTP/1.1", error_code_, target_ip_, target_port_);
        em.constructHTTPMsg(http_msg_);
        return -1;
    }
    return 0;
}

//-------------------------------------------------------------------
// Handle GET method request, open the file according to URL, get
// the content, construct response message and send it back.
//-------------------------------------------------------------------
// Response example:
// HTTP/1.1 200 OK
// Target-IP: 127.0.0.1
// Target-Port: 8080
// Content-Type: text/plain
// Content-Length: 15
// 
// message to get
// 
//-------------------------------------------------------------------
HTTPMessage ResponseHandler::getResponse()
{
    // If return value is -1, there is an error, and http_msg_ 
    // is corresponding error response 
    if (handleHeaders("GET") == -1)
        return http_msg_;
    
    int fd = open(url_.c_str(), O_RDONLY);
    if (fd == -1)
    {
        if (errno == ENOENT) 
        {
            std::cout << "This file doesn't exist.\n";
            // HEAD404 = "404 Not Found"
            error_code_ = StatusCode::ClientErrorStatusCode::HEAD404;
        }
        else if (errno == EACCES) 
        {
            std::cout << "No access to this file.\n";
            //HEAD401 = "401 Unauthorized"
            error_code_ = StatusCode::ClientErrorStatusCode::HEAD401;
        }
        else 
        {
            std::cout << "Internal error.\n";
            // HEAD500 = "500 Internal Server Error"
            error_code_ = StatusCode::ServerErrorStatusCode::HEAD500;
        }

        ErrorMessage em("HTTP/1.1", error_code_, target_ip_, target_port_);
        em.constructHTTPMsg(http_msg_);
        return http_msg_;
    }
    
    // declare a read file lock
    struct flock fl;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;

    // The content read from the file should not be too long,
    // because length of HTTP message is 4096
    char buffer[HTTPMessage::HTTP_MSG_SIZE - 1096]; // 2650
    memset(buffer, '\0', HTTPMessage::HTTP_MSG_SIZE - 1096);

    // Lock the file to avoid conflict 
    int status;
    status = fcntl(fd, F_SETLKW, &fl);
    if (status == -1)
        perror("fcntl");
    
    ssize_t num_read = read(fd, buffer, HTTPMessage::HTTP_MSG_SIZE - 1096);

    // Unlock the file
    fl.l_type = F_UNLCK;
    status = fcntl(fd, F_SETLKW, &fl);
    if (status == -1) 
        perror("fcntl");

    buffer[strlen(buffer) - 1] = '\0';
    close(fd);

    // Construct response message
    ResponseMessage rm("HTTP/1.1", StatusCode::SuccessStatusCode::HEAD200, target_ip_, target_port_);
    rm.addContentType("text/plain");
    std::string content_length = convertToString<int>(strlen(buffer));
    rm.addContentLength(content_length);
    rm.addBody(buffer);
    rm.constructHTTPMsg(http_msg_);

    return http_msg_;
}

//-------------------------------------------------------------------
// Handle HEAD method request, open the file according to URL, get
// the content, construct response message without the content and 
// send it back.
// ------------------------------------------------------------------
// Response example:
// HTTP/1.1 200 OK
// Target-IP: 127.0.0.1
// Target-Port: 8080
// Content-Type: text/plain
// Content-Length: 15
// 
//-------------------------------------------------------------------
HTTPMessage ResponseHandler::headResponse()
{
    // If return value is -1, there is an error, and http_msg_ 
    // is corresponding error response 
    if (handleHeaders("HEAD") == -1)
        return http_msg_;

    // HEAD method response is almost the same with GET, with the only 
    // difference that head response has no body
    
    int fd = open(url_.c_str(), O_RDONLY);
    if (fd == -1)
    {
        if (errno == ENOENT)
        {
            std::cout << "This file doesn't exist.\n";
            // HEAD404 = "404 Not Found"
            error_code_ = StatusCode::ClientErrorStatusCode::HEAD404;
        }
        else if (errno == EACCES)
        {
            std::cout << "No access to this file.\n";
            //HEAD401 = "401 Unauthorized"
            error_code_ = StatusCode::ClientErrorStatusCode::HEAD401;
        }
        else
        {
            std::cout << "Internal error.\n";
            // HEAD500 = "500 Internal Server Error"
            error_code_ = StatusCode::ServerErrorStatusCode::HEAD500;
        }

        ErrorMessage em("HTTP/1.1", error_code_, target_ip_, target_port_);
        em.constructHTTPMsg(http_msg_);
        return http_msg_;
    }

    // declare a read file lock
    struct flock fl;
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;

    // The content read from the file should not be too long,
    // because length of HTTP message is 4096
    char buffer[HTTPMessage::HTTP_MSG_SIZE - 1096]; // 2650
    memset(buffer, '\0', HTTPMessage::HTTP_MSG_SIZE - 1096);

    // Lock the file to avoid conflict 
    int status;
    status = fcntl(fd, F_SETLKW, &fl);
    if (status == -1)
        perror("fcntl");
    
    ssize_t num_read = read(fd, buffer, HTTPMessage::HTTP_MSG_SIZE - 1096);

    // Unlock the file
    fl.l_type = F_UNLCK;
    status = fcntl(fd, F_SETLKW, &fl);
    if (status == -1) 
        perror("fcntl");

    buffer[strlen(buffer) - 1] = '\0';
    close(fd);

    // Construct response message
    ResponseMessage rm("HTTP/1.1", StatusCode::SuccessStatusCode::HEAD200, target_ip_, target_port_);
    rm.addContentType("text/plain");
    std::string content_length = convertToString<int>(strlen(buffer));
    rm.addContentLength(content_length);
    rm.constructHTTPMsg(http_msg_);

    return http_msg_;
}

//-------------------------------------------------------------------
// Handle PUT method request, write body of the request into the file
// named by URL, and send back a response.
// ------------------------------------------------------------------
// Response example:
// HTTP/1.1 201 Created
// Location: ./put.txt
// Target-IP: 127.0.0.1
// Target-Port: 8080
// Content-Type: text/plain
// Content-Length: 10
// 
// ./put.txt
//
//-------------------------------------------------------------------
HTTPMessage ResponseHandler::putResponse(std::string& body)
{
    // If return value is -1, there is an error, and http_msg_ 
    // is corresponding error response 
    if (handleHeaders("PUT") == -1)
        return http_msg_;

    int fd = open(url_.c_str(), O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        if (errno == EACCES) 
        {
            std::cout << "No access to this file.\n";
            // HEAD401 = "401 Unauthorized"
            error_code_ = StatusCode::ClientErrorStatusCode::HEAD401;
        }
        else
        {
            std::cout << "Internal error.\n";
            // HEAD500 = "500 Internal Server Error"
            error_code_ = StatusCode::ServerErrorStatusCode::HEAD500;
        }

        ErrorMessage em("HTTP/1.1", error_code_, target_ip_, target_port_);
        em.constructHTTPMsg(http_msg_);
        return http_msg_;
    }
    
    // Declare a write file lock
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;

    // Lock the file to avoid conflict
    int status;
    status = fcntl(fd, F_SETLKW, &fl);
    if (status == -1) 
        perror("fcntl - F_SETLKW");

    ssize_t num_written = write(fd, body.c_str(), body.size());

    // Unlock the file before determine value of num_write.
    // Make the lock time as least as possible.
    fl.l_type = F_UNLCK;
    status = fcntl(fd, F_SETLKW, &fl);
    if (status == -1) 
        perror("fcntl - F_SETLKW");

    close(fd);

    // construct response message
    if (num_written != body.size()) 
    {
        ErrorMessage em("HTTP/1.1", StatusCode::ServerErrorStatusCode::HEAD500, target_ip_, target_port_);
        em.constructHTTPMsg(http_msg_);
        return http_msg_;
    }
    else 
    {
        ResponseMessage rm("HTTP/1.1", StatusCode::SuccessStatusCode::HEAD201, target_ip_, target_port_);
        rm.addLocation(url_);
        rm.addContentType("text/plain");
        rm.addContentLength(convertToString<unsigned>(url_.size()));
        rm.addBody(url_);
        rm.constructHTTPMsg(http_msg_);

        return http_msg_;
    }
}

//-------------------------------------------------------------------
// Handle POST method request. In reality, POST means to insert the 
// body of the request into a HTML file and send a response back.
// While, in my handler, I just concatenate " is in stock." to the
// request body and send response back.
// ------------------------------------------------------------------
// Response example:
// HTTP/1.1 200 OK
// Target-IP: 127.0.0.1
// Target-Port: 8080
// Content-Type: text/plain
// Content-Length: 25
// 
// color=green is in stock.
//
//-------------------------------------------------------------------
HTTPMessage ResponseHandler::postResponse(std::string& body)
{
    // If return value is -1, there is an error, and http_msg_ 
    // is corresponding error response 
    if (handleHeaders("POST") == -1)
        return http_msg_;

    // Construct response message
    std::string result = body + " is in stock";
    ResponseMessage rm("HTTP/1.1", StatusCode::SuccessStatusCode::HEAD200, target_ip_, target_port_);
    rm.addContentType("text/plain");
    rm.addContentLength(convertToString<unsigned>(result.size() + 1));
    rm.addBody(result);
    rm.constructHTTPMsg(http_msg_);

    return http_msg_;

}

//-------------------------------------------------------------------
// Handle TRACE method request, put the whole request as response body
// and send response back.
// ------------------------------------------------------------------
// Response example:
// HTTP/1.1 200 OK
// Target-IP: 127.0.0.1
// Target-Port: 8080
// Content-Type: text/plain
// Content-Length: 53
// 
// TRACE ./trace.txt HTTP/1.1
// Host: localhost
// Accept: *
//
//-------------------------------------------------------------------
HTTPMessage ResponseHandler::traceResponse(std::string& request_msg_)
{
    // If return value is -1, there is an error, and http_msg_ 
    // is corresponding error response
    if (handleHeaders("TRACE") == -1)
        return http_msg_;

    // Construct response message
    ResponseMessage rm("HTTP/1.1", StatusCode::SuccessStatusCode::HEAD200, target_ip_, target_port_);
    rm.addContentType("text/plain");
    rm.addContentLength(convertToString<int>(request_msg_.size()));
    rm.addBody(request_msg_);
    rm.constructHTTPMsg(http_msg_);

    return http_msg_;
}

//-------------------------------------------------------------------
// Handle OPTIONS method request, put allowed methods in the response
// header and send it back.
// ------------------------------------------------------------------
// Response example:
// HTTP/1.1 200 OK
// Target-IP: 127.0.0.1
// Target-Port: 8080
// Allow: PUT, GET, OPTIONS
// Content-Length: 0
//
//-------------------------------------------------------------------
HTTPMessage ResponseHandler::optionsResponse()
{
    // If return value is -1, there is an error, and http_msg_ 
    // is corresponding error response
    if (handleHeaders("OPTIONS") == -1)
        return http_msg_;

    // Construct response message
    ResponseMessage rm("HTTP/1.1", StatusCode::SuccessStatusCode::HEAD200, target_ip_, target_port_);
    rm.addContentLength("0");
    rm.addAllow("GET, HEAD, PUT, POST, TRACE, OPTIONS, DELETE"); // No server check is here because server check
                                                                 // should not be transparent to clients 
    rm.constructHTTPMsg(http_msg_);

    return http_msg_;
}

//-------------------------------------------------------------------
// Handle DELETE method request, delete the file named by URL in the 
// request and send response back.
// ------------------------------------------------------------------
// Response example:
// HTTP/1.1 200 OK
// Target-IP: 127.0.0.1
// Target-Port: 8080
// Content-Type: text/plain
// Content-Length: 17
//
// File is deleted.
//-------------------------------------------------------------------
HTTPMessage ResponseHandler::deleteResponse()
{
    // If return value is -1, there is an error, and http_msg_ 
    // is corresponding error response
    if (handleHeaders("DELETE") == -1)
        return http_msg_;

    int fd = open("./lock.txt", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd == -1)
    {
        ErrorMessage em("HTTP/1.1", StatusCode::ServerErrorStatusCode::HEAD500, target_ip_, target_port_);
        em.constructHTTPMsg(http_msg_);
        return http_msg_;
    }

    // Declare a write file lock
    struct flock fl;
    fl.l_type = F_WRLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    fl.l_pid = 0;

    // Lock the file to avoid delete conflict
    int status;
    status = fcntl(fd, F_SETLKW, &fl);
    if (status == -1) 
        perror("fcntl - F_SETLKW");

    // Remove action is locked.
    int ret = remove(url_.c_str());

    // Unlock the file before checking value of ret.
    // Make the lock time as least as possible.
    fl.l_type = F_UNLCK;
    status = fcntl(fd, F_SETLKW, &fl);
    if (status == -1) 
        perror("fcntl - F_SETLKW");

    if (ret == -1)
    {
        perror("remove");
        if (errno == EACCES) 
        {
            std::cout << "Permission denied.\n";
            // HEAD401 = "401 Unauthorized"
            error_code_ = StatusCode::ClientErrorStatusCode::HEAD401;
        }
        else if (errno == ENOENT) 
        {
            std::cout << "No such file.\n";
            // HEAD404 = "Not Found"
            error_code_ = StatusCode::ClientErrorStatusCode::HEAD404;
        }
        else if (errno == ENAMETOOLONG) 
        {
            std::cout << "File name is too long.\n";
            // HEAD414 = "Request URL Too Long"
            error_code_ = StatusCode::ClientErrorStatusCode::HEAD414;
        }
        else 
        {
            std::cout << "Internal error.\n";
            // HEAD500 = "Internal Server error.\n"
            error_code_ = StatusCode::ServerErrorStatusCode::HEAD500;
        }

        ErrorMessage em("HTTP/1.1", error_code_, target_ip_, target_port_);
        em.constructHTTPMsg(http_msg_);
        return http_msg_;
    }
    
    // Construct response message
    ResponseMessage rm("HTTP/1.1", StatusCode::SuccessStatusCode::HEAD200, target_ip_, target_port_);
    rm.addContentType("text/plain");
    rm.addContentLength("17");
    rm.addBody("File is deleted.");
    rm.constructHTTPMsg(http_msg_);

    return http_msg_;
}

//-------------------------------------------------------------------
// Handle SERVERCHECK method request, put max load of a real server
// in the response body and send it back. SERVERCHECK method is used
// between load balancer and real servers.
// ------------------------------------------------------------------
// Response example:
// HTTP/1.1 200 OK
// Target-IP: 127.0.0.1
// Target-Port: 8080
//
// 10
//
//-------------------------------------------------------------------
HTTPMessage ResponseHandler::serverCheckResponse(std::string& max_load)
{
    // If return value is -1, there is an error, and http_msg_ 
    // is corresponding error response
    if (handleHeaders("SERVERCHECK") == -1)
        return http_msg_;

    ResponseMessage rm("HTTP/1.1", StatusCode::SuccessStatusCode::HEAD200, target_ip_, target_port_);
    rm.addBody(max_load);
    rm.constructHTTPMsg(http_msg_);

    return http_msg_;
}

//-------------------------------------------------------------------
// Handle ERROR request, send corresponding error message back.
// ------------------------------------------------------------------
// Response example:
// HTTP/1.1 404 Not Found
// Target-IP: 127.0.0.1
// Target-Port: 8080
//
//-------------------------------------------------------------------
HTTPMessage ResponseHandler::errorResponse(std::string& error)
{
    // Do not need to check the return value, because an error
    // has already taken place and should be returned to the client
    handleHeaders("ERROR");

    ErrorMessage em("HTTP/1.1", error, target_ip_, target_port_);
    em.constructHTTPMsg(http_msg_);
    return http_msg_;
}


