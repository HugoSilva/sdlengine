#include <Core.h>
#include <glm\glm.hpp>
#include "./Views/SetupView.h"
#include "./Views/SelectProjectView.h"
#include "./Views/WorkSpaceView.h"

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

		SceneManager::add(new SetupView("Setup", m_Window->GetWindow()));
		SceneManager::add(new SelectProjectView("SelectProject", m_Window->GetWindow()));
		SceneManager::add(new WorkSpaceView("WorkSpace", m_Window->GetWindow()));

		#ifdef EMSCRIPTEN
		#else
			ThreadManager::Init();
			ThreadManager::Start();
		#endif
	}

	void render() override
	{
		
	}

private:
	graphics::Window * m_Window;
};

int main(int argc, char **argv)
{
	Game game;
	game.start();
	return EXIT_SUCCESS;
}