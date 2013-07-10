#pragma once
#ifndef __RemoteAuthPlayer_H__
#define __RemoteAuthPlayer_H__

class RemoteAuthPlayer
	: public CPlayer
{
public:
	RemoteAuthPlayer();
	RemoteAuthPlayer(u_int nPlayerId);
	~RemoteAuthPlayer();

	void SendUpdate() {}
};

#endif // __RemoteAuthPlayer_H__
