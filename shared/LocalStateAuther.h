#pragma once
#ifndef __LocalStateAuther_H__
#define __LocalStateAuther_H__

class LocalStateAuther
	: public PlayerStateAuther
{
public:
	LocalStateAuther(CPlayer & oPlayer);
	~LocalStateAuther();

	void ProcessCommands();
	void ProcessWpnCommands();
	void ProcessUpdates();
	void SendUpdate();

	bool IsLocal(void) { return true; }
};

#endif // __LocalStateAuther_H__
