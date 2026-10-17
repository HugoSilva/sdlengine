
cmake_minimum_required(VERSION 3.30.0)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("RSEngine" LANGUAGES C CXX)

# ---------------------------------------------------------------------------
# Options
# ---------------------------------------------------------------------------
option(RSE_BUILD_EDITOR "Build the RSE editor" ON)
option(RSE_BUILD_GAME   "Build the sample game" ON)
option(RSE_BUILD_TESTS  "Build the unit tests" ON)

# ---------------------------------------------------------------------------
# Shared build logic + dependency definitions (single source of truth)
# ---------------------------------------------------------------------------
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(Platform)
include(Dependencies)

# Collect the Resources/ folders that must end up next to the executables, then
# create the single shared deployment target (see cmake/Deploy.cmake).
set(RSE_RESOURCE_DIRS "")
if(RSE_BUILD_EDITOR)
  list(APPEND RSE_RESOURCE_DIRS "${CMAKE_SOURCE_DIR}/Editor/Resources")
endif()
if(RSE_BUILD_GAME)
  list(APPEND RSE_RESOURCE_DIRS "${CMAKE_SOURCE_DIR}/Game/Resources")
endif()
include(Deploy)

# ---------------------------------------------------------------------------
# Vendored code built as part of the solution
# ---------------------------------------------------------------------------
add_subdirectory("Thirdparty/imgui")

# ---------------------------------------------------------------------------
# First-party targets
# ---------------------------------------------------------------------------
add_subdirectory("Engine")

if(RSE_BUILD_EDITOR)
  add_subdirectory("Editor")
endif()

if(RSE_BUILD_GAME)
  add_subdirectory("Game")
endif()

if(RSE_BUILD_TESTS)
  enable_testing()
  add_subdirectory("UnitTests")
endif()
