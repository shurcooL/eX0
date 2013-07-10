#pragma once
#ifndef __ServerConnection_H__
#define __ServerConnection_H__

class ServerConnection
	: public NetworkConnection
{
public:
	ServerConnection();
	~ServerConnection();

	bool Connect(const char * szHostname, u_short nPort);
	void GenerateSignature();

	u_char		cLastUpdateSequenceNumber;
};

#endif // __ServerConnection_H__
