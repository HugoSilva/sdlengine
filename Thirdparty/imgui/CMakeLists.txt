
# ---------------------------------------------------------------------------
# ImGui (vendored, 1.93.0) + the official SDL3 / OpenGL3 backends.
#
# The engine previously shipped hand-written SDL/GL backends built on the
# ImGui 1.60 API (io.KeyMap / io.KeysDown), which no longer exists. The
# official backends replace them:
#   backends/imgui_impl_sdl3.cpp        platform (window, input)
#   backends/imgui_impl_opengl3.cpp     renderer (desktop GL + GLES)
# ---------------------------------------------------------------------------
file(GLOB IMGUI_CORE_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h")

file(GLOB IMGUI_BACKEND_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/backends/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/backends/*.h")

add_library(Imgui ${IMGUI_CORE_FILES} ${IMGUI_BACKEND_FILES})

set_property(TARGET Imgui PROPERTY CXX_STANDARD 20)
set_property(TARGET Imgui PROPERTY CXX_STANDARD_REQUIRED ON)

target_include_directories(Imgui PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/include"
  "${CMAKE_CURRENT_SOURCE_DIR}/backends")

# The SDL3 backend needs SDL3 (RSE::ThirdParty carries its include directory,
# since the imported SDL3 target only points at the .lib/.dll).
target_link_libraries(Imgui PUBLIC SDL3::SDL3 RSE::ThirdParty)
