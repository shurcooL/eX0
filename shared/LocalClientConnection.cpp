// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

LocalClientConnection::LocalClientConnection()
	: ClientConnection()
{
	printf("LocalClientConnection() Ctor.\n");

	SetJoinStatus(TCP_CONNECTED);
}

LocalClientConnection::~LocalClientConnection()
{
	printf("LocalClientConnection() ~Dtor.\n");
}

u_short LocalClientConnection::GetLastLatency() const
{
	return 1337;
}

bool LocalClientConnection::SendTcp(CPacket & oPacket, JoinStatus nMinimumJoinStatus)
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
		if (false == NetworkProcessTcpPacket(oPacket))
		{
			printf("Error: LocalClientConnection::SendTcp() packet execution failed.\n");
			return false;
		}
	}

	return true;
}

bool LocalClientConnection::SendUdp(CPacket & oPacket, JoinStatus nMinimumJoinStatus)
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
		if (false == NetworkProcessUdpPacket(oPacket))
		{
			printf("Error: LocalClientConnection::SendUdp() packet execution failed.\n");
			return false;
		}
	}

	return true;
}
