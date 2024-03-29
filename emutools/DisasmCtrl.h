#ifndef CDISASMCTRL_H
#define CDISASMCTRL_H
#pragma once

#include <QPainter>
#include <QTimer>
#include "Debugger.h"

struct DbgLineLayout {
    int DBG_LINE_BP_START;
    int DBG_LINE_BP_WIDTH;
    int DBG_LINE_CUR_START;
    int DBG_LINE_CUR_WIDTH;
    int DBG_LINE_NEXTLINE_POS;
    int DBG_LINE_NEXTLINE_WIDTH;
    int DBG_LINE_LBL_START;
    int DBG_LINE_LBL_WIDTH;
    int DBG_LINE_ADR_START;
    int DBG_LINE_ADR_WIDTH;
    int DBG_LINE_INS_START;
    int DBG_LINE_INS_WIDTH;
    int DBG_LINE_COM_START;
    int DBG_LINE_COM_WIDTH;

    void RecalculatePositions(bool isLabelEnabled)
    {
        DBG_LINE_BP_WIDTH     = 16;
        DBG_LINE_CUR_WIDTH    = 16;
        DBG_LINE_NEXTLINE_WIDTH = 10;
        DBG_LINE_LBL_WIDTH    = isLabelEnabled ? 120 : 0;
        DBG_LINE_ADR_WIDTH    = 65;
        DBG_LINE_INS_WIDTH    = 280;

        DBG_LINE_BP_START     = 0;
        DBG_LINE_CUR_START    = DBG_LINE_BP_START + DBG_LINE_BP_WIDTH;         // 16;
        DBG_LINE_NEXTLINE_POS = DBG_LINE_CUR_START + DBG_LINE_CUR_WIDTH/2 - 2; // 22
        DBG_LINE_ADR_START    = DBG_LINE_CUR_START + DBG_LINE_CUR_WIDTH;       // 32;
        DBG_LINE_LBL_START    = DBG_LINE_ADR_START + DBG_LINE_ADR_WIDTH;
        DBG_LINE_INS_START    = DBG_LINE_LBL_START + DBG_LINE_LBL_WIDTH;       // 100;
        DBG_LINE_COM_START    = DBG_LINE_INS_START + DBG_LINE_INS_WIDTH;       // 350;
    }
};

constexpr auto DBG_RES_BEFORE_TOP = -1;
constexpr auto DBG_RES_AFTER_BOTTOM = -2;

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

    DbgLineLayout m_LineLayout;

private:
        CDebugger *m_pDebugger;
        uint m_nlineHeight;
        QFont m_Font;
        QTimer m_DblClickTimer;
        bool m_bIsDblClick;
        Qt::MouseButtons m_DblClickButtons;
        QPoint m_DblClickPos;

signals:
    void DisasmStepUp();
    void DisasmStepDn();
    void DisasmPgUp(const int wp);
    void DisasmPgDn(const int wp);
    void DisasmDelBP(const int wp);
    void DisasmCheckBp(const int wp, const bool cond);
    void ShowAddrEdit();
    void HideAddrEdit();
    void ShowLabelEdit(const int nLine, CString str);
    void HideLabelEdit();
    void ShowAsmEdit(const int nLine, CString str);
    void HideAsmEdit();


protected:
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

public slots:
    void DblClickTimeout();



};

#endif // CDISASMCTRL_H
