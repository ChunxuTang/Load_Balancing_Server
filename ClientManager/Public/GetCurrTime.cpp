/////////////////////////////////////////////////////////////////////
//  GetCurrTime.cpp - implementation of GetCurrTime class   
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "GetCurrTime.h"

//-------------------------------------------------------------------
// Get current time and store it in a timeval struct passed in as
// an argument by reference.
//-------------------------------------------------------------------
void GetCurrTime::getTime(struct timeval& tv)
{
	if (gettimeofday(&tv, NULL) == -1) 
	{
		perror("gettimeofday");
		exit(EXIT_FAILURE);
	}
}

//-------------------------------------------------------------------
// Get current time and return by seconds.
//-------------------------------------------------------------------
time_t GetCurrTime::getTime_s()
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1) 
	{
		perror("gettimeofday");
		exit(EXIT_FAILURE);
	}

	return tv.tv_sec;
}

//-------------------------------------------------------------------
// Get current time and return by microseconds.
//-------------------------------------------------------------------
suseconds_t GetCurrTime::getTime_us()
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1) 
	{
		perror("gettimeofday");
		exit(EXIT_FAILURE);
	}

	return tv.tv_usec;
}

//-------------------------------------------------------------------
// Get current time and return by nanoseconds.
//-------------------------------------------------------------------
long GetCurrTime::getTime_ns()
{
	struct timespec ts;

	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) 
	{
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}
		
	return ts.tv_nsec;
}


//-------------------------------------------------------------------
// Test Case
//-------------------------------------------------------------------
#ifdef GET_CURR_TIME_TEST

int main(int argc, char *argv[])
{
	GetCurrTime gct;
	std::cout << "Now time is " << gct.getTime_s() << std::endl;
	std::cout << "Now time is " << gct.getTime_us() << std::endl;
	std::cout << "Now time is " << gct.getTime_ns() << std::endl;

	return 0;
}

#endif
