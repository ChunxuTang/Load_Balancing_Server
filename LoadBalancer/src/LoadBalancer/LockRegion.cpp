/////////////////////////////////////////////////////////////////////
//  LockRegion.cpp - implementation of locking a file region
//  ver 1.0                                                        
//  Language:      standard C++                                 
//  Platform:      Ubuntu 14.04, 32-bit                               
//  Application:   2014 Summer Project                            
//  Author:        Chunxu Tang, Syracuse University
//                 chunxutang@gmail.com
/////////////////////////////////////////////////////////////////////

#include "../../include/LoadBalancer/LockRegion.h"            


//-------------------------------------------------------------------
// Lock a file region. It's static function and used only in this file.
//-------------------------------------------------------------------
static int lockReg(int fd, int cmd, int type, int whence, int start, off_t len)
{
	struct flock fl;

	fl.l_type = type;
	fl.l_whence = whence;
	fl.l_start = start;
	fl.l_len = len;

	return fcntl(fd, cmd, &fl);
}

//-------------------------------------------------------------------
// Lock a file region using nonblocking F_SETLK
//-------------------------------------------------------------------
int lockRegion(int fd, int type, int whence, int start, int len)
{
	return lockReg(fd, F_SETLK, type, whence, start, len);
}

//-------------------------------------------------------------------
// Lock a file region using blocking F_SETLKW
//-------------------------------------------------------------------
int lockRegionWait(int fd, int type, int whence, int start, int len)
{
	return lockReg(fd, F_SETLKW, type, whence, start, len);
}

//-------------------------------------------------------------------
// Unlock a file region using blocking F_SETLKW
//-------------------------------------------------------------------
int unlockRegion(int fd, int whence, int start, int len)
{
	return lockReg(fd, F_SETLKW, F_UNLCK, whence, start, len);
}

//-------------------------------------------------------------------
// Test if a file region is lockable. Return 0 if lockable, or PID of 
// process holding incompatible lock, or - 1 on error.
//-------------------------------------------------------------------
pid_t regionIsLocked(int fd, int type, int whence, int start, int len)
{
	struct flock fl;

	fl.l_type = type;
	fl.l_whence = whence;
	fl.l_start = start;
	fl.l_len = len;

	if (fcntl(fd, F_GETLK, &fl) == -1)
		return -1;

	return (fl.l_type == F_UNLCK) ? 0 : fl.l_pid;
}