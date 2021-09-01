#include "BreakPointView.h"
#include "Debugger.h"
#include <QWheelEvent>
#include <QMenu>

// массив цветов для подсветки синтаксиса
enum : int
{
    BPCOLOR_TITLE = 0,
    BPCOLOR_ADDR,
    BPCOLOR_COND,
    BPCOLOR_ADR_ACTIVE,
    BPCOLOR_COND_ACTIVE,
    BPCOLOR_NUM_COLS    // количество цветов
};
static const COLORREF g_crBPColorHighLighting[] =
{
    RGB(0, 0, 0),       // MEMCOLOR_TITLE
    RGB(0xff, 0x66, 0), // MEMCOLOR_LEFT_VAL
    RGB(0, 0x66, 0xcc), // MEMCOLOR_RIGHT_VAL
    RGB(0x66, 0, 0xcc), // MEMCOLOR_LEFT_CHAR
    RGB(0x60, 0x66, 0xff), // MEMCOLOR_RIGHT_CHAR
    RGB(0xFF, 0, 0),       // MEMCOLOR_TITLE
    RGB(0xff, 0xFF, 0), // MEMCOLOR_LEFT_VAL
    RGB(0, 0xFF, 0xcc), // MEMCOLOR_RIGHT_VAL
    RGB(0xFF, 0, 0xcc), // MEMCOLOR_LEFT_CHAR
    RGB(0x60, 0xFF, 0xff), // MEMCOLOR_RIGHT_CHAR
};

CBreakPointView::CBreakPointView(QWidget *parent)
    : QDockWidget(parent)
    , m_Font("Monospace")
    , m_nStartIndex(0)
{
    m_Font.setStyleHint(QFont::TypeWriter);
    m_Font.setPixelSize(14);

    QFontMetricsF fm(m_Font);
    m_nlineHeight = fm.height();  // font height

    m_nIconStart = 10;
    m_nAddrStart = m_nIconStart + 20;
    m_nCondStart = m_nAddrStart + int(fm.horizontalAdvance("0000000  "));

    m_hBPIcon.load(":icons/dbg_bpt");
    m_hBPCIcon.load(":icons/dbg_cbpt");
    m_hBPDisIcon.load(":icons/dbg_bpt_disabled");
    m_hBPCDisIcon.load(":icons/dbg_cbpt_disabled");

    setMinimumSize(m_nCondStart + 150, m_nlineHeight * 10 + winHeaderHight + 5);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(ShowContextMenu(const QPoint &)));
}

void CBreakPointView::AttachDebugger( CDebugger *dbg)
{
    m_pDebugger = dbg;
    m_pBreakpointList = &dbg->m_breakpointList;
    m_pMemBreakpointList = &dbg->m_memBreakpointList;

}

void CBreakPointView::resizeEvent(QResizeEvent* event)
{
    QDockWidget::resizeEvent(event);
}

void CBreakPointView::DrawBreakpointLine(int nLine, CBreakPoint *pb, QPainter& pnt)
{
    int pos_y = nLine * m_nlineHeight + winHeaderHight;
    CString strTxt;

    int isActive = pb->IsActive() ? 0 : 2;

    if(pb->GetType() == BREAKPOINT_ADDRESS_COND)
        pnt.drawImage(m_nIconStart, pos_y - m_nlineHeight / 2 - 3,
                      pb->IsActive() ? m_hBPCIcon : m_hBPCDisIcon);
    else
        pnt.drawImage(m_nIconStart, pos_y - m_nlineHeight / 2 - 3,
                      pb->IsActive() ? m_hBPIcon : m_hBPDisIcon);


    pnt.setPen(g_crBPColorHighLighting[BPCOLOR_ADDR + isActive]);
    strTxt = ::WordToOctString(pb->m_breakAddress);
    pnt.drawText(m_nAddrStart, pos_y, strTxt);

    if(pb->GetType() == BREAKPOINT_ADDRESS_COND) {
        pnt.setPen(g_crBPColorHighLighting[BPCOLOR_COND + isActive]);
        pnt.drawText(m_nCondStart, pos_y, ((CCondBreakPoint *)pb)->m_cond);
    }
}

void CBreakPointView::paintEvent(QPaintEvent *event)
{
    QDockWidget::paintEvent(event);

    QPainter  pnt(this);
    int nIndex = 0;

    if(!m_pDebugger)
        return;

    pnt.setFont(m_Font);

    int nLines = numRowsVisible() - 1;

    pnt.setPen(g_crBPColorHighLighting[BPCOLOR_TITLE]);
    pnt.drawText(m_nAddrStart, winHeaderHight, "Address  Condition");

    CBreakPointList::const_iterator i = m_pBreakpointList->cbegin();
    CBreakPointList::const_iterator last = m_pBreakpointList->cend();
    for(; i != last; i++, nIndex++) {
        if(nIndex > (nLines + m_nStartIndex))
            break;
        if(nIndex < m_nStartIndex)
            continue;
        DrawBreakpointLine(nIndex - m_nStartIndex + 1, i.value(), pnt);
    }
}

void CBreakPointView::ShowContextMenu(const QPoint &pos)
{
   QMenu contextMenu(tr("Context menu"), this);

   int idx = (pos.y() - winHeaderHight) / m_nlineHeight + m_nStartIndex;
   int nIndex = 0;

//   CBreakPoint *cb = nullptr;
   uint32_t addr = -1;

   CBreakPointList::const_iterator i = m_pBreakpointList->cbegin();
   CBreakPointList::const_iterator last = m_pBreakpointList->cend();
   for(; i != last; i++, nIndex++) {
       if(nIndex == idx) {
           addr = i.key();
           break;
       }
   }

   QAction actDel("Remove", this);
   if(addr != (uint32_t)-1) {
       connect(&actDel, &QAction::triggered, this, [=](){ this->DeleteBreakpoint(addr); });
       contextMenu.addAction(&actDel);
   }
   QAction actAdd("Add", this);
   connect(&actAdd, &QAction::triggered, this, [=](){ this->AddBreakpoint(); });
   contextMenu.addAction(&actAdd);

   contextMenu.exec(mapToGlobal(pos));
}

void CBreakPointView::DeleteBreakpoint(uint32_t addr)
{
    m_pDebugger->RemoveBreakpoint(addr);
}

void CBreakPointView::AddBreakpoint()
{

}


void CBreakPointView::keyPressEvent(QKeyEvent *event)
{
    QDockWidget::keyPressEvent(event);
}

void CBreakPointView::mousePressEvent(QMouseEvent *event)
{

}

void CBreakPointView::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void CBreakPointView::wheelEvent(QWheelEvent *event)
{
    QPoint degrees = event->angleDelta() / 8;

    if(degrees.y() == 0) return;

    if(degrees.y() > 0) {
        if (m_nStartIndex) {
            m_nStartIndex--;
        }
    } else {
        if (m_nStartIndex < (m_pBreakpointList->count() - numRowsVisible())) {
            m_nStartIndex++;
        }
    }

    repaint();
}
