# cmake/Deploy.cmake
# Creates the single shared deployment target that puts the third-party runtime
# DLLs and every project's Resources/ folder next to the executables.
#
# Expected to be included from the root CMakeLists.txt after RSE_RESOURCE_DIRS
# has been filled in, e.g.:
#
#   set(RSE_RESOURCE_DIRS "")
#   if(RSE_BUILD_EDITOR)
#     list(APPEND RSE_RESOURCE_DIRS "${CMAKE_SOURCE_DIR}/Editor/Resources")
#   endif()
#   include(Deploy)

set(_rse_deploy_dest "${CMAKE_BINARY_DIR}/bin/$<CONFIG>")

set(_rse_deploy_cmds "")
list(APPEND _rse_deploy_cmds
  COMMAND "${CMAKE_COMMAND}" -E make_directory "${_rse_deploy_dest}")

foreach(_rse_res_dir ${RSE_RESOURCE_DIRS})
  if(EXISTS "${_rse_res_dir}")
    list(APPEND _rse_deploy_cmds
      COMMAND "${CMAKE_COMMAND}" -E copy_directory "${_rse_res_dir}" "${_rse_deploy_dest}/Resources")
  endif()
endforeach()

if(WIN32 AND RSE_RUNTIME_DLLS)
  list(APPEND _rse_deploy_cmds
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${RSE_RUNTIME_DLLS} "${_rse_deploy_dest}")
endif()

add_custom_target(RSE_Deploy ALL
  ${_rse_deploy_cmds}
  COMMENT "Deploying runtime DLLs and resources"
  COMMAND_EXPAND_LISTS
  VERBATIM)
