/////////////////////////////////////////////////////////////////////
//  RequestMessage.cpp - implementations of request message writer          
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Win 8.1, 64-bit                            
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../../include/HTTP/HTTPWriter/HTTPWriter.h"


//-------------------------------------------------------------------
// Add method behind current start line
// such as addMethod("TRACE")
// Method should usually be the first information in a request.
//-------------------------------------------------------------------
HTTPWriter& RequestMessage::addMethod(const std::string& method)
{
    start_line_ += method + " ";
    return *this;
}

//-------------------------------------------------------------------
// Add request URL behind current start line
// such as addRequestURL("./request.txt")
// Request URL is often used to point out target file to be operated.
//-------------------------------------------------------------------
HTTPWriter& RequestMessage::addRequestURL(const std::string& url)
{
    start_line_ += url + " ";
    return *this;
}

//-------------------------------------------------------------------
// Add source IP behind current header
// such as addSourceIP("127.0.0.1")
//-------------------------------------------------------------------
HTTPWriter& RequestMessage::addSourceIP(const std::string& source_ip)
{
    header_ += "Source-IP: " + source_ip + "\r\n";
    return *this;
}

//-------------------------------------------------------------------
// Add source port behind current header
// such as addSourceIP("8080")
//-------------------------------------------------------------------
HTTPWriter& RequestMessage::addSourcePort(const std::string source_port)
{
    header_ += "Source-Port: " + source_port + "\r\n";
    return *this;
}