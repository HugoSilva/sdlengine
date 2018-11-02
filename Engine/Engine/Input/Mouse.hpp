#pragma once

#include <SDL.h>

namespace Input
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
		static double s_x;
		static double s_y;

		static bool s_buttons[];
		static bool s_buttonsDown[];
		static bool s_buttonsUp[];

		static double s_lastX, s_lastY;
		static double s_xOffset, s_yOffset;
		static bool s_firstMouse;
	};
}