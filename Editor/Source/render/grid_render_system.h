#pragma once

#include <core.h>
#include <editor_manager.h>

#include "grid_renderer.h"
#include <camera/editor_camera.h>

// ---------------------------------------------------------------------------
// Submits the grid before anything that draws the scene.
//
// This system is the reason SystemPhase exists: at a single depth the draw order
// is what decides who is on top, and the grid has to lose to the sprites.
// ---------------------------------------------------------------------------
class GridRenderSystem : public BaseSystem {
 public:
  GridRenderSystem(rse::Renderer2D* renderer, const EditorCamera* camera)
      : BaseSystem(), m_renderer(renderer), m_camera(camera) {}

  void Update(const float alpha) override {
    if (m_camera == nullptr) {
      return;
    }

    m_grid.Submit(m_renderer, *m_camera, EditorManager::Settings().grid);
  }

  GridRenderer& Grid() { return m_grid; }

 private:
  rse::Renderer2D* m_renderer{nullptr};
  const EditorCamera* m_camera{nullptr};
  GridRenderer m_grid;
};
