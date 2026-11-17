#pragma once

#include <array>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace rse {
// A 128-bit name that stays with the thing it names.
//
// The engine needs one because nothing else it has is stable enough. An entt id
// is an index into one registry: it is reused after a destroy and is not
// written to a file. A component's `Name` can be edited, and two entities are
// free to share it. A uuid is generated once, travels with whatever it names,
// and is not handed out for anything else.
//
// The text form is the canonical 36-character one -- 8-4-4-4-12 lowercase hex,
// separated by dashes -- so a scene file and a log line show the same bytes.
// Generate() produces a version 4 (random) uuid with the RFC 4122 variant
// bits, which is what makes one from this engine recognisable as a uuid rather
// than 16 arbitrary bytes.
class Uuid {
 public:
  static constexpr std::size_t kSize{16};
  static constexpr std::size_t kTextLength{36};

  // The nil uuid: all zeroes, meaning "no uuid". It is what a default value
  // holds; nothing is ever named by it.
  constexpr Uuid() = default;

  explicit constexpr Uuid(const std::array<uint8_t, kSize>& bytes) : m_bytes(bytes) {}

  // A new random uuid. Generate() never returns nil and, in practice, never
  // returns the same value twice: 122 of the 128 bits are random.
  static Uuid Generate();

  // Reads `text` in the canonical form. Uppercase hex is accepted and
  // lowercased on output. Returns false, leaving `uuid` untouched, when `text`
  // is not exactly 36 characters in the right shape.
  static bool Parse(std::string_view text, Uuid* uuid);

  bool IsNil() const;

  // The canonical text form: lowercase hex, 8-4-4-4-12.
  std::string ToString() const;

  const std::array<uint8_t, kSize>& Bytes() const { return m_bytes; }

  bool operator==(const Uuid&) const = default;
  auto operator<=>(const Uuid&) const = default;

 private:
  std::array<uint8_t, kSize> m_bytes{};
};
}  // namespace rse
