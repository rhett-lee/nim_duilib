# DUILIB_OS_MACOS
if(NOT DUILIB_OS_MACOS)
    message(FATAL_ERROR "NOT DUILIB_OS_MACOS!")
endif()

# MacOS平台：查找必需框架
find_library(ACCELERATE Accelerate)
find_library(COREFOUNDATION CoreFoundation)
find_library(CORETEXT CoreText)
find_library(COREGRAPHICS CoreGraphics)

# 平台的标准库
set(DUILIB_MACOS_LIBS pthread dl)

if(DUILIB_ENABLE_CEF)
    # CEF在MACOS的实现比较复杂，相关的代码单独放一个文件中
    include("${CMAKE_CURRENT_LIST_DIR}/duilib_cef_macos.cmake") 
else()
    # 保持编译器的参数与CEF内部设置一致
    set(DUILIB_COMPILER_FLAGS
        -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
        -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
        -funwind-tables                 # Support stack unwinding for backtrace()
        -fvisibility=hidden             # Give hidden visibility to declarations that are not explicitly marked as visible
        -Wall                           # Enable all warnings
        -Werror                         # Treat warnings as errors
        -Wno-unknown-pragmas
        -Wextra                         # Enable additional warnings
        -Wendif-labels                  # Warn whenever an #else or an #endif is followed by text
        -Wno-missing-field-initializers # Don't warn about missing field initializers
        -Wno-unused-parameter           # Don't warn about unused parameters
    )
    set(DUILIB_CXX_COMPILER_FLAGS
        -fno-threadsafe-statics         # Don't generate thread-safe statics
        -fvisibility-inlines-hidden     # Give hidden visibility to inlined class member functions
        -frtti
        -Wno-narrowing                  # Don't warn about type narrowing
        -Wsign-compare                  # Warn about mixed signed/unsigned type comparisons
        -Wno-unused-variable
    )
    
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND DUILIB_CXX_COMPILER_FLAGS
            -Wno-reorder
          )
    endif()
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        list(APPEND DUILIB_COMPILER_FLAGS
            -Wnewline-eof                   # Warn about no newline at end of file
          )
        list(APPEND DUILIB_CXX_COMPILER_FLAGS
            -fobjc-call-cxx-cdtors          # Call the constructor/destructor of C++ instance variables in ObjC objects
            -Wno-reorder-ctor
            -Wno-unused-lambda-capture
          )
    endif()
    
    #设置编译可执行程序依赖的源码
    add_executable(${PROJECT_NAME} ${SRC_FILES})
    
    #设置编译器的参数
    target_compile_options(${PROJECT_NAME} PRIVATE ${DUILIB_COMPILER_FLAGS} ${DUILIB_CXX_COMPILER_FLAGS})
    
    # MacOS平台：设置链接库（注意顺序！）
    target_link_libraries(${PROJECT_NAME}
                            # 第三方库（按依赖顺序）
                            ${DUILIB_LIBS} ${DUILIB_SDL_LIBS} ${DUILIB_SKIA_LIBS} ${DUILIB_CEF_LIBS}
                            # 系统库
                            ${ACCELERATE} ${COREFOUNDATION} ${CORETEXT} ${COREGRAPHICS} ${DUILIB_MACOS_LIBS}
                            # 显式框架声明（必须放在最后）
                            "-framework AppKit" "-framework Foundation" "-framework Metal"
                          )
endif()
