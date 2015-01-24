#ifndef SCHED_ALGORITHMS_H
#define SCHED_ALGORITHMS_H
/////////////////////////////////////////////////////////////////////
//  SchedAlgorithms.h - definitions of scheduling algorithms to select
//                      an appropriate server
//  ver 1.0                                                        
//  Language:      standard C++ 11                           
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////
/*
* File Description:
* ==================
* This class defines nine classes:
* (1) AbstractSchedAlgorithms, base class of other scheduling algorithms.
* (2) SchedRR, Round Robin scheduling algorithm.
* (3) SchedWRR, Weighted Round Robin scheduling algorithm.
* (4) SchedLC, Least Connection scheduling algorithm.
* (5) SchedWLC, weighted Least Connection scheduling algorithm.
* (6) SchedDH, Destination Hashing scheduling algorithm.
* (7) SchedSH, Source Hashing scheduling algorithm.
* (8) AlgorithmSelector, a simple factory and also a delegate to produce
*     an object of a scheduling algorithm.
*
* Required Files:
* ===============
* Interface.h ErrorHandler.h, ErrorHandler.cpp, SchedRR.cpp, SchedWRR.cpp,
* SchedLC.cpp, SchedWLC.cpp, SchedDH.cpp, SchedSH.cpp, AlgorithmSelector.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 1 Aug 2014
* - first release
*/


#include <iostream>
#include <unordered_map>
#include <string>
#include <netdb.h>
#include <arpa/inet.h>

#include "../Common/ErrorHandler.h"


//***********************************************************************
// RealServer
//
// A struct that represents a real server. The struct holds a real server's
// IP address, port number, max load and current load. This struct is used
// in a load balancer to schedule.
//***********************************************************************

struct RealServer
{
    std::string address;
    std::string port_num;
    int max_load;
    int cur_load;
};


//***********************************************************************
// SchedAlgorithm
//
// An enumeration type that defines different scheduling algorithm. This
// type is used in AlgorithmSelector to determine what kind of scheduling
// algorithm object need to produce.
//***********************************************************************

enum SchedAlgorithm { Round_Robin, 
                      Weighted_Round_Robin, 
                      Least_Connection, 
                      Weighted_Least_Connection, 
                      Destination_Hashing, 
                      Source_Hashing };


//***********************************************************************
// AbstractSchedAlgorithms
//
// The class defines an interface of a scheduling algorithm class.
//***********************************************************************

class AbstractSchedAlgorithms
{
public:
    using SchedMap = std::unordered_map<int, RealServer>;

    AbstractSchedAlgorithms(){};
    virtual ~AbstractSchedAlgorithms(){};
    virtual int selectServer() = 0;
    virtual void setSchedMap(const SchedMap&) = 0;
    virtual void setHandleIP(const std::string&){}
protected:
    // Reserved capacity of a server to avoid over load.
    static const int RESERVED_CAPACITY = 1;

    // These are the hash constants used in Destination Hashing
    // and Source Hashing algorithms.
    static const int HASH_TAB_BITS = 12;
    static const int HASH_TAB_SIZE = 1 << HASH_TAB_BITS;
    static const int HASH_TAB_MASK = HASH_TAB_SIZE - 1;
};


//***********************************************************************
// SchedRR
//
// This class defines operations of Round Robin scheduling algorithm.
// In Round Robin algorithm, the load balancer sends each request to
// the real servers one by one, sequentially.
//***********************************************************************

class SchedRR : public AbstractSchedAlgorithms
{
public:
    SchedRR(){};
    SchedRR(SchedMap& sched_map)
        : sched_map_(sched_map){}
    void setSchedMap(const SchedMap& sched_map) { sched_map_ = sched_map; }
    int selectServer();
private:
    SchedMap sched_map_;
};


//***********************************************************************
// SchedWRR
//
// This class defines operations of Weighted Round Robin scheduling algorithm.
// In Weighted Round Robin algorithm, the difference from Round Robin
// algorithm is that, there is a capacity for every real server, representing
// by max load in this design. When send requests to next server, load
// balancer should also consider weight.
//***********************************************************************

class SchedWRR : public AbstractSchedAlgorithms
{
public:
    SchedWRR(){}
    SchedWRR(SchedMap& sched_map)
        : sched_map_(sched_map){}
    void setSchedMap(const SchedMap& sched_map) { sched_map_ = sched_map; }
    int selectServer();
    
private:
    SchedMap sched_map_;
};


//***********************************************************************
// SchedLC
//
// This class defines operations of Least Connection scheduling algorithm.
// In Least Connection algorithm, the load balancer sends each request to
// the real server with least connections, which is represented by current
// load in this design.
//***********************************************************************

class SchedLC : public AbstractSchedAlgorithms
{
public:
    SchedLC(){}
    SchedLC(SchedMap& sched_map)
        : sched_map_(sched_map){}
    int selectServer();
    void setSchedMap(const SchedMap& sched_map) { sched_map_ = sched_map; }
private:
    SchedMap sched_map_;
};


//***********************************************************************
// SchedWLC
//
// This class defines operations of Weighted Least Connection scheduling 
// algorithm. The difference between this algorithm and Least Connection
// algorithm is similar to the difference between Round Robin and Weighted
// Round Robin. That is when load balancer sends requests, it should also
// consider capacity, and send request to min(connection / capacity).
//***********************************************************************

class SchedWLC : public AbstractSchedAlgorithms
{
public:
    SchedWLC(){}
    SchedWLC(SchedMap& sched_map)
        : sched_map_(sched_map){}
    int selectServer();
    void setSchedMap(const SchedMap& sched_map) { sched_map_ = sched_map; }
private:
    SchedMap sched_map_;
};


//***********************************************************************
// SchedDH
//
// This class defines operations of Destination Hashing scheduling algorithm.
// In Destination Hashing algorithm, the load balancer uses a hash function
// to map destination IP address to a server. This algorithm can be used
// in cache cluster, combined with cache bypass feature. When the statically
// assigned server is dead or overloaded, the load balancer can bypass
// the cache server and send requests to original server directly.
//***********************************************************************

class SchedDH : public AbstractSchedAlgorithms
{
public:
    SchedDH(){}
    SchedDH(SchedMap& sched_map, std::string dest_ip)
        : sched_map_(sched_map), dest_ip_(dest_ip) {}
    int selectServer();
    void setSchedMap(const SchedMap& sched_map) { sched_map_ = sched_map; }
    void setHandleIP(const std::string& dest_ip) { dest_ip_ = dest_ip; }
private:
    unsigned hashkey(unsigned int hashed_ip);
    SchedMap sched_map_;
    std::string dest_ip_;
};


//***********************************************************************
// SchedSH
//
// This class defines operations of Source Hashing scheduling algorithm.
// Source Hashing algorithm is very similar to Destination Hashing
// algorithm, only replacing destination IP address with source IP address. 
//***********************************************************************

class SchedSH : public AbstractSchedAlgorithms
{
public:
    SchedSH(){}
    SchedSH(SchedMap& sched_map, std::string source_ip)
        : sched_map_(sched_map), source_ip_(source_ip) {}
    int selectServer();
    void setSchedMap(const SchedMap& sched_map) { sched_map_ = sched_map; }
    void setHandleIP(const std::string& source_ip) { source_ip_ = source_ip; }
private:
    unsigned hashkey(unsigned int hashed_ip);
    SchedMap sched_map_;
    std::string source_ip_;
};


//***********************************************************************
// AlgorithmSelector
//
// This class acts as a simple factory and a delegate of the scheduling
// algorithms. To schedule, load balancer doesn't need to access concrete
// scheduling algorithm. It can simply construct an AlgorithmSelector
// object and designate which type scheduling algorithm need to be used.
//***********************************************************************

class AlgorithmSelector
{
public:
    using SchedMap = std::unordered_map<int, RealServer>;

    AlgorithmSelector(SchedAlgorithm sched_type);
    ~AlgorithmSelector();

    // Select scheduling algorithm, needed be invoked first
    void selectAlgorithm();

    // set and get functions
    void setSchedMap(const SchedMap& sched_map);
    void setSchedType(const SchedAlgorithm sched_type);
    const SchedAlgorithm getSchedType();
    void setHandleIP(const std::string& handle_ip);
    const AbstractSchedAlgorithms* getSchedAlgoPtr();

    // Invoke a scheduling algorithm's selectServer() function
    int selectServer();
private:
    SchedAlgorithm sched_type_;
    AbstractSchedAlgorithms *sched_algo_;
};


#endif