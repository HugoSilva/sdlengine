#pragma once

#include <SDL.h>

namespace Input
{
	class Keyboard
	{
	public:
		static void KeyCallback(SDL_KeyboardEvent event);
		static bool IsKeyDown(int key);
		static bool IsKeyUp(int key);
		static bool IsKeyPressed(int key);

	private:
		static bool s_keys[];
		static bool s_keysDown[];
		static bool s_keysUp[];
	};
}