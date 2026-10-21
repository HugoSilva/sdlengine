#pragma once

// ---------------------------------------------------------------------------
// Editor settings.
//
// The state behind View > Show Grid, kept in one struct so that writing the
// whole thing to editor.cfg needs nothing re-plumbed. Only what something
// actually reads lives here: a settings field nobody uses is a field that
// quietly rots.
//
// The file is editor.cfg in the working directory -- a file of its own rather
// than the engine's default.cfg, which means something else entirely. Camera
// state is deliberately not in here: where you are looking is a view of the
// scene rather than a setting, and F recovers it.
// ---------------------------------------------------------------------------

#include <charconv>
#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <system_error>

#include <utils/file.h>
#include <utils/logger.h>

struct EditorSettings {
  struct Grid {
    bool visible{true};

    // World units across one grid square, and the quantum snapping rounds
    // to. Both uses are the same number on purpose: a grid you can see has
    // to be the grid you snap to.
    float base{32.0f};
  };

  // Which gizmo the viewport draws. The viewport HUD reports it and the W/E/R
  // keys set it.
  enum class GizmoMode { kTranslate, kRotate, kScale };

  struct Gizmo {
    GizmoMode mode{GizmoMode::kTranslate};
  };

  Grid grid;
  Gizmo gizmo;

  // The project and scene the editor had open last. The project is what the
  // next run reopens when no path is given; the scene is recorded for the same
  // reason, and is informational until the Scenes menu learns to switch.
  std::string last_project;
  std::string last_scene;

  // Panel visibility, one entry per `EditorPanel::Id()`, written as
  // `panel.<id>=true|false`. A map, because the set of panels is whatever the
  // editor added: an id the file has and the editor no longer does is simply
  // never read, and a panel the file does not mention keeps its default.
  std::map<std::string, bool> panels;

  // The settings file, in the working directory next to imgui.ini.
  static constexpr const char* kFilePath = "editor.cfg";

  // Reads the file over the built-in defaults. A key that is missing -- a file
  // from an older build, or one somebody has trimmed -- leaves its default in
  // place, and so does a value that cannot be parsed: a typo should not
  // silently turn a setting into something arbitrary. The path is a parameter
  // so the tests can keep their own file out of the working directory.
  void Load(const std::string& file = kFilePath);

  // Writes the whole struct. There is no "dirty" flag anywhere because there
  // are two places a setting can change and they both call this; nothing else
  // has to remember, and nothing rewrites the file every frame.
  void Save(const std::string& file = kFilePath) const;
};

// The project the editor opens on a fresh run. An explicit command-line path
// wins even when it is wrong: the error should name what was typed rather than
// quietly opening some other project. Otherwise the last project this machine
// opened, when it is still there; a path that has gone is a warning, never a
// failure, so a folder somebody moved cannot stop the editor from starting.
// With neither, the manifest next to the binary.
inline std::filesystem::path StartupProject(const EditorSettings& settings, const std::string& argument) {
  if (!argument.empty()) {
    return argument;
  }

  if (!settings.last_project.empty()) {
    if (std::filesystem::exists(settings.last_project)) {
      return settings.last_project;
    }

    Logger::Warning("The last project is gone (" + settings.last_project + "); falling back to project.xml.");
  }

  return "project.xml";
}

inline const char* GizmoModeName(EditorSettings::GizmoMode mode) {
  switch (mode) {
    case EditorSettings::GizmoMode::kRotate:
      return "rotate";
    case EditorSettings::GizmoMode::kScale:
      return "scale";
    case EditorSettings::GizmoMode::kTranslate:
      break;
  }

  return "translate";
}

inline void EditorSettings::Load(const std::string& file) {
  // exists() first: File::read logs an error for a file that is not there, and
  // the very first run has no settings file yet. That is not an error.
  if (!File::Exists(file.c_str())) {
    return;
  }

  // Files get hand-edited, and hand-edited files get spaces around the '=' and
  // CRLF line endings. Both are the editor's problem, not the writer's.
  const auto trimmed = [](std::string text) {
    const std::size_t first = text.find_first_not_of(" \t\r\n");

    if (first == std::string::npos) {
      return std::string{};
    }

    return text.substr(first, text.find_last_not_of(" \t\r\n") - first + 1);
  };

  std::istringstream lines(File::Read(file.c_str()));
  std::string line;

  while (std::getline(lines, line)) {
    // A comment, or a line somebody left half-finished. Both are ignored
    // rather than guessed at.
    if (line.empty() || line.front() == '#') {
      continue;
    }

    const std::size_t separator = line.find('=');

    if (separator == std::string::npos) {
      continue;
    }

    const std::string key = trimmed(line.substr(0, separator));
    const std::string value = trimmed(line.substr(separator + 1));

    if (key == "grid.visible") {
      if (value == "true") {
        grid.visible = true;
      } else if (value == "false") {
        grid.visible = false;
      }
    } else if (key == "grid.base") {
      float parsed = 0.0f;
      const char* first = value.data();
      const char* last = first + value.size();
      const std::from_chars_result result = std::from_chars(first, last, parsed);

      // from_chars rather than stof: it does not throw, and it has no
      // locale to disagree with the decimal point the file was written
      // with. Trailing junk and a base of zero or less are not a grid, so
      // they are ignored like everything else unparsable.
      if (result.ec == std::errc() && result.ptr == last && parsed > 0.0f) {
        grid.base = parsed;
      }
    } else if (key == "gizmo.mode") {
      // The names come from the one place they are written, so a mode
      // cannot be spelled one way in the file and another in the HUD.
      for (const GizmoMode candidate : {GizmoMode::kTranslate, GizmoMode::kRotate, GizmoMode::kScale}) {
        if (value == GizmoModeName(candidate)) {
          gizmo.mode = candidate;
        }
      }
    } else if (key == "last.project") {
      last_project = value;
    } else if (key == "last.scene") {
      last_scene = value;
    } else if (key.rfind("panel.", 0) == 0) {
      // Only a real boolean is a visibility; anything else leaves the panel at
      // its default rather than hiding it on a typo.
      if (value == "true") {
        panels[key.substr(6)] = true;
      } else if (value == "false") {
        panels[key.substr(6)] = false;
      }
    }
  }
}

inline void EditorSettings::Save(const std::string& file) const {
  std::ostringstream out;

  out << "grid.visible=" << (grid.visible ? "true" : "false") << "\n";

  // The default float formatting, because it round-trips what a person would
  // write: 32 saves as "32" and reloads as 32.
  out << "grid.base=" << grid.base << "\n";
  out << "gizmo.mode=" << GizmoModeName(gizmo.mode) << "\n";
  out << "last.project=" << last_project << "\n";
  out << "last.scene=" << last_scene << "\n";

  // The map is ordered, so the file is stable across runs and a diff shows the
  // panel that actually changed.
  for (const std::pair<const std::string, bool>& panel : panels) {
    out << "panel." << panel.first << "=" << (panel.second ? "true" : "false") << "\n";
  }

  File::Write(file.c_str(), out.str().c_str());
}
