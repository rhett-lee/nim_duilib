## 第三方库的源码说明
1. third_party目录中放的是第三方库的源码；
2. 一般不会在项目中修改第三方的源码，以免后续升级时遇到困难；
3. 第三方库的源码，一般会定期升级到最新版本，以提供最优的功能，减少缺陷。

## 当前各个第三方库的使用情况
| 名称        | 更新日期   | 版本号|是否修改| 获取URL | 备注 |
| :---        | :---       | :---  |:---    | :---    |:---  |
| apng        | 2024-05-30 |       |无 ||暂未记录出处，通过libpng + libpng-1.6.43-apng.patch来支持的 |
| libpng      | 2024-05-30 |v1.6.43|有 |https://sourceforge.net/projects/libpng/|参见后续说明|
| cximage     | 2024-05-30 |v7.02  |无 |https://sourceforge.net/projects/cximage/|2011-02-11后已停止更新|
| libwebp     | 2024-05-30 |v1.4.0 |无 |https://github.com/webmproject/libwebp|tag/v1.4.0，config.h文件需要用cmake生成后再更新，使用了src目录下的子目录，覆盖后，删除不必要的文件即可|
| stb_image   | 2024-05-30 |v2.29  |无 |https://github.com/nothings/stb||
| svg         | 2024-05-30 |无     |有 |https://github.com/memononen/nanosvg| 代码更新到2023-12-30，修改参见提交记录|
| zlib        | 2024-05-30 |v1.3.1 |无 |https://github.com/madler/zlib | |
| xml         | 2024-05-30 |v1.14  |无 |https://github.com/zeux/pugixml| pugixml|
| convert_utf | 2024-05-30 |v18.1.6|无 |https://releases.llvm.org      | 下载最新版的源码包，解压后找到这两个文件  |

## libpng 库更新说明
1. 下载最新版本的压缩包：https://sourceforge.net/projects/libpng/
2. 下载压缩包后，解压，进入解压后的目录，按照INSTALL的说明，使用VS的命令行模式，运行：`nmake -f scripts\makefile.vcwin32`，会生成pnglibconf.h文件
3. 然后整个目录（去除.lib和.obj文件），覆盖`duilib\third_party\libpng`里面的所有文件，里面的文件与下载的压缩包保持一致，多删少补，新增加的文件要添加到git库
3. 注意检查`pnglibconf.h`文件，保持必要的同步
4. 注意`libpng.vcxproj`和`pnglibconf.vcxproj`工程文件不轻易更新，除非有文件变化时需要同步
5. 对代码打支持APNG的补丁：
 - 补丁下载地址：https://sourceforge.net/projects/libpng-apng/ ，如果找不到对应的补丁，可用搜索引擎搜`libpng PNG_APNG_SUPPORTED`，尝试查找
 - 如果找不到对应版本的补丁，可以考虑暂缓更新libpng
 - 更新步骤1：将`libpng-1.6.44-apng.patch.gz`文件解压出`libpng-1.6.44-apng.patch`文件，放在`duilib\third_party\libpng`目录中
 - 更新步骤2：命令行模式，进入`duilib\third_party\libpng`目录，运行命令：`git apply .\libpng-1.6.44-apng.patch`
 - 更新步骤3：补丁打完以后，检查、测试并提交。
 
