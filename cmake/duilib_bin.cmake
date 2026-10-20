if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_CURRENT_BINARY_DIR)
  message(FATAL_ERROR "Prevented in-tree build. Please create a build directory outside of the source code and run \"cmake -S ${CMAKE_SOURCE_DIR} -B .\" from there")
endif()

#启用C++20
set(CMAKE_CXX_STANDARD 20) # C++20
set(CMAKE_CXX_STANDARD_REQUIRED ON) # C++20

#输出基本信息
message(STATUS "PROJECT_NAME: ${PROJECT_NAME}")
message(STATUS "C compiler  : ${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")

#设置项目的include路径（duilib）
# 说明：先收集到列表，等可执行目标创建后，用 target_include_directories/target_link_directories
#       应用到目标上（CM-17：避免目录级的全局设置影响其它目标）
list(APPEND DUILIB_TARGET_INCLUDE_DIRS
    "${DUILIB_ROOT}"
    "${DUILIB_PROJECT_SRC_DIR}"
)

#设置项目的link路径
list(APPEND DUILIB_TARGET_LINK_DIRS
    "${DUILIB_LIB_PATH}"        #duilib库所在路径
    "${DUILIB_SKIA_LIB_PATH}"   #skia库所在路径
)

if(DUILIB_ENABLE_SDL)
    list(APPEND DUILIB_TARGET_LINK_DIRS "${DUILIB_SDL_LIB_PATH}")  #SDL库所在路径
endif()

#设置可执行文件的输出目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${DUILIB_BIN_PATH}")

#添加源码，保存在变量SRC_FILES中
aux_source_directory(${DUILIB_PROJECT_SRC_DIR} SRC_FILES)

#追加子目录的源代码文件
if(DUILIB_SRC_SUB_DIRS)
    foreach(ITEM IN LISTS DUILIB_SRC_SUB_DIRS)
        # 逐个子目录添加
        aux_source_directory("${DUILIB_PROJECT_SRC_DIR}/${ITEM}" SUB_DIR_SRC_FILES)
        list(APPEND SRC_FILES ${SUB_DIR_SRC_FILES})  # 合并列表
    endforeach()
endif()

# 把上面收集的include/link目录应用到已创建的可执行目标上（CM-17）
# 需在各平台的 add_executable() 之后调用
function(duilib_apply_target_directories)
    target_include_directories(${PROJECT_NAME} PRIVATE ${DUILIB_TARGET_INCLUDE_DIRS})
    if(DUILIB_TARGET_LINK_DIRS)
        target_link_directories(${PROJECT_NAME} PRIVATE ${DUILIB_TARGET_LINK_DIRS})
    endif()
endfunction()

#按平台，分开实现
if(DUILIB_OS_WINDOWS)
    # Winows平台
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_bin_windows.cmake") 
elseif(DUILIB_OS_LINUX)
    # Linux平台
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_bin_linux.cmake") 
elseif(DUILIB_OS_MACOS)
    # MacOS平台
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_bin_macos.cmake") 
elseif(DUILIB_OS_FREEBSD)
    # FreeBSD平台
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_bin_freebsd.cmake") 
else()
    message(FATAL_ERROR "Unknown OS!")
endif()
