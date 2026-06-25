
set(SKIA_NIM_DUILIB ON)

add_subdirectory(zlib)

# 设置 ZLIB 路径，供 libpng 使用
set(ZLIB_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/zlib" CACHE PATH "ZLIB root directory" FORCE)
set(ZLIB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/zlib" CACHE PATH "ZLIB include directory" FORCE)
set(ZLIB_LIBRARY "${CMAKE_BINARY_DIR}/third_party/zlib/${CMAKE_CFG_INTDIR}/zlibstatic.lib" CACHE FILEPATH "ZLIB library" FORCE)

add_subdirectory(cximage)
add_subdirectory(libpng)
add_subdirectory(libwebp)
add_subdirectory(skia)


set(DUILIB_CEF_109 ON)
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
if(DUILIB_CEF_109)
add_subdirectory(libcef/libcef_win_109)
else()
add_subdirectory(libcef/libcef_win)
endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
add_subdirectory(libcef_linux)
elseif(APPLE)
add_subdirectory(libcef_macos)
elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
else()
    message(WARNING "Unknown OS: ${CMAKE_SYSTEM_NAME}")
endif()
