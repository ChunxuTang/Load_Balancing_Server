#ifndef LOAD_BALANCER_H
#define LOAD_BALANCER_H
/////////////////////////////////////////////////////////////////////
//  LoadBlancer.h - define operations to balance load among servers.
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
* This file defines the LoadBalancer class to balance loads among
* servers. The load balancer receives requests from clients and send
* them to appropriate servers by using different scheduling algorithms.
*
* Required Files:
* ===============
* Interface.h, ErrorHandle.h, ErrorHandler.cpp, SocketCreator.h, 
* SocketCreator.cpp, HTTPBasic.h, HTTPWriter.cpp, ResponseMessage.cpp,
* RequestMessage.cpp, HTTPReader.h, HTTPReader.cpp, ResponseHandler.cpp,
* FdHandler.h, SchedAlgorithm.h, SchedRR.cpp, SchedWRR.cpp, SchedLC.cpp,
* SchedWLC.cpp, SchedDH.cpp, SchedSH.cpp, AlgorithmSelector.cpp, 
* LoadBalancer.h, LoadBalancer.cpp
*
* Maintenance History:
* ====================
* ver 1.0 : 6 Aug 2014
* - first release
*/


#include <sys/epoll.h>
#include <sys/timerfd.h> 
#include <sys/signalfd.h>
#include <sys/select.h> 
#include <signal.h>
#include <unordered_map>
#include <map>
#include <string>
#include <iomanip>

#include "../Common/SocketCreator.h"
#include "../Common/FdHandler.h"
#include "../HTTP/HTTPReader/HTTPReader.h"
#include "../SchedulingAlgorithms/SchedAlgorithms.h"
#include "CreatePidFile.h"


//-------------------------------------------------------------------
// One line of a header is like this:
// Content-Type: text/plain
// (target)       (content)
// This function is used to get content after symbol ':' according
// to target string.
//-------------------------------------------------------------------
static void getHeaderInfo(const HTTPMessage& msg, 
						  std::string& content, 
						  const std::string& target)
{
	std::string received_msg = msg.http_msg;
	decltype(target.size()) found = received_msg.find(target);
	decltype(target.size()) index;

	if (found != std::string::npos)
	{
		index = found + target.size();
		for (decltype(index) i = index; i < received_msg.size() - 1; i++)
		{
			if (received_msg.at(i) == '\r' && received_msg.at(i + 1) == '\n')
			{
				content = received_msg.substr(index, i - index);
				break;
			}
		}
	}
}

//-------------------------------------------------------------------
// Get target IP address by finding the content after "Target-IP: ".
// This function is used to find target client's file descriptor in
// clients' requests map.
//-------------------------------------------------------------------
static void getTargetIP(const HTTPMessage& msg, std::string& target_ip)
{
	getHeaderInfo(msg, target_ip, "Target-IP: ");
}

//-------------------------------------------------------------------
// Get target port number by finding the content after "Target-Port: ".
// This function is used to find target client's file descriptor in
// clients' requests map.
//-------------------------------------------------------------------
static void getTargetPort(const HTTPMessage& msg, std::string& target_port)
{
	getHeaderInfo(msg, target_port, "Target-Port: ");
}

//-------------------------------------------------------------------
// Get HTTP message body by finding the content after "\r\n".
//-------------------------------------------------------------------
static void getBody(const HTTPMessage& msg, std::string& body)
{
	getHeaderInfo(msg, body, "\r\n\r\n");
}


//***********************************************************************
// RequestInfo
//
// This struct stores information of a request, including the client's
// IP address and file descriptor.
//***********************************************************************

struct RequestInfo
{
	std::string client_addr;
	int client_fd;
};


//***********************************************************************
// Status
//
// This enumeration type defines return type of a function.
//***********************************************************************

enum Status { SUCCESS = 0,     // function succeed
			  MINOR_ERROR = 1, // occur a minor error, show error message
			  FATAL_ERROR = -1 // occur a fatal error, show error message and exit
};


//***********************************************************************
// LoadBalancer
//
// The core of a load balancing server. This class defines operations to 
// balance loads between real servers. 
// The principle of a load balancer is:
// A load balancer receives a request from a client, stores the client's
// information as a RequestInfo object and puts it in request map. Then
// the load balancer sends this request to an available server by a 
// scheduling algorithm. After the real server finishes its work and sends
// response back, load balancer checks the message, get target IP and port
// number, find corresponding socket file descriptor in request map and
// sends it back.
//***********************************************************************

class LoadBalancer
{
public:
	using ServerPool = std::unordered_map<int, RealServer>;
	using RequestMap = std::multimap<std::string, RequestInfo>;

	// Use singleton pattern to create only one load balancer
	static LoadBalancer* create(SchedAlgorithm sched_type);
	~LoadBalancer();
	LoadBalancer(const LoadBalancer& ) = delete;
	LoadBalancer& operator=(const LoadBalancer& ) = delete;

	void start();  // entry point of work of load balancer
	Status connectRealServers(); // try to connect to different real servers
	Status handleRequestFromClient(); // get requests from clients and send them to servers
	Status handleResultFromServer(int trigger_fd); // get results from servers and send them to clients
	Status healthCheck(); // check the servers' health
	Status handleSignal(); // handle different signals

	void setServerPool(const ServerPool& server_pool);
private:
	// Constructor is private.
	LoadBalancer(SchedAlgorithm sched_type);

	// initialize data members 
	Status initEpollfd();
	Status initTimerfd(struct itimerspec& ts);
	Status initSignalfd();
	Status initListenfd();

	void listRealServers();
	void listRequests();
	Status getSourceInfo(struct sockaddr* addr, socklen_t len, char *host, char *service);
	void clearAll();

	static LoadBalancer *instance; // static instance used in Singleton Pattern
	int lock_file_fd_;
	int epoll_fd_;
	int timer_fd_;
	int listen_fd_;
	int signal_fd_;
	fd_set server_fds_;
	bool balancer_run_;
	std::string target_ip_;
	std::string target_port_;
	AlgorithmSelector *algorithm_selector_;

	// Hash table to store information of servers.
	// Key is servers' file descriptors.
	ServerPool server_pool_;

	// Red Black Tree to store information of requests.
	// Key is clients' port number.
	RequestMap request_map_;

	static const char *PROGRAM_NAME;    // prpgram name used in createPidFile()
	static const char *PID_FILE;        // pid file needs to be locked
	static const char *PORT_NUM;        // load balancer's port number             
	static const char *SERVER_PORT_NUM; // real servers' port number
	static const char *BIND_ADDRESS;    // load balancer's IP address
	static const int MAX_EVENTS = 10;   // max number of events an epollfd can monitor
	static const int BACKLOG = 50;      // max number of fds a socket can listen one time
	static const int HEALTH_CHECK_INTERVAL = 30; // interval between two health check
	static const int HEALTH_CHECK_TIME_OUT = 2;  // time out in one health check
	static const int MAX_REAL_SERVER = 3; // max number of real servers a load balancer
										  // can communicate with
};


#endif
