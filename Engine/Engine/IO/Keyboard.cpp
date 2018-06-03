#include "Keyboard.h"

namespace IO
{
	bool Keyboard::keys[SDL_NUM_SCANCODES] = { 0 };
	bool Keyboard::keysDown[SDL_NUM_SCANCODES] = { 0 };
	bool Keyboard::keysUp[SDL_NUM_SCANCODES] = { 0 };

	void Keyboard::KeyCallback(SDL_KeyboardEvent event)
	{
		SDL_Scancode key = event.keysym.scancode;
		if (key < 0)
			return;

		if (event.state != SDL_RELEASED && keys[key] == false)
		{
			keysDown[key] = true;
			keysUp[key] = false;
		}

		if (event.state == SDL_RELEASED && keys[key] == true)
		{
			keysDown[key] = false;
			keysUp[key] = true;
		}

		keys[key] = event.state != SDL_RELEASED;
	}

	bool Keyboard::IsKeyDown(int key)
	{
		if (key < 0 || key >= SDL_NUM_SCANCODES)
		{
			return false;
		}
		return keysDown[key];
	}

	bool Keyboard::IsKeyUp(int key)
	{
		if (key < 0 || key >= SDL_NUM_SCANCODES)
		{
			return false;
		}
		return keysUp[key];
	}

	bool Keyboard::IsKeyPressed(int key)
	{
		if (key < 0 || key >= SDL_NUM_SCANCODES)
		{
			return false;
		}
		return keys[key];
	}
}