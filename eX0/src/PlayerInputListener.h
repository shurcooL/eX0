#pragma once
#ifndef __PlayerInputListener_H__
#define __PlayerInputListener_H__

class PlayerInputListener
	: public InputListener
{
public:
	PlayerInputListener(CPlayer & oPlayer);
	~PlayerInputListener();

	Command_st GetNextCommand();
	double GetRotationAmount();

	WpnCommand_st GetNextWpnCommand();

	void Reset();

protected:
	bool MutexProcessButton(int nDevice, int nButton, bool bPressed);
	bool MutexProcessSlider(int nDevice, int nSlider, double dMovedAmount);
	bool MutexProcessAxis(int nDevice, int nAxis, double dPosition);
	void MutexTimePassed(double dTimePassed);

private:
	char GetMoveDirection();
	bool GetStealth();

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

	bool	m_bWeaponFireTEST;
	bool	m_bWeaponReloadTEST;
	int32	m_WeaponChangeTEST;

	CPlayer & m_oPlayer;		// TODO: This dependency is only needed to be able to player.GetZ() when firing weapons, perhaps remove later if possible
};

#endif // __PlayerInputListener_H__
