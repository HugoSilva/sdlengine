
cmake_minimum_required(VERSION 3.30.0)

# Enable Hot Reload for MSVC compilers if supported.
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project("RSEngine")

# Include sub-projects.
add_subdirectory ("Engine")
add_subdirectory ("Editor")
add_subdirectory ("Thirdparty/imgui")

# TODO - enable full dependency build system
#add_subdirectory ("Thirdparty/SDL")
#add_subdirectory ("Thirdparty/SDL_image")