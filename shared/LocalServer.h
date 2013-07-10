#pragma once
#ifndef __LocalServer_H__
#define __LocalServer_H__

class LocalServer
{
public:
	LocalServer();
	~LocalServer();

private:
	bool Start(void);
	static void BroadcastPingPacket(void *);

	SOCKET			listener;		// listening socket descriptor
	SOCKET			nUdpSocket;

	Thread *		m_pThread;
	static void GLFWCALL ThreadFunction(void * pArgument);
};

#endif // __LocalServer_H__
