#include "Keyboard.h"

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

bool Keyboard::KeyDown(int key)
{
	bool x = keysDown[key];
	keysDown[key] = false;
	return x;
}

bool Keyboard::KeyUp(int key)
{
	bool x = keysUp[key];
	keysUp[key] = false;
	return x;
}

bool Keyboard::Key(int key)
{
	return keys[key];
}