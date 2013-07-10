#pragma once
#ifndef __InputListener_H__
#define __InputListener_H__

class InputListener
{
public:
	InputListener();
	virtual ~InputListener();

	// Raw low-level input
	virtual bool ProcessButton(int nDevice, int nButton, bool bPressed);
	virtual bool ProcessSlider(int nDevice, int nSlider, double dMovedAmount);
	virtual bool ProcessAxis(int nDevice, int nAxis, double dPosition);
	virtual void TimePassed(double dTimePassed);

	// Virtual high-level input
	virtual bool ProcessCharacter(int nCharacter, bool bPressed);
	virtual bool ProcessMouseButton(int nMouseButton, bool bPressed);
	virtual bool ProcessMousePosition(int nMousePositionX, int nMousePositionY);

protected:
	// Mutex-using raw low-level input
	virtual bool MutexProcessButton(int /*nDevice*/, int /*nButton*/, bool /*bPressed*/) { return false; }
	virtual bool MutexProcessSlider(int /*nDevice*/, int /*nSlider*/, double /*dMovedAmount*/) { return false; }
	virtual bool MutexProcessAxis(int /*nDevice*/, int /*nAxis*/, double /*dPosition*/) { return false; }
	virtual void MutexTimePassed(double dTimePassed) = 0;

	// Mutex-using virtual high-level input
	virtual bool MutexProcessCharacter(int /*nCharacter*/, bool /*bPressed*/) { return false; }
	virtual bool MutexProcessMouseButton(int /*nMouseButton*/, bool /*bPressed*/) { return false; }
	virtual bool MutexProcessMousePosition(int /*nMousePositionX*/, int /*nMousePositionY*/) { return false; }

	GLFWmutex	m_oInputMutex;

private:
	InputListener(const InputListener &);
	InputListener & operator =(const InputListener &);
};

#endif // __InputListener_H__
