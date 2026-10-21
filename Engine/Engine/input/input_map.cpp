#include "input_map.h"

#include <cctype>
#include <set>

#include <input/input_manager.h>
#include <utils/logger.h>

namespace rse {
namespace {
std::string ToLower(const std::string& text) {
  std::string lower{text};

  for (char& character : lower) {
    character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
  }

  return lower;
}

// The names a person is likely to type that are not SDL's own spelling. SDL
// calls the Enter key "Return", for instance, and everyone writes "Enter".
SDL_Scancode ScancodeFor(const std::string& name) {
  static const std::map<std::string, SDL_Scancode> aliases = {
      {"enter", SDL_SCANCODE_RETURN},
      {"return", SDL_SCANCODE_RETURN},
      {"escape", SDL_SCANCODE_ESCAPE},
      {"esc", SDL_SCANCODE_ESCAPE},
      {"space", SDL_SCANCODE_SPACE},
      {"tab", SDL_SCANCODE_TAB},
      {"backspace", SDL_SCANCODE_BACKSPACE},
      {"delete", SDL_SCANCODE_DELETE},
      {"del", SDL_SCANCODE_DELETE},
      {"up", SDL_SCANCODE_UP},
      {"down", SDL_SCANCODE_DOWN},
      {"left", SDL_SCANCODE_LEFT},
      {"right", SDL_SCANCODE_RIGHT},
      {"home", SDL_SCANCODE_HOME},
      {"end", SDL_SCANCODE_END},
      {"pageup", SDL_SCANCODE_PAGEUP},
      {"pagedown", SDL_SCANCODE_PAGEDOWN},
  };

  const std::string lower = ToLower(name);

  if (const auto found = aliases.find(lower); found != aliases.end()) {
    return found->second;
  }

  // A single letter or digit: SDL's names are upper case, and a manifest may
  // just as well have written "s".
  if (name.size() == 1 && std::isalnum(static_cast<unsigned char>(name.front())) != 0) {
    const std::string upper(1, static_cast<char>(std::toupper(static_cast<unsigned char>(name.front()))));
    return SDL_GetScancodeFromName(upper.c_str());
  }

  // Everything else is SDL's name ("F2", "Left", "Return") as typed, then with
  // its first letter capitalised, which is how the key names are spelled.
  if (SDL_Scancode scancode = SDL_GetScancodeFromName(name.c_str()); scancode != SDL_SCANCODE_UNKNOWN) {
    return scancode;
  }

  std::string capitalised{name};

  if (!capitalised.empty()) {
    capitalised.front() = static_cast<char>(std::toupper(static_cast<unsigned char>(capitalised.front())));
  }

  return SDL_GetScancodeFromName(capitalised.c_str());
}

std::vector<std::string> Split(const std::string& text, char separator) {
  std::vector<std::string> parts;
  std::string part;

  for (const char character : text) {
    if (character == separator) {
      parts.push_back(part);
      part.clear();
    } else {
      part.push_back(character);
    }
  }

  parts.push_back(part);
  return parts;
}

std::string Trim(const std::string& text) {
  const std::size_t first = text.find_first_not_of(" \t");

  if (first == std::string::npos) {
    return "";
  }

  const std::size_t last = text.find_last_not_of(" \t");
  return text.substr(first, last - first + 1);
}

// What the project overrode, on top of Defaults(). Empty means the defaults.
std::map<std::string, Binding> s_overrides;
}  // namespace

bool Binding::Matches(SDL_Scancode key, bool ctrl, bool shift, bool alt) const {
  return scancode == key && this->ctrl == ctrl && this->shift == shift && this->alt == alt;
}

Binding InputMap::Parse(const std::string& text) {
  Binding binding;
  bool has_key = false;

  for (const std::string& raw : Split(text, '+')) {
    const std::string token = Trim(raw);

    if (token.empty()) {
      // "Ctrl+" and a bare "" are both nothing to bind.
      return Binding{};
    }

    const std::string lower = ToLower(token);

    if (lower == "ctrl" || lower == "control") {
      binding.ctrl = true;
    } else if (lower == "shift") {
      binding.shift = true;
    } else if (lower == "alt" || lower == "option") {
      binding.alt = true;
    } else if (!has_key) {
      binding.scancode = ScancodeFor(token);
      has_key = true;

      if (!binding.IsValid()) {
        return Binding{};
      }
    } else {
      // Two keys in one binding: not something a single action can be.
      return Binding{};
    }
  }

  return has_key ? binding : Binding{};
}

std::string InputMap::Format(const Binding& binding) {
  if (!binding.IsValid()) {
    return "";
  }

  std::string text;

  if (binding.ctrl) {
    text += "Ctrl+";
  }

  if (binding.shift) {
    text += "Shift+";
  }

  if (binding.alt) {
    text += "Alt+";
  }

  text += SDL_GetScancodeName(binding.scancode);
  return text;
}

const std::map<std::string, Binding>& InputMap::Defaults() {
  static const std::map<std::string, Binding> defaults = {
      {"editor.gizmo.rotate", {SDL_SCANCODE_E, false, false, false}},
      {"editor.gizmo.scale", {SDL_SCANCODE_R, false, false, false}},
      {"editor.gizmo.translate", {SDL_SCANCODE_W, false, false, false}},
      {"editor.reload", {SDL_SCANCODE_F2, false, false, false}},
      {"editor.save", {SDL_SCANCODE_S, true, false, false}},
      {"editor.scene.next", {SDL_SCANCODE_TAB, true, false, false}},
      {"editor.scene.previous", {SDL_SCANCODE_TAB, true, true, false}},
      {"game.down", {SDL_SCANCODE_DOWN, false, false, false}},
      {"game.left", {SDL_SCANCODE_LEFT, false, false, false}},
      {"game.right", {SDL_SCANCODE_RIGHT, false, false, false}},
      {"game.up", {SDL_SCANCODE_UP, false, false, false}},
  };

  return defaults;
}

bool InputMap::IsKnown(const std::string& action) { return Defaults().find(action) != Defaults().end(); }

void InputMap::SetOverrides(const std::vector<std::pair<std::string, std::string>>& overrides) {
  Reset();

  // One warning per bad entry, not one per frame and not one per duplicate.
  std::set<std::string> warned;

  for (const std::pair<std::string, std::string>& override : overrides) {
    if (!IsKnown(override.first)) {
      if (warned.insert(override.first).second) {
        Logger::Warning("Unknown input action \"" + override.first + "\" in the project; ignored.");
      }

      continue;
    }

    const Binding binding = Parse(override.second);

    if (!binding.IsValid()) {
      if (warned.insert(override.first).second) {
        Logger::Warning("Input action \"" + override.first + "\" has an unreadable key \"" + override.second +
                        "\"; keeping the default.");
      }

      continue;
    }

    s_overrides[override.first] = binding;
  }
}

void InputMap::Reset() { s_overrides.clear(); }

Binding InputMap::Get(const std::string& action) {
  if (const auto found = s_overrides.find(action); found != s_overrides.end()) {
    return found->second;
  }

  if (const auto found = Defaults().find(action); found != Defaults().end()) {
    return found->second;
  }

  return Binding{};
}

bool InputMap::IsPressed(const Binding& binding) {
  if (!binding.IsValid()) {
    return false;
  }

  const bool ctrl = InputManager::IsKeyPressed(SDL_SCANCODE_LCTRL) || InputManager::IsKeyPressed(SDL_SCANCODE_RCTRL);
  const bool shift = InputManager::IsKeyPressed(SDL_SCANCODE_LSHIFT) || InputManager::IsKeyPressed(SDL_SCANCODE_RSHIFT);
  const bool alt = InputManager::IsKeyPressed(SDL_SCANCODE_LALT) || InputManager::IsKeyPressed(SDL_SCANCODE_RALT);

  return InputManager::IsKeyPressed(binding.scancode) && binding.Matches(binding.scancode, ctrl, shift, alt);
}

bool InputMap::IsActionPressed(const std::string& action) { return IsPressed(Get(action)); }
}  // namespace rse
