#pragma once
#ifndef __LocalAuthPlayer_H__
#define __LocalAuthPlayer_H__

typedef struct {
	Input_t	oInput;
	u_char	cSequenceNumber;
} SequencedInput_t;

class LocalAuthPlayer
	: public CPlayer
{
public:
	LocalAuthPlayer();
	~LocalAuthPlayer();

	void ProcessInputCmdTEST();
	void SendUpdate();

	u_char		cLatestAuthStateSequenceNumber;
	ThreadSafeQueue<SequencedInput_t, 100>		m_oInputCmdsTEST;
};

#endif // __LocalAuthPlayer_H__
