# 用于检测一些编译参数
include(CheckCXXSourceCompiles)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    # 检测 AVX 支持
    set(CMAKE_REQUIRED_FLAGS "-mavx")
    check_cxx_source_compiles("
        #include <immintrin.h>
        int main() {
            __m256 a = _mm256_set1_ps(0.0f);
            return 0;
        }
    " DUILIB_HAVE_AVX)

    # 检测 AVX2 支持
    set(CMAKE_REQUIRED_FLAGS "-mavx2")
    check_cxx_source_compiles("
        #include <immintrin.h>
        int main() {
            __m256i a = _mm256_set1_epi32(0);
            return 0;
        }
    " DUILIB_HAVE_AVX2)
endif()