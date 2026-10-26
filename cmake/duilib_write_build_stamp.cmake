# 由示例目标的 POST_BUILD 步骤以脚本模式调用：
# 把目标构建时的配置写进 bin/<目标>.build-config。
#
# Debug 与 Release 的可执行文件同名，切换配置后若没有全部重新链接，bin/ 里会混
# 有两种配置的产物；build/pack.py 通过该标记文件判断并跳过/提示，避免打出的
# Debug 包混入 Release 示例（或反之）。
if(NOT DEFINED STAMP_FILE OR "${STAMP_FILE}" STREQUAL "")
    message(FATAL_ERROR "duilib_write_build_stamp: 未指定 STAMP_FILE")
endif()
if(NOT DEFINED STAMP_CONFIG OR "${STAMP_CONFIG}" STREQUAL "")
    message(FATAL_ERROR "duilib_write_build_stamp: 未指定 STAMP_CONFIG")
endif()
file(WRITE "${STAMP_FILE}" "${STAMP_CONFIG}\n")
