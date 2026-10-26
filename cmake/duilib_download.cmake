# 下载工具函数（脚本模式可用）
#
# 语义：
#   - 优先 curl（支持断点续传）；没有 curl 时退回 file(DOWNLOAD)；
#   - 统一重试参数：DUILIB_DOWNLOAD_RETRIES / DUILIB_DOWNLOAD_RETRY_DELAY，
#     来源于 cmake/third_party_versions.json 的 retry 段（0 = 无限重试）；
#   - 重试间隔固定为 DUILIB_DOWNLOAD_RETRY_DELAY（默认 20 秒）；
#   - 完整性校验由调用方负责（sha256 不匹配时删除缓存并重新下载，最多 3 次）。

include_guard(GLOBAL)
include("${CMAKE_CURRENT_LIST_DIR}/duilib_i18n.cmake")

function(_duilib_sleep seconds)
    if(seconds GREATER 0)
        execute_process(COMMAND "${CMAKE_COMMAND}" -E sleep "${seconds}")
    endif()
endfunction()

# 读取统一的重试参数（空值按默认：无限重试 / 20 秒）
function(_duilib_retry_settings out_max out_delay)
    set(_max "${DUILIB_DOWNLOAD_RETRIES}")
    if("${_max}" STREQUAL "")
        set(_max 0)
    endif()
    set(_delay "${DUILIB_DOWNLOAD_RETRY_DELAY}")
    if("${_delay}" STREQUAL "")
        set(_delay 20)
    endif()
    set(${out_max} "${_max}" PARENT_SCOPE)
    set(${out_delay} "${_delay}" PARENT_SCOPE)
endfunction()

# duilib_run_with_retry(<out_ok> <what> <command...>)
# 用统一的重试参数执行外部命令（git clone/fetch 等网络操作）。
# 成功时 out_ok=TRUE；达到最大次数仍失败时 out_ok=FALSE（由调用方决定如何报错）。
function(duilib_run_with_retry out_ok what)
    _duilib_retry_settings(_max _delay)
    set(_attempt 1)
    while(TRUE)
        execute_process(COMMAND ${ARGN} RESULT_VARIABLE _rc)
        if(_rc EQUAL 0)
            set(${out_ok} TRUE PARENT_SCOPE)
            return()
        endif()
        if(_max GREATER 0 AND _attempt GREATER_EQUAL _max)
            set(${out_ok} FALSE PARENT_SCOPE)
            return()
        endif()
        message(STATUS "[deps] ${what} failed (attempt ${_attempt}), retrying in ${_delay}s")
        _duilib_sleep("${_delay}")
        math(EXPR _attempt "${_attempt} + 1")
    endwhile()
endfunction()

# 返回 curl 路径（可为空）
function(_duilib_find_curl out_var)
    find_program(_duilib_curl curl)
    set(${out_var} "${_duilib_curl}" PARENT_SCOPE)
endfunction()

# duilib_download_file(<url> <dest> [<logical-name>])
# 失败时直接 FATAL_ERROR。
function(duilib_download_file url dest)
    set(_name "${url}")
    if(ARGC GREATER 2)
        set(_name "${ARGV2}")
    endif()
    get_filename_component(_dest_dir "${dest}" DIRECTORY)
    file(MAKE_DIRECTORY "${_dest_dir}")

    _duilib_retry_settings(_max_attempts _delay)
    set(_connect_timeout "${DUILIB_DOWNLOAD_CONNECT_TIMEOUT}")
    if("${_connect_timeout}" STREQUAL "")
        set(_connect_timeout 30)
    endif()

    _duilib_find_curl(_duilib_curl)
    set(_attempt 1)
    while(TRUE)
        if(_duilib_curl)
            execute_process(
                COMMAND "${_duilib_curl}" -fL --continue-at - --connect-timeout "${_connect_timeout}"
                        -o "${dest}" "${url}"
                RESULT_VARIABLE _rc)
        else()
            file(DOWNLOAD "${url}" "${dest}"
                 STATUS _status
                 TIMEOUT "${_connect_timeout}"
                 TLS_VERIFY ON)
            list(GET _status 0 _rc)
        endif()
        if(_rc EQUAL 0 AND EXISTS "${dest}" AND NOT "${dest}" STREQUAL "")
            file(SIZE "${dest}" _size)
            if(_size GREATER 0)
                message(STATUS "[deps] downloaded ${_name}")
                return()
            endif()
        endif()
        if(_max_attempts GREATER 0 AND _attempt GREATER_EQUAL _max_attempts)
            duilib_error("deps.cef_download_failed" "deps.cef_download_failed_hint")
        endif()
        message(STATUS "[deps] download attempt ${_attempt} failed: ${_name}, retrying in ${_delay}s")
        _duilib_sleep("${_delay}")
        math(EXPR _attempt "${_attempt} + 1")
    endwhile()
endfunction()

# 校验 sha256；expected 为空时仅告警并返回成功。
function(duilib_verify_sha256 path expected out_ok)
    if("${expected}" STREQUAL "")
        duilib_warn("deps.cef_no_sha")
        set(${out_ok} TRUE PARENT_SCOPE)
        return()
    endif()
    if(NOT EXISTS "${path}")
        set(${out_ok} FALSE PARENT_SCOPE)
        return()
    endif()
    file(SHA256 "${path}" _actual)
    string(TOLOWER "${_actual}" _actual_lower)
    string(TOLOWER "${expected}" _expected_lower)
    if(_actual_lower STREQUAL _expected_lower)
        set(${out_ok} TRUE PARENT_SCOPE)
    else()
        set(${out_ok} FALSE PARENT_SCOPE)
    endif()
endfunction()
