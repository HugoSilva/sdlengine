#pragma once

#include "Components\Label.h"
#include "Components\Sprite.h"
#include "Graphics\Renderer2D.h"
#include "Graphics\OpenGLRenderer.h"
#include "Graphics\Window.h"
#include "Graphics\Layer.h"
#include "Components\TextureManager.h"

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
	const unsigned int getUPS() const { return m_UpdatesPerSecond; }

private:
	void run();

	graphics::Window* m_Window;
	Timer* m_Timer;
	unsigned int m_FramesPerSecond, m_UpdatesPerSecond;
}