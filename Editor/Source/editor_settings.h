#pragma once

// ---------------------------------------------------------------------------
// Editor settings.
//
// The editor's own state -- the grid and the gizmo, which panels are visible,
// and the session it should reopen -- kept in one struct so that writing the
// whole thing to editor.cfg needs nothing re-plumbed. Only what something
// actually reads lives here: a settings field nobody uses is a field that
// quietly rots.
//
// The file is editor.cfg in the working directory -- a file of its own rather
// than the engine's default.cfg, which means something else entirely. Camera
// state is deliberately not in here: where you are looking is a view of the
// scene rather than a setting, and F recovers it.
// ---------------------------------------------------------------------------

#include <algorithm>
#include <charconv>
#include <filesystem>
#include <map>
#include <sstream>
#include <string>
#include <system_error>
#include <vector>

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

  // The projects this machine opened, most recent first. Five is a menu that
  // fits and a file that stays short; the list is per machine, like the rest of
  // this file, so nothing about it is written into a project.
  std::vector<std::string> recent_projects;

  // How many the list keeps: one `recent.<n>` line per entry, and `recent.0` is
  // the project the next run opens.
  static constexpr std::size_t kMaxRecentProjects{5};

  // The scene the editor had open last, recorded so the next run can go back to
  // it once the Scenes menu learns to switch. Informational otherwise.
  std::string last_scene;

  // Panel visibility, one entry per `EditorPanel::Id()`, written as
  // `panel.<id>=true|false`. A map, because the set of panels is whatever the
  // editor added: an id the file has and the editor no longer does is simply
  // never read, and a panel the file does not mention keeps its default.
  std::map<std::string, bool> panels;

  // The settings file, in the working directory next to imgui.ini.
  static constexpr const char* kFilePath = "editor.cfg";

  // Puts `project_file` at the front of the recent list, dropping the copy that
  // was already there and trimming the tail to kMaxRecentProjects.
  //
  // The path is stored absolute: the working directory is not the project's,
  // and a bare "project.xml" means nothing from inside another folder. Doing
  // this twice in a row is one entry, so a caller can record every frame and
  // persist only when the list actually changes.
  void RememberRecentProject(const std::string& project_file);

  // Drops one entry. The menu uses it for a project that has gone between the
  // read and the click; saving is the caller's choice, because the same list is
  // written when the scene changes.
  void ForgetRecentProject(std::size_t index);

  // Reads the file over the built-in defaults. A key that is missing -- a file
  // from an older build, or one somebody has trimmed -- leaves its default in
  // place, and so does a value that cannot be parsed: a typo should not
  // silently turn a setting into something arbitrary. The path is a parameter
  // so the tests can keep their own file out of the working directory.
  void Load(const std::string& file = kFilePath);

  // Writes the whole struct. There is no "dirty" flag anywhere: every caller
  // writes after the change it made, so nothing has to remember, and nothing
  // rewrites the file every frame.
  void Save(const std::string& file = kFilePath) const;
};

// The project the editor opens on a fresh run. An explicit command-line path
// wins even when it is wrong: the error should name what was typed rather than
// quietly opening some other project. Otherwise the most recent project, which
// Load has already pruned to paths that still exist; a path that went missing
// after the read is a warning, never a failure, so a folder somebody moved
// cannot stop the editor from starting. With no list -- a first run, or a file
// written before there was one -- the manifest next to the binary.
inline std::filesystem::path StartupProject(const EditorSettings& settings, const std::string& argument) {
  if (!argument.empty()) {
    return argument;
  }

  if (!settings.recent_projects.empty()) {
    const std::string& project = settings.recent_projects.front();

    if (std::filesystem::exists(project)) {
      return project;
    }

    Logger::Warning("The most recent project is gone (" + project + "); falling back to project.xml.");
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

  // Collected while the lines are read, then applied in index order once the
  // whole file is in: a hand-edited file may list recent.2 before recent.0, and
  // "most recent first" is the number, not the line.
  std::map<int, std::string> recent_by_index;
  std::string legacy_last_project;

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
    } else if (key.rfind("recent.", 0) == 0) {
      // Numbered, so the file carries the order. A path that no longer exists
      // is dropped here rather than greeting the next run with an error; the
      // same goes for a number that is not one.
      int index = 0;
      const char* first = key.data() + 7;
      const char* last = key.data() + key.size();
      const std::from_chars_result result = std::from_chars(first, last, index);

      if (result.ec == std::errc() && result.ptr == last && index >= 0 && !value.empty() &&
          std::filesystem::exists(value)) {
        recent_by_index[index] = value;
      }
    } else if (key == "last.project") {
      // What version #46a wrote instead of a list. Read, used once, and never
      // written again -- see the end of this function.
      legacy_last_project = value;
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

  // Oldest first, so each one lands at the front of the list: recent.0 ends up
  // the most recent. RememberRecentProject also dedupes, so a file that names
  // the same project twice is one entry.
  for (auto entry = recent_by_index.rbegin(); entry != recent_by_index.rend(); ++entry) {
    RememberRecentProject(entry->second);
  }

  // Before there was a list there was one project. It becomes the newest entry
  // once, so an upgrade keeps reopening what it reopened before; the next Save
  // writes a recent list and drops the key.
  if (recent_projects.empty() && !legacy_last_project.empty() && std::filesystem::exists(legacy_last_project)) {
    RememberRecentProject(legacy_last_project);
  }
}

inline void EditorSettings::RememberRecentProject(const std::string& project_file) {
  if (project_file.empty()) {
    return;
  }

  std::error_code error;
  const std::filesystem::path absolute = std::filesystem::absolute(project_file, error);
  const std::string entry = (error ? std::filesystem::path{project_file} : absolute).string();

  recent_projects.erase(std::remove(recent_projects.begin(), recent_projects.end(), entry), recent_projects.end());
  recent_projects.insert(recent_projects.begin(), entry);

  if (recent_projects.size() > kMaxRecentProjects) {
    recent_projects.resize(kMaxRecentProjects);
  }
}

inline void EditorSettings::ForgetRecentProject(std::size_t index) {
  if (index >= recent_projects.size()) {
    return;
  }

  recent_projects.erase(recent_projects.begin() + static_cast<std::ptrdiff_t>(index));
}

inline void EditorSettings::Save(const std::string& file) const {
  std::ostringstream out;

  out << "grid.visible=" << (grid.visible ? "true" : "false") << "\n";

  // The default float formatting, because it round-trips what a person would
  // write: 32 saves as "32" and reloads as 32.
  out << "grid.base=" << grid.base << "\n";
  out << "gizmo.mode=" << GizmoModeName(gizmo.mode) << "\n";

  // Most recent first, numbered so the reader does not depend on line order.
  // There is no last.project any more: the first entry is the project the next
  // run opens.
  for (std::size_t index = 0; index < recent_projects.size(); ++index) {
    out << "recent." << index << "=" << recent_projects.at(index) << "\n";
  }

  out << "last.scene=" << last_scene << "\n";

  // The map is ordered, so the file is stable across runs and a diff shows the
  // panel that actually changed.
  for (const std::pair<const std::string, bool>& panel : panels) {
    out << "panel." << panel.first << "=" << (panel.second ? "true" : "false") << "\n";
  }

  File::Write(file.c_str(), out.str().c_str());
}
