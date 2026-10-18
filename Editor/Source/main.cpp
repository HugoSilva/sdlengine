#include <Core.h>
#include <glm/glm.hpp>
#include "./Views/SetupView.h"
#include "./EditorManager.h"

#if _WIN32
extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

class Game : public Core
{

public:
	Game()
	{

	}

	~Game()
	{
	}

	void init() override
	{
		m_Window = createWindow("RockSlide Engine", 1280, 720);

		SceneManager::add(new SetupView("Setup", Core::getWindow()->getWindow()));

		#ifdef EMSCRIPTEN
		#else
			ThreadManager::init();
			ThreadManager::start();
		#endif
	}

	void render() override
	{
		
	}

	// The editor renders the scene into its viewport panel's framebuffer
	// instead of straight to the window.
	rse::Framebuffer* getSceneTarget() override { return EditorManager::getSceneTarget(); }

private:
	rse::Window * m_Window;
};

int main(int argc, char **argv)
{
	Game game;
	game.start();
	return EXIT_SUCCESS;
}