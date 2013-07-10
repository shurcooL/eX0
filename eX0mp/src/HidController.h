#pragma once
#ifndef __HidController_H__
#define __HidController_H__

class HidController
	: public LocalController
{
public:
	HidController(CPlayer & oPlayer);
	~HidController();

	void ProvideRealtimeInput(double dTimePassed);

	PlayerInputListener		m_oPlayerInputListener;

protected:
	void ProvideNextCommand();

	void ProvideNextWpnCommand();

	void SubReset();

private:
	HidController(const HidController &);
	HidController & operator =(const HidController &);
};

#endif // __HidController_H__
