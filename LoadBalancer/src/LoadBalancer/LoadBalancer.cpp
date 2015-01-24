/////////////////////////////////////////////////////////////////////
//  LoadBlancer.cpp - implementations of a load balancing server.
//  ver 1.0                                                        
//  Language:      standard C++ 11                                
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../include/LoadBalancer/LoadBalancer.h"

LoadBalancer* LoadBalancer::instance = nullptr;
const char* LoadBalancer::PROGRAM_NAME = "LoadBalancer";
const char* LoadBalancer::PID_FILE = "BalancerPidFile.txt";
const char* LoadBalancer::PORT_NUM = "60000";
const char* LoadBalancer::BIND_ADDRESS = "127.0.0.1";
const char* LoadBalancer::SERVER_PORT_NUM = "50000";

//-------------------------------------------------------------------
// Create function to create only one instance of LoadBalancer by
// using Singleton Pattern.
//-------------------------------------------------------------------
LoadBalancer* LoadBalancer::create(SchedAlgorithm sched_type)
{
    if (instance == nullptr) 
        instance = new LoadBalancer(sched_type);

    return instance;
}

//-------------------------------------------------------------------
// Constructor
// Get scheduling algorithm type and initialize data members.
//-------------------------------------------------------------------
LoadBalancer::LoadBalancer(SchedAlgorithm sched_type)
{
    lock_file_fd_ = 0;
    epoll_fd_ = 0;
    timer_fd_ = 0;
    listen_fd_ = 0;
    signal_fd_ = 0;
    FD_ZERO(&server_fds_);
    balancer_run_ = true;
    algorithm_selector_ = new AlgorithmSelector(sched_type);
}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
LoadBalancer::~LoadBalancer() 
{
    lock_file_fd_ = 0;
    epoll_fd_ = 0;
    timer_fd_ = 0;
    listen_fd_ = 0;
    signal_fd_ = 0;
    FD_ZERO(&server_fds_);
    balancer_run_ = false;
    delete algorithm_selector_;
}

//-------------------------------------------------------------------
// Entry point of the whole work of a load balancer. This function
// invokes other relative functions.
//-------------------------------------------------------------------
void LoadBalancer::start()
{
    lock_file_fd_ = createPidFile(PROGRAM_NAME, PID_FILE, 1);
    if (lock_file_fd_ == -1)
        return;

    // Select scheduling algorithm by sched_type;
    algorithm_selector_->selectAlgorithm();

    struct itimerspec ts;

    // initialize relative file descriptors
    if (initEpollfd() == FATAL_ERROR ||
        initSignalfd() == FATAL_ERROR ||
        connectRealServers() == FATAL_ERROR ||
        initTimerfd(ts) == FATAL_ERROR ||
        initListenfd() == FATAL_ERROR)
        return;

    DebugCode(listRealServers();)

    struct epoll_event evlist[MAX_EVENTS];
    int ready;
    int trigger_fd;
    Status ret;

    // A while loop to continuously read events from epoll fd,
    // until there is an error and balancer_run_ is false.
    while (balancer_run_)
    {
        ready = epoll_wait(epoll_fd_, evlist, MAX_EVENTS, -1);
        if (ready == -1)
        {
            if (errno == EINTR)
                continue;
            else
            {
                ErrorHandler eh("epoll_wait", __FILE__, __FUNCTION__, __LINE__);
                eh.errMsg();
                balancer_run_ = false;
                break;
            }
        }

        // read the events and invoke corresponding handlers
        for (int i = 0; i < ready; i++)
        {
            DebugCode(printf("\tfd=%d; events: %s%s%s\n", evlist[i].data.fd,
                (evlist[i].events & EPOLLIN) ? "EPOLLIN " : "",
                (evlist[i].events & EPOLLHUP) ? "EPOLLHUP " : "",
                (evlist[i].events & EPOLLERR) ? "EPOLLERR " : "");)

            trigger_fd = evlist[i].data.fd;

            // Get a request from a client
            if ((trigger_fd == listen_fd_) & evlist[i].events & EPOLLIN)
            {
                ret = handleRequestFromClient();

                if (ret == Status::MINOR_ERROR)
                    continue;
                if (ret == Status::FATAL_ERROR)
                {
                    balancer_run_ = false;
                    break;
                }
            }

            // get response from a real server
            else if ((FD_ISSET(trigger_fd, &server_fds_)) & evlist[i].events & EPOLLIN)
            {
                ret = handleResultFromServer(trigger_fd);

                if (ret == Status::MINOR_ERROR)
                    continue;
                if (ret == Status::FATAL_ERROR)
                {
                    balancer_run_ = false;
                    break;
                }
            }

            // time to begin a health check
            else if ((trigger_fd == timer_fd_) & evlist[i].events & EPOLLIN)
            {
                ret = healthCheck();

                if (ret == Status::MINOR_ERROR)
                {
                    timerfd_settime(timer_fd_, 0, &ts, NULL);
                    continue;
                }

                if (ret == Status::FATAL_ERROR)
                {
                    balancer_run_ = false;
                    std::cout << "fatal error from health check\n";
                    break;
                }

                // reset the timer
                timerfd_settime(timer_fd_, 0, &ts, NULL);
            }

            // catch a signal
            else if ((trigger_fd == signal_fd_) & evlist[i].events & EPOLLIN)
            {
                // Do not need to check return value, because SIGINT and SIGTERM
                // causes program termination.
                handleSignal();
            }

            // occur an error
            else
            {
                std::cout << "Unknown trigger_fd: " << trigger_fd << std::endl;
                deleteEvent(epoll_fd_, trigger_fd);
            }
        }
    }

    // clear all the resources and exit
    clearAll();
}

//-------------------------------------------------------------------
// Initialize epoll fd
//-------------------------------------------------------------------
Status LoadBalancer::initEpollfd()
{
    epoll_fd_ = epoll_create(MAX_EVENTS);
    if (epoll_fd_ == -1)
    {
        ErrorHandler eh("epoll_create", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        return FATAL_ERROR;
    }

    return SUCCESS;
}

//-------------------------------------------------------------------
// Initialize signal fd
//-------------------------------------------------------------------
Status LoadBalancer::initSignalfd()
{
    // Can catch SIGINT and SIGTERM
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);

    if (sigprocmask(SIG_BLOCK, &mask, NULL) == -1)
    {
        ErrorHandler eh("sigprocmask", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        return FATAL_ERROR;
    }

    signal_fd_ = signalfd(-1, &mask, 0);
    if (signal_fd_ == -1)
    {
        ErrorHandler eh("signalfd", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        return FATAL_ERROR;
    }

    addEvent(epoll_fd_, signal_fd_, NON_ONESHOT, BLOCK);

    return SUCCESS;
}

//-------------------------------------------------------------------
// Initialize timer fd
//-------------------------------------------------------------------
Status LoadBalancer::initTimerfd(struct itimerspec& ts)
{
    // Every HEALTH_CHECK_INTERVAL seconds, load balancer
    // will begin a health check.
    ts.it_interval.tv_sec = 0;
    ts.it_interval.tv_nsec = 0;
    ts.it_value.tv_sec = HEALTH_CHECK_INTERVAL;
    ts.it_value.tv_nsec = 0;

    timer_fd_ = timerfd_create(CLOCK_REALTIME, 0);
    if (timer_fd_ == -1)
    {
        ErrorHandler eh("timerfd_create", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        return FATAL_ERROR;
    }

    if (timerfd_settime(timer_fd_, 0, &ts, NULL) == -1)
    {
        ErrorHandler eh("timerfd_settime", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        return FATAL_ERROR;
    }

    addEvent(epoll_fd_, timer_fd_, OneShotType::NON_ONESHOT, BlockType::BLOCK);

    return SUCCESS;
}

//-------------------------------------------------------------------
// Initialize listen fd
//-------------------------------------------------------------------
Status LoadBalancer::initListenfd()
{
    SocketCreator sc;
    socklen_t addrlen;

    listen_fd_ = sc.inetListen(BIND_ADDRESS, PORT_NUM, BACKLOG, &addrlen);
    if (listen_fd_ == -1)
    {
        fprintf(stderr, "socket inetListen error\n");
        return FATAL_ERROR;
    }

    addEvent(epoll_fd_, listen_fd_, OneShotType::NON_ONESHOT, BlockType::NON_BLOCK);

    return SUCCESS;
}

//-------------------------------------------------------------------
// Try to connect different real servers by sending HTTP message with
// method of SERVERCHECK, and check whether there is a server can
// send a response back with information of max load.
//-------------------------------------------------------------------
Status LoadBalancer::connectRealServers()
{
    SocketCreator sc;
    int cfd;
    HTTPMessage check_msg;

    // Try to connect 127.0.0.2, 127.0.0.3, 127.0.0.4
    // when MAX_REAL_SERVER is 3.
    for (int i = 1; i <= MAX_REAL_SERVER; i++)
    {
        char host_buf[NI_MAXHOST];
        sprintf(host_buf, "%s%d", "127.0.0.", i + 1);
        cfd = sc.inetConnect(host_buf, SERVER_PORT_NUM, SOCK_STREAM);
        if (cfd == -1)
        {
            std::cout << "connect fail\n";
            continue;
        }

        ServerCheckMethodWriter scmw(host_buf, "HTTP/1.1", host_buf, BIND_ADDRESS, PORT_NUM);
        scmw.constructHTTPMsg(check_msg);
        if (write(cfd, check_msg.http_msg, HTTPMessage::HTTP_MSG_SIZE) == -1)
        {
            ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__);
            eh.errMsg();
            return FATAL_ERROR;
        }

        HTTPMessage recv_msg;
        ssize_t num_read = read(cfd, recv_msg.http_msg, HTTPMessage::HTTP_MSG_SIZE);
        if (num_read == -1)
        {
            ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__);
            eh.errMsg();
            return Status::FATAL_ERROR;
        }
        if (num_read == 0) 
        {
            fprintf(stderr, "Unexpected EOF from a server\n");
            return Status::FATAL_ERROR;
        }

        std::cout << "-----Load balancer receive health check response:-----\n";
        std::cout << recv_msg.http_msg;

        // Get max load of a real server by reading body of the response
        std::string msg_body;
        getBody(recv_msg, msg_body);
        int max_load = convertStringToInt(msg_body);
        std::cout << "Max load of server " << host_buf << " is " << max_load << std::endl;

        addEvent(epoll_fd_, cfd, OneShotType::NON_ONESHOT, BlockType::BLOCK);
        FD_SET(cfd, &server_fds_);

        // a real server's information: IP address, port number, max_load, cur_load
        RealServer real_server = { host_buf, SERVER_PORT_NUM, max_load, 0 };
        server_pool_.insert(std::pair<int, RealServer>(cfd, real_server));
    }

    // If no real server is available, terminate load balancer. 
    if (server_pool_.size() <= 0)
        return Status::FATAL_ERROR;

    return Status::SUCCESS;
}

//-------------------------------------------------------------------
// Receive a request from a client, check the server_pool_ to find an
// appropriate server and send request to the server.
//-------------------------------------------------------------------
Status LoadBalancer::handleRequestFromClient()
{
    socklen_t addrlen;
    struct sockaddr_storage claddr;
    int cfd;

    addrlen = sizeof(struct sockaddr_storage);
    cfd = accept(listen_fd_, (struct sockaddr*)&claddr, &addrlen);
    if (cfd == -1)
    {
        ErrorHandler eh("accept", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        return FATAL_ERROR; // listen_fd_ cannot be read, no requests can be
                            // received, so this is a fatal error.
    }
    std::cout << "Load balancer accepts client's fd: " << cfd << std::endl;

    HTTPMessage recv_msg;
    memset(recv_msg.http_msg, '\0', HTTPMessage::HTTP_MSG_SIZE);

    ssize_t num_read = read(cfd, recv_msg.http_msg, HTTPMessage::HTTP_MSG_SIZE);
    if (num_read == -1) 
    {
        ErrorHandler eh("read", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        return MINOR_ERROR;
    }
    if (num_read == 0) 
    {
        fprintf(stderr, "EOF of client\n");
        return Status::MINOR_ERROR;
    }

    std::cout << "===========================================\n";
    std::cout << "Load Balancer receive a request from client:\n";
    std::cout << recv_msg.http_msg;
    std::cout << "===========================================\n";

    // get client's IP address and port number 
    RequestInfo request;
    char host[NI_MAXHOST], service[NI_MAXSERV];
    if (getSourceInfo((struct sockaddr*)&claddr, addrlen, host, service) == SUCCESS)
        request.client_addr = host;
    else
        return MINOR_ERROR;

    request.client_fd = cfd;

    int handle_fd;

    // Select an appropriate real server.
    if (server_pool_.size() > 0)
    {
        // update scheduling algorithm's server pool
        algorithm_selector_->setSchedMap(server_pool_);
        handle_fd = algorithm_selector_->selectServer();
    }
    else
    {
        fprintf(stderr, "No real server is available.\n");
        return Status::FATAL_ERROR;
    }

    if (handle_fd == -1 || handle_fd == 0)
    {
        std::string error_code;
        if (handle_fd == -1)
        {
            std::cout << "cannot handle any requests" << std::endl;
            error_code = StatusCode::ServerErrorStatusCode::HEAD503;
        }
        else
        {
            std::cout << "format is not correct" << std::endl;
            error_code = StatusCode::ServerErrorStatusCode::HEAD500;
        }

        HTTPMessage send_msg;
        ResponseMessage rm("HTTP/1.1", error_code, host, service);
        rm.constructHTTPMsg(send_msg);
        write(cfd, send_msg.http_msg, HTTPMessage::HTTP_MSG_SIZE);
        return Status::MINOR_ERROR;
    }

    // Send the request to a server
    if (write(handle_fd, recv_msg.http_msg, HTTPMessage::HTTP_MSG_SIZE) == -1)
    {
        ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        
        // Because there is an error in the server, delete resources of this
        // server in load balancer.
        deleteEvent(epoll_fd_, handle_fd);
        FD_CLR(handle_fd, &server_fds_);
        server_pool_.erase(handle_fd);
        close(handle_fd);

        return Status::MINOR_ERROR;
    }

    // Because load balancer has just sent a request, the server's current
    // load should increment 1.
    server_pool_[handle_fd].cur_load += 1;

    listRealServers();
    request_map_.insert(std::pair<std::string, RequestInfo>(service, request));

    return Status::SUCCESS;
}

//-------------------------------------------------------------------
// Get a response from a real server, parse the response message and
// get target IP and port number. According to this information, find
// corresponding client file descriptor and send it back.
//-------------------------------------------------------------------
Status LoadBalancer::handleResultFromServer(int trigger_fd)
{
    HTTPMessage recv_msg;
    memset(recv_msg.http_msg, '\0', HTTPMessage::HTTP_MSG_SIZE);

    ssize_t num_read = read(trigger_fd, recv_msg.http_msg, HTTPMessage::HTTP_MSG_SIZE);
    if (num_read == -1 || num_read == 0)
    {
        if (num_read == -1)
        {
            ErrorHandler eh("read", __FILE__, __FUNCTION__, __LINE__);
            eh.errMsg();
        }
        else
            fprintf(stderr, "unexpected EOF of a real server\n");

        // Because a real server meets an error, delete the resources of
        // this server in load balancer.
        deleteEvent(epoll_fd_, trigger_fd);
        FD_CLR(trigger_fd, &server_fds_);
        server_pool_.erase(trigger_fd);
        close(trigger_fd);

        if (server_pool_.size() <= 0)
            return Status::FATAL_ERROR;

        return Status::MINOR_ERROR;
    }

    std::cout << "Load Balancer receive response:\n";
    std::cout << recv_msg.http_msg;

    int target_fd = 0;
    getTargetIP(recv_msg, target_ip_);
    getTargetPort(recv_msg, target_port_);
    std::cout << "Target IP: " << target_ip_ << std::endl;
    std::cout << "Target Port: " << target_port_ << std::endl;

    listRequests();

    // Find target client by IP address and port number
    std::pair<RequestMap::iterator, RequestMap::iterator> ret;
    ret = request_map_.equal_range(target_port_);
    for (RequestMap::iterator it = ret.first; it != ret.second; it++)
    {
        if (it->second.client_addr == target_ip_)
        {
            target_fd = it->second.client_fd;
            request_map_.erase(it); 
            break;
        }
    }

    // When target_fd = 0, it means that either request_map_ is empty, 
    // or there's no same target_port element. A child of a real server
    // terminates can usually cause this happen. Under this situation, 
    // no response needed to be returned.
    if (target_fd == 0)
    {
        std::cout << "A child of a real server terminates.\n";
        return Status::MINOR_ERROR;
    }

    std::cout << "target port is " << target_port_ << "\n target client_fd = " << target_fd << std::endl;

    if (write(target_fd, recv_msg.http_msg, HTTPMessage::HTTP_MSG_SIZE) == -1)
    {
        ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        return Status::MINOR_ERROR;
    }

    if (close(target_fd) == -1)
    {
        ErrorHandler eh("close", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        return Status::MINOR_ERROR;
    }

    // Because a real server has just finished a request, decrement
    // current load by 1.
    server_pool_[trigger_fd].cur_load -= 1;

    listRealServers();

    return Status::SUCCESS;
}

//-------------------------------------------------------------------
// Check health of every real server on the server_pool_
//-------------------------------------------------------------------
Status LoadBalancer::healthCheck()
{
    std::cout << "======== Begin Health Check ========\n";
    //sleep(1); 
    
    // If real servers are still handling requests, there is no need
    // to check health. Only check health when the servers are free.
    if (request_map_.size() > 0)
        return Status::MINOR_ERROR;

    // If using Weighted Least Connection scheduling algorithm (default), 
    // when there is a server that is free, there is no possibility
    // that a server is handling two requests. Sleep to wait all requests
    // are finished and begin health check.
    else if (request_map_.size() <= server_pool_.size())
        sleep(3);

    HTTPMessage check_msg;
    HTTPMessage recv_msg;
    int server_fd;
    ServerPool::iterator it = server_pool_.begin();

    // Send an HTTP message with method of OPTIONS to real servers.
    while (it != server_pool_.end())
    {
        OptionsMethodWriter hmw("*", "HTTP/1.1", it->second.address, "*", BIND_ADDRESS, PORT_NUM);
        hmw.constructHTTPMsg(check_msg);

        std::cout << "check message:\n";
        std::cout << check_msg.http_msg;

        server_fd = it->first;
        if (write(server_fd, check_msg.http_msg, HTTPMessage::HTTP_MSG_SIZE) == -1)
        {
            ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__);
            eh.errMsg();

            deleteEvent(epoll_fd_, server_fd);
            FD_CLR(server_fd, &server_fds_);
            close(server_fd);
            it = server_pool_.erase(it);
            continue;
        }

        memset(recv_msg.http_msg, '\0', HTTPMessage::HTTP_MSG_SIZE);
        size_t numRead = read(server_fd, recv_msg.http_msg, HTTPMessage::HTTP_MSG_SIZE);
        if (numRead == -1 || numRead == 0) 
        {
            if (numRead == -1) 
            {
                ErrorHandler eh("read", __FILE__, __FUNCTION__, __LINE__);
                eh.errMsg();
            }
            else 
                fprintf(stderr, "unexpected EOF from server\n");

            deleteEvent(epoll_fd_, server_fd);
            FD_CLR(server_fd, &server_fds_);
            close(server_fd);
            it = server_pool_.erase(it);
            continue;
        }

        std::cout << "Health Check Result:\n";
        std::cout << recv_msg.http_msg;

        it++;
    }

    // All real servers are not available.
    if (server_pool_.size() <= 0) 
    {
        std::cout << "No real server is available.\n";
        return Status::FATAL_ERROR;
    }

    return Status::SUCCESS;
}

//-------------------------------------------------------------------
// Catch signals. In this handler, SIGTERM and SIGINT both causes
// termination of load balancer.
//-------------------------------------------------------------------
Status LoadBalancer::handleSignal()
{
    struct signalfd_siginfo fdsi;

    ssize_t s = read(signal_fd_, &fdsi, sizeof(fdsi));
    if (s == -1)
    {
        ErrorHandler eh("read", __FILE__, __FUNCTION__, __LINE__);
        clearAll();
        exit(EXIT_FAILURE);
    }

    DebugCode(std::cout << "send from " << fdsi.ssi_pid << std::endl;)

    int receive_signal = fdsi.ssi_signo;
    switch (receive_signal)
    {
    case SIGTERM:
    case SIGINT:
        std::cout << "catch SIGINT\n";
        clearAll();
        exit(EXIT_SUCCESS);
        break;
    default:
        std::cout << "Unknown signal " << receive_signal << std::endl;
        break;
    }

    return Status::SUCCESS;
}

//-------------------------------------------------------------------
// Reset server_pool_
//-------------------------------------------------------------------
void LoadBalancer::setServerPool(const ServerPool& server_pool) 
{ 
    server_pool_ = server_pool; 
}

//-------------------------------------------------------------------
// Get source IP address and port number
//-------------------------------------------------------------------
Status LoadBalancer::getSourceInfo(struct sockaddr* addr, 
                                   socklen_t len, 
                                   char *host, 
                                   char *service)
{
    if (getnameinfo(addr, len, host, NI_MAXHOST,
        service, NI_MAXSERV, NI_NUMERICSERV) == 0)
    {
        if (strcmp(host, "localhost") == 0)
            strcpy(host, "127.0.0.1");
        return SUCCESS;
    }
    else
    {
        ErrorHandler eh("getnameinfo", __FILE__, __FUNCTION__, __LINE__);
        eh.errMsg();
        return MINOR_ERROR;
    }
}

//-------------------------------------------------------------------
// Clear all the resources in the load balancer and exit
//-------------------------------------------------------------------
void LoadBalancer::clearAll()
{
    std::cout << "Load Balancer shuts down...\n";
    close(epoll_fd_);

    for (auto x : server_pool_)
        close(x.first);

    for (auto x : request_map_)
        close(x.second.client_fd);

    close(timer_fd_);
    close(signal_fd_);
    shutdown(listen_fd_, SHUT_RDWR);
    close(listen_fd_);
    close(lock_file_fd_);

    std::cout << "close epoll_fd_ = " << epoll_fd_ << std::endl;
    std::cout << "close signal_fd_ = " << signal_fd_ << std::endl;
    std::cout << "close timer_fd_ = " << timer_fd_ << std::endl;
    std::cout << "close listen_fd_ = " << listen_fd_ << std::endl;
    std::cout << "close lock_file_fd_ = " << lock_file_fd_ << std::endl;
}

//-------------------------------------------------------------------
// List information of all the available real servers
//-------------------------------------------------------------------
void LoadBalancer::listRealServers()
{
    std::cout << std::left << std::setw(12) << "Server" << std::setw(8)
        << "Port" << std::setw(10) << "Max Load" << std::setw(18) << "Current Load" << std::endl;

    for (auto it : server_pool_)
    {
        std::cout << std::left << std::setw(12) << it.first << std::setw(8) << it.second.port_num
            << std::setw(10) << it.second.max_load << std::setw(18) << it.second.cur_load << std::endl;
    }
}

//-------------------------------------------------------------------
// List information of all the requests
//-------------------------------------------------------------------
void LoadBalancer::listRequests()
{
    std::cout << std::left << std::setw(8) << "Port" << std::setw(12)
        << "Address" << std::setw(10) << "Client fd" << std::endl;

    for (auto it : request_map_)
    {
        std::cout << std::left << std::setw(8) << it.first << std::setw(12)
            << it.second.client_addr << std::setw(10) << it.second.client_fd << std::endl;
    }
}



int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " <scheduling algorithm>\n";
        std::cout << "RR:  Round Robin\n";
        std::cout << "WRR: Weighted Round Robin\n";
        std::cout << "LC:  Least Connection\n";
        std::cout << "WLC: Weighted Least Connection (Recommended)\n";
        std::cout << "DH:  Destination Hashing\n";
        std::cout << "SH:  Source Hashing\n";
        exit(EXIT_SUCCESS);
    }

    std::unordered_map<std::string, SchedAlgorithm> algorithm_map;
    algorithm_map.insert({ "RR", Round_Robin });
    algorithm_map.insert({ "WRR", Weighted_Round_Robin });
    algorithm_map.insert({ "LC", Least_Connection });
    algorithm_map.insert({ "WLC", Weighted_Least_Connection });
    algorithm_map.insert({ "DH", Destination_Hashing });
    algorithm_map.insert({ "SH", Source_Hashing });

    if (algorithm_map.find(argv[1]) != algorithm_map.end())
    {
        LoadBalancer *lb = LoadBalancer::create(algorithm_map.at(argv[1]));
        lb->start();
    }
    else
    {
        std::cout << "Incorrect scheduling algorithm.\n";
    }

    return 0;
}
