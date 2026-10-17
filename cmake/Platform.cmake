# cmake/Platform.cmake
# Cross-cutting configuration shared by every RSE target.
#
# Nothing in here knows about specific dependencies (see Dependencies.cmake) or
# about individual projects (see the per-project CMakeLists.txt files).

# ---------------------------------------------------------------------------
# Architecture
# ---------------------------------------------------------------------------
# The vendored libraries ship prebuilt for a specific folder name: "Win32" or
# "x64". The CMake presets set ARCHITECTURE explicitly; when they don't we fall
# back to a best guess so a plain `cmake -S . -B build` still works.
if(NOT ARCHITECTURE)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE "x64")
  else()
    set(ARCHITECTURE "Win32")
  endif()
endif()

# ---------------------------------------------------------------------------
# Standard target configuration
# ---------------------------------------------------------------------------
# Apply this to every first-party target (Engine, Editor, Game, UnitTests) so
# language level, warning level and output locations stay identical everywhere.
function(rse_configure_target target)
  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    # Keep artifacts grouped per configuration instead of scattered per project.
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/$<CONFIG>"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/$<CONFIG>"
    # Let IDE debuggers find Resources/ next to the executable.
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${target}>")

  if(MSVC)
    target_compile_options(${target} PRIVATE /W3 /bigobj)
  endif()

  # Suppress the "structure was padded due to alignment" noise coming from
  # third-party headers (entt / Box2D) that we do not control.
  target_compile_definitions(${target} PRIVATE WINDOWS_IGNORE_PACKING_MISMATCH)
endfunction()
