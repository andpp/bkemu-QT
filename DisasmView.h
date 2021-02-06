#ifndef CDISASMVIEW_H
#define CDISASMVIEW_H

#include <QDockWidget>
#include "DisasmDlg.h"
#include "Debugger.h"

class CDisasmView : public QDockWidget
{
    Q_OBJECT

public:
    CDisasmView();
    virtual ~CDisasmView() {};

private:
    CDisasmDlg     *m_pDisasmDlg;
    CDebugger      *m_pDebugger;

public:
    void            AttachDebugger(CDebugger *pDebugger);

};

#endif // CDISASMVIEW_H
