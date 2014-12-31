#makefile of RealServer

include ../../makefile.inc

SERVER_FILE = $(COMMON_FILE) \
			  $($HTTP_FILE) \
			  ../../include/Common/FdHandler.h \
			  ../../include/RealServer/Server.h

SERVER_SOURCE_FILE = $(COMMON_SOURCE_FILE) \
					 ../Common/FdHandler.cpp \
					 ./Server.cpp
					 
all:
	make server
	
server: $(SERVER_FILE)
	g++ -g -DDEBUG -std=c++11 -o ../../debug/server $(SERVER_SOURCE_FILE) /usr/lib/libhttp.so
	g++ -std=c++11 -o ../../release/server $(SERVER_SOURCE_FILE) /usr/lib/libhttp.so

clean:
	rm -rf ../../debug/server ../../release/server