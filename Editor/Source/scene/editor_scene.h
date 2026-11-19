#pragma once

#include <core/scene_manager.h>

// The scene the editor is editing.
//
// The editor only ever edits the active scene, and during a play session that
// is SceneManager's copy of the open scene. Naming the lookup once keeps the
// panels from each spelling out where the active scene comes from, and keeps
// that answer in one place if it ever changes.
inline rse::Scene& EditorScene() { return SceneManager::ActiveScene(); }
