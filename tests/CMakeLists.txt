# duilib 单元测试
#
# 这些测试不创建窗口、不加载主题资源，只校验与界面无关的纯逻辑，
# 因此可以在命令行（含 CI）直接运行：ctest --test-dir <build dir> -C Release

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
