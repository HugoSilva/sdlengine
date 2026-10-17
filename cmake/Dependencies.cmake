# cmake/Dependencies.cmake
# Single source of truth for every vendored third-party dependency.
#
# No other file in the build may reference a Thirdparty/ path directly. To use a
# dependency, link one of the imported targets defined here.
#
#   RSE::ThirdParty   header-only interface (all include directories)
#   GLEW::GLEW        OpenGL extension loader            (shared)
#   SDL2::SDL2        windowing / input / platform        (shared)
#   SDL2::SDL2main    WinMain entry-point shim            (static)
#   SDL2::Image       image loading                       (shared)
#   SDL2::Mixer       audio                               (shared)
#   SDL2::TTF         font rendering                       (shared)
#   Box2D::Box2D      2D physics                           (static, per-config)
#   gtest             GoogleTest (only if vendored for $ARCHITECTURE)
#   OpenGL::GL        system OpenGL
#
# Runtime DLLs of the shared dependencies are collected in RSE_RUNTIME_DLLS and
# deployed next to each executable through rse_deploy_runtime().

set(RSE_TP "${CMAKE_SOURCE_DIR}/Thirdparty")

if(NOT ARCHITECTURE)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCHITECTURE "x64")
  else()
    set(ARCHITECTURE "Win32")
  endif()
endif()
message(STATUS "RSE: resolving vendored libraries for architecture '${ARCHITECTURE}'")

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------
function(rse_import_shared name implib dll)
  add_library(${name} SHARED IMPORTED GLOBAL)
  set_target_properties(${name} PROPERTIES
    IMPORTED_IMPLIB "${implib}"
    IMPORTED_LOCATION "${dll}")
endfunction()

function(rse_import_static name lib)
  add_library(${name} STATIC IMPORTED GLOBAL)
  set_target_properties(${name} PROPERTIES IMPORTED_LOCATION "${lib}")
endfunction()

# ---------------------------------------------------------------------------
# Header-only / include-only interface
# ---------------------------------------------------------------------------
add_library(RSE::ThirdParty INTERFACE IMPORTED GLOBAL)
target_include_directories(RSE::ThirdParty INTERFACE
  "${RSE_TP}/Glew/include"
  "${RSE_TP}/SDL2/include"
  "${RSE_TP}/SDL2_image/include"
  "${RSE_TP}/SDL2_mixer/include"
  "${RSE_TP}/SDL2_ttf/include"
  "${RSE_TP}/glm/include"
  "${RSE_TP}/imgui/include"
  "${RSE_TP}/Box2D/include"
  "${RSE_TP}/cereal"
  "${RSE_TP}/rapidxml"
  "${RSE_TP}/entt")

# ---------------------------------------------------------------------------
# Shared libraries: import library (.lib) + runtime library (.dll)
# ---------------------------------------------------------------------------
rse_import_shared(GLEW::GLEW
  "${RSE_TP}/Glew/lib/${ARCHITECTURE}/glew32.lib"
  "${RSE_TP}/Glew/bin/${ARCHITECTURE}/glew32.dll")

rse_import_shared(SDL2::SDL2
  "${RSE_TP}/SDL2/lib/${ARCHITECTURE}/SDL2.lib"
  "${RSE_TP}/SDL2/bin/${ARCHITECTURE}/SDL2.dll")

rse_import_shared(SDL2::Image
  "${RSE_TP}/SDL2_image/lib/${ARCHITECTURE}/SDL2_image.lib"
  "${RSE_TP}/SDL2_image/bin/${ARCHITECTURE}/SDL2_image.dll")

rse_import_shared(SDL2::Mixer
  "${RSE_TP}/SDL2_mixer/lib/${ARCHITECTURE}/SDL2_mixer.lib"
  "${RSE_TP}/SDL2_mixer/bin/${ARCHITECTURE}/SDL2_mixer.dll")

rse_import_shared(SDL2::TTF
  "${RSE_TP}/SDL2_ttf/lib/${ARCHITECTURE}/SDL2_ttf.lib"
  "${RSE_TP}/SDL2_ttf/bin/${ARCHITECTURE}/SDL2_ttf.dll")

# SDL2main is a static helper that provides the WinMain -> main shim.
rse_import_static(SDL2::SDL2main
  "${RSE_TP}/SDL2/lib/${ARCHITECTURE}/SDL2main.lib")

# ---------------------------------------------------------------------------
# Box2D -- static, config-specific (Box2D_d.lib in Debug, Box2D.lib otherwise).
# This replaces the old hard-coded Box2D_d.lib that broke Release builds.
# ---------------------------------------------------------------------------
add_library(Box2D::Box2D STATIC IMPORTED GLOBAL)
set_target_properties(Box2D::Box2D PROPERTIES
  IMPORTED_LOCATION_DEBUG   "${RSE_TP}/Box2D/lib/${ARCHITECTURE}/Box2D_d.lib"
  IMPORTED_LOCATION_RELEASE "${RSE_TP}/Box2D/lib/${ARCHITECTURE}/Box2D.lib"
  MAP_IMPORTED_CONFIG_RELWITHDEBINFO "RELEASE"
  MAP_IMPORTED_CONFIG_MINSIZEREL     "RELEASE")

# ---------------------------------------------------------------------------
# GoogleTest
# ---------------------------------------------------------------------------
# Built from source so every architecture/configuration works. The old
# Thirdparty/googletest binaries were Win32/Debug only, which is why UnitTests
# could not be built for x64.
#
# Default: fetch a pinned release with FetchContent.
# Offline : point RSE_GTEST_SOURCE_DIR at a local GoogleTest checkout, or set
#           RSE_BUILD_TESTS=OFF to skip tests entirely.
# ---------------------------------------------------------------------------
set(RSE_GTEST_AVAILABLE FALSE)

if(RSE_BUILD_TESTS)
  include(FetchContent)
  set(RSE_GTEST_SOURCE_DIR "" CACHE PATH "Local GoogleTest source tree (skips the download)")
  set(RSE_GTEST_TAG "v1.15.2" CACHE STRING "GoogleTest release tag used by FetchContent")

  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  set(BUILD_GMOCK    OFF CACHE BOOL "" FORCE)

  if(RSE_GTEST_SOURCE_DIR)
    message(STATUS "RSE: using local GoogleTest at '${RSE_GTEST_SOURCE_DIR}'")
    add_subdirectory("${RSE_GTEST_SOURCE_DIR}" "${CMAKE_BINARY_DIR}/_deps/googletest-build")
  else()
    message(STATUS "RSE: fetching GoogleTest ${RSE_GTEST_TAG}")
    FetchContent_Declare(googletest
      GIT_REPOSITORY https://github.com/google/googletest.git
      GIT_TAG        ${RSE_GTEST_TAG}
      GIT_SHALLOW    TRUE)
    FetchContent_MakeAvailable(googletest)
  endif()

  if(TARGET GTest::gtest)
    set(RSE_GTEST_AVAILABLE TRUE)
    message(STATUS "RSE: GoogleTest available")
  else()
    message(WARNING "RSE: GoogleTest target not found - unit tests will be skipped")
  endif()
endif()

# ---------------------------------------------------------------------------
# System OpenGL
# ---------------------------------------------------------------------------
find_package(OpenGL REQUIRED)
if(OPENGL_FOUND)
  message(STATUS "RSE: OpenGL found (${OPENGL_gl_LIBRARY})")
endif()

# ---------------------------------------------------------------------------
# Runtime DLL collection + deployment helpers
# ---------------------------------------------------------------------------
set(RSE_RUNTIME_DLLS "")
foreach(_rse_dll_dir
    "${RSE_TP}/Glew/bin/${ARCHITECTURE}"
    "${RSE_TP}/SDL2/bin/${ARCHITECTURE}"
    "${RSE_TP}/SDL2_image/bin/${ARCHITECTURE}"
    "${RSE_TP}/SDL2_mixer/bin/${ARCHITECTURE}"
    "${RSE_TP}/SDL2_ttf/bin/${ARCHITECTURE}")
  if(EXISTS "${_rse_dll_dir}")
    file(GLOB _rse_dlls "${_rse_dll_dir}/*.dll")
    list(APPEND RSE_RUNTIME_DLLS ${_rse_dlls})
  endif()
endforeach()
list(REMOVE_DUPLICATES RSE_RUNTIME_DLLS)
set(RSE_RUNTIME_DLLS "${RSE_RUNTIME_DLLS}" CACHE INTERNAL "Runtime DLLs copied next to RSE executables")

# ---------------------------------------------------------------------------
# Deployment
# ---------------------------------------------------------------------------
# Every executable shares the same output directory (bin/$<CONFIG>), so DLLs and
# Resources must be deployed exactly once -- otherwise Ninja runs the per-target
# copy commands in parallel and they race on the same destination files.
#
# A single `RSE_Deploy` target does the work; executables simply depend on it.
# It is created in the root CMakeLists.txt (see cmake/Deploy.cmake) because it
# also needs the list of per-project Resources directories.
function(rse_deploy target)
  set_target_properties(${target} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${target}>")
  if(TARGET RSE_Deploy)
    add_dependencies(${target} RSE_Deploy)
  endif()
endfunction()
