#pragma once

#include <array>
#include <cstddef>
#include <set>
#include <string>

#include <gtest/gtest.h>

#include <core/uuid.h>

/*
An entity's uuid is the only identity that survives a save, a copy and a prefab
placement, so the two things that matter are that generation cannot repeat and
that the text form reads back as the same bytes. Both failures are silent: two
entities with one identity look like one entity to anything that looks one up,
and a scene that stores text it cannot parse loses every name it had.
*/
namespace {

using namespace rse;

bool IsLowerHex(char c) { return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'); }

TEST(UuidTest, GenerateWritesTheCanonicalText) {
  const Uuid uuid = Uuid::Generate();
  const std::string text = uuid.ToString();

  ASSERT_EQ(text.size(), Uuid::kTextLength);
  EXPECT_FALSE(uuid.IsNil());

  for (std::size_t i = 0; i < text.size(); ++i) {
    const bool separator = i == 8 || i == 13 || i == 18 || i == 23;

    if (separator) {
      EXPECT_EQ(text[i], '-') << "at " << i;
      continue;
    }

    EXPECT_TRUE(IsLowerHex(text[i])) << "at " << i << ": " << text[i];
  }
}

TEST(UuidTest, GenerateMarksTheVersionAndVariant) {
  for (int i = 0; i < 100; ++i) {
    const std::string text = Uuid::Generate().ToString();

    // The version is the high nibble of the third group; the variant is the
    // top two bits of the fourth, so that group starts with 8, 9, a or b.
    EXPECT_EQ(text[14], '4') << text;
    EXPECT_NE(std::string{"89ab"}.find(text[19]), std::string::npos) << text;
  }
}

TEST(UuidTest, GenerateDoesNotRepeat) {
  std::set<std::string> seen;

  for (int i = 0; i < 1000; ++i) {
    EXPECT_TRUE(seen.insert(Uuid::Generate().ToString()).second) << "repeated after " << i;
  }
}

TEST(UuidTest, ParseReadsTheTextBack) {
  for (int i = 0; i < 100; ++i) {
    const Uuid uuid = Uuid::Generate();
    Uuid parsed;

    ASSERT_TRUE(Uuid::Parse(uuid.ToString(), &parsed)) << uuid.ToString();
    EXPECT_EQ(parsed, uuid);
  }
}

TEST(UuidTest, ParseReadsTheCanonicalText) {
  Uuid uuid;

  ASSERT_TRUE(Uuid::Parse("01234567-89ab-4def-8123-456789abcdef", &uuid));
  EXPECT_EQ(uuid.ToString(), "01234567-89ab-4def-8123-456789abcdef");
}

TEST(UuidTest, ParseAcceptsUppercaseHex) {
  Uuid lower;
  Uuid upper;

  ASSERT_TRUE(Uuid::Parse("01234567-89ab-4def-8123-456789abcdef", &lower));
  ASSERT_TRUE(Uuid::Parse("01234567-89AB-4DEF-8123-456789ABCDEF", &upper));

  EXPECT_EQ(lower, upper);
  EXPECT_EQ(upper.ToString(), "01234567-89ab-4def-8123-456789abcdef");
}

TEST(UuidTest, ParseRejectsMalformedText) {
  const Uuid original = Uuid::Generate();
  const std::string valid{"01234567-89ab-4def-8123-456789abcdef"};

  // Empty, too short, too long, no separators, a separator in the wrong place,
  // a character that is not hex, and a group of the wrong width.
  const std::array<std::string, 7> malformed{"",
                                             "not a uuid",
                                             valid + "0",
                                             "0123456789ab4def8123456789abcdef",
                                             "0123456-789ab-4def-8123-456789abcdef",
                                             "01234567-89ab-4def-8123-456789abcde-",
                                             "01234567-89ag-4def-8123-456789abcdef"};

  for (const std::string& text : malformed) {
    Uuid target = original;

    EXPECT_FALSE(Uuid::Parse(text, &target)) << text;
    EXPECT_EQ(target, original) << "a failed parse must not touch the target";
  }

  EXPECT_FALSE(Uuid::Parse(valid, nullptr));
}

TEST(UuidTest, DefaultsToNil) {
  const Uuid nil;

  EXPECT_TRUE(nil.IsNil());
  EXPECT_EQ(nil.ToString(), "00000000-0000-0000-0000-000000000000");

  Uuid parsed;
  ASSERT_TRUE(Uuid::Parse(nil.ToString(), &parsed));
  EXPECT_TRUE(parsed.IsNil());
  EXPECT_EQ(parsed, nil);
}

TEST(UuidTest, OrdersByBytes) {
  std::array<uint8_t, Uuid::kSize> first{};
  std::array<uint8_t, Uuid::kSize> second{};

  first[0] = 1;
  second[0] = 2;

  EXPECT_LT(Uuid{first}, Uuid{second});
  EXPECT_GT(Uuid{second}, Uuid{first});
  EXPECT_NE(Uuid{first}, Uuid{second});
  EXPECT_EQ(Uuid{first}, Uuid{first});
}
}  // namespace
