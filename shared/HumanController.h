#pragma once
#ifndef __HumanController_H__
#define __HumanController_H__

class HumanController
	: public PlayerController
{
public:
	HumanController(CPlayer & oPlayer);
	~HumanController();

	bool RequestInput(u_char cSequenceNumber);
};

#endif // __HumanController_H__
