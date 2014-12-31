#makefile of LoadBalancer

include ../../makefile.inc

SCHED_FILE = ../../include/SchedulingAlgorithms/SchedAlgorithms.h

SCHED_SOURCE_FILE = ../SchedulingAlgorithms/AlgorithmSelector.cpp \
					../SchedulingAlgorithms/SchedRR.cpp \
					../SchedulingAlgorithms/SchedWRR.cpp \
					../SchedulingAlgorithms/SchedLC.cpp \
					../SchedulingAlgorithms/SchedWLC.cpp \
					../SchedulingAlgorithms/SchedDH.cpp \
					../SchedulingAlgorithms/SchedSH.cpp

BALANCER_FILE = $(COMMON_FILE) \
			    $($HTTP_FILE) \
				$(SCHED_FILE) \
			    ../../include/Common/FdHandler.h \
				../../include/LoadBalancer/CreatePidFile.h \
				../../include/LoadBalancer/LockRegion.h \
				../../include/LoadBalancer/LoadBalancer.h
				
BALANCER_SOURCE_FILE = $(COMMON_SOURCE_FILE) \
					   $(SCHED_SOURCE_FILE) \
					   ../Common/FdHandler.cpp \
					   ./CreatePidFile.cpp \
					   ./LockRegion.cpp \
					   ./LoadBalancer.cpp
					   
all:
	make balancer

balancer: $(BALANCER_FILE)
	g++ -g -DDEBUG -std=c++11 -o ../../debug/balancer $(BALANCER_SOURCE_FILE) /usr/lib/libhttp.so
	g++ -std=c++11 -o ../../release/balancer $(BALANCER_SOURCE_FILE) /usr/lib/libhttp.so
	
clean:
	rm -rf ../../debug/balancer ../../release/balancer