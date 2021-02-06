#ifndef CDISASMCTRL_H
#define CDISASMCTRL_H
#pragma once

#include <QPainter>

#include "Debugger.h"

class CDebugger;

class CDisasmCtrl : public QWidget
{
    Q_OBJECT

public:
    CDisasmCtrl();
    virtual ~CDisasmCtrl() {};

    void        AttachDebugger(CDebugger *pDebugger);

private:
        CDebugger *m_pDebugger;

        int numRowsVisible() const;

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) override;



};

#endif // CDISASMCTRL_H
