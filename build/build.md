# 项目编译相关文档和脚本
该目录中主要包含了项目编译的相关文档和依赖的脚本文件，假设项目的根目录为变量${DUILIB_ROOT}。
## 该目录中的文件列表
| FileName/文件名               | OS/操作系统   | Compiler/编译器        |  Comment/备注   |
| :---                          | :---          |:---                    |:---             |
| `build_duilib_all_in_one.bat` | Windows       |Visual Studio 2022      |一键编译脚本，脚本完成所有源码下载（含skia等依赖项）和编译<br>默认编译内容：<br>skia: llvm.x64.debug/llvm.x64.release/llvm.x86.release/llvm.x86.debug|
| `duilib.sln`                  | Windows       |Visual Studio 2022      |duilib的Visual Studio的解决方案文件，只包含lib，不包含示例程序|
| `examples.sln`                | Windows       |Visual Studio 2022      |示例程序的Visual Studio的解决方案文件，包含lib，包含示例程序|
| `msvc_build.bat`              | Windows       |cmake/MSVC              |命令行编译脚本，使用cmake编译，编译器为MSVC|
| `gcc-mingw-w64_build.bat`     | Windows       |MinGW-w64 gcc/g++       |MinGW-w64的编译脚本，编译器为gcc/g++|
| `llvm-mingw-w64_build.bat`    | Windows       |MinGW-w64 clang/clang++ |MinGW-w64的编译脚本，编译器为clang/clang++|
| `build_duilib_all_in_one.sh`  | Windows       |MSYS2 gcc/g++ or clang/clang++|一键编译脚本，脚本完成所有源码下载（含skia等依赖项）和编译，编译器为gcc/g++ 或者 clang/clang++|
| `msys2_build.sh`              | Windows       |MSYS2 gcc/g++ or clang/clang++|MSYS2系统下的编译脚本，编译器为gcc/g++ 或者 clang/clang++|
| `build_duilib_all_in_one.sh`  | Linux         |gcc/g++ or clang/clang++|一键编译脚本，脚本完成所有源码下载（含skia等依赖项）和编译，编译器为gcc/g++ 或者 clang/clang++|
| `linux_build.sh`              | Linux         |gcc/g++ or clang/clang++|Linux系统下的编译脚本，编译器为gcc/g++ 或者 clang/clang++|
| `build_duilib_all_in_one.sh`  | MacOS         |clang/clang++           |一键编译脚本，脚本完成所有源码下载（含skia等依赖项）和编译，编译器为gcc/g++ 或者 clang/clang++|
| `macos_build.sh`              | MacOS         |clang/clang++           |MacOS系统下的编译脚本，编译器为clang/clang++|
| `build_duilib_all_in_one.sh`  | FreeBSD       |gcc/g++ or clang/clang++|一键编译脚本，脚本完成所有源码下载（含skia等依赖项）和编译，编译器为gcc/g++ 或者 clang/clang++|
| `freebsd_build.sh`            | FreeBSD       |gcc/g++ or clang/clang++|FreeBSD系统下的编译脚本，编译器为gcc/g++ 或者 clang/clang++|

## 编译的目标文件
1. lib文件：`${DUILIB_ROOT}/lib`
2. bin文件：`${DUILIB_ROOT}/bin`
3. 临时文件（可清理）    
（1）`${DUILIB_ROOT}/build/build_temp`: 编译的临时目录，可清理    
（2）`${DUILIB_ROOT}/build/.vs`: 隐藏的目录，Visual Studio 生成的缓存目录，占有空间很大，且越来越大，可定期清理。    
（3）`${DUILIB_ROOT}/cef_temp`: libCEF模块运行时的网络缓存目录，可删除。
    
## 程序发布时所依赖的文件
1. `${DUILIB_ROOT}/bin/resources`目录：保存的资源文件（XML文件、图片资源等）    
（1）`${DUILIB_ROOT}/bin/resources/fonts`：字体目录，如果未使用自定义字体，此目录可删除。    
（2）`${DUILIB_ROOT}/bin/resources/lang`：多语言版的文字文件，如果未使用多语相关内容，此目录可删除。    
（3）`${DUILIB_ROOT}/bin/resources/themes/default`：这个是主题资源目录，只保留`public`目录和`global.xml`文件，其他目录均可删除。    
2. Windows平台，如果需要将资源打包成zip，放在exe的资源中：    
   可以将第1步整理好的`bin/resources`目录，打包为`resources.zip`文件，替换现在`bin/resources.zip`文件，再重新编译exe即可。    
   关于使用`resources.zip`的方法，可以参考`examples/basic`示例程序的工程代码。    
3. 如果使用了CEF模块：需要将CEF的Release目录和Resources目录里面的文件放到`bin\libcef_win`或者`bin\libcef_win_109`目录。    
   详细内容可参考CEF的使用说明文档：[docs/CEF.md](../docs/CEF.md)。
