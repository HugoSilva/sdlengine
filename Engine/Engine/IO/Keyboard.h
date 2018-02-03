#pragma once

#include <SDL.h>

namespace IO
{
	class Keyboard
	{
	public:
		static void KeyCallback(SDL_KeyboardEvent event);
		static bool IsKeyDown(int key);
		static bool IsKeyUp(int key);
		static bool IsKeyPressed(int key);

	private:
		static bool keys[];
		static bool keysDown[];
		static bool keysUp[];
	};
}