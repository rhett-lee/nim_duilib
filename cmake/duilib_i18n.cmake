# duilib 构建系统：交互语言解析与消息目录（简体中文 + 英文）
#
# 单一维护点：cmake/i18n/messages.json
# 本文件既可用于 project() 之前的普通 include，也可用于 `cmake -P` 脚本模式。
#
# 对外接口：
#   duilib_t(<key> <out_var>)        读取当前语言的消息文本（缺失时回退英文）
#   duilib_msg(<key> [<value>])      输出 [OK] 风格的状态行
#   duilib_warn(<key> [<value>])     输出 [WARN] 行
#   duilib_error(<key> <hint_key>)   FATAL_ERROR，固定两行：[ERROR] + Hint:
#   duilib_error_values(<key> <hint_key> <what> <hint>)  带动态内容的错误
#
# 语言判定顺序：DUILIB_LANG -> 环境变量 DUILIB_LANG -> LC_ALL/LC_MESSAGES/LANG
# -> Windows 注册表 LocaleName -> 英文。

set(DUILIB_I18N_MESSAGES_FILE "${CMAKE_CURRENT_LIST_DIR}/i18n/messages.json")
set(DUILIB_I18N_LANG "en")

# 读取一条消息文本：先按当前语言，缺失时回退英文，再缺失时返回 key 本身。
function(duilib_t key out_var)
    if(NOT EXISTS "${DUILIB_I18N_MESSAGES_FILE}")
        set(${out_var} "${key}" PARENT_SCOPE)
        return()
    endif()
    file(READ "${DUILIB_I18N_MESSAGES_FILE}" _duilib_i18n_json)
    set(_duilib_i18n_text "")
    if(DUILIB_I18N_LANG STREQUAL "zh_CN")
        string(JSON _duilib_i18n_text ERROR_VARIABLE _duilib_i18n_err
               GET "${_duilib_i18n_json}" messages "${key}" "zh_CN")
    endif()
    if(_duilib_i18n_text STREQUAL "" OR _duilib_i18n_err)
        string(JSON _duilib_i18n_text ERROR_VARIABLE _duilib_i18n_err
               GET "${_duilib_i18n_json}" messages "${key}" "en")
        if(_duilib_i18n_err)
            set(_duilib_i18n_text "${key}")
        endif()
    endif()
    set(${out_var} "${_duilib_i18n_text}" PARENT_SCOPE)
endfunction()

# 输出 [OK]/[check] 等状态行；value 为可选动态内容（命令、路径、版本号）。
function(duilib_msg key)
    duilib_t("${key}" _duilib_i18n_line)
    set(_duilib_i18n_extra "")
    if(ARGC GREATER 1)
        set(_duilib_i18n_extra " ${ARGV1}")
    endif()
    message(STATUS "${_duilib_i18n_line}${_duilib_i18n_extra}")
endfunction()

function(duilib_warn key)
    duilib_t("${key}" _duilib_i18n_line)
    set(_duilib_i18n_extra "")
    if(ARGC GREATER 1)
        set(_duilib_i18n_extra " ${ARGV1}")
    endif()
    message(STATUS "${_duilib_i18n_line}${_duilib_i18n_extra}")
endfunction()

function(duilib_error key hint_key)
    duilib_t("${key}" _duilib_i18n_what)
    duilib_t("${hint_key}" _duilib_i18n_hint)
    message(FATAL_ERROR "${_duilib_i18n_what}\n  Hint: ${_duilib_i18n_hint}")
endfunction()

# 动态内容版错误：what/hint 直接给出（用于拼装路径、版本号、安装命令）。
function(duilib_error_values what hint)
    message(FATAL_ERROR "[ERROR] ${what}\n  Hint: ${hint}")
endfunction()

# 判断 Windows 控制台是否为 UTF-8（65001）；无法判断时按非 UTF-8 处理。
function(_duilib_i18n_check_console out_var)
    set(${out_var} FALSE PARENT_SCOPE)
    if(NOT WIN32)
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()
    if(DEFINED ENV{DUILIB_CONSOLE_UTF8} AND "$ENV{DUILIB_CONSOLE_UTF8}" STREQUAL "1")
        set(${out_var} TRUE PARENT_SCOPE)
        return()
    endif()
    execute_process(COMMAND cmd /c chcp
                    OUTPUT_VARIABLE _duilib_i18n_chcp
                    ERROR_QUIET
                    RESULT_VARIABLE _duilib_i18n_chcp_rc)
    if(_duilib_i18n_chcp_rc EQUAL 0 AND _duilib_i18n_chcp MATCHES "([0-9]+)")
        if(CMAKE_MATCH_1 STREQUAL "65001")
            set(${out_var} TRUE PARENT_SCOPE)
        endif()
    endif()
endfunction()

# 语言判定：返回 en / zh_CN
function(_duilib_i18n_resolve out_var)
    set(_duilib_i18n_lang "")

    if(DEFINED DUILIB_LANG AND NOT "${DUILIB_LANG}" STREQUAL ""
       AND NOT DUILIB_LANG STREQUAL "auto")
        set(_duilib_i18n_lang "${DUILIB_LANG}")
    elseif(DEFINED ENV{DUILIB_LANG} AND NOT "$ENV{DUILIB_LANG}" STREQUAL ""
           AND NOT "$ENV{DUILIB_LANG}" STREQUAL "auto")
        set(_duilib_i18n_lang "$ENV{DUILIB_LANG}")
    else()
        set(_duilib_i18n_locale "")
        foreach(_duilib_i18n_env LC_ALL LC_MESSAGES LANG)
            if(DEFINED ENV{${_duilib_i18n_env}} AND NOT "$ENV{${_duilib_i18n_env}}" STREQUAL "")
                set(_duilib_i18n_locale "$ENV{${_duilib_i18n_env}}")
                break()
            endif()
        endforeach()
        if(WIN32)
            execute_process(
                COMMAND reg query "HKCU\\Control Panel\\International" /v LocaleName
                OUTPUT_VARIABLE _duilib_i18n_reg
                ERROR_QUIET
                RESULT_VARIABLE _duilib_i18n_reg_rc)
            if(_duilib_i18n_reg_rc EQUAL 0 AND
               _duilib_i18n_reg MATCHES "zh[-_]CN|zh[-_]Hans")
                set(_duilib_i18n_lang "zh_CN")
            endif()
        endif()
        if(_duilib_i18n_lang STREQUAL "" AND
           _duilib_i18n_locale MATCHES "^zh[-_]CN|^zh[-_]Hans|^zh_CN")
            set(_duilib_i18n_lang "zh_CN")
        endif()
    endif()

    if(_duilib_i18n_lang MATCHES "^zh([-_]CN|[-_]Hans)?$")
        set(_duilib_i18n_lang "zh_CN")
    else()
        set(_duilib_i18n_lang "en")
    endif()

    set(${out_var} "${_duilib_i18n_lang}" PARENT_SCOPE)
endfunction()

function(duilib_i18n_init)
    _duilib_i18n_resolve(_duilib_i18n_lang)
    if(_duilib_i18n_lang STREQUAL "zh_CN")
        _duilib_i18n_check_console(_duilib_i18n_utf8)
        if(NOT _duilib_i18n_utf8)
            set(_duilib_i18n_lang "en")
            set(DUILIB_I18N_FELL_BACK TRUE PARENT_SCOPE)
        endif()
    endif()
    set(DUILIB_I18N_LANG "${_duilib_i18n_lang}" PARENT_SCOPE)
    set(DUILIB_LANG_RESOLVED "${_duilib_i18n_lang}" PARENT_SCOPE)
endfunction()

if(NOT DEFINED DUILIB_I18N_INITIALIZED)
    duilib_i18n_init()
    set(DUILIB_I18N_INITIALIZED TRUE)
    if(DUILIB_I18N_FELL_BACK)
        duilib_t("lang.non_utf8_fallback" _duilib_i18n_fallback_text)
        message(STATUS "${_duilib_i18n_fallback_text}")
    endif()
endif()
