#include "MemDumpView.h"
#include "MainWindow.h"

CMemDumpView::CMemDumpView(QWidget *parent) : QDockWidget(parent)
{
    m_MemDumpDlg = new CMemDumpDlg(this);
    setWidget(m_MemDumpDlg);

    connect(m_MemDumpDlg,  &CMemDumpDlg::UpdateWachpointView,  (CMainFrame *)parent, &CMainFrame::OnUpdateWatchpoinView);
}


