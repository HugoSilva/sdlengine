#include "InputManager.hpp"

#include <imgui.h>
#include "Mouse.hpp"
#include "Keyboard.hpp"

#ifdef EMSCRIPTEN
#include <SDL_opengles2.h>
#include "../Utils/imgui_impl_sdl_gles2.h"
#else
#include <GL/glew.h>
#include "../Utils/imgui_impl_sdl_gl3.h"
#endif // EMSCRIPTEN

namespace rse
{
	bool InputManager::s_AvailableKeyboard;
	bool InputManager::s_AvailableMouse;
	bool InputManager::s_AvailableController;
	bool InputManager::s_Locked;
	bool InputManager::s_QuitRequested;

	void InputManager::Init()
	{
		// TODO: check available input devices
		s_AvailableKeyboard = true;
		s_AvailableMouse = true;
		s_AvailableController = false;
		
		s_Locked = false;
		s_QuitRequested = false;
	}

	void InputManager::Update()
	{
		SDL_Event event;
		//Cannot be called outside of the main thread
		if (SDL_PollEvent(&event))
		{
			switch (event.type)
			{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				HandleKeyboard(event.key);
				break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
				HandleMouseButton(event.button);
			case SDL_MOUSEMOTION:
				HandleMouseMotion(event.motion);
				break;
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
				HandleControllerButton(event.cbutton);
			case SDL_CONTROLLERAXISMOTION:
				HandleControllerAxis(event.caxis);
				break;
			case SDL_QUIT:
				s_QuitRequested = true;
				break;
			case SDL_MOUSEWHEEL:
				// event.x; // Ammount scrolled horizontally
				// // If negative, scrolled to the right
				// // If positive, scrolled to the left

				// event.y; // Ammount scrolled vertically
				// // If negative, scrolled down
				// // If positive, scrolled up
				break;
			}
			ImGui_ImplSdlGL_ProcessEvent(&event);
		}
	}

	void InputManager::HandleKeyboard(SDL_KeyboardEvent keyEvent)
	{
		if (s_AvailableKeyboard) {
			Keyboard::KeyCallback(keyEvent);
		}
	}

	void InputManager::HandleMouseButton(SDL_MouseButtonEvent mouseEvent)
	{
		if (s_AvailableMouse) {
			Mouse::MouseButtonCallback(mouseEvent);
		}
	}

	void InputManager::HandleMouseMotion(SDL_MouseMotionEvent mouseEvent)
	{
		if (s_AvailableMouse) {
			Mouse::MousePosCallback(mouseEvent);
		}
	}

	void InputManager::HandleControllerButton(SDL_ControllerButtonEvent event)
	{
		//TODO Add controller support
	}

	void InputManager::HandleControllerAxis(SDL_ControllerAxisEvent event)
	{
		//TODO Add controller support
	}

	const bool InputManager::IsKeyDown(int key)
	{
		if (s_Locked)
		{
			return false;
		}

		return Keyboard::IsKeyDown(key);
	}

	const bool InputManager::IsKeyUp(int key)
	{
		if (s_Locked)
		{
			return false;
		}

		return Keyboard::IsKeyUp(key);
	}

	const bool InputManager::IsKeyPressed(SDL_Scancode key)
	{
		if (s_Locked)
		{
			return false;
		}
		
		return Keyboard::IsKeyPressed(key);
	}
}