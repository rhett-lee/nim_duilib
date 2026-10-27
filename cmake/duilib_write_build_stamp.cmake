# 由示例目标的 POST_BUILD 步骤以脚本模式调用：
# 把目标构建时的配置写进 bin/.build-config/<目标>。
#
# Debug 与 Release 的可执行文件同名，切换配置后若没有全部重新链接，bin/ 里会混
# 有两种配置的产物；build/pack.py 通过该标记文件判断并跳过/提示，避免打出的
# Debug 包混入 Release 示例（或反之）。标记集中放在隐藏目录，避免 bin/ 根目录
# 散落大量同名附属文件。
if(NOT DEFINED STAMP_FILE OR "${STAMP_FILE}" STREQUAL "")
    message(FATAL_ERROR "duilib_write_build_stamp: 未指定 STAMP_FILE")
endif()
if(NOT DEFINED STAMP_CONFIG OR "${STAMP_CONFIG}" STREQUAL "")
    message(FATAL_ERROR "duilib_write_build_stamp: 未指定 STAMP_CONFIG")
endif()
get_filename_component(_stamp_dir "${STAMP_FILE}" DIRECTORY)
get_filename_component(_stamp_name "${STAMP_FILE}" NAME)
get_filename_component(_bin_dir "${_stamp_dir}" DIRECTORY)
file(MAKE_DIRECTORY "${_stamp_dir}")
file(WRITE "${STAMP_FILE}" "${STAMP_CONFIG}\n")

# 旧版本把标记写在 bin/ 根目录（bin/<目标>.build-config），这里顺手删除，
# 升级后重新链接过的目标不会在 bin/ 里留下旧文件。
set(_legacy_stamp "${_bin_dir}/${_stamp_name}.build-config")
if(EXISTS "${_legacy_stamp}")
    file(REMOVE "${_legacy_stamp}")
endif()
