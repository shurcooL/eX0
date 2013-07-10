#pragma once
#ifndef __InputManager_H__
#define __InputManager_H__

class InputManager
{
public:
	InputManager();
	~InputManager();

	void RegisterListener(InputListener * pListener);

	void ShowMouseCursor(void);
	void HideMouseCursor(void);
	bool IsMousePointerVisible(void);

	static void GLFWCALL ProcessKey(int nKey, int nAction);
	static void GLFWCALL ProcessChar(int nChar, int nAction);
	static void GLFWCALL ProcessMouseButton(int nMouseButton, int nAction);
	static void GLFWCALL ProcessMousePos(int nMousePosX, int nMousePosY);
	static void GLFWCALL ProcessMouseWheel(int nMouseWheelPosition);
	void ProcessJoysticks();

	void TimePassed(double dTimePassed);

private:
	void SetGlfwCallbacks(void);
	void RemoveGlfwCallbacks(void);

	void InitializeJoysticks(void);

	std::vector<InputListener *>		m_oListeners;

	bool	m_bIsMousePointerVisible;

	struct Joystick_t {
		u_int	nId;
		u_int	nAxes;
		u_int	nButtons;
	};
	std::vector<Joystick_t>		m_oJoysticks;

	static InputManager *		m_pInstance;
};

#endif // __InputManager_H__
