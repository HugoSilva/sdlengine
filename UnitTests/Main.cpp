#include <iostream>

#include <gtest/gtest.h>

#include "ECSManagerProjectIo_test.h"
#include "OrthoView_test.h"
#include "SoundManager_test.h"
#include "TextureManager_test.h"

using namespace std;

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}