#include "globals.h"

LocalServerConnection::LocalServerConnection()
	: ServerConnection()
{
	printf("LocalServerConnection() Ctor.\n");
}

LocalServerConnection::~LocalServerConnection()
{
	printf("LocalServerConnection() ~Dtor.\n");
}

bool LocalServerConnection::Connect(const char * szHostname, u_short)
{
	eX0_assert(szHostname == NULL || *szHostname == '\0');
	eX0_assert(pGameServer != NULL, "connecting to a local server when it's not running, doesn't make sense why do that since it won't work!");

	SetJoinStatus(TCP_CONNECTED);
	printf("Established a 'TCP' connection with local server, attempting to join the game.\n");

	return true;
}

bool LocalServerConnection::SendTcp(CPacket & oPacket, JoinStatus nMinimumJoinStatus)
{
	if (GetJoinStatus() < nMinimumJoinStatus) {
		printf("LocalSendTcp(type #%d) failed, not high enough JoinStatus.\n", oPacket.GetPacket()[2]);
		return false;
	}
	else if (oPacket.GetSendMode() == CPacket::NETWORK) {
		printf("LocalSendTcp(type #%d) failed, Network-only packet.\n", oPacket.GetPacket()[2]);
		return false;
	}
	else
	{
		oPacket.ConvertToReadOnly();
		if (false == NetworkProcessTcpPacket(oPacket, pLocalPlayer->pConnection))
		{
			printf("Error: LocalServerConnection::SendTcp() packet execution failed.\n");
			return false;
		}
	}

	return true;
}

bool LocalServerConnection::SendUdp(CPacket & oPacket, JoinStatus nMinimumJoinStatus)
{
	if (GetJoinStatus() < nMinimumJoinStatus) {
		printf("LocalSendUdp(type #%d) failed, not high enough JoinStatus.\n", oPacket.GetPacket()[0]);
		return false;
	}
	else if (oPacket.GetSendMode() == CPacket::NETWORK) {
		printf("LocalSendUdp(type #%d) failed, Network-only packet.\n", oPacket.GetPacket()[0]);
		return false;
	}
	else
	{
		oPacket.ConvertToReadOnly();
		if (false == NetworkProcessUdpPacket(oPacket, pLocalPlayer->pConnection))
		{
			printf("Error: LocalServerConnection::SendUdp() packet execution failed.\n");
			return false;
		}
	}

	return true;
}
