#pragma once
#ifndef __LocalController_H__
#define __LocalController_H__

class LocalController
	: public PlayerController
{
public:
	LocalController(CPlayer & oPlayer);
	~LocalController();

	bool RequestInput(u_char cSequenceNumber);
};

#endif // __LocalController_H__
