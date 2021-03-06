#pragma once
#ifndef __NetworkIncludes_H__
#define __NetworkIncludes_H__

#ifdef WIN32
#	include <winsock2.h>
//#	include <ws2tcpip.h>

#	ifndef socklen_t
	typedef int socklen_t;
#	endif

#	include "ws-util.h"
#else // Linux
#	include <unistd.h>
#	include <errno.h>
#	include <netdb.h>
#	include <sys/types.h>
#	include <netinet/in.h>
#	include <netinet/tcp.h>
#	include <sys/socket.h>
#	include <arpa/inet.h>
#	include <signal.h>

	typedef int SOCKET;

#	define INVALID_SOCKET	(-1)
#	define SOCKET_ERROR		(-1)

#	define SD_BOTH			SHUT_RDWR
#endif

#endif // __NetworkIncludes_H__
