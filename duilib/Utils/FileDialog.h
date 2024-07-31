#ifndef UI_UTILS_FILE_DIALOG_H_
#define UI_UTILS_FILE_DIALOG_H_

#include "duilib/Utils/FilePath.h"
#include <string>
#include <vector>

namespace ui
{

/** 文件或者目录选择对话框，支持多选
*/
class Window;
class UILIB_API FileDialog
{
public:
    /** 选择文件夹（单选）
    * @param [in] pWindow 父窗口
    * @param [out] folderPath 返回选择的文件夹路径
    * @param [in] defaultLocation 初始选择的文件夹路径(可选参数)
    */
    bool BrowseForFolder(Window* pWindow, FilePath& folderPath,
                         const FilePath& defaultLocation = FilePath());

    /** 选择文件夹（多选）
    * @param [in] pWindow 父窗口
    * @param [out] folderPaths 返回选择的文件夹路径
    * @param [in] defaultLocation 初始选择的文件夹路径(可选参数)
    */
    bool BrowseForFolders(Window* pWindow, std::vector<FilePath>& folderPaths,
                          const FilePath& defaultLocation = FilePath());

public:
    /** 文件类型筛选器
    */
    struct FileType
    {
        /** 文件类型筛选器的显示名称, 举例: "Text files"
        *   编码规则：Unicode版本为UTF16编码，非Unicode版本为UTF8编码
        */
        DString szName;

        /** 文件类型筛选器, 举例："*.txt"
        *   编码规则：Unicode版本为UTF16编码，非Unicode版本为UTF8编码
        */
        DString szExt;
    };

    /** 选择文件（单选）
    * @param [in] pWindow 父窗口
    * @param [in] bOpenFileDialog true表示打开文件，false表示保存文件
    * @param [in] fileTypes 对话框可以打开或保存的文件类型
    * @param [in] nFileTypeIndex 选择的文件类型，有效范围：[0, fileTypes.size())
    * @param [in] defaultExt 默认的文件类型, 举例："doc;docx", 编码规则：Unicode版本为UTF16编码，非Unicode版本为UTF8编码
    * @param [in] fileName 当前的文件名
    * @param [in] defaultLocation 初始的文件夹路径(可选参数)
    * @param [out] filePath 返回选择的文件路径
    */
    bool BrowseForFile(Window* pWindow, 
                       FilePath& filePath,
                       bool bOpenFileDialog, 
                       const std::vector<FileType>& fileTypes = std::vector<FileType>(),
                       int32_t nFileTypeIndex = -1,
                       const DString& defaultExt = _T(""),
                       const DString& fileName = _T(""),
                       const FilePath& defaultFilePath = FilePath());

    /** 选择文件（多选）
    * @param [in] pWindow 父窗口
    * @param [in] fileTypes 对话框可以打开的文件类型
    * @param [in] nFileTypeIndex 选择的文件类型，有效范围：[0, fileTypes.size())
    * @param [in] defaultExt 默认的文件类型, 举例："doc;docx", 编码规则：Unicode版本为UTF16编码，非Unicode版本为UTF8编码
    * @param [in] defaultLocation 初始的文件夹路径(可选参数)
    * @param [out] filePaths 返回选择的文件路径
    */
    bool BrowseForFiles(Window* pWindow, 
                        std::vector<FilePath>& filePaths,
                        const std::vector<FileType>& fileTypes = std::vector<FileType>(),
                        int32_t nFileTypeIndex = -1,
                        const DString& defaultExt = _T(""),
                        const FilePath& defaultLocation = FilePath());

};

}

#endif // UI_UTILS_FILE_DIALOG_H_
