#pragma once

#include <array>
#include <cmath>
#include <core.h>
#include <cstdio>
#include <editor_manager.h>
#include <imgui.h>
#include <optional>

#include <glm/glm.hpp>

#include <camera/editor_camera.h>
#include <gizmo/gizmo.h>
#include <layout/editor_panel.h>
#include <scene/entity_bounds.h>

// ---------------------------------------------------------------------------
// The main scene viewport.
//
// The scene is not drawn into this window -- it is drawn into an offscreen
// framebuffer (handed to Core through EditorManager::setSceneTarget) and the
// resulting texture is shown here as an image. That indirection is what lets
// the scene behave like any other dockable panel, and it is the same shape a
// game view or a second viewport would take later on.
//
// The panel publishes its framebuffer's real pixel size to the camera every
// frame: the projection is built from the *shape of the target*, so a panel
// that is not 16:9 shows more world rather than a stretched one.
// ---------------------------------------------------------------------------
class ViewportPanel : public EditorPanel {
 public:
  explicit ViewportPanel(EditorCamera* camera) : m_camera(camera) {}

  const char* Id() const override { return "Viewport"; }

  void Draw() override {
    if (!ImGui::Begin(Id(), &visible)) {
      // Collapsed. There is nothing to present, but the scene still needs
      // a target: without one it would be drawn into the window and show
      // through the dock space, whose host window has no background of
      // its own. The framebuffer keeps its last size instead.
      ClaimSceneTarget();
      ImGui::End();
      return;
    }

    const ImVec2 available = ImGui::GetContentRegionAvail();
    const float dpi_scale = DisplayScale();

    m_framebuffer.Resize(static_cast<int>(available.x * dpi_scale), static_cast<int>(available.y * dpi_scale));

    if (m_camera != nullptr) {
      m_camera->SetViewportSize(
          glm::vec2(static_cast<float>(m_framebuffer.GetWidth()), static_cast<float>(m_framebuffer.GetHeight())),
          dpi_scale);
    }

    // The image's own screen position: mouse input arrives in window
    // coordinates and has to be made relative to this.
    const ImVec2 image_pos = ImGui::GetCursorScreenPos();

    // GL textures are bottom-up, ImGui images are top-down.
    ImGui::Image((ImTextureID)(intptr_t)m_framebuffer.GetColorTexture(), available, ImVec2(0.0f, 1.0f),
                 ImVec2(1.0f, 0.0f));

    HandleNavigation(image_pos);

    // The gizmo gets the press before the picking does: a click on a handle
    // is a drag, not a selection.
    const bool gizmo_owns_mouse = HandleGizmo(image_pos);

    HandlePicking(image_pos, gizmo_owns_mouse);
    HandleShortcuts();
    DrawSelection(image_pos);
    DrawGizmo(image_pos);
    DrawHud(image_pos);

    // Tells Core::run() where the scene pass should draw this frame.
    ClaimSceneTarget();

    ImGui::End();
  }

 private:
  static float DisplayScale() {
    const float scale = ImGui::GetIO().DisplayFramebufferScale.x;
    return scale > 0.0f ? scale : 1.0f;
  }

  void ClaimSceneTarget() { EditorManager::SetSceneTarget(&m_framebuffer); }

  void HandleNavigation(const ImVec2& image_pos) {
    if (m_camera == nullptr) {
      return;
    }

    // The hover test only decides whether a gesture *starts*. Once a pan is
    // latched it is the button, not the cursor position, that keeps it
    // going -- otherwise the pan stops the moment a drag leaves the panel.
    const bool hovered = ImGui::IsItemHovered();
    const ImGuiIO& io = ImGui::GetIO();

    if (hovered && io.MouseWheel != 0.0f) {
      // Anchored on the cursor, so whatever is under it stays under it.
      // The mouse and the image are both in ImGui pixels, so this is a
      // subtraction; the camera turns it into device pixels.
      const glm::vec2 anchor(io.MousePos.x - image_pos.x, io.MousePos.y - image_pos.y);
      m_camera->ZoomAt(io.MouseWheel, anchor);
    }

    // Middle-drag pans, as it does in most editors.
    if (hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Middle)) {
      m_panning = true;
    }

    if (!ImGui::IsMouseDown(ImGuiMouseButton_Middle)) {
      m_panning = false;
    }

    if (m_panning) {
      m_camera->Pan(glm::vec2(io.MouseDelta.x, io.MouseDelta.y));
    }
  }

  // The gizmo's mouse work, and whether it took the press. Guarded here rather
  // than in the gizmo so that a panel with no camera does not have to know
  // about one.
  bool HandleGizmo(const ImVec2& image_pos) {
    if (m_camera == nullptr) {
      return false;
    }

    return m_gizmo.HandleInput(*m_camera, image_pos, ImGui::IsItemHovered());
  }

  void DrawGizmo(const ImVec2& image_pos) {
    if (m_camera == nullptr) {
      return;
    }

    m_gizmo.Draw(*m_camera, image_pos);
  }

  // A left click that does not turn into a drag selects what is under it;
  // clicking empty space deselects. Latched on press and judged on release, so
  // a drag -- a pan, or one of the gizmo's own -- is never mistaken for a
  // click. `taken` is this frame's answer from the gizmo: pressing a handle
  // starts a drag, and the same press must not also re-pick.
  void HandlePicking(const ImVec2& image_pos, bool taken) {
    if (m_camera == nullptr) {
      return;
    }

    const ImGuiIO& io = ImGui::GetIO();

    if (!taken && ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      m_click_armed = true;
      m_click_pos = io.MousePos;
    }

    if (!m_click_armed || !ImGui::IsMouseReleased(ImGuiMouseButton_Left)) {
      return;
    }

    m_click_armed = false;

    const float travelled = glm::max(std::abs(io.MousePos.x - m_click_pos.x), std::abs(io.MousePos.y - m_click_pos.y));

    if (travelled > kClickSlop) {
      return;
    }

    PickAt(glm::vec2(io.MousePos.x - image_pos.x, io.MousePos.y - image_pos.y));
  }

  // Whatever is under the cursor; the last hit wins, because the renderer has
  // no depth to sort by and "on top" means "drawn later".
  void PickAt(glm::vec2 pixel) {
    const glm::vec2 world = m_camera->ScreenToWorld(pixel);

    uint32_t hit = INVALID_ENTITY_ID;

    rse::ECSManager::GetView<TransformComponent, SpriteComponent>().each(
        [&hit, &world](uint32_t entity, TransformComponent&, SpriteComponent&) {
          const std::optional<EntityBounds> bounds = editor_bounds::OfEntity(entity);

          if (bounds && rse::PointInOrientedBox(world, bounds->center, bounds->rotation_degrees, bounds->half_extent)) {
            hit = entity;
          }
        });

    EditorManager::SetSelection(hit);
  }

  // One box per selected entity, over the scene. It is the same rectangle the
  // renderer draws -- EntityBounds mirrors submit() -- so it lands on the
  // sprite's own pixels rather than near them. An entity with no sprite has no
  // rectangle to draw, and gets a cross at its position instead of nothing.
  void DrawSelection(const ImVec2& image_pos) {
    if (m_camera == nullptr) {
      return;
    }

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    for (const uint32_t entity : EditorManager::GetSelectedEntities()) {
      // Only one entity can be selected today, so this is always the
      // active colour; the other one is what multi-select will use to
      // tell the rest of the set from the entity the inspector edits.
      const ImU32 colour = entity == EditorManager::GetSelectedEntity() ? kActiveOutline : kOutline;

      const std::optional<EntityBounds> bounds = editor_bounds::OfEntity(entity);

      if (!bounds) {
        const std::optional<glm::vec2> position = editor_bounds::PositionOf(entity);

        if (position) {
          const glm::vec2 screen = m_camera->WorldToScreen(*position);
          const ImVec2 at(image_pos.x + screen.x, image_pos.y + screen.y);

          draw_list->AddLine(ImVec2(at.x - kCrossSize, at.y), ImVec2(at.x + kCrossSize, at.y), kOutlineHalo, 4.0f);
          draw_list->AddLine(ImVec2(at.x, at.y - kCrossSize), ImVec2(at.x, at.y + kCrossSize), kOutlineHalo, 4.0f);
          draw_list->AddLine(ImVec2(at.x - kCrossSize, at.y), ImVec2(at.x + kCrossSize, at.y), colour, 2.0f);
          draw_list->AddLine(ImVec2(at.x, at.y - kCrossSize), ImVec2(at.x, at.y + kCrossSize), colour, 2.0f);
        }

        continue;
      }

      const std::array<glm::vec2, 4> corners = editor_bounds::Corners(*bounds);
      ImVec2 points[4];

      for (int i = 0; i < 4; i++) {
        const glm::vec2 screen = m_camera->WorldToScreen(corners[i]);
        points[i] = ImVec2(image_pos.x + screen.x, image_pos.y + screen.y);
      }

      // Halo first, colour over it. An outline is one colour and the scene
      // is whatever colour it likes: measured against the test sprite, the
      // amber line landed on a pixel of (200,95,21) and the two oranges
      // washed into each other.
      draw_list->AddPolyline(points, 4, kOutlineHalo, 4.0f, ImDrawFlags_Closed);
      draw_list->AddPolyline(points, 4, colour, 2.0f, ImDrawFlags_Closed);
    }
  }

  // F frames the selection, or the whole scene when nothing is selected. The
  // hierarchy's double-click asks for the same thing, through EditorManager,
  // because it happens in another panel.
  void HandleShortcuts() {
    if (m_camera == nullptr) {
      return;
    }

    const ImGuiIO& io = ImGui::GetIO();
    const bool viewport_active =
        ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) || ImGui::IsItemHovered();

    // Not while a text field has the keyboard: `W` is a letter before it is a
    // shortcut.
    const bool keys = viewport_active && !io.WantTextInput;

    // W/E/R choose the gizmo, as they do in most editors. Nothing else has to
    // happen on a mode change: the gizmo reads the mode every frame and so
    // does the HUD, so the handle set and the readout follow in the same
    // frame.
    if (keys) {
      EditorSettings& settings = EditorManager::Settings();
      const EditorSettings::GizmoMode previous = settings.gizmo.mode;

      if (ImGui::IsKeyPressed(ImGuiKey_W)) {
        settings.gizmo.mode = EditorSettings::GizmoMode::kTranslate;
      } else if (ImGui::IsKeyPressed(ImGuiKey_E)) {
        settings.gizmo.mode = EditorSettings::GizmoMode::kRotate;
      } else if (ImGui::IsKeyPressed(ImGuiKey_R)) {
        settings.gizmo.mode = EditorSettings::GizmoMode::kScale;
      }

      // Only on a real change: holding a key down would otherwise rewrite
      // the file once a frame for nothing.
      if (settings.gizmo.mode != previous) {
        settings.Save();
      }
    }

    const bool pressed = keys && ImGui::IsKeyPressed(ImGuiKey_F);

    // Consumed even when the key was the reason, so a request cannot be
    // left pending and fire a second time later.
    const bool requested = EditorManager::ConsumeFrameRequest();

    if (pressed || requested) {
      FrameSelection();
    }
  }

  void FrameSelection() {
    // The selection first, then everything there is. An empty scene has
    // nothing to frame, so the view is left alone rather than zoomed to its
    // maximum on the origin.
    std::optional<rse::Bounds2D> bounds;

    const std::optional<EntityBounds> selection = editor_bounds::OfEntity(EditorManager::GetSelectedEntity());
    if (selection) {
      // The box that frames it, which is not the box that gets drawn: a
      // rotated entity needs its corners boxed first.
      bounds = editor_bounds::Aabb(*selection);
    }

    if (!bounds) {
      bounds = editor_bounds::OfScene();
    }

    if (!bounds) {
      return;
    }

    m_camera->FocusOn(bounds->Center(), bounds->Size());
  }

  // Zoom, camera centre and gizmo mode, over the top-left of the scene. Drawn
  // into the draw list rather than as widgets: widgets here would add to the
  // window's content size and could give the panel scrollbars.
  void DrawHud(const ImVec2& image_pos) {
    if (m_camera == nullptr) {
      return;
    }

    char text[192];
    snprintf(text, sizeof(text), "Zoom %.0f%%\nCentre %.0f, %.0f\nGizmo %s", m_camera->GetZoom() * 100.0f,
             m_camera->GetCenter().x, m_camera->GetCenter().y, GizmoModeName(EditorManager::Settings().gizmo.mode));

    const ImVec2 text_size = ImGui::CalcTextSize(text);
    const ImVec2 block_min(image_pos.x + 6.0f, image_pos.y + 6.0f);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(block_min, ImVec2(block_min.x + text_size.x + 8.0f, block_min.y + text_size.y + 8.0f),
                             IM_COL32(0, 0, 0, 140), 3.0f);
    draw_list->AddText(ImVec2(block_min.x + 4.0f, block_min.y + 4.0f), IM_COL32(235, 235, 235, 255), text);
  }

  // A click is a press and a release that did not travel. The slop is wide
  // enough for an unsteady hand and narrow enough that a pan, or one of M5's
  // gizmo drags, can never be mistaken for one.
  static constexpr float kClickSlop{4.0f};

  // Half the arm of the cross drawn for a selected entity with no sprite. It
  // is deliberately bigger than the gizmo's centre square, which lands on the
  // same point -- but the gizmo's arrows run along the cross's +X and +Y arms,
  // so what actually shows is the marker's other two halves. That is enough
  // for what it is for: saying that the selected thing is here, and has no
  // size, when there is no rectangle to outline.
  static constexpr float kCrossSize{12.0f};

  static constexpr ImU32 kActiveOutline{IM_COL32(255, 176, 0, 255)};
  static constexpr ImU32 kOutline{IM_COL32(190, 190, 190, 255)};

  // Drawn under the colour and one pixel wider on each side, so the outline
  // reads against content of its own colour as well as against the scene.
  static constexpr ImU32 kOutlineHalo{IM_COL32(0, 0, 0, 200)};

  EditorCamera* m_camera{nullptr};
  Gizmo m_gizmo;
  rse::Framebuffer m_framebuffer;

  // Latched on middle-press and cleared on release, so a pan survives the
  // cursor leaving the panel mid-drag.
  bool m_panning{false};

  // The same latch for the left button, so a click can be judged by how far
  // the cursor travelled rather than by whether it is still over the image.
  bool m_click_armed{false};
  ImVec2 m_click_pos{0.0f, 0.0f};
};
