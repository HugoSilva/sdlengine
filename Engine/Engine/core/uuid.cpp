#include "uuid.h"

#include <random>

namespace rse {
namespace {
// -1 when `c` is not a hex digit. Both cases are accepted on the way in; the
// text form is always written lowercase.
int HexValue(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  }

  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }

  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }

  return -1;
}
}  // namespace

Uuid Uuid::Generate() {
  // One engine per thread: seeding it per call would lean on random_device for
  // every uuid, and a shared one would serialise every caller. mt19937_64 is
  // not a cryptographic generator, which does not matter here -- identities
  // need to be unique, not unguessable.
  static thread_local std::mt19937_64 engine{std::random_device{}()};

  std::array<uint8_t, kSize> bytes{};
  const uint64_t high = engine();
  const uint64_t low = engine();

  for (std::size_t i = 0; i < 8; ++i) {
    bytes[i] = static_cast<uint8_t>(high >> (i * 8));
    bytes[i + 8] = static_cast<uint8_t>(low >> (i * 8));
  }

  // Version 4 in the high nibble of byte 6, and the RFC 4122 variant -- the
  // top two bits of byte 8 set to 10 -- so a reader can tell this is a random
  // uuid. The rest of both bytes stays random.
  bytes[6] = static_cast<uint8_t>((bytes[6] & 0x0F) | 0x40);
  bytes[8] = static_cast<uint8_t>((bytes[8] & 0x3F) | 0x80);

  return Uuid{bytes};
}

bool Uuid::Parse(std::string_view text, Uuid* uuid) {
  if (uuid == nullptr || text.size() != kTextLength) {
    return false;
  }

  std::array<uint8_t, kSize> bytes{};
  std::size_t byte{0};

  for (std::size_t i = 0; i < kTextLength; ++i) {
    // The four separators are fixed positions in the canonical form; anywhere
    // else a dash is just a character that is not hex.
    if (i == 8 || i == 13 || i == 18 || i == 23) {
      if (text[i] != '-') {
        return false;
      }

      continue;
    }

    const int high = HexValue(text[i]);
    const int low = HexValue(text[++i]);

    if (high < 0 || low < 0) {
      return false;
    }

    bytes[byte++] = static_cast<uint8_t>((high << 4) | low);
  }

  *uuid = Uuid{bytes};
  return true;
}

bool Uuid::IsNil() const {
  for (const uint8_t byte : m_bytes) {
    if (byte != 0) {
      return false;
    }
  }

  return true;
}

std::string Uuid::ToString() const {
  static constexpr char kHex[]{"0123456789abcdef"};

  std::string text;
  text.reserve(kTextLength);

  for (std::size_t i = 0; i < kSize; ++i) {
    if (i == 4 || i == 6 || i == 8 || i == 10) {
      text.push_back('-');
    }

    text.push_back(kHex[m_bytes[i] >> 4]);
    text.push_back(kHex[m_bytes[i] & 0x0F]);
  }

  return text;
}
}  // namespace rse
