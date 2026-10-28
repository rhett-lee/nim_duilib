# 把 bin/resources 中按清单裁剪出的资源同步到 macOS App Bundle。
#
# 用法（由 cmake/duilib_macos_bundle.cmake 或 cmake/duilib_cef_macos.cmake 的
# 依赖目标调用）：
#   cmake -DDUILIB_RES_SRC=<bin/resources>
#         -DDUILIB_RES_DST=<App>.app/Contents/Resources/duilib/resources
#         -DDUILIB_RES_ENTRIES=<条目1|条目2|...>
#         -P cmake/duilib_copy_bundle_resources.cmake
#
# 为什么需要单独的脚本：资源复制曾挂在目标的 POST_BUILD 上，只有重新链接时才
# 执行；只改 XML/图片等资源、不改 C++ 源码时目标不会重新链接，Bundle 里就会
# 一直是旧资源，测试时看到的仍是旧界面。这里改为由每次构建都会执行的依赖目标
# 调用本脚本，并按内容（SHA256）逐个文件比较，只复制有变化的文件。
#
# 条目可以是目录或单个文件，用 `|` 分隔（避免与 CMake 列表的 `;` 冲突）。
# 目标目录按「完全由构建管理」处理：清单之外的文件会被删除，因此裁剪清单后
# 不会残留旧资源；重复构建时不会改写未变化文件的 mtime。

if(NOT DUILIB_RES_SRC)
    message(FATAL_ERROR "duilib_copy_bundle_resources: 缺少 DUILIB_RES_SRC")
endif()
if(NOT DUILIB_RES_DST)
    message(FATAL_ERROR "duilib_copy_bundle_resources: 缺少 DUILIB_RES_DST")
endif()
if(NOT DUILIB_RES_ENTRIES)
    message(FATAL_ERROR "duilib_copy_bundle_resources: 缺少 DUILIB_RES_ENTRIES")
endif()
if(NOT IS_DIRECTORY "${DUILIB_RES_SRC}")
    message(FATAL_ERROR
        "duilib_copy_bundle_resources: 源资源目录不存在：${DUILIB_RES_SRC}")
endif()

string(REPLACE "|" ";" _duilib_res_entries "${DUILIB_RES_ENTRIES}")

set(_duilib_res_expected "")
set(_duilib_res_copied 0)
set(_duilib_res_removed 0)

foreach(_duilib_res_entry IN LISTS _duilib_res_entries)
    if("${_duilib_res_entry}" STREQUAL "")
        continue()
    endif()
    set(_duilib_res_src "${DUILIB_RES_SRC}/${_duilib_res_entry}")
    set(_duilib_res_dst "${DUILIB_RES_DST}/${_duilib_res_entry}")

    if(IS_DIRECTORY "${_duilib_res_src}")
        file(GLOB_RECURSE _duilib_res_files
             LIST_DIRECTORIES false
             RELATIVE "${_duilib_res_src}"
             "${_duilib_res_src}/*")
        file(MAKE_DIRECTORY "${_duilib_res_dst}")
        foreach(_duilib_res_rel IN LISTS _duilib_res_files)
            list(APPEND _duilib_res_expected "${_duilib_res_entry}/${_duilib_res_rel}")
            set(_duilib_res_src_file "${_duilib_res_src}/${_duilib_res_rel}")
            set(_duilib_res_dst_file "${_duilib_res_dst}/${_duilib_res_rel}")
            set(_duilib_res_need_copy TRUE)
            if(EXISTS "${_duilib_res_dst_file}")
                file(SHA256 "${_duilib_res_src_file}" _duilib_res_src_hash)
                file(SHA256 "${_duilib_res_dst_file}" _duilib_res_dst_hash)
                if("${_duilib_res_src_hash}" STREQUAL "${_duilib_res_dst_hash}")
                    set(_duilib_res_need_copy FALSE)
                endif()
            endif()
            if(_duilib_res_need_copy)
                get_filename_component(_duilib_res_dst_dir
                                       "${_duilib_res_dst_file}" DIRECTORY)
                file(MAKE_DIRECTORY "${_duilib_res_dst_dir}")
                file(COPY "${_duilib_res_src_file}"
                     DESTINATION "${_duilib_res_dst_dir}")
                math(EXPR _duilib_res_copied "${_duilib_res_copied} + 1")
            endif()
        endforeach()
    elseif(EXISTS "${_duilib_res_src}")
        list(APPEND _duilib_res_expected "${_duilib_res_entry}")
        set(_duilib_res_need_copy TRUE)
        if(EXISTS "${_duilib_res_dst}")
            file(SHA256 "${_duilib_res_src}" _duilib_res_src_hash)
            file(SHA256 "${_duilib_res_dst}" _duilib_res_dst_hash)
            if("${_duilib_res_src_hash}" STREQUAL "${_duilib_res_dst_hash}")
                set(_duilib_res_need_copy FALSE)
            endif()
        endif()
        if(_duilib_res_need_copy)
            get_filename_component(_duilib_res_dst_dir
                                   "${_duilib_res_dst}" DIRECTORY)
            file(MAKE_DIRECTORY "${_duilib_res_dst_dir}")
            file(COPY "${_duilib_res_src}"
                 DESTINATION "${_duilib_res_dst_dir}")
            math(EXPR _duilib_res_copied "${_duilib_res_copied} + 1")
        endif()
    else()
        # 与 configure 期的默认清单逻辑一致：条目不存在时跳过（例如某示例没有
        # 自己的主题目录），显式清单写错已在 configure 阶段报错。
        message(STATUS
            "duilib_copy_bundle_resources: 跳过不存在的资源条目：${_duilib_res_entry}")
    endif()
endforeach()

# 删除清单之外的文件，保证 Bundle 内容与当前清单一致。
if(IS_DIRECTORY "${DUILIB_RES_DST}")
    file(GLOB_RECURSE _duilib_res_dst_files
         LIST_DIRECTORIES false
         RELATIVE "${DUILIB_RES_DST}"
         "${DUILIB_RES_DST}/*")
    foreach(_duilib_res_rel IN LISTS _duilib_res_dst_files)
        list(FIND _duilib_res_expected "${_duilib_res_rel}" _duilib_res_index)
        if(_duilib_res_index EQUAL -1)
            file(REMOVE "${DUILIB_RES_DST}/${_duilib_res_rel}")
            math(EXPR _duilib_res_removed "${_duilib_res_removed} + 1")
        endif()
    endforeach()
endif()

if(_duilib_res_copied GREATER 0 OR _duilib_res_removed GREATER 0)
    message(STATUS
        "Bundle 资源已同步：${DUILIB_RES_DST}（更新 ${_duilib_res_copied} 个，"
        "删除 ${_duilib_res_removed} 个）")
endif()
