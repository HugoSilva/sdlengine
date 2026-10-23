#include "texture_atlas.h"

#include <algorithm>

namespace rse {
AtlasPacker::AtlasPacker(int max_size, int padding)
    : m_max_size(std::max(1, max_size)), m_padding(std::max(0, padding)) {}

bool AtlasPacker::Add(int width, int height, AtlasRegion* region) {
  if (region == nullptr || width <= 0 || height <= 0 || width > m_max_size || height > m_max_size) return false;

  int x = m_cursor_x;
  int y = m_cursor_y;
  int row_height = m_row_height;

  if (x + width > m_max_size) {
    // Start a new shelf. The padding only applies between shelves; the first
    // shelf starts at zero.
    y += row_height + m_padding;
    x = 0;
    row_height = 0;
  }

  if (y + height > m_max_size) return false;

  region->x = x;
  region->y = y;
  region->width = width;
  region->height = height;

  m_cursor_x = x + width + m_padding;
  m_cursor_y = y;
  m_row_height = std::max(row_height, height);
  m_used_width = std::max(m_used_width, x + width);
  m_used_height = std::max(m_used_height, y + height);
  return true;
}

int AtlasPacker::GetPageWidth() const { return IsEmpty() ? 1 : NextPowerOfTwo(m_used_width); }

int AtlasPacker::GetPageHeight() const { return IsEmpty() ? 1 : NextPowerOfTwo(m_used_height); }

int AtlasPacker::NextPowerOfTwo(int value) {
  if (value <= 1) return 1;
  int result = 1;
  while (result < value) {
    if (result >= (1 << 30)) return value;
    result <<= 1;
  }
  return result;
}
}  // namespace rse
