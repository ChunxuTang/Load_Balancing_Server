
/////////////////////////////////////////////////////////////////////
//  Server.cpp - implementations of a simple server
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "Server.h"

const char* Server::PORT_NUM = "50000";
const char* Server::BIND_ADDRESS = "127.0.0.1";

//-------------------------------------------------------------------
// Entry point of the server
//-------------------------------------------------------------------
void Server::start()
{
	struct sockaddr_storage claddr;
	socklen_t addrlen = sizeof(claddr);
    SocketCreator sc;

	int lfd = sc.inetListen(BIND_ADDRESS, PORT_NUM, BACKLOG, &addrlen);
    if (lfd == -1) 
    {
        fprintf(stderr, "socket inetListen error\n");
        exit(EXIT_FAILURE);
    }

    while (1) 
    {
        int cfd = accept(lfd, (struct sockaddr*)&claddr, &addrlen);
        if(cfd == -1)
        {
            perror("accept");
            continue;
        }

        char buf[BUF_SIZE];
        memset(buf, 0, BUF_SIZE);

        ssize_t num_read = read(cfd, buf, BUF_SIZE);
		if (num_read == -1) 
        {
            perror("read");
            close(cfd);
            continue;
        } 
        else 
            std::cout << "Receive: " << buf << std::endl;

        if (write(cfd, "1", 1) != 1) 
        {
            perror("write");
            close(cfd);
            continue;
        }

		if (close(cfd) == -1) 
        {
			perror("close");
			exit(EXIT_FAILURE);
		}
    }

	if (shutdown(lfd, SHUT_RDWR) == -1)
    {
		perror("sutdown");
		exit(EXIT_FAILURE);
	}
	if (close(lfd) == -1) 
    {
		perror("close");
		exit(EXIT_FAILURE);
	}
}


// Test case
#ifndef SERVER_TEST

int main(int argc, char* argv[])
{
    Server server;
    server.start();

    return 0;
}

#endif
