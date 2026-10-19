#pragma once

// Needed for the entt lib
#ifndef NOMINMAX
#define NOMINMAX
#endif

#ifdef EMSCRIPTEN
#include <emscripten/emscripten.h>
#endif

#include <cstdint>
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
#include "Ecs/Components/IncludeList.hpp"
#include "Ecs/ECSManager.hpp"
#include "Ecs/Systems/BaseSystem.hpp"
#include "Input/InputManager.hpp"
#include "Physics/Collider.h"
#include "Physics/PhysicsManager.h"
#include "Physics/Rigidbody.h"
#include "Render/Camera.hpp"
#include "Render/Font.hpp"
#include "Render/FontManager.hpp"
#include "Render/Framebuffer.hpp"
#include "Render/Layer.hpp"
#include "Render/OpenGLRenderer.hpp"
#include "Render/Renderable2D.hpp"
#include "Render/Renderer2D.hpp"
#include "Render/Window.hpp"
#include "Utils/File.hpp"

class Core {
 public:
  void Start();
  static bool GetRunning() { return m_Running; }
  static rse::Renderer2D* GetRenderer() { return m_Renderer; }
  static rse::Window* GetWindow() { return m_Window; }

  // Render target the scene pass should draw into, or nullptr to draw into the
  // window's back buffer. The editor overrides this so the scene lands in its
  // viewport panel instead of the whole window.
  virtual rse::Framebuffer* GetSceneTarget() { return nullptr; }

 protected:
  Core();
  virtual ~Core();

  rse::Window* createWindow(const char* name, int width, int height);

  virtual void Init() = 0;
  virtual void Tick() {}
  virtual void Update() {}
  virtual void Render() = 0;

  const unsigned int GetFps() const { return m_FramesPerSecond; }

 private:
  void Run();

  // SDL3's SDL_GetTicks() returns Uint64 (milliseconds).
  uint64_t now = 0, last = 0;
  float deltaTime = 0.0;
  float deltaAccumulator = 0.0f;
  int frames = 0;
  unsigned int m_FramesPerSecond;
  static bool m_Running;
  static rse::Renderer2D* m_Renderer;
  static rse::Window* m_Window;
};