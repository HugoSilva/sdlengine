#pragma once

#include <iostream>
#include <SDL.h>

namespace graphics
{
	class Window
	{
	public:
		Window(const char *title, int width, int height);
		~Window();

		bool init();
		bool getRunning();
		SDL_Window* getWindow() { return m_Window; };

	private:
		SDL_Event m_WindowEvent;
		SDL_Window* m_Window;
		SDL_GLContext m_Context;

		const char* m_Title;
		int m_Width, m_Height;
	};
}