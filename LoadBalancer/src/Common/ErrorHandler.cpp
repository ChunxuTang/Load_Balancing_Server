/////////////////////////////////////////////////////////////////////
//  ErrorHandler.cpp - implementation of ErrorHandler   
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../include/Common/ErrorHandler.h"

//-------------------------------------------------------------------
// Constructor
// Only initialize msg, and do not show information of file name,
// function name and line number.
//-------------------------------------------------------------------
ErrorHandler::ErrorHandler(const char *msg)
:msg_(msg)
{
	file_name_ = NULL;
	func_name_ = NULL;
	line_num_ = 0;
}

//-------------------------------------------------------------------
// Constructor
// Initialize error message, file name, function name and line number,
// provide with the most information of an error. This constructor is
// the most often used in 2014 Summer Project.
//-------------------------------------------------------------------
ErrorHandler::ErrorHandler(const char *msg, 
						   const char *file_name, 
						   const char *func_name, 
						   int line_num)
:msg_(msg), 
file_name_(file_name), 
func_name_(func_name), 
line_num_(line_num)
{}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
ErrorHandler::~ErrorHandler()
{
	msg_ = NULL;
	file_name_ = NULL;
	func_name_ = NULL;
	line_num_ = 0;
}

//-------------------------------------------------------------------
// Show error message by invoke showMsg() function
//-------------------------------------------------------------------
void ErrorHandler::errMsg()
{
	showMsg();
}

//-------------------------------------------------------------------
// Show error message and exit the program
//-------------------------------------------------------------------
void ErrorHandler::errExit()
{
	showMsg();
	exit(EXIT_FAILURE);
}

//-------------------------------------------------------------------
// Show error message
//-------------------------------------------------------------------
void ErrorHandler::showMsg()
{
	int saved_errno;
	saved_errno = errno;

	std::cout << msg_ << ": ";
	std::cout << strerror(errno) << std::endl;

	if (file_name_ != NULL)
		std::cout << basename((char*)file_name_) << " ";
	if (func_name_ != NULL)
		std::cout << func_name_ << " ";
	if (line_num_ != 0)
		std::cout << line_num_;

	std::cout << "\n";

	errno = saved_errno;
}


#ifdef ERROR_HANDLER_TEST

int main(int argc, char *argv[])
{
	errno = 3;
	ErrorHandler eh("test", __FILE__, __FUNCTION__, __LINE__);
	eh.errMsg();

	ErrorHandler eh2("test2");
	eh2.errExit();

	return 0;

}
#endif
