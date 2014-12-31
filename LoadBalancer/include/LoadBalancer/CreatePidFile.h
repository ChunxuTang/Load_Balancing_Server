#ifndef CREATE_PID_FILE_H   
#define CREATE_PID_FILE_H
/////////////////////////////////////////////////////////////////////
//  CreatePidFile.h - create a PID lock file
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
* Define operations to create a PID lock file, used to ensure run only 
* one instance of a program. This code is reference to "The Linux 
* Programming Interface: A Linux and Unix System Programming Handbook", 
* Michael Kerrisk, No Starch Press, 2010.
*
* Required Files:
* ===============
* Interface.h, CreatePidFile.h, CreatePidFile.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 7 Aug 2014
* - first release
*/

#include <sys/stat.h>
#include "../Common/Interface.h"
#include "LockRegion.h"

#define CPF_CLOEXEC 1

int createPidFile(const char *progName, const char *pidFile, int flags);

#endif