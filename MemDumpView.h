#pragma once

#include <QDockWidget>
#include "MemDumpDlg.h"
#include "Debugger.h"

class CMemDumpView : public QDockWidget
{
    Q_OBJECT
protected:
    CMemDumpDlg *m_MemDumpDlg;

public:
    CMemDumpView(QWidget *parent = nullptr);
    virtual ~CMemDumpView() {};

    void AttachDebugger(CDebugger *pDbgr)
    {
        m_MemDumpDlg->AttachDebugger(pDbgr);
        m_pDebugger = pDbgr;
    }

    void DisplayMemDump()
    {
        if (!isHidden())  repaint();
    }

    void Toggle()
    {
        if(isHidden())
            show();
        else
            hide();
    }

    void SetDumpAddress(u_int16_t addr)
    {
        m_MemDumpDlg->SetDumpAddress(addr);
    }
private:
    CDebugger        *m_pDebugger;

};
