
# ---------------------------------------------------------------------------
# Engine -- static library containing the whole engine (Engine/Engine only).
# The unit tests used to be globbed into this target; they are now their own
# target (see UnitTests/CMakeLists.txt).
# ---------------------------------------------------------------------------
file(GLOB_RECURSE ENGINE_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/*.h")

# Engine/Tests holds a runtime debug helper (Test::TestClearColor) that
# OpenGLRenderer depends on -- it is NOT a unit-test folder. Include it
# explicitly rather than pulling in an unrelated Tests/ tree by accident.
file(GLOB_RECURSE ENGINE_DEBUG_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Tests/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Tests/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Tests/*.h")
list(APPEND ENGINE_FILES ${ENGINE_DEBUG_FILES})

# ImGui's SDL backend: GL3 on desktop, GLES2 on Emscripten. Exactly one of the
# two implementations must be compiled (previously the selection never fired
# because of the invalid `if(DEFINED ${EMSCRIPTEN})` condition).
if(EMSCRIPTEN)
  set(_rse_imgui_backend "imgui_impl_sdl_gl3")
else()
  set(_rse_imgui_backend "imgui_impl_sdl_gles2")
endif()
list(REMOVE_ITEM ENGINE_FILES
  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Utils/${_rse_imgui_backend}.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Engine/Utils/${_rse_imgui_backend}.h")

add_library(Engine STATIC ${ENGINE_FILES})
add_library(RSE::Engine ALIAS Engine)

rse_configure_target(Engine)

# Expose the engine's own headers so consumers can `#include <Core.h>`.
target_include_directories(Engine PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Engine")

# PUBLIC: everyone who links Engine gets the third-party includes, import
# libraries and OpenGL transitively.
target_link_libraries(Engine
  PUBLIC
    RSE::ThirdParty
    Imgui
    GLEW::GLEW
    SDL2::SDL2
    SDL2::SDL2main
    SDL2::Image
    SDL2::Mixer
    SDL2::TTF
    Box2D::Box2D
    OpenGL::GL)
