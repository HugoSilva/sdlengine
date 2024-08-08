
# Add source to this project's executable.
file(GLOB_RECURSE IMGUI_FILES "include/*.cpp" "include/*.h")
add_library(Imgui ${IMGUI_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Imgui PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.