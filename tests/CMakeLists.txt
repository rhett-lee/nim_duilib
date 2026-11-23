# duilib 单元测试
#
# 这些测试不创建窗口、不加载主题资源，只校验与界面无关的纯逻辑，
# 因此可以在命令行（含 CI）直接运行：ctest --test-dir <build dir> -C Release

# 解析第三方依赖的库路径/库名（与示例程序使用同一套规则）：
# 实例化控件的测试目标需要链接 Skia 等静态库
include("${CMAKE_CURRENT_LIST_DIR}/../cmake/duilib_common.cmake")

# SDL 模式下，测试目标需要与 duilib 库使用相同的平台宏（与 cmake/duilib_bin_windows.cmake
# 中示例目标的处理一致）：头文件里存在按平台区分的成员/类型，宏不一致会导致测试与库的
# 类布局不一致；同时测试代码也据此选择与平台匹配的输入消息模型。
if(DUILIB_ENABLE_SDL AND DUILIB_OS_WINDOWS)
    add_compile_definitions(DUILIB_SDL=1)
endif()

# SDL3 静态库在 Windows 上直接引用 Version/Winmm/Setupapi/Hid/Mincore/Dinput8 中的符号，
# 测试目标需要补齐这些系统库（与 cmake/duilib_bin_windows.cmake 中示例目标的处理一致），
# 否则 SDL 模式下测试目标链接时会报未解析的外部符号。
if(DUILIB_ENABLE_SDL AND DUILIB_OS_WINDOWS)
    list(APPEND DUILIB_SDL_LIBS Version Winmm Setupapi Hid Mincore Dinput8)
endif()

# AddressBar 路径解析：覆盖盘符、UNC、扩展长度路径、相对路径与"~"展开
add_executable(test_address_bar_path
    test_address_bar_path.cpp
)
target_include_directories(test_address_bar_path PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_address_bar_path PRIVATE duilib)
# duilib 在 Windows 下统一使用 Unicode 字符集，测试目标必须保持一致，
# 否则 DString 会是 std::string 而库中是 std::wstring（链接期符号不匹配）
if(WIN32)
    target_compile_definitions(test_address_bar_path PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_address_bar_path PRIVATE /utf-8)
endif()
add_test(NAME address_bar_path
         COMMAND test_address_bar_path
)

# Calendar 每周禁用星期（disabled_weekdays）：覆盖星期列表的解析与判断逻辑
# （Calendar::SetDisabledWeekdays / IsDateDisabled 使用的纯逻辑实现）
add_executable(test_calendar_disabled_weekdays
    test_calendar_disabled_weekdays.cpp
)
target_include_directories(test_calendar_disabled_weekdays PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_calendar_disabled_weekdays PRIVATE duilib)
if(WIN32)
    target_compile_definitions(test_calendar_disabled_weekdays PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_calendar_disabled_weekdays PRIVATE /utf-8)
endif()
add_test(NAME calendar_disabled_weekdays
         COMMAND test_calendar_disabled_weekdays
)

# RichText 纯逻辑（RichTextUtil）：覆盖 XML 转义、对齐属性、标签校验、
# 链接地址归一化与提示判断、空白裁剪、跨节点空格处理、选中文本归一化
add_executable(test_rich_text_util
    test_rich_text_util.cpp
)
target_include_directories(test_rich_text_util PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_rich_text_util PRIVATE duilib)
if(WIN32)
    target_compile_definitions(test_rich_text_util PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_rich_text_util PRIVATE /utf-8)
endif()
add_test(NAME rich_text_util
         COMMAND test_rich_text_util
)

# Label 控件：覆盖默认属性、文本/文本ID/UTF8、对齐方式、单行/多行、
# 省略号、字体与间距、纵向文本、状态颜色、文本样式、空文本尺寸估算
add_executable(test_label
    test_label.cpp
)
target_include_directories(test_label PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_label PRIVATE duilib)
# 该测试会实例化控件对象，静态库模式下需要补齐 duilib 的第三方依赖（与示例程序一致）
if(WIN32)
    target_link_directories(test_label PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_label PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_label PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_label PRIVATE ${DUILIB_CEF_LIBS})
    endif()
endif()
if(WIN32)
    target_compile_definitions(test_label PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_label PRIVATE /utf-8)
endif()
add_test(NAME label
         COMMAND test_label
)

# RichText 控件：覆盖默认属性、对齐方式、格式化文本的解析/追加/清空与序列化、
# 花括号替换、Trim策略、链接与选择接口的安全行为、空文本尺寸估算
add_executable(test_rich_text
    test_rich_text.cpp
)
target_include_directories(test_rich_text PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_rich_text PRIVATE duilib)
# 该测试会实例化控件对象，静态库模式下需要补齐 duilib 的第三方依赖（与示例程序一致）
if(WIN32)
    target_link_directories(test_rich_text PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_rich_text PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_rich_text PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_rich_text PRIVATE ${DUILIB_CEF_LIBS})
    endif()
endif()
if(WIN32)
    target_compile_definitions(test_rich_text PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_rich_text PRIVATE /utf-8)
endif()
add_test(NAME rich_text
         COMMAND test_rich_text
)

# LangManager：语言文件取值的转义规则（\\、\r、\n、\s，其他转义保持原样、
# 单次扫描避免重复解析），直接从内存加载语言表，不创建窗口
add_executable(test_lang_manager
    test_lang_manager.cpp
)
target_include_directories(test_lang_manager PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_lang_manager PRIVATE duilib)
if(WIN32)
    target_compile_definitions(test_lang_manager PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_lang_manager PRIVATE /utf-8)
endif()
add_test(NAME lang_manager
         COMMAND test_lang_manager
)

# BitmapAlpha 的单元测试：覆盖 ClearAlpha 与 RestoreAlpha（两个重载）的
# 整图/局部清除、越界裁剪、阴影边距、RGB 保持不变等全部语义
add_executable(test_bitmap_alpha
    test_bitmap_alpha.cpp
)
target_include_directories(test_bitmap_alpha PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_bitmap_alpha PRIVATE duilib)
if(WIN32)
    target_compile_definitions(test_bitmap_alpha PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_bitmap_alpha PRIVATE /utf-8)
endif()
add_test(NAME bitmap_alpha
         COMMAND test_bitmap_alpha
)

# 窗口属性解析与推导（WindowAttributes）：覆盖单趟解析、属性别名/大小写不敏感、
# 未知属性记录，以及阴影/层窗口/渲染后端/初始大小的统一推导规则，不创建窗口
add_executable(test_window_attributes
    test_window_attributes.cpp
)
target_include_directories(test_window_attributes PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_window_attributes PRIVATE duilib)
# 该测试会链接窗口属性解析所依赖的模块（DPI、阴影等），静态库模式下需要补齐第三方依赖
if(WIN32)
    target_link_directories(test_window_attributes PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_window_attributes PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_window_attributes PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_window_attributes PRIVATE ${DUILIB_CEF_LIBS})
    endif()
endif()
if(WIN32)
    target_compile_definitions(test_window_attributes PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_window_attributes PRIVATE /utf-8)
endif()
add_test(NAME window_attributes
         COMMAND test_window_attributes
)

# ColorConvert：HSV/HSL 与 RGB 的互转、取色缓冲区生成（GetRGB/HSV_*/HSL_*）
add_executable(test_color_convert
    test_color_convert.cpp
)
target_include_directories(test_color_convert PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_color_convert PRIVATE duilib)
if(WIN32)
    target_link_directories(test_color_convert PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_color_convert PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_color_convert PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_color_convert PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_color_convert PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_color_convert PRIVATE /utf-8)
endif()
add_test(NAME color_convert
         COMMAND test_color_convert
)

# Calendar 日期工具：日期比较、范围判断、字符串解析/格式化、time_t 互转
add_executable(test_calendar_date
    test_calendar_date.cpp
)
target_include_directories(test_calendar_date PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_calendar_date PRIVATE duilib)
if(WIN32)
    target_link_directories(test_calendar_date PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_calendar_date PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_calendar_date PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_calendar_date PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_calendar_date PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_calendar_date PRIVATE /utf-8)
endif()
add_test(NAME calendar_date
         COMMAND test_calendar_date
)

# TextDrawer::ToPlainText：富文本标记去除的纯文本转换
add_executable(test_text_drawer
    test_text_drawer.cpp
)
target_include_directories(test_text_drawer PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_text_drawer PRIVATE duilib)
if(WIN32)
    target_link_directories(test_text_drawer PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_text_drawer PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_text_drawer PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_text_drawer PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_text_drawer PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_text_drawer PRIVATE /utf-8)
endif()
add_test(NAME text_drawer
         COMMAND test_text_drawer
)

# 文本绘制性能基准（不注册到 ctest，属于手工运行的评估工具）
#
# 用于对比"字体回退性能优化"前后的效果，覆盖全部文本测量/绘制路径，
# 基准文本中约 1/3 的字符由回退字体渲染（运行时校验需 >= 20%）。
# 评测请使用 Release 配置，例如：
#   python build/build.py build --config Release --arch x64 --offline
#   out/x64/Release/tests/bench_text_render.exe --iterations 200
if(WIN32)
    add_executable(bench_text_render
        bench_text_render.cpp
    )
    target_include_directories(bench_text_render PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(bench_text_render PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(bench_text_render PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(bench_text_render PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(bench_text_render PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(bench_text_render PRIVATE UNICODE _UNICODE)
    # 默认资源目录：源码树的 bin/resources（可在运行时用 --resources 覆盖）
    target_compile_definitions(bench_text_render PRIVATE
        DUILIB_BENCH_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(bench_text_render PRIVATE /utf-8)
    endif()
endif()

# RichEdit2 文本绘制性能基准（不注册到 ctest，属于手工运行的评估工具）
#
# 使用离屏渲染 + 轻量窗口桩对象驱动 RichEdit2::Paint，覆盖：
#   1) 中文+英文（无字体回退）与中文+英文+大量 emoji（触发字体回退）两类文本；
#   2) 缓存有效的稳态绘制、文本变化后的重新排版绘制、滚动绘制三类场景；
#   3) 小文档（约一屏）与大文档（远大于一屏）两种规模。
# 评测请使用 Release 配置，例如：
#   python3 build/build.py build --config Release --arch x64 --offline
#   out/x64/Release/tests/bench_richedit2.exe --iterations 200
if(WIN32)
    add_executable(bench_richedit2
        bench_richedit2.cpp
    )
    target_include_directories(bench_richedit2 PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(bench_richedit2 PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(bench_richedit2 PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(bench_richedit2 PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(bench_richedit2 PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(bench_richedit2 PRIVATE UNICODE _UNICODE)
    # 默认资源目录：源码树的 bin/resources（可在运行时用 --resources 覆盖）
    target_compile_definitions(bench_richedit2 PRIVATE
        DUILIB_BENCH_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(bench_richedit2 PRIVATE /utf-8)
    endif()
endif()


# RichEdit2 纯文本内存占用基准（不注册到 ctest，属于手工运行的评估工具）
#
# 用于评估"每增加 1MB 纯文本，RichEdit2 的内存占用增加多少"：
#   1) 每个文本规模在独立进程中运行，避免不同规模的堆复用互相污染；
#   2) 分阶段采样：空白基线 -> 生成调用方文本 -> SetText 后 -> 首次绘制后 -> 稳态绘制后；
#   3) 使用 GetProcessMemoryInfo 读取进程私有提交内存与工作集。
# 评测请使用 Release 配置，例如：
#   python3 build/build.py build --config Release --arch x64 --offline --dll --target bench_richedit2_memory
#   out/x64/Release/tests/bench_richedit2_memory.exe --mb 4
if(WIN32)
    add_executable(bench_richedit2_memory
        bench_richedit2_memory.cpp
    )
    target_include_directories(bench_richedit2_memory PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(bench_richedit2_memory PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(bench_richedit2_memory PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet Psapi
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(bench_richedit2_memory PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(bench_richedit2_memory PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(bench_richedit2_memory PRIVATE UNICODE _UNICODE)
    # 默认资源目录：源码树的 bin/resources（可在运行时用 --resources 覆盖）
    target_compile_definitions(bench_richedit2_memory PRIVATE
        DUILIB_BENCH_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(bench_richedit2_memory PRIVATE /utf-8)
    endif()
endif()

# RichEdit2 编辑功能单元测试：
# 覆盖替换文本、插入字符（InsertText/键盘输入）、插入行（回车）、删除字符
# （退格/删除）、删除行、替换文本行、方向键（上/下/左/右）移动光标，
# 以及滚动切换视图时按可视区域构建的绘制缓存与全量缓存逐像素一致。
# 该测试使用离屏渲染（不创建窗口），仅在 Windows 平台构建。
# SDL 模式下整体跳过（输出 [SKIP] 并返回成功）：用例按 Windows 原生输入模型编写
# （WM_CHAR 消息、GetKeyState 模拟按键状态、"\r\n" 换行），与 SDL 的输入路径不同。
if(WIN32)
    add_executable(test_richedit2_edit
        test_richedit2_edit.cpp
    )
    target_include_directories(test_richedit2_edit PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(test_richedit2_edit PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_richedit2_edit PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_richedit2_edit PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_richedit2_edit PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_richedit2_edit PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    # 默认资源目录：源码树的 bin/resources
    target_compile_definitions(test_richedit2_edit PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(test_richedit2_edit PRIVATE /utf-8)
    endif()
    add_test(NAME richedit2_edit
             COMMAND test_richedit2_edit
    )
endif()

# ClearAlpha / RestoreAlpha / Render Clear / Render ClearRect 的性能基准
# （不注册到 ctest，属于手工运行的评估工具）。评测请使用 Release 配置，例如：
#   python3 build/build.py build --config Release --arch x64 --offline
#   out/x64/Release/tests/bench_clear_alpha.exe --iterations 200
if(WIN32)
    add_executable(bench_clear_alpha
        bench_clear_alpha.cpp
    )
    target_include_directories(bench_clear_alpha PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(bench_clear_alpha PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(bench_clear_alpha PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(bench_clear_alpha PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(bench_clear_alpha PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(bench_clear_alpha PRIVATE UNICODE _UNICODE)
    target_compile_definitions(bench_clear_alpha PRIVATE
        DUILIB_BENCH_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(bench_clear_alpha PRIVATE /utf-8)
    endif()
endif()

# 图片缩放结果缓存的性能基准（不注册到 ctest，属于手工运行的评估工具）
#
# 对比 DrawSkiaImage 的"缩放结果缓存 + 1:1绘制"开启/关闭时的单次绘制耗时，
# 覆盖静态大图、动图首轮帧序列、小图（低于缓存阈值）三类场景。
# 评测请使用 Release 配置：
#   python build/build.py build --offline --target bench_image_scale_cache
#   out/x64/Release/tests/bench_image_scale_cache.exe --iterations 1000
if(WIN32)
    add_executable(bench_image_scale_cache
        bench_image_scale_cache.cpp
    )
    target_include_directories(bench_image_scale_cache PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
        "${DUILIB_SKIA_SRC_ROOT_DIR}"
    )
    target_link_directories(bench_image_scale_cache PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(bench_image_scale_cache PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(bench_image_scale_cache PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(bench_image_scale_cache PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(bench_image_scale_cache PRIVATE
        UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN
        DUILIB_BENCH_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(bench_image_scale_cache PRIVATE /utf-8)
    endif()
endif()


# 动画帧对象复用的性能基准（不注册到 ctest，属于手工运行的评估工具）
#
# 对比 ImageInfo::GetFrame 在"帧号+目标尺寸未变化时复用上一帧对象"前后的耗时，
# 覆盖同帧重复读取、逐帧轮换、两个目标尺寸交替三类场景。
# 评测请使用 Release 配置：
#   python build/build.py build --offline --target bench_animation_frame_reuse
#   out/x64/Release/tests/bench_animation_frame_reuse.exe --iterations 200000
if(WIN32)
    add_executable(bench_animation_frame_reuse
        bench_animation_frame_reuse.cpp
    )
    target_include_directories(bench_animation_frame_reuse PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(bench_animation_frame_reuse PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(bench_animation_frame_reuse PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(bench_animation_frame_reuse PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(bench_animation_frame_reuse PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(bench_animation_frame_reuse PRIVATE UNICODE _UNICODE)
    target_compile_definitions(bench_animation_frame_reuse PRIVATE
        DUILIB_BENCH_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(bench_animation_frame_reuse PRIVATE /utf-8)
    endif()
endif()

# 文字绘制相关类的单元测试：
# 覆盖文本归一化、按编码枚举/转换、字形运行构建、单字符/整段测量、
# 断行与布局、横向对齐/字间距/纵向文本测量与绘制、富文本省略号截断的绘制缓存路径。
# 该测试使用离屏渲染（不创建窗口），但依赖系统字体资源，仅在 Windows 平台构建。
# 注意：本测试直接使用 RenderSkia 的内部实现类（ui::DrawSkiaText / ui::SkiaTextData /
# ui::Font_Skia 等），这些类没有导出（未标记 DUILIB_API）。DLL 模式下测试目标改为
# 输出 [SKIP] 并返回成功（见 test_text_draw.cpp 顶部的条件编译），用例覆盖以静态库模式为准，
# 这样 DLL 模式下 ctest 也能正常执行，不会因链接失败而缺少用例。
if(WIN32)
    add_executable(test_text_draw
        test_text_draw.cpp
    )
    target_include_directories(test_text_draw PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
        "${DUILIB_SKIA_SRC_ROOT_DIR}"
    )
    target_link_directories(test_text_draw PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_text_draw PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_text_draw PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_text_draw PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_text_draw PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    # 默认资源目录：源码树的 bin/resources（可在运行时用 --resources 覆盖）
    target_compile_definitions(test_text_draw PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(test_text_draw PRIVATE /utf-8)
    endif()
    add_test(NAME text_draw
             COMMAND test_text_draw
    )
endif()

# 图片绘制相关接口的单元测试：
# 覆盖 DrawImage 的 ImageDrawParam::rcPaint 参数（传 UiRect() 表示不限制、
# 与目标区域不相交时跳过绘制、部分相交时只绘制相交部分）。
# 该测试使用离屏渲染（不创建窗口），仅在 Windows 平台构建。
if(WIN32)
    add_executable(test_image_draw
        test_image_draw.cpp
    )
    target_include_directories(test_image_draw PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(test_image_draw PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_image_draw PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_image_draw PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_image_draw PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_image_draw PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    # 默认资源目录：源码树的 bin/resources
    target_compile_definitions(test_image_draw PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(test_image_draw PRIVATE /utf-8)
    endif()
    add_test(NAME image_draw
             COMMAND test_image_draw
    )
endif()

# 覆盖 Image 异步解码与取消逻辑：GIF 多帧异步解码(立即取消/解码部分帧后取消/恢复解码)、
# 静态 WEBP 解码期降采样、静态 WEBP/ICO 异步解码的 pending 与取消语义、
# ImageInfo::GetBitmap 的尺寸容差与内联缩放缓存。仅在 Windows 平台构建。
if(WIN32)
    add_executable(test_image_async
        test_image_async.cpp
    )
    target_include_directories(test_image_async PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(test_image_async PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_image_async PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_image_async PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_image_async PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_image_async PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    # 默认资源目录：源码树的 bin/resources
    target_compile_definitions(test_image_async PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(test_image_async PRIVATE /utf-8)
    endif()
    add_test(NAME image_async
             COMMAND test_image_async
    )
endif()

# 覆盖常用图形绘制接口（FillEllipse/DrawEllipse/FillArc/FillPolygon/DrawPolygon/
# ClipPath）与图片采样质量设置。该测试使用离屏渲染（不创建窗口），仅在 Windows 平台构建。
if(WIN32)
    add_executable(test_render_shape
        test_render_shape.cpp
    )
    target_include_directories(test_render_shape PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(test_render_shape PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_render_shape PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_render_shape PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_render_shape PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_render_shape PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    # 默认资源目录：源码树的 bin/resources
    target_compile_definitions(test_render_shape PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(test_render_shape PRIVATE /utf-8)
    endif()
    add_test(NAME render_shape
             COMMAND test_render_shape
    )
endif()

# 覆盖 IRender 扩展接口：IFont 度量/宽度/信息、IBitmap 只读查询、
# MeasureString 轻量重载、MakeImageSnapshot(rc)、SaveLayer/RestoreLayer、
# BlendMode、SetTransform、IMatrix 扩展、IPen 抗锯齿/斜接/虚线、IBrush 渐变。
# 该测试使用离屏渲染（不创建窗口），仅在 Windows 平台构建。
if(WIN32)
    add_executable(test_render_ext
        test_render_ext.cpp
    )
    target_include_directories(test_render_ext PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(test_render_ext PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_render_ext PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_render_ext PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_render_ext PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_render_ext PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    # 默认资源目录：源码树的 bin/resources
    target_compile_definitions(test_render_ext PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(test_render_ext PRIVATE /utf-8)
    endif()
    add_test(NAME render_ext
             COMMAND test_render_ext
    )
endif()

# 覆盖 IRender 中尚未被其他测试覆盖的接口：
# 窗口原点（SetWindowOrg/OffsetWindowOrg/GetWindowOrg）、IMatrix 全部变换与 ConcatMatrix、
# 裁剪（SaveClip/RestoreClip/ClipRect 交集与差集/ClipRoundRect/ClipPath/GetClipInfo/IsClipEmpty）、
# 图层、BitBlt/StretchBlt/AlphaBlend、常用图形绘制、ClearAlpha/RestoreAlpha、
# WritePixels 两个重载、ClearRect、快照与克隆、IPen/IBrush/IPath/IBitmap 属性与 Clone。
# 该测试使用离屏渲染（不创建窗口），仅在 Windows 平台构建。
if(WIN32)
    add_executable(test_render_api
        test_render_api.cpp
    )
    target_include_directories(test_render_api PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(test_render_api PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_render_api PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_render_api PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_render_api PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_render_api PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    # 默认资源目录：源码树的 bin/resources
    target_compile_definitions(test_render_api PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(test_render_api PRIVATE /utf-8)
    endif()
    add_test(NAME render_api
             COMMAND test_render_api
    )
endif()

# 覆盖 duilib/Render 目录的基础设施类：
# RichTextAttrTable（字体/颜色/图片索引表）、RichTextDataList、RichTextCharLayout
# （含超宽行 32 位偏移切换）、RichTextLineText/RichTextLineInfo（共享文本与行文本视图）、
# AutoClip（矩形/圆角/嵌套裁剪与析构恢复）、CanPresentBackBufferOnly。
# AutoClip 部分使用离屏渲染（不创建窗口），仅在 Windows 平台构建。
if(WIN32)
    add_executable(test_render_core
        test_render_core.cpp
    )
    target_include_directories(test_render_core PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(test_render_core PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_render_core PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_render_core PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_render_core PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_render_core PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    # 默认资源目录：源码树的 bin/resources
    target_compile_definitions(test_render_core PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(test_render_core PRIVATE /utf-8)
    endif()
    add_test(NAME render_core
             COMMAND test_render_core
    )
endif()

# 覆盖离屏渲染接口（IRenderFactory::CreateOffscreenRender）：
# 创建/Resize/超大尺寸、绘制与像素正确性、快照、Clone、多后端能力查询、
# 线程与生命周期、窗口控件迁移回归（需窗口，打印 SKIP）。
# 该测试不创建窗口，仅在 Windows 平台构建。
if(WIN32)
    add_executable(test_offscreen_render
        test_offscreen_render.cpp
    )
    target_include_directories(test_offscreen_render PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
        "${DUILIB_SKIA_SRC_ROOT_DIR}"
    )
    target_link_directories(test_offscreen_render PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_offscreen_render PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_offscreen_render PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_offscreen_render PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_offscreen_render PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    # 默认资源目录：源码树的 bin/resources
    target_compile_definitions(test_offscreen_render PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(test_offscreen_render PRIVATE /utf-8)
    endif()
    add_test(NAME offscreen_render
             COMMAND test_offscreen_render
    )

    # GL 多窗口冒烟测试：两个真实窗口的GL渲染、交替绘制、窗口缩放、
    # 运行过程中切换后端（GL/CPU）、离屏GPU渲染复用窗口上下文。
    # 运行环境不支持OpenGL时测试内部打印 SKIP，仍返回成功，可在CI自动执行。
    add_executable(test_gl_window_smoke
        test_gl_window_smoke.cpp
    )
    target_include_directories(test_gl_window_smoke PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(test_gl_window_smoke PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_gl_window_smoke PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_gl_window_smoke PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_gl_window_smoke PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_gl_window_smoke PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    # 默认资源目录：源码树的 bin/resources
    target_compile_definitions(test_gl_window_smoke PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(MSVC)
        target_compile_options(test_gl_window_smoke PRIVATE /utf-8)
    endif()
    add_test(NAME gl_window_smoke
             COMMAND test_gl_window_smoke
    )

    # SDL Raster 后端直绘路径测试：Skia后备缓冲直接包装SDL窗口表面的像素，
    # 覆盖整幅绘制、present-only提交、局部更新、窗口缩放、透明度回退等路径。
    if(DUILIB_ENABLE_SDL)
        add_executable(test_sdl_raster_direct
            test_sdl_raster_direct.cpp
        )
        target_include_directories(test_sdl_raster_direct PRIVATE
            "${CMAKE_CURRENT_LIST_DIR}/../"
            "${DUILIB_SDL_INCLUDE_DIR}"
        )
        target_link_directories(test_sdl_raster_direct PRIVATE
            "${DUILIB_LIB_PATH}"
            "${DUILIB_SKIA_LIB_PATH}"
        )
        target_link_libraries(test_sdl_raster_direct PRIVATE
            duilib
            ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
            Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
        )
        if(DUILIB_ENABLE_CEF)
            target_link_directories(test_sdl_raster_direct PRIVATE "${DUILIB_CEF_LIB_PATH}")
            target_link_libraries(test_sdl_raster_direct PRIVATE ${DUILIB_CEF_LIBS})
        endif()
        target_compile_definitions(test_sdl_raster_direct PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
        # 默认资源目录：源码树的 bin/resources
        target_compile_definitions(test_sdl_raster_direct PRIVATE
            DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
        )
        if(MSVC)
            target_compile_options(test_sdl_raster_direct PRIVATE /utf-8)
        endif()
        add_test(NAME sdl_raster_direct
                 COMMAND test_sdl_raster_direct
        )
    endif()
endif()

# controls 示例页面的静态校验（XML 节点名、属性、class/颜色引用、语言词条、
# Form 与 XML 的绑定关系）。该测试不创建窗口、不加载主题，仅用 Python3 解析
# 资源文件，Windows/Linux/macOS 均可运行，可直接在命令行（含 CI）执行。
find_package(Python3 3.6 COMPONENTS Interpreter QUIET)
if(Python3_Interpreter_FOUND)
    add_test(NAME controls_pages_xml
             COMMAND ${Python3_EXECUTABLE}
                     "${CMAKE_CURRENT_LIST_DIR}/test_controls_pages.py"
                     --source-root "${CMAKE_SOURCE_DIR}")
    set_tests_properties(controls_pages_xml PROPERTIES TIMEOUT 120)
else()
    message(STATUS "Python3 not found: the controls_pages_xml test is disabled")
endif()

# GL vs Raster 渲染后端性能基准（不注册到 ctest，属于手工运行的评估工具）
#
# 对比同一绘制负载在 Raster/GL 后端下的墙钟时间与 CPU 时间，识别 GPU 红利是否被
# CPU 侧开销抵消。默认 200 帧 960x600 medium 负载，Release 构建运行：
#   python build/build.py build --offline --target test_gl_benchmark
#   out/x64/Release/tests/test_gl_benchmark.exe frames=200 size=1280x800 workload=heavy
if(WIN32)
    add_executable(test_gl_benchmark
        test_gl_benchmark.cpp
    )
    target_include_directories(test_gl_benchmark PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_directories(test_gl_benchmark PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_gl_benchmark PRIVATE
        duilib
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet Psapi
    )
    target_compile_definitions(test_gl_benchmark PRIVATE
        UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_gl_benchmark PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_gl_benchmark PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    if(MSVC)
        target_compile_options(test_gl_benchmark PRIVATE /utf-8)
    endif()
endif()

# Core 基础几何类型（UiPoint/UiSize/UiRect/UiMargin/UiPadding 及浮点版本）：
# 覆盖构造、有效性校验、偏移、膨胀/收缩、并集/交集、包含判断与比较运算
add_executable(test_ui_geometry
    test_ui_geometry.cpp
)
target_include_directories(test_ui_geometry PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_ui_geometry PRIVATE duilib)
if(WIN32)
    target_compile_definitions(test_ui_geometry PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_ui_geometry PRIVATE /utf-8)
endif()
add_test(NAME ui_geometry
         COMMAND test_ui_geometry
)

# Core 字符串与字体结构（UiString/UiFont）：覆盖构造、拷贝/移动、赋值、
# 比较、清空，以及 string_view 含内嵌 '\0' 时按长度完整复制
add_executable(test_ui_string
    test_ui_string.cpp
)
target_include_directories(test_ui_string PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_ui_string PRIVATE duilib)
if(WIN32)
    target_compile_definitions(test_ui_string PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_ui_string PRIVATE /utf-8)
endif()
add_test(NAME ui_string
         COMMAND test_ui_string
)

# Core 尺寸数值类型（UiFixedInt/UiEstInt/UiTypes 工具函数）：覆盖三种数值类型、
# 拉伸百分比计算、尺寸转换、CEILF/IsFloatEqual/Truncate* 等
add_executable(test_ui_int_types
    test_ui_int_types.cpp
)
target_include_directories(test_ui_int_types PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_ui_int_types PRIVATE duilib)
if(WIN32)
    target_compile_definitions(test_ui_int_types PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_ui_int_types PRIVATE /utf-8)
endif()
add_test(NAME ui_int_types
         COMMAND test_ui_int_types
)

# Core 颜色（UiColor/StandardColorMap/ColorMap）：覆盖 ARGB 分量、COLORREF 互转、
# Hex 解析/格式化、标准颜色名查询与颜色表增删查
add_executable(test_ui_color
    test_ui_color.cpp
)
target_include_directories(test_ui_color PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_ui_color PRIVATE duilib)
# Debug 构建下 ColorMap 会访问 GlobalManager（重复添加断言），需要补齐第三方依赖
if(WIN32)
    target_link_directories(test_ui_color PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_ui_color PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_ui_color PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_ui_color PRIVATE ${DUILIB_CEF_LIBS})
    endif()
endif()
if(WIN32)
    target_compile_definitions(test_ui_color PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_ui_color PRIVATE /utf-8)
endif()
add_test(NAME ui_color
         COMMAND test_ui_color
)

# Core DPI 管理器（DpiManager）：覆盖构造默认状态、缩放变化判断、
# 100% 缩放下的数值换算与静态 MulDiv 工具函数
add_executable(test_dpi_manager
    test_dpi_manager.cpp
)
target_include_directories(test_dpi_manager PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_dpi_manager PRIVATE duilib)
# DpiManager 的其它接口依赖全局管理器与窗口系统模块，需要补齐第三方依赖
if(WIN32)
    target_link_directories(test_dpi_manager PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_dpi_manager PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_dpi_manager PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_dpi_manager PRIVATE ${DUILIB_CEF_LIBS})
    endif()
endif()
if(WIN32)
    target_compile_definitions(test_dpi_manager PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_dpi_manager PRIVATE /utf-8)
endif()
add_test(NAME dpi_manager
         COMMAND test_dpi_manager
)

# Core 颜色空间转换（ColorConverter）：覆盖 Hex/RGB 互转、RGB/HSL 互转、
# 相对亮度与对比度、RGB/OKLCH 互转及参数钳制
add_executable(test_color_converter
    test_color_converter.cpp
)
target_include_directories(test_color_converter PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_color_converter PRIVATE duilib)
if(WIN32)
    target_compile_definitions(test_color_converter PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_color_converter PRIVATE /utf-8)
endif()
add_test(NAME color_converter
         COMMAND test_color_converter
)

# Utils/StringUtil：覆盖字符串替换、大小写转换、裁剪、分割、比较、数值与字符串互转、
# 字符串复制（含超长截断）以及超长格式化输出
add_executable(test_string_util
    test_string_util.cpp
)
target_include_directories(test_string_util PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_string_util PRIVATE duilib)
if(WIN32)
    target_link_directories(test_string_util PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_string_util PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_string_util PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_string_util PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_string_util PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_string_util PRIVATE /utf-8)
endif()
add_test(NAME string_util
         COMMAND test_string_util
)

# Utils/StringConvert + StringCharset：覆盖多编码相互转换与往返一致性、
# BOM检测、ASCII/GBK/UTF-8/UTF-16流校验、字节流到字符串的转换
add_executable(test_string_convert
    test_string_convert.cpp
)
target_include_directories(test_string_convert PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_string_convert PRIVATE duilib)
if(WIN32)
    target_compile_definitions(test_string_convert PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_string_convert PRIVATE /utf-8)
endif()
add_test(NAME string_convert
         COMMAND test_string_convert
)

# Utils/FilePath + FilePathUtil + FileUtil + FileTime：覆盖路径解析/拼接/规范化、
# 父子目录判断、文件读写（含空内容）、文件时间往返与格式化
add_executable(test_file_path
    test_file_path.cpp
)
target_include_directories(test_file_path PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_file_path PRIVATE duilib)
if(WIN32)
    target_compile_definitions(test_file_path PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_file_path PRIVATE /utf-8)
endif()
add_test(NAME file_path
         COMMAND test_file_path
)

# Utils/AttributeUtil + DiskUtils + ProcessSingletonData：覆盖尺寸/矩形/边距解析、
# 百分比解析、属性列表解析、盘符提取、进程单例序列化/反序列化（含异常场景）
add_executable(test_attribute_util
    test_attribute_util.cpp
)
target_include_directories(test_attribute_util PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_attribute_util PRIVATE duilib)
# AttributeUtil 引用了窗口/全局管理器模块，静态库模式下需要补齐第三方依赖
if(WIN32)
    target_link_directories(test_attribute_util PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_attribute_util PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_attribute_util PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_attribute_util PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_attribute_util PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_attribute_util PRIVATE /utf-8)
endif()
add_test(NAME attribute_util
         COMMAND test_attribute_util
)

# Animation 目录：覆盖缓动函数（查找/名称解析/端点/对称性/取值）、
# 物理动画（弹簧/惯性积分器与全局默认参数）、AnimationPlayer 状态与同步完成、
# AnimationManager 的播放器管理、PopupAnimation 全局配置、调度器时间基准
add_executable(test_animation
    test_animation.cpp
)
target_include_directories(test_animation PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_animation PRIVATE duilib)
# 该测试会实例化控件对象（Label），静态库模式下需要补齐 duilib 的第三方依赖（与示例程序一致）
if(WIN32)
    target_link_directories(test_animation PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_animation PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_animation PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_animation PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_animation PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_animation PRIVATE /utf-8)
endif()
add_test(NAME animation
         COMMAND test_animation
)

# Box 目录：覆盖 Box 基类的子项增删/重排与 FindSelectable、
# HBox/VBox/GridBox/TileBox 布局与 GridBox 行列属性、
# ListBox 单选/多选/索引维护/框选配置、TabBox 页签选择与 InvalidIndex 回归、
# Panel 折叠布局对 min_height 的临时放宽与恢复
add_executable(test_box
    test_box.cpp
)
target_include_directories(test_box PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_box PRIVATE duilib)
# 该测试会实例化控件对象（Box/ListBox/TabBox/Panel），静态库模式下需要补齐第三方依赖
if(WIN32)
    target_link_directories(test_box PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_box PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_box PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_box PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_box PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_box PRIVATE /utf-8)
endif()
add_test(NAME box
         COMMAND test_box
)

# Layout 目录：覆盖 Layout 基类的 child_margin/child_align 属性解析与取值钳制、
# HLayout/VLayout 的固定+拉伸分配、间距与对齐、零尺寸子控件不占边距的回归，
# GridLayout 的等分单元格、HTileLayout/VTileLayout 的自动行列计算、
# HFlowLayout/VFlowLayout 的换行换列与 justify_content 分布
add_executable(test_layout
    test_layout.cpp
)
target_include_directories(test_layout PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_layout PRIVATE duilib)
# 该测试会实例化控件对象（Box/GridBox/TileBox/FlowBox），静态库模式下需要补齐第三方依赖
if(WIN32)
    target_link_directories(test_layout PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_layout PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_layout PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_layout PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_layout PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_layout PRIVATE /utf-8)
endif()
add_test(NAME layout
         COMMAND test_layout
)

# RenderSkia 目录：覆盖 SkUTF 的多编码计数/解码/编码与非法输入、
# FastVector 的容量管理与移动语义、SkiaTextData 的字符枚举、
# TextGlyphRunList 的 run 构建（含 positions 模式）、
# 位图渲染的像素读写与快照裁剪、MeasureString 的行间距规则
add_executable(test_render_skia
    test_render_skia.cpp
)
target_include_directories(test_render_skia PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
    "${DUILIB_SKIA_SRC_ROOT_DIR}"
)
target_link_libraries(test_render_skia PRIVATE duilib)
# 该测试直接使用 Skia 接口（位图像素、快照），静态库模式下需要补齐第三方依赖
if(WIN32)
    target_link_directories(test_render_skia PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_render_skia PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_render_skia PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_render_skia PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_render_skia PRIVATE UNICODE _UNICODE)
endif()
if(MSVC)
    target_compile_options(test_render_skia PRIVATE /utf-8)
endif()
target_compile_definitions(test_render_skia PRIVATE
    DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
)
add_test(NAME render_skia
         COMMAND test_render_skia
)

# Image 目录：覆盖 ImageUtil 的缩放比例/尺寸偏移换算、像素行翻转与像素缩放、
# GetBestImageScale 的最佳加载比例（含单边超出目标区域的回归用例）、
# ImageLoadParam 的固定宽高/百分比/DPI 缩放与缓存 KEY、
# ImageAttribute 的属性字符串解析与自适应矩形计算、
# StateImage 的状态图片管理、ImageDecoderUtil 的 ICO 目录解析与选择规则。
# 该测试为纯逻辑测试，不创建窗口，也不依赖图片资源。
add_executable(test_image_core
    test_image_core.cpp
)
target_include_directories(test_image_core PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/../"
)
target_link_libraries(test_image_core PRIVATE duilib)
if(WIN32)
    target_link_directories(test_image_core PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_image_core PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_image_core PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_image_core PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_image_core PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
endif()
if(MSVC)
    target_compile_options(test_image_core PRIVATE /utf-8)
endif()
add_test(NAME image_core
         COMMAND test_image_core
)

# CEFControl 目录：覆盖 CefMemoryBlock 的初始化校验、整图/脏区域增量复制、
# 尺寸变化重建、PaintData 的目标区域与裁剪、MakeImageSnapshot 的尺寸调整与参数校验，
# 以及 CefUnregistedCallbackList / CefAutoUnregister 的回调注册、注销、清空与析构语义。
# 该测试不启动 CEF 浏览器进程，只使用离屏渲染校验像素数据。
if(DUILIB_ENABLE_CEF)
    add_executable(test_cef_control
        test_cef_control.cpp
    )
    target_include_directories(test_cef_control PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_libraries(test_cef_control PRIVATE duilib)
    # 该测试会创建离屏渲染对象并读取像素，静态库模式下需要补齐第三方依赖
    if(WIN32)
        target_link_directories(test_cef_control PRIVATE
            "${DUILIB_LIB_PATH}"
            "${DUILIB_SKIA_LIB_PATH}"
            "${DUILIB_CEF_LIB_PATH}"
        )
        target_link_libraries(test_cef_control PRIVATE
            ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_CEF_LIBS}
            Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
        )
        target_compile_definitions(test_cef_control PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    endif()
    if(MSVC)
        target_compile_options(test_cef_control PRIVATE /utf-8)
    endif()
    target_compile_definitions(test_cef_control PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    add_test(NAME cef_control
             COMMAND test_cef_control
    )
endif()

# WebView2 目录：覆盖 ComPtr 的引用计数/所有权语义、ComCallback 的回调对象创建与
# Invoke 调用（引用计数不能多1，否则泄漏）、WebView2EnvironmentOptions 的接口查询与
# 属性读写、WebView2Manager 的配置与语言回退、WebView2Control 的默认状态与
# 初始URL拼接规则。该测试不启动 WebView2 运行时，也不创建窗口。
# 测试目标需要与库使用相同的 DUILIB_WEBVIEW2 宏，否则看不到 WebView2 的类型定义。
if(WIN32)
    add_executable(test_webview2
        test_webview2.cpp
    )
    target_include_directories(test_webview2 PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_libraries(test_webview2 PRIVATE duilib)
    target_link_directories(test_webview2 PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_webview2 PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_webview2 PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_webview2 PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_webview2 PRIVATE
        UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN
        DUILIB_WEBVIEW2=1
    )
    if(MSVC)
        target_compile_options(test_webview2 PRIVATE /utf-8)
    endif()
    target_compile_definitions(test_webview2 PRIVATE
        DUILIB_TEST_RESOURCE_DIR="${CMAKE_SOURCE_DIR}/bin/resources"
    )
    add_test(NAME webview2
             COMMAND test_webview2
    )
endif()

# UrlBar 浏览器地址栏控件：
#  - UrlBarUtil 纯函数：协议解析、主机名解析、网址判定、file:// URL 生成、
#    百分号编码、输入归一化（补全协议、本地路径、搜索模板）；
#  - UrlBar 控件在未加入窗口时的默认行为：控件类型、网址读写、安全状态、
#    搜索模板、历史记录管理与参数边界处理。
# 该测试不创建窗口、不加载主题资源。
if(WIN32)
    add_executable(test_url_bar
        test_url_bar.cpp
    )
    target_include_directories(test_url_bar PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
    )
    target_link_libraries(test_url_bar PRIVATE duilib)
    target_link_directories(test_url_bar PRIVATE
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    target_link_libraries(test_url_bar PRIVATE
        ${DUILIB_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_SDL_LIBS}
        Comctl32 Imm32 Opengl32 User32 shlwapi WinInet
    )
    if(DUILIB_ENABLE_CEF)
        target_link_directories(test_url_bar PRIVATE "${DUILIB_CEF_LIB_PATH}")
        target_link_libraries(test_url_bar PRIVATE ${DUILIB_CEF_LIBS})
    endif()
    target_compile_definitions(test_url_bar PRIVATE UNICODE _UNICODE NOMINMAX WIN32_LEAN_AND_MEAN)
    if(MSVC)
        target_compile_options(test_url_bar PRIVATE /utf-8)
    endif()
    add_test(NAME url_bar
             COMMAND test_url_bar
    )
endif()

# 静态库模式下，实例化控件/渲染相关对象的测试目标需要补齐 duilib 的第三方依赖；
# Windows 已在各测试目标处单独配置（还需额外的系统库与 CEF），这里为非 Windows 平台
# 统一补齐：链接 libduilib.a 时若不显式链接 SDL3/Skia 等库，会报
# SDL_DestroyCursor、SkSVGDOM::MakeFromStream 等未定义符号。
# 链接列表与 cmake/duilib_bin_linux.cmake、duilib_bin_macos.cmake、
# duilib_bin_freebsd.cmake 中示例目标的配置保持一致。
if(NOT DUILIB_OS_WINDOWS)
    set(_duilib_test_link_dirs
        "${DUILIB_LIB_PATH}"
        "${DUILIB_SKIA_LIB_PATH}"
    )
    if(DUILIB_ENABLE_SDL)
        list(APPEND _duilib_test_link_dirs "${DUILIB_SDL_LIB_PATH}")
    endif()

    # DUILIB_LIBS 含 duilib 自身，而各测试目标前面已单独链接 duilib 目标；
    # 若这里再展开一次，macOS/Linux 的 ld 会告警 "ignoring duplicate libraries"
    set(_duilib_test_third_party_libs ${DUILIB_LIBS})
    list(REMOVE_ITEM _duilib_test_third_party_libs duilib)
    set(_duilib_test_platform_libs
        ${_duilib_test_third_party_libs} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS}
    )
    if(DUILIB_OS_LINUX OR DUILIB_OS_FREEBSD)
        # Linux/FreeBSD：X11 与字体相关依赖（统一使用 imported target）
        include("${CMAKE_CURRENT_LIST_DIR}/../cmake/duilib_unix_font_libs.cmake")
        find_package(X11 REQUIRED)
        list(APPEND _duilib_test_platform_libs
            X11::X11 ${DUILIB_FONT_LIBS} pthread dl
        )
    elseif(DUILIB_OS_MACOS)
        # macOS：系统框架（与 cmake/duilib_bin_macos.cmake 保持一致）
        find_library(_duilib_test_accelerate Accelerate)
        find_library(_duilib_test_corefoundation CoreFoundation)
        find_library(_duilib_test_coretext CoreText)
        find_library(_duilib_test_coregraphics CoreGraphics)
        list(APPEND _duilib_test_platform_libs
            ${_duilib_test_accelerate} ${_duilib_test_corefoundation}
            ${_duilib_test_coretext} ${_duilib_test_coregraphics}
            pthread dl
            "-framework AppKit" "-framework Foundation"
            "-framework Metal" "-framework Cocoa"
        )
    endif()

    get_property(_duilib_test_targets DIRECTORY PROPERTY BUILDSYSTEM_TARGETS)
    foreach(_duilib_test_target IN LISTS _duilib_test_targets)
        target_link_directories(${_duilib_test_target} PRIVATE ${_duilib_test_link_dirs})
        target_link_libraries(${_duilib_test_target} PRIVATE ${_duilib_test_platform_libs})
    endforeach()
endif()
