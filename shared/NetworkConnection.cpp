// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "../eX0mp/src/globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

NetworkConnection::NetworkConnection()
	: m_nJoinStatus(DISCONNECTED),
	  m_nTcpSocket(INVALID_SOCKET),
	  m_nUdpSocket(INVALID_SOCKET),
	  m_oUdpAddress(),
	  m_cSignature()
{
	printf("Created a disconnected NetworkConnection.\n");
}

NetworkConnection::NetworkConnection(const SOCKET nTcpSocket)
	: m_nJoinStatus(TCP_CONNECTED),
	  m_nTcpSocket(nTcpSocket),
	  m_nUdpSocket(INVALID_SOCKET),
	  m_oUdpAddress(),
	  m_cSignature()
{
	printf("Created a NetworkConnection (socket %d opened).\n", GetTcpSocket());
}

NetworkConnection::~NetworkConnection()
{
	if (GetJoinStatus() >= TCP_CONNECTED && GetTcpSocket() != INVALID_SOCKET) {
		// Close the connection socket
		shutdown(GetTcpSocket(), SD_BOTH);
		NetworkCloseSocket(GetTcpSocket());
	}

	printf("Deleted a NetworkConnection (socket %d closed).\n", GetTcpSocket());
}

void NetworkConnection::SetJoinStatus(const JoinStatus nJoinStatus) { m_nJoinStatus = nJoinStatus; }
const JoinStatus NetworkConnection::GetJoinStatus() const { return m_nJoinStatus; }

void NetworkConnection::SetTcpSocket(const SOCKET nTcpSocket) { m_nTcpSocket = nTcpSocket; }
const SOCKET NetworkConnection::GetTcpSocket() const { return m_nTcpSocket; }

void NetworkConnection::SetUdpSocket(const SOCKET nUdpSocket) { m_nUdpSocket = nUdpSocket; }
const SOCKET NetworkConnection::GetUdpSocket() const { return m_nUdpSocket; }
void NetworkConnection::SetUdpAddress(const sockaddr_in & oUdpAddress) { m_oUdpAddress = oUdpAddress; }
const sockaddr_in & NetworkConnection::GetUdpAddress() const { return m_oUdpAddress; }

void NetworkConnection::SetSignature(const u_char cSignature[m_knSignatureSize]) { memcpy(m_cSignature, cSignature, m_knSignatureSize); }
const u_char * NetworkConnection::GetSignature() const { return m_cSignature; }

bool NetworkConnection::SendTcp(CPacket & oPacket, JoinStatus nMinimumJoinStatus)
{
	if (GetJoinStatus() < nMinimumJoinStatus) {
		printf("SendTcp(type #%d) failed, not high enough JoinStatus.\n", oPacket.GetPacket()[2]);
		return false;
	}
	else if (sendall(GetTcpSocket(), reinterpret_cast<const char *>(oPacket.GetPacket()), oPacket.size(), 0) == SOCKET_ERROR)
	{
		NetworkPrintError("sendall");
		return false;
	}

	return true;
}

bool NetworkConnection::SendUdp(CPacket & oPacket, JoinStatus nMinimumJoinStatus)
{
	if (GetJoinStatus() < nMinimumJoinStatus) {
		printf("SendUdp(type #%d) failed, not high enough JoinStatus.\n", oPacket.GetPacket()[0]);
		return false;
	}
	else if (sendudp(GetUdpSocket(),
					 reinterpret_cast<const char *>(oPacket.GetPacket()),
					 oPacket.size(),
					 0,
					 reinterpret_cast<const sockaddr *>(&GetUdpAddress()),
					 sizeof(GetUdpAddress()))
						!= static_cast<int>(oPacket.size()))
	{
		NetworkPrintError("sendudp (sendto)");
		return false;
	}
	/*else if (sendudp(GetUdpSocket(), (char *)oPacket.GetPacket(), oPacket.size(), 0) != oPacket.size()) {
		NetworkPrintError("sendudp (send)");
		return false;
	}*/

	return true;
}
