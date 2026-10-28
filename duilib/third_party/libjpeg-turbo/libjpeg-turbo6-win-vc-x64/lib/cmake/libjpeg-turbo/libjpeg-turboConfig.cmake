
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was Config.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

set(WITH_SYSTEM_SPNG 0)
set(WITH_SYSTEM_ZLIB 0)

if(WITH_SYSTEM_SPNG OR WITH_SYSTEM_ZLIB)
  include(CMakeFindDependencyMacro)
  if(WITH_SYSTEM_SPNG)
    set(SPNG_LIBRARY )
    if(SPNG_LIBRARY MATCHES "PkgConfig::")
      find_dependency(PkgConfig)
      if(PkgConfig_FOUND)
        pkg_check_modules(spng REQUIRED spng IMPORTED_TARGET)
      endif()
    else()
      find_dependency(SPNG)
    endif()
  else()
    find_dependency(ZLIB)
  endif()
endif()

include("${CMAKE_CURRENT_LIST_DIR}/libjpeg-turboTargets.cmake")
check_required_components("libjpeg-turbo")
