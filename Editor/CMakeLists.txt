
# Add source to this project's executable.
file(GLOB_RECURSE EDITOR_FILES "Source/*.cpp" "Source/*.hpp" "Source/*.h")
add_executable(Editor ${EDITOR_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Editor PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_include_directories(Editor PRIVATE "../Engine/Engine")

find_package(OpenGL REQUIRED)

if (OPENGL_FOUND)
    message("opengl found")
    #message("include dir: ${OPENGL_INCLUDE_DIR}")
    #message("link libraries: ${OPENGL_gl_LIBRARY}")
else (OPENGL_FOUND)
    message("opengl not found")
endif()

target_link_libraries(Editor PRIVATE Engine ${OPENGL_gl_LIBRARY})

if(WIN32)
  add_custom_command(TARGET Editor POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:Editor> $<TARGET_FILE_DIR:Editor>
    COMMAND_EXPAND_LISTS
    )
endif()