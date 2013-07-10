#pragma once
#ifndef __LocalServerConnection_H__
#define __LocalServerConnection_H__

class LocalServerConnection
	: public ServerConnection
{
public:
	LocalServerConnection();
	~LocalServerConnection();

	bool Connect(const char * szHostname, u_short nPort);

	bool SendTcp(CPacket & oPacket, JoinStatus nMinimumJoinStatus = IN_GAME);
	bool SendUdp(CPacket & oPacket, JoinStatus nMinimumJoinStatus = IN_GAME);

	bool IsLocal() { return true; }

	/*void GenerateSignature();

	u_char		cLastUpdateSequenceNumber;*/
};

#endif // __LocalServerConnection_H__
