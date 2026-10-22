
# ---------------------------------------------------------------------------
# UnitTests -- GoogleTest based unit tests for the engine.
# Skipped automatically when GoogleTest is not available (RSE_BUILD_TESTS=OFF,
# or the fetch/subdirectory failed).
# ---------------------------------------------------------------------------
if(NOT RSE_GTEST_AVAILABLE)
  message(WARNING "RSE: skipping UnitTests target - GoogleTest is not available (enable RSE_BUILD_TESTS or set RSE_GTEST_SOURCE_DIR)")
  return()
endif()

file(GLOB_RECURSE UNITTEST_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(UnitTests ${UNITTEST_FILES})

rse_configure_target(UnitTests)

# GTest::gtest only (not GTest::gtest_main) -- Main.cpp provides the entry point.
target_link_libraries(UnitTests PRIVATE Engine GTest::gtest)

# The editor's project commands are file operations, so they are tested here
# even though they live next to the editor. Only the command translation unit is
# compiled in; the dialogs it can open never run headless.
if(RSE_BUILD_EDITOR AND EXISTS "${CMAKE_SOURCE_DIR}/Editor/Source/project_commands.cpp")
  target_sources(UnitTests PRIVATE "${CMAKE_SOURCE_DIR}/Editor/Source/project_commands.cpp")
  target_include_directories(UnitTests PRIVATE "${CMAKE_SOURCE_DIR}/Editor/Source")
  target_compile_definitions(UnitTests PRIVATE RSE_EDITOR_COMMANDS)
endif()

# The sample project is tested from the source tree, not from the deployed copy,
# so the tests fail when the checked-in project falls behind the format.
target_compile_definitions(UnitTests PRIVATE RSE_SAMPLE_PROJECT_DIR="${CMAKE_SOURCE_DIR}/SampleProject")

# Checked-in format fixtures: one file per published version. The tests read
# them, never write them, so a format change has to keep old files loading.
target_compile_definitions(UnitTests PRIVATE RSE_FIXTURES_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

rse_deploy(UnitTests)

add_test(NAME UnitTests COMMAND UnitTests)
