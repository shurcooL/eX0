class CClient
{
public:
	CClient(SOCKET nTcpSocket);
	~CClient();

	SOCKET GetSocket();
	struct sockaddr_in & GetAddress();
	void SetAddress(struct sockaddr_in & oUdpAddress);

	char * GetSignature(void);
	void SetSignature(char cSignature[4]);

	JoinStatus GetJoinStatus(void);
	void SetJoinStatus(JoinStatus nJoinStatus);

	int GetPlayerID(void);
	void SetPlayerID(int nPlayerID);

	CPlayer * GetPlayer(void);

	u_char cLastMovementSequenceNumber;

private:
	SOCKET		m_nTcpSocket;
	struct sockaddr_in	m_oUdpAddress;
	char		m_cSignature[4];
	JoinStatus	m_nJoinStatus;

	int			m_nPlayerID;
};

// Returns a client from their socket number
CClient * ClientGetFromSocket(SOCKET nSocket);

// Returns a client from their address
CClient * ClientGetFromAddress(struct sockaddr_in & oAddress);

CClient * ClientGetFromSignature(char cSignature[4]);

// Returns a player from their socket number
//CPlayer * ClientGetPlayerFromSocket(SOCKET nSocket);

// Returns a player from their address
//CPlayer * ClientGetPlayerFromAddress(struct sockaddr_in & oAddress);

void ClientDeinit(void);
