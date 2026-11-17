#pragma once

#include <gtest/gtest.h>

#include <input/input_manager.h>
#include <input/keyboard.h>

/*
The keyboard is read once a frame, so there are three answers and not one: is
the key down now, did it go down since the frame began, and did it come up.
These tests drive the same callback SDL drives and pin which of the three moves
when, because getting it wrong is silent -- a keyboard that only walks while a
key is tapped looks like a broken scene, not a broken input system.
*/
namespace {

using namespace rse;

SDL_KeyboardEvent KeyEvent(SDL_Scancode scancode, bool down) {
  SDL_KeyboardEvent event{};
  event.scancode = scancode;
  event.down = down;
  return event;
}

class KeyboardTest : public ::testing::Test {
 protected:
  // Every test starts and ends on a frame boundary, so a latch left by one test
  // cannot be read by the next.
  void SetUp() override {
    Keyboard::BeginFrame();
    InputManager::Unlock();
  }

  void TearDown() override {
    Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_A, false));
    Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_B, false));
    Keyboard::BeginFrame();
    InputManager::Unlock();
  }
};

TEST_F(KeyboardTest, PressingAKeyHoldsItAndLatchesThePress) {
  Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_A, true));

  EXPECT_TRUE(Keyboard::IsKeyDown(SDL_SCANCODE_A));
  EXPECT_TRUE(Keyboard::IsKeyPressed(SDL_SCANCODE_A));
  EXPECT_FALSE(Keyboard::IsKeyReleased(SDL_SCANCODE_A));
}

TEST_F(KeyboardTest, ThePressLatchLastsOneFrameButTheKeyStaysDown) {
  Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_A, true));
  Keyboard::BeginFrame();

  EXPECT_TRUE(Keyboard::IsKeyDown(SDL_SCANCODE_A));
  EXPECT_FALSE(Keyboard::IsKeyPressed(SDL_SCANCODE_A));

  // A second frame with nothing new is the same: down, not pressed.
  Keyboard::BeginFrame();

  EXPECT_TRUE(Keyboard::IsKeyDown(SDL_SCANCODE_A));
  EXPECT_FALSE(Keyboard::IsKeyPressed(SDL_SCANCODE_A));
}

TEST_F(KeyboardTest, HoldingAKeyIsOnePress) {
  Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_A, true));
  Keyboard::BeginFrame();

  // SDL repeats a held key; the repeat arrives as another key-down event.
  Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_A, true));

  EXPECT_FALSE(Keyboard::IsKeyPressed(SDL_SCANCODE_A));
  EXPECT_TRUE(Keyboard::IsKeyDown(SDL_SCANCODE_A));
}

TEST_F(KeyboardTest, ReleasingAKeyLatchesTheReleaseForOneFrame) {
  Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_A, true));
  Keyboard::BeginFrame();

  Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_A, false));

  EXPECT_FALSE(Keyboard::IsKeyDown(SDL_SCANCODE_A));
  EXPECT_TRUE(Keyboard::IsKeyReleased(SDL_SCANCODE_A));
  EXPECT_FALSE(Keyboard::IsKeyPressed(SDL_SCANCODE_A));

  Keyboard::BeginFrame();

  EXPECT_FALSE(Keyboard::IsKeyReleased(SDL_SCANCODE_A));
}

TEST_F(KeyboardTest, AReleaseWithoutAPressIsNotARelease) {
  // The window lost focus while the key was held, so the release arrives for a
  // key this system never saw go down.
  Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_A, false));

  EXPECT_FALSE(Keyboard::IsKeyDown(SDL_SCANCODE_A));
  EXPECT_FALSE(Keyboard::IsKeyReleased(SDL_SCANCODE_A));
}

TEST_F(KeyboardTest, OneKeyDoesNotAffectAnother) {
  Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_A, true));

  EXPECT_TRUE(Keyboard::IsKeyDown(SDL_SCANCODE_A));
  EXPECT_FALSE(Keyboard::IsKeyDown(SDL_SCANCODE_B));
  EXPECT_FALSE(Keyboard::IsKeyPressed(SDL_SCANCODE_B));
}

TEST_F(KeyboardTest, AKeyOutsideTheTableIsNeverAnything) {
  EXPECT_FALSE(Keyboard::IsKeyDown(-1));
  EXPECT_FALSE(Keyboard::IsKeyDown(SDL_SCANCODE_COUNT));
  EXPECT_FALSE(Keyboard::IsKeyPressed(-1));
  EXPECT_FALSE(Keyboard::IsKeyPressed(SDL_SCANCODE_COUNT));
  EXPECT_FALSE(Keyboard::IsKeyReleased(-1));

  // And a callback for one cannot write outside the table either.
  Keyboard::KeyCallback(KeyEvent(static_cast<SDL_Scancode>(SDL_SCANCODE_COUNT + 4), true));
  Keyboard::KeyCallback(KeyEvent(static_cast<SDL_Scancode>(-2), true));
}

TEST_F(KeyboardTest, WhileTheUiOwnsTheKeyboardNothingElseSeesIt) {
  Keyboard::KeyCallback(KeyEvent(SDL_SCANCODE_A, true));

  InputManager::Lock();

  EXPECT_FALSE(InputManager::IsKeyDown(SDL_SCANCODE_A));
  EXPECT_FALSE(InputManager::IsKeyPressed(SDL_SCANCODE_A));
  EXPECT_FALSE(InputManager::IsKeyReleased(SDL_SCANCODE_A));

  InputManager::Unlock();

  // The key is still held underneath: the lock hides it, it does not release
  // it.
  EXPECT_TRUE(InputManager::IsKeyDown(SDL_SCANCODE_A));
}
}  // namespace
