#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H
/////////////////////////////////////////////////////////////////////
//  ClientManager.h - definitions of a client manager which can 
//                    send different kinds of requests
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
* This file defines operations of ClientManager. This class can produce
* many threads, each send one kind of request to a load balancer.
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


#include "../Common/SocketCreator.h"
#include "GetCurrTime.h"
#include <pthread.h>
#include <list>
#include <signal.h>


//***********************************************************************
// ClientManager
//
// A ClientManager object can produces many threads, and each thread can
// send a request to the server. The request is a number, and the server
// will sleep for that number seconds. 
//***********************************************************************

class ClientManager
{
public:
    ClientManager(int client_count, char *host, char *service);
    ~ClientManager();
    void start(); // Entry point of ClientManager
private:
    void createThread();
    static void handleInterrupt(int sig);
    static void* createClient(void *arg);

    int client_count_;         // the total of clients needed created
    char host_[NI_MAXHOST];    // server's IP address 
    char service_[NI_MAXSERV]; // server's port number

    static int client_exist_;         // the number of children hasn't finished
    static std::list<int> sock_list_; // list to store client fds
    static pthread_mutex_t mtx_;      // mutex

    static const int BUF_SIZE = 1024;
};

#endif
