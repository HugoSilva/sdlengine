#include <core.h>

#include <glm/glm.hpp>

#if _WIN32
extern "C" {
_declspec(dllexport) DWORD nv_optimus_enablement = 0x00000001;
__declspec(dllexport) int amd_power_xpress_request_high_performance = 1;
}
#endif

// The game: a window, the camera the player moves, and whatever scene the
// project has open. What the scene is made of lives in the ECS, and what it
// does lives in the engine's systems.
class Game : public Core {
 public:
  void Init() override {
#ifdef EMSCRIPTEN
    m_shader = new rse::Shader("Resources/ES3.vert", "Resources/ES3.frag");
#else
    m_shader = new rse::Shader("Resources/Default.vert", "Resources/Default.frag");
#endif  // EMSCRIPTEN

    m_camera = new rse::Camera(glm::vec3(0.f, 0.f, 0.f));

    rse::FontManager::Add(new rse::Font("Arial", "Resources/arial.ttf", 24));

#ifdef EMSCRIPTEN
#else
    ThreadManager::Init();
    ThreadManager::Start();
#endif
  }

  void Update(float delta_time) override { m_camera->Update(delta_time); }

  void Render() override {
    m_shader->Enable();
    m_camera->Render(m_shader);
  }

 private:
  rse::Shader* m_shader{nullptr};
  rse::Camera* m_camera{nullptr};
};

int main(int argc, char** argv) {
  Game game;
  game.Start();
  return EXIT_SUCCESS;
}
