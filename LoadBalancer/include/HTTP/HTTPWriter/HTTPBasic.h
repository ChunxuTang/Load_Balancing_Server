#ifndef HTTP_BASIC_H
#define HTTP_BASIC_H
/////////////////////////////////////////////////////////////////////
//  HTTPBasic.h - definitions of basic concepts of HTTP        
//  ver 1.0                                                        
//  Language:      standard C++                                
//  Platform:      Win 8.1, 64-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////
/*
* File Description:
* ==================
* Define concepts of status code and HTTP message. Status code is used
* for clients to understand results of their requests. HTTP message is
* defined as a struct, with a static const variable and a string.
*
* Required Files:
* ===============
* HTTPBasic.h
*
* Maintenance History:
* ====================
* ver 1.0 : 10 Jul 2014
* - first release
*/


#include <string>
using namespace std;

//***********************************************************************
// StatusCode
//
// Status code is used for clients to understand results of their requests.
// It falls into five categories:
// (1) 100-199 Informational Status Code
// (2) 200-299 Success Status Code
// (3) 300-399 Redirection Status Code
// (4) 400-499 Client Error Status Code
// (5) 500-599 Server Error Status Code
//***********************************************************************

namespace StatusCode
{
    namespace InformationalStatusCode
    {
        const string HEAD100 = "100 Continue";
        const string HEAD101 = "101 Switching Protocols";
    }
    namespace SuccessStatusCode
    {
        const string HEAD200 = "200 OK";
        const string HEAD201 = "201 Created";
        const string HEAD202 = "202 Accepted";
        const string HEAD203 = "203 Non-Authoritative Information";
        const string HEAD204 = "204 No Content";
        const string HEAD205 = "205 Reset Content";
        const string HEAD206 = "206 Partial Content";
    }
    namespace RedirectionStatusCode
    {
        const string HEAD300 = "300 Multiple Choices";
        const string HEAD301 = "301 Moved Permanently";
        const string HEAD302 = "302 Found";
        const string HEAD303 = "303 See Other";
        const string HEAD304 = "304 Not Modified";
        const string HEAD305 = "305 Use Proxy";
        const string HEAD306 = ""; // currently unused
        const string HEAD307 = "307 Temporary Redirect";
    }
    namespace ClientErrorStatusCode
    {
        const string HEAD400 = "400 Bad Request";
        const string HEAD401 = "401 Unauthorized";
        const string HEAD402 = "402 Payment Required";
        const string HEAD404 = "404 Not Found"; 
        const string HEAD405 = "405 Method Not Allowed"; 
        const string HEAD406 = "406 Not Acceptable";
        const string HEAD407 = "407 Proxy Authentication Required";
        const string HEAD408 = "408 Request Timeout";
        const string HEAD409 = "409 Conflict";
        const string HEAD410 = "410 Gone";
        const string HEAD411 = "411 Length Required";
        const string HEAD412 = "412 Precondition Failed";
        const string HEAD413 = "413 Request Entity Too Long";
        const string HEAD414 = "414 Request URL Too Long";
        const string HEAD415 = "415 Unsupported Media Type";
        const string HEAD416 = "416 Requested Range Not Satisfiable";
        const string HEAD417 = "417 Expectation Failed";
    }
    namespace ServerErrorStatusCode
    {
        const string HEAD500 = "500 Internal Server Error"; 
        const string HEAD501 = "501 Not Implemented";
        const string HEAD502 = "502 Bad Gateway";
        const string HEAD503 = "503 Service Unavailable"; 
        const string HEAD504 = "504 Gateway Timeout";
        const string HEAD505 = "505 HTTP Version Not Supported"; 
    }
}


//***********************************************************************
// HTTPMessage
//
// The struct is used in HTTPWriter and HTTPReader, defined as the bridge
// between clients and servers.
//***********************************************************************

struct HTTPMessage
{
    static const int HTTP_MSG_SIZE = 4096;
    char http_msg[HTTP_MSG_SIZE];
};


#endif