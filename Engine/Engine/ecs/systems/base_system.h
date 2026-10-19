#pragma once

class BaseSystem {
 public:
  BaseSystem() {}
  virtual ~BaseSystem() {}

  // Simulation half of the frame, run while the scene is being built.
  // Systems only interested in drawing UI override renderUI() instead.
  virtual void Update(const float alpha) {}

  // UI half of the frame, run while the ImGui frame is open and before the
  // scene is drawn.
  virtual void RenderUi() {}
};