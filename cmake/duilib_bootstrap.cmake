# Ninja 生成器自举（cmake 脚本/函数，供根 CMakeLists 在 project() 之前调用）
#
# 某些 Visual Studio 安装不含 ninja（可选组件），而 Skia 补丁归档自带 Windows 版
# bin/ninja.exe。系统里找不到 ninja 时按下列顺序取用：
#   1. -DDUILIB_NINJA_EXECUTABLE=<path> / 缓存中已有的 CMAKE_MAKE_PROGRAM；
#   2. 已解压的 skia/bin/ninja.exe（依赖准备阶段的产物）；
#   3. PATH 里的 ninja；
#   4. 从补丁归档解压 bin/ninja.exe 到构建树（不写 skia 源码树，避免影响尚未
#      clone 的目录；build.py clean 会一并清掉 out/ 下的副本）。
# 找到后必须在 project() 之前写入 CMAKE_MAKE_PROGRAM 缓存，Ninja 生成器才会采用；
# 同时写入 DUILIB_NINJA_EXECUTABLE，Skia 编译直接复用同一个 ninja。
#
# 只处理 Ninja 生成器：VS 生成器配置阶段不需要 ninja；Skia 编译所需的 ninja 由
# 依赖脚本在解压补丁后自行找到（顺序见 cmake/duilib_deps_run.cmake）。
#
# 可传入：
#   -DDUILIB_SKIA_ROOT=<dir>
#   -DDUILIB_NINJA_EXECUTABLE=<path>

cmake_minimum_required(VERSION 3.24)

# 归档里是否有指定条目（先列条目，避免 file(ARCHIVE_EXTRACT) 因条目缺失直接报错）
function(_duilib_archive_has_entry archive entry out_var)
    set(_has FALSE)
    if(EXISTS "${archive}")
        execute_process(COMMAND "${CMAKE_COMMAND}" -E tar tf "${archive}"
                        OUTPUT_VARIABLE _entries
                        ERROR_QUIET
                        RESULT_VARIABLE _list_rc)
        if(_list_rc EQUAL 0 AND _entries MATCHES "(^|\r?\n)(\\./)?${entry}(\r?\n|$)")
            set(_has TRUE)
        endif()
    endif()
    set(${out_var} "${_has}" PARENT_SCOPE)
endfunction()

# 记录选用的 ninja：CMAKE_MAKE_PROGRAM 供 Ninja 生成器使用，DUILIB_NINJA_EXECUTABLE
# 供阶段 A 预检与 Skia 依赖编译复用（同一个可执行文件）。
function(_duilib_bootstrap_use_ninja ninja source)
    if(NOT EXISTS "${ninja}")
        return()
    endif()
    set(CMAKE_MAKE_PROGRAM "${ninja}" CACHE FILEPATH
        "Ninja executable (bundled with the nim_duilib toolchain)" FORCE)
    set(DUILIB_NINJA_EXECUTABLE "${ninja}" CACHE FILEPATH
        "Override the ninja executable" FORCE)
    message(STATUS "[OK]    ninja ${ninja} (${source})")
endfunction()

function(duilib_bootstrap_ninja patch_dir patch_zip)
    if(NOT CMAKE_GENERATOR MATCHES "Ninja")
        return()
    endif()
    # 已有可用的 CMAKE_MAKE_PROGRAM（上一次配置写入的缓存或命令行传入）
    if(NOT "${CMAKE_MAKE_PROGRAM}" STREQUAL "" AND EXISTS "${CMAKE_MAKE_PROGRAM}")
        return()
    endif()

    set(_ninja "")
    set(_source "")
    if(NOT "${DUILIB_NINJA_EXECUTABLE}" STREQUAL "" AND EXISTS "${DUILIB_NINJA_EXECUTABLE}")
        set(_ninja "${DUILIB_NINJA_EXECUTABLE}")
        set(_source "DUILIB_NINJA_EXECUTABLE")
    elseif(WIN32 AND EXISTS "${DUILIB_SKIA_ROOT}/bin/ninja.exe")
        set(_ninja "${DUILIB_SKIA_ROOT}/bin/ninja.exe")
        set(_source "skia/bin")
    elseif(NOT WIN32 AND EXISTS "${DUILIB_SKIA_ROOT}/bin/ninja")
        set(_ninja "${DUILIB_SKIA_ROOT}/bin/ninja")
        set(_source "skia/bin")
    else()
        # 临时变量：预先置空会让 find_program 跳过搜索
        find_program(_duilib_ninja_hit NAMES ninja PATHS ENV PATH NO_DEFAULT_PATH NO_CACHE)
        if(_duilib_ninja_hit)
            set(_ninja "${_duilib_ninja_hit}")
            set(_source "PATH")
        endif()
    endif()
    if(NOT "${_ninja}" STREQUAL "")
        _duilib_bootstrap_use_ninja("${_ninja}" "${_source}")
        return()
    endif()

    # 系统没有 ninja：归档自带的是 Windows 程序，只在 Windows 上尝试
    if(NOT WIN32 OR "${patch_zip}" STREQUAL "")
        return()
    endif()
    set(_archive "${patch_dir}/${patch_zip}")
    _duilib_archive_has_entry("${_archive}" "bin/ninja\\.exe" _has)
    if(NOT _has)
        return()
    endif()

    set(_tools "${CMAKE_CURRENT_BINARY_DIR}/.duilib-tools")
    if(NOT EXISTS "${_tools}/bin/ninja.exe")
        file(ARCHIVE_EXTRACT INPUT "${_archive}" DESTINATION "${_tools}" PATTERNS "bin/ninja.exe")
    endif()
    if(NOT EXISTS "${_tools}/bin/ninja.exe")
        return()
    endif()

    _duilib_bootstrap_use_ninja("${_tools}/bin/ninja.exe" "from ${patch_zip}")
endfunction()
