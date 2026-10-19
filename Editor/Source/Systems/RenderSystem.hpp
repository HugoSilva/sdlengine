#pragma once

#include <Core.h>

#include "../Layout/EditorLayout.hpp"

// Bridges the ECS system list to the editor UI: it is the one system that runs
// in the UI phase, and all it does is hand control to EditorLayout.
class RenderSystem : public BaseSystem {
 public:
  RenderSystem(EditorLayout* layout) : BaseSystem(), m_Layout(layout) {}

  void renderUI() override { m_Layout->draw(); }

 private:
  EditorLayout* m_Layout;
};
