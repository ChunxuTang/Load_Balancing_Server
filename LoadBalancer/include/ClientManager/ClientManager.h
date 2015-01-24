#ifndef CLIENT_MANAGER_H
#define CLIENT_MANAGER_H
/////////////////////////////////////////////////////////////////////
//  ClientManager.h - definitions of a client manager which can 
//                    send different kinds of requests
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
* This file defines operations of ClientManager. This class can produce
* many threads, each send one kind of request to a load balancer.
*
* Required Files:
* ===============
* Interface.h, ErrorHandle.h, ErrorHandler.cpp, SocketCreator.h,
* SocketCreator.cpp, HTTPBasic.h, HTTPWriter.cpp, ResponseMessage.cpp,
* RequestMessage.cpp, HTTPReader.h, HTTPReader.cpp, ResponseHandler.cpp,
* GetCurrTime.h, GetCurrTime.cpp, Cache.h, ClientManager.h, ClientManager.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 5 Aug 2014
* - first release
*/


#include <pthread.h>
#include <list>
#include <signal.h>
#include <sys/time.h>
#include <unordered_map>
#include <memory>

#include "../Common/GetCurrTime.h"
#include "../Common/SocketCreator.h"
#include "../HTTP/HTTPReader/HTTPReader.h"
#include "Cache.h"


//***********************************************************************
// ClientManager
//
// A ClientManager object can produces many threads, and each thread can
// send one kind of request with one method from { GET, HEAD, PUT, POST,
// TRACE, OPTIONS, TRACE, DELETE }. The functions to construct different
// kinds of requests are store in a hash table.
//***********************************************************************

class ClientManager
{
public:
    typedef std::unordered_map<int, std::function<void(ClientManager*)>> RequestMap;

    ClientManager(int client_count, char *host, char *service);
    ~ClientManager();
    void start();  // Entry point of ClientManager

    static LRUCache<int, HTTPMessage> *request_cache_;
private:
    void initRequestMap();
    static void sendGetRequest(ClientManager *cm);
    static void sendHeadRequest(ClientManager *cm);
    static void sendPutRequest(ClientManager *cm);
    static void sendPostRequest(ClientManager *cm);
    static void sendTraceRequest(ClientManager *cm);
    static void sendOptionsRequest(ClientManager *cm);
    static void sendDeleteRequest(ClientManager *cm);
    static void handleInterrupt(int sig);

    void createThread(); // create a detached thread
    static void* createClient(void *arg); // a client main function
    static void listCache(); // list cache information

    static int client_exist_;         // the number of children hasn't finished
    static std::list<int> sock_list_; // list to store client fds
    static pthread_mutex_t mtx_;      // mutex
    static int cache_hit_count_;       // times of cache hit
    
    int client_count_;         // the total of clients needed created
    char host_[NI_MAXHOST];    // server's IP address 
    char service_[NI_MAXSERV]; // server's port number
    std::string port_num_;     // a client's port number
    RequestMap request_map_;   // request functions hash table
    HTTPMessage send_msg_;     // HTTP message to be sent
    
};


#endif
