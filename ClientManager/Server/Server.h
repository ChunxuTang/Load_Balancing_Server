#ifndef SERVER_H
#define SERVER_H
/////////////////////////////////////////////////////////////////////
//  Server.h - a simple test server
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
* Define operations to create a simple server to test functionality
* of client managers.
*
* Required Files:
* ===============
* Interface.h, Server.h, Server.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 7 Aug 2014
* - first release
*/

#include "../Public/SocketCreator.h"


//***********************************************************************
// Server
//
// A simple server which can bind to an IP address, and can be used to
// test functionality of client managers.
//***********************************************************************

class Server
{
public:
    void start();
private:
    static const char *PORT_NUM;
	static const char *BIND_ADDRESS;
    static const int BUF_SIZE = 1024;
	static const int BACKLOG = 100;
};


#endif
