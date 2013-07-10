#pragma once
#ifndef __GameServer_H__
#define __GameServer_H__

class GameServer
{
public:
	GameServer(bool bNetworkEnabled);
	~GameServer();

private:
	bool Start();
	static void BroadcastPingPacket(void *);

	SOCKET			listener;		// listening socket descriptor
	SOCKET			nUdpSocket;

	Thread *		m_pThread;
	static void GLFWCALL ThreadFunction(void * pArgument);
};

#endif // __GameServer_H__
