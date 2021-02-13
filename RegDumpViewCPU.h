#pragma once

#include <QDockWidget>
#include "RegDumpCPUDlg.h"
#include "Debugger.h"

class CRegDumpViewCPU : public QDockWidget
{
    Q_OBJECT
protected:
    CRegDumpCPUDlg *m_RegDumpCPUDlg;

public:
    CRegDumpViewCPU(QWidget *parent = nullptr);
    virtual ~CRegDumpViewCPU() {};

    void AttachDebugger(CDebugger *pDbgr)
    {
        m_RegDumpCPUDlg->AttachDebugger(pDbgr);
        m_pDebugger = pDbgr;
    }

    void SetFreqParam()
    {
        m_RegDumpCPUDlg->DisplayFreqParam();
    }

    void UpdateFreq()
    {
        m_RegDumpCPUDlg->UpdateFreq();
    }

    void DisplayRegDump()
    {
        if (!this->isHidden())
        {
            m_RegDumpCPUDlg->DisplayRegisters();
            m_RegDumpCPUDlg->DisplayPortRegs();
            m_RegDumpCPUDlg->DisplayAltProData();
            // выводим на экран данные FDD
        }
    }

private:
    CDebugger        *m_pDebugger;

};
