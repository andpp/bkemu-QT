#include "MemDumpView.h"

CMemDumpView::CMemDumpView(QWidget *parent) : QDockWidget(parent)
{
    m_MemDumpDlg = new CMemDumpDlg(this);
    setWidget(m_MemDumpDlg);
}


