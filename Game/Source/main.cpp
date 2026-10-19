#include <Core.h>

#include <glm/glm.hpp>

#include "Scene01.h"

#if _WIN32
extern "C" {
_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

class Game : public Core {
 public:
  Game() {}

  ~Game() {}

  void Init() override {
    // m_Window = createWindow("RockSlide Engine", 1280, 720);
    SceneManager::Add(new SpriteTest(Core::GetWindow()->GetWindow()));
#ifdef EMSCRIPTEN
#else
    ThreadManager::Init();
    ThreadManager::Start();
#endif
  }

  void Render() override {}

 private:
  rse::Window* m_Window;
};

int main(int argc, char** argv) {
  Game game;
  game.Start();
  return EXIT_SUCCESS;
}