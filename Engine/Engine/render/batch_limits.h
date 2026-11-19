#pragma once

#include <cstddef>

namespace rse {
// The 2D batch limits shared by the builder and every backend. They live in one
// header so a console port can state its own limits without editing the desktop
// renderer. Indices are 16-bit, which caps a batch at 65,536 vertices; the cap
// is rounded down to whole quads so a batch never ends half a quad.
inline constexpr std::size_t kVerticesPerQuad = 4;
inline constexpr std::size_t kIndicesPerQuad = 6;
inline constexpr std::size_t kMaxVertices = 65532;
inline constexpr std::size_t kMaxQuads = kMaxVertices / kVerticesPerQuad;
inline constexpr std::size_t kMaxIndices = kMaxQuads * kIndicesPerQuad;
}  // namespace rse
