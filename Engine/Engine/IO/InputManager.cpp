#include "InputManager.h"

namespace IO
{
	InputManager::InputManager()
		: m_Locked(false)
	{
		// TODO: check available input devices
		availableKeyboard = true;
		availableMouse = true;
		availableController = false;
	}

	void InputManager::Update()
	{
		SDL_Event event;

		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				HandleKeyboard(event.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				HandleMouseButton(event.button);
			case SDL_MOUSEMOTION:
				HandleMouseMotion(event.motion);
				break;
			case SDL_QUIT:
				//End application
				break;
			case SDL_MOUSEWHEEL:
				// event.x; // Ammount scrolled horizontally
				// // If negative, scrolled to the right
				// // If positive, scrolled to the left

				// event.y; // Ammount scrolled vertically
				// // If negative, scrolled down
				// // If positive, scrolled up
				break;
			}
		}
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

	bool InputManager::isKeyDown(int key)
	{
		if (m_Locked)
		{
			return false;
		}

		return Keyboard::KeyCallback(key);
	}

	bool InputManager::isKeyUp(int key)
	{
		if (m_Locked)
		{
			return false;
		}

		return Keyboard::KeyCallback(key);
	}

	bool InputManager::isKeyPressed(SDL_Scancode key)
	{
		if (m_Locked)
		{
			return false;
		}
		
		return Keyboard::KeyCallback(key);
	}

	void InputManager::Lock()
	{
		m_Locked = true;
	}

	void InputManager::Unlock()
	{
		m_Locked = false;
	}
}