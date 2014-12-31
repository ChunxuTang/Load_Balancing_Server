HTTP_FILE = ../../include/HTTP/HTTPWriter/HTTPBasic.h \
			../../include/HTTP/HTTPWriter/HTTPWriter.h \
			../../include/HTTP/HTTPReader/HTTPReader.h \
			$(HTTP_SOURCE_FILE)

HTTP_SOURCE_FILE = ../HTTP/HTTPWriter/HTTPWriter.cpp \
				   ../HTTP/HTTPWriter/RequestMessage.cpp \
				   ../HTTP/HTTPWriter/ResponseMessage.cpp \
				   ../HTTP/HTTPReader/HTTPReader.cpp \
				   ../HTTP/HTTPReader/ResponseHandler.cpp

COMMON_FILE = ../../include/Common/Interface.h \
			  ../../include/Common/ErrorHandler.h \
			  ../../include/Common/GetCurrTime.h \
			  ../../include/Common/SocketCreator.h \
			  $(COMMON_SOURCE_FILE)
			  
COMMON_SOURCE_FILE = ../Common/ErrorHandler.cpp \
					 ../Common/GetCurrTime.cpp \
					 ../Common/SocketCreator.cpp