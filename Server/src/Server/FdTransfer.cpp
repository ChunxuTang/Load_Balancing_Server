/////////////////////////////////////////////////////////////////////
//  FdTransfer.cpp - implementations of file descriptors transfer  
//  ver 1.0                                                        
//  Language:      standard C++ 11                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../include/Server/FdTransfer.h"

//-------------------------------------------------------------------
// Write a file descriptor to another process.
//-------------------------------------------------------------------
ssize_t writeFd(int fd, void *ptr, size_t nbytes, int sendfd)
{
	struct msghdr	msg;
	struct iovec	iov[1];

#ifndef	HAVE_MSGHDR_MSG_CONTROL
	union 
	{
		struct cmsghdr	cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr	*cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);

	cmptr = CMSG_FIRSTHDR(&msg);
	cmptr->cmsg_len = CMSG_LEN(sizeof(int));
	cmptr->cmsg_level = SOL_SOCKET;
	cmptr->cmsg_type = SCM_RIGHTS;
	*((int *)CMSG_DATA(cmptr)) = sendfd;
#else
	msg.msg_accrights = (caddr_t)&sendfd;
	msg.msg_accrightslen = sizeof(int);
#endif

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	return(sendmsg(fd, &msg, 0));
}

//-------------------------------------------------------------------
// Receive a file descriptor from another process.
//-------------------------------------------------------------------
ssize_t readFd(int fd, void *ptr, size_t nbytes, int *recvfd)
{
	struct msghdr	msg;
	struct iovec	iov[1];
	ssize_t			n;

#ifndef	HAVE_MSGHDR_MSG_CONTROL
	union 
	{
		struct cmsghdr	cm;
		char control[CMSG_SPACE(sizeof(int))];
	} control_un;
	struct cmsghdr *cmptr;

	msg.msg_control = control_un.control;
	msg.msg_controllen = sizeof(control_un.control);
#else
	int	newfd;

	msg.msg_accrights = (caddr_t)&newfd;
	msg.msg_accrightslen = sizeof(int);
#endif

	msg.msg_name = NULL;
	msg.msg_namelen = 0;

	iov[0].iov_base = ptr;
	iov[0].iov_len = nbytes;
	msg.msg_iov = iov;
	msg.msg_iovlen = 1;

	if ((n = recvmsg(fd, &msg, 0)) <= 0)
		return(n);

#ifndef	HAVE_MSGHDR_MSG_CONTROL
	if ((cmptr = CMSG_FIRSTHDR(&msg)) != NULL &&
		cmptr->cmsg_len == CMSG_LEN(sizeof(int))) 
	{
		if (cmptr->cmsg_level != SOL_SOCKET)
			perror("control level != SOL_SOCKET");
		if (cmptr->cmsg_type != SCM_RIGHTS)
			perror("control type != SCM_RIGHTS");
		*recvfd = *((int *)CMSG_DATA(cmptr));
	}
	else
		*recvfd = -1;		/* descriptor was not passed */
#else
	/* *INDENT-OFF* */
	if (msg.msg_accrightslen == sizeof(int))
		*recvfd = newfd;
	else
		*recvfd = -1;		/* descriptor was not passed */
	/* *INDENT-ON* */
#endif

	return(n);
}