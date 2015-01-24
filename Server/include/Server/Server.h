#ifndef SERVER_H
#define SERVER_H
/////////////////////////////////////////////////////////////////////
//  Server.h - define operations of a server to handle requests.
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
* This file defines operations of a server. The server can pre-forks
* a definite number of children to handle requests from clients. If 
* the simultaneous requests are more than pre-forked children, the 
* server can fork some more children to handle requests. While, the 
* total number cannot be larger than max children provided by user.
*
* Required Files:
* ===============
* Interface.h, ErrorHandle.h, ErrorHandler.cpp, SocketCreator.h,
* SocketCreator.cpp, FdTransfer.h, FdTransfer.cpp, FdHandler.h, 
* FdHandler.cpp, Server.h, Server.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 9 Aug 2014
* - first release
*/


#include <sys/epoll.h>
#include <sys/timerfd.h> 
#include <sys/select.h> 
#include <signal.h>
#include <sys/signalfd.h> 
#include <sys/wait.h>
#include <iomanip>
#include <list>
#include <vector>

#include "../Common/SocketCreator.h"
#include "FdTransfer.h"
#include "FdHandler.h"


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
// This class defines operations of a server. THe server can pre-fork some
// children to handle requests from clients. If the number of requests is
// larger than pre-forked amount of children, the server could fork some
// more children, no more than max children controlled by user. And each
// new forked child has a timer. If it's time out, the child will be deleted
// by the server. For a child, after it finishes a request, it has a chance 
// to terminate unexpectedly. If it is a pre-forked child, the server should 
// replace it with a new one. It it is a temporary child, the server just
// remove the information of this child.
//***********************************************************************

class Server
{
public:
    using VectorPos = std::vector<ChildInfo>::size_type;
    using VectorSize = std::vector<ChildInfo>::size_type;

    Server(int maxChildren);
    ~Server();

    // Forbid default copy constructor and assignment operator overload
    Server(const Server& server) = delete;
    Server operator=(const Server& server) = delete;

    void start();  // Entry point of a server
   
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
    int epoll_fd_;          // epoll fd to monitor other fds
    int signal_fd_;         // signal fd to receive signals
    fd_set child_fds_;      // fd set of child spipes
    fd_set timer_fds_;      // fd set of child timers
    bool server_stop_;      // decide whether a server should exit or not
    struct itimerspec ts_;  // time for a timer to alarm
    static int child_pfd_;  // used for childSigHandler to remove fd
    std::vector<ChildInfo> child_pool_; // vector to store children's information
    
    static const char *BIND_ADDRESS;
    static const char *PORT_NUM;
    static const int BUF_SIZE = 1024;
    static const int BACKLOG = 50;
    static const int MAX_EVENTS = 10; 
    static const int PREFORKED_CHILDREN = 5;
    static const int TEMPORARY_CHILD_TIME_OUT = 30;
    static const int CHILD_EXIT_PROBABILITY = 50;
};


#endif
