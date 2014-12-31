#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H
/////////////////////////////////////////////////////////////////////
//  ClientManager.h - definitions of a multi-process client manager 
//  ver 1.0                                                        
//  Language:      standard C++ 11                                
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////
/*
* File Description:
* ==================
* This file defines operations of a multi-process client manager. It can 
* fork many children to send requests to a server.
*
* Required Files:
* ===============
* Interface.h, ErrorHandle.h, ErrorHandler.cpp, SocketCreator.h,
* SocketCreator.cpp, GetCurrTime.h, GetCurrTime.cpp, ClientManager.h, 
* ClientManager.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 5 Aug 2014
* - first release
*/

#include "../Public/SocketCreator.h"
#include "../Public/GetCurrTime.h"
#include <sys/time.h>
#include <sys/wait.h>


//***********************************************************************
// ClientManager
//
// A ClientManager object can fork many children, and each child can send
// a request to a server. The request is a number, and the server
// will sleep for that number seconds. 
//***********************************************************************

class ClientManager
{
public:
	ClientManager(int client_count, char *host, char *service);
	~ClientManager();
	void start();
	void childWork();
private:
	int client_count_;
	char *host_;
	char *service_;
	static const int BUF_SIZE = 1024;
};

#endif
