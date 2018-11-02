#include "Mouse.hpp"

namespace Input
{
	double Mouse::s_x = 0;
	double Mouse::s_y = 0;
	// TODO: fix the initialization of the buttons
	bool Mouse::s_buttons[MAX_BUTTON_SIZE] = { 0 };
	bool Mouse::s_buttonsDown[MAX_BUTTON_SIZE] = { 0 };
	bool Mouse::s_buttonsUp[MAX_BUTTON_SIZE] = { 0 };

	double Mouse::s_lastX{ 0 }, Mouse::s_lastY{ 0 };
	double Mouse::s_xOffset, Mouse::s_yOffset;
	bool Mouse::s_firstMouse{ true };
	
	void Mouse::MousePosCallback(SDL_MouseMotionEvent event)
	{
		int width, height;
		SDL_GetWindowSize(SDL_GetWindowFromID(event.windowID), &width, &height);

		s_x = event.x;
		s_y = event.y;

		if (s_firstMouse)
		{
			s_lastX = s_x;
			s_lastY = s_y;
			s_firstMouse = false;
		}

		s_xOffset = s_x - s_lastX;
		s_yOffset = s_lastY - s_y;  // Reversed since y-coordinates go from bottom to left

		s_lastX = s_x;
		s_lastY = s_y;
	}

	void Mouse::MouseButtonCallback(SDL_MouseButtonEvent event)
	{
		Uint8 button = event.button;
		if (button < 0)
			return;

		if (event.state != SDL_RELEASED && s_buttons[button] == false)
		{
			s_buttonsDown[button] = true;
			s_buttonsUp[button] = false;
		}

		if (event.state == SDL_RELEASED && s_buttons[button] == true)
		{
			s_buttonsDown[button] = false;
			s_buttonsUp[button] = true;
		}

		s_buttons[button] = event.state != SDL_RELEASED;
	}

	const bool Mouse::IsButtonDown(int button)
	{
		if (button < 0 || button >= MAX_BUTTON_SIZE)
		{
			return false;
		}
		return s_buttonsDown[button];
	}

	const bool Mouse::IsButtonUp(int button)
	{
		if (button < 0 || button >= MAX_BUTTON_SIZE)
		{
			return false;
		}
		return s_buttonsUp[button];
	}

	const bool Mouse::IsButtonPressed(int button)
	{
		if (button < 0 || button >= MAX_BUTTON_SIZE)
		{
			return false;
		}
		return s_buttons[button];
	}
}