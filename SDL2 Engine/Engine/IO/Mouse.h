#ifndef MOUSE
#define MOUSE

#include <sdl2/SDL.h>

class Mouse
{
public:
	static void MousePosCallback(SDL_MouseMotionEvent event);
	static void MouseButtonCallback(SDL_MouseButtonEvent event);

	static double GetMouseX();
	static double GetMouseY();

	static bool ButtonDown(int button);
	static bool ButtonUp(int button);
	static bool Button(int button);

private:
	static double x;
	static double y;

	static bool buttons[];
	static bool buttonsDown[];
	static bool buttonsUp[];
};

#endif
