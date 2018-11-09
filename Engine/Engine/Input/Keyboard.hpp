#pragma once

#include <SDL.h>

namespace rse
{
	class Keyboard
	{
	public:
		static void KeyCallback(SDL_KeyboardEvent event);
		static const bool IsKeyDown(int key);
		static const bool IsKeyUp(int key);
		static const bool IsKeyPressed(int key);

	private:
		static bool s_keys[];
		static bool s_keysDown[];
		static bool s_keysUp[];
	};
}