#ifndef DISASMDLG_H
#define DISASMDLG_H

#pragma once

#include <QWidget>
#include "EnterEditCtrl.h"
#include "DisasmCtrl.h"
#include "Debugger.h"

class CDebugger;
class CDisasmCtrl;
class CEnterEdit;

class CDisasmDlg : public QWidget
{
    Q_OBJECT

    CDisasmCtrl         *m_ListDisasm;
    CEnterEdit          *m_EditAddr;
    CDebugger           *m_pDebugger;

public:
    explicit CDisasmDlg(QWidget *parent = nullptr);
    ~CDisasmDlg();

    void                AttachDebugger(CDebugger *pDebugger);

public slots:
    void OnDisasmTopAddressUpdate();
    void OnDisasmCurrentAddressChange(int wp);
    void OnDisasmStepUp();
    void OnDisasmStepDn();
    void OnDisasmPgUp(const int wp);
    void OnDisasmPgDn(const int wp);
    void OnDisasmCheckBp(const int wp);

    CDisasmCtrl *GetDisasmCtrl() { return m_ListDisasm; }

};

#endif // DISASMDLG_H
