/////////////////////////////////////////////////////////////////////
//  ClientManager.cpp - implementations of a multi-process client manager 
//  ver 1.0                                                        
//  Language:      standard C++ 11                                
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "ClientManager.h"

//-------------------------------------------------------------------
// Constructor
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
// Entry point
// Fork a definite number of children to send requests
//-------------------------------------------------------------------
void ClientManager::start()
{
	pid_t pid;
	
	for (int i = 0; i < client_count_; i++) 
	{
		if((pid = fork()) == 0) 
		{
			childWork();
			//std::cout << "child " << i << " done.\n";
			_exit(EXIT_SUCCESS);
		}
	}
	
	// Wait for all children complete
	while(wait(NULL) != -1)
		continue;
	if(errno != ECHILD)
		perror("wait");
}

//-------------------------------------------------------------------
// Main function of a child process
//-------------------------------------------------------------------
void ClientManager::childWork()
{
	SocketCreator sc;
	int cfd = sc.inetConnect(host_, service_, SOCK_STREAM);
	if(cfd == -1) 
	{
		fprintf(stderr, "inetConnect error\n");
		exit(EXIT_FAILURE);
	}

	struct timeval tv;
	if(gettimeofday(&tv, NULL) == -1) 
	{
		ErrorHandler eh("gettimeofday", __FILE__, __FUNCTION__, __LINE__ - 1);
		eh.errExit();
	}

	srand(tv.tv_usec);
	
	int sleep_count = rand() % 10 + 1;
	char sleep_num[10];
	sprintf(sleep_num, "%d", sleep_count);

	if (write(cfd, sleep_num, strlen(sleep_num)) != strlen(sleep_num)) 
	{
		ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__ - 1);
		eh.errExit();
	}

	int num_read = 0;
	char read_buf[BUF_SIZE];
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
	}

	//int close_num = rand() % 3 + 1;

	//std::cout << "Received " << read_buf << std::endl;
	//std::cout << "number = " << close_num << std::endl;

	if (close(cfd) == -1) 
	{
		ErrorHandler eh("close", __FILE__, __FUNCTION__, __LINE__ - 1);
		eh.errExit();
	}
	
}

// Test case
#ifndef CLIENT_MANAGER_TEST

int main(int argc, char *argv[])
{
    if(argc < 4)
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
