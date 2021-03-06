#include "Window.hpp"

#include <imgui.h>

#include "../Input/InputManager.hpp"

#ifdef EMSCRIPTEN
	#include <SDL_opengles2.h>
	#include "../Utils/imgui_impl_sdl_gles2.h"
#else
	#include <GL/glew.h>
	#include "../Utils/imgui_impl_sdl_gl3.h"
#endif // EMSCRIPTEN

namespace rse
{
	Window::Window(const char *title, int width, int height)
		: m_Title(title), m_Width(width), m_Height(height)
	{

	}

	Window::~Window()
	{
		//TODO Isolate the Imgui implementation
		ImGui_ImplSdlGL_Shutdown();

		SDL_DestroyWindow(m_Window);
		SDL_Quit();
	}

	bool Window::init()
	{
		SDL_Init(SDL_INIT_EVERYTHING);

		SDL_DisplayMode mode;
		SDL_GetCurrentDisplayMode(0, &mode);

		SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
		SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
		SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

		m_Window = SDL_CreateWindow(m_Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Width, m_Height, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (m_Window == nullptr) {
			std::cout << "Failed to create SDL Window: " << SDL_GetError() << std::endl;
			SDL_Quit();
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
		//SDL_ShowCursor(SDL_DISABLE);
		//SDL_SetWindowGrab(window, SDL_TRUE);
		//TODO Fix this, I had to Disabled VSync to test performance in release mode
		SDL_GL_SetSwapInterval(0);

		glViewport(0, 0, m_Width, m_Height);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL); 
		
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		std::cout << "OpenGL " << glGetString(GL_VERSION) << std::endl;

		//TODO Isolate the Imgui implementation
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplSdlGL_Init(m_Window);
		ImGui::StyleColorsDark();

		return true;
	}

	bool Window::getRunning()
	{
		//TODO I need to rethink the quit logic for the window
		return !InputManager::IsQuitRequested();
	}
}