#pragma once

// ---------------------------------------------------------------------------
// Base class for every dockable editor panel.
//
// id() doubles as the ImGui window name and is therefore the docking key: it
// has to stay stable and unique for the whole run. draw() is called once per
// frame while the panel is visible; each panel owns its Begin/End pair.
// ---------------------------------------------------------------------------

// Where a panel is docked the first time the editor runs, or after
// View > Reset Layout. Once the user rearranges anything, imgui.ini takes over
// and this is ignored.
enum class DockSlot {
  Left,
  Right,
  Bottom,
  Center,
};

class EditorPanel {
 public:
  virtual ~EditorPanel() = default;

  virtual const char* Id() const = 0;
  virtual void Draw() = 0;

  virtual DockSlot Slot() const { return DockSlot::Center; }

  // Toggled from the View menu and by the panel's own close button.
  bool visible{true};
};
