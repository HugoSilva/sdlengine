#include "Window.h"

#include <imgui.h>

#include "../IO/InputManager.h"
#include "../Audio/SoundManager.h"
#include "../Graphics/FontManager.h"
#include "../Components/TextureManager.h"
#include "../Physics/PhysicsManager.h"
#include "../Ecs/ECSManager.hpp"

#ifdef EMSCRIPTEN
	#include <SDL_opengles2.h>
	#include "../Utils/imgui_impl_sdl_gles2.h"
#else
	#include <GL/glew.h>
	#include "../Utils/imgui_impl_sdl_gl3.h"
#endif // EMSCRIPTEN

namespace graphics
{
	Window::Window(const char *title, int width, int height)
		: m_Title(title), m_Width(width), m_Height(height)
	{

	}

	Window::~Window()
	{
		//TODO move manager clean logic to the core
		audio::SoundManager::clean();
		graphics::FontManager::clean();
		TextureManager::clean();
		PhysicsManager::Clean();
		ecs::ECSManager::clean();

		ImGui_ImplSdlGL_Shutdown();

		SDL_DestroyWindow(window);
		SDL_Quit();
	}

	bool Window::Init()
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

		window = SDL_CreateWindow(m_Title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, m_Width, m_Height, SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		if (window == nullptr) {
			std::cout << "Failed to create SDL Window: " << SDL_GetError() << std::endl;
			SDL_Quit();
		}

		context = SDL_GL_CreateContext(window);

		#ifndef EMSCRIPTEN
			glewExperimental = GL_TRUE;
			GLenum result = glewInit();
			if (result != GLEW_OK)
			{
				std::cout << "Failed to initialize GLEW: " << glewGetErrorString(result) << std::endl;
			}
		#endif

		audio::SoundManager::init();
		graphics::FontManager::init();
		TextureManager::init();
		IO::InputManager::Init();
		PhysicsManager::init();

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

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		ImGui_ImplSdlGL_Init(window);
		ImGui::StyleColorsDark();

		return true;
	}

	bool Window::GetRunning()
	{
		return !IO::InputManager::IsQuitRequested();
	}
}