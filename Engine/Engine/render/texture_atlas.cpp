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

AtlasBuilder::AtlasBuilder(int max_size, int padding)
    : m_max_size(std::max(1, max_size)), m_padding(std::max(0, padding)) {}

AtlasPlacement AtlasBuilder::Add(int width, int height) {
  AtlasPlacement placement;

  // An image that failed to load has no size, and one larger than a page can
  // never share one. Neither is an error for the project: the caller draws that
  // image from an individual texture and the rest keep batching.
  if (width <= 0 || height <= 0 || width > m_max_size || height > m_max_size) {
    return placement;
  }

  for (std::size_t page = 0; page < m_pages.size(); ++page) {
    if (m_pages[page].Add(width, height, &placement.region)) {
      placement.page = page;
      placement.packed = true;
      return placement;
    }
  }

  // No page had room. An image that fits a page always fits an empty one, so a
  // failure here would be a bug in the precondition above; the empty page is
  // dropped rather than left behind as a blank texture.
  m_pages.emplace_back(m_max_size, m_padding);

  if (!m_pages.back().Add(width, height, &placement.region)) {
    m_pages.pop_back();
    return placement;
  }

  placement.page = m_pages.size() - 1;
  placement.packed = true;
  return placement;
}

int AtlasBuilder::GetPageWidth(std::size_t page) const {
  return page < m_pages.size() ? m_pages[page].GetPageWidth() : 1;
}

int AtlasBuilder::GetPageHeight(std::size_t page) const {
  return page < m_pages.size() ? m_pages[page].GetPageHeight() : 1;
}
}  // namespace rse
