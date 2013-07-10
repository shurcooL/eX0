#pragma once
#ifndef __LocalStateAuther_H__
#define __LocalStateAuther_H__

class LocalStateAuther
	: public PlayerStateAuther
{
public:
	LocalStateAuther(CPlayer & oPlayer);
	~LocalStateAuther();

	void AfterTick();
	void ProcessAuthUpdateTEST();
	void SendUpdate();
};

#endif // __LocalStateAuther_H__
