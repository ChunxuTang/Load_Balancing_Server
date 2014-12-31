/////////////////////////////////////////////////////////////////////
//  ClientManager.cpp - implementation of a multi-thread client manager 
//                    	without locks
//  ver 1.0                                                        
//  Language:      standard C++ 11                                
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#define _POSIX_C_SOURCE 199309

#include "ClientManager.h"

int ClientManager::client_exist_ = 0;

//-------------------------------------------------------------------
// Constructor
// Initialize number of clients needed created, server's IP address
// and server's port number
//-------------------------------------------------------------------
ClientManager::ClientManager(int client_count, char *host, char *service)
    : client_count_(client_count), host_(host), service_(service) {}

//-------------------------------------------------------------------
// Destructor
//-------------------------------------------------------------------
ClientManager::~ClientManager()
{
    client_count_ = 0;
	host_ = NULL;
	service_ = NULL;
}

//-------------------------------------------------------------------
// Entry point of a client manager
// Create a certain amount of clients and wait until all children
// have finish their work.
//-------------------------------------------------------------------
void ClientManager::start()
{
	client_exist_ = client_count_;

    for (int i = 0; i < client_count_; i++) 
        createThread();
   	
   	// No lock here
	while (client_exist_ > 0)
		;
}

//-------------------------------------------------------------------
// A client main function
// Send a request to the server and receive the response.
//-------------------------------------------------------------------
void* ClientManager::createClient(void *arg)
{
    struct serverInfo *info = (struct serverInfo*)arg;

    SocketCreator sc;
    int cfd = sc.inetConnect(info->host, info->service, SOCK_STREAM);
    if (cfd == -1) 
    { 
        fprintf(stderr, "inetConnect error\n");
		exit(EXIT_FAILURE);
    }

	GetCurrTime gct;
	suseconds_t time_now = gct.getTime_us();
    srand(time_now);

	int sleep_count;
	int num_read;
	char sleep_num[12];
	char read_buf[BUF_SIZE];

    while (1) 
    {
		sleep_count = rand() % 10 + 1;
		memset(sleep_num, 0, sizeof(sleep_num));
		sprintf(sleep_num, "%d", sleep_count);

		if (write(cfd, sleep_num, strlen(sleep_num)) != strlen(sleep_num)) 
		{
			ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__ - 1);
			eh.errExit();
		}

		num_read = 0;
		memset(read_buf, 0, sizeof(read_buf));

		num_read = read(cfd, read_buf, BUF_SIZE);
		if (num_read == -1) 
		{
			ErrorHandler eh("read", __FILE__, __FUNCTION__, __LINE__ - 1);
			eh.errExit();
		}
		if (num_read == 0) 
		{
			fprintf(stderr, "unexpected EOF from server\n");
			_exit(EXIT_FAILURE);
		}


        //int close_num = rand() % 3 + 1;
        //std::cout << "number = " << close_num << std::endl;
        //std::cout << "Received " << read_buf << std::endl;

		close(cfd);
		client_exist_--;
		break;
    }
}

//-------------------------------------------------------------------
// Create a detached thread
//-------------------------------------------------------------------
void ClientManager::createThread()
{
    pthread_t t1;
	pthread_attr_t attr;
	int s;

    serverInfo sInfo;
    strcpy(sInfo.host, host_);
    strcpy(sInfo.service, service_);

	s = pthread_attr_init(&attr);
	if (s != 0)
	{
		perror("pthread_attr_init");
		exit(EXIT_FAILURE);
	}
	s = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (s != 0)
	{
		perror("pthread_attr_setdetachstate");
		exit(EXIT_FAILURE);
	}
	s = pthread_create(&t1, &attr, createClient, &sInfo);
	if (s != 0)
	{
		perror("pthread_create");
		exit(EXIT_FAILURE);
	}
	s = pthread_attr_destroy(&attr);
	if (s != 0)
	{
		perror("pthread_attr_destroy");
		exit(EXIT_FAILURE);
	}
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

	struct timeval start_tv;
	GetCurrTime gct;
	gct.getTime(start_tv);

	int num_client = atoi(argv[1]);
	ClientManager cm(num_client, argv[2], argv[3]);
	cm.start();

	struct timeval finish_tv;
	gct.getTime(finish_tv);

	long time_use = 1000000 * (finish_tv.tv_sec - start_tv.tv_sec) + finish_tv.tv_usec - start_tv.tv_usec;
	std::cout << " Time used :" << time_use << " microseconds\n";

	return 0;
}

#endif
