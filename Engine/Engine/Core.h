#pragma once

#ifdef EMSCRIPTEN
	#include <emscripten/emscripten.h>
#endif

#include <vector>
#include <glm\glm.hpp>

#include "Components\Imgui.h"
#include "Components\Label.h"
#include "Components\Sprite.h"
#include "Components\TextureManager.h"
#include "Graphics\Renderer2D.h"
#include "Graphics\OpenGLRenderer.h"
#include "Graphics\Window.h"
#include "Graphics\Layer.h"
#include "Graphics\Font.h"
#include "Graphics\FontManager.h"
#include "IO\InputManager.h"
#include "Physics\PhysicsManager.h"
#include "Physics\Collider.h"
#include "Physics\Rigidbody.h"
#include "Scene.h"

#include "Audio\Music.h"
#include "Audio\Sound.h"
#include "Audio\SoundManager.h"

class Core
{
public:
	void start();

protected:
	Core();
	virtual ~Core();

	graphics::Window* createWindow(const char *name, int width, int height);

	virtual void init() = 0;
	virtual void tick() { }
	virtual void update() { }
	virtual void render() = 0;

	const unsigned int getFPS() const { return m_FramesPerSecond; }

	bool AddScene(Scene* scene);
	bool ChangeScene(Scene* newScene);

private:
	void run();

	graphics::Window* m_Window;
	long now, last = 0;
	float deltaTime = 0.0;
	float deltaAccumulator = 0.0f;
	int frames = 0;
	unsigned int m_FramesPerSecond;
	
	std::vector<Scene*> m_Scenes;
	unsigned int m_ActiveScene;
};