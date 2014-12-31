#pragma once
#ifndef INTERFACE_H
#define INTERFACE_H
/////////////////////////////////////////////////////////////////////
//  Interface.h - define header files.
//  ver 2.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////
/*
* File Description:
* ==================
* Define header files used in 2014 Summer Project Load Balancing 
* Server design.
*
* Required Files:
* ===============
* Interface.h
*
* Maintenance History:
* ====================
* ver 1.0 : 25 May 2014
* - first release
* ver 2.0 : 31 Jul 2014
* - add DebugCode macro
*/

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <libgen.h>
#include <stdarg.h>
#include <time.h>
#include <fcntl.h>


#ifdef DEBUG
#define DebugCode( code_fragment ) { code_fragment }
#else
#define DebugCode( code_fragment )
#endif


#endif
