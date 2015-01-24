#ifndef SERVER_H
#define SERVER_H
/////////////////////////////////////////////////////////////////////
//  Server.h - define operations of a server to handle requests.
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
* This file defines operations of a server. The server can pre-forks
* a definite number of children to handle requests from the load
* balancer. If there are many requests, the server can fork some
* more children to handle requests. While, the total number cannot
* be more than max children provided by user.
*
* Required Files:
* ===============
* Interface.h, ErrorHandle.h, ErrorHandler.cpp, SocketCreator.h,
* SocketCreator.cpp, HTTPBasic.h, HTTPWriter.cpp, ResponseMessage.cpp,
* RequestMessage.cpp, HTTPReader.h, HTTPReader.cpp, ResponseHandler.cpp,
* FdHandler.h, Server.h, Server.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 3 Aug 2014
* - first release
*/

#include <sys/epoll.h>
#include <sys/timerfd.h> // timerfd_create()
#include <sys/select.h> // fd_set
#include <signal.h>
#include <sys/signalfd.h> // signalfd
#include <sys/wait.h>
#include <iomanip>
#include <vector>
#include <algorithm>

#include "../Common/SocketCreator.h"
#include "../Common/FdHandler.h"
#include "../HTTP/HTTPReader/HTTPReader.h"


//-------------------------------------------------------------------
// One line of a header is like this:
// Content-Type: text/plain
// (target)       (content)
// This function is used to get content after symbol ':' according
// to target string.
//-------------------------------------------------------------------
static void getHeaderInfo(const HTTPMessage& msg,
    std::string& content,
    const std::string& target)
{
    std::string received_msg = msg.http_msg;
    decltype(target.size()) found = received_msg.find(target);
    decltype(target.size()) index;

    if (found != std::string::npos)
    {
        index = found + target.size();
        for (decltype(index) i = index; i < received_msg.size() - 1; i++)
        {
            if (received_msg.at(i) == '\r' && received_msg.at(i + 1) == '\n')
            {
                content = received_msg.substr(index, i - index);
                break;
            }
        }
    }
}

//-------------------------------------------------------------------
// Get source IP address by finding the content after "Source-IP: ".
// This function is used to find source client IP address and construct
// error response message in a server.
//-------------------------------------------------------------------
static void getSourceIP(const HTTPMessage& msg, std::string& source_ip)
{
    getHeaderInfo(msg, source_ip, "Source-IP: ");
}

//-------------------------------------------------------------------
// Get target port number by finding the content after "Target-Port: ".
// This function is used to find source client port number and construct
// error response message in a server.
//-------------------------------------------------------------------
static void getSourcetPort(const HTTPMessage& msg, std::string& source_port)
{
    getHeaderInfo(msg, source_port, "Source-Port: ");
}


//***********************************************************************
// Status
//
// This enumeration type defines return type of a function.
//***********************************************************************

enum Status {
    SUCCESS = 0,     // function succeed
    MINOR_ERROR = 1, // occur a minor error, show error message
    FATAL_ERROR = -1 // occur a fatal error, show error message and exit
};


//***********************************************************************
// ChildStatus
//
// This enumeration type defines status of a child, which are used
// in the server to store children's status.
// FREE: The child is not working.
// BUSY: The child is handling a request.
//***********************************************************************

enum ChildStatus { FREE = 0, BUSY = 1 };


//***********************************************************************
// ChildInfo
//
// The struct defines information of a child process. This is used in a 
// vector of the server to store children's information.
//***********************************************************************

struct ChildInfo
{
    int child_pid;            // child's pid
    ChildStatus child_status; // status of the child, FREE or BUSY
    int child_index;          // child's index, representing index in a vector
    int child_spipe_fd[2];    // the stream pipe between parent and child
                              // child uses child_spipe_fd[0]
    
    int child_timer_fd;       // timerfd of a child, only effective for 
                              // children forked when there are too many
                              // requests
};


//***********************************************************************
// Server
//
// This class defines operations of a server. At first, the server pre-forks
// a definite number of children and store their information in a child
// pool. When the server receives a request, it will send this to the first
// available child. If there are too many request, and there is no available
// child, the server can fork some more children no less than PREFORKED_CHILDREN.
// For there newly forked children, every one will have a timer. If the timer
// is time out, corresponding child will be removed.
//***********************************************************************

class Server
{
public:
    using VectorPos = std::vector<ChildInfo>::size_type;
    using VectorSize = std::vector<ChildInfo>::size_type;

    Server(int maxChildren, const char *host);
    ~Server();

    // Forbid default copy constructor and assignment operator overload
    Server(const Server& server) = delete;
    Server operator=(const Server& server) = delete;

    void start(); // Entry point of a server

    Status handleRequestFromClient(); 
    Status handleResponseFromChild(int trigger_fd);
    Status handleChildTimeOut(int trigger_fd);
    Status serverSigHandler();
    Status addTimer(int index);
    Status forkChild(int index);
    Status updateChild(int index); 
private:
    Status initEpollfd();
    Status initListenfd();
    Status initClientfd();
    Status initSignalfd();

    void childWork(ChildInfo &child_info);
    static void childSigHandler(int sig);

    void clearAll();
    void listChildrenAvail();
    void listChildren();

    int max_children_;      // max children a server can fork
    int children_exist_;    // number of children forked
    int children_free_;     // number of children whose status is FREE
    int listen_fd_;         // socket fd to listen to load balancer
    int client_fd_;         // socket fd to communicate with load balancer
    int epoll_fd_;          // epoll fd to monitor other fds
    int signal_fd_;         // signal fd to receive signals
    int timer_fd_;          // timer fd to receive timer alarms
    fd_set child_fds_;      // fd set of child spipes
    fd_set timer_fds_;      // fd set of child timers
    struct itimerspec ts_;  // time for a timer to alarm
    bool server_stop_;      // decide whether a server should exit or not
    char host_[NI_MAXHOST]; // server's IP address
    static int child_pfd_;  // used for childSigHandler to remove fd
    std::vector<ChildInfo> child_pool_; // vector to store children's information

    static const char *PORT_NUM;
    static const int BACKLOG = 50;
    static const int MAX_EVENTS = 10;  
    static const int PREFORKED_CHILDREN = 5;
    static const int TEMPORARY_CHILD_TIME_OUT = 20;
};


#endif
