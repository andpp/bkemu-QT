#include "RegDumpViewCPU.h"

CRegDumpViewCPU::CRegDumpViewCPU(QWidget *parent) : QDockWidget(parent)
{
    m_RegDumpCPUDlg = new CRegDumpCPUDlg(this);
    setWidget(m_RegDumpCPUDlg);
}

CRegDumpViewCPU::~CRegDumpViewCPU() = default;
