#ifndef FD_HANDLER_H
#define FD_HANDLER_H
/////////////////////////////////////////////////////////////////////
//  FdHandler.h - define functions to handle file descriptors
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
* Define functions set O_NONBLOCK, O_CLOEXEC to a file descriptor.
* Also define functions to add an fd to, delete an fd from, an epoll
* fd, and can reset EPOLLONESHOT of an fd.
*
* Required Files:
* ===============
* Interface.h, FdHandler.h
*
* Maintenance History:
* ====================
* ver 1.0 : 8 June 2014
* - first release
*/

#include <sys/epoll.h>
#include "Interface.h"


//***********************************************************************
// OneShotType
//
// An enumeration type that defines whether set an fd EPOLLONESHOT or not,
// used in addEvent() function.
//***********************************************************************
enum OneShotType { ONESHOT, NON_ONESHOT };


//***********************************************************************
// BlockType
//
// An enumeration type that defines whether set an fd BLOCK or not, used
// in addEvent() function.
//***********************************************************************
enum BlockType { BLOCK, NON_BLOCK };


// Enable and disable O_NONBLOCK of a file descriptor 
int setNonBlocking(int fd);
int disableNonBlocking(int fd);

// Enable and disable FD_CLOEXEC of a file descriptor 
int setCloseOnExec(int fd);
int disableCloseOnExec(int fd);


// Add and Remove an fd to and from an epollfd monitoring list
void addEvent(int epollfd, int fd, OneShotType oneshot_type, BlockType block_type);
void deleteEvent(int epollfd, int fd);

// Enable and disable EPOLLONESHOT of a file descriptor 
void setOneshot(int epollfd, int fd);
void disableOneShot(int epollfd, int fd);


#endif

