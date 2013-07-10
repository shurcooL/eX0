#include <stdarg.h>

// TODO: Properly fix this, by making this file independent of globals.h
#ifdef EX0_CLIENT
#	include "globals.h"
#else
#	include "../eX0ds/src/globals.h"
#endif // EX0_CLIENT

CPacket::CPacket(SendMode nSendMode)
	: m_pBuffer(new u_char[MAX_PACKET_SIZE]),
	  m_nSize(0),
	  m_bOwnBuffer(true),
	  m_pBufferPosition(m_pBuffer),
	  m_nPacketType(WRITE_ONLY),
	  m_nSendMode(nSendMode)
{
}

CPacket::CPacket(u_char * pBuffer, u_int nSize)
	: m_pBuffer(pBuffer),
	  m_nSize(nSize),
	  m_bOwnBuffer(false),
	  m_pBufferPosition(m_pBuffer),
	  m_nPacketType(READ_ONLY),
	  m_nSendMode(BOTH)		// Not used/needed in the READ_ONLY version of CPacket
{
	eX0_assert(nSize > 0, "nSize > 0");
}

CPacket::~CPacket()
{
	if (m_bOwnBuffer)
		delete[] m_pBuffer;
}

const u_char * CPacket::GetPacket() const
{
	return m_pBuffer;
}

const u_int CPacket::size() const
{
	/*if (m_bOwnBuffer)
		return (u_int)(m_pBufferPosition - m_pBuffer);
	else
		return m_nSize;*/

	eX0_assert(m_nPacketType == READ_ONLY || (u_int)(m_pBufferPosition - m_pBuffer) == m_nSize, "packet equality");
	return m_nSize;
}

const CPacket::SendMode CPacket::GetSendMode() const
{
	return m_nSendMode;
}

void CPacket::CompleteTpcPacketSize()
{
	eX0_assert(m_nPacketType == WRITE_ONLY, "m_nPacketType == WRITE_ONLY in ConvertToReadOnly()");
	eX0_assert(size() >= 3);

	packi16(m_pBuffer, size() - 3);
}

void CPacket::ConvertToReadOnly()
{
	m_pBufferPosition = m_pBuffer;
	m_nPacketType = READ_ONLY;
}

/*
** pack() -- store data dictated by the format string in the buffer
**
**  c - 8-bit char          b - 8-bit bool
**  h - 16-bit              l - 32-bit
**  f - float, 32-bit       d - double, 64-bit
**  t - std::string (known length)
**  s - string (known length)
**  z - string (16-bit length is automatically prepended)
*/
u_int CPacket::pack(char *format, ...)
{
	//eX0_assert(m_bOwnBuffer, "Tried to pack a constant (existing) CPacket");
	eX0_assert(m_nPacketType == WRITE_ONLY, "m_nPacketType == WRITE_ONLY in pack()");

	va_list ap;
	int h;
	int l;
	long long ll;
	char c;
	bool b;
	float f;
	double d;
	char *s;
	string *t;
	u_int size = 0, len;

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
			c = static_cast<char>(va_arg(ap, int)); // promoted
			*m_pBufferPosition++ = c & 0xFF;
			break;

		case 'b': // 8-bit
			size += 1;
			b = (0 != va_arg(ap, int)); // promoted
			*m_pBufferPosition++ = b ? 0xFF : 0;
			break;

		case 'f': // float
			size += 4;
			f = (float)va_arg(ap, double); // promoted
			l = (int)pack754_32(f); // convert to IEEE 754
			packi32(m_pBufferPosition, l);
			m_pBufferPosition += 4;
			break;

		case 'd': // double
			size += 8;
			d = (double)va_arg(ap, double);
			ll = (long long)pack754_64(d); // convert to IEEE 754
			packi64(m_pBufferPosition, ll);
			m_pBufferPosition += 8;
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
			packi16(m_pBufferPosition, (u_int)len);
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

	m_nSize += size;

	eX0_assert(CPacket::size() <= MAX_PACKET_SIZE, "Tried to pack more data than can fit into a packet");

	return size;
}

/*
** unpack() -- unpack data dictated by the format string into the buffer
**
**  e - set length as an int parameter
*/
void CPacket::unpack(char *format, ...)
{
	eX0_assert(m_nPacketType == READ_ONLY, "m_nPacketType == READ_ONLY");

	va_list ap;
	short *h;
	int *l;
	int pf;
	long long pd;
	char *c;
	bool *b;
	float *f;
	double *d;
	char *s;
	string *t;
	u_int len, count, maxstrlen = 0;

	va_start(ap, format);

	for (; *format != '\0'; format++) {
		switch (*format) {
		case 'h': // 16-bit
			h = va_arg(ap, short *);
			*h = static_cast<short>(unpacki16(m_pBufferPosition));
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

		case 'b': // 8-bit
			b = va_arg(ap, bool *);
			*b = (0 != *m_pBufferPosition++);
			break;

		case 'f': // float
			f = va_arg(ap, float *);
			pf = unpacki32(m_pBufferPosition);
			*f = (float)unpack754_32(pf);
			m_pBufferPosition += 4;
			break;

		case 'd': // double
			d = va_arg(ap, double *);
			pd = unpacki64(m_pBufferPosition);
			*d = (double)unpack754_64(pd);
			m_pBufferPosition += 8;
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
			if (maxstrlen > 0) count = std::min<u_int>(len, maxstrlen - 1);
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

	eX0_assert((u_int)(m_pBufferPosition - m_pBuffer) <= m_nSize, "Tried to unpack more data than exists in a CPacket");
	if (false == ((u_int)(m_pBufferPosition - m_pBuffer) <= m_nSize)) {
		Print();
	}
}

/*
** pack754() -- pack a floating point number into IEEE-754 format
*/
long long CPacket::pack754(long double f, u_int bits, u_int expbits)
{
	long double fnorm;
	int shift;
	long long sign, exp, significand;
	u_int significandbits = bits - expbits - 1; // -1 for sign bit

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
long double CPacket::unpack754(long long i, u_int bits, u_int expbits)
{
	long double result;
	long long shift;
	u_int bias;
	u_int significandbits = bits - expbits - 1; // -1 for sign bit

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
void CPacket::packi16(u_char *buf, u_int i)
{
	*buf++ = static_cast<u_char>(i>>8); *buf++ = static_cast<u_char>(i);
}

/*
** packi32() -- store a 32-bit int into a char buffer (like htonl())
*/
void CPacket::packi32(u_char *buf, u_long i)
{
	*buf++ = (u_char)(i>>24); *buf++ = (u_char)(i>>16);
	*buf++ = (u_char)(i>>8);  *buf++ = (u_char)i;
}

/*
** packi64() -- store a 64-bit int into a char buffer
*/
void CPacket::packi64(u_char *buf, u_int64 i)
{
	*buf++ = (u_char)(i>>56); *buf++ = (u_char)(i>>48);
	*buf++ = (u_char)(i>>40); *buf++ = (u_char)(i>>32);
	*buf++ = (u_char)(i>>24); *buf++ = (u_char)(i>>16);
	*buf++ = (u_char)(i>>8);  *buf++ = (u_char)i;
}

/*
** unpacki16() -- unpack a 16-bit int from a char buffer (like ntohs())
*/
u_int CPacket::unpacki16(u_char *buf)
{
	return (buf[0]<<8) | buf[1];
}

/*
** unpacki32() -- unpack a 32-bit int from a char buffer (like ntohl())
*/
u_long CPacket::unpacki32(u_char *buf)
{
	return (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
}

/*
** unpacki64() -- unpack a 64-bit int from a char buffer
*/
u_int64 CPacket::unpacki64(u_char *buf)
{
//	return ((u_int64)buf[0]<<56) | ((u_int64)buf[1]<<48) | ((u_int64)buf[2]<<40) | ((u_int64)buf[3]<<32)
//		| (buf[4]<<24) | (buf[5]<<16) | (buf[6]<<8) | buf[7];
//	return (((u_int64)buf[0])<<56) | (((u_int64)buf[1])<<48) | (((u_int64)buf[2])<<40) | (((u_int64)buf[3])<<32)
//		| (((u_int64)buf[4])<<24) | (((u_int64)buf[5])<<16) | (((u_int64)buf[6])<<8) | ((u_int64)buf[7]);
	return ((u_int64)buf[0]<<56) | ((u_int64)buf[1]<<48) | ((u_int64)buf[2]<<40) | ((u_int64)buf[3])<<32
		| ((u_int64)buf[4]<<24) | ((u_int64)buf[5]<<16) | ((u_int64)buf[6]<<8) | (u_int64)buf[7];
}

void CPacket::Print() const
{
	printf("Packet (%d bytes): ", size());
	for (u_int nByte = 0; nByte < size(); ++nByte)
		printf("%d ", m_pBuffer[nByte]);
	printf("\n");
}
