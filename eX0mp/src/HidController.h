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

	PlayerInputListener * m_pPlayerInputListener;

protected:
	void ProvideNextCommand();
	void ChildReset();

private:
	HidController(const HidController &);
	HidController & operator =(const HidController &);
};

#endif // __HidController_H__
