#pragma once

class BaseSystem {
 public:
  BaseSystem() {}
  virtual ~BaseSystem() {}

  // The frame's drawing phases: pre-scene, then scene, once per frame. Systems
  // submit their quads here.
  //
  // `alpha` is how far the frame is into the next simulation step, in [0, 1].
  // A render system mixes the previous step's transform into the current one by
  // this much, so a 60 Hz simulation looks smooth on a faster display. It is 1
  // while the scene is stopped, so a stopped scene draws exactly what it
  // contains rather than a mix of two steps that never happened.
  virtual void Update(const float alpha) {}

  // The simulation phase, run zero or more times per frame, every run advancing
  // by the same `step` seconds. Logic that must not depend on the frame rate
  // belongs here.
  virtual void UpdateSimulation(const float step) {}

  // UI half of the frame, run while the ImGui frame is open and before the
  // scene is drawn.
  virtual void RenderUi() {}
};