#include <stdarg.h>
//#include <stdint.h>

#include "globals.h"

CPacket::CPacket()
{
	m_pBuffer = new u_char[MAX_PACKET_SIZE];
	m_bOwnBuffer = true;
	m_pBufferPosition = m_pBuffer;
}

CPacket::CPacket(char *pBuffer)
{
	m_pBuffer = (u_char *)pBuffer;
	m_bOwnBuffer = false;
	m_pBufferPosition = m_pBuffer;
}

CPacket::~CPacket()
{
	if (m_bOwnBuffer)
		delete[] m_pBuffer;
}

u_char * CPacket::GetPacket(void)
{
	return m_pBuffer;
}

int CPacket::GetSize(void)
{
	return (int)(m_pBufferPosition - m_pBuffer);
}

/*
** pack() -- store data dictated by the format string in the buffer
**
**  h - 16-bit              l - 32-bit
**  c - 8-bit char          f - float, 32-bit
**  t - std::string (known length)
**  s - string (known length)
**  z - string (16-bit length is automatically prepended)
*/
size_t CPacket::pack(char *format, ...)
{
	va_list ap;
	int h;
	int l;
	char c;
	float f;
	char *s;
	string *t;
	size_t size = 0, len;

	va_start(ap, format);

	for (; *format != '\0'; format++) {
		switch (*format) {
		case 'h': // 16-bit
			size += 2;
			h = va_arg(ap, int); // promoted
			packi16(m_pBufferPosition, h);
			m_pBufferPosition += 2;
			break;

		case 'l': // 32-bit
			size += 4;
			l = va_arg(ap, int);
			packi32(m_pBufferPosition, l);
			m_pBufferPosition += 4;
			break;

		case 'c': // 8-bit
			size += 1;
			c = va_arg(ap, int); // promoted
			*m_pBufferPosition++ = (c>>0)&0xff;
			break;

		case 'f': // float
			size += 4;
			f = (float)va_arg(ap, double); // promoted
			l = (int)pack754_32(f); // convert to IEEE 754
			packi32(m_pBufferPosition, l);
			m_pBufferPosition += 4;
			break;

		case 't': // std::string (known length)
			t = va_arg(ap, string *);
			len = t->length();
			size += len;
			memcpy(m_pBufferPosition, t->c_str(), len);
			m_pBufferPosition += len;
			break;

		case 's': // string (known length)
			s = va_arg(ap, char *);
			len = strlen(s);
			size += len;
			memcpy(m_pBufferPosition, s, len);
			m_pBufferPosition += len;
			break;

		case 'z': // string (16-bit length is automatically prepended)
			s = va_arg(ap, char *);
			len = strlen(s);
			size += len + 2;
			packi16(m_pBufferPosition, (unsigned int)len);
			m_pBufferPosition += 2;
			memcpy(m_pBufferPosition, s, len);
			m_pBufferPosition += len;
			break;

		default:
			printf("Error in unpack format.\n");
			break;
		}
	}

	va_end(ap);

	return size;
}

/*
** unpack() -- unpack data dictated by the format string into the buffer
**
**  e - set length as an int parameter
*/
void CPacket::unpack(char *format, ...)
{
	va_list ap;
	short *h;
	int *l;
	int pf;
	char *c;
	float *f;
	char *s;
	string *t;
	size_t len, count, maxstrlen = 0;

	va_start(ap, format);

	for (; *format != '\0'; format++) {
		switch (*format) {
		case 'h': // 16-bit
			h = va_arg(ap, short *);
			*h = unpacki16(m_pBufferPosition);
			m_pBufferPosition += 2;
			break;

		case 'l': // 32-bit
			l = va_arg(ap, int *);
			*l = unpacki32(m_pBufferPosition);
			m_pBufferPosition += 4;
			break;

		case 'c': // 8-bit
			c = va_arg(ap, char *);
			*c = *m_pBufferPosition++;
			break;

		case 'f': // float
			f = va_arg(ap, float *);
			pf = unpacki32(m_pBufferPosition);
			*f = (float)unpack754_32(pf);
			m_pBufferPosition += 4;
			break;

		case 'e': // set length as an int parameter
			maxstrlen = va_arg(ap, int);
			break;

		case 't': // std::string (known length)
			t = va_arg(ap, string *);
			len = count = maxstrlen;
			t->assign((char *)m_pBufferPosition, count);
			m_pBufferPosition += len;
			break;

		case 's': // string (known length)
			s = va_arg(ap, char *);
			len = count = maxstrlen;
			memcpy(s, m_pBufferPosition, count);
			s[count] = '\0';
			m_pBufferPosition += len;
			break;

		case 'z': // string (16-bit length is automatically prepended)
			s = va_arg(ap, char *);
			len = unpacki16(m_pBufferPosition);
			m_pBufferPosition += 2;
			if (maxstrlen > 0) count = __min(len, maxstrlen - 1);
			else count = len;
			memcpy(s, m_pBufferPosition, count);
			s[count] = '\0';
			m_pBufferPosition += len;
			break;

		default:
			if (isdigit(*format)) { // track max str len
				maxstrlen = maxstrlen * 10 + (*format - '0');
			} else
				printf("Error in unpack format.\n");
			break;
		}

		if (*format != 'e' && !isdigit(*format)) maxstrlen = 0;
	}

	va_end(ap);
}

/*
** pack754() -- pack a floating point number into IEEE-754 format
*/ 
long long CPacket::pack754(long double f, unsigned bits, unsigned expbits)
{
	long double fnorm;
	int shift;
	long long sign, exp, significand;
	unsigned significandbits = bits - expbits - 1; // -1 for sign bit

	if (f == 0.0) return 0; // get this special case out of the way

	// check sign and begin normalization
	if (f < 0) { sign = 1; fnorm = -f; }
	else { sign = 0; fnorm = f; }

	// get the normalized form of f and track the exponent
	shift = 0;
	while(fnorm >= 2.0) { fnorm /= 2.0; shift++; }
	while(fnorm < 1.0) { fnorm *= 2.0; shift--; }
	fnorm = fnorm - 1.0;

	// calculate the binary form (non-float) of the significand data
	significand = (long long)(fnorm * ((1LL<<significandbits) + 0.5f));

	// get the biased exponent
	exp = shift + ((1<<(expbits-1)) - 1); // shift + bias

	// return the final answer
	return (sign<<(bits-1)) | (exp<<(bits-expbits-1)) | significand;
}

/*
** unpack754() -- unpack a floating point number from IEEE-754 format
*/ 
long double CPacket::unpack754(long long i, unsigned bits, unsigned expbits)
{
	long double result;
	long long shift;
	unsigned bias;
	unsigned significandbits = bits - expbits - 1; // -1 for sign bit

	if (i == 0) return 0.0;

	// pull the significand
	result = (long double)(i&((1LL<<significandbits)-1)); // mask
	result /= (1LL<<significandbits); // convert back to float
	result += 1.0f; // add the one back on

	// deal with the exponent
	bias = (1<<(expbits-1)) - 1;
	shift = ((i>>significandbits)&((1LL<<expbits)-1)) - bias;
	while(shift > 0) { result *= 2.0; shift--; }
	while(shift < 0) { result /= 2.0; shift++; }

	// sign it
	result *= (i>>(bits-1))&1? -1.0: 1.0;

	return result;
}

/*
** packi16() -- store a 16-bit int into a char buffer (like htons())
*/ 
void CPacket::packi16(u_char *buf, unsigned int i)
{
	*buf++ = i>>8; *buf++ = i;
}

/*
** packi32() -- store a 32-bit int into a char buffer (like htonl())
*/ 
void CPacket::packi32(u_char *buf, unsigned long i)
{
	*buf++ = (unsigned char)(i>>24); *buf++ = (unsigned char)(i>>16);
	*buf++ = (unsigned char)(i>>8);  *buf++ = (unsigned char)i;
}

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
*/ 
unsigned int CPacket::unpacki16(u_char *buf)
{
	return (buf[0]<<8) | buf[1];
}

/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/ 
unsigned long CPacket::unpacki32(u_char *buf)
{
	return (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
}

void CPacket::CompleteTpcPacketSize(void)
{
	packi16((u_char *)GetPacket(), GetSize());
}

bool CPacket::SendTcp(CClient * pClient, JoinStatus nMinimumJoinStatus)
{
	if (pClient->GetJoinStatus() < nMinimumJoinStatus) { printf("SendTcp failed\n"); return false; }
	else if (sendall(pClient->GetSocket(), (char *)GetPacket(), GetSize(), 0) == SOCKET_ERROR) {
		NetworkPrintError("sendall");
		return false;
	}
	return true;
}
bool CPacket::BroadcastTcp(JoinStatus nMinimumJoinStatus)
{
	for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
	{
		// Broadcast the packet to all players that are connected
		if (PlayerGet(nPlayer)->pClient != NULL
		  && PlayerGet(nPlayer)->pClient->GetJoinStatus() >= nMinimumJoinStatus) {
			if (sendall(PlayerGet(nPlayer)->pClient->GetSocket(), (char *)GetPacket(), GetSize(), 0) == SOCKET_ERROR) {
				NetworkPrintError("sendall");
				return false;
			}
		}
	}
	return true;
}
bool CPacket::BroadcastTcpExcept(CClient * pClient, JoinStatus nMinimumJoinStatus)
{
	for (int nPlayer = 0; nPlayer < nPlayerCount; ++nPlayer)
	{
		// Broadcast the packet to all players that are connected, except the specified one
		if (pClient != PlayerGet(nPlayer)->pClient && PlayerGet(nPlayer)->pClient != NULL
		  && PlayerGet(nPlayer)->pClient->GetJoinStatus() >= nMinimumJoinStatus) {
			if (sendall(PlayerGet(nPlayer)->pClient->GetSocket(), (char *)GetPacket(), GetSize(), 0) == SOCKET_ERROR) {
				NetworkPrintError("sendall");
				return false;
			}
		}
	}
	return false;
}

bool CPacket::SendUdp(CClient * pClient, JoinStatus nMinimumJoinStatus)
{
	if (pClient->GetJoinStatus() < nMinimumJoinStatus) { printf("SendUdp failed\n"); return false; }
	// DEBUG: Need to lock a UDP Send Mutex before calling sendto()
	else if (sendto(nUdpSocket, (char *)GetPacket(), GetSize(), 0,
		(struct sockaddr *)&pClient->GetAddress(), sizeof(pClient->GetAddress())) != GetSize()) {
		NetworkPrintError("sendto");
		return false;
	}
	return true;
}
bool CPacket::BroadcastUdp(JoinStatus nMinimumJoinStatus)
{
	// TODO
	printf("BroadcastUdp failed because it's not finished\n"); 
	return false;
}
bool CPacket::BroadcastUdpExcept(CClient * pClient, JoinStatus nMinimumJoinStatus)
{
	// TODO
	printf("BroadcastUdpExcept failed because it's not finished\n"); 
	return false;
}

void CPacket::Print(int nPacketSize)
{
	printf("Packet (%d bytes): ", nPacketSize);
	for (int nByte = 0; nByte < nPacketSize; ++nByte)
		printf("%d ", (u_char)m_pBuffer[nByte]);
	printf("\n");
}
