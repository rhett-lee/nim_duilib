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
