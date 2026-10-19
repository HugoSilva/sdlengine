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

  inline const unsigned int GetId() const { return m_Id; }
  inline const std::string& GetName() const { return m_Name; }
  inline const unsigned int GetWidth() const { return m_Width; }
  inline const unsigned int GetHeight() const { return m_Height; }

  template <typename Archive>
  void serialize(Archive& archive) {
    archive(cereal::make_nvp("Id", m_Id), cereal::make_nvp("Name", m_Name), cereal::make_nvp("Filename", m_Filename),
            cereal::make_nvp("Width", m_Width), cereal::make_nvp("Height", m_Height));
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
  unsigned int m_Id;
  std::string m_Name;
  std::string m_Filename;
  int m_Width = 0, m_Height = 0;
};
}  // namespace rse