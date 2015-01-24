/////////////////////////////////////////////////////////////////////
//  ResponseMessage.cpp - implementations of response message writer          
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Win 8.1, 64-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../../include/HTTP/HTTPWriter/HTTPWriter.h"
#include <iostream>

//-------------------------------------------------------------------
// Constructor
//-------------------------------------------------------------------
ResponseMessage::ResponseMessage(const std::string& version,
                                 const std::string& status_code)
{
    start_line_ = version + " " + status_code + " ";
}

//-------------------------------------------------------------------
// Constructor 
// Include arguments of target-ip and target_port, which are used
// between Load Balancer and real server to determine target client.
//-------------------------------------------------------------------
ResponseMessage::ResponseMessage(const std::string& version, 
                                 const std::string& status_code,
                                 const std::string& target_ip, 
                                 const std::string& target_port)
{
    start_line_ = version + " " + status_code + " ";
    header_ = "Target-IP: " + target_ip + "\r\n" + 
              "Target-Port: " + target_port + "\r\n";
}

//-------------------------------------------------------------------
// Constructor
// Includes many arguments and can construct response message's header 
// of all methods.
//-------------------------------------------------------------------
ResponseMessage::ResponseMessage(const std::string& version, 
                                 const std::string& status_code,
                                 const std::string& content_type, 
                                 const std::string& content_length,
                                 const std::string& allow_method, 
                                 const std::string& location,
                                 const std::string& target_ip, 
                                 const std::string& target_port)
{
    start_line_ = version + " " + status_code + " ";

    if (location.size() > 0)
        header_ += "Location: " + location + "\r\n";

    if (allow_method.size() > 0)
        header_ += "Allow: " + allow_method + "\r\n";

    if (content_type.size() > 0)
        addContentType(content_type);
    if (content_length.size() > 0)
        addContentLength(content_length);

    addTargetIP(target_ip);
    addTargetPort(target_port);

}

//-------------------------------------------------------------------
// Add status code behind current start line
// such as addStatusCode("200 OK")
// status code = status + reason phrase
//-------------------------------------------------------------------
HTTPWriter& ResponseMessage::addStatusCode(const std::string& status_code)
{
    start_line_ += status_code + " ";
    return *this;
}

//-------------------------------------------------------------------
// Add status behind current start line
// such as addStatus("200")
//-------------------------------------------------------------------
HTTPWriter& ResponseMessage::addStatus(const std::string& status)
{
    start_line_ += status + " ";
    return *this;
}

//-------------------------------------------------------------------
// Add reason phrase behind current start line
// such as addReasonPhrase("Internal Server Error")
//-------------------------------------------------------------------
HTTPWriter& ResponseMessage::addReasonPhrase(const std::string& reason_phrase)
{
    start_line_ += reason_phrase + " ";
    return *this;
}

//-------------------------------------------------------------------
// Add location behind start line, which is used to illustrate the 
// location of a file.
// such as addLocation("./put.txt")
//-------------------------------------------------------------------
HTTPWriter& ResponseMessage::addLocation(const std::string& location)
{
    header_ += "Location: " + location + "\r\n";
    return *this;
}

//-------------------------------------------------------------------
// Add allow methods behind current start line
// such as addAllow("GET PUT POST OPTIONS")
//-------------------------------------------------------------------
HTTPWriter& ResponseMessage::addAllow(const std::string& allow_method)
{
    header_ += "Allow: " + allow_method + "\r\n";
    return *this;
}

//-------------------------------------------------------------------
// Add target IP behind current start line
// such as addTargetIP("127.0.0.1")
//-------------------------------------------------------------------
HTTPWriter& ResponseMessage::addTargetIP(const std::string& target_ip)
{
    header_ += "Target-IP: " + target_ip + "\r\n";
    return *this;
}

//-------------------------------------------------------------------
// Add target port behind current start line
// such as addTargetPort("8080")
//-------------------------------------------------------------------
HTTPWriter& ResponseMessage::addTargetPort(const std::string& target_port)
{
    header_ += "Target-Port: " + target_port + "\r\n";
    return *this;
}


//-------------------------------------------------------------------
// Test case
//-------------------------------------------------------------------
#ifdef RESPONSE_MESSAGE_TEST

using namespace StatusCode;
using namespace SuccessStatusCode;

int main(int argc, char *argv[])
{
    ResponseMessage rm("HTTP/1.1", HEAD200, "text/plain", "54", "OPTION", "./new.txt", "127.0.0.2", "50000");
    rm.addBody("I'm a message");

    HTTPMessage http_msg;
    rm.constructHTTPMsg(http_msg);
    rm.showInfo(http_msg);

    system("pause");
    return 0;
}

#endif