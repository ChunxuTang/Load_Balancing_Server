include ../makefile.inc

all:
	make client

client: $(PUBLIC_CLIENT_FILE) ClientManager.h ClientManager.cpp
	g++ -g -o client $(PUBLIC_CLIENT_SOURCE_FILE) ClientManager.cpp -pthread

clean:
	rm -rf client *.o
