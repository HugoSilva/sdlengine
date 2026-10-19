#include "Font.hpp"

namespace rse {
Font::Font(std::string name, std::string filename, int size) : m_name(name), m_filename(filename), m_size(size) {
  m_font = TTF_OpenFont(filename.c_str(), size);
}

Font::~Font() { TTF_CloseFont(m_font); }
}  // namespace rse