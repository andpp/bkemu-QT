// CTapeCtrlView.cpp: файл реализации
//

#include "pch.h"
//#include "resource.h"
#include "TapeCtrlView.h"
#include "BKMessageBox.h"
#include <QApplication>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

CTapeCtrlView::CTapeCtrlView()
{
    m_TapeCtrlDlg.CreateActions(this);
    setWindowTitle(tr("Tape control"));
}

CTapeCtrlView::~CTapeCtrlView() = default;

void CTapeCtrlView::UpdateToolbar()
{
}

void CTapeCtrlView::UpdateTapeControls(bool bEmuLoad, bool bEmuSave)
{
//	m_TapeCtrlDlg.GetDlgItem(IDC_CHECK_TC_RECORD)->EnableWindow(bEmuSave ? FALSE : TRUE);
    m_TapeCtrlDlg.m_pActRecord->setEnabled(!bEmuSave);
    bool b = bEmuLoad && bEmuSave;
//	m_TapeCtrlDlg.GetDlgItem(IDC_BUTTON_TC_STOP)->EnableWindow(b ? FALSE : TRUE);
    m_TapeCtrlDlg.m_pActStop->setEnabled(!b);
    bool b1 = m_TapeCtrlDlg.GetTape() ? m_TapeCtrlDlg.GetTape()->IsWaveLoaded() : false;
//	m_TapeCtrlDlg.GetDlgItem(IDC_BUTTON_TC_PLAY)->EnableWindow(!b && b1 ? TRUE : FALSE);
    m_TapeCtrlDlg.m_pActPlay->setEnabled(!b && b1);
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

