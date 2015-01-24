include ../../makefile.inc

SERVER_FILE = $(SERVER_LIB) \
              ../../include/Server/FdHandler.h \
              ../../include/Server/FdTransfer.h \
              ../../include/Server/Server.h \
              $(SERVER_SOURCE_FILE)

SERVER_SOURCE_FILE = FdHandler.cpp \
                     FdTransfer.cpp \
                     Server.cpp

all:
    make server

server: $(SERVER_FILE)
    g++ -std=c++11 -DDEBUG -g -o ../../debug/server $(SERVER_SOURCE_FILE) $(SERVER_LIB)
    g++ -std=c++11 -o ../../release/server $(SERVER_SOURCE_FILE) $(SERVER_LIB)

clean:
    rm -rf *.o ../../debug/server ../../release/server

