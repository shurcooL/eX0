#pragma once
#ifndef __PlayerStateAuther_H__
#define __PlayerStateAuther_H__

class PlayerStateAuther
{
public:
	PlayerStateAuther(CPlayer & oPlayer);
	virtual ~PlayerStateAuther();

	virtual void AfterTick() = 0;
	virtual void ProcessAuthUpdateTEST() = 0;
	virtual void SendUpdate() = 0;

	virtual bool IsLocal(void) = 0;

protected:
	PlayerStateAuther(const PlayerStateAuther &);
	PlayerStateAuther & operator =(const PlayerStateAuther &);

	CPlayer & m_oPlayer;
};

#endif // __PlayerStateAuther_H__
