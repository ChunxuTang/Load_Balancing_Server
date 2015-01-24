
DIRS = ChildProcessWithoutIPC \
       ThreadWithLock \
       ThreadWithoutLock \
       Server

all:    
    @ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} all) ; done
    
clean: 
    @ for dir in ${DIRS}; do (cd $${dir}; ${MAKE} clean) ; done


    