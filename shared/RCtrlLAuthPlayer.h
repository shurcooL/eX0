#pragma once
#ifndef __RCtrlLAuthPlayer_H__
#define __RCtrlLAuthPlayer_H__

typedef struct {
	Input_t	oInput;
	u_char	cSequenceNumber;
} SequencedInput_t;

class RCtrlLAuthPlayer
	: public CPlayer
{
public:
	RCtrlLAuthPlayer();
	~RCtrlLAuthPlayer();

	void ProcessInputCmdTEST();

	u_char		cCurrentCommandSequenceNumber;
	ThreadSafeQueue<SequencedInput_t, 100>		m_oInputCmdsTEST;
};

#endif // __RCtrlLAuthPlayer_H__
