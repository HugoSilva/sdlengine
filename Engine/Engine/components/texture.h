#pragma once

#include <memory>
#include <string>

#include <cereal/access.hpp>
#include <cereal/cereal.hpp>
#include <glm/glm.hpp>

namespace rse {
class Font;  // forward declaration for Font

// One uploaded GL texture. Several Texture objects can share a page after
// atlas packing; the shared_ptr makes the page's lifetime the union of all the
// sprites that use it, and the page (not the individual Texture) owns the GL
// name.
struct TexturePage {
  unsigned int id{0};
  ~TexturePage();
};

class Texture {
 public:
  Texture(const std::string& name, const std::string& filename);

  // A texture that knows its asset id: the id a scene stores, as opposed to the
  // name the inspector shows. The two-argument constructor is the same thing
  // with no id, which is what a texture loaded from a bare path has.
  Texture(const std::string& asset_id, const std::string& name, const std::string& filename);
  Texture(const std::string& name, Font* font, const std::string text, unsigned int color);

  // A region of an existing page, used by the runtime atlas. `uv` is
  // (u0, v0, u1, v1) in the page's normalized coordinates.
  Texture(const std::string& asset_id, const std::string& name, std::shared_ptr<TexturePage> page, const glm::vec4& uv,
          int width, int height);
  ~Texture();

  void Bind() const;
  void Unbind() const;

  inline const unsigned int GetId() const { return m_page == nullptr ? 0 : m_page->id; }
  inline const glm::vec4& GetUvRect() const { return m_uv; }
  inline const std::string& GetAssetId() const { return m_asset_id; }
  inline const std::string& GetName() const { return m_name; }
  inline const unsigned int GetWidth() const { return m_width; }
  inline const unsigned int GetHeight() const { return m_height; }

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Id", GetId()), cereal::make_nvp("Name", m_name), cereal::make_nvp("Filename", m_filename),
            cereal::make_nvp("Width", m_width), cereal::make_nvp("Height", m_height));
  };

  template <typename Archive>
  static void load_and_construct(Archive& archive, cereal::construct<Texture>& construct) {
    unsigned int id;
    std::string name;
    std::string filename;
    int width, height;

    archive(id, name, filename, width, height);
    construct(name, filename);
  };

 private:
  // Null until a GL context uploads it. A texture loaded without one (a unit
  // test, a tool) keeps its name and size and stays at zero, which is what
  // draws white.
  std::shared_ptr<TexturePage> m_page;
  glm::vec4 m_uv{0.0f, 0.0f, 1.0f, 1.0f};
  // Empty when the texture came from a bare path rather than a registry entry.
  std::string m_asset_id;
  std::string m_name;
  std::string m_filename;
  int m_width = 0, m_height = 0;
};
}  // namespace rse
