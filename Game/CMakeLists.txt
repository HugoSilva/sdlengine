
# ---------------------------------------------------------------------------
# Game -- sample project that exercises the engine.
# All dependencies come transitively from Engine.
# ---------------------------------------------------------------------------
file(GLOB_RECURSE GAME_FILES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.hpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/Source/*.h")

add_executable(Game ${GAME_FILES})

rse_configure_target(Game)
target_link_libraries(Game PRIVATE Engine)
rse_deploy(Game)
