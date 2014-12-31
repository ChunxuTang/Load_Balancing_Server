#ifndef HTTP_READER_H
#define HTTP_READER_H
/////////////////////////////////////////////////////////////////////
//  HTTPReader.h - definitions of HTTP reader classes     
//  ver 1.0                                                        
//  Language:      standard C++ 11                              
//  Platform:      Win 8.1, 64-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////
/*
* File Description:
* ==================
* Define classes to parse an HTTP message and construct corresponding
* response message.
*
* Required Files:
* ===============
* Interface.h, HTTPBasic.h, HTTPWriter.h, HTTPWriter.cpp, 
* HTTPReader.h, HTTPReader.cpp, ResponseHandler.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 10 Jul 2014
* - first release
*/


#include "../HTTPWriter/HTTPBasic.h"
#include "../HTTPWriter/HTTPWriter.h"
#include "../../Common/Interface.h"
#include <queue>
#include <iostream>
#include <unordered_map>
#include <sstream>
#include <functional>


//-------------------------------------------------------------------
// Convert a string into int
// In this file, it is used to set max load of a real server
//-------------------------------------------------------------------
static int convertStringToInt(const std::string &s)
{
	int val;
	std::stringstream ss;
	ss << s;
	ss >> val;
	return val;
}

//-------------------------------------------------------------------
// Convert a generic type into string
//-------------------------------------------------------------------
template<typename T>
static std::string convertToString(const T val)
{
	std::string s;
	std::stringstream ss;
	ss << val;
	ss >> s;
	return s;
}


//***********************************************************************
// Method
//
// An enumeration type that defines different methods in a HTTP requests.
// These methods are relative to the different method writers in
// HTTPWriter.h
//***********************************************************************

enum Method { GET = 1, HEAD, PUT, POST, TRACE, OPTIONS, DELETE, SERVERCHECK, ERROR };


//***********************************************************************
// HTTPReader
//
// This class is used to read an HTTP message and partition it into three
// parts: start line, header and body. It can read start line word by,
// word, push the words into a start line queue. Additionally, it can
// read header line by line, and push them into a header queue. Then the
// class will invoke ResponseHandler to dispose the messages.
//***********************************************************************

class HTTPReader
{
public:
	using StringPos = std::string::size_type;

	// constructors and destructor
	HTTPReader(const HTTPMessage& http_msg);
	HTTPReader(const HTTPReader& http_reader);
	HTTPReader& operator=(const HTTPReader& http_reader);
	virtual ~HTTPReader();

	// set and get functions, operating on private data members
	void setRequestMsg(const HTTPMessage& http_msg);
	std::string getRequestMsg();
	std::string getStartLine() const;
	std::string getHeader() const;
	std::string getBody() const;
	HTTPMessage getResponseMsg() const;
	void setMaxLoad(const std::string& max_load);

	void start(); // function to control the whole procedure
private:
	void checkStartLine(); // partition start line word by word and push them 
						   // into start line queue
	void checkHeader();    // partition header line by line and push them into
						   // header queue
	void parseHTTPMsg();   // read start line queue and header queue, invoke
						   // a ResponseHandler to dispose the information

	std::string request_msg_;
	HTTPMessage response_msg_;
	std::string start_line_;
	std::string header_;
	std::string body_;
	std::string max_load_;
	std::queue<std::string> start_line_queue_;
	std::queue<std::string> header_queue_;
};


//***********************************************************************
// ResponseHandler
//
// The class to handle different information of an HTTP Message. An object
// of this class is invoked in HTTPReader's parseHTTPMsg() function.
// According to methods in Method enumeration, there are corresponding
// handlers to dispose requests and construct response messages.
//***********************************************************************

class ResponseHandler
{
public:
	typedef std::unordered_map<std::string, std::function<void(ResponseHandler*)>> HandlerTable;
	typedef std::unordered_map<std::string, HandlerTable&> HeaderHandlerTable;

	ResponseHandler(const std::string& url,
					const std::string& version,
					const std::queue<std::string>& header_queue);
	~ResponseHandler(){}

	// method handlers, used to handle different requests and construct responses
	HTTPMessage getResponse();
	HTTPMessage headResponse();
	HTTPMessage putResponse(std::string& body);
	HTTPMessage postResponse(std::string& body);
	HTTPMessage traceResponse(std::string& request_msg_);
	HTTPMessage optionsResponse();
	HTTPMessage deleteResponse();
	HTTPMessage serverCheckResponse(std::string& max_load);
	HTTPMessage errorResponse(std::string& error_code);
private:
	void initHeaderHandlerTable();
	void getHeaderInfo(); // get information after ':' in a line of header

	// invoke corresponding method handlers 
	int handleHeaders(const std::string& method); 
	
	// handle different headers, used in every method handler table
	static void handleHost(ResponseHandler*);
	static void handleAccept(ResponseHandler*);
	static void handleSourceIP(ResponseHandler*);
	static void handleSourcePort(ResponseHandler*);
	static void handleContentType(ResponseHandler*);
	static void handleContentLength(ResponseHandler*);

	// header handler table and different handler tables, used to 
	// store handler functions. When parsing a header, directly
	// access corresponding handler.
	HeaderHandlerTable header_handler_table_;
	HandlerTable get_handler_table_;
	HandlerTable head_handler_table_;
	HandlerTable put_handler_table_;
	HandlerTable post_handler_table_;
	HandlerTable trace_handler_table_;
	HandlerTable options_handler_table_;
	HandlerTable delete_handler_table_;
	HandlerTable server_check_handler_table_;
	HandlerTable error_handler_table_;

	// data members needed when construct an object 
	std::string url_;
	std::string version_;
	std::queue<std::string> header_queue_;

	// data members used in various handlers
	HTTPMessage http_msg_;
	std::string msg_;
	std::string error_code_;
	std::string header_;
	std::string content_;
	std::string target_ip_;
	std::string target_port_;
};



#endif
