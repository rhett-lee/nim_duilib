#ifndef UI_CONTROL_RICHEDIT_DATA_H_
#define UI_CONTROL_RICHEDIT_DATA_H_

#include "duilib/Core/UiTypes.h"
#include "duilib/Core/SharePtr.h"
#include "duilib/Render/IRender.h"
#include <unordered_map>
#include <map>
#include <list>

namespace ui
{
/** 生成格式化文本的接口，用于绘制文本
*/
class IRichTextData
{
public:
    /** 将文本生成可绘制的格式
    * @param [in] textView 按行组织切分后的文本视图，每行一条数据（以'\n'切分的行）
    * @param [out] richTextDataList 返回格式化的文本，返回的容器个数应与传入的textView相同，如果此行不更新，可用填充空数据: RichTextData()
    * @param [in] nStartLine 重新计算的起始行号（增量计算时使用）
    * @param [in] modifiedLines 有修改的行号（增量计算时使用）
    */
    virtual bool GetRichTextForDraw(const std::vector<std::wstring_view>& textView,
                                    std::vector<RichTextData>& richTextDataList,
                                    size_t nStartLine = (size_t)-1,
                                    const std::vector<size_t>& modifiedLines = std::vector<size_t>()) const = 0;

    /** 获取文本绘制矩形范围（需要时，随时调用该接口获取绘制文本的矩形范围）
    * @return 返回当前文本绘制的矩形范围，该范围需要去除内边距，滚动条所占空间
    */
    virtual UiRect GetRichTextDrawRect() const = 0;

    /** 获取绘制的透明度
    */
    virtual uint8_t GetDrawAlpha() const = 0;

    /** 文字区域已经重新计算过的事件
    */
    virtual void OnTextRectsChanged() = 0;

    /** 获取行高值
    */
    virtual int32_t GetTextRowHeight() const = 0;

    /** 获取光标的宽度
    */
    virtual int32_t GetTextCaretWidth() const = 0;

    /** 当前是否为密码模式
    */
    virtual bool IsTextPasswordMode() const = 0;

    /** 处理密码模式下的显示字符
    */
    virtual void ReplacePasswordChar(DStringW& text) const = 0;

    /** 获取文本限制长度
    */
    virtual int32_t GetTextLimitLength() const = 0;
};

class RichEditData
{
public:
    explicit RichEditData(IRichTextData* pRichTextData);
    RichEditData(const RichEditData&) = delete;
    RichEditData& operator=(const RichEditData&) = delete;
    virtual ~RichEditData();

public:
    /** 设置关联的渲染接口
    */
    void SetRender(IRender* pRender);

    /** 设置关联的渲染接口工厂
    */
    void SetRenderFactory(IRenderFactory* pRenderFactory);

    /** 设置单行文本模式，绘制的时候不分行，忽略换行符('\n')
    */
    void SetSingleLineMode(bool bSingleLineMode);

    /** 是否为单行文本模式
    */
    bool IsSingleLineMode() const;

    /** 设置文本水平对齐方式(应用于整体文本)
    */
    void SetHAlignType(HorAlignType hAlignType);

    /** 获取文本水平对齐方式
    */
    HorAlignType GetHAlignType() const;

    /** 设置文本垂直对齐方式(应用于整体文本)
    */
    void SetVAlignType(VerAlignType vAlignType);

    /** 获取文本垂直对齐方式
    */
    VerAlignType GetVAlignType() const;

    /** 文本的滚动条位置
    */
    void SetScrollOffset(const UiSize& szScrollOffset);

    /** 计算显示文本所需要的矩形范围(用于估算控件大小)
    * @param [int] rcAvailable 可用的矩形范围
    */
    UiRect EstimateTextDisplayBounds(const UiRect& rcAvailable);

public:
    /** 设置文本，并清空Undo/Redo历史
     * @param [in] text 文本内容
     * @return 返回true表示文本有变化，返回false表示文本无变化
     */
    bool SetText(const DStringW& text);

    /** 替换指定范围的文本(文本的添加，修改，删除功能，均通过这个函数完成)
     *  (1) 如果 nStartChar == nEndChar，表示在此位置插入文本
     *  (2) 如果 nEndChar > nStartChar，表示替换文本
     *  (3) 如果 nStartChar < nEndChar，参数错误
     * @param [in] text 文本内容
     * @param [in] nStartChar 起始下标值
     * @param [in] nEndChar 结束下标值
     * @param [in] bCanUndo 是否可以撤销，true 为可以，否则为 false
     * @param [in] bClearRedo 是否清空Redo列表, 仅当bCanUndo为false时生效
     * @return 返回true表示文本有变化，返回false表示文本无变化
     */
    bool ReplaceText(int32_t nStartChar, int32_t nEndChar, const DStringW& text, bool bCanUndo = true, bool bClearRedo = true);

    /** 获取文本
    */
    DStringW GetText() const;

    /** 获取文本视图，文本视图是按行组织，每行一条数据（以'\n'切分的行）
    */
    void GetTextView(std::vector<std::wstring_view>& textView) const;

    /** 获取内容的长度(按UTF16编码的字符个数)
     * @return 返回文本内容长度
     */
    size_t GetTextLength() const;

    /** 是否为空
    */
    bool IsEmpty() const;

    /** 获取指定范围[nStartChar, nEndChar)的文本
     * @param[in] nStartChar 起始下标值
     * @param[in] nEndChar 结束下标值
     */
    DStringW GetTextRange(int32_t nStartChar, int32_t nEndChar) const;

    /** 判断指定范围是否含有文本内容
     * @param[in] nStartChar 起始下标值
     * @param[in] nEndChar 结束下标值
     */
    bool HasTextRange(int32_t nStartChar, int32_t nEndChar) const;

    /** 设置可撤销的限制次数
    */
    void SetUndoLimit(uint32_t nUndoLimit);

    /** 获取可撤销的限制次数
    */
    uint32_t GetUndoLimit() const;

    /** 是否可撤销
    */
    bool CanUndo() const;

    /** 撤销操作
     * @param [out] nEndCharIndex 返回结束的字符下标
     * @return 成功返回 true，失败返回 false
     */
    bool Undo(int32_t& nEndCharIndex);

    /** 是否可以重做
    */
    bool CanRedo() const;

    /** 重做操作
     * @param [out] nEndCharIndex 返回结束的字符下标
     * @return 成功返回 true，失败返回 false
     */
    bool Redo(int32_t& nEndCharIndex);

    /** 清空撤销列表
    */
    void EmptyUndoBuffer();

    /** 清空
     */
    void Clear();

    /** 查找文本
    * @param [in] bMatchCase 查找时，是否区分大小写
    * @param [in] bMatchWholeWord 查找时，是否按词匹配
    * @param [in] bFindDown 是否向后查找，为true表示向后查找，false表示反向查找
    * @param [in] nFindStartChar 字符的查找范围的起始值
    * @param [in] nFindEndChar 字符的查找范围的结束值
    * @param [in] findText 待查找的文本内容
    * @param [out] chrgText 匹配的文本，字符的索引号范围
    */
    bool FindRichText(bool bMatchCase, bool bMatchWholeWord, bool bFindDown,
                      int32_t nFindStartChar, int32_t nFindEndChar,
                      const DStringW& findText,
                      int32_t& nFoundStartChar, int32_t& nFoundEndChar) const;

public:
    /** 获取总行数
     * @return 返回总行数
     */
    int32_t GetRowCount();

    /** 获取一行数据
     * @param[in] nRowIndex 行号
     * @return 返回获取的一行数据
     */
    DStringW GetRowText(int32_t nRowIndex);

    /** 获取指定行的第一个字符索引
     * @param[in] nRowIndex 行号
     * @return 返回指定行的第一个字符索引, 失败时返回-1
     */
    int32_t RowIndex(int32_t nRowIndex);

    /** 获取指定行的数据长度
     * @param[in] nRowIndex 行号
     * @return 返回指定行的数据长度
     */
    int32_t RowLength(int32_t nRowIndex);

    /** 获取指定字符所在行号
     * @param[in] nCharIndex 字符的索引下标
     * @return 返回当前字符所在的行号
     */
    int32_t RowFromChar(int32_t nCharIndex);

public:
    /** 获取指定字符的光标位置
    * @param[in] nCharIndex 字符索引位置
    */
    UiPoint CaretPosFromChar(int32_t nCharIndex);

    /** 检索编辑控件中指定字符的工作区坐标。
     * @param[in] nCharIndex 字符索引位置
     * @return 返回值包含字符的客户端区域坐标。
     */
    UiPoint PosFromChar(int32_t nCharIndex);

    /** 获取有关距离编辑控件客户区中指定点最近的字符的信息
     * @param[in] pt 坐标信息（调用方负责处理滚动条的偏移）
     * @return 返回值指定了距指定点最近字符的从零开始的字符索引。 如果指定点超出控件中的最后一个字符，则返回值会指示编辑控件中的最后一个字符。
     */
    int32_t CharFromPos(UiPoint pt);

    /** 获取指定字符所在行的区域（包含无文字的区域）
    * @param[in] nCharIndex 字符的索引下标
    */
    UiRect GetCharRowRect(int32_t nCharIndex);

    /** 获取选择文本的所在行的区域
     * @param [in] nStartChar 起始下标值
     * @param [in] nEndChar 结束下标值
     * @param [out] rowTextRectFs 每行的矩形范围（逻辑行）
     */
    void GetCharRangeRects(int32_t nStartChar, int32_t nEndChar, std::map<int32_t, UiRectF>& rowTextRectFs);

public:
    /** 获取下一个有效字符的索引号(换行符会被跳过)
    * @param [in] nCharIndex 字符的索引下标
    * @return 返回后一个有效字符的索引号
    */
    int32_t GetNextValidCharIndex(int32_t nCharIndex);

    /** 获取前一个有效字符的索引号(换行符会被跳过)
    * @param [in] nCharIndex 字符的索引下标
    * @return 返回前一个有效字符的索引号
    */
    int32_t GetPrevValidCharIndex(int32_t nCharIndex);

    /** 获取下一个有效单词的索引号(按空格或者标点符号分割)
    * @param [in] nCharIndex 字符的索引下标
    * @return 返回后一个有效字符的索引号
    */
    int32_t GetNextValidWordIndex(int32_t nCharIndex);

    /** 获取前一个有效单词的索引号(按空格或者标点符号分割)
    * @param [in] nCharIndex 字符的索引下标
    * @return 返回前一个有效字符的索引号
    */
    int32_t GetPrevValidWordIndex(int32_t nCharIndex);

    /** 获取当前位置附近单词的起始索引号和结束索引号
    * @param [in] nCharIndex 字符的索引下标
    * @param [out] nWordStartIndex 单词的索引开始索引号
    * @param [out] nWordEndIndex 单词的索引结束索引号
    */
    bool GetCurrentWordIndex(int32_t nCharIndex, int32_t& nWordStartIndex, int32_t& nWordEndIndex);

    /** 获取本行的行首的字符索引下标值
    * @param [in] nCharIndex 字符的索引下标
    * @return 返回本行的行首的字符索引下标值
    */
    int32_t GetRowStartCharIndex(int32_t nCharIndex);

    /** 获取本行的行尾的字符索引下标值
    * @param [in] nCharIndex 字符的索引下标
    * @return 返回本行的行尾的字符索引下标值
    */
    int32_t GetRowEndCharIndex(int32_t nCharIndex);

    /** 获取索引下标对应的字符宽度
    * @param [in] nCharIndex 字符的索引下标
    */
    int32_t GetCharWidthValue(int32_t nCharIndex);

public:
    /** 设置文本绘制缓存
    */
    void SetDrawRichTextCache(const std::shared_ptr<DrawRichTextCache>& spDrawRichTextCache);

    /** 获取文本绘制缓存
    */
    const std::shared_ptr<DrawRichTextCache>& GetDrawRichTextCache() const;

    /** 清除绘制缓存
    */
    void ClearDrawRichTextCache();

    /** 对重新计算做标记
    */
    void SetCacheDirty(bool bDirty);

    /** 获取文本所占的矩形范围
    */
    const UiRect& GetTextRect() const;

    /** 获取纵向对齐的偏移量
    */
    int32_t GetTextRectOfssetY() const;

    /** 获取横向对齐的偏移量（按逻辑行）
    */
    const std::vector<int32_t>& GetTextRowXOffset() const;

    /** 检查并按需重新计算文本区域
    */
    void CheckCalcTextRects();

    /** 按字符数限制，截断文本
    */
    void TruncateLimitText(DStringW& text, int32_t nLimitLen) const;

private:
    /** 将内部坐标转换为外部坐标
    */
    const UiPoint& ConvertToExternal(UiPoint& pt) const;
    const UiRect& ConvertToExternal(UiRect& rect) const;
    const UiRectF& ConvertToExternal(UiRectF& rect) const;

    /** 将外部坐标转换为内部坐标
    */
    const UiPoint& ConvertToInternal(UiPoint& pt) const;
    const UiRect& ConvertToInternal(UiRect& rect) const;

private:
    /** 设置文本绘制区域
    */
    void SetTextDrawRect(const UiRect& rcTextDrawRect, bool bCheckDirty);

    /** 将文本按照换行符（'\n'）切分为多行
    */
    void SplitLines(const std::wstring_view& textView, std::vector<std::wstring_view>& lineTextViewList);

    /** 清空撤销列表
    */
    void ClearUndoList();

    /** 记录操作到撤销列表
    */
    void AddToUndoList(int32_t nStartChar, const DStringW& newText, const DStringW& oldText);

    /** 从缓存中计算文本所占的矩形区域
    */
    void CalcCacheTextRects(UiRect& rcTextRect);

    /** 按对齐方式，更新每行文本的纵坐标
    */
    void UpdateRowTextOffsetY(RichTextLineInfoList& lineTextInfo, int32_t nOffsetY) const;

    /** 按对齐方式，更新每行文本的横坐标
    */
    void UpdateRowTextOffsetX(RichTextLineInfoList& lineTextInfo, HorAlignType hAlignType, std::vector<int32_t>& rowXOffset, bool& bTextRectXOffsetUpdated) const;

    /** 计算文本的区域信息（全部重新计算）
    */
    void CalcTextRects();

    /** 计算文本的区域信息（只重新计算修改的部分文本）
    * @param [in] nStartLine 重新计算的起始行号
    * @param [in] modifiedLines 有修改的行号
    * @param [in] deletedLines 删除的行
    * @param [in] nDeletedRows 删除了几行
    */
    void CalcTextRects(size_t nStartLine,
                       const std::vector<size_t>& modifiedLines,
                       const std::vector<size_t>& deletedLines,
                       size_t nDeletedRows);

    /** 定位字符范围所属的行和行文本偏移量
    * @param [in] nStartChar 起始下标值
    * @param [in] nEndChar 结束下标值， nEndChar >= nStartChar
    * @param [out] nStartLine 起始行号，物理行
    * @param [out] nEndLine  结束行号，物理行，nEndLine >= nStartLine
    * @param [out] nStartCharLineOffset 在起始行中，开始字符的偏移量，有效范围[0, 行文本长度)
    * @param [out] nEndCharLineOffset 在结束行中，结束字符的偏移量，有效范围[0, 行文本长度)
    */
    bool FindLineTextPos(int32_t nStartChar, int32_t nEndChar,
                         size_t& nStartLine, size_t& nEndLine,
                         size_t& nStartCharLineOffset, size_t& nEndCharLineOffset) const;

    /** 判断一个字符是否为分隔符（空格，标点符号等）
    */
    bool IsSeperatorChar(DStringW::value_type ch) const;

    /** 获取指定字符的所在的物理行号和行内逻辑行号
    * @param [in] nCharIndex 字符索引位置
    * @param [out] nLineNumber 物理行号
    * @param [out] nLineRowIndex 在物理行中的逻辑行号（每行中从0开始编号）
    * @param [out] nStartCharRowOffset 在逻辑行中的字符偏移量
    */
    bool GetCharLineRowIndex(int32_t nCharIndex,
                             size_t& nLineNumber,
                             size_t& nLineRowIndex,
                             size_t& nStartCharRowOffset) const;

    /** 获取指定字符的所在行的数据
    * @param [in] nCharIndex 字符索引位置
    * @param [out] nStartCharRowOffset 在逻辑行中的字符偏移量
    */
    RichTextRowInfoPtr GetCharRowInfo(int32_t nCharIndex, size_t& nStartCharRowOffset) const;

    /** 获取一个点所在的行
    */
    RichTextRowInfoPtr GetRowInfoFromPoint(const UiPoint& pt) const;

    /** 获取首行的数据
    */
    RichTextRowInfoPtr GetFirstRowInfo() const;

    /** 获取尾行的数据
    */
    RichTextRowInfoPtr GetLastRowInfo() const;

    /** 获取一行数据的起始字符下标值，如果找不到返回(size_t)-1
    */
    size_t GetRowInfoStartIndex(const RichTextRowInfoPtr& spRowInfo) const;

    /** 更新行高数据（增量绘制后的更新）
    * @param [in] nDrawStartLineIndex 从哪一行数据开始处理
    */
    void UpdateRowInfo(size_t nDrawStartLineIndex);

    /** 获取空文本的光标位置
    */
    UiPoint PosForEmptyText() const;

    /** 适合业务逻辑的Union函数
    */
    void UnionRect(UiRect& rect, const UiRect& r) const;
    void UnionRectF(UiRectF& rect, const UiRectF& r) const;

private:
    /** 将文本生成可绘制的格式的接口
    */
    IRichTextData* m_pRichText;

    /** 是否为单行文本模式
    */
    bool m_bSingleLineMode;

    /** 文本水平对齐方式
    */
    HorAlignType m_hAlignType;

    /** 文本垂直对齐方式
    */
    VerAlignType m_vAlignType;

    /** 关联的渲染接口
    */
    IRender* m_pRender;

    /** 关联的渲染接口工厂
    */
    IRenderFactory* m_pRenderFactory;

    /** 文本绘制区域
    */
    UiRect m_rcTextDrawRect;

    /** 文本所占的区域
    */
    UiRect m_rcTextRect;

    /** 文本的纵向偏移是否有更新过（Y坐标）
    */
    bool m_bTextRectYOffsetUpdated;

    /** 文本的横向偏移是否有更新过（X坐标）
    */
    bool m_bTextRectXOffsetUpdated;

    /** 每行的横向偏移列表（逻辑行）
    */
    std::vector<int32_t> m_rowXOffset;

    /** 文本的滚动条位置
    */
    UiSize m_szScrollOffset;

private:
    /** 文本数据，按物理分行切分
    */
    RichTextLineInfoList m_lineTextInfo;

    /** 文本绘制缓存
    */
    std::shared_ptr<DrawRichTextCache> m_spDrawRichTextCache;

    /** 缓存数据有效性标志
    */
    bool m_bCacheDirty;

private:
    /** Undo的数据
    */
    struct TUndoData
    {
        int32_t m_nStartChar = -1;
        DStringW m_newText;
        DStringW m_oldText;
    };

    /** Undo的数据列表
    */
    std::list<TUndoData> m_undoList;

    /** Redo的数据列表
    */
    std::list<TUndoData> m_redoList;

    /** 重做的最大次数限制
    */
    uint32_t m_nUndoLimit;
};

} //namespace ui

#endif // UI_CONTROL_RICHEDIT_DATA_H_
