#ifndef UI_CONTROL_RICHEDIT_DRAG_SOURCE_WINDOWS_H_
#define UI_CONTROL_RICHEDIT_DRAG_SOURCE_WINDOWS_H_

#include "duilib/Core/UiTypes.h"

#if defined(DUILIB_BUILD_FOR_WIN)

#include <objidl.h>

namespace ui 
{

class RichEdit2;

/**
 * @brief RichEdit控件的拖放源类（Windows平台）
 * @details 实现IDropSource接口，用于支持从RichEdit控件拖出文本的功能
 */
class RichEditDragSource_Windows : public IDropSource
{
public:
    /** 创建拖放源对象
     * @return 返回新创建的拖放源对象指针
     */
    static RichEditDragSource_Windows* Create();

    /** 构造函数
     * @param [in] pRichEdit 关联的RichEdit2控件指针
     */
    explicit RichEditDragSource_Windows(RichEdit2* pRichEdit);

    // IUnknown 接口实现
    HRESULT __stdcall QueryInterface(REFIID iid, void** object) override;
    ULONG __stdcall AddRef() override;
    ULONG __stdcall Release() override;

    // IDropSource 接口实现
    /** 给予反馈
     * @param [in] dwEffect 拖放效果
     * @return 返回DRAGDROP_S_USEDEFAULTCURSORS使用默认光标
     */
    HRESULT __stdcall GiveFeedback(DWORD dwEffect) override;

    /** 查询是否继续拖放
     * @param [in] fEscapePressed 是否按下了ESC键
     * @param [in] grfKeyState 键盘状态
     * @return 返回S_OK继续拖放，DRAGDROP_S_CANCEL取消拖放，DRAGDROP_S_DROP执行放置
     */
    HRESULT __stdcall QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState) override;

private:
    RichEditDragSource_Windows();
    virtual ~RichEditDragSource_Windows() = default;

private:
    /** 关联的RichEdit2控件指针 */
    RichEdit2* m_pRichEdit;

    /** 引用计数 */
    ULONG m_refCount;
};

/**
 * @brief RichEdit控件的拖放数据对象类（Windows平台）
 * @details 实现IDataObject接口，用于封装拖放的文本数据
 */
class RichEditDataObject_Windows : public IDataObject
{
public:
    /** 创建数据对象
     * @param [in] text 拖放的文本内容
     * @return 返回新创建的数据对象指针
     */
    static RichEditDataObject_Windows* Create(const DStringW& text);

    /** 构造函数
     * @param [in] text 拖放的文本内容
     */
    explicit RichEditDataObject_Windows(const DStringW& text);

    // IUnknown 接口实现
    HRESULT __stdcall QueryInterface(REFIID iid, void** object) override;
    ULONG __stdcall AddRef() override;
    ULONG __stdcall Release() override;

    // IDataObject 接口实现
    HRESULT __stdcall GetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium) override;
    HRESULT __stdcall GetDataHere(FORMATETC* pFormatEtc, STGMEDIUM* pMedium) override;
    HRESULT __stdcall QueryGetData(FORMATETC* pFormatEtc) override;
    HRESULT __stdcall GetCanonicalFormatEtc(FORMATETC* pFormatEtcIn, FORMATETC* pFormatEtcOut) override;
    HRESULT __stdcall SetData(FORMATETC* pFormatEtc, STGMEDIUM* pMedium, BOOL fRelease) override;
    HRESULT __stdcall EnumFormatEtc(DWORD dwDirection, IEnumFORMATETC** ppEnumFormatEtc) override;
    HRESULT __stdcall DAdvise(FORMATETC* pFormatEtc, DWORD advf, IAdviseSink* pAdvSink, DWORD* pdwConnection) override;
    HRESULT __stdcall DUnadvise(DWORD dwConnection) override;
    HRESULT __stdcall EnumDAdvise(IEnumSTATDATA** ppEnumAdvise) override;

private:
    RichEditDataObject_Windows();
    virtual ~RichEditDataObject_Windows();

    /** 查找格式索引
     * @param [in] pFormatEtc 格式描述
     * @return 返回格式索引，未找到返回-1
     */
    int LookupFormatEtc(FORMATETC* pFormatEtc);

    /** 复制全局内存
     * @param [in] hMem 源内存句柄
     * @return 返回新分配的内存句柄
     */
    static HGLOBAL DupGlobalMem(HGLOBAL hMem);

private:
    /** 引用计数 */
    ULONG m_refCount;

    /** 格式数量 */
    int m_nNumFormats;

    /** 格式数组 */
    FORMATETC* m_pFormatEtc;

    /** 存储介质数组 */
    STGMEDIUM* m_pStgMedium;
};

/**
 * @brief RichEdit控件的格式枚举类（Windows平台）
 * @details 实现IEnumFORMATETC接口，用于枚举支持的剪贴板格式
 */
class RichEditEnumFormatEtc_Windows : public IEnumFORMATETC
{
public:
    /** 创建格式枚举对象
     * @param [in] cfmt 格式数量
     * @param [in] afmt 格式数组
     * @param [out] ppEnumFormatEtc 输出的枚举对象指针
     * @return 返回S_OK成功
     */
    static HRESULT CreateEnumFormatEtc(UINT cfmt, FORMATETC* afmt, IEnumFORMATETC** ppEnumFormatEtc);

    /** 构造函数
     * @param [in] pFormatEtc 格式数组
     * @param [in] nNumFormats 格式数量
     */
    RichEditEnumFormatEtc_Windows(FORMATETC* pFormatEtc, int nNumFormats);

    virtual ~RichEditEnumFormatEtc_Windows();

    // IUnknown 接口实现
    HRESULT __stdcall QueryInterface(REFIID iid, void** object) override;
    ULONG __stdcall AddRef() override;
    ULONG __stdcall Release() override;

    // IEnumFORMATETC 接口实现
    HRESULT __stdcall Next(ULONG celt, FORMATETC* pFormatEtc, ULONG* pceltFetched) override;
    HRESULT __stdcall Skip(ULONG celt) override;
    HRESULT __stdcall Reset() override;
    HRESULT __stdcall Clone(IEnumFORMATETC** ppEnumFormatEtc) override;

    /** 深拷贝格式描述
     * @param [out] dest 目标格式
     * @param [in] source 源格式
     */
    static void DeepCopyFormatEtc(FORMATETC* dest, FORMATETC* source);

private:
    RichEditEnumFormatEtc_Windows();

private:
    /** 引用计数 */
    ULONG m_refCount;

    /** 当前索引 */
    ULONG m_nIndex;

    /** 格式数量 */
    ULONG m_nNumFormats;

    /** 格式数组 */
    FORMATETC* m_pFormatEtc;
};

} // namespace ui

#endif // DUILIB_BUILD_FOR_WIN

#endif // UI_CONTROL_RICHEDIT_DRAG_SOURCE_WINDOWS_H_
