## 第三方库的源码说明
1. third_party目录中放的是第三方库的源码；
2. 一般不会在项目中修改第三方的源码，以免后续升级时遇到困难；
3. 第三方库的源码，一般会定期升级到最新版本，以提供最优的功能，减少缺陷。

## 当前各个第三方库的使用情况
| 名称        | 更新日期   | 版本号|是否修改| 获取URL | 备注 |
| :---        | :---       | :---  |:---    | :---    |:---  |
| apng        | 2025-03-10 |v1.6.47|无 |https://sourceforge.net/projects/libpng-apng/ |通过libpng + libpng-1.6.47-apng.patch来支持的 |
| libpng      | 2025-03-10 |v1.6.47|有 |https://sourceforge.net/projects/libpng/|参见后续说明|
| cximage     | 2024-05-30 |v7.02  |无 |https://sourceforge.net/projects/cximage/|2011-02-11后已停止更新|
| libwebp     | 2025-03-11 |v1.5.0 |无 |https://github.com/webmproject/libwebp|tag/v1.5.0，使用了src目录下的子目录，覆盖后，删除不必要的文件即可（基本方法：新旧版本源码比较，然后看文件增删改，再同步，项目只使用了图片解码功能，没有用到图片编码功能，目前没有用到config.h文件）|
| stb_image   | 2025-03-11 |v2.30  |无 |https://github.com/nothings/stb|无修改，直接取master的代码|
| svg         | 2025-03-11 |无（2024-12-20）     |有 |https://github.com/memononen/nanosvg| 代码更新到2024-12-20，修改参见提交记录|
| zlib        | 2025-03-11 |v1.3.1（2024-01-22） |有 |https://github.com/madler/zlib | 修改了zlib.h 和 CMakeLists.txt，并将zconf.h重命名为zconf_msvc.h|
| xml         | 2025-03-11 |v1.15（2025-01-11）  |无 |https://github.com/zeux/pugixml| pugixml|
| convert_utf | 2025-03-11 |v20.1.0|无 |https://releases.llvm.org      | 下载最新版的源码包，解压后找到这两个文件  |
| udis86      | 2025-04-11 |v1.7.2 |无 |https://sourceforge.net/projects/udis86/      |  |
| libcef_win_109|2025-03-10|cef_binary 109.1.18+gf1c41e4<br>chromium-109.0.5414.120（2023-02-03）|无 |x64版本：https://cef-builds.spotifycdn.com/index.html#windows64:109 <br> Win32版本：https://cef-builds.spotifycdn.com/index.html#windows32:109| x64版本：https://cef-builds.spotifycdn.com/cef_binary_109.1.18%2Bgf1c41e4%2Bchromium-109.0.5414.120_windows64.tar.bz2 <br> Win32版本：https://cef-builds.spotifycdn.com/cef_binary_109.1.18%2Bgf1c41e4%2Bchromium-109.0.5414.120_windows32.tar.bz2|
| libcef_win    |2025-03-10|cef_binary_133.4.8+g99a2ab1<br>chromium-133.0.6943.142（2023-03-08）|无 |x64版本：https://cef-builds.spotifycdn.com/index.html#windows64:133 <br> Win32版本：https://cef-builds.spotifycdn.com/index.html#windows32:133| x64版本：https://cef-builds.spotifycdn.com/cef_binary_133.4.8%2Bg99a2ab1%2Bchromium-133.0.6943.142_windows64.tar.bz2 <br> Win32版本：https://cef-builds.spotifycdn.com/cef_binary_133.4.8%2Bg99a2ab1%2Bchromium-133.0.6943.142_windows32.tar.bz2|
| libcef_linux  |2025-03-10|cef_binary_133.4.8+g99a2ab1<br>chromium-133.0.6943.142（2023-03-08）|无 |x64版本：https://cef-builds.spotifycdn.com/index.html#linux64:133| x64版本：https://cef-builds.spotifycdn.com/cef_binary_133.4.8%2Bg99a2ab1%2Bchromium-133.0.6943.142_linux64.tar.bz2|

## libpng 库更新说明
1. 下载最新版本的压缩包：https://sourceforge.net/projects/libpng/
2. 下载压缩包后，解压，进入解压后的目录，按照INSTALL的说明，使用VS的命令行模式，运行：`nmake -f scripts\makefile.vcwin32`，会生成`pnglibconf.h`文件
3. 然后整个目录（去除.lib和.obj文件），覆盖`duilib\third_party\libpng`里面的所有文件，里面的文件与下载的压缩包保持一致，多删少补，新增加的文件要添加到git库
3. 注意检查`pnglibconf.h`文件，保持必要的同步
4. 注意`libpng\projects\vstudio\libpng\libpng.vcxproj`和`libpng\projects\vstudio\zlib\zlib.vcxproj`工程文件不轻易更新，除非有文件变化时需要同步（可以用新版和旧版的官方包，在两个目录中分别运行`nmake -f scripts\makefile.vcwin32`，然后比较生成的vcxproj是否有变化，然后决定是否要修改）
5. 对代码打支持APNG的补丁：
 - 补丁下载地址：https://sourceforge.net/projects/libpng-apng/ ，如果找不到对应的补丁，可用搜索引擎搜`libpng PNG_APNG_SUPPORTED`，尝试查找
 - 如果找不到对应版本的补丁，可以考虑暂缓更新libpng
 - 更新步骤1：将`libpng-1.6.47-apng.patch.gz`文件解压出`libpng-1.6.47-apng.patch`文件，放在`duilib\third_party\libpng`目录中
 - 更新步骤2：命令行模式，进入`duilib\third_party\libpng`目录，运行命令：`git apply .\libpng-1.6.47-apng.patch`
 - 更新步骤3：补丁打完以后，检查、测试并提交。
 
