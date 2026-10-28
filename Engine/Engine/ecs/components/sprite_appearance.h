#pragma once

#include <string>

#include <cereal/cereal.hpp>
#include <cereal/types/string.hpp>

// The parts of a sprite's look that are not the sprite itself: whether the image
// is mirrored on each axis, and which material shader draws it.
//
// This is a separate component rather than fields inside SpriteComponent because
// Sprite is a polymorphic payload with its own serialiser, and adding fields to
// it would change what every existing scene's <Sprite> node means. A group
// appended to the scene/prefab format can be version-gated instead, and an old
// entity simply gets the defaults when the group is absent.
struct SpriteAppearanceComponent {
  SpriteAppearanceComponent() = default;

  bool flip_x{false};
  bool flip_y{false};

  // An asset id for a registered material, or a project-relative path for a
  // file the registry does not know. Empty means "use the scene shader".
  std::string material;

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("FlipX", flip_x), cereal::make_nvp("FlipY", flip_y),
            cereal::make_nvp("Material", material));
  }
};
