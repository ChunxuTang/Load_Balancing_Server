#makefile of libhttp

HTTP_FILE = ../include/HTTP/HTTPWriter/HTTPBasic.h \
		    ../include/HTTP/HTTPWriter/HTTPWriter.h \
			../include/HTTP/HTTPReader/HTTPReader.h \
			$(HTTP_SOURCE_FILE)

HTTP_SOURCE_FILE = ../src/HTTP/HTTPWriter/HTTPWriter.cpp \
				   ../src/HTTP/HTTPWriter/RequestMessage.cpp \
				   ../src/HTTP/HTTPWriter/ResponseMessage.cpp \
				   ../src/HTTP/HTTPReader/HTTPReader.cpp \
				   ../src/HTTP/HTTPReader/ResponseHandler.cpp
				   
HTTP_LIB_FILE = ../include/Common/Interface.h \
				../include/Common/ErrorHandler.h \
				$(HTTP_LIB_SOURCE_FILE) \
				$(HTTP_FILE) 

HTTP_LIB_SOURCE_FILE = ../src/Common/ErrorHandler.cpp \
					   $(HTTP_SOURCE_FILE)
				   
all:
	make libhttp

libhttp: $(HTTP_LIB_FILE)
	g++ -g -fPIC $(HTTP_LIB_SOURCE_FILE) -std=c++11 -shared -Wl,-soname,libhttp.so.1 -o libhttp.so.1.0.1 
	mv libhttp.so.1.0.1 /usr/lib
	ln -s /usr/lib/libhttp.so.1.0.1 /usr/lib/libhttp.so.1
	ln -s /usr/lib/libhttp.so.1 /usr/lib/libhttp.so
	
clean:
	rm -rf /usr/lib/libhttp.so.1.0.1 /usr/lib/libhttp.so.1 /usr/lib/libhttp.so