
# Add source to this project's executable.

file(GLOB_RECURSE ENGINE_FILES "Engine/*.cpp" "Engine/*.h" "Engine/*.hpp" "Tests/*.cpp" "Tests/*.h")

if(DEFINED EMSCRIPTEN)
  set(GL3_FILES)
  set(GL3_HEADER_PATH)
  file(REAL_PATH "Engine/Utils/imgui_impl_sdl_gl3.h" GL3_HEADER_PATH)
  set(GL3_SOURCE_PATH)
  file(REAL_PATH "Engine/Utils/imgui_impl_sdl_gl3.cpp" GL3_SOURCE_PATH)
  list(APPEND GL3_FILES ${GL3_HEADER_PATH} ${GL3_SOURCE_PATH})
  list(REMOVE_ITEM ENGINE_FILES ${GL3_FILES})
else()
  set(GLES2_FILES)
  set(GLES2_HEADER_PATH)
  file(REAL_PATH "Engine/Utils/imgui_impl_sdl_gles2.h" GLES2_HEADER_PATH)
  set(GLES2_SOURCE_PATH)
  file(REAL_PATH "Engine/Utils/imgui_impl_sdl_gles2.cpp" GLES2_SOURCE_PATH)
  list(APPEND GLES2_FILES ${GLES2_HEADER_PATH} ${GLES2_SOURCE_PATH})
  list(REMOVE_ITEM ENGINE_FILES ${GLES2_FILES})
endif()

add_library(Engine ${ENGINE_FILES})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET Engine PROPERTY CXX_STANDARD 20)
endif()

# TODO: Add tests and install targets if needed.
target_include_directories(Engine PUBLIC "../Thirdparty/Glew/include")
target_include_directories(Engine PUBLIC "../Thirdparty/SDL2/include")
target_include_directories(Engine PUBLIC "../Thirdparty/SDL2_image/include")
target_include_directories(Engine PUBLIC "../Thirdparty/SDL2_mixer/include")
target_include_directories(Engine PUBLIC "../Thirdparty/SDL2_ttf/include")
target_include_directories(Engine PUBLIC "../Thirdparty/glm/include")
target_include_directories(Engine PUBLIC "../Thirdparty/imgui/include")
target_include_directories(Engine PUBLIC "../Thirdparty/Box2D/include")
target_include_directories(Engine PUBLIC "../Thirdparty/cereal")
target_include_directories(Engine PUBLIC "../Thirdparty/rapidxml")
target_include_directories(Engine PUBLIC "../Thirdparty/entt")

add_library(Box2D SHARED IMPORTED)
set_target_properties(Box2D PROPERTIES
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/Thirdparty/Box2D/lib/x64/Box2D_d.lib"
)

add_library(Glew SHARED IMPORTED)
set_target_properties(Glew PROPERTIES 
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Thirdparty/Glew/bin/x64/glew32.dll"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/Thirdparty/Glew/lib/x64/glew32.lib"
)

add_library(SDL2 SHARED IMPORTED)
set_target_properties(SDL2 PROPERTIES 
  IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2/bin/x64/SDL2.dll"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2/lib/x64/SDL2.lib"
)

add_library(SDL2MAIN SHARED IMPORTED)
set_target_properties(SDL2MAIN PROPERTIES
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2/lib/x64/SDL2main.lib"
)

add_library(SDL2_Image SHARED IMPORTED)
set(SDL2_Image_LIBS
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_image/bin/x64/SDL2_image.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_image/bin/x64/libjpeg-9.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_image/bin/x64/libpng16-16.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_image/bin/x64/libtiff-5.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_image/bin/x64/libwebp-7.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_image/bin/x64/zlib1.dll"
)
set_target_properties(SDL2_Image PROPERTIES 
  IMPORTED_LOCATION "${SDL2_Image_LIBS}"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_image/lib/x64/SDL2_image.lib"
)

add_library(SDL2_Mixer SHARED IMPORTED)
set(SDL2_Mixer_LIBS
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_mixer/bin/x64/SDL2_mixer.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_mixer/bin/x64/libFLAC-8.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_mixer/bin/x64/libmodplug-1.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_mixer/bin/x64/libmpg123-0.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_mixer/bin/x64/libogg-0.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_mixer/bin/x64/libvorbis-0.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_mixer/bin/x64/libvorbisfile-3.dll"
)
set_target_properties(SDL2_Mixer PROPERTIES 
  IMPORTED_LOCATION "${SDL2_Mixer_LIBS}"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_mixer/lib/x64/SDL2_mixer.lib"
)

add_library(SDL2_TTF SHARED IMPORTED)
set(SDL2_TTF_LIBS
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_ttf/bin/x64/SDL2_ttf.dll"
  "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_ttf/bin/x64/libfreetype-6.dll"
)
set_target_properties(SDL2_TTF PROPERTIES 
  IMPORTED_LOCATION "${SDL2_TTF_LIBS}"
  IMPORTED_IMPLIB "${CMAKE_SOURCE_DIR}/Thirdparty/SDL2_ttf/lib/x64/SDL2_ttf.lib"
)

target_link_libraries(Engine PUBLIC Box2D Glew Imgui SDL2 SDL2MAIN SDL2_Image SDL2_Mixer SDL2_TTF)