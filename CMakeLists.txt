cmake_minimum_required(VERSION 3.10)

project(nim_duilib)

# Disable vcpkg integration to avoid conflicting with duilib's own zlib
set(CMAKE_VS_GLOBALS "VcpkgEnabled=false;VcpkgAutoLink=false")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if(POLICY CMP0177)
    cmake_policy(SET CMP0177 OLD)
endif()

function(force_set_folder_recursive dir folder_name)
    get_property(targets DIRECTORY "${dir}" PROPERTY BUILDSYSTEM_TARGETS)
    foreach(target IN LISTS targets)
        set_target_properties(${target} PROPERTIES FOLDER "${folder_name}")
    endforeach()
    get_property(subdirs DIRECTORY "${dir}" PROPERTY SUBDIRECTORIES)
    foreach(subdir IN LISTS subdirs)
        force_set_folder_recursive("${subdir}" "${folder_name}")
    endforeach()
endfunction()

add_compile_options("$<$<C_COMPILER_ID:MSVC>:/utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/utf-8>")

add_definitions(-DUNICODE -D_UNICODE)

option(USE_VCPKG "Use vcpkg for dependencies" OFF)
set(VCPKG_TARGET_TRIPLET "" CACHE STRING "Clearing vcpkg target triplet")
set(CMAKE_TOOLCHAIN_FILE "" CACHE PATH "Clearing toolchain file")

add_subdirectory(duilib)
add_subdirectory(examples)

force_set_folder_recursive("${CMAKE_CURRENT_SOURCE_DIR}/examples" "examples")
