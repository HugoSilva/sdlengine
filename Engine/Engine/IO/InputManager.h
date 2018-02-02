#pragma once

#include <SDL.h>
#include "Mouse.h"
#include "Keyboard.h"

namespace IO
{
	class InputManager
	{
	public:
		static void init();
		static void Update();
		static void HandleKeyboard(SDL_KeyboardEvent event);
		static void HandleMouseButton(SDL_MouseButtonEvent event);
		static void HandleMouseMotion(SDL_MouseMotionEvent event);
		static void HandleControllerButton(SDL_ControllerButtonEvent event);
		static void HandleControllerAxis(SDL_ControllerAxisEvent event);
		static void clean();

		void HandleKeyboard(SDL_KeyboardEvent windowEvent);
		void HandleMouseButton(SDL_MouseButtonEvent windowEvent);
		void HandleMouseMotion(SDL_MouseMotionEvent windowEvent);

		bool isKeyDown(int key);
		bool isKeyUp(int key);
		bool isKeyPressed(SDL_Scancode key);

		void Lock();
		void Unlock();

	private:
		InputManager();

		bool availableKeyboard;
		bool availableMouse;
		bool availableController;
		bool m_Locked;
	};
}