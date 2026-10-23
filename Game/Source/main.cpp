#include <core.h>
#include <filesystem>

#include <glm/glm.hpp>

#include <ecs/scene_io.h>

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
  explicit Game(const std::filesystem::path& project_file) : Core(project_file) {}

  void Init() override {
#ifdef EMSCRIPTEN
    const std::string vertex_shader = Core::ResolveResource("ES3.vert");
    const std::string fragment_shader = Core::ResolveResource("ES3.frag");
#else
    const std::string vertex_shader = Core::ResolveResource("Default.vert");
    const std::string fragment_shader = Core::ResolveResource("Default.frag");
#endif  // EMSCRIPTEN

    m_shader = new rse::Shader(vertex_shader.c_str(), fragment_shader.c_str());

    // The manifest's mix, applied once the project is open. The scene's sounds
    // were loaded with the default volumes and are re-gained by SetVolumes.
    const rse::Project& project = Core::GetProject();
    rse::SoundManager::SetVolumes(project.audio.master, project.audio.music, project.audio.sfx);

    // The scene says where its camera looks, and the game applies that once,
    // when it starts: the arrow keys still move the camera afterwards.
    const rse::SceneIO::SceneCamera& scene_camera = rse::SceneIO::GetCamera();
    m_camera = new rse::Camera(glm::vec3(scene_camera.position, 0.0f));
    m_camera->SetZoom(scene_camera.zoom);

    rse::FontManager::Add(new rse::Font("Arial", Core::ResolveResource("arial.ttf"), 24));

#ifdef EMSCRIPTEN
#else
    ThreadManager::Init();
    ThreadManager::Start();
#endif
  }

  void Update(float delta_time) override { m_camera->Update(delta_time); }

  void Render() override {
    m_shader->Enable();
    // The batched renderer binds every batch to texture unit 0.
    m_shader->SetUniform1i("texture0", 0);
    m_camera->Render(m_shader);

    const rse::OrthoView view = m_camera->GetView();
    Core::GetRenderer()->SetViewBounds(view.Min(), view.Max());
  }

 private:
  rse::Shader* m_shader{nullptr};
  rse::Camera* m_camera{nullptr};
};

int main(int argc, char** argv) {
  const std::filesystem::path project_file = argc > 1 ? std::filesystem::path{argv[1]} : "project.xml";

  Game game(project_file);
  game.Start();
  return EXIT_SUCCESS;
}
