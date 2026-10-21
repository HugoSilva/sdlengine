#pragma once

#include <string>

#include <gtest/gtest.h>

#include <input/input_map.h>
#include <utils/logger.h>

/*
The input map is the one thing a project can use to change a key without the
engine knowing about the action that key belongs to. Nothing here needs a
window: parsing, formatting and matching are data, and the defaults are the
list every build ships.
*/
namespace {
using namespace rse;

class InputMapTest : public ::testing::Test {
 protected:
  void SetUp() override { Logger::Clear(); }

  void TearDown() override {
    InputMap::Reset();
    Logger::Clear();
  }
};

TEST_F(InputMapTest, ParsesAKeyWithModifiers) {
  const Binding binding = InputMap::Parse("Ctrl+Shift+Alt+S");

  EXPECT_EQ(binding.scancode, SDL_SCANCODE_S);
  EXPECT_TRUE(binding.ctrl);
  EXPECT_TRUE(binding.shift);
  EXPECT_TRUE(binding.alt);
}

TEST_F(InputMapTest, ParsesNamesCaseInsensitively) {
  const Binding binding = InputMap::Parse("shift+f2");

  EXPECT_EQ(binding.scancode, SDL_SCANCODE_F2);
  EXPECT_TRUE(binding.shift);
  EXPECT_FALSE(binding.ctrl);
}

TEST_F(InputMapTest, ParsesTheNamesPeopleActuallyType) {
  EXPECT_EQ(InputMap::Parse("Enter").scancode, SDL_SCANCODE_RETURN);
  EXPECT_EQ(InputMap::Parse("Escape").scancode, SDL_SCANCODE_ESCAPE);
  EXPECT_EQ(InputMap::Parse("Space").scancode, SDL_SCANCODE_SPACE);
  EXPECT_EQ(InputMap::Parse("Up").scancode, SDL_SCANCODE_UP);
  EXPECT_EQ(InputMap::Parse("s").scancode, SDL_SCANCODE_S);
}

TEST_F(InputMapTest, AnUnreadableTextIsInvalid) {
  EXPECT_FALSE(InputMap::Parse("").IsValid());
  EXPECT_FALSE(InputMap::Parse("Ctrl+").IsValid());
  EXPECT_FALSE(InputMap::Parse("NotAKey").IsValid());
  EXPECT_FALSE(InputMap::Parse("Ctrl+S+E").IsValid()) << "two keys are not one binding";
  EXPECT_FALSE(InputMap::Parse("Ctrl").IsValid()) << "modifiers alone are not a key";
}

TEST_F(InputMapTest, FormatRoundTripsWhatParseReads) {
  for (const std::string text : {"Ctrl+S", "Shift+F2", "Ctrl+Shift+Alt+A", "Space", "Up"}) {
    const Binding binding = InputMap::Parse(text);
    ASSERT_TRUE(binding.IsValid()) << text;

    // The spelling is normalised, so compare parse(format(parse(x))) rather
    // than the text itself: "shift+f2" and "Shift+F2" are the same binding.
    EXPECT_EQ(InputMap::Format(InputMap::Parse(InputMap::Format(binding))), InputMap::Format(binding)) << text;
  }

  EXPECT_EQ(InputMap::Format(InputMap::Parse("ctrl+s")), "Ctrl+S");
  EXPECT_EQ(InputMap::Format(Binding{}), "");
}

TEST_F(InputMapTest, MatchesHonoursEveryModifier) {
  const Binding ctrl_s{SDL_SCANCODE_S, true, false, false};

  EXPECT_TRUE(ctrl_s.Matches(SDL_SCANCODE_S, true, false, false));
  EXPECT_FALSE(ctrl_s.Matches(SDL_SCANCODE_S, false, false, false));
  EXPECT_FALSE(ctrl_s.Matches(SDL_SCANCODE_S, true, true, false));
  EXPECT_FALSE(ctrl_s.Matches(SDL_SCANCODE_A, true, false, false));

  // And the other way round: a plain binding does not fire while a modifier is
  // held, which is what keeps "W" out of the way of "Ctrl+W".
  const Binding plain_w{SDL_SCANCODE_W, false, false, false};
  EXPECT_TRUE(plain_w.Matches(SDL_SCANCODE_W, false, false, false));
  EXPECT_FALSE(plain_w.Matches(SDL_SCANCODE_W, true, false, false));
}

TEST_F(InputMapTest, TheDefaultsCoverEveryActionTheBuildUses) {
  for (const std::string action : {"editor.save", "editor.reload", "editor.gizmo.translate", "editor.gizmo.rotate",
                                   "editor.gizmo.scale", "game.up", "game.down", "game.left", "game.right"}) {
    EXPECT_TRUE(InputMap::IsKnown(action)) << action;
    EXPECT_TRUE(InputMap::Get(action).IsValid()) << action;
  }
}

TEST_F(InputMapTest, AProjectOverridesTheDefault) {
  InputMap::SetOverrides({{"editor.save", "F5"}});

  EXPECT_EQ(InputMap::Get("editor.save").scancode, SDL_SCANCODE_F5);

  // Everything the project did not mention keeps the compiled default.
  EXPECT_EQ(InputMap::Get("editor.reload").scancode, SDL_SCANCODE_F2);
}

TEST_F(InputMapTest, AnUnknownActionWarnsOnceAndIsIgnored) {
  InputMap::SetOverrides({{"editor.stop", "F9"}, {"editor.stop", "F10"}});

  EXPECT_FALSE(InputMap::IsKnown("editor.stop"));

  int warnings = 0;

  for (const Logger::Entry& entry : Logger::GetEntries()) {
    if (entry.level == Logger::Level::kWarning) {
      ++warnings;
    }
  }

  EXPECT_EQ(warnings, 1) << "a duplicate action is one problem, not two";
}

TEST_F(InputMapTest, AnUnreadableKeyKeepsTheDefault) {
  InputMap::SetOverrides({{"editor.save", "NotAKey"}});

  EXPECT_EQ(InputMap::Get("editor.save").scancode, SDL_SCANCODE_S);
  EXPECT_TRUE(InputMap::Get("editor.save").ctrl);
}

TEST_F(InputMapTest, ResetGoesBackToTheDefaults) {
  InputMap::SetOverrides({{"editor.save", "F5"}});
  InputMap::Reset();

  EXPECT_EQ(InputMap::Get("editor.save").scancode, SDL_SCANCODE_S);
  EXPECT_TRUE(InputMap::Get("editor.save").ctrl);
}

TEST_F(InputMapTest, AnUnknownActionHasNoBinding) { EXPECT_FALSE(InputMap::Get("nothing.bind.this").IsValid()); }
}  // namespace
