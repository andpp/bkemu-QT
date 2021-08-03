#pragma once

#include <QDockWidget>
#include "DisasmDlg.h"
#include "Debugger.h"

class CDisasmView : public QDockWidget
{
    Q_OBJECT

public:
    CDisasmView(QWidget *parent = nullptr);
    virtual ~CDisasmView() {};

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;

signals:
    void DebugBreak();
    void DebugStepinto();
    void DebugStepover();
    void DebugStepout();
#ifdef ENABLE_BACKTRACE
    void DebugStepback();
    void DebugStepforward();
#endif

private:
    CDisasmDlg     *m_pDisasmDlg;
    CDebugger      *m_pDebugger;

public:
    void            AttachDebugger(CDebugger *pDebugger);

};

