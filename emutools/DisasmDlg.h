#pragma once

#include <QWidget>
#include "NumberEditCtrl.h"
#include "DisasmCtrl.h"
#include "Debugger.h"

class CDebugger;
class CDisasmCtrl;
class CNumberEdit;

class CDisasmDlg : public QWidget
{
    Q_OBJECT

    CDisasmCtrl         *m_ListDisasm;
    CNumberEdit          *m_EditAddr;
    CDebugger           *m_pDebugger;

public:
    explicit CDisasmDlg(QWidget *parent = nullptr);
    ~CDisasmDlg();

    void                AttachDebugger(CDebugger *pDebugger);
    CDisasmCtrl *GetDisasmCtrl() { return m_ListDisasm; }

protected:
    virtual void resizeEvent(QResizeEvent *event);

public slots:
    void OnDisasmTopAddressUpdate();
    void OnDisasmCurrentAddressChange(int wp);
    void OnDisasmStepUp();
    void OnDisasmStepDn();
    void OnDisasmPgUp(const int wp = 0);
    void OnDisasmPgDn(const int wp = 0);
    void OnDisasmDelBp(const int wp);
    void OnDisasmCheckBp(const int wp, const bool cond);
    void OnShowAddrEdit();
    void OnHideAddrEdit();
    void OnShowLabelEdit(const int nLine);
    void OnHideLabelEdit();

signals:
    void UpdateBreakPointView();
};
