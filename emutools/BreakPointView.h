#ifndef CBREAKPOINTVIEW_H
#define CBREAKPOINTVIEW_H

#include <QDockWidget>
#include <QScrollBar>
#include <QScrollEvent>
#include "BreakPoint.h"

class CDebugger;

class CBreakPointView : public QDockWidget
{
    Q_OBJECT

    CDebugger          * m_pDebugger;
    CBreakPointList    * m_pBreakpointList;
    CMemBreakPointList * m_pMemBreakpointList;

    const int winHeaderHight = 35;  // Ofset of first line from window top

    int         numRowsVisible() const {return (height()-winHeaderHight)/m_nlineHeight; }
    int         lineStartPos(const int line) {return line * m_nlineHeight; }

    uint        m_nlineHeight;
    QFont       m_Font;
    QImage      m_hBPIcon, m_hBPCIcon;
    QImage      m_hBPDisIcon, m_hBPCDisIcon;
    QScrollBar *m_pScrollBar;


    int         m_nStartIndex;
    int         m_nIconStart;
    int         m_nAddrStart;
    int         m_nCondStart;
    int         m_nMemTypeStart;

    void       DrawBreakpointLine(int nLine, CBreakPoint *bp, QPainter& pnt);
    uint32_t   GetBreakpointByPos(const QPoint &pos, CBreakPoint **bp = nullptr);


public:
    CBreakPointView(QWidget *parent = nullptr);
    void AttachDebugger( CDebugger *dbg);

    void Update() { if(isVisible()) update(); }

    void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
    void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE;
    void keyPressEvent(QKeyEvent *event) Q_DECL_OVERRIDE;
    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseDoubleClickEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void wheelEvent(QWheelEvent *event) Q_DECL_OVERRIDE;

    void Toggle()
    {
        if(isHidden())
            show();
        else
            hide();
    }


private slots:
    void OnShowContextMenu(const QPoint &pos);
    void OnDeleteBreakpoint(uint32_t addr);
    void OnAddBreakpoint();
    void OnEditBreakpoint(CBreakPoint *bp);
    void OnEnableAllBreakpoints(bool enable);
    void OnRemoveAllBreakpoints();

signals:
     void UpdateDisasmView();
};

#endif // CBREAKPOINTVIEW_H
