
# ---------------------------------------------------------------------------
# UnitTests -- GoogleTest based unit tests for the engine.
# Skipped automatically when the vendored googletest has no build for the
# requested architecture (it currently only ships Win32/Debug).
# ---------------------------------------------------------------------------
if(NOT RSE_GTEST_AVAILABLE)
  message(WARNING "RSE: skipping UnitTests target - no googletest build for architecture '${ARCHITECTURE}'")
  return()
endif()

file(GLOB_RECURSE UNITTEST_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

add_executable(UnitTests ${UNITTEST_FILES})

rse_configure_target(UnitTests)

# gtest only (not gtest_main) -- Main.cpp already provides the entry point.
target_link_libraries(UnitTests PRIVATE Engine gtest)

rse_deploy(UnitTests)

add_test(NAME UnitTests COMMAND UnitTests)
