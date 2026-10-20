# 公共的编译器参数（GCC / Clang）
#
# 说明：macOS 的 duilib 库、macOS 的示例程序（含 CEF 分支）、Linux/FreeBSD 的示例程序
#       共用这一份定义，避免多处各写一遍导致参数漂移（CM-10）。
#
# 使用方式：
#   include("${DUILIB_SRC_ROOT_DIR}/cmake/duilib_compiler_flags.cmake")
#   结果变量：DUILIB_COMPILER_FLAGS / DUILIB_CXX_COMPILER_FLAGS
#   如需不同的变量名（例如 CEF 分支的 CEF_COMPILER_FLAGS），直接拷贝这两个变量即可。

set(DUILIB_COMPILER_FLAGS
    -fno-strict-aliasing            # Avoid assumptions regarding non-aliasing of objects of different types
    -fstack-protector               # Protect some vulnerable functions from stack-smashing (security feature)
    -funwind-tables                 # Support stack unwinding for backtrace()
    -fvisibility=hidden             # Give hidden visibility to declarations that are not explicitly marked as visible
    -Wall                           # Enable all warnings
#    -Werror                         # Treat warnings as errors
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
        -Wno-unused-private-field
    )
endif()
