/////////////////////////////////////////////////////////////////////
//  Server.cpp - implementations of a server  
//  ver 1.0                                                        
//  Language:      standard C++ 11                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../include/Server/Server.h"


const char* Server::PORT_NUM = "50000";
const char* Server::BIND_ADDRESS = "127.0.0.1";
int Server::child_pfd_ = 0;

//-------------------------------------------------------------------
// Constructor
// Initialize data members. max_children_ is assigned by user.
//-------------------------------------------------------------------
Server::Server(int max_children)
    :max_children_(max_children)
{
    if(max_children_ < PREFORKED_CHILDREN) 
	{
        fprintf(stderr, "#max children is too small.\n");
        exit(EXIT_FAILURE);
    }

	children_exist_ = PREFORKED_CHILDREN;
	children_free_ = 0;

	listen_fd_ = 0;
	epoll_fd_ = 0;
	signal_fd_ = 0;
	FD_ZERO(&timer_fds_);
	server_stop_ = false;

	ts_.it_interval.tv_sec = 0;
	ts_.it_interval.tv_nsec = 0;
	ts_.it_value.tv_sec = TEMPORARY_CHILD_TIME_OUT;
	ts_.it_value.tv_nsec = 0;
}

Server::~Server()
{
	max_children_ = 0;
	children_exist_ = 0;
	children_free_ = 0;

	listen_fd_ = 0;
	epoll_fd_ = 0;
	signal_fd_ = 0;
	FD_ZERO(&timer_fds_);
	server_stop_ = false;

	ts_.it_interval.tv_sec = 0;
	ts_.it_interval.tv_nsec = 0;
	ts_.it_value.tv_sec = 0;
	ts_.it_value.tv_nsec = 0;
}

//-------------------------------------------------------------------
// Use epoll_create() function to produce an epoll fd.
//-------------------------------------------------------------------
Status Server::initEpollfd()
{
	epoll_fd_ = epoll_create(MAX_EVENTS);
	if (epoll_fd_ == -1)
	{
		ErrorHandler eh("epoll_create", __FILE__, __FUNCTION__, __LINE__ - 1);
		eh.errMsg();
		return FATAL_ERROR;
	}

	return SUCCESS;
}

//-------------------------------------------------------------------
// Bind and listen to an IP address and port number, produce a listen
// fd.
//-------------------------------------------------------------------
Status Server::initListenfd()
{
	socklen_t addrlen;
	SocketCreator sc;

	listen_fd_ = sc.inetListen(BIND_ADDRESS, PORT_NUM, BACKLOG, &addrlen);
	if (listen_fd_ == -1)
	{
		fprintf(stderr, "socket inetListen error\n");
		close(epoll_fd_);
		return FATAL_ERROR;
	}

	addEvent(epoll_fd_, listen_fd_, NON_ONESHOT, NON_BLOCK);

	DebugCode(std::cout << "listen_fd_ = " << listen_fd_ << std::endl;)

	return SUCCESS;
}

//-------------------------------------------------------------------
// Produce a signal fd, and add SIGCHLD, SIGTERM and SIGINT into this 
// file descriptor.
//-------------------------------------------------------------------
Status Server::initSignalfd()
{
	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGCHLD);
	sigaddset(&mask, SIGINT);
	sigaddset(&mask, SIGTERM);

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

	addEvent(epoll_fd_, signal_fd_, OneShotType::NON_ONESHOT, BlockType::BLOCK);
	DebugCode(std::cout << "signal_fd_ = " << signal_fd_ << std::endl;)

	return SUCCESS;
}

//-------------------------------------------------------------------
// The entry point of a server's operations. This function can invoke
// others functions to work.
//-------------------------------------------------------------------
void Server::start()
{
	if (initEpollfd() == FATAL_ERROR ||
		initListenfd() == FATAL_ERROR)
		return;

	for (int index = 0; index < PREFORKED_CHILDREN; index++)
	{
		if (forkChild(index) == FATAL_ERROR)
			return;
	}
    sleep(1); // wait for all children are forked, can be deleted

	if (initSignalfd() == FATAL_ERROR)
	{
		clearAll();
		return;
	}

	std::cout << "Server can receive requests now.\n";

	struct epoll_event evlist[MAX_EVENTS];
    int ready;

	// This while loop is used to handle requests from load balancer, 
	// handle finish message from children, handle signals, and handle
	// timers' alarm.
    while(!server_stop_)
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
				server_stop_ = true;
				break;
			}
		}
        DebugCode(std::cout << "Server gets " << ready << " requests" << std::endl;)

        for(int i = 0; i < ready; i++)
        {

            DebugCode(printf("\tfd=%d; events: %s%s%s\n", evlist[i].data.fd,
            		  (evlist[i].events & EPOLLIN) ? "EPOLLIN " : "",
            		  (evlist[i].events & EPOLLHUP) ? "EPOLLHUP " : "",
            		  (evlist[i].events & EPOLLERR) ? "EPOLLERR " : "");)

            int trigger_fd = evlist[i].data.fd;

			// handle requests from a client
            if((trigger_fd == listen_fd_) & evlist[i].events & EPOLLIN)
			{ 
				if (handleRequestFromClient() == FATAL_ERROR)
				{
					server_stop_ = true;
					break;
				}
               
            }
			// handle timers' alarm
			else if ((FD_ISSET(trigger_fd, &timer_fds_)) & evlist[i].events & EPOLLIN)
			{
				if (handleChildTimeOut(trigger_fd) == FATAL_ERROR)
				{
					server_stop_ = true;
					break;
				}
			}
			// handle signals
			else if ((trigger_fd == signal_fd_) & evlist[i].events & EPOLLIN)
			{
				if (serverSigHandler() == FATAL_ERROR)
				{
					server_stop_ = true;
					break;
				}
			}
			// handle children's finish messages
            else if (evlist[i].events & EPOLLIN)
			{ 
				DebugCode(std::cout << "Server gets that child " << trigger_fd << " finished\n";)

				Status ret = handleResponseFromChild(trigger_fd);
				if (ret == MINOR_ERROR)
					continue;
				else if (ret == FATAL_ERROR)
				{
					server_stop_ = true;
					break;
				}

            }
			// Occur an error
            else 
			{
                std::cout << "Error\n";
				server_stop_ = true;
				break;
            }
        } // end - if statement used to determine trigger_fd
    } // end - main while loop

	clearAll();
}

//-------------------------------------------------------------------
// Handle requests from a client. 
// The server first accept listen fd and get a socket fd. Then the sever
// check child pool to find whether there is a free child or not. If 
// there is one, the server will send the socket fd to it by UNIX domain
// socket. If no children is free, and the number of existed children
// is less than max permitted children, the server will fork a new
// child and send the socket fd to it. Otherwise, the server will send
// back a fail message.
//-------------------------------------------------------------------
Status Server::handleRequestFromClient()
{
	socklen_t addrlen;
	struct sockaddr_storage claddr;
	int cfd;

	addrlen = sizeof(struct sockaddr_storage);
	cfd = accept(listen_fd_, (struct sockaddr*)&claddr, &addrlen);
	if (cfd == -1){
		ErrorHandler eh("accept", __FILE__, __FUNCTION__, __LINE__);
		eh.errMsg();
		return FATAL_ERROR;
	}

	children_free_ = 0;
	VectorPos first_free_child = 0;

	// Traverse the children vector to check the number of children
	// which are not working, and find the position of the first
	// free child.
	for (VectorPos i = 0; i < child_pool_.size(); i++)
	{
		if (child_pool_.at(i).child_status == FREE)
		{
			if (children_free_ == 0)
				first_free_child = i;
			children_free_++;
		}
	}
	DebugCode(std::cout << "children_free_ = " << children_free_ << std::endl;)

	// There are some children that are not working. Send the socket fd
	// to the first free child.
	if (children_free_ > 0)
	{
		char c = '0';
		writeFd(child_pool_.at(first_free_child).child_spipe_fd[1], &c, 1, cfd);
		child_pool_.at(first_free_child).child_status = BUSY;
		close(cfd);
	}
	// No free children, but children_exist_ doesn't reach limit. Fork
	// a new child and send the socket fd to it.
	else if (children_exist_ < max_children_)
	{
		DebugCode(std::cout << "children_exist_ = " << children_exist_ << std::endl;)

		if (forkChild(children_exist_) == FATAL_ERROR)
			return FATAL_ERROR;

		if (addTimer(children_exist_) == FATAL_ERROR)
			return FATAL_ERROR;

		char c = '0';
		VectorPos i = children_exist_;
		writeFd(child_pool_.at(i).child_spipe_fd[1], &c, 1, cfd);
		child_pool_.at(i).child_status = BUSY;
		close(cfd);
		children_exist_++;
	}
	// No free children and the server reaches limit. 
	else
	{
		std::cout << "Server has reached max children limit.\n";
		const char *fail = "0";
		if (write(cfd, fail, strlen(fail)) == -1)
		{
			ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__ - 1);
			eh.errMsg();
			close(cfd);
		}

	}
}

//-------------------------------------------------------------------
// Handle finish message from a child.
// When a child exits, this function can also be invoked and the child
// spipe_fd will be removed.
//-------------------------------------------------------------------
Status Server::handleResponseFromChild(int trigger_fd)
{
	ssize_t num_read;
	struct ChildInfo result;

	num_read = read(trigger_fd, &result, sizeof(ChildInfo));
	if (num_read == -1)
	{
		ErrorHandler eh("read", __FILE__, __FUNCTION__, __LINE__);
		eh.errMsg();
		return FATAL_ERROR;
	}
	if (num_read == 0)
	{
		fprintf(stderr, "server read - end of file\n");
		fprintf(stderr, "A child may exit.\n");

		deleteEvent(epoll_fd_, trigger_fd);

		// Set status to BUSY to avoid child process terminate unexpectedly 
		// when it doesn't work, while server does not know and may still 
		// send a task to the child.
		for (auto &x : child_pool_)
		{
			if (x.child_spipe_fd[1] == trigger_fd)
				x.child_status = BUSY;
		}

		return MINOR_ERROR;
	}

	if (result.child_pid > 0)
	{
		int index = result.child_index;
		child_pool_[index].child_status = FREE;

		DebugCode(std::cout << "the finished child pid = " << result.child_pid << std::endl;)

		// If the child is one forked afterwards, its timer should be reset.
		if (index >= PREFORKED_CHILDREN)
		{
			DebugCode(std::cout << "Reset the timer\n";)
			if (timerfd_settime(child_pool_[index].child_timer_fd, 0, &ts_, NULL) == -1)
			{
				ErrorHandler eh("timerfd_settime", __FILE__, __FUNCTION__, __LINE__);
				eh.errMsg();
				return FATAL_ERROR;
			}
		}

		DebugCode(listChildrenAvail();)
	}

	return SUCCESS;
}

//-------------------------------------------------------------------
// Handle time out messages.
// When a timer alarms, find corresponding child and remove it.
//-------------------------------------------------------------------
Status Server::handleChildTimeOut(int trigger_fd)
{
	DebugCode(std::cout << trigger_fd << " Time is up!\n";)

	std::vector<ChildInfo>::iterator it = child_pool_.begin();
	std::advance(it, PREFORKED_CHILDREN);

	for (; it != child_pool_.end(); it++)
	{
		// Find corresponding child with same timer fd
		if (it->child_timer_fd == trigger_fd)
		{
			DebugCode(std::cout << "kill a child " << it->child_pid << std::endl;)

			kill(it->child_pid, SIGINT);

			FD_CLR(trigger_fd, &timer_fds_);
			deleteEvent(epoll_fd_, trigger_fd);
			deleteEvent(epoll_fd_, it->child_spipe_fd[1]);
			close(trigger_fd);
			close(it->child_spipe_fd[1]);

			children_exist_--;

			// After this action, the iterator will has no effect. 
			// Because next statement is break, it will jump the for 
			// loop, there's may not an error. In C++ STL, erase() 
			// will automatically return the iterator pointing to 
			// next element.
			child_pool_.erase(it);

			break;
		}
	}

	return SUCCESS;
}

//-------------------------------------------------------------------
// Handle signals.
// The signal handler can catch SIGCHLD, SIGTERM and SIGINT. When SIGINT 
// and SIGTERM arrives, the server will clear all the resource and exits. 
// When SIGCHLD is caught, it means that a child terminates. Resources 
// of that child will be cleared.
//-------------------------------------------------------------------
Status Server::serverSigHandler()
{
	struct signalfd_siginfo fdsi;

	ssize_t s = read(signal_fd_, &fdsi, sizeof(struct signalfd_siginfo));
	if (s == -1)
	{
		ErrorHandler eh("read", __FILE__, __FUNCTION__, __LINE__);
		eh.errMsg();
		return FATAL_ERROR;
	}

	DebugCode(std::cout << "Signal is sent from " << fdsi.ssi_pid << std::endl;)

	int received_signal = fdsi.ssi_signo;
	switch (received_signal)
	{
	case SIGCHLD:
	{
		DebugCode(std::cout << "catch SIGCHLD\n";)

		std::vector<ChildInfo>::iterator it;
		it = child_pool_.begin();
		for (; it != child_pool_.end(); it++)
		{
			if (it->child_pid == fdsi.ssi_pid)
			{
				// A pre-forked child terminates. In this situation, a new child should 
				// be forked.
				if (it->child_index < PREFORKED_CHILDREN)
				{
					DebugCode(std::cout << "an child (0-PREFORKED_CHILDREN) exit unexpectedlly";)
					DebugCode(std::cout << "the child's pipe is " << it->child_spipe_fd[1] << std::endl;)

					// Here child_spipe_fd needn't be removed from epollfd monitoring
					// list, because one child termination first invokes EOF of server
					// pipe, at that time, this fd has been removed.
					close(it->child_spipe_fd[1]);
					if (updateChild(it->child_index) == FATAL_ERROR)
					{
						server_stop_ = true;
						return FATAL_ERROR;
					}
					break;
				}
				else
				{
					// A child forked afterwards terminates, its resource should be cleared.
					DebugCode(std::cout << "a child(>PREFORKED_CHILDREN) terminates unexpectedlly\n";)
					DebugCode(std::cout << "the child pid = " << it->child_pid << std::endl;)

					FD_CLR(it->child_timer_fd, &timer_fds_);
					deleteEvent(epoll_fd_, it->child_timer_fd);
					close(it->child_timer_fd);
					close(it->child_spipe_fd[1]);

					children_exist_--;
					child_pool_.erase(it);
					break;
				}
			}

		}
		break;
	}
	case SIGTERM: // SIGTERM handler is same with SIGINT's

	case SIGINT:
		std::cout << "Server is interrupted\n";
		server_stop_ = true;
		clearAll();
		exit(EXIT_SUCCESS);
		break;
	default:
		std::cout << "Unknown signal: " << received_signal << std::endl;
		break;
	}

	return SUCCESS;
}

//-------------------------------------------------------------------
// Add timer for every children forked when there are too many
// requests. When a timer alarms, the corresponding child should be
// removed.
//-------------------------------------------------------------------
Status Server::addTimer(int index)
{
	int timer_fd = timerfd_create(CLOCK_REALTIME, 0);
	if (timer_fd == -1)
	{
		ErrorHandler eh("timerfd_create", __FILE__, __FUNCTION__, __LINE__);
		eh.errMsg();
		return FATAL_ERROR;
	}

	if (timerfd_settime(timer_fd, 0, &ts_, NULL) == -1)
	{
		ErrorHandler eh("timerfd_settime", __FILE__, __FUNCTION__, __LINE__);
		eh.errMsg();
		return FATAL_ERROR;
	}

	DebugCode(std::cout << "timer fd created = " << timer_fd << std::endl;)

	FD_SET(timer_fd, &timer_fds_);
	addEvent(epoll_fd_, timer_fd, NON_ONESHOT, BLOCK);

	child_pool_.at(index).child_timer_fd = timer_fd;

	return SUCCESS;
}

//-------------------------------------------------------------------
// Clear all the resources used by the server.
//-------------------------------------------------------------------
void Server::clearAll()
{
	close(epoll_fd_);
	close(signal_fd_);

	for (VectorPos i = 0; i < child_pool_.size(); i++)
	{
		kill(child_pool_[i].child_pid, SIGINT);
		DebugCode(std::cout << "server kill " << child_pool_[i].child_pid << std::endl;)

		close(child_pool_[i].child_spipe_fd[1]);

		if (child_pool_[i].child_timer_fd != 0)
			close(child_pool_[i].child_timer_fd);
	}

	// Wait until all of the children exit
	while (wait(NULL) != -1)
		continue;
	if (errno != ECHILD)
		perror("wait");

	std::cout << "server shutdown...\n";
	shutdown(listen_fd_, SHUT_RDWR);
	close(listen_fd_);

	DebugCode(std::cout << "close epoll_fd_ " << epoll_fd_ << std::endl;
			  std::cout << "close signal_fd_ " << signal_fd_ << std::endl;
			  std::cout << "close listen_fd_ " << listen_fd_ << std::endl;)
}

//-------------------------------------------------------------------
// Child main function.
// A child receives a socket fd from the server, reads the fd, and get
// a number. Then the child will sleep for that seconds, simulating
// work. After sleep, the child will generate a random number from
// 1 to CHILD_EXIT_PROBABILITY. If it is 1, the child will exit.
//-------------------------------------------------------------------
void Server::childWork(ChildInfo &child_info)
{
	signal(SIGINT, childSigHandler);
	
    child_pfd_ = child_info.child_spipe_fd[0]; 
	ssize_t num_read;
    int cfd;
    char c;
	srand(time(NULL));

    while (true)
    {
		// Child gets the client's socket fd.
        num_read = readFd(child_info.child_spipe_fd[0], &c, 1, &cfd);
		if (num_read == -1)
		{
			ErrorHandler eh("readFd", __FILE__, __FUNCTION__, __LINE__);
			eh.errMsg();
			return;
        }
		if (num_read == 0)
		{
			fprintf(stderr, "%d, Server stream pipe is closed.\n", child_info.child_pid);
			return;
        }

        if(cfd > 0)
        {
            char buf[BUF_SIZE];
            memset(buf, 0, BUF_SIZE);

			ssize_t num_read = read(cfd, buf, BUF_SIZE);
			if (num_read == -1) 
			{
				ErrorHandler eh("read", __FILE__, __FUNCTION__, __LINE__ - 2);
				eh.errMsg();
				return;
            } 

            int sleep_num = atoi(buf);
			std::cout << "Child " << getpid() << " sleeps for " << sleep_num << " seconds\n";
			sleep(sleep_num);

            const char *success = "1";
            if (write(cfd, success, strlen(success)) == -1) 
			{
				ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__ - 1);
				eh.errMsg();
				return;
            }

			// close the socket fd between child and client
            if (close(cfd) == -1) 
			{
				ErrorHandler eh("close", __FILE__, __FUNCTION__, __LINE__ - 1);
				eh.errMsg();
				return;
            }

			// Send finish message to the server. Then server could assign new
			// request to this child.
            if (write(child_info.child_spipe_fd[0], &child_info, sizeof(ChildInfo)) == -1) 
			{
				ErrorHandler eh("write", __FILE__, __FUNCTION__, __LINE__ - 1);
				eh.errMsg();
				return;
            }

			// This is used to simulate a child's unexpected termination. But
			// here may be a conflict. When a child terminates unexpectedly, my
			// method is to delete child_spipe_fd[1] from epoll_fd_ when child_spipe_fd[1]
			// is triggered and close child_spipe_fd[1] when server catches SIGCHLD.
			// It depends on the order that trigger of child_spipe_fd[1] should arrive
			// earlier than SIGCHLD. This is always the case, but if the order is 
			// reversed, there will be a "bad file descriptor" error in epoll_ctl(). 
			// To avoid this happen, I add sleep(1).
			
			int num = rand() % CHILD_EXIT_PROBABILITY + 1;
			if (num == 1)
			{
				close(child_pfd_);
				sleep(1);
				exit(EXIT_SUCCESS);
			}
        }
    }
}

//-------------------------------------------------------------------
// When a pre-forked child terminates, this function will be invoked
// to fork a new child to replace terminated one.
//-------------------------------------------------------------------
Status Server::updateChild(int index)
{
    child_pool_[index].child_status = FREE;
    child_pool_[index].child_timer_fd = 0;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, child_pool_[index].child_spipe_fd) == -1)
	{
		ErrorHandler eh("socketpair", __FILE__, __FUNCTION__, __LINE__);
		eh.errMsg();
		return FATAL_ERROR;
	}
	addEvent(epoll_fd_, child_pool_[index].child_spipe_fd[1], NON_ONESHOT, BLOCK);

    pid_t child_pid;
    switch (child_pid = fork())
	{
	case -1:
		perror("fork");
		return FATAL_ERROR;
	case 0:  // child
		DebugCode(std::cout << "update a (0-4)child " << getpid() << std::endl;)

		// Some useless fds should be closed in the child
		close(child_pool_.at(index).child_spipe_fd[1]);
		close(epoll_fd_);
		close(signal_fd_);

		child_pool_.at(index).child_pid = getpid();
		childWork(child_pool_.at(index));
		_exit(EXIT_SUCCESS);
	default:   // parent
		close(child_pool_.at(index).child_spipe_fd[0]);
		child_pool_.at(index).child_pid = child_pid;
		break;

	}
	// wait for the child be forked
	sleep(1);

	return SUCCESS;
}

//-------------------------------------------------------------------
// Fork a new child.
// This function is used at the beginning of a server or when there
// are too many requests and the server needs to fork a new one. 
//-------------------------------------------------------------------
Status Server::forkChild(int index)  
{
	ChildInfo child_info;
	child_info.child_index = index;
	child_info.child_pid = 0;
	child_info.child_status = FREE;
	child_info.child_timer_fd = 0;

	if (socketpair(AF_UNIX, SOCK_STREAM, 0, child_info.child_spipe_fd) == -1)
	{
		ErrorHandler eh("socketpair", __FILE__, __FUNCTION__, __LINE__);
		eh.errMsg();
		return FATAL_ERROR;
	}
	addEvent(epoll_fd_, child_info.child_spipe_fd[1], NON_ONESHOT, BLOCK);

	pid_t child_pid;
	switch (child_pid = fork())
	{
	case -1:
		perror("fork");
		exit(EXIT_FAILURE);
	case 0:  // child
		DebugCode(std::cout << "fork a child " << getpid() << std::endl;)
		close(child_info.child_spipe_fd[1]);
		close(listen_fd_); 
		child_info.child_pid = getpid();
		childWork(child_info);
		_exit(EXIT_SUCCESS);
	default:  // parent
		close(child_info.child_spipe_fd[0]); 
		child_info.child_pid = child_pid;
		break;
	}

	child_pool_.push_back(child_info);

	// wait for the child be forked
	sleep(1);

	return SUCCESS;
}

//-------------------------------------------------------------------
// Handle SIGINT of a child.
//-------------------------------------------------------------------
void Server::childSigHandler(int sig)
{
    std::cout << "child " << getpid() << " is killed.\n";
    close(child_pfd_);
    exit(EXIT_SUCCESS);
}

//-------------------------------------------------------------------
// List available children. "Available" means their status are "FREE",
// and can be sent requests.
//-------------------------------------------------------------------
void Server::listChildrenAvail()
{
	std::cout << std::left << std::setw(12) << "Child PID"
		<< std::setw(14) << "Child Status"
		<< std::setw(14) << "Child Index"
		<< std::setw(18) << "Child Pipe fd[1]"
		<< std::setw(18) << "Child Timer fd" << std::endl;

	for (auto x : child_pool_)
	{
		if (x.child_status == FREE)
		{
			std::cout << std::left << std::setw(12) << x.child_pid
				<< std::setw(14) << x.child_status
				<< std::setw(14) << x.child_index
				<< std::setw(18) << x.child_spipe_fd[1]
				<< std::setw(18) << x.child_timer_fd << std::endl;
		}
	}
}

//-------------------------------------------------------------------
// List all the children of the server.
//-------------------------------------------------------------------
void Server::listChildren()
{
	std::cout << std::left << std::setw(12) << "Child PID"
			  << std::setw(14) << "Child Status"
			  << std::setw(14) << "Child Index"
			  << std::setw(18) << "Child Pipe fd[1]"
			  << std::setw(18) << "Child Timer fd" << std::endl;

	for (auto x : child_pool_)
	{
		std::cout << std::left << std::setw(12) << x.child_pid
				  << std::setw(14) << x.child_status
				  << std::setw(14) << x.child_index
				  << std::setw(18) << x.child_spipe_fd[1]
				  << std::setw(18) << x.child_timer_fd << std::endl;

	}
}


//-------------------------------------------------------------------
// Test Case
//-------------------------------------------------------------------
#ifndef SERVER_TEST

int main(int argc, char *argv[])
{
    if(argc < 2)
	{
        std::cout << "Usage: " << argv[0] << " <#max children>\n";
        exit(EXIT_SUCCESS);
    }

    int max_children = atoi(argv[1]);
	Server server(max_children);
    server.start();

	return 0;
}

#endif
