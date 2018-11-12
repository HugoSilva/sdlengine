#pragma once

#include <SDL.h>

namespace rse
{
	#define MAX_BUTTON_SIZE 32

	class Mouse
	{
	public:
		static void MousePosCallback(SDL_MouseMotionEvent event);
		static void MouseButtonCallback(SDL_MouseButtonEvent event);

		inline static const double GetMouseX() { return s_xOffset; }
		inline static const double GetMouseY() { return s_yOffset; }
		static const bool IsButtonDown(int button);
		static const bool IsButtonUp(int button);
		static const bool IsButtonPressed(int button);

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