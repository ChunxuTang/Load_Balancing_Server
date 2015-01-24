/////////////////////////////////////////////////////////////////////
//  HTTPWriter.cpp - implementations of HTTP message writers          
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Win 8.1, 64-bit                           
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////


#include <iostream>
#include "../../../include/HTTP/HTTPWriter/HTTPWriter.h"

//-------------------------------------------------------------------
// copy constructor
//-------------------------------------------------------------------
HTTPWriter::HTTPWriter(const HTTPWriter& http_writer)
    : start_line_(http_writer.start_line_), 
      header_(http_writer.header_), 
      body_(http_writer.body_)
{}

//-------------------------------------------------------------------
// overloading operator =
//-------------------------------------------------------------------
HTTPWriter& HTTPWriter::operator=(const HTTPWriter& http_writer)
{
    start_line_ = http_writer.start_line_;
    header_ = http_writer.header_;
    body_ = http_writer.body_;

    return *this;
}

//-------------------------------------------------------------------
// add information behind current start line
//-------------------------------------------------------------------
HTTPWriter& HTTPWriter::addStartLine(const std::string& start_line)
{
    start_line_ += start_line;
    return *this;
}

//-------------------------------------------------------------------
// add information behind current header
//-------------------------------------------------------------------
HTTPWriter& HTTPWriter::addHeader(const std::string& header)
{
    header_ += header;
    return *this;
}

//-------------------------------------------------------------------
// add information behind current body
//-------------------------------------------------------------------
HTTPWriter& HTTPWriter::addBody(const std::string& body)
{
    body_ += body;
    return *this;
}

//-------------------------------------------------------------------
// return information of current start line
//-------------------------------------------------------------------
std::string HTTPWriter::getStartLine() const
{
    return start_line_;
}

//-------------------------------------------------------------------
// return information of current header
//-------------------------------------------------------------------
std::string HTTPWriter::getHeader() const
{
    return header_;
}

//-------------------------------------------------------------------
// return information of current body
//-------------------------------------------------------------------
std::string HTTPWriter::getBody() const
{
    return body_;
}

//-------------------------------------------------------------------
// add version behind current start line
// Version represents HTTP version, such as HTTP/1.1
//-------------------------------------------------------------------
HTTPWriter& HTTPWriter::addVersion(const std::string& version)
{
    start_line_ += version + " ";
    return *this;
}

//-------------------------------------------------------------------
// add content type behind current header
// such as addContentType("text/plain")
//-------------------------------------------------------------------
HTTPWriter& HTTPWriter::addContentType(const std::string& content_type)
{
    header_ += "Content-Type: " + content_type + "\r\n";
    return *this;
}

//-------------------------------------------------------------------
// add content length behind current header
// such as addContentLength("10")
//-------------------------------------------------------------------
HTTPWriter& HTTPWriter::addContentLength(const std::string& content_length)
{
    header_ += "Content-Length: " + content_length + "\r\n";
    return *this;
}

//-------------------------------------------------------------------
// turn information of a HTTPWriter object into an HTTP message
//-------------------------------------------------------------------
void HTTPWriter::constructHTTPMsg(HTTPMessage &http_msg)
{
    // Before transforming, clear content in HTTP message
    // to avoid '\0' error.
    memset(&http_msg, '\0', sizeof(HTTPMessage)); 

    start_line_ += "\r\n";
    header_ += "\r\n";

    // If there is no content in body, body is not necessary, 
    // HTTP message should end with header and "\r\n".
    if (body_.length() > 0) 
        body_ += "\r\n";
    
    // In Visual Studio 2013, sprintf() cannot pass compilation.
#ifdef WINDOWS
    sprintf_s(http_msg.http_msg, "%s%s%s", start_line_.c_str(), header_.c_str(), body_.c_str());
#else
    sprintf(http_msg.http_msg, "%s%s%s", start_line_.c_str(), header_.c_str(), body_.c_str());
#endif
}

//-------------------------------------------------------------------
// turn information of a HTTPWriter object into a C++ string
//-------------------------------------------------------------------
void HTTPWriter::constructString(std::string &msg)
{
    start_line_ += "\r\n";
    header_ += "\r\n";
    if (body_.length() > 0) 
        body_ += "\r\n";
    
    msg = start_line_ + header_ + body_;
}

//-------------------------------------------------------------------
// clear information in a HTTPWriter object
//-------------------------------------------------------------------
HTTPWriter& HTTPWriter::clear()
{
    start_line_ = "";
    header_ = "";
    body_ = "";
    return *this;
}

//-------------------------------------------------------------------
// show content of an HTTP message
//-------------------------------------------------------------------
void HTTPWriter::showInfo(const HTTPMessage& http_msg)
{
    cout << http_msg.http_msg << std::endl;;
}


//-------------------------------------------------------------------
// Test case
//-------------------------------------------------------------------
#ifdef HTTP_WRITER_TEST

int main(int argc, char *argv[])
{
    HTTPWriter writer;
    writer.addStartLine("HTTP/1.1 200 OK\r\n");
    writer.addHeader("Content-Type: text/plain\r\n\r\n");
    writer.addBody("message\r\n");

    HTTPMessage msg;
    writer.constructHTTPMsg(msg);
    writer.showInfo(msg);

    system("pause");
    return 0;
}

#endif
