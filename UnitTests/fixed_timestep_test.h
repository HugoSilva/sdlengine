#pragma once

#include <cmath>
#include <limits>

#include <gtest/gtest.h>

#include <core/fixed_timestep.h>

/*
The accumulator is the half of the fixed timestep that decides when to step, so
these tests pin the three things a wrong version would get wrong: how many
steps a frame owes, how much of the leftover becomes the interpolation alpha,
and what happens to a stall.
*/
namespace {

using namespace rse;

TEST(FixedTimestepTest, SixtyFramesAtSixtyHertzAreSixtySteps) {
  FixedTimestep timestep;

  int steps = 0;
  for (int frame = 0; frame < 60; ++frame) {
    steps += timestep.Advance(1.0f / 60.0f);
  }

  EXPECT_EQ(steps, 60);
  EXPECT_NEAR(timestep.Alpha(), 0.0f, 0.001f);
}

TEST(FixedTimestepTest, TheStepDoesNotDependOnTheFrameRate) {
  for (float rate : {60.0f, 144.0f, 240.0f}) {
    FixedTimestep timestep;
    int steps = 0;

    // One second of wall time, at three refresh rates, owes the same number of
    // steps: 60. This is the whole point of the class. A step either way is
    // float rounding -- 1/144 is not exact, so a hundred and forty-four of them
    // do not add up to exactly one second -- not a rate dependence.
    for (int frame = 0; frame < static_cast<int>(rate); ++frame) {
      steps += timestep.Advance(1.0f / rate);
    }

    EXPECT_GE(steps, 59) << "at " << rate << " Hz";
    EXPECT_LE(steps, 60) << "at " << rate << " Hz";
  }
}

TEST(FixedTimestepTest, BetweenTwoStepsReturnsNoStep) {
  FixedTimestep timestep;

  // Half a step is owed to the next one, not run now.
  EXPECT_EQ(timestep.Advance(1.0f / 120.0f), 0);
  EXPECT_NEAR(timestep.Alpha(), 0.5f, 0.001f);

  EXPECT_EQ(timestep.Advance(1.0f / 120.0f), 1);
  EXPECT_NEAR(timestep.Alpha(), 0.0f, 0.001f);
}

TEST(FixedTimestepTest, LongFrameRunsTheCapAndDropsTheRest) {
  FixedTimestep timestep;

  // A one-second stall is twelve steps of owed time; five run and the rest is
  // dropped, so the frames after the stall are not spent catching up.
  EXPECT_EQ(timestep.Advance(1.0f), FixedTimestep::kDefaultMaxSteps);
  EXPECT_NEAR(timestep.Alpha(), 0.0f, 0.001f);

  // The next ordinary frame owes exactly its own step.
  EXPECT_EQ(timestep.Advance(1.0f / 60.0f), 1);
}

TEST(FixedTimestepTest, TheCapAppliesUntilTheAccumulatorIsDrained) {
  FixedTimestep timestep(1.0f / 60.0f, 4);

  for (int frame = 0; frame < 100; ++frame) {
    EXPECT_LE(timestep.Advance(1.0f), 4);
    EXPECT_LT(timestep.Alpha(), 1.0f);
  }
}

TEST(FixedTimestepTest, AClockThatDidNotAdvanceIsNotAStep) {
  FixedTimestep timestep;

  EXPECT_EQ(timestep.Advance(0.0f), 0);
  EXPECT_EQ(timestep.Advance(-0.5f), 0);
  EXPECT_EQ(timestep.Advance(std::nanf("")), 0);
  EXPECT_EQ(timestep.Advance(std::numeric_limits<float>::infinity()), 0);
  EXPECT_NEAR(timestep.Alpha(), 0.0f, 0.001f);
}

TEST(FixedTimestepTest, ResetDropsTheTimeBanked) {
  FixedTimestep timestep;

  EXPECT_EQ(timestep.Advance(1.0f / 120.0f), 0);
  EXPECT_NEAR(timestep.Alpha(), 0.5f, 0.001f);

  timestep.Reset();

  EXPECT_NEAR(timestep.Alpha(), 0.0f, 0.001f);
  EXPECT_EQ(timestep.Advance(1.0f / 60.0f), 1);
}
}  // namespace
