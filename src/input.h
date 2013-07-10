#define MOUSE_FILTERING_SAMPLES		5

// do mouse movement calcs
void InputMouseMovCalcs();

// check if a key is held down
void InputKeyHold();

// key processing
void InputProcessKey(int iKey, int iAction);

// do whatever if mouse has moved
void InputMouseMoved();

// calculate the filtered mouse moved vars
void InputFilteredMouseMoved();

// check if a mouse button is held down
void InputMouseHold();

// processes mouse clicks
void InputProcessMouse(int iButton, int iAction);

// check if a mouse button is pressed
//bool InputIsMouseButtonDown(int iButton);