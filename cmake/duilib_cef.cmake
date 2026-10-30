# CEF 运行时准备（脚本模式）
#
# 只下载 Release/Resources 运行时（Windows/Linux），编译期仍使用仓库内
# duilib/third_party/libcef 跟踪的头文件与导入库。macOS 解压完整发行包到
# DUILIB_CEF_ROOT。
#
# 版本一致性：下载包里的 include/cef_version.h 必须与仓库内跟踪的头文件一致，
# 避免“头文件与运行时版本不一致”（如默认版本头文件配 109 运行时）这类难以定位的崩溃。
#
# 清单统一使用 `_minimal` 发行包：包含 include/、Release/、Resources/ 等
# 运行与校验所需文件，体积约为标准包的一半。

include("${CMAKE_CURRENT_LIST_DIR}/duilib_i18n.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/duilib_download.cmake")

function(_duilib_cef_read_version header out_var)
    set(_version "")
    if(EXISTS "${header}")
        file(STRINGS "${header}" _line REGEX "^#define CEF_VERSION ")
        if(_line MATCHES "\"([^\"]+)\"")
            set(_version "${CMAKE_MATCH_1}")
        endif()
    endif()
    set(${out_var} "${_version}" PARENT_SCOPE)
endfunction()

function(_duilib_cef_tracked_dir out_var)
    # 跟踪头文件（include/cef_version.h）必须取当前平台的 libcef 源目录，
    # 否则 Linux/macOS 会错对 Windows 的跟踪头做版本比对与迁移采纳。
    if(DUILIB_DEPS_PLATFORM STREQUAL "linux")
        set(${out_var}
            "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_linux" PARENT_SCOPE)
        return()
    endif()
    if(DUILIB_DEPS_PLATFORM STREQUAL "macos")
        set(${out_var}
            "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_macos" PARENT_SCOPE)
        return()
    endif()
    set(_suffix "")
    if("${DUILIB_CEF_VERSION}" STREQUAL "109")
        set(_suffix "_109")
    endif()
    set(${out_var}
        "${DUILIB_ROOT}/duilib/third_party/libcef/libcef_win${_suffix}" PARENT_SCOPE)
endfunction()

function(_duilib_cef_dest_dir out_var)
    if(DUILIB_DEPS_PLATFORM STREQUAL "windows")
        set(_suffix "")
        if("${DUILIB_CEF_VERSION}" STREQUAL "109")
            set(_suffix "_109")
        endif()
        if(DUILIB_ARCH STREQUAL "x86")
            set(_cef_arch "Win32")
        else()
            set(_cef_arch "${DUILIB_ARCH}")
        endif()
        set(${out_var} "${DUILIB_ROOT}/bin/libcef_win${_suffix}/${_cef_arch}" PARENT_SCOPE)
    elseif(DUILIB_DEPS_PLATFORM STREQUAL "linux")
        set(${out_var} "${DUILIB_ROOT}/bin/libcef_linux" PARENT_SCOPE)
    else()
        set(${out_var} "${DUILIB_CEF_ROOT}" PARENT_SCOPE)
    endif()
endfunction()

function(_duilib_cef_stamp_value out_var)
    set(${out_var} "${DUILIB_CEF_VERSION}|${DUILIB_ARCH}|${DUILIB_DEPS_PLATFORM}" PARENT_SCOPE)
endfunction()

# 从官方 .sha1 文件内容中提取 40 位十六进制 SHA1（文件可能只含哈希一行，
# 也可能带文件名）。内容无效时返回空串，由调用方回退其他校验方式。
function(_duilib_cef_read_sha1 file out_var)
    set(_value "")
    if(EXISTS "${file}")
        file(READ "${file}" _content LIMIT 4096)
        string(REGEX MATCHALL "[0-9a-fA-F]+" _tokens "${_content}")
        foreach(_token IN LISTS _tokens)
            string(LENGTH "${_token}" _token_len)
            if(_token_len EQUAL 40)
                string(TOLOWER "${_token}" _value)
                break()
            endif()
        endforeach()
    endif()
    set(${out_var} "${_value}" PARENT_SCOPE)
endfunction()

# 归档完整性校验：优先使用 CEF 官方发布的 .sha1；拿不到时回退版本清单里的
# sha256（两者都为空时由 duilib_verify_* 告警并放行）。
function(_duilib_cef_verify_archive archive sha1 sha256 out_ok)
    if(NOT "${sha1}" STREQUAL "")
        duilib_verify_sha1("${archive}" "${sha1}" _ok)
    else()
        duilib_verify_sha256("${archive}" "${sha256}" _ok)
    endif()
    set(${out_ok} "${_ok}" PARENT_SCOPE)
endfunction()

function(_duilib_cef_is_ready dest out_ready)
    set(_ready FALSE)
    if(EXISTS "${dest}/.duilib-cef")
        file(READ "${dest}/.duilib-cef" _stamp)
        _duilib_cef_stamp_value(_expected)
        if(_stamp STREQUAL _expected)
            if(DUILIB_DEPS_PLATFORM STREQUAL "windows")
                if(EXISTS "${dest}/libcef.dll")
                    set(_ready TRUE)
                endif()
            elseif(DUILIB_DEPS_PLATFORM STREQUAL "linux")
                if(EXISTS "${dest}/libcef.so")
                    set(_ready TRUE)
                endif()
            else()
                # macOS 的运行时是完整发行包：必须同时存在框架，否则只凭指纹会
                # 判定“已就绪”，构建期才发现框架缺失
                if(EXISTS "${dest}/Release/Chromium Embedded Framework.framework"
                   OR EXISTS "${dest}/Debug/Chromium Embedded Framework.framework")
                    set(_ready TRUE)
                endif()
            endif()
        endif()
    endif()
    set(${out_ready} ${_ready} PARENT_SCOPE)
endfunction()

# 清理 CEF 发行包目录（macOS 的 CEF_ROOT）中的旧内容。
# 默认情况下 DUILIB_CEF_ROOT 与 DUILIB_CEF_CACHE 是同一个目录（<deps_root>/cef_binary），
# 所以只清理发行包自带的条目，保留 downloads/、extract/ 等缓存目录。
# 目的：升级 CEF 后目录里不会同时留有上一版本（如 142）的文件。
function(_duilib_cef_clean_distribution dest)
    set(_entries
        include libcef_dll cmake Debug Release Resources tests bazel
        BUILD.bazel CMakeLists.txt Doxyfile LICENSE.txt MODULE.bazel README.md
        README.txt WORKSPACE cef_paths.gypi cef_paths2.gypi)
    foreach(_entry IN LISTS _entries)
        if(EXISTS "${dest}/${_entry}")
            file(REMOVE_RECURSE "${dest}/${_entry}")
        endif()
    endforeach()
    # 旧版本直接解压出来的目录（如 cef_binary_142.../）同样清掉
    file(GLOB _subdirs LIST_DIRECTORIES true "${dest}/*")
    foreach(_dir IN LISTS _subdirs)
        if(IS_DIRECTORY "${_dir}" AND EXISTS "${_dir}/include/cef_version.h")
            file(REMOVE_RECURSE "${_dir}")
        endif()
    endforeach()
endfunction()

function(duilib_cef_prepare)
    set(_key "${DUILIB_DEPS_PLATFORM}/${DUILIB_ARCH}")
    string(JSON _file ERROR_VARIABLE _json_err
           GET "${DUILIB_VERSIONS_JSON}" cef versions "${DUILIB_CEF_VERSION}" archives "${_key}" "file")
    if(_json_err OR _file STREQUAL "")
        duilib_error("deps.cef_combo_missing" "deps.cef_combo_missing_hint")
    endif()
    string(JSON _sha ERROR_VARIABLE _json_err
           GET "${DUILIB_VERSIONS_JSON}" cef versions "${DUILIB_CEF_VERSION}" archives "${_key}" "sha256")
    if(_json_err)
        set(_sha "")
    endif()

    _duilib_cef_dest_dir(_dest)
    if(NOT DUILIB_DEPS_FORCE)
        _duilib_cef_is_ready("${_dest}" _ready)
        if(_ready)
            duilib_msg("deps.cef_ready" " (${DUILIB_CEF_VERSION}/${DUILIB_ARCH})")
            return()
        endif()

        # 迁移：已经手动解压过、但没有指纹的运行时，只有能读到自身版本时才采纳。
        # Windows/Linux 的运行时目录只放二进制（没有 include/cef_version.h），读不到
        # 版本：升级 CEF 后旧 libcef.dll/libcef.so 会被误标成新版本，因此不采纳，
        # 按归档重新解压覆盖（见下方“清理旧版本残留”）。
        if(DUILIB_DEPS_PLATFORM STREQUAL "macos")
            set(_runtime "${_dest}/include/cef_version.h")
            if(EXISTS "${_runtime}"
               AND (EXISTS "${_dest}/Release/Chromium Embedded Framework.framework"
                    OR EXISTS "${_dest}/Debug/Chromium Embedded Framework.framework"))
                _duilib_cef_read_version("${_runtime}" _runtime_version)
                if(_runtime_version MATCHES "^${DUILIB_CEF_VERSION}\\.")
                    _duilib_cef_stamp_value(_stamp)
                    file(WRITE "${_dest}/.duilib-cef" "${_stamp}")
                    duilib_msg("deps.cef_ready" " (${DUILIB_CEF_VERSION}/${DUILIB_ARCH}, adopted existing runtime)")
                    return()
                endif()
            endif()
        endif()
    endif()

    # 1) 选择/获取归档
    set(_url "${DUILIB_CEF_URL_BASE}/${_file}")
    # 官方为每个归档发布 <url>.sha1（内容为 40 位 SHA1）。下载到归档旁缓存，
    # 后续构建与离线模式都可复用；拿不到时回退清单里的 sha256。
    set(_sha1_file "${DUILIB_CEF_CACHE}/downloads/${_file}.sha1")
    set(_sha1 "")
    if(EXISTS "${_sha1_file}")
        _duilib_cef_read_sha1("${_sha1_file}" _sha1)
    endif()
    if(_sha1 STREQUAL "" AND NOT DUILIB_OFFLINE)
        duilib_msg("deps.cef_sha1_fetching" " ${_url}.sha1")
        duilib_download_file_optional("${_url}.sha1" "${_sha1_file}" _sha1_ok)
        if(_sha1_ok)
            _duilib_cef_read_sha1("${_sha1_file}" _sha1)
        endif()
        if(_sha1 STREQUAL "")
            # 内容不是有效的 40 位 SHA1 时同样删除，避免反复读到坏文件
            file(REMOVE "${_sha1_file}")
            message(STATUS "[deps] SHA1 unavailable for ${_file}; falling back to manifest sha256")
        endif()
    endif()

    set(_archive "${DUILIB_CEF_ARCHIVE}")
    set(_archive_is_user FALSE)
    if(_archive STREQUAL "")
        set(_archive "${DUILIB_CEF_CACHE}/downloads/${_file}")
    else()
        set(_archive_is_user TRUE)
    endif()
    set(_ok FALSE)
    if(EXISTS "${_archive}")
        _duilib_cef_verify_archive("${_archive}" "${_sha1}" "${_sha}" _ok)
        if(NOT _ok)
            if(_archive_is_user)
                # 用户指定的归档不删除，直接报错以免破坏其文件
                duilib_error("deps.cef_hash_mismatch" "deps.cef_hash_mismatch_hint")
            endif()
            # 缓存损坏/不完整时重新下载，而不是直接终止
            message(STATUS "[deps] cached archive checksum mismatch, re-downloading ${_file}")
            file(REMOVE "${_archive}")
        endif()
    endif()
    if(NOT EXISTS "${_archive}")
        if(DUILIB_OFFLINE)
            duilib_error("deps.offline_missing" "deps.offline_missing_hint")
        endif()
        duilib_msg("deps.cef_downloading" " ${_url}")
        set(_attempt 1)
        while(_attempt LESS_EQUAL 3)
            duilib_download_file("${_url}" "${_archive}" "CEF ${DUILIB_CEF_VERSION}")
            _duilib_cef_verify_archive("${_archive}" "${_sha1}" "${_sha}" _ok)
            if(_ok)
                break()
            endif()
            message(STATUS "[deps] checksum mismatch for ${_file}, retrying (${_attempt}/3)")
            file(REMOVE "${_archive}")
            math(EXPR _attempt "${_attempt} + 1")
        endwhile()
        if(NOT _ok)
            duilib_error("deps.cef_hash_mismatch" "deps.cef_hash_mismatch_hint")
        endif()
    endif()

    # 2) 解压到缓存下的临时目录，校验版本后再同步到运行时目录。
    #    macOS 不能直接解压到 CEF_ROOT：默认它与缓存目录是同一个（<deps_root>/cef_binary），
    #    --force 时会连带删掉刚校验过的归档，且旧版本（142）的残留文件不会被清理。
    set(_extract "${DUILIB_CEF_CACHE}/extract/${DUILIB_CEF_VERSION}/${DUILIB_ARCH}")
    if(DUILIB_DEPS_PLATFORM STREQUAL "macos")
        set(_extract "${_extract}-macos")
    endif()
    # 每次解压前清空临时目录，避免上次中断留下的半截产物混进本次结果
    file(REMOVE_RECURSE "${_extract}")
    file(MAKE_DIRECTORY "${_extract}")

    message(STATUS "[deps] extracting ${_archive}")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E tar xf "${_archive}"
        WORKING_DIRECTORY "${_extract}"
        RESULT_VARIABLE _tar_rc)
    if(NOT _tar_rc EQUAL 0)
        duilib_error("deps.cef_extract_failed" "deps.cef_extract_failed_hint")
    endif()

    # 3) 找到发行包根目录并校验版本（优先选与跟踪头文件版本完全一致的目录，
    #    避免临时目录里残留的其他版本解压结果导致误报）
    _duilib_cef_tracked_dir(_tracked)
    _duilib_cef_read_version("${_tracked}/include/cef_version.h" _tracked_version)
    file(GLOB _entries "${_extract}/*")
    set(_top "")
    set(_archive_version "")
    foreach(_entry IN LISTS _entries)
        if(IS_DIRECTORY "${_entry}" AND EXISTS "${_entry}/include/cef_version.h")
            _duilib_cef_read_version("${_entry}/include/cef_version.h" _entry_version)
            if(_archive_version STREQUAL "")
                set(_archive_version "${_entry_version}")
            endif()
            if(_entry_version STREQUAL _tracked_version)
                set(_top "${_entry}")
                break()
            endif()
        endif()
    endforeach()
    if(_top STREQUAL "" AND EXISTS "${_extract}/include/cef_version.h")
        set(_top "${_extract}")
        _duilib_cef_read_version("${_extract}/include/cef_version.h" _archive_version)
    endif()
    if(_top STREQUAL "")
        if(_archive_version STREQUAL "")
            duilib_error("deps.cef_extract_failed" "deps.cef_extract_failed_hint")
        endif()
        duilib_error_values(
            "tracked CEF_VERSION='${_tracked_version}' but archive CEF_VERSION='${_archive_version}'"
            "use a matching DUILIB_CEF_VERSION, or update the tracked libcef headers")
    endif()

    # 4) 同步运行时：先清理旧版本残留，保证从 142 之类的旧版本升级后目录里
    #    只有当前版本的文件（否则旧 DLL/旧 .so 会被一起打包或干扰加载）
    if(DUILIB_DEPS_PLATFORM STREQUAL "macos")
        # 完整发行包：清理 CEF_ROOT 下的旧内容（保留下载/解压缓存）后覆盖复制
        file(MAKE_DIRECTORY "${_dest}")
        _duilib_cef_clean_distribution("${_dest}")
        file(COPY "${_top}/" DESTINATION "${_dest}")
    else()
        file(REMOVE_RECURSE "${_dest}")
        file(MAKE_DIRECTORY "${_dest}")
        if(EXISTS "${_top}/Release/")
            file(COPY "${_top}/Release/" DESTINATION "${_dest}")
        endif()
        if(EXISTS "${_top}/Resources/")
            file(COPY "${_top}/Resources/" DESTINATION "${_dest}")
        endif()
        # 校验复制结果：目标文件被占用（CEF 程序还在运行）时复制会失败，此时必须
        # 报错，否则旧版本的 libcef 会被写入新版本指纹，运行时行为不可预期
        if(DUILIB_DEPS_PLATFORM STREQUAL "windows")
            set(_runtime_file "libcef.dll")
        else()
            set(_runtime_file "libcef.so")
        endif()
        file(SIZE "${_top}/Release/${_runtime_file}" _cef_src_size)
        file(SIZE "${_dest}/${_runtime_file}" _cef_dst_size)
        if(_cef_src_size EQUAL 0 OR NOT _cef_dst_size EQUAL _cef_src_size)
            message(STATUS "[deps] failed to refresh ${_dest}/${_runtime_file}: "
                           "the file may be in use by a running CEF program")
            duilib_error("deps.cef_extract_failed" "deps.cef_extract_failed_hint")
        endif()
    endif()

    _duilib_cef_stamp_value(_stamp)
    file(WRITE "${_dest}/.duilib-cef" "${_stamp}")
    duilib_msg("deps.cef_ready" " (${DUILIB_CEF_VERSION}/${DUILIB_ARCH})")
endfunction()
