#include "Mouse.h"

namespace IO
{
	double Mouse::x = 0;
	double Mouse::y = 0;
	// TODO: fix the initialization of the buttons
	bool Mouse::buttons[MAX_BUTTON_SIZE] = { 0 };
	bool Mouse::buttonsDown[MAX_BUTTON_SIZE] = { 0 };
	bool Mouse::buttonsUp[MAX_BUTTON_SIZE] = { 0 };

	double lastX = 0, lastY = 0;
	double xOffset, yOffset;
	bool firstMouse = true;

	void Mouse::MousePosCallback(SDL_MouseMotionEvent event)
	{
		int width, height;
		SDL_GetWindowSize(SDL_GetWindowFromID(event.windowID), &width, &height);

		x = event.x;
		y = event.y;

		if (firstMouse)
		{
			lastX = x;
			lastY = y;
			firstMouse = false;
		}

		xOffset = x - lastX;
		yOffset = lastY - y;  // Reversed since y-coordinates go from bottom to left

		lastX = x;
		lastY = y;
	}

	void Mouse::MouseButtonCallback(SDL_MouseButtonEvent event)
	{
		Uint8 button = event.button;
		if (button < 0)
			return;

		if (event.state != SDL_RELEASED && buttons[button] == false)
		{
			buttonsDown[button] = true;
			buttonsUp[button] = false;
		}

		if (event.state == SDL_RELEASED && buttons[button] == true)
		{
			buttonsDown[button] = false;
			buttonsUp[button] = true;
		}

		buttons[button] = event.state != SDL_RELEASED;
	}

	double Mouse::GetMouseX()
	{
		return xOffset;
	}

	double Mouse::GetMouseY()
	{
		return yOffset;
	}

	bool Mouse::IsButtonDown(int button)
	{
		if (button < 0 || button >= MAX_BUTTON_SIZE)
		{
			return false;
		}
		return buttonsDown[button];
	}

	bool Mouse::IsButtonUp(int button)
	{
		if (button < 0 || button >= MAX_BUTTON_SIZE)
		{
			return false;
		}
		return buttonsUp[button];
	}

	bool Mouse::IsButtonPressed(int button)
	{
		if (button < 0 || button >= MAX_BUTTON_SIZE)
		{
			return false;
		}
		return buttons[button];
	}
}