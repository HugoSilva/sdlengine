#pragma once

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
}  // namespace rse
