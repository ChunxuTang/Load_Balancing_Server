/////////////////////////////////////////////////////////////////////
//  FdHandler.cpp - implementation of handling fds
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////


#include "../../include/Common/FdHandler.h"

//-------------------------------------------------------------------
// Set an fd NONBLOCK
//-------------------------------------------------------------------
int setNonBlocking(int fd)
{
	int old_option;
	if ((old_option = fcntl(fd, F_GETFL)) == -1)
	{
		perror("fcntl - F_GETFL");
		exit(EXIT_FAILURE);
	}

	int new_option = old_option | O_NONBLOCK;
	if (fcntl(fd, F_SETFL, new_option) == -1)
	{
		perror("fcntl - F_SETFL");
		exit(EXIT_FAILURE);
	}

	return old_option;
}

//-------------------------------------------------------------------
// Disable NONBLOCK of a file descriptor
//-------------------------------------------------------------------
int disableNonBlocking(int fd)
{
	int old_option;
	if ((old_option = fcntl(fd, F_GETFL)) == -1)
	{
		perror("fcntl - F_GETFL");
		exit(EXIT_FAILURE);
	}

	int new_option = old_option & ~O_NONBLOCK;
	if (fcntl(fd, F_SETFL, new_option) == -1)
	{
		perror("fcntl - F_SETFL");
		exit(EXIT_FAILURE);
	}

	return old_option;
}

//-------------------------------------------------------------------
// Set an fd CLOEXEC. CLOEXEC means the fd will be closed automatically 
// in a child process if it invokes exec() functions
//-------------------------------------------------------------------
int setCloseOnExec(int fd)
{
	int old_option;
	if ((old_option = fcntl(fd, F_GETFL)) == -1)
	{
		perror("fcntl - F_GETFL");
		exit(EXIT_FAILURE);
	}

	int new_option = old_option | FD_CLOEXEC;
	if (fcntl(fd, F_SETFL, new_option) == -1)
	{
		perror("fcntl - F_SETFL");
		exit(EXIT_FAILURE);
	}

	return old_option;
}

//-------------------------------------------------------------------
// Disable CLOEXEC of a file descriptor
//-------------------------------------------------------------------
int disableCloseOnExec(int fd)
{
	int old_option;
	if ((old_option = fcntl(fd, F_GETFL)) == -1)
	{
		perror("fcntl - F_GETFL");
		exit(EXIT_FAILURE);
	}

	int new_option = old_option & ~FD_CLOEXEC;
	if (fcntl(fd, F_SETFL, new_option) == -1)
	{
		perror("fcntl - F_SETFL");
		exit(EXIT_FAILURE);
	}

	return old_option;
}

//-------------------------------------------------------------------
// Add an fd to an epoll fd monitoring list
//-------------------------------------------------------------------
void addEvent(int epollfd, int fd, OneShotType oneshot_type, BlockType block_type)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN; // Level triggered

	if (oneshot_type == ONESHOT)
		ev.events |= EPOLLONESHOT;

	if (epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev) == -1)
	{
		perror("epoll_ctl - EPOLL_CTL_ADD");
		exit(EXIT_FAILURE);
	}

	if (block_type == NON_BLOCK)
		setNonBlocking(fd);
}

//-------------------------------------------------------------------
// Delete an fd from an epoll fd monitoring list
//-------------------------------------------------------------------
void deleteEvent(int epollfd, int fd)
{
	struct epoll_event ev;
	if (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev) == -1)
		perror("epoll_ctl - EPOLL_CTL_DEL");
}

//-------------------------------------------------------------------
// Set an fd EPOLLONESHOT. EPOLLONESHOT means that one fd will only
// trigger once.
//-------------------------------------------------------------------
void setOneshot(int epollfd, int fd)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;

	if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == -1)
	{
		perror("epoll_ctl - EPOLL_CTL_MOD");
		exit(EXIT_FAILURE);
	}
}

//-------------------------------------------------------------------
// Disable EPOLLONESHOT of a file descriptor
//-------------------------------------------------------------------
void disableOneShot(int epollfd, int fd)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN | EPOLLET;

	if (epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &ev) == -1)
	{
		perror("epoll_ctl - EPOLL_CTL_MOD");
		exit(EXIT_FAILURE);
	}
}