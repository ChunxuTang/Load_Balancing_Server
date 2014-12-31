/////////////////////////////////////////////////////////////////////
//  HTTPReader.cpp - implementation of HTTP reader    
//  ver 1.0                                                        
//  Language:      standard C++ 11                               
//  Platform:      Win 8.1, 64-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../../include/HTTP/HTTPReader/HTTPReader.h"


//-------------------------------------------------------------------
// Constructor
// Initialize request HTTP message which is needed to be read and
// analysis.
//-------------------------------------------------------------------
HTTPReader::HTTPReader(const HTTPMessage& http_msg)
	: request_msg_(http_msg.http_msg) 
{}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
HTTPReader::~HTTPReader()
{}

//-------------------------------------------------------------------
// Copy Constructor
//-------------------------------------------------------------------
HTTPReader::HTTPReader(const HTTPReader& http_reader)
	: request_msg_(http_reader.request_msg_), 
	  response_msg_(http_reader.response_msg_),
	  start_line_(http_reader.start_line_),
	  header_(http_reader.header_),
	  body_(http_reader.body_),
	  max_load_(http_reader.max_load_),
	  start_line_queue_(http_reader.start_line_queue_),
	  header_queue_(http_reader.header_queue_)
{}

//-------------------------------------------------------------------
// overloading operator =
//-------------------------------------------------------------------
HTTPReader& HTTPReader::operator=(const HTTPReader& http_reader)
{
	request_msg_ = http_reader.request_msg_;
	response_msg_ = http_reader.response_msg_;
	start_line_ = http_reader.start_line_;
	header_ = http_reader.header_;
	body_ = http_reader.body_;
	max_load_ = http_reader.max_load_;
	start_line_queue_ = http_reader.start_line_queue_;
	header_queue_ = http_reader.header_queue_;

	return *this;
}

//-------------------------------------------------------------------
// Set request HTTP message
// Can be used to reset a new HTTP message and read again
//-------------------------------------------------------------------
void HTTPReader::setRequestMsg(const HTTPMessage& http_msg) 
{
	request_msg_ = http_msg.http_msg; 
}

//-------------------------------------------------------------------
// Get request HTTP message
//-------------------------------------------------------------------
std::string HTTPReader::getRequestMsg() 
{ 
	return request_msg_; 
}

//-------------------------------------------------------------------
// Get start line information
//-------------------------------------------------------------------
std::string HTTPReader::getStartLine() const
{
	return start_line_;
}

//-------------------------------------------------------------------
// Get header information
//-------------------------------------------------------------------
std::string HTTPReader::getHeader() const
{
	return header_;
}

//-------------------------------------------------------------------
// Get body information
//-------------------------------------------------------------------
std::string HTTPReader::getBody() const
{
	return body_;
}

//-------------------------------------------------------------------
// Get response HTTP message
// The return type is HTTPMessage, which is different from request
// message (std::string), meaning the class can directly construct
// an HTTP response message.
//-------------------------------------------------------------------
HTTPMessage HTTPReader::getResponseMsg() const 
{ 
	return response_msg_; 
}

//-------------------------------------------------------------------
// Set max load of a server
// This function is specifically used in handling SERVERCHECK method 
// requests.
//-------------------------------------------------------------------
void HTTPReader::setMaxLoad(const std::string& max_load) 
{ 
	max_load_ = max_load; 
}

//-------------------------------------------------------------------
// Entry point of reading and parsing procedure
//-------------------------------------------------------------------
void HTTPReader::start()
{
	if (request_msg_.size() <= 0)
		return;

	StringPos index = 0;

	// get start line of an HTTP message, construct start line queue by
	// pushing every words into the queue
	for (StringPos i = 0; i <= request_msg_.size() - 2; i++)
	{
		// a start line ends with "\r\n"
		if (request_msg_.at(i) == '\r' && request_msg_.at(i + 1) == '\n') 
		{
			start_line_ = request_msg_.substr(index, i - index);
			index = i + 2;
			checkStartLine();
			break;
		}
	}
	if (start_line_.size() <= 0)
		return;

	// get header of an HTTP message, construct header queue by
	// pushing every line into the queue
	for (StringPos i = index; i <= request_msg_.size() - 4; i++)
	{
		// header ends with "\r\n\r\n"
		if (request_msg_.at(i) == '\r' && request_msg_.at(i + 1) == '\n' &&
			request_msg_.at(i + 2) == '\r' && request_msg_.at(i + 3) == '\n') 
		{
			// In this way, there should be a "\r\n" in the end of header. 
			// If it is not necessary, substr(index, i - index).
			header_ = request_msg_.substr(index, i - index + 2); 
			index = i + 4;
			checkHeader();
			break;
		}
	}
	if (header_.size() <= 0)
		return;

	// get body of an HTTP message
	for (StringPos i = index; i <= request_msg_.size() - 2; i++)
	{
		// body ends with "\r\n"
		if (request_msg_.at(i) == '\r' && request_msg_.at(i + 1) == '\n')
		{
			body_ = request_msg_.substr(index, i - index);
			break;
		}
	}
	
	parseHTTPMsg();
}

//-------------------------------------------------------------------
// Partition whole start line into words, marked by " ", and push
// them into start line queue.
//-------------------------------------------------------------------
void HTTPReader::checkStartLine()
{
	StringPos index = 0;
	for (StringPos i = 0; i < start_line_.size(); i++)
	{
		if (start_line_.at(i) == ' ')
		{
			std::string str;
			str = start_line_.substr(index, i - index);
			start_line_queue_.push(str);
			index = i + 1;
		}
	}

	// to avoid that there is not a whitespace in the end of start line
	if (index < start_line_.size())
		start_line_queue_.push(start_line_.substr(index, start_line_.size() - index));
}

//-------------------------------------------------------------------
// Partition whole header into lines, marked by "\r\n", and push
// them into header queue.
//-------------------------------------------------------------------
void HTTPReader::checkHeader()
{
	StringPos index = 0;
	for (StringPos i = 0; i < header_.size() - 1; i++)
	{
		if (header_.at(i) == '\r' && header_.at(i + 1) == '\n') {
			header_queue_.push(header_.substr(index, i - index));
			index = i + 2;
		}
	}
	// to avoid that there is not a whitespace in the end of header
	if (index < header_.size())
		header_queue_.push(header_.substr(index, header_.size() - index));

}

//-------------------------------------------------------------------
// Parse start line queue and header queue, invoke corresponding
// method handlers to dispose requests.
// This function uses a FSM (Finite State Machine). There are three
// states: PARSE_START_LINE, PARSE_HEADER and FINISH, which are defined
// in CheckState enumeration.
//-------------------------------------------------------------------
void HTTPReader::parseHTTPMsg()
{
	enum CheckState { PARSE_START_LINE, PARSE_HEADER, FINISH };
	CheckState check_state = PARSE_START_LINE; 

	// method_map is used to transform std::string into Method type
	std::unordered_map<std::string, Method> method_map;
	method_map.insert(std::pair<std::string, Method>("GET", GET));
	method_map.insert(std::pair<std::string, Method>("HEAD", HEAD));
	method_map.insert(std::pair<std::string, Method>("PUT", PUT));
	method_map.insert(std::pair<std::string, Method>("POST", POST));
	method_map.insert(std::pair<std::string, Method>("TRACE", TRACE));
	method_map.insert(std::pair<std::string, Method>("OPTIONS", OPTIONS));
	method_map.insert(std::pair<std::string, Method>("DELETE", DELETE));
	method_map.insert(std::pair<std::string, Method>("SERVERCHECK", SERVERCHECK));
	method_map.insert(std::pair<std::string, Method>("ERROR", ERROR));

	std::string method;
	std::string url;
	std::string version;
	std::string error_code;
	bool finish = false;

	while (!finish)
	{
		switch (check_state)
		{
		case PARSE_START_LINE: 

			// Get basic information of a start line
			method = start_line_queue_.front();
			start_line_queue_.pop();
			url = start_line_queue_.front();
			start_line_queue_.pop();
			version = start_line_queue_.front();
			start_line_queue_.pop();
			if (version != "HTTP/1.1") 
			{
				// HEAD505 = "505 HTTP Version Not Supported"
				error_code = StatusCode::ServerErrorStatusCode::HEAD505;
				method = "ERROR";
			}

			// After PARSE_START_LINE, next state is PARSE_HEADER
			check_state = PARSE_HEADER;
			break;
		case PARSE_HEADER:
		{
			// According to method in the request, invoke corresponding
			// method handler
			ResponseHandler response_handler(url, version, header_queue_);
			switch (method_map[method])
			{
			case GET:
				response_msg_ = response_handler.getResponse();
				break;
			case HEAD:
				response_msg_ = response_handler.headResponse();
				break;
			case PUT:
				response_msg_ = response_handler.putResponse(body_);
				break;
			case POST:
				response_msg_ = response_handler.postResponse(body_);
				break;
			case TRACE:
				response_msg_ = response_handler.traceResponse(request_msg_);
				break;
			case OPTIONS:
				response_msg_ = response_handler.optionsResponse();
				break;
			case DELETE:
				response_msg_ = response_handler.deleteResponse();
				break;
			case SERVERCHECK:
				response_msg_ = response_handler.serverCheckResponse(max_load_);
				break;
			case ERROR:
				response_msg_ = response_handler.errorResponse(error_code);
				break;
			default:
				std::cout << "Unknown Method: " << method << std::endl;

				// HEAD405 = "405 Method Not Allowed"
				error_code = StatusCode::ClientErrorStatusCode::HEAD405;
				response_msg_ = response_handler.errorResponse(error_code);
				break;
			}
			finish = true;
			break;
		}
		default:
			check_state = PARSE_START_LINE;
			break;
		}
	}
}



//-------------------------------------------------------------------
// Test stub
//-------------------------------------------------------------------
#ifdef HTTP_READER_TEST

int main()
{

	HTTPMessage msg = { "GETPUT ./file.txt HTTP/1.1 \r\nHost: localhost\r\nAccept: *\r\nSource-IP: 127.0.0.1\r\nSource-Port: 50000\r\n\r\n\r\n" };
	HTTPReader reader(msg);
	reader.start();
	std::cout << "response:--- \n" << reader.getResponseMsg().http_msg << std::endl;

	std::cout << "\nGET method =================\n";
	HTTPMessage http_msg;
	GetMethodWriter gmw("./file.txt", "HTTP/1.1", "localhost", "*", "127.0.0.1", "50000");
	gmw.constructHTTPMsg(http_msg);
	HTTPReader reader1(http_msg);
	reader1.start();
	std::cout << "response:--- \n" << reader1.getResponseMsg().http_msg << std::endl;

	std::cout << "\nHEAD method =================\n";
	HeadMethodWriter hmw("./file.txt", "HTTP/1.1", "localhost", "*", "127.0.0.1", "50000");
	hmw.constructHTTPMsg(http_msg);
	HTTPReader reader2(http_msg);
	reader2.start();
	std::cout << "response:--- \n" << reader2.getResponseMsg().http_msg << std::endl;

	std::cout << "\nPUT method =================\n";
	PutMethodWriter pmw("./file.txt", "HTTP/1.1", "localhost", "text/plain", "40", "127.0.0.1", "50000");
	pmw.addBody("I'm a message.");
	pmw.constructHTTPMsg(http_msg);
	HTTPReader reader3(http_msg);
	reader3.start();
	std::cout << "response:--- \n" << reader3.getResponseMsg().http_msg << std::endl;

	std::cout << "\nPOST method =================\n";
	PostMethodWriter pomw("./file.txt", "HTTP/1.1", "localhost", "text/plain", "10", "127.0.0.1", "50000");
	pomw.addBody("color=red");
	pomw.constructHTTPMsg(http_msg);
	HTTPReader reader4(http_msg);
	reader4.start();
	std::cout << "response:--- \n" << reader4.getResponseMsg().http_msg << std::endl;

	std::cout << "\nTRACE method =================\n";
	TraceMethodWriter tmw("./file.txt", "HTTP/1.1", "localhost", "*", "127.0.0.1", "50000");
	tmw.addBody("I'm a message.");
	tmw.constructHTTPMsg(http_msg);
	HTTPReader reader5(http_msg);
	reader5.start();
	std::cout << "response:--- \n" << reader5.getResponseMsg().http_msg << std::endl;

	std::cout << "\nOPTIONS method =================\n";
	OptionsMethodWriter omw("./file.txt", "HTTP/1.1", "localhost", "*", "127.0.0.1", "50000");
	omw.constructHTTPMsg(http_msg);
	HTTPReader reader6(http_msg);
	reader6.start();
	std::cout << "response:--- \n" << reader6.getResponseMsg().http_msg << std::endl;

	std::cout << "\nDELETE method =================\n";
	DeleteMethodWriter dmw("./file.txt", "HTTP/1.1", "localhost", "127.0.0.1", "50000");
	dmw.constructHTTPMsg(http_msg);
	HTTPReader reader7(http_msg);
	reader7.start();
	std::cout << "response:--- \n" << reader7.getResponseMsg().http_msg << std::endl;

	std::cout << "\nSERVERCHECK method =================\n";
	ServerCheckMethodWriter scmw("./file.txt", "HTTP/1.1", "localhost", "127.0.0.1", "50000");
	scmw.constructHTTPMsg(http_msg);
	HTTPReader reader8(http_msg);
	reader8.setMaxLoad("10"); 
	reader8.start();
	std::cout << "response:--- \n" << reader8.getResponseMsg().http_msg << std::endl;

	system("pause");
	return 0;

}

#endif
