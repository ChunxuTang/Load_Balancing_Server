#ifndef FD_TRANSFER_H
#define FD_TRANSFER_H
/////////////////////////////////////////////////////////////////////
//  FdTransfer.h - define functions to transfer file descriptors
//                 between processes.
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
* Define functions to write and receive file descriptors between
* different processes. The functions use sendmsg() and recvmsg().
* This file refers to "Unix Network Programming, Volume 1: The sockets
* Networking API", 3rd Edition, W.Richard Stevens, Bill Fenner, 
* Andrew M.Rudoff, Addison-Wesley, 2003.
*
* Required Files:
* ===============
* Interface.h, FdTransfer.h, FdTransfer.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 17 June 2014
* - first release
*/


#include <sys/socket.h> 
#include <arpa/inet.h>
#include <netinet/in.h>
#include "../Common/Interface.h"

// write a file descriptor to another process
ssize_t writeFd(int fd, void *ptr, size_t nbytes, int sendfd);

// receive a file descriptor from another process
ssize_t readFd(int fd, void *ptr, size_t nbytes, int *recvfd);


#endif