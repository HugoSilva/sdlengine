#pragma once

#include <SDL.h>
#include "Mouse.h"
#include "Keyboard.h"

namespace IO {

	class InputManager
	{
	public:
		InputManager();
		~InputManager();

		void HandleKeyboard(SDL_KeyboardEvent windowEvent);
		void HandleMouseButton(SDL_MouseButtonEvent windowEvent);
		void HandleMouseMotion(SDL_MouseMotionEvent windowEvent);

	private:
		bool availableKeyboard;
		bool availableMouse;
		bool availableController;
	};
}