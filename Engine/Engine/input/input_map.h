#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <SDL3/SDL.h>

namespace rse {
// One key combination: the key plus the modifiers that must be held with it.
//
// The modifiers are part of the match rather than a "at least" filter, because
// "S" and "Ctrl+S" are two different actions: a project that binds save to
// Ctrl+S does not want every tap of S to save, and one that binds translate to
// W does not want Ctrl+W to move the gizmo.
struct Binding {
  SDL_Scancode scancode{SDL_SCANCODE_UNKNOWN};
  bool ctrl{false};
  bool shift{false};
  bool alt{false};

  bool Matches(SDL_Scancode key, bool ctrl, bool shift, bool alt) const;

  bool IsValid() const { return scancode != SDL_SCANCODE_UNKNOWN; }
};

// The project's action -> key table, resolved over the compiled defaults.
//
// This is pure data plus one adapter. The map knows how to read "Ctrl+S", how
// to match a binding against a key and its modifiers, and which actions exist;
// InputManager knows what is held right now. Neither grows a list of actions,
// so a project cannot misspell its way into a build error and the engine never
// needs to know that an editor has a save command.
class InputMap {
 public:
  // "Ctrl+Shift+S" -> binding. Modifier names and key names are matched
  // case-insensitively; an empty, unreadable or multi-key text yields an
  // invalid binding, which is never pressed.
  static Binding Parse(const std::string& text);

  // The inverse, in the spelling Parse reads: "Ctrl+Shift+S". An invalid
  // binding formats as an empty string.
  static std::string Format(const Binding& binding);

  // The actions this build knows, with their default keys. Adding an action is
  // one entry here; everything else reads the table.
  static const std::map<std::string, Binding>& Defaults();

  static bool IsKnown(const std::string& action);

  // Applies a project's overrides on top of the defaults. An action that is
  // not in Defaults() is logged once and ignored, and so is a key that cannot
  // be read: a typo in a manifest must not take the whole input map down or
  // silently leave an action unbindable.
  static void SetOverrides(const std::vector<std::pair<std::string, std::string>>& overrides);

  // Back to the compiled defaults. A run without a project, and the end of an
  // application, need nothing from a manifest.
  static void Reset();

  // Where an action resolves to: the project's binding, or the default. An
  // action this build does not know is an invalid binding.
  static Binding Get(const std::string& action);

  // True while `binding` is held. This is the one place InputManager is read,
  // so every binding -- the table's and a hard-coded fallback -- behaves the
  // same and is off while the UI has locked input.
  static bool IsPressed(const Binding& binding);

  // True while the action's binding is held. An unknown action is never
  // pressed rather than an error: the caller may be an older build's scene and
  // the next frame is more useful than a crash.
  static bool IsActionPressed(const std::string& action);
};
}  // namespace rse
