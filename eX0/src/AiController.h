#pragma once
#ifndef __AiController_H__
#define __AiController_H__

class AiController
	: public LocalController
{
public:
	AiController(CPlayer & oPlayer);
	~AiController();

	void ProvideRealtimeInput(double dTimePassed);

protected:
	void ProvideNextCommand();

	void ProvideNextWpnCommand();

	void SubReset();

private:
	AiController(const AiController &);
	AiController & operator =(const AiController &);
};

#endif // __AiController_H__
