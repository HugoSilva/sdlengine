#include "Window.hpp"

#include "../Input/InputManager.hpp"

#ifdef EMSCRIPTEN
	#include <SDL3/SDL_opengles2.h>
#else
	#include <GL/glew.h>
#endif // EMSCRIPTEN

namespace rse
{
	Window::Window(const char *title, int width, int height)
		: m_Title(title), m_Width(width), m_Height(height)
	{

	}

	Window::~Window()
	{
		// ImGui is owned by rse::ImGuiLayer now (see ImGuiLayer::shutdown()).
		SDL_DestroyWindow(m_Window);
		SDL_Quit();
	}

	bool Window::init()
	{
		// SDL3 removed SDL_INIT_EVERYTHING; ask for exactly what the engine uses.
		// VIDEO/AUDIO/GAMEPAD all imply SDL_INIT_EVENTS.
		if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMEPAD))
		{
			std::cout << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
			return false;
		}

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		// SDL3 dropped the x/y window position arguments (SDL_WINDOWPOS_CENTERED);
		// SDL now places the window, or you can SDL_SetWindowPosition() later.
		m_Window = SDL_CreateWindow(m_Title, m_Width, m_Height, SDL_WINDOW_HIGH_PIXEL_DENSITY | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (m_Window == nullptr) {
			std::cout << "Failed to create SDL Window: " << SDL_GetError() << std::endl;
			SDL_Quit();
			return false;
		}

		m_Context = SDL_GL_CreateContext(m_Window);

		#ifndef EMSCRIPTEN
			glewExperimental = GL_TRUE;
			GLenum result = glewInit();
			if (result != GLEW_OK)
			{
				std::cout << "Failed to initialize GLEW: " << glewGetErrorString(result) << std::endl;
			}
		#endif

		//TODO show cursor and window focus 
		//SDL_HideCursor();
		//TODO Fix this, I had to Disabled VSync to test performance in release mode
		SDL_GL_SetSwapInterval(0);

		glViewport(0, 0, m_Width, m_Height);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL); 
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

		// ImGui is initialised separately once the window exists -- see
		// rse::ImGuiLayer::init(), called from Core.
		return true;
	}

	bool Window::getRunning()
	{
		//TODO I need to rethink the quit logic for the window
		return !InputManager::IsQuitRequested();
	}
}
