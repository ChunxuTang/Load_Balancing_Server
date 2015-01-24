#ifndef HTTP_WRITER_H
#define HTTP_WRITER_H
/////////////////////////////////////////////////////////////////////
//  HTTPWriter.h - definitions of HTTP message writers          
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      HP 4441s, Win 8.1                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////
/*
 * File Description:
 * ==================
 * This file defines these classes:
 * (1) HTTPWriter, base class of other classes
 * (2) ResponseMessage, inherits HTTPWriter and construct HTTP response
 *     messages.
 * (3) RequestMessage, also inherits HTTPWriter and construct HTTP request
 *     messages.
 * (4) ErrorMessage, constructs an response according an error.
 * (5) GetMethodWriter, constructs an HTTP message with GET method.
 * (6) HeadMethodWriter, constructs an HTTP message with HEAD method.
 * (7) PutMethodWriter, constructs an HTTP message with PUT method.
 * (8) PostMethodWriter, constructs an HTTP message with POST method.
 * (9) TraceMethodWriter, constructs an HTTP message with TRACE method.
 * (10)OptionsMethodWriter, constructs and HTTP message with OPTIONS method.
 * (11)DeleteMethodWriter, constructs an HTTP message with DELETE method.
 * (12)ServerCheckMethodWriter, constructs and HTTP message with SERVERCHECK
 *     method, which is used in Load Balancer to check situation of other
 *     real servers.
 *
 * From (4) to (12), these classes are application specific in Summer Project
 * - Load Balancing Server Design. With (1) to (4) classes, a programmer can
 * organize a complete HTTP message.
 *
 * Required Files:
 * ===============
 * HTTPBasic.h, HTTPWriter.cpp, ResponseMessage.cpp, RequestMessage.cpp
 *
 * Maintenance History:
 * ====================
 * ver 1.0 : 10 Jul 2014
 * - first release
*/


#include "HTTPBasic.h"
#include <string>
#include <cstring> // used for sprintf



//***********************************************************************
// HTTPWriter
//
// This class is the base class of other classes in this file. This class
// partitions an HTTP message into three parts, start line, header and body.
// The class defines basic operations of organization of these parts. 
// This class can itself construct an HTTP message, but need many operations.
//***********************************************************************

class HTTPWriter
{
public:
    HTTPWriter(){}
    virtual ~HTTPWriter(){}

    HTTPWriter(const HTTPWriter& http_writer);
    HTTPWriter& operator=(const HTTPWriter& http_writer);

    // operations of start line, header and body
    HTTPWriter& addStartLine(const std::string& start_line);
    HTTPWriter& addHeader(const std::string& header);
    HTTPWriter& addBody(const std::string& body);
    std::string getStartLine() const;
    std::string getHeader() const;
    std::string getBody() const;
    
    // add information of start line
    HTTPWriter& addVersion(const std::string& version);

    // add entity headers
    HTTPWriter& addContentType(const std::string& content_type);
    HTTPWriter& addContentLength(const std::string& content_length);

    void constructHTTPMsg(HTTPMessage &http_msg);
    void constructString(std::string &msg);

    HTTPWriter& clear();
    void showInfo(const HTTPMessage& http_msg);
protected:
    std::string start_line_;
    std::string header_;
    std::string body_;
};



//***********************************************************************
// ResponseMessage
//
// This class defines operations to construct a response HTTP message. A
// response message is usually sent by a server, according to a request
// message sent by a client. There is no method in a response message,
// while, on the other side, there is a status code, representing the result
// status. The details of status code are defined in HTTPBasic.h
//
// An example of a response message:
// HTTP/1.1 200 OK
// Allow: GET, PUT, OPTIONS, DELETE
// Content-Length: 0
// Target-IP: 127.0.0.1
// Target-Port: 8080
// 
//***********************************************************************

class ResponseMessage : public HTTPWriter
{
public:
    // Three kinds of constructors, which are used in different applications.
    // target_ip and target_port are specifically used between Load Balancer
    // and Real Servers to decide target client.
    ResponseMessage(const std::string& version, 
                    const std::string& status_code);
    ResponseMessage(const std::string& version, 
                    const std::string& status_code,
                    const std::string& target_ip, 
                    const std::string& target_port);
    ResponseMessage(const std::string& version, 
                    const std::string& status_code, 
                    const std::string& content_type, 
                    const std::string& content_length, 
                    const std::string& allow_method, 
                    const std::string& location, 
                    const std::string& target_ip,
                    const std::string& target_port);
    ~ResponseMessage(){}

    // add response start line information
    HTTPWriter& addStatusCode(const std::string& status_code);
    HTTPWriter& addStatus(const std::string& status);
    HTTPWriter& addReasonPhrase(const std::string& reason_phrase);

    // add response headers
    HTTPWriter& addLocation(const std::string& location);
    HTTPWriter& addAllow(const std::string& allow_method);
    HTTPWriter& addTargetIP(const std::string& target_ip);
    HTTPWriter& addTargetPort(const std::string& target_port);
};


//***********************************************************************
// RequestMessage
//
// This class defines operations to construct a request HTTP message. A
// request message is usually sent by a client, with a method defined in
// Method enumeration.
//
// An example of a request message:
// GET ./request.txt HTTP/1.1
// Host: localhost
// Accept: *
// Source-IP: 127.0.0.1
// Source-Port: 8080
//
//***********************************************************************

class RequestMessage : public HTTPWriter
{
public:
    RequestMessage(){}
    ~RequestMessage(){}
    HTTPWriter& addMethod(const std::string& method);
    HTTPWriter& addRequestURL(const std::string& url);

    // source_ip and source_port are used in Summer Project 2014. 
    // A client sends these information to make Load Balancer acquire
    // the target of a response message.
    HTTPWriter& addSourceIP(const std::string& source_ip);
    HTTPWriter& addSourcePort(const std::string source_port);
};


//***********************************************************************
// ErrorMessage
//
// This class defines operations to construct an HTTP message, representing
// an error. It is used when a server or load balancer occurs an error 
// when handling clients' requests.
//
// An example of an error message:
// HTTP/1.1 404 Not Found
// Target-IP: 127.0.0.1
// Target-Port: 8080
//
//***********************************************************************

class ErrorMessage : public HTTPWriter
{
public:
    ErrorMessage(const std::string& version,
                 const std::string& status_code,
                 const std::string& target_ip,
                 const std::string& target_port)
    {
        start_line_ = version + " " + status_code + " ";
        header_ = "Target-IP: " + target_ip + "\r\n" + 
                  "Target-Port: " + target_port + "\r\n";
        addContentLength("0");
    }

};


//***********************************************************************
// GetMethodWriter
//
// This class defines operations to construct an HTTP message with GET method.
// Get method means the client asks a server to send a resource. 
//
// An example:
// GetMethodWriter gmw("./get.txt", "HTTP/1.1", "localhost", "*", "127.0.0.1", "8080");
// HTTPMessage http_msg;
// gmw.constructHTTPMsg(http_msg);
// =>
// GET ./get.txt HTTP/1.1
// Host: localhost
// Accept: *
// Source-IP: 127.0.0.1
// Source-Port: 8080
//
//***********************************************************************

class GetMethodWriter : public HTTPWriter
{
public:
    GetMethodWriter(const std::string& url, 
                    const std::string& version, 
                    const std::string& hostname, 
                    const std::string& accept_format,
                    const std::string& source_ip, 
                    const std::string& source_port)
    {
        start_line_ = "GET " + url + " " + version + " ";
        header_ = "Host: " + hostname + "\r\n" + 
                  "Accept: " + accept_format + "\r\n" +
                  "Source-IP: " + source_ip + "\r\n" + 
                  "Source-Port: " + source_port + "\r\n";
    }
};


//***********************************************************************
// HeadMethodWriter
//
// This class defines operations to construct an HTTP message with HEAD method.
// HEAD method is similar to GET method, and the only difference is that
// in HEAD method, the server only needs to send the headers, no entity body
// is ever returned.
//
// An example:
// HEAD ./head.txt HTTP/1.1
// Host: localhost
// Accept: *
// Source-IP: 127.0.0.1
// Source-Port: 8080
//
//***********************************************************************

class HeadMethodWriter : public HTTPWriter
{
public:
    HeadMethodWriter(const std::string& url, 
                     const std::string& version, 
                     const std::string& hostname, 
                     const std::string& accept_format,
                     const std::string& source_ip, 
                     const std::string& source_port)
    {
        start_line_ = "HEAD " + url + " " + version + " ";
        header_ = "Host: " + hostname + "\r\n" + 
                  "Accept: " + accept_format + "\r\n" + 
                  "Source-IP: " + source_ip + "\r\n" + 
                  "Source-Port: " + source_port + "\r\n";
    }
};


//***********************************************************************
// PutMethodWriter
//
// This class defines operations to construct an HTTP message with PUT method.
// Put method is opposite to GET method, it asks the server to create a file
// with content of the HTTP message's body. If the file has already existed,
// replace the original content with HTTP message's body. 
//
// An example:
// PUT ./put.txt HTTP/1.1
// Host: localhost
// Content-Type: text/plain
// Content-Length: 18
// Source-IP: 127.0.0.1
// Source-Port: 8080
//
// message to be put
//
//***********************************************************************

class PutMethodWriter: public HTTPWriter
{
public:
    PutMethodWriter(const std::string& url, 
                    const std::string& version, 
                    const std::string& hostname, 
                    const std::string& content_type,
                    const std::string& content_length, 
                    const std::string& source_ip, 
                    const std::string& source_port)
    {
        start_line_ += "PUT " + url + " " + version + " ";
        header_ += "Host: " + hostname + "\r\n";
        header_ += "Content-Type: " + content_type + "\r\n";
        header_ += "Content-Length: " + content_length + "\r\n";
        header_ += "Source-IP: " + source_ip + "\r\n";
        header_ += "Source-Port: " + source_port + "\r\n";
    }
};


//***********************************************************************
// PostMethodWriter
//
// This class defines operation to construct an HTTP message with POST method.
// Post method means the client is trying to send input data to the server.
// In practice, it is usually used to support HTML forms.
//
// An example:
// POST post.html HTTP/1.1
// Host: localhost
// Content-Type: text/plain
// Content-Length: 10
// Source-IP: 127.0.0.1
// Source-Port: 8080
//
// color=red
//
//***********************************************************************

class PostMethodWriter : public HTTPWriter
{
public:
    PostMethodWriter(const std::string& url, 
                     const std::string& version, 
                     const std::string& hostname, 
                     const std::string& content_type, 
                     const std::string& content_length, 
                     const std::string& source_ip, 
                     const std::string& source_port)
    {
        start_line_ += "POST " + url + " " + version + " ";
        header_ += "Host: " + hostname + "\r\n";
        header_ += "Content-Type: " + content_type + "\r\n";
        header_ += "Content-Length: " + content_length + "\r\n";
        header_ += "Source-IP: " + source_ip + "\r\n";
        header_ += "Source-Port: " + source_port + "\r\n";
    }
};


//***********************************************************************
// TraceMethodWriter
//
// This class defines operation to construct an HTTP message with TRACE 
// method. Trace method is used for diagnostics. A server is asked to 
// encapsulate what it receives from the client in a HTTP message body 
// and sends it back.
//
// An example:
// TRACE ./trace.txt HTTP/1.1
// Host: localhost
// Accept: *
// Source-IP: 127.0.0.1
// Source-Port: 8080
//
//***********************************************************************

class TraceMethodWriter : public HTTPWriter
{
public:
    TraceMethodWriter(const std::string& url, 
                      const std::string& version, 
                      const std::string& hostname, 
                      const std::string& accept_format,
                      const std::string& source_ip, 
                      const std::string& source_port)
    {
        start_line_ += "TRACE " + url + " " + version + " ";
        header_ += "Host: " + hostname + "\r\n";
        header_ += "Accept: " + accept_format + "\r\n";
        header_ += "Source-IP: " + source_ip + "\r\n";
        header_ += "Source-Port: " + source_port + "\r\n";
    }
};


//***********************************************************************
// OptionsMethodWriter
//
// This class defines operations to construct an HTTP message with OPTIONS 
// method. OPTIONS method means that a client asks a server to send back 
// supported capabilities, such as methods.  
//
// An example:
// OPTIONS * HTTP/1.1
// Host: localhost
// Accept: *
// Source-IP: 127.0.0.1
// Source-Port: 8080
//
//***********************************************************************

class OptionsMethodWriter : public HTTPWriter
{
public:
    OptionsMethodWriter(const std::string& url, 
                        const std::string& version, 
                        const std::string& hostname, 
                        const std::string& accept_format,
                        const std::string& source_ip,
                        const std::string& source_port)
    {
        start_line_ += "OPTIONS " + url + " " + version + " ";
        header_ += "Host: " + hostname + "\r\n";
        header_ += "Accept: " + accept_format + "\r\n";
        header_ += "Source-IP: " + source_ip + "\r\n";
        header_ += "Source-Port: " + source_port + "\r\n";
    }
};


//***********************************************************************
// DeleteMethodWriter
//
// This class defines operations to construct an HTTP message with DELETE 
// method. DELETE method asks a server to delete a resource specified by 
// URL in the HTTP message start line.
//
// An example:
// DELETE ./delete.txt HTTP/1.1
// Host: localhost
// Source-IP: 127.0.0.1
// Source-Port: 8080
//
//***********************************************************************

class DeleteMethodWriter : public HTTPWriter
{
public:
    DeleteMethodWriter(const std::string& url, 
                       const std::string& version, 
                       const std::string& hostname, 
                       const std::string& source_ip, 
                       const std::string& source_port)
    {
        start_line_ += "DELETE " + url + " " + version + " ";
        header_ += "Host: " + hostname + "\r\n" + 
                   "Source-IP: " + source_ip + "\r\n" + 
                   "Source-Port: " + source_port + "\r\n";
    }
};


//***********************************************************************
// ServerCheckMethodWriter
//
// This class defines operations to construct an HTTP message with SERVERCHECK 
// method. SERVERCHECK is used specifically in Summer Project 2014. The Load 
// Balancer communicates with real servers by sending SERVERCHECK messages
// and get a real server's max load.
//
// An example:
// SERVERCHECK * HTTP/1.1
// Host: localhost
// Source-IP: 127.0.0.1
// Source-Port: 8080
//
//***********************************************************************

class ServerCheckMethodWriter : public HTTPWriter
{
public:
    ServerCheckMethodWriter(const std::string& url, 
                            const std::string& version,
                            const std::string& hostname, 
                            const std::string& source_ip,
                            const std::string& source_port)
    {
        start_line_ += "SERVERCHECK " + url + " " + version + " ";
        header_ += "Host: " + hostname + "\r\n" +
                   "Source-IP: " + source_ip + "\r\n" + 
                   "Source-Port: " + source_port + "\r\n";
    }
};


#endif
