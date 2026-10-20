# Linux / FreeBSD 公共的字体相关依赖（CM-13）
#
# 使用方式：
#   include("${CMAKE_CURRENT_LIST_DIR}/duilib_unix_font_libs.cmake")
#   target_link_libraries(<target> ${DUILIB_FONT_LIBS})
#
# 统一使用 imported target，避免裸库名依赖链接器的搜索路径
# （裸库名在换发行版或交叉编译时容易找不到库，且报错信息不明确）。

find_package(Freetype REQUIRED)
find_package(Fontconfig REQUIRED)

set(DUILIB_FONT_LIBS Freetype::Freetype Fontconfig::Fontconfig)
