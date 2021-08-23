// CTapeCtrlView.cpp: файл реализации
//

#include "pch.h"
//#include "resource.h"
#include "TapeCtrlView.h"
#include "BKMessageBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

// CTapeCtrlView

//IMPLEMENT_DYNAMIC(CTapeCtrlView, CDockablePane)

CTapeCtrlView::CTapeCtrlView()
{
    m_TapeCtrlDlg.CreateActions(this);
}

CTapeCtrlView::~CTapeCtrlView()
{
}

//BEGIN_MESSAGE_MAP(CTapeCtrlView, CDockablePane)
//	ON_WM_CREATE()
//	ON_WM_SIZE()
//	ON_WM_PAINT()
//END_MESSAGE_MAP()


// обработчики сообщений CTapeCtrlView

//int CTapeCtrlView::OnCreate(LPCREATESTRUCT lpCreateStruct)
//{
//	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
//	{
//		return -1;
//	}

//	if (!m_TapeCtrlDlg.Create(IDD_TAPE_CONTROL, this))
//	{
//		g_BKMsgBox.Show(IDS_BK_ERROR_NOCTAPECTRLDLGINIT);
//		TRACE0("Не удалось создать диалог Tape Control\n");
//		return -1;      // не удалось создать
//	}

//	AdjustLayout();
//	return 0;
//}

//void CTapeCtrlView::OnSize(UINT nType, int cx, int cy)
//{
//	AdjustLayout();
//	CDockablePane::OnSize(nType, cx, cy);
//}

//void CTapeCtrlView::AdjustLayout()
//{
//	if (GetSafeHwnd())
//	{
//		CSize size(m_TapeCtrlDlg.GetMinSize());
//		int c = IsDocked() ? GetSystemMetrics(SM_CYSMCAPTION) : 0;
//		size.cy += c;
//		SetMinSize(size);
//		CRect rectClient;
//		GetClientRect(rectClient);
//		int minx = size.cx >= rectClient.Width() ? size.cx : rectClient.Width();
//		int miny = size.cy >= rectClient.Height() ? size.cy : rectClient.Height();
//		m_TapeCtrlDlg.SetWindowPos(nullptr, rectClient.left, rectClient.top, minx, miny, SWP_NOACTIVATE | SWP_NOZORDER);
//	}

//	CDockablePane::AdjustLayout();
//}

//void CTapeCtrlView::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting

//	// TODO: добавьте свой код обработчика сообщений
//	// Не вызывать CDockablePane::OnPaint() для сообщений рисования
//	if (IsVisible())
//	{
//		m_TapeCtrlDlg.Invalidate(FALSE);
//	}
//}

void CTapeCtrlView::UpdateTapeControls(bool bEmuLoad, bool bEmuSave)
{
//	m_TapeCtrlDlg.GetDlgItem(IDC_CHECK_TC_RECORD)->EnableWindow(bEmuSave ? FALSE : TRUE);
    bool b = bEmuLoad && bEmuSave;
//	m_TapeCtrlDlg.GetDlgItem(IDC_BUTTON_TC_STOP)->EnableWindow(b ? FALSE : TRUE);
    bool b1 = m_TapeCtrlDlg.GetTape() ? m_TapeCtrlDlg.GetTape()->IsWaveLoaded() : false;
//	m_TapeCtrlDlg.GetDlgItem(IDC_BUTTON_TC_PLAY)->EnableWindow(!b && b1 ? TRUE : FALSE);
}

void CTapeCtrlView::StartPlayTape()
{
	m_TapeCtrlDlg.RunPlay();
}

void CTapeCtrlView::InitParams(CTape *pTape)
{
	m_TapeCtrlDlg.SetTape(pTape);
	m_TapeCtrlDlg.InitParam();
}

