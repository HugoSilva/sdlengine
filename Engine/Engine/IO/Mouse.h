#pragma once

#include <SDL.h>

namespace IO
{
	#define MAX_BUTTON_SIZE 32

	class Mouse
	{
	public:
		static void MousePosCallback(SDL_MouseMotionEvent event);
		static void MouseButtonCallback(SDL_MouseButtonEvent event);

		static double GetMouseX();
		static double GetMouseY();
		static bool IsButtonDown(int button);
		static bool IsButtonUp(int button);
		static bool IsButtonPressed(int button);

	private:
		static double x;
		static double y;

		static bool buttons[];
		static bool buttonsDown[];
		static bool buttonsUp[];
	};
}