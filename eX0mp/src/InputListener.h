#pragma once
#ifndef __InputListener_H__
#define __InputListener_H__

class InputListener
{
public:
	InputListener();
	virtual ~InputListener();

	// Raw low-level input
	virtual bool ProcessButton(int /*nDevice*/, int /*nButton*/, bool /*bPressed*/) { return false; }
	virtual bool ProcessSlider(int /*nDevice*/, int /*nSlider*/, double /*dMovedAmount*/) { return false; }
	virtual bool ProcessAxis(int /*nDevice*/, int /*nAxis*/, double /*dPosition*/) { return false; }
	virtual void TimePassed(double dTimePassed) = 0;

	// Virtual high-level input
	virtual bool ProcessCharacter(int /*nCharacter*/, bool /*bPressed*/) { return false; }
	virtual bool ProcessMouseButton(int /*nMouseButton*/, bool /*bPressed*/) { return false; }
	virtual bool ProcessMousePosition(int /*nMousePositionX*/, int /*nMousePositionY*/) { return false; }
};

#endif // __InputListener_H__
