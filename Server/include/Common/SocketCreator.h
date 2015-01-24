#ifndef SOCKET_CREATOR_H
#define SOCKET_CREATOR_H
/////////////////////////////////////////////////////////////////////
//  SocketCreator.h - define class to create a socket
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
* Define operations to create a passive or an active socket. This code
* is reference to "The Linux Programming Interface: A Linux and Unix 
* System Programming Handbook", Michael Kerrisk, No Starch Press, 2010.
*
* Required Files:
* ===============
* Interface.h, ErrorHandler.h, ErrorHandler.cpp,
* SocketCreator.h, SocketCreator.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 4 June 2014
* - first release
*/

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "ErrorHandler.h"


//***********************************************************************
// SocketCreator
//
// This class defines operations to create an active socket, create a 
// passive socket, and operations to bind to an concrete host and service.
//***********************************************************************

class SocketCreator
{
public:
    int inetConnect(const char *host, 
                    const char *service, 
                    int type);
    int inetListen(const char *host, 
                   const char *service, 
                   int backlog, 
                   socklen_t *addrlen);
    int inetBind(const char *host, 
                 const char *service, 
                 int type, 
                 socklen_t *addrlen);
    char *inetAddressStr(const struct sockaddr *addr, 
                         socklen_t addrlen,
                         char *addr_str, 
                         int addr_strlen);
private:
    // Public interfaces: inetBind() and inetListen() 
    int inetPassiveSocket(const char *host, 
                          const char *service, 
                          int type, 
                          socklen_t *addrlen,
                          bool do_listen, int backlog);

    const static int IS_ADDR_STR_LEN = 4096;
};


#endif
