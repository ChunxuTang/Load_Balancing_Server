#makefile of project LoadBalancer

DIRS = ./lib \
       ./src/ClientManager \
       ./src/RealServer \
       ./src/LoadBalancer

all: 
    @ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} all) ; done
    
clean: 
    @ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} clean) ; done
    
