#pragma once

#include <vector>

namespace rse {
class Scene;
}

class BaseSystem;  // forward declaration for BaseSystem

namespace rse {
// When a system runs relative to everything else.
//
// There is no depth to lean on in the 2D scene pass -- every sprite sits at
// the same depth -- so anything that has to end up *under* the sprites, the
// editor's grid, has to be submitted before the systems that draw them.
//
// kSimulation is the scene's logic rather than its picture. The editor skips
// this phase unless the scene is playing, while the render phases keep running
// so a stopped scene still shows what it contains.
enum class SystemPhase { kPreScene, kSimulation, kScene };

// Runs the systems over one scene per frame.
//
// The manager owns the systems, not the world: the scene is handed in by the
// caller that owns it -- Core, with whatever SceneManager says is active -- so
// there is no static path from here to an active scene. A system never reaches
// for one either; it receives the scene it runs on.
class ECSManager {
 public:
  ECSManager() {}

  static void Init();

  static void AddSystem(BaseSystem* system, SystemPhase phase = SystemPhase::kScene);

  // Runs the frame's drawing phases -- pre-scene, then scene -- with the
  // frame's interpolation alpha. Simulation is not run here: it advances in
  // fixed steps, through UpdateSimulation, before this is called.
  static void Update(Scene& scene, float alpha);

  // Runs the simulation phase once, advancing by `step` seconds. Core calls
  // this zero or more times per frame, however many whole steps the frame owes,
  // and only while the scene is playing.
  static void UpdateSimulation(Scene& scene, float step);

  static void UpdateUi(Scene& scene);
  static void Clean();

 private:
  static std::vector<BaseSystem*> m_systems_list;
  static std::vector<BaseSystem*> m_pre_scene_systems;
  static std::vector<BaseSystem*> m_simulation_systems;
};
}  // namespace rse
