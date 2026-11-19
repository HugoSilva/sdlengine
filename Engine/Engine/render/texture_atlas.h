#pragma once

#include <cstddef>
#include <vector>

namespace rse {
struct AtlasRegion {
  int x{0};
  int y{0};
  int width{0};
  int height{0};
};

// A small shelf packer. It is deliberately CPU-only and knows nothing about
// OpenGL: the texture manager asks it for positions, copies pixels, and creates
// one page. Keeping it separate is what makes the packing rules testable
// without a window or a GL context.
class AtlasPacker {
 public:
  explicit AtlasPacker(int max_size, int padding = 1);

  // Packs one rectangle. Returns false when it no longer fits; the packer is
  // then unchanged, so a caller can fall back to an independent texture.
  bool Add(int width, int height, AtlasRegion* region);

  int GetPageWidth() const;
  int GetPageHeight() const;
  int GetUsedWidth() const { return m_used_width; }
  int GetUsedHeight() const { return m_used_height; }
  bool IsEmpty() const { return m_used_width == 0 && m_used_height == 0; }

  static int NextPowerOfTwo(int value);

 private:
  int m_max_size;
  int m_padding;
  int m_cursor_x{0};
  int m_cursor_y{0};
  int m_row_height{0};
  int m_used_width{0};
  int m_used_height{0};
};

// Where one image ended up. `packed` is false when the image cannot share a
// page at all -- it failed to load, or it is larger than a page -- and the
// caller has to upload it on its own.
struct AtlasPlacement {
  std::size_t page{0};
  AtlasRegion region;
  bool packed{false};
};

// Packs a whole project's images into as many pages as they need. The shelf
// packer above fills one page; this is the layer that opens the next one
// instead of giving up, which is what keeps a large asset set batching.
class AtlasBuilder {
 public:
  explicit AtlasBuilder(int max_size, int padding = 1);

  // Packs one image on the first page with room, or on a new page when every
  // page is full. A width or height of zero or less (an image that failed to
  // load) and anything larger than a page are left unpacked for the caller to
  // draw from a texture of its own. The builder is unchanged in that case.
  AtlasPlacement Add(int width, int height);

  std::size_t GetPageCount() const { return m_pages.size(); }
  int GetPageWidth(std::size_t page) const;
  int GetPageHeight(std::size_t page) const;

 private:
  int m_max_size;
  int m_padding;
  std::vector<AtlasPacker> m_pages;
};
}  // namespace rse
