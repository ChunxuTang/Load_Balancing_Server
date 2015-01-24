/////////////////////////////////////////////////////////////////////
//  ClientManager.cpp - implementation of a client manager   
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#define _POSIX_C_SOURCE 199309

#include "../../include/ClientManager/ClientManager.h"

pthread_mutex_t ClientManager::mtx_ = PTHREAD_MUTEX_INITIALIZER;
int ClientManager::client_exist_ = 0;
int ClientManager::cache_hit_count_ = 0;
std::list<int> ClientManager::sock_list_;
LRUCache<int, HTTPMessage>* ClientManager::request_cache_ = new LRUCache<int, HTTPMessage>(3);

//-------------------------------------------------------------------
// List cache list and map information
//-------------------------------------------------------------------
void ClientManager::listCache()
{
    std::cout << "-------------List Cache -------------\n";

    std::list<int> cacheList = request_cache_->getCacheList();
    std::unordered_map<int, HTTPMessage> cacheMap = request_cache_->getCacheMap();

    for (auto const &x : cacheList)
        std::cout << x << "  ";
    std::cout << std::endl;

    for (auto const &x : cacheMap)
        std::cout << x.first << ": " << x.second.http_msg << std::endl;
}

//-------------------------------------------------------------------
// Constructor
// Initialize number of clients needed created, server's IP address
// and server's port number
//-------------------------------------------------------------------
ClientManager::ClientManager(int client_count, char *host, char *service)
: client_count_(client_count)
{
    client_exist_ = client_count_;
    strcpy(host_, host);
    strcpy(service_, service);
}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
ClientManager::~ClientManager()
{
    client_count_ = 0;
}

//-------------------------------------------------------------------
// Entry point of a client manager
// Create a certain amount of clients and wait until all children
// have finish their work.
//-------------------------------------------------------------------
void ClientManager::start()
{
    initRequestMap();
    
    struct sigaction myaction;
    myaction.sa_handler = &(handleInterrupt);
    if(sigaction(SIGINT, &myaction, NULL) == -1)
    {
        ErrorHandler eh("sigaction", __FILE__, __FUNCTION__, __LINE__);
        eh.errExit();
    }

    for (int i = 0; i < client_count_; i++)
    {
        if (i % 10 == 0)
            sleep(1);

        createThread();
    }   

    // Wait until all clients have finished
    while (true) 
    {
        pthread_mutex_lock(&mtx_);
        if (client_exist_ == 0)
        {
            pthread_mutex_unlock(&mtx_);
            DebugCode(std::cout << "Finish\n";)
            break;
        }
        pthread_mutex_unlock(&mtx_);
    }

    std::cout << "Cache hit rate: " << static_cast<float>(cache_hit_count_) / 
                                       static_cast<float>(client_count_) << "%\n";
}

//-------------------------------------------------------------------
// Initialize request map
//-------------------------------------------------------------------
void ClientManager::initRequestMap()
{
    request_map_.insert({ 0, sendGetRequest });
    request_map_.insert({ 1, sendHeadRequest });
    request_map_.insert({ 2, sendPutRequest });
    request_map_.insert({ 3, sendPostRequest });
    request_map_.insert({ 4, sendTraceRequest });
    request_map_.insert({ 5, sendOptionsRequest });
    request_map_.insert({ 6, sendDeleteRequest });
}

//-------------------------------------------------------------------
// Construct SEND request
//-------------------------------------------------------------------
void ClientManager::sendGetRequest(ClientManager *cm)
{
    std::cout << "Send get request\n";
    GetMethodWriter gmw("../testfile/download.txt", "HTTP/1.1", "localhost", "*", "127.0.0.1", cm->port_num_);
    gmw.constructHTTPMsg(cm->send_msg_);
}

//-------------------------------------------------------------------
// Construct HEAD request
//-------------------------------------------------------------------
void ClientManager::sendHeadRequest(ClientManager *cm)
{
    std::cout << "Send head request\n";
    HeadMethodWriter hmw("../testfile/download.txt", "HTTP/1.1", "localhost", "*", "127.0.0.1", cm->port_num_);
    hmw.constructHTTPMsg(cm->send_msg_);
}

//-------------------------------------------------------------------
// Construct PUT request
//-------------------------------------------------------------------
void ClientManager::sendPutRequest(ClientManager *cm)
{
    std::cout << "Send put request\n";
    PutMethodWriter pmw("../testfile/upload.txt", "HTTP/1.1", "localhost", "text/plain", "15", "127.0.0.1", cm->port_num_);
    pmw.addBody("I'm a message.");
    pmw.constructHTTPMsg(cm->send_msg_);
}

//-------------------------------------------------------------------
// Construct POST request
//-------------------------------------------------------------------
void ClientManager::sendPostRequest(ClientManager *cm)
{
    std::cout << "Send post request\n";
    PostMethodWriter pmw("../testfile/upload.txt", "HTTP/1.1", "localhost", "text/plain", "10", "127.0.0.1", cm->port_num_);
    pmw.addBody("color=red");
    pmw.constructHTTPMsg(cm->send_msg_);
}

//-------------------------------------------------------------------
// Construct TRACE request
//-------------------------------------------------------------------
void ClientManager::sendTraceRequest(ClientManager *cm)
{
    std::cout << "Send trace request\n";
    TraceMethodWriter tmw("../testfile/download.txt", "HTTP/1.1", "localhost", "*", "127.0.0.1", cm->port_num_);
    tmw.constructHTTPMsg(cm->send_msg_);
}

//-------------------------------------------------------------------
// Construct OPTIONS request
//-------------------------------------------------------------------
void ClientManager::sendOptionsRequest(ClientManager *cm)
{
    std::cout << "Send options request\n";
    OptionsMethodWriter omw("*", "HTTP/1.1", "localhost", "*", "127.0.0.1", cm->port_num_);
    omw.constructHTTPMsg(cm->send_msg_);
}

//-------------------------------------------------------------------
// Construct DELETE request
//-------------------------------------------------------------------
void ClientManager::sendDeleteRequest(ClientManager *cm)
{
    std::cout << "Send delete request\n";
    DeleteMethodWriter dmw("../testfile/delete.txt", "HTTP/1.1", "localhost", "127.0.0.1", cm->port_num_);
    dmw.constructHTTPMsg(cm->send_msg_);
}

//-------------------------------------------------------------------
// A client main function
// Send a request to the server and receive the response.
//-------------------------------------------------------------------
void* ClientManager::createClient(void *arg)
{
    ClientManager *cm = (ClientManager *)arg;

    SocketCreator sc;
    int cfd = sc.inetConnect(cm->host_, cm->service_, SOCK_STREAM);
    if(cfd == -1)
    {
        fprintf(stderr, "inetConnect error\n");
        exit(EXIT_FAILURE);
    }

    pthread_mutex_lock(&mtx_);
    sock_list_.push_back(cfd);
    pthread_mutex_unlock(&mtx_);
    
    struct sockaddr claddr;
    socklen_t addrlen = sizeof(struct sockaddr);
    
    memset(&claddr, 0, sizeof(struct sockaddr));
    
    if (getsockname(cfd, &claddr, &addrlen) == -1)
    {
        ErrorHandler eh("getsockname", __FILE__, __FUNCTION__, __LINE__ - 2);
        eh.errExit();
    }
    
    char client_host[NI_MAXHOST], client_service[NI_MAXSERV];

    // Get the client's IP address and port number. This inforamtion is used
    // in constructing HTTP message as source IP and source port number.
    if (getnameinfo(&claddr, addrlen, client_host, NI_MAXHOST, client_service, NI_MAXSERV, NI_NUMERICSERV) != 0)
    {
        ErrorHandler eh("getnameinfo", __FILE__, __FUNCTION__, __LINE__ - 2);
        eh.errExit();
    }

    struct timeval tv;
    if (gettimeofday(&tv, NULL) == -1)
    {
        ErrorHandler eh("gettimeofday", __FILE__, __FUNCTION__, __LINE__ - 2);
        eh.errExit();
    }

    // Here srand(time(NULL)) will produce same random numbers in threads, 
    // because the time used to produce is very tiny. Use microsecond or
    // nanosecond instead.
    srand(tv.tv_usec);
   
    pthread_mutex_lock(&mtx_);
    cm->port_num_ = client_service;

    // Get a random option
    int option = rand() % 7;
    std::cout << "option: " << option << std::endl;
    //listCache();


    if (request_cache_->isCached(option))
    {
        std::cout << "Cache hits!\n";
        cache_hit_count_++;
        DebugCode(std::cout << "Client receive:\n";
                  std::cout << request_cache_->getElement(option).http_msg;)
        
        pthread_mutex_unlock(&mtx_);
    }
    else
    {
        cm->request_map_.at(option)(cm);

        if (write(cfd, cm->send_msg_.http_msg, HTTPMessage::HTTP_MSG_SIZE) == -1)
        {
            ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__);
            eh.errExit();
        }
        pthread_mutex_unlock(&mtx_);

        HTTPMessage recv_msg;
        ssize_t num_read = read(cfd, recv_msg.http_msg, HTTPMessage::HTTP_MSG_SIZE);
        if (num_read == -1)
        {
            ErrorHandler eh("read", __FILE__, __FUNCTION__, __LINE__);
            eh.errMsg();
        }
        if (num_read == 0)
            fprintf(stderr, "unexpected EOF from server\n");

        DebugCode(std::cout << "Client receive:\n";
                  std::cout << recv_msg.http_msg;)

        pthread_mutex_lock(&mtx_);
        request_cache_->putElement(option, recv_msg);
        pthread_mutex_unlock(&mtx_);
    }


    close(cfd);

    pthread_mutex_lock(&mtx_);
    client_exist_--;
    sock_list_.remove(cfd);
    pthread_mutex_unlock(&mtx_);   
}

//-------------------------------------------------------------------
// Create a detached thread
//-------------------------------------------------------------------
void ClientManager::createThread()
{
    pthread_t t1;
    pthread_attr_t attr;
    int s;

    s = pthread_attr_init(&attr);
    if (s != 0)
    {
        ErrorHandler eh("pthread_attr_int", __FILE__, __FUNCTION__, __LINE__);
        eh.errExit();
    }

    s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (s != 0)
    {
        ErrorHandler eh("pthread_attr_setdetachstate", __FILE__, __FUNCTION__, __LINE__);
        eh.errExit();
    }

    s = pthread_create(&t1, &attr, createClient, this);
    if (s != 0)
    {
        ErrorHandler eh("pthread_create", __FILE__, __FUNCTION__, __LINE__);
        eh.errExit();
    }

    s = pthread_attr_destroy(&attr);
    if (s != 0)
    {
        ErrorHandler eh("pthread_destroy", __FILE__, __FUNCTION__, __LINE__);
        eh.errExit();
    }
}

//-------------------------------------------------------------------
// Catch SIGINT, clear resources and exit.
//-------------------------------------------------------------------
void ClientManager::handleInterrupt(int sig)
{
    std::list<int>::iterator it;
    it = sock_list_.begin();
    for (; it != sock_list_.end(); it++) 
    {
        DebugCode(std::cout << "close " << *it << std::endl;)
        if (close(*it) == -1) 
        {
            ErrorHandler eh("close", __FILE__, __FUNCTION__, __LINE__ - 1);
            eh.errMsg();
        }
    }

    exit(EXIT_SUCCESS);
}


//-------------------------------------------------------------------
// Test case
//-------------------------------------------------------------------
#ifndef CLIENT_MANAGER_TEST

int main(int argc, char *argv[])
{
    if (argc < 4) 
    {
        std::cout << "Usage: " << argv[0] << " <#clients> <hostname> <port>\n";
        exit(EXIT_SUCCESS);
    }

    // Calculate elapsed time
    struct timeval start_tv;
    GetCurrTime gct;
    gct.getTime(start_tv);

    int num_client = atoi(argv[1]);
    ClientManager cm(num_client, argv[2], argv[3]);
    cm.start();

    struct timeval finish_tv;
    gct.getTime(finish_tv);

    long time_use = 1000000 * (finish_tv.tv_sec - start_tv.tv_sec) + finish_tv.tv_usec - start_tv.tv_usec;
    std::cout << "Time used: " << time_use << " microseconds\n";

    return 0;

}

#endif
