
# ---------------------------------------------------------------------------
# Editor -- ImGui based scene/entity editor.
# All include directories / import libraries / OpenGL come transitively from
# Engine, so nothing third-party is listed here.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE EDITOR_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")

add_executable(Editor ${EDITOR_FILES})

rse_configure_target(Editor)
target_link_libraries(Editor PRIVATE Engine)
rse_deploy(Editor)
