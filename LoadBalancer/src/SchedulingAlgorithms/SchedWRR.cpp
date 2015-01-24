/////////////////////////////////////////////////////////////////////
//  SchedWRR.cpp - implementation of Weighted Round Robin algorithm   
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../include/SchedulingAlgorithms/SchedAlgorithms.h"


//-------------------------------------------------------------------
// Select servers one by one, according to their weights, which are
// represented by max load - cur_load.
// return : >0  file descriptor of selected server's socket
//          -1  no available server 
//-------------------------------------------------------------------
int SchedWRR::selectServer()
{
    SchedMap::iterator it = sched_map_.begin();

    for (; it != sched_map_.end(); it++)
    {
        // First select a server who has enough capacity to 
        // handle a request. If there's not one available,
        // the for loop will be terminated, and return -1.
        if (it->second.cur_load >= it->second.max_load - RESERVED_CAPACITY)
            continue;

        SchedMap::iterator it2 = it;
        it2++;

        // Traverse the left servers to find whether there is
        // one server's weight larger than this one.
        for (; it2 != sched_map_.end(); it2++)
        {
            if (it2->second.max_load - it2->second.cur_load >
                it->second.max_load - it->second.cur_load)
                it = it2;
        }

        DebugCode(std::cout << "selected server: " << it->first << std::endl;)
        return it->first;
    }

    return -1;
}
