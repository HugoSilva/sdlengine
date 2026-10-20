#pragma once

#include <string>

#include <cereal/access.hpp>
#include <cereal/cereal.hpp>

namespace rse {
class Font;  // forward declaration for Font

class Texture {
 public:
  Texture(const std::string& name, const std::string& filename);
  Texture(const std::string& name, Font* font, const std::string text, unsigned int color);
  ~Texture();

  void Bind() const;
  void Unbind() const;

  inline const unsigned int GetId() const { return m_id; }
  inline const std::string& GetName() const { return m_name; }
  inline const unsigned int GetWidth() const { return m_width; }
  inline const unsigned int GetHeight() const { return m_height; }

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Id", m_id), cereal::make_nvp("Name", m_name), cereal::make_nvp("Filename", m_filename),
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
  // Zero until a GL context uploads it. A texture loaded without one (a unit
  // test, a tool) keeps its name and size and stays at zero, which is what
  // draws white.
  unsigned int m_id{0};
  std::string m_name;
  std::string m_filename;
  int m_width = 0, m_height = 0;
};
}  // namespace rse