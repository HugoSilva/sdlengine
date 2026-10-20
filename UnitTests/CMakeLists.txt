
# ---------------------------------------------------------------------------
# UnitTests -- GoogleTest based unit tests for the engine.
# Skipped automatically when the vendored googletest has no build for the
# requested architecture (it currently only ships Win32/Debug).
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

# The sample project is tested from the source tree, not from the deployed copy,
# so the tests fail when the checked-in project falls behind the format.
target_compile_definitions(UnitTests PRIVATE RSE_SAMPLE_PROJECT_DIR="${CMAKE_SOURCE_DIR}/SampleProject")

rse_deploy(UnitTests)

add_test(NAME UnitTests COMMAND UnitTests)
