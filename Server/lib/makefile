include ../makefile.inc

all: $(LIB_FILE)
    g++ -std=c++11 -g -c $(LIB_SOURCE_FILE)
    ar r libserver.a $(LIB_OBJECT_FILE)
    
clean:
    rm -rf *.o libserver.a