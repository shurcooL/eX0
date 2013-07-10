#include "globals.h"

InputListener::InputListener()
	: m_oInputMutex(glfwCreateMutex())
{
}

InputListener::~InputListener()
{
	glfwDestroyMutex(m_oInputMutex);
}

// Raw low-level input
bool InputListener::ProcessButton(int nDevice, int nButton, bool bPressed)
{
	glfwLockMutex(m_oInputMutex);
	bool bResult = MutexProcessButton(nDevice, nButton, bPressed);
	glfwUnlockMutex(m_oInputMutex);

	return bResult;
}

bool InputListener::ProcessSlider(int nDevice, int nSlider, double dMovedAmount)
{
	glfwLockMutex(m_oInputMutex);
	bool bResult = MutexProcessSlider(nDevice, nSlider, dMovedAmount);
	glfwUnlockMutex(m_oInputMutex);

	return bResult;
}

bool InputListener::ProcessAxis(int nDevice, int nAxis, double dPosition)
{
	glfwLockMutex(m_oInputMutex);
	bool bResult = MutexProcessAxis(nDevice, nAxis, dPosition);
	glfwUnlockMutex(m_oInputMutex);

	return bResult;
}

void InputListener::TimePassed(double dTimePassed)
{
	glfwLockMutex(m_oInputMutex);
	MutexTimePassed(dTimePassed);
	glfwUnlockMutex(m_oInputMutex);
}

// Virtual high-level input
bool InputListener::ProcessCharacter(int nCharacter, bool bPressed)
{
	glfwLockMutex(m_oInputMutex);
	bool bResult = MutexProcessCharacter(nCharacter, bPressed);
	glfwUnlockMutex(m_oInputMutex);

	return bResult;
}

bool InputListener::ProcessMouseButton(int nMouseButton, bool bPressed)
{
	glfwLockMutex(m_oInputMutex);
	bool bResult = MutexProcessMouseButton(nMouseButton, bPressed);
	glfwUnlockMutex(m_oInputMutex);

	return bResult;
}

bool InputListener::ProcessMousePosition(int nMousePositionX, int nMousePositionY)
{
	glfwLockMutex(m_oInputMutex);
	bool bResult = MutexProcessMousePosition(nMousePositionX, nMousePositionY);
	glfwUnlockMutex(m_oInputMutex);

	return bResult;
}
