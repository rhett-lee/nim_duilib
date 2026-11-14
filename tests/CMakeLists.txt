# duilib 单元测试
#
# 这些测试不创建窗口、不加载主题资源，只校验与界面无关的纯逻辑，
# 因此可以在命令行（含 CI）直接运行：ctest --test-dir <build dir> -C Release

# 解析第三方依赖的库路径/库名（与示例程序使用同一套规则）：
# 实例化控件的测试目标需要链接 Skia 等静态库
include("${CMAKE_CURRENT_LIST_DIR}/../cmake/duilib_common.cmake")

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


# 文字绘制相关类的单元测试：
# 覆盖文本归一化、按编码枚举/转换、字形运行构建、单字符/整段测量、
# 断行与布局、横向对齐/字间距/纵向文本测量与绘制、富文本省略号截断的绘制缓存路径。
# 该测试使用离屏渲染（不创建窗口），但依赖系统字体资源，仅在 Windows 平台构建。
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

# 覆盖离屏渲染接口（IRenderFactory::CreateOffscreenRender）：
# 创建/Resize/超大尺寸、绘制与像素正确性、快照、Clone、多后端能力查询、
# 线程与生命周期、兼容入口 CreateRender。
# 该测试不创建窗口，仅在 Windows 平台构建。
if(WIN32)
    add_executable(test_offscreen_render
        test_offscreen_render.cpp
    )
    target_include_directories(test_offscreen_render PRIVATE
        "${CMAKE_CURRENT_LIST_DIR}/../"
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
