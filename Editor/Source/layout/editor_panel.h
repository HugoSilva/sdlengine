#pragma once

#include <filesystem>

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
  kLeft,
  kRight,
  kBottom,
  kCenter,
};

class EditorPanel {
 public:
  virtual ~EditorPanel() = default;

  virtual const char* Id() const = 0;
  virtual void Draw() = 0;

  virtual DockSlot Slot() const { return DockSlot::kCenter; }

  // A different project is open. A panel that caches project paths (the content
  // browser) drops them and starts again from the new directory; most panels
  // read from the engine every frame and do not care.
  virtual void ProjectChanged() {}

  // Show `path` in this panel. Only the Content Browser implements it today:
  // the inspector uses it to turn an asset name click into a navigation. A panel
  // that has no notion of a file ignores it.
  virtual void RevealAsset(const std::filesystem::path& path) { (void)path; }

  // The panel went from hidden to shown. A panel that snapshots the state it
  // edits -- Project Settings' Cancel -- takes the snapshot here, so reopening
  // it starts from what is on screen now rather than from the first opening.
  virtual void Shown() {}

  // Toggled from the View menu and by the panel's own close button.
  bool visible{true};
};
