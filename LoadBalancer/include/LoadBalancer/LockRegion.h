#ifndef REGION_LOCKING_H
#define REGION_LOCKING_H
/////////////////////////////////////////////////////////////////////
//  LockRegion.h - lock or unlock a file region
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
* Define operations to lock or unlock a file region. This code
* is reference to "The Linux Programming Interface: A Linux and Unix
* System Programming Handbook", Michael Kerrisk, No Starch Press, 2010.
*
* Required Files:
* ===============
* LockRegion.h, LockRegion.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 7 Aug 2014
* - first release
*/

#include <sys/types.h>
#include <fcntl.h>


// Lock a file region with nonblock
int lockRegion(int fd, int type, int whence, int start, int len);

// Lock a file region with block
int lockRegionWait(int fd, int type, int whence, int start, int len);

// Unlock a file region with block
int unlockRegion(int fd, int whence, int start, int len);

// Test to lock a file region
pid_t regionIsLocked(int fd, int type, int whence, int start, int len);


#endif
