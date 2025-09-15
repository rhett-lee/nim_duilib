#if defined(__GNUC__) && !defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wformat"
    #pragma GCC diagnostic ignored "-Wformat-extra-args"
#endif

#if defined (_MSC_VER)
    #pragma warning (push)
    #pragma warning (disable: 4244 4201 4100 4267 5030)
#endif
