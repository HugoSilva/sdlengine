#include "InputManager.h"

namespace IO
{
	InputManager::InputManager()
	{
		// TODO: check available input devices
		availableKeyboard = true;
		availableMouse = true;
		availableController = false;
	}

	InputManager::~InputManager()
	{
	}

	void InputManager::HandleKeyboard(SDL_KeyboardEvent keyEvent)
	{
		if (availableKeyboard) {
			Keyboard::KeyCallback(keyEvent);
		}
	}

	void InputManager::HandleMouseButton(SDL_MouseButtonEvent mouseEvent)
	{
		if (availableMouse) {
			Mouse::MouseButtonCallback(mouseEvent);
		}
	}

	void InputManager::HandleMouseMotion(SDL_MouseMotionEvent mouseEvent)
	{
		if (availableMouse) {
			Mouse::MousePosCallback(mouseEvent);
		}
	}
}