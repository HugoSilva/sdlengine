#include "Keyboard.hpp"

namespace rse
{
	bool Keyboard::s_keys[SDL_NUM_SCANCODES] = { 0 };
	bool Keyboard::s_keysDown[SDL_NUM_SCANCODES] = { 0 };
	bool Keyboard::s_keysUp[SDL_NUM_SCANCODES] = { 0 };

	void Keyboard::KeyCallback(SDL_KeyboardEvent event)
	{
		SDL_Scancode key = event.keysym.scancode;
		if (key < 0)
			return;

		if (event.state != SDL_RELEASED && s_keys[key] == false)
		{
			s_keysDown[key] = true;
			s_keysUp[key] = false;
		}

		if (event.state == SDL_RELEASED && s_keys[key] == true)
		{
			s_keysDown[key] = false;
			s_keysUp[key] = true;
		}

		s_keys[key] = event.state != SDL_RELEASED;
	}

	const bool Keyboard::IsKeyDown(int key)
	{
		if (key < 0 || key >= SDL_NUM_SCANCODES)
		{
			return false;
		}
		return s_keysDown[key];
	}

	const bool Keyboard::IsKeyUp(int key)
	{
		if (key < 0 || key >= SDL_NUM_SCANCODES)
		{
			return false;
		}
		return s_keysUp[key];
	}

	const bool Keyboard::IsKeyPressed(int key)
	{
		if (key < 0 || key >= SDL_NUM_SCANCODES)
		{
			return false;
		}
		return s_keys[key];
	}
}