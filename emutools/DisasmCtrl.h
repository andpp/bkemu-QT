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
        uint m_nlineHeight;
        int numRowsVisible() const {return this->height()/m_nlineHeight; }

signals:
    void DisasmStepUp();
    void DisasmStepDn();
    void DisasmCheckBp(const int wp);

protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;



};

#endif // CDISASMCTRL_H
