#include "Pagination.h"
#include "duilib/Control/Button.h"
#include "duilib/Control/CheckBox.h"
#include "duilib/Control/Combo.h"
#include "duilib/Core/GlobalManager.h"
#include "duilib/Core/Window.h"
#include "duilib/Utils/StringUtil.h"
#include <algorithm>

namespace ui
{

Pagination::Pagination(Window* pWindow) :
    BaseClass(pWindow),
    m_nPageCount(0),
    m_nCurrentPage(1),
    m_nTotalCount(0),
    m_nPageSize(0),
    m_nMaxVisibleCount(7),
    m_bShowPrevNext(true),
    m_bShowFirstLast(false),
    m_strPrevText(_T("‹")),
    m_strNextText(_T("›")),
    m_strFirstText(_T("«")),
    m_strLastText(_T("»")),
    m_strEllipsisText(_T("…")),
    m_strButtonClass(_T("pagination_button")),
    m_nButtonWidth(32),
    m_nButtonHeight(32),
    m_bRebuildPending(false),
    m_bMuteEvent(false),
    m_bShowPageSize(false),
    m_pageSizeOptions({ 10, 20, 50, 100 }),
    m_strPageSizeClass(_T("pagination_page_size")),
    m_strPageSizeFormat(_T("%d")),
    m_nPageSizeWidth(112),
    m_rebuildFlag()
{
}

Pagination::~Pagination()
{
    //取消尚未执行的重建定时器
    m_rebuildFlag.Cancel();
}

DString Pagination::GetType() const
{
    return DUI_CTR_PAGINATION;
}

void Pagination::SetAttribute(const DString& strName, const DString& strValue2)
{
    const DString strValue = GetExpandVarStrings(strValue2);
    if ((strName == _T("page_count")) || (strName == _T("total_page"))) {
        SetPageCount(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("current_page")) || (strName == _T("page"))) {
        SetCurrentPageNoEvent(StringUtil::StringToInt32(strValue));
    }
    else if (strName == _T("total_count")) {
        m_nTotalCount = StringUtil::StringToInt32(strValue);
        if (m_nTotalCount < 0) {
            m_nTotalCount = 0;
        }
        if ((m_nTotalCount > 0) && (m_nPageSize > 0)) {
            SetPageCount((m_nTotalCount + m_nPageSize - 1) / m_nPageSize);
        }
    }
    else if (strName == _T("page_size")) {
        m_nPageSize = StringUtil::StringToInt32(strValue);
        if (m_nPageSize < 0) {
            m_nPageSize = 0;
        }
        if ((m_nTotalCount > 0) && (m_nPageSize > 0)) {
            SetPageCount((m_nTotalCount + m_nPageSize - 1) / m_nPageSize);
        }
    }
    else if ((strName == _T("show_page_size")) || (strName == _T("showpagesize"))) {
        SetShowPageSize(StringUtil::IsValueTrue(strValue));
    }
    else if ((strName == _T("page_size_options")) || (strName == _T("pagesizeoptions"))) {
        SetPageSizeOptions(strValue);
    }
    else if ((strName == _T("page_size_format")) || (strName == _T("pagesizeformat"))) {
        SetPageSizeFormat(strValue);
    }
    else if ((strName == _T("page_size_format_id")) || (strName == _T("pagesizeformatid"))) {
        SetPageSizeFormatId(strValue);
    }
    else if ((strName == _T("page_size_class")) || (strName == _T("pagesizeclass"))) {
        SetPageSizeClass(strValue);
    }
    else if ((strName == _T("page_size_width")) || (strName == _T("pagesizewidth"))) {
        SetPageSizeWidth(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("max_visible_count")) || (strName == _T("maxvisiblecount"))) {
        SetMaxVisibleCount(StringUtil::StringToInt32(strValue));
    }
    else if ((strName == _T("show_prev_next")) || (strName == _T("showprevnext"))) {
        m_bShowPrevNext = StringUtil::IsValueTrue(strValue);
        RequestRebuild();
    }
    else if ((strName == _T("show_first_last")) || (strName == _T("showfirstlast"))) {
        m_bShowFirstLast = StringUtil::IsValueTrue(strValue);
        RequestRebuild();
    }
    else if ((strName == _T("prev_text")) || (strName == _T("prevtext"))) {
        m_strPrevText = strValue;
        RequestRebuild();
    }
    else if ((strName == _T("next_text")) || (strName == _T("nexttext"))) {
        m_strNextText = strValue;
        RequestRebuild();
    }
    else if ((strName == _T("first_text")) || (strName == _T("firsttext"))) {
        m_strFirstText = strValue;
        RequestRebuild();
    }
    else if ((strName == _T("last_text")) || (strName == _T("lasttext"))) {
        m_strLastText = strValue;
        RequestRebuild();
    }
    else if ((strName == _T("ellipsis_text")) || (strName == _T("ellipsistext"))) {
        m_strEllipsisText = strValue;
        RequestRebuild();
    }
    else if ((strName == _T("button_class")) || (strName == _T("buttonclass"))) {
        SetButtonClass(strValue);
    }
    else if ((strName == _T("button_width")) || (strName == _T("buttonwidth"))) {
        m_nButtonWidth = StringUtil::StringToInt32(strValue);
        RequestRebuild();
    }
    else if ((strName == _T("button_height")) || (strName == _T("buttonheight"))) {
        m_nButtonHeight = StringUtil::StringToInt32(strValue);
        RequestRebuild();
    }
    else {
        BaseClass::SetAttribute(strName, strValue);
    }
}

// ---- 数据模型 ----

void Pagination::ClampCurrentPage()
{
    if (m_nPageCount <= 0) {
        m_nCurrentPage = 1;
        return;
    }
    if (m_nCurrentPage < 1) {
        m_nCurrentPage = 1;
    }
    else if (m_nCurrentPage > m_nPageCount) {
        m_nCurrentPage = m_nPageCount;
    }
}

void Pagination::EmitPageChanged(int32_t nOldPage, int32_t nOldPageSize)
{
    if (m_bMuteEvent) {
        return;
    }
    //页码通过 pEventData 携带（栈上临时对象，仅在本次同步派发期间有效）
    PaginationValue value;
    value.page = m_nCurrentPage;
    value.oldPage = nOldPage;
    value.pageSize = m_nPageSize;
    value.oldPageSize = nOldPageSize;
    SendEvent(kEventValueChanged, 0, 0, static_cast<void*>(&value));
}

void Pagination::SetPageCount(int32_t nPageCount)
{
    if (nPageCount < 0) {
        nPageCount = 0;
    }
    if (m_nPageCount == nPageCount) {
        return;
    }
    m_nPageCount = nPageCount;
    ClampCurrentPage();
    RequestRebuild();
}

void Pagination::SetCurrentPage(int32_t nPage)
{
    const int32_t nOldPage = m_nCurrentPage;
    int32_t nNewPage = nPage;
    if (m_nPageCount <= 0) {
        nNewPage = 1;
    }
    else {
        if (nNewPage < 1) {
            nNewPage = 1;
        }
        if (nNewPage > m_nPageCount) {
            nNewPage = m_nPageCount;
        }
    }
    if (nNewPage == nOldPage) {
        return;
    }
    m_nCurrentPage = nNewPage;
    RequestRebuild();
    EmitPageChanged(nOldPage, m_nPageSize);
}

void Pagination::SetCurrentPageNoEvent(int32_t nPage)
{
    const bool bOldMute = m_bMuteEvent;
    m_bMuteEvent = true;
    SetCurrentPage(nPage);
    m_bMuteEvent = bOldMute;
}

void Pagination::SetTotalCount(int32_t nTotalCount, int32_t nPageSize)
{
    m_nTotalCount = (nTotalCount < 0) ? 0 : nTotalCount;
    m_nPageSize = (nPageSize < 0) ? 0 : nPageSize;
    if ((m_nTotalCount > 0) && (m_nPageSize > 0)) {
        SetPageCount((m_nTotalCount + m_nPageSize - 1) / m_nPageSize);
    }
}

bool Pagination::Next()
{
    if (m_nCurrentPage >= m_nPageCount) {
        return false;
    }
    SetCurrentPage(m_nCurrentPage + 1);
    return true;
}

bool Pagination::Prev()
{
    if (m_nCurrentPage <= 1) {
        return false;
    }
    SetCurrentPage(m_nCurrentPage - 1);
    return true;
}

bool Pagination::GoTo(int32_t nPage)
{
    if ((nPage < 1) || (nPage > m_nPageCount)) {
        return false;
    }
    SetCurrentPage(nPage);
    return true;
}

void Pagination::SetMaxVisibleCount(int32_t nCount)
{
    if (nCount < 3) {
        nCount = 3;
    }
    if (m_nMaxVisibleCount != nCount) {
        m_nMaxVisibleCount = nCount;
        RequestRebuild();
    }
}

void Pagination::SetButtonClass(const DString& strClass)
{
    if (m_strButtonClass != strClass) {
        m_strButtonClass = strClass;
        RequestRebuild();
    }
}

// ---- 每页条数选择器 ----

void Pagination::SetShowPageSize(bool bShow)
{
    if (m_bShowPageSize == bShow) {
        return;
    }
    m_bShowPageSize = bShow;
    RequestRebuild();
}

void Pagination::SetPageSize(int32_t nPageSize)
{
    if ((nPageSize <= 0) || (nPageSize == m_nPageSize)) {
        return;
    }
    const int32_t nOldPage = m_nCurrentPage;
    const int32_t nOldPageSize = m_nPageSize;
    m_nPageSize = nPageSize;
    if (m_nTotalCount > 0) {
        SetPageCount((m_nTotalCount + m_nPageSize - 1) / m_nPageSize);
    }
    //切换每页条数后回到第 1 页
    if (m_nPageCount > 0) {
        m_nCurrentPage = 1;
    }
    RequestRebuild();
    EmitPageChanged(nOldPage, nOldPageSize);
}

void Pagination::SetPageSizeOptions(const std::vector<int32_t>& options)
{
    std::vector<int32_t> validOptions;
    for (int32_t nValue : options) {
        if ((nValue > 0) && (std::find(validOptions.begin(), validOptions.end(), nValue) == validOptions.end())) {
            validOptions.push_back(nValue);
        }
    }
    if (validOptions.empty() || (validOptions == m_pageSizeOptions)) {
        return;
    }
    m_pageSizeOptions = validOptions;
    RequestRebuild();
}

void Pagination::SetPageSizeOptions(const DString& strOptions)
{
    DString strValue = strOptions;
    StringUtil::ReplaceAll(_T("|"), _T(","), strValue);
    std::vector<int32_t> options;
    const std::list<DString> items = StringUtil::Split(strValue, _T(","));
    for (const DString& strItem : items) {
        DString strText = strItem;
        StringUtil::Trim(strText);
        const int32_t nValue = StringUtil::StringToInt32(strText);
        if (nValue > 0) {
            options.push_back(nValue);
        }
    }
    SetPageSizeOptions(options);
}

void Pagination::SetPageSizeFormat(const DString& strFormat)
{
    if (m_strPageSizeFormat == strFormat) {
        return;
    }
    m_strPageSizeFormat = strFormat;
    if (m_bShowPageSize) {
        RequestRebuild();
    }
}

void Pagination::SetPageSizeFormatId(const DString& strFormatId)
{
    if (m_strPageSizeFormatId == strFormatId) {
        return;
    }
    m_strPageSizeFormatId = strFormatId;
    if (m_bShowPageSize) {
        RequestRebuild();
    }
}

void Pagination::SetPageSizeClass(const DString& strClass)
{
    if (m_strPageSizeClass == strClass) {
        return;
    }
    m_strPageSizeClass = strClass;
    if (m_bShowPageSize) {
        RequestRebuild();
    }
}

void Pagination::SetPageSizeWidth(int32_t nWidth)
{
    if (nWidth < 0) {
        nWidth = 0;
    }
    if (m_nPageSizeWidth == nWidth) {
        return;
    }
    m_nPageSizeWidth = nWidth;
    if (m_bShowPageSize) {
        RequestRebuild();
    }
}

DString Pagination::FormatPageSizeText(int32_t nPageSize) const
{
    DString strFormat = m_strPageSizeFormat;
    if (!m_strPageSizeFormatId.empty()) {
        const DString strLangText = GlobalManager::GetTextById(m_strPageSizeFormatId);
        if (!strLangText.empty()) {
            strFormat = strLangText;
        }
    }
    if (strFormat.empty()) {
        strFormat = _T("%d");
    }
    //使用文本替换而不是 printf，避免格式串中出现其他 % 转义导致未定义行为
    const DString strValue = StringUtil::Printf(_T("%d"), nPageSize);
    if (strFormat.find(_T("%d")) == DString::npos) {
        return strFormat + strValue;
    }
    StringUtil::ReplaceAll(_T("%d"), strValue, strFormat);
    return strFormat;
}

Control* Pagination::CreatePageSizeCombo()
{
    Window* pWindow = GetWindow();
    if ((pWindow == nullptr) || m_pageSizeOptions.empty()) {
        return nullptr;
    }

    std::vector<int32_t> options = m_pageSizeOptions;
    //当前每页条数不在候选项内时补到最前面，保证选择框能正确回显
    if ((m_nPageSize > 0) && (std::find(options.begin(), options.end(), m_nPageSize) == options.end())) {
        options.insert(options.begin(), m_nPageSize);
    }

    Combo* pCombo = new Combo(pWindow);
    pCombo->SetClass(m_strPageSizeClass.empty() ? _T("pagination_page_size") : m_strPageSizeClass);
    pCombo->SetComboType(Combo::kCombo_DropList);
    if (m_nButtonHeight > 0) {
        pCombo->SetFixedHeight(UiFixedInt(m_nButtonHeight), false, true);
    }
    if (m_nPageSizeWidth > 0) {
        pCombo->SetFixedWidth(UiFixedInt(m_nPageSizeWidth), false, true);
    }
    const int32_t nDropWidth = (m_nPageSizeWidth > 0) ? m_nPageSizeWidth : 96;
    const int32_t nDropHeight = static_cast<int32_t>(options.size()) * 26 + 8;
    pCombo->SetDropBoxSize(UiSize(nDropWidth, nDropHeight), true);

    size_t nSelIndex = Box::InvalidIndex;
    for (size_t i = 0; i < options.size(); ++i) {
        const size_t nItemIndex = pCombo->AddTextItem(FormatPageSizeText(options[i]));
        pCombo->SetItemData(nItemIndex, static_cast<size_t>(options[i]));
        if (options[i] == m_nPageSize) {
            nSelIndex = nItemIndex;
        }
    }
    //未设置每页条数时，默认选中第一个候选项
    if ((nSelIndex == Box::InvalidIndex) && !options.empty()) {
        nSelIndex = 0;
        m_nPageSize = options[0];
    }
    if (nSelIndex != Box::InvalidIndex) {
        pCombo->SetCurSel(nSelIndex, false);
    }

    pCombo->AttachSelect([this](const EventArgs& args) -> bool {
        Combo* pSender = dynamic_cast<Combo*>(args.GetSender());
        if (pSender == nullptr) {
            return true;
        }
        const size_t nIndex = pSender->GetCurSel();
        if ((nIndex == Box::InvalidIndex) || !pSender->HasItemData(nIndex)) {
            return true;
        }
        SetPageSize(static_cast<int32_t>(pSender->GetItemData(nIndex)));
        return true;
        });

    if (!AddItem(pCombo)) {
        delete pCombo;
        return nullptr;
    }
    return pCombo;
}

void Pagination::OnLanguageChanged(bool bRedraw)
{
    BaseClass::OnLanguageChanged(bRedraw);
    //多语言切换后，每页条数候选项的文本需要重新生成
    if (m_bShowPageSize && !m_strPageSizeFormatId.empty()) {
        RequestRebuild();
    }
}

// ---- 按钮重建 ----

void Pagination::RequestRebuild()
{
    m_bRebuildPending = true;
    //使用一次性定时器，把重建推迟到当前事件派发返回之后，
    //避免在子控件（页码按钮）自身的点击事件中删除该子控件
    m_rebuildFlag.Cancel();
    std::function<void()> callback = UiBind(&Pagination::OnRebuildTimer, this);
    GlobalManager::Instance().Timer().AddTimer(m_rebuildFlag.GetWeakFlag(), callback, 1, 1);
}

void Pagination::OnRebuildTimer()
{
    if (!m_bRebuildPending) {
        return;
    }
    m_bRebuildPending = false;
    RebuildButtons();
}

std::vector<int32_t> Pagination::CalcVisiblePages() const
{
    std::vector<int32_t> pages; //0 表示省略号
    const int32_t nTotal = m_nPageCount;
    if (nTotal <= 0) {
        return pages;
    }
    int32_t nMax = m_nMaxVisibleCount;
    if (nMax < 3) {
        nMax = 3;
    }
    if (nTotal <= nMax) {
        for (int32_t i = 1; i <= nTotal; ++i) {
            pages.push_back(i);
        }
        return pages;
    }

    //除首页、末页与两个省略号外的中间页码数量
    int32_t nMiddles = nMax - 4;
    if (nMiddles < 1) {
        nMiddles = 1;
    }
    const int32_t nHalf = nMiddles / 2;
    int32_t nStart = m_nCurrentPage - nHalf;
    int32_t nEnd = m_nCurrentPage + (nMiddles - nHalf - 1);

    if (nStart <= 2) {
        //靠近首页：左侧不显示省略号
        nStart = 2;
        nEnd = nStart + nMiddles - 1;
        if (nEnd > (nTotal - 1)) {
            nEnd = nTotal - 1;
        }
        pages.push_back(1);
        for (int32_t i = nStart; i <= nEnd; ++i) {
            pages.push_back(i);
        }
        if (nEnd < (nTotal - 1)) {
            pages.push_back(0);
        }
        pages.push_back(nTotal);
    }
    else if (nEnd >= (nTotal - 1)) {
        //靠近末页：右侧不显示省略号
        nEnd = nTotal - 1;
        nStart = nEnd - nMiddles + 1;
        if (nStart < 2) {
            nStart = 2;
        }
        pages.push_back(1);
        if (nStart > 2) {
            pages.push_back(0);
        }
        for (int32_t i = nStart; i <= nEnd; ++i) {
            pages.push_back(i);
        }
        pages.push_back(nTotal);
    }
    else {
        //中间：两侧均显示省略号
        pages.push_back(1);
        pages.push_back(0);
        for (int32_t i = nStart; i <= nEnd; ++i) {
            pages.push_back(i);
        }
        pages.push_back(0);
        pages.push_back(nTotal);
    }
    return pages;
}

Control* Pagination::CreateTextButton(const DString& strText)
{
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return nullptr;
    }
    Button* pButton = new Button(pWindow);
    //导航 / 省略按钮是普通 Button，不能用页码按钮类（其含 selected_* 等 CheckBox 专有属性，Button 不认识会触发断言）
    pButton->SetClass(_T("pagination_nav_button"));
    pButton->SetText(strText);
    if (m_nButtonWidth > 0) {
        pButton->SetFixedWidth(UiFixedInt(m_nButtonWidth), false, true);
    }
    if (m_nButtonHeight > 0) {
        pButton->SetFixedHeight(UiFixedInt(m_nButtonHeight), false, true);
    }
    if (!AddItem(pButton)) {
        delete pButton;
        return nullptr;
    }
    return pButton;
}

Control* Pagination::CreatePageButton(int32_t nPage, bool bSelected)
{
    Window* pWindow = GetWindow();
    if (pWindow == nullptr) {
        return nullptr;
    }
    CheckBox* pCheckBox = new CheckBox(pWindow);
    pCheckBox->SetClass(m_strButtonClass);
    pCheckBox->SetText(StringUtil::Printf(_T("%d"), nPage));
    pCheckBox->SetSelected(bSelected);
    if (m_nButtonWidth > 0) {
        pCheckBox->SetFixedWidth(UiFixedInt(m_nButtonWidth), false, true);
    }
    if (m_nButtonHeight > 0) {
        pCheckBox->SetFixedHeight(UiFixedInt(m_nButtonHeight), false, true);
    }

    //点击页码：跳转；已选中页码被再次点击会触发"取消选中"，此时重新选回
    pCheckBox->AttachSelect([this, nPage](const EventArgs& /*args*/) -> bool {
        GoTo(nPage);
        return true;
    });
    pCheckBox->AttachUnSelect([this, nPage](const EventArgs& args) -> bool {
        if (nPage == m_nCurrentPage) {
            if (CheckBox* pSender = dynamic_cast<CheckBox*>(args.GetSender())) {
                pSender->SetSelected(true);
            }
        }
        return true;
    });

    if (!AddItem(pCheckBox)) {
        delete pCheckBox;
        return nullptr;
    }
    return pCheckBox;
}

void Pagination::RebuildButtons()
{
    //先移除全部旧按钮
    RemoveAllItems();
    if ((m_nPageCount <= 0) || (GetWindow() == nullptr)) {
        Invalidate();
        return;
    }

    //首页 / 上一页
    if (m_bShowFirstLast) {
        if (Control* pFirst = CreateTextButton(m_strFirstText)) {
            pFirst->AttachClick([this](const EventArgs& /*args*/) -> bool {
                GoTo(1);
                return true;
            });
        }
    }
    if (m_bShowPrevNext) {
        if (Control* pPrev = CreateTextButton(m_strPrevText)) {
            pPrev->AttachClick([this](const EventArgs& /*args*/) -> bool {
                Prev();
                return true;
            });
        }
    }

    //页码按钮（含省略号）
    const std::vector<int32_t> pages = CalcVisiblePages();
    for (int32_t nPage : pages) {
        if (nPage == 0) {
            CreateTextButton(m_strEllipsisText);
        }
        else {
            CreatePageButton(nPage, nPage == m_nCurrentPage);
        }
    }

    //下一页 / 末页
    if (m_bShowPrevNext) {
        if (Control* pNext = CreateTextButton(m_strNextText)) {
            pNext->AttachClick([this](const EventArgs& /*args*/) -> bool {
                Next();
                return true;
            });
        }
    }
    if (m_bShowFirstLast) {
        if (Control* pLast = CreateTextButton(m_strLastText)) {
            pLast->AttachClick([this](const EventArgs& /*args*/) -> bool {
                GoTo(m_nPageCount);
                return true;
            });
        }
    }

    //每页条数选择器（可选功能，默认不显示）
    if (m_bShowPageSize) {
        CreatePageSizeCombo();
    }

    Invalidate();
}

} // namespace ui
