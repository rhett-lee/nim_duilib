# 依赖准备脚本（cmake -P）
#
# 由 cmake/duilib_deps.cmake 的 duilib_deps 目标调用，也被 build.py 复用。
# 所有输入通过 -D 传入；不依赖已生成的构建树。

cmake_minimum_required(VERSION 3.24)
include("${CMAKE_CURRENT_LIST_DIR}/duilib_i18n.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/duilib_skia_toolchain.cmake")

# ---------------------------------------------------------------------------
# 参数校验
# ---------------------------------------------------------------------------
if("${DUILIB_DEPS_MODE}" STREQUAL "")
    set(DUILIB_DEPS_MODE "prepare")
endif()
if("${DUILIB_CONFIG}" STREQUAL "")
    set(DUILIB_CONFIG "Release")
endif()
if(DUILIB_CONFIG STREQUAL "Debug")
    set(DUILIB_BUILD_TYPE "debug")
else()
    set(DUILIB_BUILD_TYPE "release")
endif()
# 多配置生成器（Visual Studio）在配置期无法确定目标配置，依赖按 Release 处理。
if("${DUILIB_MULTI_CONFIG}" STREQUAL "")
    set(DUILIB_MULTI_CONFIG OFF)
endif()
if("${DUILIB_DEPS_JOBS}" STREQUAL "")
    set(DUILIB_DEPS_JOBS 6)
endif()
if("${DUILIB_DEPS_LOCK_TIMEOUT}" STREQUAL "")
    set(DUILIB_DEPS_LOCK_TIMEOUT 5)
endif()

if(NOT EXISTS "${DUILIB_VERSIONS_FILE}")
    duilib_error("deps.versions_invalid" "deps.versions_invalid_hint")
endif()
file(READ "${DUILIB_VERSIONS_FILE}" DUILIB_VERSIONS_JSON)

# 源码仓库地址优先用 -D 传入的值（可换成镜像），否则取自版本清单
if("${DUILIB_SKIA_REPO}" STREQUAL "")
    string(JSON DUILIB_SKIA_REPO ERROR_VARIABLE _duilib_json_err
           GET "${DUILIB_VERSIONS_JSON}" skia repo)
    if(_duilib_json_err OR DUILIB_SKIA_REPO STREQUAL "")
        duilib_error("deps.versions_invalid" "deps.versions_invalid_hint")
    endif()
endif()
if("${DUILIB_SDL_REPO}" STREQUAL "")
    string(JSON DUILIB_SDL_REPO ERROR_VARIABLE _duilib_json_err
           GET "${DUILIB_VERSIONS_JSON}" sdl repo)
    if(_duilib_json_err OR DUILIB_SDL_REPO STREQUAL "")
        duilib_error("deps.versions_invalid" "deps.versions_invalid_hint")
    endif()
endif()
if("${DUILIB_SKIA_PATCH_DIR}" STREQUAL "")
    # 补丁归档随仓库分发，目录由清单 skia.patchDir 指定（缺省 build/skia_patch）
    string(JSON _duilib_patch_dir ERROR_VARIABLE _duilib_json_err
           GET "${DUILIB_VERSIONS_JSON}" skia patchDir)
    if(_duilib_json_err OR _duilib_patch_dir STREQUAL "")
        set(_duilib_patch_dir "build/skia_patch")
    endif()
    if("${DUILIB_ROOT}" STREQUAL "")
        get_filename_component(DUILIB_ROOT "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
    endif()
    get_filename_component(DUILIB_SKIA_PATCH_DIR
        "${DUILIB_ROOT}/${_duilib_patch_dir}" ABSOLUTE)
endif()

# 统一重试参数：根工程会从清单读取后通过 -D 传入；脚本直接运行时在此兜底
if("${DUILIB_DOWNLOAD_RETRIES}" STREQUAL "" OR "${DUILIB_DOWNLOAD_RETRY_DELAY}" STREQUAL "")
    string(JSON _duilib_retry_max ERROR_VARIABLE _duilib_json_err
           GET "${DUILIB_VERSIONS_JSON}" retry maxAttempts)
    if(_duilib_json_err)
        set(_duilib_retry_max 0)
    endif()
    string(JSON _duilib_retry_delay ERROR_VARIABLE _duilib_json_err
           GET "${DUILIB_VERSIONS_JSON}" retry delaySeconds)
    if(_duilib_json_err)
        set(_duilib_retry_delay 20)
    endif()
    if("${DUILIB_DOWNLOAD_RETRIES}" STREQUAL "")
        set(DUILIB_DOWNLOAD_RETRIES "${_duilib_retry_max}")
    endif()
    if("${DUILIB_DOWNLOAD_RETRY_DELAY}" STREQUAL "")
        set(DUILIB_DOWNLOAD_RETRY_DELAY "${_duilib_retry_delay}")
    endif()
endif()

# 依赖根目录与锁（并发 deps 只允许一个进程写入）
get_filename_component(DUILIB_DEPS_ROOT "${DUILIB_DEPS_ROOT}" ABSOLUTE)
file(MAKE_DIRECTORY "${DUILIB_DEPS_ROOT}")
set(_duilib_lock_file "${DUILIB_DEPS_ROOT}/.duilib_deps.lock")
file(LOCK "${_duilib_lock_file}" GUARD PROCESS TIMEOUT "${DUILIB_DEPS_LOCK_TIMEOUT}"
     RESULT_VARIABLE _duilib_lock_rc)
if(NOT _duilib_lock_rc EQUAL 0)
    duilib_error("deps.lock_failed" "deps.lock_failed_hint")
endif()

include("${CMAKE_CURRENT_LIST_DIR}/duilib_download.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/duilib_cef.cmake")

# ---------------------------------------------------------------------------
# Skia
# ---------------------------------------------------------------------------
function(_duilib_skia_candidates out_var)
    set(_candidates "")
    if(DUILIB_DEPS_PLATFORM STREQUAL "windows")
        if(NOT DUILIB_MINGW)
            # 与 duilib_common.cmake 保持一致：/MD 构建使用 .mdd（Debug）/.md（Release）
            if(DUILIB_BUILD_TYPE STREQUAL "debug")
                if(DUILIB_MD)
                    list(APPEND _candidates "llvm.${DUILIB_ARCH}.debug.mdd")
                else()
                    list(APPEND _candidates "llvm.${DUILIB_ARCH}.debug")
                endif()
            else()
                if(DUILIB_MD)
                    list(APPEND _candidates "llvm.${DUILIB_ARCH}.release.md")
                else()
                    list(APPEND _candidates "llvm.${DUILIB_ARCH}.release")
                endif()
            endif()
        else()
            if(DUILIB_COMPILER_GCC)
                if(DUILIB_MSYS2_ENV STREQUAL "mingw64")
                    list(APPEND _candidates "gcc.msvcrt.${DUILIB_ARCH}.release")
                endif()
            endif()
            if(DUILIB_MINGW_NAME STREQUAL "")
                set(_prefixes "" "mingw64-")
            else()
                set(_prefixes "mingw64-" "")
            endif()
            if(DUILIB_COMPILER_GCC)
                set(_compilers "gcc")
            else()
                set(_compilers "llvm")
            endif()
            foreach(_prefix IN LISTS _prefixes)
                foreach(_compiler IN LISTS _compilers)
                    if(DUILIB_BUILD_TYPE STREQUAL "debug")
                        list(APPEND _candidates
                            "${_prefix}${_compiler}.${DUILIB_ARCH}.debug"
                            "${_prefix}${_compiler}.${DUILIB_ARCH}.release")
                    else()
                        list(APPEND _candidates
                            "${_prefix}${_compiler}.${DUILIB_ARCH}.release")
                    endif()
                endforeach()
            endforeach()
        endif()
    else()
        duilib_skia_toolchain(_duilib_skia_variant _duilib_skia_cc _duilib_skia_cxx)
        list(APPEND _candidates "${_duilib_skia_variant}.${DUILIB_ARCH}.release")
    endif()
    set(${out_var} "${_candidates}" PARENT_SCOPE)
endfunction()

function(_duilib_skia_lib_name out_var)
    if(DUILIB_DEPS_PLATFORM STREQUAL "windows" AND NOT DUILIB_MINGW)
        set(${out_var} "skia.lib" PARENT_SCOPE)
    else()
        set(${out_var} "libskia.a" PARENT_SCOPE)
    endif()
endfunction()

function(_duilib_skia_stamp_value out_var)
    set(${out_var}
        "${DUILIB_SKIA_REF}|${DUILIB_SKIA_REF_VALUE}|${DUILIB_CRT_ID}|${DUILIB_ARCH}"
        PARENT_SCOPE)
endfunction()

function(_duilib_write_crt dir)
    file(WRITE "${dir}/.duilib-crt" "${DUILIB_CRT_ID}")
endfunction()

function(_duilib_git_head repo out_var)
    execute_process(COMMAND git -C "${repo}" rev-parse HEAD
                    OUTPUT_VARIABLE _head OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        set(_head "")
    endif()
    set(${out_var} "${_head}" PARENT_SCOPE)
endfunction()

# 把 ref 解析成提交哈希；本地没有对应对象时返回空串，由调用方决定是否 fetch。
function(_duilib_git_resolve repo spec out_commit)
    execute_process(COMMAND git -C "${repo}" rev-parse --verify --quiet "${spec}"
                    OUTPUT_VARIABLE _commit OUTPUT_STRIP_TRAILING_WHITESPACE
                    ERROR_QUIET RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        set(_commit "")
    endif()
    set(${out_commit} "${_commit}" PARENT_SCOPE)
endfunction()

function(_duilib_find_gn out_var)
    set(_gn "")
    if(NOT "${DUILIB_GN_EXECUTABLE}" STREQUAL "" AND EXISTS "${DUILIB_GN_EXECUTABLE}")
        set(_gn "${DUILIB_GN_EXECUTABLE}")
    elseif(WIN32 AND EXISTS "${DUILIB_SKIA_ROOT}/bin/gn.exe")
        set(_gn "${DUILIB_SKIA_ROOT}/bin/gn.exe")
    elseif(NOT WIN32 AND EXISTS "${DUILIB_SKIA_ROOT}/bin/gn")
        set(_gn "${DUILIB_SKIA_ROOT}/bin/gn")
    else()
        # 不能用 _gn 作为结果变量：它已在上方被置空，find_program 会认为变量
        # 已有值而跳过搜索（脚本模式下必须显式 PATHS ENV PATH）。
        find_program(_duilib_gn_hit gn PATHS ENV PATH NO_DEFAULT_PATH NO_CACHE)
        if(_duilib_gn_hit)
            set(_gn "${_duilib_gn_hit}")
        endif()
    endif()
    set(${out_var} "${_gn}" PARENT_SCOPE)
endfunction()

function(_duilib_find_ninja out_var)
    set(_ninja "")
    if(NOT "${DUILIB_NINJA_EXECUTABLE}" STREQUAL "" AND EXISTS "${DUILIB_NINJA_EXECUTABLE}")
        set(_ninja "${DUILIB_NINJA_EXECUTABLE}")
    elseif(WIN32 AND EXISTS "${DUILIB_SKIA_ROOT}/bin/ninja.exe")
        set(_ninja "${DUILIB_SKIA_ROOT}/bin/ninja.exe")
    elseif(NOT WIN32 AND EXISTS "${DUILIB_SKIA_ROOT}/bin/ninja")
        set(_ninja "${DUILIB_SKIA_ROOT}/bin/ninja")
    else()
        # 同 _duilib_find_gn：用临时变量，避免预先置空导致 find_program 跳过搜索。
        find_program(_duilib_ninja_hit ninja PATHS ENV PATH NO_DEFAULT_PATH NO_CACHE)
        if(_duilib_ninja_hit)
            set(_ninja "${_duilib_ninja_hit}")
        endif()
    endif()
    set(${out_var} "${_ninja}" PARENT_SCOPE)
endfunction()

function(_duilib_find_python3 out_var)
    # Skia 的构建脚本固定调用 python3（不认 python / py -3），因此这里必须查
    # python3 本身，不能复用预检里对 Python 3 的宽松判断。
    # 未命中时 find_program 会写入 <var>-NOTFOUND（非空），需显式归一为空串。
    find_program(_duilib_python3_hit python3 PATHS ENV PATH NO_DEFAULT_PATH NO_CACHE)
    if(_duilib_python3_hit)
        set(${out_var} "${_duilib_python3_hit}" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

function(_duilib_skia_gn_args out_var variant)
    set(_common
        "is_trivial_abi=false"
        "is_official_build=true"
        "skia_use_libwebp_encode=false"
        "skia_use_libwebp_decode=false"
        "skia_use_libpng_encode=false"
        "skia_use_libpng_decode=false"
        "skia_use_zlib=false"
        "skia_use_libjpeg_turbo_encode=false"
        "skia_use_libjpeg_turbo_decode=false"
        "skia_enable_fontmgr_win_gdi=false"
        "skia_use_icu=false"
        "skia_use_expat=false"
        "skia_use_xps=false"
        "skia_enable_pdf=false"
        "skia_use_wuffs=false"
        "skia_enable_svg=true"
        "skia_use_expat=true"
        "skia_use_system_expat=false"
        "skia_use_partition_alloc=false"
        "is_debug=false")

    set(_target_cpu "${DUILIB_ARCH}")
    if(_target_cpu STREQUAL "x86")
        set(_target_cpu "x86")
    endif()

    if(DUILIB_DEPS_PLATFORM STREQUAL "windows")
        if(NOT DUILIB_MINGW)
            if(DUILIB_CONFIG STREQUAL "Debug")
                if(DUILIB_MD)
                    set(_runtime "/MDd")
                else()
                    set(_runtime "/MTd")
                endif()
            else()
                if(DUILIB_MD)
                    set(_runtime "/MD")
                else()
                    set(_runtime "/MT")
                endif()
            endif()
            set(_args "target_cpu=\"${_target_cpu}\" cc=\"clang\" cxx=\"clang++\" clang_win=\"${DUILIB_LLVM_ROOT}\"")
        else()
            if(DUILIB_COMPILER_GCC)
                set(_args "target_cpu=\"${_target_cpu}\" cc=\"gcc\" cxx=\"g++\"")
            else()
                set(_args "target_cpu=\"${_target_cpu}\" cc=\"clang\" cxx=\"clang++\"")
            endif()
            set(_runtime "")
        endif()
    elseif(DUILIB_DEPS_PLATFORM STREQUAL "linux")
        duilib_skia_toolchain(_skia_variant _skia_cc _skia_cxx)
        set(_args "target_cpu=\"${_target_cpu}\" cc=\"${_skia_cc}\" cxx=\"${_skia_cxx}\" skia_enable_fontmgr_fontconfig=true skia_use_freetype=true")
        set(_runtime "")
    elseif(DUILIB_DEPS_PLATFORM STREQUAL "freebsd")
        duilib_skia_toolchain(_skia_variant _skia_cc _skia_cxx)
        set(_args "target_cpu=\"${_target_cpu}\" cc=\"${_skia_cc}\" cxx=\"${_skia_cxx}\" ar=\"llvm-ar\" skia_enable_fontmgr_fontconfig=true skia_use_freetype=true extra_ldflags=[\"-L/usr/local/lib\"]")
        set(_runtime "")
    else()
        duilib_skia_toolchain(_skia_variant _skia_cc _skia_cxx)
        set(_args "target_cpu=\"${_target_cpu}\" cc=\"${_skia_cc}\" cxx=\"${_skia_cxx}\"")
        set(_runtime "")
    endif()

    set(_cflags "extra_cflags=[\"-DSK_DISABLE_LEGACY_PNG_WRITEBUFFER\"")
    if(NOT _runtime STREQUAL "")
        string(APPEND _cflags ",\"${_runtime}\"")
    endif()
    if(DUILIB_DEPS_PLATFORM STREQUAL "linux" OR DUILIB_DEPS_PLATFORM STREQUAL "freebsd")
        string(APPEND _cflags ",\"-I/usr/local/include/freetype2\",\"-I/usr/local/include\"")
    endif()
    string(APPEND _cflags "]")

    # 组装 gn --args 参数（每个赋值用空格分隔）
    set(_all_args "${_args}")
    foreach(_item IN LISTS _common)
        string(APPEND _all_args " ${_item}")
    endforeach()
    string(APPEND _all_args " ${_cflags}")
    set(${out_var} "${_all_args}" PARENT_SCOPE)
endfunction()

function(_duilib_clone ref_type ref_value repo dest)
    if(DUILIB_OFFLINE)
        duilib_error("deps.offline_missing" "deps.offline_missing_hint")
    endif()
    if(NOT DUILIB_DEPS_AUTO_CLONE)
        duilib_error_values("${dest} does not exist and automatic cloning is disabled"
                            "clone the repository manually or enable DUILIB_DEPS_AUTO_CLONE")
    endif()
    if(ref_type STREQUAL "commit")
        # 克隆/fetch 是网络操作，统一按清单里的 retry 参数重试
        duilib_run_with_retry(_ok "git clone ${repo}"
                              git clone --filter=blob:none --no-checkout "${repo}" "${dest}")
        if(NOT _ok)
            file(REMOVE_RECURSE "${dest}")
            duilib_error_values("git clone failed for ${repo}" "check the network/proxy settings")
        endif()
        duilib_run_with_retry(_ok "git fetch ${ref_value}"
                              git -C "${dest}" fetch --depth 1 origin "${ref_value}")
        if(NOT _ok)
            file(REMOVE_RECURSE "${dest}")
            duilib_error_values("git fetch ${ref_value} failed" "check the network/proxy settings")
        endif()
        execute_process(COMMAND git -C "${dest}" checkout "${ref_value}"
                        RESULT_VARIABLE _rc)
    else()
        duilib_run_with_retry(_ok "git clone ${repo}"
                              git clone --depth 1 --branch "${ref_value}" "${repo}" "${dest}")
        if(NOT _ok)
            file(REMOVE_RECURSE "${dest}")
            duilib_error_values("git clone failed for ${repo}" "check the network/proxy settings")
        endif()
        set(_rc 0)
    endif()
    if(NOT _rc EQUAL 0)
        duilib_error_values("git clone/checkout failed for ${repo}" "check the network/proxy settings")
    endif()
endfunction()

# 补丁归档内的 SHA1.txt 记录归档对应的 Skia 提交，用于发现 zip 与清单不配套。
function(_duilib_skia_patch_sha1 patch out_sha1)
    set(_sha1 "")
    if(EXISTS "${patch}")
        # 归档没有 SHA1.txt 时 file(ARCHIVE_EXTRACT) 会直接抛 tar 错误，
        # 这里先列出条目判断；找不到就返回空 SHA-1，由校验函数给出统一的可读错误。
        execute_process(COMMAND "${CMAKE_COMMAND}" -E tar tf "${patch}"
                        OUTPUT_VARIABLE _duilib_patch_entries
                        ERROR_QUIET
                        RESULT_VARIABLE _duilib_list_rc)
        if(_duilib_list_rc EQUAL 0 AND
           _duilib_patch_entries MATCHES "(^|\r?\n)(\\./)?SHA1\\.txt(\r?\n|$)")
            set(_probe "${CMAKE_CURRENT_BINARY_DIR}/.skia_patch_probe")
            file(REMOVE_RECURSE "${_probe}")
            file(MAKE_DIRECTORY "${_probe}")
            file(ARCHIVE_EXTRACT INPUT "${patch}" DESTINATION "${_probe}" PATTERNS "SHA1.txt")
            if(EXISTS "${_probe}/SHA1.txt")
                file(STRINGS "${_probe}/SHA1.txt" _lines LIMIT_COUNT 1)
                if(_lines)
                    string(REGEX REPLACE "^[^:]*:" "" _sha1 "${_lines}")
                    string(STRIP "${_sha1}" _sha1)
                    string(TOLOWER "${_sha1}" _sha1)
                endif()
            endif()
            file(REMOVE_RECURSE "${_probe}")
        endif()
    endif()
    set(${out_sha1} "${_sha1}" PARENT_SCOPE)
endfunction()

# 归档与清单提交不一致时说明补丁包与版本配置不配套（tag/branch ref 不校验）。
# 错误文案与 build.py 共用同一组消息键，两个入口的输出保持一致。
function(_duilib_skia_patch_verify patch)
    if(NOT "${DUILIB_SKIA_REF_TYPE}" STREQUAL "commit")
        return()
    endif()
    _duilib_skia_patch_sha1("${patch}" _sha1)
    string(TOLOWER "${DUILIB_SKIA_REF_VALUE}" _ref_value)
    duilib_t("deps.skia_patch_sha1_mismatch" _duilib_patch_what)
    duilib_t("deps.skia_patch_sha1_mismatch_hint" _duilib_patch_hint)
    if(_sha1 STREQUAL "")
        message(FATAL_ERROR
            "${_duilib_patch_what}: ${patch}\n  SHA1.txt missing\n  Hint: ${_duilib_patch_hint}")
    endif()
    if(NOT _sha1 STREQUAL _ref_value)
        message(FATAL_ERROR
            "${_duilib_patch_what}: ${patch}\n  ${_sha1} != ${_ref_value}\n  Hint: ${_duilib_patch_hint}")
    endif()
endfunction()

# 归档缺失但已有产物可复用时只告警，与 build.py 的策略保持一致。
function(_duilib_skia_warn_patch_missing patch)
    duilib_t("deps.skia_patch_missing_reuse" _duilib_patch_warn)
    message(STATUS "[WARN]  ${_duilib_patch_warn} (${patch})")
endfunction()

# 把本地依赖仓库切到清单指定的 ref：commit/tag 要求 HEAD 等于目标提交，
# branch 要求当前就在该分支上（不主动追远端新提交，避免每次全量重编依赖）。
# 本地仓库停在别的分支（如 SDL 源码停在 main）时，stamp 只记录 ref 名称会把
# “用错版本编译出来的库”当成清单版本的产物复用，因此这里必须先对齐。
# 依赖源码树由 deps 管理（Skia 的补丁从归档重新应用），切换版本前把本地修改存进
# git stash：旧版本的补丁直接改在工作区，不先保存会让 git checkout 因“本地修改
# 将被覆盖”而失败。用 stash 而不是 reset --hard / clean，避免用户手工修改（以及
# 未跟踪的辅助脚本）被删除，需要时可用 'git stash pop' 找回。
function(_duilib_git_stash_local repo label)
    # 只看已跟踪文件的修改（--untracked-files=no）：未跟踪文件不会被 checkout 覆盖，
    # 且 stash 不带 -u 时也不会被保存，无需为此提示或改动工作区。
    execute_process(COMMAND git -C "${repo}" status --porcelain --untracked-files=no
                    OUTPUT_VARIABLE _porcelain
                    ERROR_QUIET RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0 OR "${_porcelain}" STREQUAL "")
        return()
    endif()
    duilib_t("deps.git_stash_local" _what)
    string(REPLACE "{repo}" "${repo}" _what "${_what}")
    message(STATUS "${_what}")
    string(TIMESTAMP _now "%Y-%m-%d %H:%M:%S")
    # 只暂存已跟踪文件的修改：未跟踪文件（补丁自带的 gn/is_mingw.py 等、用户自己的
    # 脚本）留在工作区，不会被删除；补丁自带的部分随后由归档重新解压刷新。
    execute_process(COMMAND git -C "${repo}" stash push
                    --message "duilib-deps auto-stash ${label} ${_now}"
                    OUTPUT_VARIABLE _stash_out ERROR_VARIABLE _stash_err
                    RESULT_VARIABLE _stash_rc)
    if(NOT _stash_rc EQUAL 0)
        duilib_error_values("cannot stash local changes in ${repo}"
                            "clean the working tree manually, or re-clone the repository")
    endif()
endfunction()

function(_duilib_ensure_git_ref repo ref_type ref_value label)
    set(_stash_local FALSE)
    if(ARGC GREATER 4 AND ARGV4)
        set(_stash_local TRUE)
    endif()
    if(NOT EXISTS "${repo}/.git")
        return()
    endif()
    _duilib_git_head("${repo}" _head)
    if(ref_type STREQUAL "branch")
        execute_process(COMMAND git -C "${repo}" rev-parse --abbrev-ref HEAD
                        OUTPUT_VARIABLE _current OUTPUT_STRIP_TRAILING_WHITESPACE
                        ERROR_QUIET RESULT_VARIABLE _rc)
        if(_rc EQUAL 0 AND _current STREQUAL ref_value)
            return()
        endif()
        if(_stash_local)
            _duilib_git_stash_local("${repo}" "${label}")
        endif()
        # 本地已有该分支（或远端跟踪分支）时直接切过去，不必联网
        _duilib_git_resolve("${repo}" "refs/heads/${ref_value}^{commit}" _local)
        _duilib_git_resolve("${repo}" "refs/remotes/origin/${ref_value}^{commit}" _remote)
        if(NOT _local STREQUAL "")
            duilib_msg("deps.git_ref_switching" " (${label}: ${_current} -> ${ref_value})")
            execute_process(COMMAND git -C "${repo}" checkout "${ref_value}"
                            RESULT_VARIABLE _rc)
            if(NOT _rc EQUAL 0)
                duilib_error_values("cannot switch ${repo} to branch ${ref_value}"
                                    "tracked changes are already saved to git stash; remove the untracked files listed above (or run 'git -C ${repo} stash --include-untracked'), then retry")
            endif()
            return()
        endif()
        if(NOT _remote STREQUAL "")
            duilib_msg("deps.git_ref_switching" " (${label}: ${_current} -> ${ref_value})")
            execute_process(COMMAND git -C "${repo}" checkout -B "${ref_value}" "${_remote}"
                            RESULT_VARIABLE _rc)
            if(NOT _rc EQUAL 0)
                duilib_error_values("cannot switch ${repo} to branch ${ref_value}"
                                    "tracked changes are already saved to git stash; remove the untracked files listed above (or run 'git -C ${repo} stash --include-untracked'), then retry")
            endif()
            return()
        endif()
        if(DUILIB_OFFLINE)
            duilib_error("deps.offline_missing" "deps.offline_missing_hint")
        endif()
        duilib_msg("deps.git_ref_switching" " (${label}: ${_current} -> ${ref_value})")
        duilib_run_with_retry(_ok "git fetch ${ref_value}"
                              git -C "${repo}" fetch --depth 1 origin "${ref_value}")
        if(NOT _ok)
            duilib_error_values("git fetch ${ref_value} failed for ${repo}"
                                "check the network/proxy settings")
        endif()
        execute_process(COMMAND git -C "${repo}" checkout -B "${ref_value}" FETCH_HEAD
                        RESULT_VARIABLE _rc)
        if(NOT _rc EQUAL 0)
            duilib_error_values("cannot switch ${repo} to branch ${ref_value}"
                                "tracked changes are already saved to git stash; remove the untracked files listed above (or run 'git -C ${repo} stash --include-untracked'), then retry")
        endif()
        return()
    endif()

    if(ref_type STREQUAL "tag")
        _duilib_git_resolve("${repo}" "refs/tags/${ref_value}^{commit}" _expected)
        set(_fetch_args fetch --depth 1 origin tag "${ref_value}")
    else()
        _duilib_git_resolve("${repo}" "${ref_value}^{commit}" _expected)
        set(_fetch_args fetch --depth 1 origin "${ref_value}")
    endif()
    if(NOT _expected STREQUAL "" AND _head STREQUAL _expected)
        return()
    endif()
    if(_stash_local)
        _duilib_git_stash_local("${repo}" "${label}")
    endif()
    if(NOT _expected STREQUAL "")
        # 目标提交已在本地：直接切过去，不必联网（离线同样可用）
        duilib_msg("deps.git_ref_switching" " (${label}: ${_head} -> ${ref_value})")
    else()
        if(DUILIB_OFFLINE)
            duilib_error("deps.offline_missing" "deps.offline_missing_hint")
        endif()
        duilib_msg("deps.git_ref_switching" " (${label}: ${_head} -> ${ref_value})")
        duilib_run_with_retry(_ok "git fetch ${ref_value}"
                              git -C "${repo}" ${_fetch_args})
        if(NOT _ok)
            duilib_error_values("git fetch ${ref_value} failed for ${repo}"
                                "check the network/proxy settings")
        endif()
    endif()
    execute_process(COMMAND git -C "${repo}" checkout "${ref_value}"
                    RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        duilib_error_values("cannot switch ${repo} to ${ref_value}"
                            "tracked changes are already saved to git stash; remove the untracked files listed above (or run 'git -C ${repo} stash --include-untracked'), then retry")
    endif()
endfunction()

# 补丁是否确实落在源码树中：只信 stamp 不够，git checkout 等操作可能还原工作区，
# 而 stamp 仍在，会让 gn 用回默认工具链（Windows 下是 cl.exe）导致编译失败。
function(_duilib_skia_patch_applied out_applied)
    set(_applied FALSE)
    if(EXISTS "${DUILIB_SKIA_ROOT}/gn/is_mingw.py"
       AND EXISTS "${DUILIB_SKIA_ROOT}/gn/BUILDCONFIG.gn")
        file(STRINGS "${DUILIB_SKIA_ROOT}/gn/BUILDCONFIG.gn" _lines REGEX "is_mingw")
        if(_lines)
            set(_applied TRUE)
        endif()
    endif()
    set(${out_applied} ${_applied} PARENT_SCOPE)
endfunction()

# 补丁指纹：基线提交 + 归档内容哈希（与旧脚本的 cksum 语义一致）
function(_duilib_skia_patch_key out_key patch)
    file(SHA256 "${patch}" _hash)
    set(${out_key} "${DUILIB_SKIA_REF_VALUE}:${_hash}" PARENT_SCOPE)
endfunction()

function(duilib_prepare_skia)
    _duilib_skia_candidates(_candidates)
    list(GET _candidates 0 _variant)
    _duilib_skia_lib_name(_lib_name)
    set(_out_dir "${DUILIB_SKIA_ROOT}/out/${_variant}")
    set(_lib "${_out_dir}/${_lib_name}")
    _duilib_skia_stamp_value(_stamp_expected)

    # 补丁归档随仓库分发（build/skia_patch），这里只校验它与清单配套。
    set(_patch "")
    set(_patch_key "")
    set(_patch_missing FALSE)
    set(_patch_stamp "${DUILIB_SKIA_ROOT}/.duilib-patch")
    set(_stamp_old "")
    if(EXISTS "${_patch_stamp}")
        file(READ "${_patch_stamp}" _stamp_old)
        string(STRIP "${_stamp_old}" _stamp_old)
    endif()
    if(NOT "${DUILIB_SKIA_PATCH_ZIP}" STREQUAL "")
        set(_patch "${DUILIB_SKIA_PATCH_DIR}/${DUILIB_SKIA_PATCH_ZIP}")
        if(EXISTS "${_patch}")
            _duilib_skia_patch_verify("${_patch}")
            _duilib_skia_patch_key(_patch_key "${_patch}")
        else()
            set(_patch_missing TRUE)
        endif()
    elseif(DUILIB_SKIA_REQUIRE_PATCH)
        duilib_error("deps.skia_patch_missing" "deps.skia_patch_missing_hint")
    endif()

    if(NOT DUILIB_DEPS_FORCE AND EXISTS "${_lib}")
        if(EXISTS "${_out_dir}/.duilib-ref")
            file(READ "${_out_dir}/.duilib-ref" _stamp)
            if(_stamp STREQUAL _stamp_expected AND
               ("${_patch_key}" STREQUAL "" OR _stamp_old STREQUAL _patch_key))
                if(_patch_missing)
                    _duilib_skia_warn_patch_missing("${_patch}")
                endif()
                _duilib_write_crt("${_out_dir}")
                duilib_msg("deps.skia_ready" " (${_variant})")
                return()
            endif()
            if(NOT "${_patch_key}" STREQUAL "" AND NOT _stamp_old STREQUAL _patch_key)
                duilib_msg("deps.skia_patch_changed" " (${DUILIB_SKIA_PATCH_ZIP})")
            endif()
        else()
            # 迁移：已有构建产物但还没有 stamp 时，源码 HEAD 与清单一致、补丁确实
            # 落在源码树中则直接采纳，避免为已有环境重复编译 Skia。
            _duilib_git_head("${DUILIB_SKIA_ROOT}" _head)
            _duilib_skia_patch_applied(_applied)
            set(_patch_ok TRUE)
            if(NOT "${_patch_key}" STREQUAL "")
                if(NOT _applied)
                    set(_patch_ok FALSE)
                elseif(EXISTS "${_patch_stamp}" AND NOT _stamp_old STREQUAL _patch_key)
                    # 补丁已更新：旧产物不能按新补丁采纳，必须重建
                    set(_patch_ok FALSE)
                endif()
            endif()
            if(_patch_ok AND
               (NOT DUILIB_SKIA_REF_TYPE STREQUAL "commit" OR _head STREQUAL DUILIB_SKIA_REF_VALUE))
                if(NOT "${_patch_key}" STREQUAL "" AND NOT _stamp_old STREQUAL _patch_key)
                    file(WRITE "${_patch_stamp}" "${_patch_key}")
                endif()
                if(_patch_missing)
                    _duilib_skia_warn_patch_missing("${_patch}")
                endif()
                file(WRITE "${_out_dir}/.duilib-ref" "${_stamp_expected}")
                _duilib_write_crt("${_out_dir}")
                duilib_msg("deps.skia_ready" " (${_variant}, adopted existing build)")
                return()
            endif()
        endif()
    endif()

    # 源码
    if(EXISTS "${DUILIB_SKIA_ROOT}" AND NOT EXISTS "${DUILIB_SKIA_ROOT}/.git")
        # 没有 .git 的残留目录无法 fetch，避免在无限重试下卡死
        duilib_error_values("${DUILIB_SKIA_ROOT} exists but is not a git checkout"
                            "remove that directory, or clone the repository manually")
    endif()
    if(NOT EXISTS "${DUILIB_SKIA_ROOT}")
        _duilib_clone("${DUILIB_SKIA_REF_TYPE}" "${DUILIB_SKIA_REF_VALUE}"
                      "${DUILIB_SKIA_REPO}" "${DUILIB_SKIA_ROOT}")
    else()
        # commit / tag / branch 统一按清单对齐工作区（HEAD 已一致时不联网）
        # 最后一个参数：切换前把工作区里的本地修改存进 git stash（旧版本补丁留下
        # 的脏工作区，不保存会让 checkout 失败；保存后可用 stash pop 找回）
        _duilib_ensure_git_ref("${DUILIB_SKIA_ROOT}" "${DUILIB_SKIA_REF_TYPE}"
                               "${DUILIB_SKIA_REF_VALUE}" "Skia" TRUE)
    endif()

    # 补丁包：需要重建时确认归档存在并校验 SHA-1
    if(NOT "${DUILIB_SKIA_PATCH_ZIP}" STREQUAL "")
        if(NOT EXISTS "${_patch}")
            duilib_error("deps.skia_patch_missing" "deps.skia_patch_missing_hint")
        endif()
        _duilib_skia_patch_verify("${_patch}")
        _duilib_skia_patch_applied(_applied)
        # 补丁内容与基线提交未变且补丁确实在源码树中时跳过解压：重复解压会刷新
        # 源码 mtime，让 Ninja 认为全部目标过期而重编整个 Skia（多配置构建尤其浪费）。
        if(_applied AND _stamp_old STREQUAL _patch_key)
            message(STATUS "[deps] Skia patch already applied, skip extraction")
        else()
            # 补丁内容变化（或需要重新应用）时先把源码树还原干净：旧补丁留在工作区
            # 里的修改会与新补丁叠加成混合状态。这里同样用 git stash 保存（不删除），
            # 用户叠加在旧补丁之上的手工修改需要时可用 'git stash pop' 找回。
            _duilib_git_stash_local("${DUILIB_SKIA_ROOT}" "Skia")
            if(NOT _applied)
                message(STATUS "[deps] Skia patch missing in source tree, re-applying")
            endif()
            message(STATUS "[deps] applying Skia patch ${_patch}")
            execute_process(COMMAND "${CMAKE_COMMAND}" -E tar xf "${_patch}"
                            WORKING_DIRECTORY "${DUILIB_SKIA_ROOT}"
                            RESULT_VARIABLE _tar_rc)
            if(NOT _tar_rc EQUAL 0)
                duilib_error_values("failed to extract ${_patch}" "check the patch archive integrity")
            endif()
            file(WRITE "${_patch_stamp}" "${_patch_key}")
        endif()
        file(REMOVE_RECURSE "${_out_dir}")
    endif()

    _duilib_find_gn(_gn)
    _duilib_find_ninja(_ninja)
    _duilib_find_python3(_python3)
    if(_gn STREQUAL "")
        duilib_error("preflight.tool_missing" "preflight.gn_missing_hint")
    endif()
    if(_ninja STREQUAL "")
        duilib_error("preflight.tool_missing" "preflight.ninja_missing_hint")
    endif()
    if(_python3 STREQUAL "")
        duilib_error("deps.python3_missing" "deps.python3_missing_hint")
    endif()

    duilib_msg("deps.skia_building" " (${_variant})")
    file(REMOVE_RECURSE "${_out_dir}")
    _duilib_skia_gn_args(_args "${_variant}")
    execute_process(
        COMMAND "${_gn}" gen "out/${_variant}" "--args=${_args}"
        WORKING_DIRECTORY "${DUILIB_SKIA_ROOT}"
        RESULT_VARIABLE _gn_rc)
    if(NOT _gn_rc EQUAL 0)
        duilib_t("deps.skia_gn_failed" _gn_what)
        duilib_t("deps.skia_gn_failed_hint" _gn_hint)
        string(REPLACE "{gn}" "${_gn}" _gn_what "${_gn_what}")
        string(REPLACE "{variant}" "${_variant}" _gn_what "${_gn_what}")
        string(REPLACE "{rc}" "${_gn_rc}" _gn_what "${_gn_what}")
        message(FATAL_ERROR "${_gn_what}\n  Hint: ${_gn_hint}")
    endif()
    execute_process(
        COMMAND "${_ninja}" -C "out/${_variant}" -j "${DUILIB_DEPS_JOBS}"
        WORKING_DIRECTORY "${DUILIB_SKIA_ROOT}"
        RESULT_VARIABLE _ninja_rc)
    if(NOT _ninja_rc EQUAL 0 OR NOT EXISTS "${_lib}")
        duilib_error("deps.skia_failed" "deps.skia_failed_hint")
    endif()
    file(WRITE "${_out_dir}/.duilib-ref" "${_stamp_expected}")
    _duilib_write_crt("${_out_dir}")
    duilib_msg("deps.skia_ready" " (${_variant})")
endfunction()

# ---------------------------------------------------------------------------
# SDL3
# ---------------------------------------------------------------------------
function(_duilib_sdl_stamp_value out_var)
    # 源码提交参与 stamp：本地仓库可能停在别的分支/标签（例如 SDL 源码停在
    # main），只记录 ref 名称会把“用错版本编译出来的库”当成清单版本的产物复用。
    _duilib_git_head("${DUILIB_SDL_SRC}" _sdl_commit)
    # Windows：stamp 带上目标配置。Debug 与 Release 生成的 SDL3-static.lib 同名
    # 且安装到同一个前缀，最终只保留最近一次安装的那一份；切换配置必须重建，
    # 否则 Debug 会链接 Release 版静态库（/MTd 与 /MT、/MDd 与 /MD 混用）。
    # Unix：SDL3 固定按 Release 构建共享库（见下方 ${DUILIB_ARCH}-release 目录），
    # 产物与 duilib 的 Debug/Release 无关；stamp 不含配置分量，避免切换配置时
    # 无谓地重新编译 SDL3。
    if(DUILIB_DEPS_PLATFORM STREQUAL "windows")
        set(${out_var}
            "${DUILIB_SDL_REF_VALUE}|${_sdl_commit}|${DUILIB_ARCH}|${DUILIB_CRT_ID}|${DUILIB_DEPS_PLATFORM}|${DUILIB_CONFIG}"
            PARENT_SCOPE)
    else()
        set(${out_var}
            "${DUILIB_SDL_REF_VALUE}|${_sdl_commit}|${DUILIB_ARCH}|${DUILIB_CRT_ID}|${DUILIB_DEPS_PLATFORM}"
            PARENT_SCOPE)
    endif()
endfunction()

function(_duilib_sdl_ready out_ready)
    set(_ok FALSE)
    if(EXISTS "${DUILIB_SDL_ROOT}/.duilib-sdl" AND EXISTS "${DUILIB_SDL_ROOT}/include/SDL3/SDL.h")
        file(READ "${DUILIB_SDL_ROOT}/.duilib-sdl" _stamp)
        _duilib_sdl_stamp_value(_expected)
        if(_stamp STREQUAL _expected)
            if(DUILIB_DEPS_PLATFORM STREQUAL "windows")
                if(DUILIB_MINGW)
                    if(EXISTS "${DUILIB_SDL_ROOT}/lib/libSDL3.a" OR
                       EXISTS "${DUILIB_SDL_ROOT}/lib/libSDL3d.a")
                        set(_ok TRUE)
                    endif()
                elseif(EXISTS "${DUILIB_SDL_ROOT}/lib/SDL3-static.lib")
                    set(_ok TRUE)
                endif()
            else()
                if(DUILIB_DEPS_PLATFORM STREQUAL "macos")
                    # macOS 安装产物是 .dylib；用 .so 判断会把已完成的构建误判为失败。
                    if(EXISTS "${DUILIB_SDL_ROOT}/lib/libSDL3.dylib")
                        set(_ok TRUE)
                    endif()
                elseif(EXISTS "${DUILIB_SDL_ROOT}/lib/libSDL3.so" OR
                       EXISTS "${DUILIB_SDL_ROOT}/lib64/libSDL3.so")
                    set(_ok TRUE)
                endif()
            endif()
        endif()
    endif()
    set(${out_ready} ${_ok} PARENT_SCOPE)
endfunction()

function(_duilib_run_checked what hint)
    execute_process(COMMAND ${ARGN} RESULT_VARIABLE _rc)
    if(NOT _rc EQUAL 0)
        duilib_error_values("${what} failed" "${hint}")
    endif()
endfunction()

function(_duilib_sdl_build_one build_dir config)
    set(_args
        -S "${DUILIB_SDL_SRC}"
        -B "${build_dir}"
        "-DCMAKE_INSTALL_PREFIX=${DUILIB_SDL_ROOT}"
        "-DSDL_TEST_LIBRARY=OFF")
    if(DUILIB_DEPS_PLATFORM STREQUAL "windows")
        list(APPEND _args -G Ninja "-DCMAKE_BUILD_TYPE=${config}")
        if(DUILIB_MINGW)
            list(APPEND _args -DSDL_SHARED=OFF -DSDL_STATIC=ON)
        else()
            list(APPEND _args -DSDL_SHARED=OFF -DSDL_STATIC=ON)
            if(DUILIB_MD)
                if(config STREQUAL "Debug")
                    list(APPEND _args "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebugDLL")
                else()
                    list(APPEND _args "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL")
                endif()
            else()
                if(config STREQUAL "Debug")
                    list(APPEND _args "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDebug")
                else()
                    list(APPEND _args "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded")
                endif()
            endif()
        endif()
    else()
        list(APPEND _args -DCMAKE_BUILD_TYPE=Release -DSDL_SHARED=ON -DSDL_STATIC=OFF
             -DSDL_X11_XSCRNSAVER=OFF -DSDL_X11_XTEST=OFF)
    endif()

    _duilib_run_checked("SDL3 configure (${config})"
                        "check the SDL3 source tree and the compiler environment"
                        "${DUILIB_CMAKE_COMMAND}" ${_args})
    _duilib_run_checked("SDL3 build (${config})"
                        "check the SDL3 compile output above"
                        "${DUILIB_CMAKE_COMMAND}" --build "${build_dir}" --parallel "${DUILIB_DEPS_JOBS}")
    if(DUILIB_DEPS_PLATFORM STREQUAL "windows")
        _duilib_run_checked("SDL3 install (${config})"
                            "check the SDL3 install output above"
                            "${DUILIB_CMAKE_COMMAND}" --install "${build_dir}" --config "${config}")
    else()
        _duilib_run_checked("SDL3 install"
                            "check the SDL3 install output above"
                            "${DUILIB_CMAKE_COMMAND}" --install "${build_dir}")
    endif()
endfunction()

function(duilib_prepare_sdl)
    # 本地源码可能停在别的分支/标签（如 main），先按清单对齐；stamp 里包含源码
    # 提交，切换后旧版本的安装产物不会被当成清单版本的产物复用。
    if(EXISTS "${DUILIB_SDL_SRC}/.git")
        _duilib_ensure_git_ref("${DUILIB_SDL_SRC}" "${DUILIB_SDL_REF_TYPE}"
                               "${DUILIB_SDL_REF_VALUE}" "SDL3")
    endif()
    _duilib_sdl_ready(_ready)
    if(_ready AND NOT DUILIB_DEPS_FORCE)
        _duilib_write_crt("${DUILIB_SDL_ROOT}")
        duilib_msg("deps.sdl_ready" " (${DUILIB_SDL_ROOT})")
        return()
    endif()

    if(EXISTS "${DUILIB_SDL_SRC}" AND NOT EXISTS "${DUILIB_SDL_SRC}/.git")
        # 没有 .git 的残留目录无法更新，避免在无限重试下卡死
        duilib_error_values("${DUILIB_SDL_SRC} exists but is not a git checkout"
                            "remove that directory, or clone the repository manually")
    endif()
    if(NOT EXISTS "${DUILIB_SDL_SRC}")
        _duilib_clone("${DUILIB_SDL_REF_TYPE}" "${DUILIB_SDL_REF_VALUE}"
                      "${DUILIB_SDL_REPO}" "${DUILIB_SDL_SRC}")
    endif()

    duilib_msg("deps.sdl_building" " (${DUILIB_SDL_REF_VALUE})")
    if(DUILIB_DEPS_PLATFORM STREQUAL "windows")
        # SDL3 的构建目录按“架构-配置”命名、与工具链无关。若上次用别的方式
        # （MSVC / MinGW / 不同运行库）构建过，目录里的 CMake 缓存仍指向旧编译器，
        # 直接复用会失败（例如 MinGW 配置时仍调用 cl.exe）。stamp 记录上次成功
        # 构建的 CRT，与本次不一致时清空构建目录重新配置。
        _duilib_sdl_stamp_value(_stamp_expected)
        set(_sdl_cache_reusable FALSE)
        if(EXISTS "${DUILIB_SDL_ROOT}/.duilib-sdl")
            file(READ "${DUILIB_SDL_ROOT}/.duilib-sdl" _sdl_old_stamp)
            if(_sdl_old_stamp STREQUAL _stamp_expected)
                set(_sdl_cache_reusable TRUE)
            endif()
        endif()
        if(DUILIB_DEPS_FORCE OR NOT _sdl_cache_reusable)
            file(REMOVE_RECURSE "${DUILIB_SDL_BUILD_ROOT}")
        endif()
        if(DUILIB_MULTI_CONFIG)
            # 多配置生成器在配置期不知道目标配置，只能把两个配置都构建一遍；
            # 两者安装到同一前缀、库同名，最后安装的 Release 会覆盖 Debug，
            # 因此 Debug 目标实际链接的是 Release 版 SDL3 静态库。
            # 需要严格的 Debug/Release 一致时请使用单配置生成器（Ninja/Makefiles）。
            duilib_msg("deps.sdl_multi_config" "")
            _duilib_sdl_build_one("${DUILIB_SDL_BUILD_ROOT}/${DUILIB_ARCH}-debug" "Debug")
            _duilib_sdl_build_one("${DUILIB_SDL_BUILD_ROOT}/${DUILIB_ARCH}-release" "Release")
        else()
            # 单配置生成器（build.py 默认的 Ninja）：只构建并安装当前配置，
            # 保证 Debug 使用 /MTd、/MDd 编译的 SDL3，Release 使用 /MT、/MD。
            _duilib_sdl_build_one("${DUILIB_SDL_BUILD_ROOT}/${DUILIB_ARCH}-${DUILIB_BUILD_TYPE}"
                                  "${DUILIB_CONFIG}")
        endif()
    else()
        _duilib_sdl_build_one("${DUILIB_SDL_BUILD_ROOT}/${DUILIB_ARCH}-release" "Release")
    endif()

    # 先写入 stamp 再校验：_duilib_sdl_ready 以 stamp + 头文件 + 库三者判断，
    # 首次安装前不存在 stamp，若先校验会导致构建成功后仍报失败。
    _duilib_sdl_stamp_value(_stamp)
    file(WRITE "${DUILIB_SDL_ROOT}/.duilib-sdl" "${_stamp}")
    _duilib_sdl_ready(_ready)
    if(NOT _ready)
        duilib_error("deps.sdl_failed" "deps.sdl_failed_hint")
    endif()
    _duilib_write_crt("${DUILIB_SDL_ROOT}")
    duilib_msg("deps.sdl_ready" " (${DUILIB_SDL_ROOT})")
endfunction()

# ---------------------------------------------------------------------------
# 主流程
# ---------------------------------------------------------------------------
if(DUILIB_DEPS_MODE STREQUAL "prepare" OR DUILIB_DEPS_MODE STREQUAL "auto")
    duilib_prepare_skia()
    if(DUILIB_ENABLE_SDL)
        duilib_prepare_sdl()
    endif()
    if(DUILIB_ENABLE_CEF)
        duilib_cef_prepare()
    endif()
    duilib_msg("deps.done")
else()
    message(STATUS "[deps] mode=${DUILIB_DEPS_MODE}: nothing to prepare")
endif()

file(LOCK "${_duilib_lock_file}" RELEASE)
