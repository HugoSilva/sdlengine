#pragma once

namespace rse {
class Scene;
}

class BaseSystem {
 public:
  BaseSystem() {}
  virtual ~BaseSystem() {}

  // The frame's drawing phases: pre-scene, then scene, once per frame. Systems
  // submit their quads here.
  //
  // `scene` is the world the frame is drawing. A system never reaches for the
  // active scene itself: the caller owns which scene is current, and taking it
  // as an argument is what lets a play session run a copy of the open scene.
  //
  // `alpha` is how far the frame is into the next simulation step, in [0, 1].
  // A render system mixes the previous step's transform into the current one by
  // this much, so a 60 Hz simulation looks smooth on a faster display. It is 1
  // while the scene is stopped, so a stopped scene draws exactly what it
  // contains rather than a mix of two steps that never happened.
  virtual void Update(rse::Scene&, const float) {}

  // The simulation phase, run zero or more times per frame, every run advancing
  // by the same `step` seconds. Logic that must not depend on the frame rate
  // belongs here.
  virtual void UpdateSimulation(rse::Scene&, const float) {}

  // UI half of the frame, run while the ImGui frame is open and before the
  // scene is drawn.
  virtual void RenderUi(rse::Scene&) {}
};
