#pragma once

//Needed for the entt lib
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef EMSCRIPTEN
	#include <emscripten/emscripten.h>
#endif

#include <vector>
#include <glm/glm.hpp>
#if _WIN32
#include <Windows.h>
#endif

#include "Audio/Music.hpp"
#include "Audio/Sound.hpp"
#include "Audio/SoundManager.hpp"
#include "Components/Sprite.hpp"
#include "Core/EventManager.hpp"
#include "Core/Scene.hpp"
#include "Core/SceneManager.hpp"
#include "Core/TextureManager.hpp"
#include "Core/ThreadManager.hpp"
#include "Ecs/ECSManager.hpp"
#include "Ecs/Components/IncludeList.hpp"
#include "Ecs/Systems/BaseSystem.hpp"
#include "Render/Camera.hpp"
#include "Render/Font.hpp"
#include "Render/FontManager.hpp"
#include "Render/Renderable2D.hpp"
#include "Render/Renderer2D.hpp"
#include "Render/OpenGLRenderer.hpp"
#include "Render/Window.hpp"
#include "Render/Layer.hpp"
#include "Input/InputManager.hpp"
#include "Physics/PhysicsManager.h"
#include "Physics/Collider.h"
#include "Physics/Rigidbody.h"
#include "Utils/File.hpp"

class Core
{
public:
	void start();
	static bool getRunning() { return m_Running; }
	static graphics::Renderer2D* getRenderer() { return m_Renderer; }
	static graphics::Window* getWindow() { return m_Window; }

protected:
	Core();
	virtual ~Core();

	graphics::Window* createWindow(const char *name, int width, int height);

	virtual void init() = 0;
	virtual void tick() { }
	virtual void update() { }
	virtual void render() = 0;

	const unsigned int getFPS() const { return m_FramesPerSecond; }

private:
	void run();

	long now, last = 0;
	float deltaTime = 0.0;
	float deltaAccumulator = 0.0f;
	int frames = 0;
	unsigned int m_FramesPerSecond;
	static bool m_Running;
	static graphics::Renderer2D* m_Renderer;
	static graphics::Window* m_Window;
};