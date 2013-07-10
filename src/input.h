#define MOUSE_FILTERING_SAMPLES		3

// do mouse movement calcs
void InputMouseMovCalcs();

// check if a key is held down
void InputKeyHold();

// key processing
void GLFWCALL InputProcessKey(int iKey, int iAction);

// Character processing function
void GLFWCALL InputProcessChar(int nChar, int nAction);

// do whatever if mouse has moved
void InputMouseMoved();

// calculate the filtered mouse moved vars
void InputFilteredMouseMoved();

// check if a mouse button is held down
void InputMouseHold();

// processes mouse clicks
void GLFWCALL InputProcessMouse(int iButton, int iAction);
