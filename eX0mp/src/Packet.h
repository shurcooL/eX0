// macros for packing floats and doubles
#define pack754_32(f)		(pack754((f), 32, 8))
#define pack754_64(f)		(pack754((f), 64, 11))
#define unpack754_32(i)		(unpack754((i), 32, 8))
#define unpack754_64(i)		(unpack754((i), 64, 11))

enum JoinStatus {
	DISCONNECTED = 0,
	TCP_CONNECTED,
	ACCEPTED,
	UDP_CONNECTED,
	IN_GAME
};

class CPacket
{
public:
	CPacket();
	CPacket(char* pBuffer);
	~CPacket();

	unsigned char * GetPacket(void);
	int GetSize(void);
	size_t pack(char *format, ...);
	void unpack(char *format, ...);
	long long pack754(long double f, unsigned bits, unsigned expbits);
	long double unpack754(long long i, unsigned bits, unsigned expbits);
	void packi16(unsigned char *buf, unsigned int i);
	void packi32(unsigned char *buf, unsigned long i);
	unsigned int unpacki16(unsigned char *buf);
	unsigned long unpacki32(unsigned char *buf);

	void CompleteTpcPacketSize(void);
	bool SendTcp(/*CClient * pClient, */JoinStatus nMinimumJoinStatus = IN_GAME);
	bool SendUdp(/*CClient * pClient, */JoinStatus nMinimumJoinStatus = IN_GAME);

	void Print(int nPacketSize);

private:
	unsigned char	*m_pBuffer;
	bool			m_bOwnBuffer;
	unsigned char	*m_pBufferPosition;
};
