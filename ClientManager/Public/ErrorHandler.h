#ifndef ERROR_HANDLER_H
#define ERROR_HANDLER_H
/////////////////////////////////////////////////////////////////////
//  ErrorHandle.h - define error handlers.
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////
/*
* File Description:
* ==================
* Define error handlers to show error information and handle errors.
*
* Required Files:
* ===============
* Interface.h, ErrorHandler.h, ErrorHandler.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 25 May 2014
* - first release
*/

#include "Interface.h"


//***********************************************************************
// ErrorHandler
//
// This class defines operations to get an error information, display it,
// and determine whether need to exit or not.
//***********************************************************************

class ErrorHandler
{
public:
	ErrorHandler(const char *msg);
	ErrorHandler(const char *msg, 
			     const char *file_name, 
				 const char *func_name, 
				 int line_num);
	~ErrorHandler();
	void errMsg();
	void errExit();
private:
	void showMsg();
	const char *msg_;
	const char *file_name_;
	const char *func_name_;
	int line_num_;
};



#endif