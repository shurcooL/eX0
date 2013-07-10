#pragma once
#ifndef __PlayerController_H__
#define __PlayerController_H__

class PlayerController
{
public:
	PlayerController(CPlayer & oPlayer);
	virtual ~PlayerController();

	void RequestNextCommand();
	u_int GetCommandRequests();
	void UseUpCommandRequest();
	//bool TryUseUpCommandRequest();

	void Reset();

	virtual bool IsLocal() = 0;

protected:
	virtual void ProvideNextCommand() = 0;
	virtual void ChildReset() = 0;

	CPlayer & m_oPlayer;

private:
	PlayerController(const PlayerController &);
	PlayerController & operator =(const PlayerController &);

	u_int	m_nCommandRequests;
};

#endif // __PlayerController_H__
