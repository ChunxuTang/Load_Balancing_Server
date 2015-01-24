/////////////////////////////////////////////////////////////////////
//  SchedSH.cpp - implementation of Source Hashing algorithm   
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../include/SchedulingAlgorithms/SchedAlgorithms.h"

//-------------------------------------------------------------------
// Select server by mapping source IP address to the servers
// with a hash function. This method is mainly used in cache server
// design, and is not a good choice in Load Balancer design.
// return : >0  file descriptor of selected server's socket
//           0  occur one error
//          -1  no available server 
//-------------------------------------------------------------------
int SchedSH::selectServer()
{
    in_addr svaddr;
    
    // Change destination IP address into a unsigned integer. 
    // inet_pton() only fits IPv4 address. And the result is
    // int big-endian format.
    int ret = inet_pton(AF_INET, source_ip_.c_str(), &svaddr);
    if (ret == 0) 
    {
        fprintf(stderr, "IP address is not in correct format.\n");
        return 0;
    }
    if (ret == -1) 
    {
        ErrorHandler eh("inet_pton", __FILE__, __FUNCTION__, __LINE__ - 6);
        eh.errMsg();
        return 0;
    }

    int selected_server = hashkey(svaddr.s_addr) % sched_map_.size();

    SchedMap::iterator it = sched_map_.begin();
    std::advance(it, selected_server);

    SchedMap::iterator backup = it;

    // If the hashed server is not available, use Round-Robin to find
    // next available server. If all servers are not available,
    // return -1.
    while (it->second.cur_load >= it->second.max_load - RESERVED_CAPACITY) 
    {
        std::advance(it, 1);
        if (it == sched_map_.end())
            it = sched_map_.begin();
        if (it == backup)
            return -1;
    }

    DebugCode(std::cout << "selected server: " << it->first << std::endl;)
    return it->first;
}

//-------------------------------------------------------------------
// Hash function.
// The number of bucket is 256. 2654435761UL is nearest to golden ratio
// between 2 and 2^32.
// golden ratio: (sqrt(5) - 1) / 2 = 0.618033989
// 2654435761 / 4294967296 = 0.618033987
//-------------------------------------------------------------------
unsigned SchedSH::hashkey(unsigned int hashed_ip)
{
    DebugCode(std::cout << hashed_ip << std::endl;)
    return (hashed_ip * 2654435761UL) & HASH_TAB_MASK;
}