#makefile of Summer Project

DIRS = ./ClientManager \
       ./Server \
       ./LoadBalancer

all: 
	@ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} all) ; done
	
clean: 
	@ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} clean) ; done
