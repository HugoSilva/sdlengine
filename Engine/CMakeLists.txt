
# ---------------------------------------------------------------------------
# Engine -- static library containing the whole engine (Engine/Engine only).
# The unit tests used to be globbed into this target; they are now their own
# target (see UnitTests/CMakeLists.txt).
# ---------------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.h")

# Engine/tests holds a runtime debug helper (test::TestClearColor) that
# OpenGLRenderer depends on -- it is NOT a unit-test folder. Include it
# explicitly rather than pulling in an unrelated tests/ tree by accident.
file(GLOB_RECURSE ENGINE_DEBUG_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h")
list(APPEND ENGINE_FILES ${ENGINE_DEBUG_FILES})


add_library(Engine STATIC ${ENGINE_FILES})
add_library(RSE::Engine ALIAS Engine)

rse_configure_target(Engine)

# Expose the engine's own headers so consumers can `#include <core.h>`.
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Engine")

# The runtime debug helper sits outside the module root, so the module
# directory is on the include path too, for `#include <tests/...>`. It is
# PUBLIC because a public engine header names that helper.
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

# PUBLIC: everyone who links Engine gets the third-party includes, import
# libraries and OpenGL transitively.
target_link_libraries(Engine
  PUBLIC
    RSE::ThirdParty
    Imgui
    GLEW::GLEW
    SDL3::SDL3
    SDL3_image::SDL3_image
    SDL3_mixer::SDL3_mixer
    SDL3_ttf::SDL3_ttf
    Box2D::Box2D
    OpenGL::GL)
