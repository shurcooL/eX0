#pragma once
#ifndef __LocalController_H__
#define __LocalController_H__

class LocalController
	: public PlayerController
{
public:
	LocalController(CPlayer & oPlayer);
	~LocalController();

	bool IsLocal(void) { return true; }

	virtual void ProvideRealtimeInput(double dTimePassed) = 0;

private:
	LocalController(const LocalController &);
	LocalController & operator =(const LocalController &);
};

#endif // __LocalController_H__
