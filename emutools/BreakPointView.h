#ifndef CBREAKPOINTVIEW_H
#define CBREAKPOINTVIEW_H

#include <QDockWidget>
#include "BreakPoint.h"

class CDebugger;

class CBreakPointView : public QDockWidget
{
    Q_OBJECT

    CDebugger          * m_pDebugger;
    CBreakPointList    * m_pBreakpointList;
    CMemBreakPointList * m_pMemBreakpointList;

    const int lineOffset = 35;  // Ofset of first line from window top

    int         numRowsVisible() const {return (height()-lineOffset)/m_nlineHeight; }
    int         lineStartPos(const int line) {return line * m_nlineHeight; }

    uint        m_nlineHeight;
    QFont       m_Font;
    QImage      m_hBPIcon, m_hBPCIcon;
    QImage      m_hBPDisIcon, m_hBPCDisIcon;


    int         m_nStartIndex;
    int         m_nIconStart;
    int         m_nAddrStart;
    int         m_nCondStart;

    void       DrawBreakpointLine(int nLine, CBreakPoint *pb, QPainter& pnt);

public:
    CBreakPointView(QWidget *parent = nullptr);
    void AttachDebugger( CDebugger *dbg);

    void Update();

    void resizeEvent(QResizeEvent* event) override;
    void paintEvent(QPaintEvent* event) override;
};

#endif // CBREAKPOINTVIEW_H
