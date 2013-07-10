#pragma once
#ifndef __NetworkController_H__
#define __NetworkController_H__

class NetworkController
	: public PlayerController
{
public:
	NetworkController(CPlayer & oPlayer);
	~NetworkController();

	bool RequestInput(u_char cSequenceNumber);

private:
	GLFWmutex		m_oMutex;
};

#endif // __NetworkController_H__
