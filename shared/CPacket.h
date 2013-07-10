#pragma once
#ifndef __CPacket_H__
#define __CPacket_H__

// macros for packing floats and doubles
#define pack754_32(f)		(pack754((f), 32, 8))
#define pack754_64(f)		(pack754((f), 64, 11))
#define unpack754_32(i)		(unpack754((i), 32, 8))
#define unpack754_64(i)		(unpack754((i), 64, 11))

class CPacket
{
public:
	CPacket();
	CPacket(u_char *pBuffer, int nSize);
	~CPacket();

	u_char * GetPacket(void);
	int Size(void) const;
	size_t pack(char *format, ...);
	void unpack(char *format, ...);
	long long pack754(long double f, u_int bits, u_int expbits);
	long double unpack754(long long i, u_int bits, u_int expbits);
	void packi16(u_char *buf, u_int i);
	void packi32(u_char *buf, u_long i);
	void packi64(u_char *buf, u_int64 i);
	u_int unpacki16(u_char *buf);
	u_long unpacki32(u_char *buf);
	u_int64 unpacki64(u_char *buf);

	void CompleteTpcPacketSize(void);
#ifdef EX0_CLIENT
	bool SendTcp(/*CClient *pClient, */JoinStatus nMinimumJoinStatus = IN_GAME);
	bool SendUdp(/*CClient *pClient, */JoinStatus nMinimumJoinStatus = IN_GAME);
#else
	bool SendTcp(CClient *pClient, JoinStatus nMinimumJoinStatus = IN_GAME);
	bool BroadcastTcp(JoinStatus nMinimumJoinStatus = IN_GAME);
	bool BroadcastTcpExcept(CClient *pClient, JoinStatus nMinimumJoinStatus = IN_GAME);

	bool SendUdp(CClient *pClient, JoinStatus nMinimumJoinStatus = IN_GAME);
	bool BroadcastUdp(JoinStatus nMinimumJoinStatus = IN_GAME);
	bool BroadcastUdpExcept(CClient *pClient, JoinStatus nMinimumJoinStatus = IN_GAME);
#endif

	void Print() const;

private:
	u_char		*m_pBuffer;
	int			m_nSize;		// Only used when it's a pre-made packet, not own buffer
	bool		m_bOwnBuffer;
	u_char		*m_pBufferPosition;
};

#endif // __CPacket_H__
