
DIRS = ./lib \
	   ./src/Server \
	   ./src/Client

all: 
	@ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} all) ; done
	
clean: 
	@ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} clean) ; done