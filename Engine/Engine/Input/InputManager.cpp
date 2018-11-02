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

namespace Input
{
	bool InputManager::m_AvailableKeyboard;
	bool InputManager::m_AvailableMouse;
	bool InputManager::m_AvailableController;
	bool InputManager::m_Locked;
	bool InputManager::m_QuitRequested;

	void InputManager::Init()
	{
		// TODO: check available input devices
		m_AvailableKeyboard = true;
		m_AvailableMouse = true;
		m_AvailableController = false;
		
		m_Locked = false;
		m_QuitRequested = false;
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
				m_QuitRequested = true;
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
		if (m_AvailableKeyboard) {
			Keyboard::KeyCallback(keyEvent);
		}
	}

	void InputManager::HandleMouseButton(SDL_MouseButtonEvent mouseEvent)
	{
		if (m_AvailableMouse) {
			Mouse::MouseButtonCallback(mouseEvent);
		}
	}

	void InputManager::HandleMouseMotion(SDL_MouseMotionEvent mouseEvent)
	{
		if (m_AvailableMouse) {
			Mouse::MousePosCallback(mouseEvent);
		}
	}

	void InputManager::HandleControllerButton(SDL_ControllerButtonEvent event)
	{
	}

	void InputManager::HandleControllerAxis(SDL_ControllerAxisEvent event)
	{
	}

	bool InputManager::IsKeyDown(int key)
	{
		if (m_Locked)
		{
			return false;
		}

		return Keyboard::IsKeyDown(key);
	}

	bool InputManager::IsKeyUp(int key)
	{
		if (m_Locked)
		{
			return false;
		}

		return Keyboard::IsKeyUp(key);
	}

	bool InputManager::IsKeyPressed(SDL_Scancode key)
	{
		if (m_Locked)
		{
			return false;
		}
		
		return Keyboard::IsKeyPressed(key);
	}

	void InputManager::Lock()
	{
		m_Locked = true;
	}

	void InputManager::Unlock()
	{
		m_Locked = false;
	}
}