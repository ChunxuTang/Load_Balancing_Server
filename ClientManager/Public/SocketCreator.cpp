/////////////////////////////////////////////////////////////////////
//  SocketCreator.cpp - implementation of ScoketCreator class   
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "SocketCreator.h"


//-------------------------------------------------------------------
// Connect to a server determined by host and service. type is used
// to judge whether it is SOCK_DGRAM orSOCK_STREAM.
//-------------------------------------------------------------------
int SocketCreator::inetConnect(const char *host,
							   const char *service, 
							   int type)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_family = AF_UNSPEC; // Allows IPv4 or IPv6 
    hints.ai_socktype = type;

    s = getaddrinfo(host, service, &hints, &result);
    if (s != 0) 
	{
		ErrorHandler eh("getaddrinfo", __FILE__, __FUNCTION__, __LINE__);
		eh.errMsg();
        errno = ENOSYS; 
        return -1;
    }

    // Walk through returned list until we find an address structure
    // that can be used to successfully connect a socket 
    for (rp = result; rp != NULL; rp = rp->ai_next) 
	{
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;                   

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1)
            break;                    

        // Connect failed: close this socket and try next address 
        close(sfd);
    }

    freeaddrinfo(result);

    return (rp == NULL) ? -1 : sfd;
}

//-------------------------------------------------------------------
// Create a passive socket. This is the public interface of inetBind()
// and inetListen().
//-------------------------------------------------------------------
int SocketCreator::inetPassiveSocket(const char *host, 
									 const char *service, 
									 int type, 
									 socklen_t *addrlen,
                                     bool do_listen, 
									 int backlog)
{
    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, optval, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_canonname = NULL;
    hints.ai_addr = NULL;
    hints.ai_next = NULL;
    hints.ai_socktype = type;
    hints.ai_family = AF_UNSPEC;  // Allows IPv4 or IPv6 
    hints.ai_flags = AI_PASSIVE;  // Use wildcard IP address 

    s = getaddrinfo(host, service, &hints, &result);
    if (s != 0) 
	{
		ErrorHandler eh("getaddrinfo", __FILE__, __FUNCTION__, __LINE__ - 2);
		eh.errExit();
    }

    // Walk through returned list until we find an address structure
    // that can be used to successfully create and bind a socket 
    optval = 1;
    for (rp = result; rp != NULL; rp = rp->ai_next) 
	{
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1)
            continue;                  

		if (do_listen) 
		{
            if (setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &optval,
                    sizeof(optval)) == -1) 
			{
				ErrorHandler eh("setsockopt", __FILE__, __FUNCTION__, __LINE__);
				eh.errMsg();
                close(sfd);
                freeaddrinfo(result);
                return -1;
            }
        }

        if (bind(sfd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;                     

        // bind() failed: close this socket and try next address 
        close(sfd);
    }

	if (rp != NULL && do_listen) 
	{
        if (listen(sfd, backlog) == -1) 
		{
			ErrorHandler eh("listen", __FILE__, __FUNCTION__, __LINE__);
			eh.errMsg();
            freeaddrinfo(result);
            return -1;
        }
    }

    if (rp != NULL && addrlen != NULL)
        *addrlen = rp->ai_addrlen;  // Return address structure size 

    freeaddrinfo(result);

    return (rp == NULL) ? -1 : sfd;
}

//-------------------------------------------------------------------
// Listen to a socket, used in a server.
//-------------------------------------------------------------------
int SocketCreator::inetListen(const char *host, 
							  const char *service, 
							  int backlog, 
							  socklen_t *addrlen)
{
    return inetPassiveSocket(host,         // host
							 service,      // service
							 SOCK_STREAM,  // type
							 addrlen,      // addrlen
							 true,         // do_listen
							 backlog);     // backlog
}

//-------------------------------------------------------------------
// Bind to an IP address and port number, used in a server.
//-------------------------------------------------------------------
int SocketCreator::inetBind(const char *host, 
							const char *service, 
							int type, 
							socklen_t *addrlen)
{
	return inetPassiveSocket(host,     // host
							 service,  // service
							 type,     // type
							 addrlen,  // addrlen
							 false,    // do_listen 
							 0);	   // backlog
}

//-------------------------------------------------------------------
// Show information of a socket.
//-------------------------------------------------------------------
char* SocketCreator::inetAddressStr(const struct sockaddr *addr, 
									socklen_t addrlen,
                                    char *addr_str, 
									int addr_strlen)
{
    char host[NI_MAXHOST], service[NI_MAXSERV];

    if (getnameinfo(addr, addrlen, host, NI_MAXHOST,
                    service, NI_MAXSERV, NI_NUMERICSERV) == 0)
        snprintf(addr_str, addr_strlen, "(%s, %s)", host, service);
    else
		snprintf(addr_str, addr_strlen, "(?UNKNOWN?)");

	addr_str[addr_strlen - 1] = '\0';
	return addr_str;
}


//-------------------------------------------------------------------
// Test Case
//-------------------------------------------------------------------
#ifdef SOCKET_CREATOR_TEST

int main(int argc, char *argv[])
{
	if (argc < 2){
		std::cout << "Usage: " << argv[0] << " <port>\n";
		exit(EXIT_SUCCESS);
	}

    SocketCreator sc;
    int ret = sc.inetConnect("127.0.0.1", NULL, argv[1], SOCK_STREAM);
    if (ret == -1)
    {
        fprintf(stderr, "inetConnect error \n");
        exit(EXIT_FAILURE);
    }

	return 0;
}

#endif
