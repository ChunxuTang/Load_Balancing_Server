#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H
////////////////////////////////////////////////////////////////////
//  ClientManager.h - definitions of a multi-thread client manager 
//                    without locks
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
* This file defines operations of a client manager, which can create
* many threads to send requests to servers.
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
#include <pthread.h>
#include <list>
#include <signal.h>


//***********************************************************************
// ServerInfo
//
// Store information of a server's IP address and port number. The struct 
// is used to send server's information to a thread.
//***********************************************************************

typedef struct serverInfo
{
    char host[1024];
    char service[1024];
} serverInfo;


//***********************************************************************
// ClientManager
//
// A ClientManager object can produces many threads, and each thread can
// send a request to the server. There is no locks between threads.
//***********************************************************************

class ClientManager
{
public:
	ClientManager(int client_count, char *host, char *service);
    ~ClientManager();
    void start();
private:
    void createThread();
    static void handleInterrupt(int sig);
    static void* createClient(void *arg);

    int client_count_;
    char *host_;
    char *service_;
    
    static int client_exist_;
	static const int BUF_SIZE = 1024;
};

#endif
