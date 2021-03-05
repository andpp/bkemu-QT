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

    const int lineOffset = 12;  // Ofset of disams lines from window top

    void        AttachDebugger(CDebugger *pDebugger);
    int         numRowsVisible() const {return (height()-lineOffset)/m_nlineHeight; }
    int         lineStartPos(const int line) {return line * m_nlineHeight; }

private:
        CDebugger *m_pDebugger;
        uint m_nlineHeight;
        QFont m_Font;

signals:
    void DisasmStepUp();
    void DisasmStepDn();
    void DisasmPgUp(const int wp);
    void DisasmPgDn(const int wp);
    void DisasmCheckBp(const int wp);
    void ShowAddrEdit();
    void HideAddrEdit();
    void ShowLabelEdit(const int nLine);
    void HideLabelEdit();


protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;



};

#endif // CDISASMCTRL_H
