#ifndef GET_CURR_TIME_H
#define GET_CURR_TIME_H
/////////////////////////////////////////////////////////////////////
//  GetCurrTime.h - define class to get current time.
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
* Define GetCurrTime class to get current time.
*
* Required Files:
* ===============
* Interface.h, GetCurrTime.h, GetCurrTime.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 4 Jun 2014
* - first release
*/

#include "Interface.h"
#include <sys/time.h>


//***********************************************************************
// GetCurrTime
//
// This class defines operations to get current time. It provides three
// kinds of precision: second, microsecond and nanosecond. This class can
// be used to calculate elapsed time of a program.
//***********************************************************************

class GetCurrTime
{
public:
	GetCurrTime(){}
	~GetCurrTime(){}
	void getTime(struct timeval&);
	time_t getTime_s();
	suseconds_t getTime_us();
	long getTime_ns();
};


#endif