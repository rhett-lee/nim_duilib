#include "Grid.h"
#include "ui_components/grid/GridBody.h"
#include "duilib/Box/VBox.h"
#include "duilib/Control/ScrollBar.h"
#include "duilib/Animation/AnimationManager.h"
#include "duilib/Animation/AnimationPlayer.h"

namespace ui
{
	Grid::Grid() : ScrollableBox(new VLayout()){

	}

	Grid::~Grid(){
	}

	GridBody* Grid::CreateGridBody()
	{
		return new GridBody(this);
	}

	void Grid::Init()
	{
		if (m_bIsInit)
			return;

		m_pBody = CreateGridBody();
		if (!m_pBody)
			m_pBody = new GridBody(this);
		AddItem(m_pBody);

		SetBkColor(L"white");
		SetBorderColor(L"splitline_level1");
		SetBorderSize({ 1, 1, 1, 1 });

		SetHeaderHeight(m_pBody->m_defaultRowHeight);
		//SetFixedBkColor(L"splitline_level2");
		//SetGridLineColor(L"grid_line");

		if (m_pVerticalScrollBar)
			m_pVerticalScrollBar->SetAutoHideScroll(false);
		if (m_pHorizontalScrollBar)
			m_pHorizontalScrollBar->SetAutoHideScroll(false);

		m_bIsInit = true;
	}

	int Grid::GetDefaultRowHeight(){ return m_pBody->GetDefaultRowHeight(); };
	void Grid::SetDefaultRowHeight(int height){ return m_pBody->SetDefaultRowHeight(height); };

	int Grid::GetDefaultColWidth(){ return m_pBody->GetDefaultColWidth(); };
	void Grid::SetDefaultColWidth(int width){ return m_pBody->SetDefaultColWidth(width); };

	int Grid::GetColCount() const{ return m_pBody->GetColCount(); }
	void Grid::SetColCount(int count){ return m_pBody->SetColCount(count); }

	int Grid::GetRowCount() const{ return m_pBody->GetRowCount(); }
	void Grid::SetRowCount(int count){ return m_pBody->SetRowCount(count); }

	int Grid::GetFixedColCount() const{ return m_pBody->GetFixedColCount(); }
	void Grid::SetFixedColCount(int fixed){ return m_pBody->SetFixedColCount(fixed); }

	int Grid::GetFixedRowCount() const{ return m_pBody->GetFixedRowCount(); }
	void Grid::SetFixedRowCount(int fixed){ return m_pBody->SetFixedRowCount(fixed); }

	int Grid::GetColumnWidth(int col_index) const{ return m_pBody->GetColumnWidth(col_index); };
	void Grid::SetColumnWidth(int col_index, int width){ m_pBody->SetColumnWidth(col_index, width); }

	int Grid::GetRowHeight(int row_index) const { return m_pBody->GetRowHeight(row_index); };
	void Grid::SetRowHeight(int row_index, int height){ return m_pBody->SetRowHeight(row_index, height); };

	int Grid::GetHeaderHeight() const{ return m_pBody->GetHeaderHeight(); }
	void Grid::SetHeaderHeight(int height){
		m_pBody->SetHeaderHeight(height);
		SetScrollBarPadding({ 0, height, 0, 0 });
	}

	const std::wstring& Grid::GetFixedBkColor() const{ return m_pBody->GetFixedBkColor(); }
	void Grid::SetFixedBkColor(const std::wstring& color){ m_pBody->SetFixedBkColor(color); }

	const std::wstring& Grid::GetSelForeColor() const{ return m_pBody->GetSelForeColor(); }
	void Grid::SetSelForeColor(const std::wstring& color){ m_pBody->SetSelForeColor(color); }

	const std::wstring& Grid::GetGridLineColor() const{ return m_pBody->GetGridLineColor(); }
	void Grid::SetGridLineColor(const std::wstring& color){ m_pBody->SetGridLineColor(color); }

	std::wstring Grid::GetGridItemText(int row_index, int col_index){ return m_pBody->GetGridItemText(row_index, col_index); }
	bool Grid::SetGridItemText(const std::wstring& text, int row_index, int col_index){ return m_pBody->SetGridItemText(text, row_index, col_index); }

	bool Grid::IsGridItemFixed(int row_index, int col_index){ return m_pBody->IsGridItemFixed(row_index, col_index); }

	int Grid::GetFixedColWidth() const{ return m_pBody->GetFixedColWidth(); }
	int Grid::GetFixedRowHeight() const{ return m_pBody->GetFixedRowHeight(); }
	
	GridHeaderItem* Grid::AddCol(const std::wstring& text, int width){ return m_pBody->AddCol(text, width); }
	bool Grid::AddRow(){ return m_pBody->AddRow(); }

	GridHeader *Grid::GetHeader(){ return m_pBody->GetHeader(); }
	GridItem *Grid::GetGridItem(int row_index, int col_index){ return m_pBody->GetGridItem(row_index, col_index); }

	void Grid::Clear(bool include_header){ return m_pBody->Clear(include_header); }
	bool Grid::RemoveRow(int row_index){ return m_pBody->RemoveRow(row_index); }
	bool Grid::RemoveCol(int col_index){ return m_pBody->RemoveCol(col_index); }

	void Grid::AttachTextChange(const EventCallback& callback){ return m_pBody->AttachTextChange(callback); }
	bool Grid::AutoFixColWidth(int col_index, int min_width, int max_width){ return m_pBody->AutoFixColWidth(col_index, min_width, max_width); };
	
	const GridSelRange& Grid::GetSelRange() const{ return m_pBody->GetSelRange(); };


	bool Grid::ButtonDown(const EventArgs& /*msg*/)
	{
		m_pBody->_EndEdit();
		return true;
	}
	void Grid::PaintChild(IRender* pRender, const UiRect& rcPaint)
	{
		UiRect rcTemp;
		if (!::IntersectRect(&rcTemp, &rcPaint, &GetRect())) return;

		for (auto it = m_items.begin(); it != m_items.end(); it++) {
			Control* pControl = *it;
			if (!pControl->IsVisible()) continue;
			pControl->AlphaPaint(pRender, rcPaint);
		}

		if (m_pHorizontalScrollBar && m_pHorizontalScrollBar->IsVisible()) {
			m_pHorizontalScrollBar->AlphaPaint(pRender, rcPaint);
		}

		if (m_pVerticalScrollBar && m_pVerticalScrollBar->IsVisible()) {
			m_pVerticalScrollBar->AlphaPaint(pRender, rcPaint);
		}

		static bool bFirstPaint = true;
		if (bFirstPaint) {
			bFirstPaint = false;
			LoadImageCache(true);
		}
	}
}