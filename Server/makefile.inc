COMMONE_FILE = ../../include/Common/Interface.h \
			   ../../include/Common/ErrorHandler.h \
			   ../../include/Common/SocketCreator.h \
			   $(COMMON_SOURCE_FILE)
			   
COMMON_SOURCE_FILE = ../Common/ErrorHandler.cpp \
					 ../Common/SocketCreator.cpp 

LIB_FILE = ../include/Common/Interface.h \
		   ../include/Common/ErrorHandler.h \
		   ../include/Common/SocketCreator.h \
		   $(LIB_SOURCE_FILE)
		 
LIB_SOURCE_FILE = ../src/Common/ErrorHandler.cpp \
				  ../src/Common/SocketCreator.cpp 
				  
LIB_OBJECT_FILE = ./ErrorHandler.o \
				  ./SocketCreator.o
				  
SERVER_LIB = ../../lib/libserver.a