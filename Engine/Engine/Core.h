#pragma once

#include "Components\Label.h"
#include "Components\Sprite.h"
#include "Graphics\Renderer2D.h"
#include "Graphics\OpenGLRenderer.h"
#include "Graphics\Window.h"
#include "Graphics\Layer.h"
#include "Components\TextureManager.h"
#include "Scene.h"

#include "Audio\Music.h"
#include "Audio\Sound.h"
#include "Audio\SoundManager.h"

#include <glm\glm.hpp>

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

	void LoadScene(Scene* scene);

private:
	void run();

	graphics::Window* m_Window;
	long now, last = 0;
	float deltaTime = 0.0;
	float deltaAccumulator = 0.0f;
	int frames = 0;
	unsigned int m_FramesPerSecond;

	Scene* m_Scene;
};