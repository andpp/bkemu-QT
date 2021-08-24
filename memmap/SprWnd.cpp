// SprView.cpp: файл реализации
//

#include "pch.h"
#if 0
#include "resource.h"
#include "SprWnd.h"


// CSprWnd

IMPLEMENT_DYNAMIC(CSprWnd, CFrameWnd)

CSprWnd::CSprWnd(uint8_t *buffer, size_t size)
	: m_pFileBuffer(buffer)
	, m_nFileLen(size)
	, m_cx(0)
	, m_cy(0)
	, m_orig_cx(512)
	, m_orig_cy(256)
	, m_nScale(1)
{
	m_pSprView = new CSprView();

	if (m_pSprView)
	{
		m_pSprView->SetParameters(m_pFileBuffer, m_nFileLen, m_nScale);
	}
}

CSprWnd::~CSprWnd()
{
	m_ComboboxFont.DeleteObject();
}

BEGIN_MESSAGE_MAP(CSprWnd, CFrameWnd)
    ON_WM_CREATE()
    ON_WM_SIZE()
    ON_COMMAND(ID_BUTTON_COLOR_MODE, &CSprWnd::OnViewColormode)
    ON_COMMAND(ID_BUTTON_BW_MODE, &CSprWnd::OnViewBWMode)
    ON_CBN_SELENDOK(ID_COMBO_PALETTE, &CSprWnd::OnSetPalette)
    ON_CBN_SELENDOK(ID_COMBO_SCALE, &CSprWnd::OnSetScale)
END_MESSAGE_MAP()



// обработчики сообщений CSprWnd
int CSprWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
    if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    {
        return -1;
    }

	m_cx = m_orig_cx * m_nScale;
	m_cy = m_orig_cy * m_nScale;
	m_cxBorder = GetSystemMetrics(SM_CXFRAME);
	m_cyBorder = GetSystemMetrics(SM_CYFRAME);
	m_cxScroll = GetSystemMetrics(SM_CXVSCROLL);
	m_cyScroll = GetSystemMetrics(SM_CYHSCROLL);
	m_cyCaption = GetSystemMetrics(SM_CYSMCAPTION);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT | TBSTYLE_TOOLTIPS, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC)
	        || !m_wndToolBar.LoadToolBar(IDR_SPRVIEWFRAME))
	{
		TRACE0("Не удалось создать тулбар\n");
		return -1; // fail to create
	}

	int nPixelW = GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSX);
	int nPixelH = GetDeviceCaps(GetDC()->GetSafeHdc(), LOGPIXELSY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	CToolBarCtrl *pToolCtrl = &m_wndToolBar.GetToolBarCtrl();
	int nIndex = pToolCtrl->CommandToIndex(ID_BUTTON_COLOR_MODE);
	m_wndToolBar.SetButtonStyle(nIndex, m_wndToolBar.GetButtonStyle(nIndex) | TBBS_CHECKBOX);
	nIndex = pToolCtrl->CommandToIndex(ID_BUTTON_BW_MODE);
	m_wndToolBar.SetButtonStyle(nIndex, m_wndToolBar.GetButtonStyle(nIndex) | TBBS_CHECKBOX);
	m_ComboboxFont.CreateFont(-MulDiv(12, nPixelH, 96), 0, 0, 0, FW_NORMAL, FALSE, FALSE, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH, nullptr);
	CRect rect;
	nIndex = pToolCtrl->CommandToIndex(ID_COMBO_SCALE);
	m_wndToolBar.SetButtonInfo(nIndex, ID_COMBO_SCALE, TBBS_SEPARATOR, MulDiv(36, nPixelW, 96));
	pToolCtrl->GetItemRect(nIndex, &rect);

	if (!m_comboBoxScale.Create(CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VISIBLE |
	                            WS_TABSTOP | WS_VSCROLL, rect, &m_wndToolBar, ID_COMBO_SCALE))
	{
		TRACE0("Не удалось создать комбобокс Масштаб\n");
		return -1;
	}

	m_comboBoxScale.SetFont(&m_ComboboxFont);
	nIndex = pToolCtrl->CommandToIndex(ID_COMBO_PALETTE);
	m_wndToolBar.SetButtonInfo(nIndex, ID_COMBO_PALETTE, TBBS_SEPARATOR, MulDiv(98, nPixelW, 96));
	pToolCtrl->GetItemRect(nIndex, &rect);

	if (!m_comboBoxPalette.Create(CBS_DROPDOWNLIST | CBS_SIMPLE | WS_VISIBLE |
	                              WS_TABSTOP | WS_VSCROLL, rect, &m_wndToolBar, ID_COMBO_PALETTE))
	{
		TRACE0("Не удалось создать комбобокс Палитра\n");
		return -1;
	}

	m_cyToolbar = rect.Height() + MulDiv(12, nPixelH, 96);
	m_wndToolBar.SetHeight(m_cyToolbar);
	m_comboBoxPalette.SetFont(&m_ComboboxFont);
	CString str;

	for (int i = 1; i <= 4; ++i)
	{
		str.Format(_T("x%d"), i);
		m_comboBoxScale.AddString(str);
	}

	for (int i = 0; i < 16; ++i)
	{
		str.Format(_T("Палитра #%02d"), i);
		m_comboBoxPalette.AddString(str);
	}

	m_comboBoxPalette.SetCurSel(m_pSprView->GetPalette());
	m_comboBoxScale.SetCurSel(m_nScale - 1);

	if (m_pSprView)
	{
		if (!m_pSprView->Create(nullptr, _T("BKSprView"), WS_VISIBLE | WS_CHILD, CRect(0, m_cyToolbar, m_cx, m_cy + m_cyToolbar), this, 0))
		{
			TRACE0("Не удалось создать вид SprView\n");
			return -1; // не удалось создать
		}

		SetActiveView(m_pSprView);
	}

	int sw = GetSystemMetrics(SM_CXSCREEN);
	int sh = GetSystemMetrics(SM_CYSCREEN);
	int width = m_cx + m_cxScroll + m_cxBorder;
	int height = m_cy + m_cyScroll + m_cyBorder + m_cyCaption + m_cyToolbar;
	MoveWindow((sw - width) / 2, (sh - height) / 2, width, height, TRUE);
	EnableDocking(CBRS_ALIGN_ANY);
	DockControlBar(&m_wndToolBar);
	RecalcLayout();
	SetButtonState();
	return 0;
}

void CSprWnd::OnSize(UINT nType, int cx, int cy)
{
	CFrameWnd::OnSize(nType, cx, cy); // cx, cy - размеры клиентской области окна
	m_cx = cx;
	m_cy = cy - m_cyToolbar; // m_cx, m_cy - размеры окна scrollview, вместе со скроллом.
	BOOL bSV = FALSE, bSH = FALSE;

	if (m_pSprView)
	{
		m_pSprView->MoveWindow(0, m_cyToolbar, m_cx, m_cy, FALSE);
		m_pSprView->CheckScrollBars(bSH, bSV);
	}

	m_orig_cx = (m_cx - (bSV ? m_cxScroll : 0)) / m_nScale;
	m_orig_cy = (m_cy - (bSH ? m_cyScroll : 0)) / m_nScale;
}

void CSprWnd::SetButtonState()
{
	int nBtnIndex = m_wndToolBar.GetToolBarCtrl().CommandToIndex(ID_BUTTON_COLOR_MODE);
	UINT nBtnStyle = m_wndToolBar.GetButtonStyle(nBtnIndex);
	nBtnStyle = m_pSprView->IsColorMode() ? (nBtnStyle | TBBS_CHECKED) : (nBtnStyle & ~TBBS_CHECKED);
	m_wndToolBar.SetButtonStyle(nBtnIndex, nBtnStyle);
	nBtnIndex = m_wndToolBar.GetToolBarCtrl().CommandToIndex(ID_BUTTON_BW_MODE);
	nBtnStyle = m_wndToolBar.GetButtonStyle(nBtnIndex);
	nBtnStyle = m_pSprView->IsAdaptMode() ? (nBtnStyle | TBBS_CHECKED)  : (nBtnStyle & ~TBBS_CHECKED);
	nBtnStyle = m_pSprView->IsColorMode() ? (nBtnStyle | TBBS_DISABLED) : (nBtnStyle & ~TBBS_DISABLED);
	m_wndToolBar.SetButtonStyle(nBtnIndex, nBtnStyle);
}

void CSprWnd::OnViewColormode()
{
	if (m_pSprView)
	{
		bool bColorMode = !m_pSprView->IsColorMode();
		m_pSprView->SetColorMode(bColorMode);
	}

	SetButtonState();
}

void CSprWnd::OnViewBWMode()
{
	if (m_pSprView)
	{
		bool bBWAdaptMode = !m_pSprView->IsAdaptMode();
		m_pSprView->SetAdaptMode(bBWAdaptMode);
	}

	SetButtonState();
}

void CSprWnd::OnSetPalette()
{
	if (m_pSprView)
	{
		int nPalette = m_comboBoxPalette.GetCurSel();
		m_pSprView->SetPalette(nPalette);
	}
}

void CSprWnd::OnSetScale()
{
	m_nScale = m_comboBoxScale.GetCurSel() + 1;
	m_cx = m_orig_cx * m_nScale;
	m_cy = m_orig_cy * m_nScale;
	int width = m_cx + m_cxScroll + m_cxBorder;
	int height = m_cy + m_cyScroll + m_cyBorder + m_cyCaption + m_cyToolbar;
	BOOL bSV = FALSE;
	BOOL bSH = FALSE;

	if (m_pSprView)
	{
		m_pSprView->SetScale(m_nScale);
		m_pSprView->CheckScrollBars(bSH, bSV);
	}

	CRect rect;
	GetWindowRect(&rect);
	MoveWindow(rect.left, rect.top, width + (bSV ? m_cxScroll : 0), height + (bSH ? m_cyScroll : 0), TRUE);
}

#endif
