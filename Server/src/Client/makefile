include ../../makefile.inc

CLIENT_FILE = $(SERVER_LIB) \
              ../../include/Client/GetCurrTime.h \
              ../../include/Client/ClientManager.h \
              $(CLIENT_SOURCE_FILE)

CLIENT_SOURCE_FILE = GetCurrTime.cpp \
                     ClientManager.cpp

all:
    make client

client: $(CLIENT_FILE)
    g++ -std=c++11 -DDEBUG -g -o ../../debug/client $(CLIENT_SOURCE_FILE) $(SERVER_LIB) -pthread
    g++ -std=c++11 -o ../../release/client $(CLIENT_SOURCE_FILE) $(SERVER_LIB) -pthread

clean:
    rm -rf *.o ../../debug/client ../../release/client