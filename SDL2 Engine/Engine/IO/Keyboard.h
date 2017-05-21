#ifndef KEYBOARD
#define KEYBOARD

#include <sdl2/SDL.h>

class Keyboard
{
public:
	static void KeyCallback(SDL_KeyboardEvent event);

	static bool KeyDown(int key);
	static bool KeyUp(int key);
	static bool Key(int key);
private:
	static bool keys[];
	static bool keysDown[];
	static bool keysUp[];
};

#endif
