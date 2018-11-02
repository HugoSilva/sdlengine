#pragma once

#include <SDL.h>

namespace Input
{
	class InputManager
	{
	public:
		static void Init();
		static void Update();

		static const bool IsKeyDown(int key);
		static const bool IsKeyUp(int key);
		static const bool IsKeyPressed(SDL_Scancode key);
		inline static const void Lock() { s_Locked = true; }
		inline static const void Unlock() { s_Locked = false; }

		inline static const bool IsQuitRequested() { return s_QuitRequested; }

	private:
		InputManager() { }

		static void HandleKeyboard(SDL_KeyboardEvent windowEvent);
		static void HandleMouseButton(SDL_MouseButtonEvent windowEvent);
		static void HandleMouseMotion(SDL_MouseMotionEvent windowEvent);
		static void HandleControllerButton(SDL_ControllerButtonEvent event);
		static void HandleControllerAxis(SDL_ControllerAxisEvent event);

		static bool s_AvailableKeyboard;
		static bool s_AvailableMouse;
		static bool s_AvailableController;
		static bool s_Locked;
		static bool s_QuitRequested;
	};
}