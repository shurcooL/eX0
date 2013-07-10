#pragma once
#ifndef __PlayerController_H__
#define __PlayerController_H__

class PlayerController
{
public:
	PlayerController(CPlayer & oPlayer);
	virtual ~PlayerController();

	virtual bool RequestInput(u_char cSequenceNumber) = 0;

protected:
	PlayerController(const PlayerController &);
	PlayerController & operator =(const PlayerController &);

	CPlayer & m_oPlayer;
};

#endif // __PlayerController_H__
