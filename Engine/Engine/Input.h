#ifndef INPUT
#define INPUT

#include <SDL.h>
#include "IO/Mouse.h"
#include "IO/Keyboard.h"

class Input
{
public:
	Input();
	~Input();

	void HandleKeyboard(SDL_KeyboardEvent windowEvent);
	void HandleMouseButton(SDL_MouseButtonEvent windowEvent);
	void HandleMouseMotion(SDL_MouseMotionEvent windowEvent);

private:
	bool availableKeyboard;
	bool availableMouse;
	bool availableController;
};

#endif