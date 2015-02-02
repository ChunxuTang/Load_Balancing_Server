Load-Balancing-Server
=====================


## Include three projects:

(1) Client Manager Design  
Implement three different kinds of clients (multi-process client, multi-thread client with lock, multi-thread client without lock), and test with a simple server.  

(2) Server Design  
Implement a multi-process server, which can pre-fork a user-defined number of children to handle requests from clients. If the simultaneous requests are more than number of pre-forked children, it can fork some more children to handle requests. The maximum number of children can be forked is also user-defined. And there is a time-out, that if a new child does not have work more than the time-out, it will be removed. The server is tested by a client manager.  

(3) Load Balancing Server Design  
The core of this whole project. Implement a load balancing server which can balance loads on different real servers. It receives request from a client, sends it to a specific server according to the specific scheduling algorithm. The real sever is similar to the server designed in part 2. After a real server handles this request, the load balancing server will receive response from the real server and send it back to the client. There is also a heartbeat mechanism between load balancing server and real servers to ensure real servers are "healthy".  

## How to Run the Programs

The programs run on Linux exclusively, and have been tested on Ubuntu 14.04, 32-bit.  
Change current directory to Load Balancing Server directory.

    cd Load_Balancing_Server

And then, type

    sudo make all

In Server and Load Balancer directories, there will be two versions of executable files: debug and release. The debug version can provide more debugging messages of the programs and may run a little bit slower. The release version can directly be used and provide less debugging information.  
In ClientManger directory, there will only be executable files that can be directly used.

