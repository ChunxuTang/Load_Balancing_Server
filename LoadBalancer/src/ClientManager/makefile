#makefile of ClientManager

include ../../makefile.inc

CLIENT_FILE = $(COMMON_FILE) \
			  $(HTTP_FILE) \
			  ../../include/ClientManager/Cache.h \
			  ../../include/ClientManager/ClientManager.h
			  
CLIENT_SOURCE_FILE = $(COMMON_SOURCE_FILE) \
					 ./ClientManager.cpp

all:
	make client

client: $(CLIENT_FILE)
	g++ -g -DDEBUG -std=c++11 -pthread -o ../../debug/client $(CLIENT_SOURCE_FILE) /usr/lib/libhttp.so 
	g++ -std=c++11 -pthread -o ../../release/client $(CLIENT_SOURCE_FILE) /usr/lib/libhttp.so 
	
clean:
	rm -rf ../../debug/client ../../release/client
