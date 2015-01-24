include ../makefile.inc

all:
    make server

server: $(SERVER_FILE) Server.h Server.cpp
    g++ -g -o server $(SERVER_SOURCE_FILE) Server.cpp 

clean:
    rm -rf server *.o