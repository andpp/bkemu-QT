#include "BreakPointView.h"
#include "Debugger.h"
#include <QWheelEvent>
#include <QMenu>

#include "BreakpointEdit.h"

// массив цветов для подсветки синтаксиса
enum : int
{
    BPCOLOR_TITLE = 0,
    BPCOLOR_ADDR,
    BPCOLOR_COND,
    BPCOLOR_ADR_DISABLED,
    BPCOLOR_COND_DISABLED,
    BPCOLOR_NUM_COLS    // количество цветов
};
static const COLORREF g_crBPColorHighLighting[] =
{
    RGB(0, 0, 0),          // BPCOLOR_TITLE
    RGB(0xff, 0x66, 0),    // BPCOLOR_ADDR
    RGB(0xff, 0x66, 0xcc), // BPCOLOR_COND
    RGB(0x55, 0x55, 0x55), // BPCOLOR_ADR_DISABLED
    RGB(0x55, 0x55, 0xaa), // BPCOLOR_COND_DISABLED
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
    m_nMemTypeStart = m_nAddrStart + int(fm.horizontalAdvance("0000000-0000000  "));

    m_hBPIcon.load(":icons/dbg_bpt");
    m_hBPCIcon.load(":icons/dbg_cbpt");
    m_hBPDisIcon.load(":icons/dbg_bpt_disabled");
    m_hBPCDisIcon.load(":icons/dbg_cbpt_disabled");

    setMinimumSize(m_nCondStart + 150, m_nlineHeight * 10 + winHeaderHight + 5);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(OnShowContextMenu(const QPoint &)));
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

void CBreakPointView::DrawBreakpointLine(int nLine, CBreakPoint *bp, QPainter& pnt)
{
    int pos_y = nLine * m_nlineHeight + winHeaderHight;
    CString strTxt;

    int isActive = bp->IsActive() ? 0 : 2;

    if(bp->GetType() == BREAKPOINT_MEMORY_ACCESS) {
        m_Font.setStrikeOut(!bp->IsActive());
        pnt.setFont(m_Font);
        pnt.setPen(g_crBPColorHighLighting[BPCOLOR_ADDR + isActive]);
        uint16_t addrStart = bp->m_breakAddress >> 16;
        uint16_t addrEnd = bp->m_breakAddress & 0xFFFF;
        strTxt = ::WordToOctString(addrStart);
        if(addrStart != addrEnd)
            strTxt += "-" + ::WordToOctString(addrEnd);
        pnt.drawText(m_nAddrStart, pos_y, strTxt);

        strTxt = "";

        if (((CMemBreakPoint *)bp)->GetAccessType() & BREAKPOINT_MEMACCESS_READ) {
            strTxt += 'R';
        }
        if (((CMemBreakPoint *)bp)->GetAccessType() & BREAKPOINT_MEMACCESS_WRITE) {
            strTxt += 'W';
        }
        pnt.setPen(g_crBPColorHighLighting[BPCOLOR_COND + isActive]);
        pnt.drawText(m_nMemTypeStart, pos_y, strTxt);
    } else {
        if(bp->GetType() == BREAKPOINT_ADDRESS_COND)
            pnt.drawImage(m_nIconStart, pos_y - m_nlineHeight / 2 - 3,
                          bp->IsActive() ? m_hBPCIcon : m_hBPCDisIcon);
        else
            pnt.drawImage(m_nIconStart, pos_y - m_nlineHeight / 2 - 3,
                          bp->IsActive() ? m_hBPIcon : m_hBPDisIcon);


        m_Font.setStrikeOut(!bp->IsActive());
        pnt.setFont(m_Font);
        pnt.setPen(g_crBPColorHighLighting[BPCOLOR_ADDR + isActive]);
        strTxt = ::WordToOctString(bp->m_breakAddress);
        pnt.drawText(m_nAddrStart, pos_y, strTxt);

        if(bp->GetType() == BREAKPOINT_ADDRESS_COND) {
            pnt.setPen(g_crBPColorHighLighting[BPCOLOR_COND + isActive]);
            pnt.drawText(m_nCondStart, pos_y, ((CCondBreakPoint *)bp)->m_cond);
        }
    }
}

void CBreakPointView::paintEvent(QPaintEvent *event)
{
    QDockWidget::paintEvent(event);

    QPainter  pnt(this);
    int nIndex = 0;

    if(!m_pDebugger)
        return;
    m_Font.setStrikeOut(false);
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

uint32_t CBreakPointView::GetBreakpointByPos(const QPoint &pos, CBreakPoint **bp)
{
    int idx = (pos.y() - winHeaderHight) / m_nlineHeight + m_nStartIndex;
    int nIndex = 0;

    uint32_t bpAddr;
    if(bp != nullptr)
        *bp = nullptr;

    {
        CBreakPointList::const_iterator i = m_pBreakpointList->cbegin();
        CBreakPointList::const_iterator last = m_pBreakpointList->cend();
        for(; i != last; i++, nIndex++) {
            if(nIndex == idx) {
                bpAddr = i.key();
                if(bp != nullptr)
                    *bp = i.value();
                return bpAddr;
            }
        }
    }

    return (uint32_t)-1;
}

void CBreakPointView::OnShowContextMenu(const QPoint &pos)
{
   QMenu contextMenu(tr("Breakpoint View menu"), this);

   CBreakPoint *bp;
   uint32_t bpAddr = GetBreakpointByPos(pos, &bp);
   bool isNotEmpty = m_pBreakpointList->count() > 0;

   QAction actDel("Remove", this);
   if(bpAddr != (uint32_t)-1) {
       connect(&actDel, &QAction::triggered, this, [=](){ OnDeleteBreakpoint(bpAddr); });
       contextMenu.addAction(&actDel);
   }
   QAction actEdit("Edit", this);
   if(bpAddr != (uint32_t)-1) {
       connect(&actEdit, &QAction::triggered, this, [=](){ OnEditBreakpoint(bp); });
       contextMenu.addAction(&actEdit);
   }
   QAction actAdd("Add", this);
   connect(&actAdd, &QAction::triggered, this, &CBreakPointView::OnAddBreakpoint);
   contextMenu.addAction(&actAdd);
   contextMenu.addSeparator();

   QAction actDisableAll("Disable All", this);
   connect(&actDisableAll, &QAction::triggered, this, [=](){ OnEnableAllBreakpoints(false); } );
   actDisableAll.setEnabled(isNotEmpty);
   contextMenu.addAction(&actDisableAll);

   QAction actEnableAll("Enable All", this);
   connect(&actEnableAll, &QAction::triggered, this, [=](){ OnEnableAllBreakpoints(true); } );
   actEnableAll.setEnabled(isNotEmpty);
   contextMenu.addAction(&actEnableAll);

   contextMenu.addAction(&actDisableAll);
   QAction actRemoveAll("Remove All", this);
   connect(&actRemoveAll, &QAction::triggered, this, &CBreakPointView::OnRemoveAllBreakpoints);
   actRemoveAll.setEnabled(isNotEmpty);
   contextMenu.addAction(&actRemoveAll);


   contextMenu.exec(mapToGlobal(pos));
}

void CBreakPointView::OnDeleteBreakpoint(uint32_t addr)
{
    m_pDebugger->RemoveBreakpoint(addr);
}

void CBreakPointView::OnEditBreakpoint(CBreakPoint *bp)
{
    CBreakPoint *oldBp = bp;
    CBreakPointEdit m_Dialog(&bp, this);
    m_Dialog.AttachDebugger(m_pDebugger);
    if(m_Dialog.exec() && bp != nullptr) {
        m_pDebugger->RemoveBreakpoint(oldBp);
        m_pDebugger->InsertBreakpoint(bp);
    }

    emit UpdateDisasmView();
    update();
}

void CBreakPointView::OnAddBreakpoint()
{
    CBreakPoint *bp = nullptr;
    OnEditBreakpoint(bp);
}

void CBreakPointView::OnEnableAllBreakpoints(bool enable)
{
    CBreakPointList::const_iterator i = m_pBreakpointList->cbegin();
    CBreakPointList::const_iterator last = m_pBreakpointList->cend();
    for(; i != last; i++) {
        i.value()->SetActive(enable);
    }
    emit UpdateDisasmView();
    update();

}

void CBreakPointView::OnRemoveAllBreakpoints()
{
    m_pDebugger->RemoveAllBreakpoints();
    emit UpdateDisasmView();
    update();
}

void CBreakPointView::keyPressEvent(QKeyEvent *event)
{
    QDockWidget::keyPressEvent(event);
}

void CBreakPointView::mousePressEvent(QMouseEvent *event)
{
    Qt::MouseButtons m_Buttons = event->buttons();
    if (m_Buttons & Qt::MouseButton::LeftButton) {
        CBreakPoint *bp;
        GetBreakpointByPos(event->pos(), &bp);
        if(bp != nullptr) {
            bp->SetActive(!bp->IsActive());
            emit UpdateDisasmView();
            update();
        }

    }
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
