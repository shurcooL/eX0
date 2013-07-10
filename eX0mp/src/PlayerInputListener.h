#pragma once
#ifndef __PlayerInputListener_H__
#define __PlayerInputListener_H__

class PlayerInputListener
	: public InputListener
{
public:
	PlayerInputListener(void);
	~PlayerInputListener(void);

	char GetMoveDirection(void);
	bool GetStealth(void);
	double GetRotationAmount(void);

	bool ProcessButton(int nDevice, int nButton, bool bPressed);
	bool ProcessSlider(int nDevice, int nSlider, double dMovedAmount);
	bool ProcessAxis(int nDevice, int nAxis, double dPosition);
	void TimePassed(double dTimePassed);

private:
	/*bool	m_bForward;
	bool	m_bBackward;
	bool	m_bLeft;
	bool	m_bRight;*/

	double	m_dForwardAxis;
	double	m_dStrafeAxis;
	double	m_dRotationAxis;
	double	m_dStealthHalfAxis;

	double	m_dForwardAxisState;
	double	m_dStrafeAxisState;
	double	m_dRotationAxisState;
	double	m_dStealthHalfAxisState;
};

#endif // __PlayerInputListener_H__
