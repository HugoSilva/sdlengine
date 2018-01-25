#include "Input.h"

Input::Input()
{
	// TODO: check available input devices
	availableKeyboard = true;
	availableMouse = true;
	availableController = false;
}

Input::~Input()
{
}

void Input::HandleKeyboard(SDL_KeyboardEvent keyEvent)
{
	if (availableKeyboard) {
		Keyboard::KeyCallback(keyEvent);
	}
}

void Input::HandleMouseButton(SDL_MouseButtonEvent mouseEvent)
{
	if (availableMouse) {
		Mouse::MouseButtonCallback(mouseEvent);
	}
}

void Input::HandleMouseMotion(SDL_MouseMotionEvent mouseEvent)
{
	if (availableMouse) {
		Mouse::MousePosCallback(mouseEvent);
	}
}