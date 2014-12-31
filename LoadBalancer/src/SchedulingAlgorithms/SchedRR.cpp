/////////////////////////////////////////////////////////////////////
//  SchedRR.cpp - implementation of Round Robin algorithm   
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../include/SchedulingAlgorithms/SchedAlgorithms.h"


//-------------------------------------------------------------------
// Select servers one by one, sequentially. If a server is not 
// available, select next server.
// return : >0  file descriptor of selected server's socket
//          -1  no available server 
//-------------------------------------------------------------------
int SchedRR::selectServer()
{
	static int count = 1;
	int offset = count % sched_map_.size();
	SchedMap::iterator it = sched_map_.begin();
	std::advance(it, offset);
	SchedMap::iterator backup = it;
	count++;

	// The loop terminate condition is that iterator it equals to
	// backup iterator, which means all the servers have been 
	// traversed and there's no server available.
	while (it->second.cur_load >= it->second.max_load - RESERVED_CAPACITY) 
	{
		offset = count % sched_map_.size();
		it = sched_map_.begin();
		std::advance(it, offset);
		if (it == backup)
			return -1;
		count++;
	}

	DebugCode(std::cout << "selected server: " << it->first << std::endl;)
	return it->first;
}