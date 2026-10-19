#pragma once

// ---------------------------------------------------------------------------
// Gizmo -- the on-screen handles for moving, turning and sizing the selection.
//
// Hand-rolled rather than ImGuizmo (a decision recorded in the plan): the
// handles are a handful of lines, triangles and circles in the panel's draw
// list and the hit tests are a box test and a ring test, which is a fraction of
// the machinery a third-party gizmo brings along -- and it draws through the
// same camera and the same draw list as the outline it sits next to.
//
// The handles are drawn at a *constant screen size*: 60 px of arm whatever the
// zoom, because a gizmo that shrinks with the view is unusable exactly when it
// is needed most. What that does not mean is that a drag is measured in world
// units. A translate drag is "the cursor moved 34 pixels along the arrow" and
// what world offset that is worth is the camera's business; a rotate drag is an
// angle about the pivot and says nothing about the camera at all.
//
// Every frame of a drag is computed from the transform as it was when the drag
// started rather than accumulated frame to frame: a latched drag that leaves
// the panel and comes back has to land where the cursor is, not where the sum
// of the deltas happened to end up. Rotation is the one place that needs the
// running total -- an angle has to be unwrapped to keep going round -- but even
// there the entity is written as "start + total", so the total cannot drift.
//
// The three modes share the latch, the grab snapshot and the snapping. What
// differs is the handle set, what the drag writes, and how the drag is turned
// into a number:
//
//   Translate  two arrows and a centre square; travel along an axis is a world
//              offset, scaled by the camera's world-per-pixel.
//   Rotate     a ring; travel is the angle swept about the pivot, and the ring
//              carries a spoke at the entity's current rotation.
//   Scale      two boxes and a centre square; travel along an axis is a ratio
//              -- one arm's length of travel doubles what was there.
//
// The handles are world-axis aligned in all three modes, including scale, so
// scaling a rotated entity scales along its own axes while the handle points
// along the screen's. Local-space handles are a real feature (every 3D editor
// has them) and are deliberately not this one.
// ---------------------------------------------------------------------------

#include <cmath>
#include <core.h>
#include <editor_manager.h>
#include <imgui.h>
#include <optional>
#include <vector>

#include <glm/glm.hpp>

#include <camera/editor_camera.h>
#include <scene/entity_bounds.h>

class Gizmo {
 public:
  // Which handle a drag is attached to: an axis arrow or box, the centre
  // square, or the rotation ring.
  enum class Axis { kNone, kX, kY, kBoth, kRing };

  // Reads the mouse for this frame and returns true when a handle owns the
  // press, so the viewport knows the click was not aimed at the scene.
  bool HandleInput(const EditorCamera& camera, const ImVec2& image_pos, bool hovered);

  void Draw(const EditorCamera& camera, const ImVec2& image_pos) const;

 private:
  // Screen pixels. The handles are the same size at every zoom by design.
  static constexpr float kArmLength{60.0f};
  static constexpr float kArrowLength{12.0f};
  static constexpr float kArrowHalfWidth{5.0f};

  // Half the thickness of an axis's grab band, half the width of the centre
  // square, and half the width of a scale handle's box.
  static constexpr float kGrabWidth{6.0f};
  static constexpr float kCentreSize{6.0f};
  static constexpr float kHandleBoxSize{5.0f};

  // The rotation ring, and how far from it the cursor can be and still grab.
  static constexpr float kRingRadius{60.0f};
  static constexpr float kRingGrabWidth{8.0f};

  // A scale cannot reach zero and cannot cross it: the renderer would draw a
  // degenerate or mirrored quad, and the hit test reads a negative half extent
  // as "nothing".
  static constexpr float kMinScale{0.01f};

  static constexpr float kRotationSnap{15.0f};
  static constexpr float kScaleSnap{0.1f};

  // Two pi, for unwrapping the drag angle. Written out rather than pulled in
  // from glm/gtc/constants.hpp for one number.
  static constexpr float kTwoPi{6.28318530718f};

  // Handle colours, dim and lit; lit means hovered, or being dragged.
  static constexpr ImU32 kXDim{IM_COL32(198, 62, 62, 255)};
  static constexpr ImU32 kXLit{IM_COL32(255, 128, 128, 255)};
  static constexpr ImU32 kYDim{IM_COL32(78, 186, 78, 255)};
  static constexpr ImU32 kYLit{IM_COL32(140, 255, 140, 255)};
  static constexpr ImU32 kPlaneDim{IM_COL32(228, 228, 228, 255)};
  static constexpr ImU32 kPlaneLit{IM_COL32(255, 240, 150, 255)};
  static constexpr ImU32 kRingDim{IM_COL32(96, 170, 255, 255)};
  static constexpr ImU32 kRingLit{IM_COL32(170, 215, 255, 255)};

  // Drawn under every part of the gizmo, for the reason the selection outline
  // has one: the scene behind it is free to be any colour, including the
  // gizmo's own.
  static constexpr ImU32 kHalo{IM_COL32(0, 0, 0, 190)};

  // A transform as it was when the drag started. Every frame writes
  // "this + the whole of what the drag has done so far".
  struct Grabbed {
    uint32_t entity{INVALID_ENTITY_ID};
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};
  };

  // Where the handles sit, in screen pixels. Fails when the selection has no
  // position at all, which is also when there is no gizmo to draw.
  bool Anchor(const EditorCamera& camera, const ImVec2& image_pos, ImVec2& out) const;

  TransformComponent* ActiveTransform() const;

  Axis HitTest(glm::vec2 mouse, const ImVec2& origin) const;
  Axis HitTestAxes(glm::vec2 mouse, const ImVec2& origin) const;
  Axis HitTestRing(glm::vec2 mouse, const ImVec2& origin) const;

  void BeginDrag(glm::vec2 mouse, Axis axis);
  void ApplyDrag(const EditorCamera& camera);
  void ApplyTranslate(glm::vec2 travel, float world_per_pixel);
  void ApplyRotate(glm::vec2 mouse);
  void ApplyScale(glm::vec2 travel);

  void DrawAxes(ImDrawList* draw_list, const ImVec2& origin, Axis lit, bool boxes) const;
  void DrawRing(ImDrawList* draw_list, const ImVec2& origin, Axis lit) const;

  static void Stroke(ImDrawList* draw_list, const ImVec2& from, const ImVec2& to, ImU32 colour);
  static void Head(ImDrawList* draw_list, const ImVec2& tip, const ImVec2& left, const ImVec2& right, ImU32 colour);
  static void Box(ImDrawList* draw_list, const ImVec2& centre, float half, ImU32 colour);

  Axis m_active{Axis::kNone};
  Axis m_hovered{Axis::kNone};

  // The cursor where the drag started, in ImGui pixels, and the pivot it was
  // measured against. The offset applied is always measured from here.
  glm::vec2 m_grab_screen{0.0f};
  glm::vec2 m_grab_origin{0.0f};

  // Rotation only: the angle the drag was last seen at, and the total it has
  // turned through, unwrapped so that a drag can keep going round instead of
  // jumping when it crosses the -X axis.
  float m_last_angle{0.0f};
  float m_angle_travelled{0.0f};

  std::vector<Grabbed> m_grabbed;
};

inline bool Gizmo::Anchor(const EditorCamera& camera, const ImVec2& image_pos, ImVec2& out) const {
  // The active entity anchors the drag; with one entity selected that is the
  // only one there is.
  const std::optional<glm::vec2> position = editor_bounds::PositionOf(EditorManager::GetSelectedEntity());

  if (!position) {
    return false;
  }

  const glm::vec2 screen = camera.WorldToScreen(*position);
  out = ImVec2(image_pos.x + screen.x, image_pos.y + screen.y);
  return true;
}

inline TransformComponent* Gizmo::ActiveTransform() const {
  const uint32_t entity = EditorManager::GetSelectedEntity();

  if (entity == INVALID_ENTITY_ID || !rse::ECSManager::HasComponent<TransformComponent>(entity)) {
    return nullptr;
  }

  return &rse::ECSManager::GetComponent<TransformComponent>(entity);
}

inline Gizmo::Axis Gizmo::HitTest(glm::vec2 mouse, const ImVec2& origin) const {
  switch (EditorManager::Settings().gizmo.mode) {
    case EditorSettings::GizmoMode::kRotate:
      return HitTestRing(mouse, origin);
    case EditorSettings::GizmoMode::kScale:
    case EditorSettings::GizmoMode::kTranslate:
    default:
      return HitTestAxes(mouse, origin);
  }
}

inline Gizmo::Axis Gizmo::HitTestAxes(glm::vec2 mouse, const ImVec2& origin) const {
  const glm::vec2 at(origin.x, origin.y);

  // The centre square is tested first: it sits on top of both axes where they
  // meet, and it is the handle most likely to be aimed at.
  if (rse::PointInBox(mouse, at - glm::vec2(kCentreSize), at + glm::vec2(kCentreSize))) {
    return Axis::kBoth;
  }

  // The bands run the length of the arms, including the heads, so the whole
  // thing is grabbable and not just the shaft.
  const glm::vec2 x_end(at.x + kArmLength + kArrowLength, at.y);
  if (rse::PointInBox(mouse, glm::vec2(at.x, at.y - kGrabWidth), glm::vec2(x_end.x, at.y + kGrabWidth))) {
    return Axis::kX;
  }

  const glm::vec2 y_end(at.x, at.y - kArmLength - kArrowLength);
  if (rse::PointInBox(mouse, glm::vec2(at.x - kGrabWidth, y_end.y), glm::vec2(at.x + kGrabWidth, at.y))) {
    return Axis::kY;
  }

  return Axis::kNone;
}

inline Gizmo::Axis Gizmo::HitTestRing(glm::vec2 mouse, const ImVec2& origin) const {
  const float distance = glm::length(mouse - glm::vec2(origin.x, origin.y));

  // A band around the circle, not a filled disc: the middle of the ring is
  // where the entity is, and dragging there should be dragging the scene.
  return std::abs(distance - kRingRadius) <= kRingGrabWidth ? Axis::kRing : Axis::kNone;
}

inline void Gizmo::BeginDrag(glm::vec2 mouse, Axis axis) {
  m_active = axis;
  m_grab_screen = mouse;

  m_grabbed.clear();

  for (const uint32_t entity : EditorManager::GetSelectedEntities()) {
    if (!rse::ECSManager::HasComponent<TransformComponent>(entity)) {
      continue;
    }

    const TransformComponent& transform = rse::ECSManager::GetComponent<TransformComponent>(entity);
    m_grabbed.push_back({entity, transform.position, transform.rotation, transform.scale});
  }
}

inline void Gizmo::ApplyTranslate(glm::vec2 travel, float world_per_pixel) {
  const bool drag_x = m_active == Axis::kX || m_active == Axis::kBoth;
  const bool drag_y = m_active == Axis::kY || m_active == Axis::kBoth;

  // Screen y grows downwards and world y grows upwards, so a drag upwards is a
  // positive move; an axis that is not being dragged contributes nothing,
  // which is what makes the handle a constraint rather than a suggestion.
  const glm::vec2 projected(drag_x ? travel.x : 0.0f, drag_y ? -travel.y : 0.0f);
  const glm::vec2 offset = projected * world_per_pixel;

  const float step = EditorManager::Settings().grid.base;
  const bool snapping = ImGui::GetIO().KeyShift;

  for (const Grabbed& grabbed : m_grabbed) {
    TransformComponent& transform = rse::ECSManager::GetComponent<TransformComponent>(grabbed.entity);

    glm::vec3 moved = grabbed.position;
    moved.x += offset.x;
    moved.y += offset.y;

    if (snapping) {
      // Only the axes being dragged are snapped: the other one is where it
      // was, and rounding it would move the entity sideways.
      if (drag_x) {
        moved.x = rse::SnapToStep(moved.x, step);
      }

      if (drag_y) {
        moved.y = rse::SnapToStep(moved.y, step);
      }
    }

    transform.position = moved;
  }
}

inline void Gizmo::ApplyRotate(glm::vec2 mouse) {
  // Anticlockwise from +X about the pivot the drag started from. Screen y grows
  // downwards, so it is negated: rotation.z turns the sprite anticlockwise on
  // screen, which is the way round the user drags it.
  const auto angle_at = [this](const glm::vec2& point) {
    return std::atan2(m_grab_origin.y - point.y, point.x - m_grab_origin.x);
  };

  const float now = angle_at(mouse);

  // The *shortest* way round between this frame's angle and the last one, so a
  // drag that crosses the -X axis keeps turning instead of wrapping. The total
  // is what the entity is written from, so it cannot drift.
  m_angle_travelled += std::remainder(now - m_last_angle, kTwoPi);
  m_last_angle = now;

  const bool snapping = ImGui::GetIO().KeyShift;

  for (const Grabbed& grabbed : m_grabbed) {
    TransformComponent& transform = rse::ECSManager::GetComponent<TransformComponent>(grabbed.entity);

    float degrees = grabbed.rotation.z + glm::degrees(m_angle_travelled);

    if (snapping) {
      degrees = rse::SnapToStep(degrees, kRotationSnap);
    }

    // X and Y rotation are not modelled by this gizmo (or by EntityBounds,
    // see the plan): a ring on a 2D viewport can only sensibly drive Z.
    transform.rotation.z = degrees;
  }
}

inline void Gizmo::ApplyScale(glm::vec2 travel) {
  const bool drag_x = m_active == Axis::kX || m_active == Axis::kBoth;
  const bool drag_y = m_active == Axis::kY || m_active == Axis::kBoth;

  // One arm's length of travel is one unit of scale: dragging a handle from
  // the pivot out to the end of its axis doubles what was there. The centre
  // square reads both axes at once -- right or up grows, left or down shrinks
  // -- because a uniform scale has no single axis to be measured along.
  const float ratio =
      m_active == Axis::kBoth ? (travel.x - travel.y) / kArmLength : (drag_x ? travel.x : -travel.y) / kArmLength;

  const float factor = 1.0f + ratio;
  const bool snapping = ImGui::GetIO().KeyShift;

  for (const Grabbed& grabbed : m_grabbed) {
    TransformComponent& transform = rse::ECSManager::GetComponent<TransformComponent>(grabbed.entity);

    glm::vec3 scaled = grabbed.scale;

    if (drag_x) {
      scaled.x = grabbed.scale.x * factor;
    }

    if (drag_y) {
      scaled.y = grabbed.scale.y * factor;
    }

    if (m_active == Axis::kBoth) {
      // Uniform means all three, so the sprite gains or loses depth too
      // rather than turning into a flat card.
      scaled.z = grabbed.scale.z * factor;
    }

    if (snapping) {
      if (drag_x) {
        scaled.x = rse::SnapToStep(scaled.x, kScaleSnap);
      }
      if (drag_y) {
        scaled.y = rse::SnapToStep(scaled.y, kScaleSnap);
      }
      if (m_active == Axis::kBoth) {
        scaled.z = rse::SnapToStep(scaled.z, kScaleSnap);
      }
    }

    scaled.x = glm::max(scaled.x, kMinScale);
    scaled.y = glm::max(scaled.y, kMinScale);
    scaled.z = glm::max(scaled.z, kMinScale);

    transform.scale = scaled;
  }
}

inline void Gizmo::ApplyDrag(const EditorCamera& camera) {
  const ImGuiIO& io = ImGui::GetIO();
  const glm::vec2 mouse(io.MousePos.x, io.MousePos.y);
  const glm::vec2 travel = mouse - m_grab_screen;

  switch (EditorManager::Settings().gizmo.mode) {
    case EditorSettings::GizmoMode::kRotate:
      ApplyRotate(mouse);
      break;
    case EditorSettings::GizmoMode::kScale:
      ApplyScale(travel);
      break;
    case EditorSettings::GizmoMode::kTranslate:
    default:
      ApplyTranslate(travel, camera.GetWorldPerPixel());
      break;
  }
}

inline bool Gizmo::HandleInput(const EditorCamera& camera, const ImVec2& image_pos, bool hovered) {
  ImVec2 origin;

  if (!Anchor(camera, image_pos, origin)) {
    // Nothing to move. Any drag in flight is dropped rather than carried on:
    // the entity it was moving may be the one that just went away.
    m_active = Axis::kNone;
    m_hovered = Axis::kNone;
    m_grabbed.clear();
    return false;
  }

  const ImGuiIO& io = ImGui::GetIO();

  if (m_active != Axis::kNone) {
    if (!ImGui::IsMouseDown(ImGuiMouseButton_Left)) {
      // Released. One last apply, in case the cursor moved and let go
      // between frames, and then the drag is over.
      ApplyDrag(camera);
      m_active = Axis::kNone;
      m_grabbed.clear();
      return false;
    }

    ApplyDrag(camera);
    return true;
  }

  // Hovering is only meaningful while the cursor is over the image: the
  // handles are drawn there, and off the panel they are not on screen.
  m_hovered = hovered ? HitTest(glm::vec2(io.MousePos.x, io.MousePos.y), origin) : Axis::kNone;

  if (m_hovered != Axis::kNone && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
    const glm::vec2 mouse(io.MousePos.x, io.MousePos.y);

    // The pivot is recorded here rather than looked up per frame: a rotate
    // or scale drag does not move the entity, but a translate drag does, and
    // measuring an angle against a moving pivot would spin the thing as it
    // moved.
    m_grab_origin = glm::vec2(origin.x, origin.y);
    m_last_angle = std::atan2(m_grab_origin.y - mouse.y, mouse.x - m_grab_origin.x);
    m_angle_travelled = 0.0f;

    BeginDrag(mouse, m_hovered);
    return true;
  }

  return false;
}

inline void Gizmo::Stroke(ImDrawList* draw_list, const ImVec2& from, const ImVec2& to, ImU32 colour) {
  draw_list->AddLine(from, to, kHalo, 4.0f);
  draw_list->AddLine(from, to, colour, 2.0f);
}

inline void Gizmo::Head(ImDrawList* draw_list, const ImVec2& tip, const ImVec2& left, const ImVec2& right,
                        ImU32 colour) {
  draw_list->AddTriangleFilled(tip, left, right, colour);
  draw_list->AddTriangle(tip, left, right, kHalo, 2.0f);
}

inline void Gizmo::Box(ImDrawList* draw_list, const ImVec2& centre, float half, ImU32 colour) {
  const ImVec2 min(centre.x - half, centre.y - half);
  const ImVec2 max(centre.x + half, centre.y + half);

  draw_list->AddRectFilled(ImVec2(min.x - 2.0f, min.y - 2.0f), ImVec2(max.x + 2.0f, max.y + 2.0f), kHalo);
  draw_list->AddRectFilled(min, max, colour);
}

inline void Gizmo::DrawAxes(ImDrawList* draw_list, const ImVec2& origin, Axis lit, bool boxes) const {
  const ImU32 x_colour = lit == Axis::kX || lit == Axis::kBoth ? kXLit : kXDim;
  const ImU32 y_colour = lit == Axis::kY || lit == Axis::kBoth ? kYLit : kYDim;
  const ImU32 plane_colour = lit == Axis::kBoth ? kPlaneLit : kPlaneDim;

  const ImVec2 x_tip(origin.x + kArmLength, origin.y);
  const ImVec2 y_tip(origin.x, origin.y - kArmLength);

  Stroke(draw_list, origin, x_tip, x_colour);
  Stroke(draw_list, origin, y_tip, y_colour);

  if (boxes) {
    // Scale: a box on the end of each arm, and a smaller one in the middle
    // to tell the uniform handle from the axis handles.
    Box(draw_list, x_tip, kHandleBoxSize, x_colour);
    Box(draw_list, y_tip, kHandleBoxSize, y_colour);
  } else {
    // Translate: an arrowhead past the end of each shaft.
    const ImVec2 x_head(x_tip.x + kArrowLength, x_tip.y);
    const ImVec2 y_head(y_tip.x, y_tip.y - kArrowLength);

    Head(draw_list, x_head, ImVec2(x_tip.x, x_tip.y - kArrowHalfWidth), ImVec2(x_tip.x, x_tip.y + kArrowHalfWidth),
         x_colour);
    Head(draw_list, y_head, ImVec2(y_tip.x + kArrowHalfWidth, y_tip.y), ImVec2(y_tip.x - kArrowHalfWidth, y_tip.y),
         y_colour);
  }

  // The centre square: the plane drag for translate, the uniform one for
  // scale. Halo with the fill inset into it, which is a border without needing
  // two rectangles drawn in the right order.
  const ImVec2 square_min(origin.x - kCentreSize, origin.y - kCentreSize);
  const ImVec2 square_max(origin.x + kCentreSize, origin.y + kCentreSize);

  draw_list->AddRectFilled(square_min, square_max, kHalo);
  draw_list->AddRectFilled(ImVec2(square_min.x + 2.0f, square_min.y + 2.0f),
                           ImVec2(square_max.x - 2.0f, square_max.y - 2.0f), plane_colour);
}

inline void Gizmo::DrawRing(ImDrawList* draw_list, const ImVec2& origin, Axis lit) const {
  const ImU32 colour = lit == Axis::kRing ? kRingLit : kRingDim;

  draw_list->AddCircle(origin, kRingRadius, kHalo, 0, 5.0f);
  draw_list->AddCircle(origin, kRingRadius, colour, 0, 2.5f);

  // A spoke at the entity's own angle, so the ring says which way round the
  // thing is as well as where to grab it.
  const TransformComponent* transform = ActiveTransform();

  if (transform != nullptr) {
    const float radians = glm::radians(transform->rotation.z);
    const ImVec2 tip(origin.x + std::cos(radians) * kRingRadius, origin.y - std::sin(radians) * kRingRadius);

    Stroke(draw_list, origin, tip, colour);
  }
}

inline void Gizmo::Draw(const EditorCamera& camera, const ImVec2& image_pos) const {
  ImVec2 origin;

  if (!Anchor(camera, image_pos, origin)) {
    return;
  }

  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  // Being dragged beats being hovered: once the pointer is latched it can be
  // anywhere, and the handle being moved is the one that should be lit.
  const Axis lit = m_active != Axis::kNone ? m_active : m_hovered;

  switch (EditorManager::Settings().gizmo.mode) {
    case EditorSettings::GizmoMode::kRotate:
      DrawRing(draw_list, origin, lit);
      break;
    case EditorSettings::GizmoMode::kScale:
      DrawAxes(draw_list, origin, lit, true);
      break;
    case EditorSettings::GizmoMode::kTranslate:
    default:
      DrawAxes(draw_list, origin, lit, false);
      break;
  }
}
