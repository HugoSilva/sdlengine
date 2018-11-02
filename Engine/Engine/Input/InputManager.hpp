#pragma once

#include <SDL.h>

namespace Input
{
	class InputManager
	{
	public:
		static void Init();
		static void Update();

		static bool IsKeyDown(int key);
		static bool IsKeyUp(int key);
		static bool IsKeyPressed(SDL_Scancode key);
		static void Lock();
		static void Unlock();

		static inline const bool IsQuitRequested() { return m_QuitRequested; }

	private:
		InputManager() { }

		static void HandleKeyboard(SDL_KeyboardEvent windowEvent);
		static void HandleMouseButton(SDL_MouseButtonEvent windowEvent);
		static void HandleMouseMotion(SDL_MouseMotionEvent windowEvent);
		static void HandleControllerButton(SDL_ControllerButtonEvent event);
		static void HandleControllerAxis(SDL_ControllerAxisEvent event);

		static bool m_AvailableKeyboard;
		static bool m_AvailableMouse;
		static bool m_AvailableController;
		static bool m_Locked;
		static bool m_QuitRequested;
	};
}