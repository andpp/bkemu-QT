#include "DisasmView.h"

CDisasmView::CDisasmView()
{
    m_pDisasmDlg = new CDisasmDlg(this);
    setWidget(m_pDisasmDlg);
}

void CDisasmView::AttachDebugger(CDebugger *pDebugger)
{
    m_pDebugger = pDebugger;
    m_pDisasmDlg->AttachDebugger(pDebugger);

}

