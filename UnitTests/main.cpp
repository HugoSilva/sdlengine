#include <iostream>

#include <gtest/gtest.h>

#include "camera_test.h"
#include "ortho_view_test.h"
#include "physics_system_test.h"
#include "prefab_io_test.h"
#include "project_commands_test.h"
#include "project_io_test.h"
#include "sample_project_test.h"
#include "scene_io_test.h"
#include "sound_manager_test.h"
#include "texture_manager_test.h"

using namespace std;

int main(int argc, char** argv) {
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
