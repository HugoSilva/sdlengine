#pragma once

#include <Core.h>

#include "../Camera/EditorCamera.hpp"
#include "../EditorManager.h"
#include "GridRenderer.hpp"

// ---------------------------------------------------------------------------
// Submits the grid before anything that draws the scene.
//
// This system is the reason SystemPhase exists: at a single depth the draw order
// is what decides who is on top, and the grid has to lose to the sprites.
// ---------------------------------------------------------------------------
class GridRenderSystem : public BaseSystem {
 public:
  GridRenderSystem(rse::Renderer2D* renderer, const EditorCamera* camera)
      : BaseSystem(), m_Renderer(renderer), m_Camera(camera) {}

  void update(const float alpha) override {
    if (m_Camera == nullptr) {
      return;
    }

    m_Grid.submit(m_Renderer, *m_Camera, EditorManager::settings().grid);
  }

  GridRenderer& grid() { return m_Grid; }

 private:
  rse::Renderer2D* m_Renderer{nullptr};
  const EditorCamera* m_Camera{nullptr};
  GridRenderer m_Grid;
};
