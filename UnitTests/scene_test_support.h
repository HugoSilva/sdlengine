#pragma once

#include <core/scene_manager.h>

// The scene the scene-level tests work on.
//
// SceneManager owns the world, so a test that used to reach the active scene
// through ECSManager asks SceneManager for it instead. Naming the lookup once
// keeps the long `... .Registry().get<T>(entity)` expressions readable.
inline rse::Scene& TestScene() { return SceneManager::ActiveScene(); }
