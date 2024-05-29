#ifndef UI_UTILS_FILE_DIALOG_H_
#define UI_UTILS_FILE_DIALOG_H_

#include "duilib/duilib_defs.h"
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
    */
    bool BrowseForFolder(Window* pWindow, DString& folderPath);

    /** 选择文件夹（多选）
    * @param [in] pWindow 父窗口
    * @param [out] folderPaths 返回选择的文件夹路径
    */
    bool BrowseForFolders(Window* pWindow, std::vector<DString>& folderPaths);

public:
    /** 文件类型筛选器
    */
    struct FileType
    {
        DString szName; //文件类型筛选器的显示名称, 举例: "Text files"
        DString szExt;  //文件类型筛选器, 举例："*.txt"
    };

    /** 选择文件（单选）
    * @param [in] pWindow 父窗口
    * @param [in] bOpenFileDialog true表示打开文件，false表示保存文件
    * @param [in] fileTypes 对话框可以打开或保存的文件类型
    * @param [in] nFileTypeIndex 选择的文件类型，有效范围：[0, fileTypes.size())
    * @param [in] defaultExt 默认的文件类型, 举例："doc;docx"
    * @param [out] filePath 返回选择的文件路径
    */
    bool BrowseForFile(Window* pWindow, 
                       DString& filePath,                       
                       bool bOpenFileDialog, 
                       const std::vector<FileType>& fileTypes = std::vector<FileType>(),
                       int32_t nFileTypeIndex = -1,
                       const DString& defaultExt = _T(""));

    /** 选择文件（多选）
    * @param [in] pWindow 父窗口
    * @param [in] fileTypes 对话框可以打开或保存的文件类型
    * @param [in] nFileTypeIndex 选择的文件类型，有效范围：[0, fileTypes.size())
    * @param [in] defaultExt 默认的文件类型, 举例："doc;docx"
    * @param [out] filePaths 返回选择的文件路径
    */
    bool BrowseForFiles(Window* pWindow, 
                        std::vector<DString>& filePaths,                        
                        const std::vector<FileType>& fileTypes = std::vector<FileType>(),
                        int32_t nFileTypeIndex = -1,
                        const DString& defaultExt = _T(""));

};

}

#endif // UI_UTILS_FILE_DIALOG_H_
