#pragma once
#ifndef __NetworkStateAuther_H__
#define __NetworkStateAuther_H__

class NetworkStateAuther
	: public PlayerStateAuther
{
public:
	NetworkStateAuther(CPlayer & oPlayer);
	~NetworkStateAuther();

	void AfterTick();
	void ProcessAuthUpdateTEST();
	void SendUpdate();

	static void ProcessUpdate(CPacket & oPacket);

	u_char		cLastAckedCommandSequenceNumber;
	u_char		cCurrentCommandSeriesNumber;
};

#endif // __NetworkStateAuther_H__
