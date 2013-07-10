#pragma once
#ifndef __PlayerController_H__
#define __PlayerController_H__

class PlayerController
{
public:
	PlayerController(CPlayer & oPlayer);
	virtual ~PlayerController(void);

	void RequestNextCommand(void);
	u_int GetCommandRequests(void);
	void UseUpCommandRequest(void);

	virtual bool IsLocal(void) = 0;

protected:
	virtual void ProvideNextCommand(void) = 0;

	CPlayer & m_oPlayer;

private:
	PlayerController(const PlayerController &);
	PlayerController & operator =(const PlayerController &);

	u_int	m_nCommandRequests;
};

#endif // __PlayerController_H__
