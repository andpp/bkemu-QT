#include "WatchpointView.h"
#include "WatchpointEdit.h"
#include "Debugger.h"
#include "Board.h"
#include <QWheelEvent>
#include <QMenu>

#include "BKMessageBox.h"

enum : int
{
    WPEDIT_MODE_BYTE = 0,
    WPEDIT_MODE_WORD,
};

// массив цветов для подсветки синтаксиса
enum : int
{
    WPCOLOR_TITLE = 0,
    WPCOLOR_ADDR,
    WPCOLOR_VALUE,
    WPCOLOR_SYMBOL,
    WPCOLOR_NUM_COLS    // количество цветов
};
static const COLORREF g_crBPColorHighLighting[] =
{
    RGB(0, 0, 0),          // WPCOLOR_TITLE
    RGB(0xff, 0x66, 0),    // BPCOLOR_ADDR
    RGB(0xff, 0x66, 0xcc), // WPCOLOR_VALUE
    RGB(0x66, 0x66, 0),    // BPCOLOR_SYMBOL
};

CWatchpointView::CWatchpointView(QWidget *parent)
    : QDockWidget(parent)
    , m_pDebugger(nullptr)
    , m_Font(g_FontMonospace)
    , m_nStartIndex(0)
    , m_pEditingWP(nullptr)
{
    m_Font.setStyleHint(QFont::TypeWriter);
    m_Font.setPixelSize(14);

    QFontMetricsF fm(m_Font);
    m_nlineHeight = fm.height();  // font height

    m_nAddrStart = 10;
    m_nValuesStart = m_nAddrStart + int(fm.horizontalAdvance("0000000      "));

    m_nWordWidth = int(fm.horizontalAdvance("0000000 "));

    setMinimumSize(m_nValuesStart + 150, m_nlineHeight * 10 + winHeaderHight + 5);

    setContextMenuPolicy(Qt::CustomContextMenu);

    m_pScrollBar = new QScrollBar(Qt::Orientation::Vertical, this);
    m_pScrollBar->resize(10, height() - winHeaderHight);
    m_pScrollBar->move(width()-m_pScrollBar->width()-4, winHeaderHight);
    m_pScrollBar->setMinimum(0);

    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(OnShowContextMenu(const QPoint &)));
    connect(m_pScrollBar, &QScrollBar::valueChanged, this, [=]{m_nStartIndex = m_pScrollBar->value(); Update(); });

    AddWatchpoint(01000, 2);
    AddWatchpoint(02000, 4, WPTYPE_BYTE);

    m_pNumberEdit = new CNumberEdit(8, this);
    m_pNumberEdit->hide();
    m_pNumberEdit->setMinimumWidth(m_nWordWidth/2);
    m_Font.setStyleHint(QFont::TypeWriter);
    m_pNumberEdit->setFont(m_Font);

    connect(m_pNumberEdit, &CNumberEdit::AddressUpdated, this, &CWatchpointView::OnEditFinished);
}

void CWatchpointView::AttachDebugger( CDebugger *dbg)
{
    m_pDebugger = dbg;
}

bool CWatchpointView::AddWatchpoint(u_int16_t startAddr, uint16_t size, int type)
{
    CWatchPoint *wp = new CWatchPoint(startAddr, size, type);
    if(m_pWatchpointTable.contains(startAddr)) {
        delete m_pWatchpointTable.value(startAddr);
        m_pWatchpointTable.remove(startAddr);
    }
    m_pWatchpointTable.insert(startAddr, wp);

    return true;
}

bool CWatchpointView::RemoveWatchpoint(u_int16_t startAddr, uint16_t size, int type)
{
    if(m_pWatchpointTable.contains(startAddr)) {
        delete m_pWatchpointTable.value(startAddr);
        m_pWatchpointTable.remove(startAddr);
    }
}

void CWatchpointView::resizeEvent(QResizeEvent* event)
{
    QDockWidget::resizeEvent(event);
    m_pScrollBar->resize(10, height() - winHeaderHight);
    m_pScrollBar->move(width()-m_pScrollBar->width()-4, winHeaderHight);
}

void CWatchpointView::DrawWatchpointLine(int nLine, CWatchPoint *wp, QPainter& pnt)
{
    int pos_y = nLine * m_nlineHeight + winHeaderHight;
    CString strTxt;

    if(m_pDebugger->m_SymTable.Contains(wp->m_nAddr)) {
        pnt.setPen(g_crBPColorHighLighting[WPCOLOR_SYMBOL]);
        if(wp->m_bShowSymbolName)
            strTxt = m_pDebugger->m_SymTable.GetSymbolForAddr(wp->m_nAddr);
        else
            strTxt = ::WordToOctString(wp->m_nAddr);
    } else {
        pnt.setPen(g_crBPColorHighLighting[WPCOLOR_ADDR]);
        strTxt = ::WordToOctString(wp->m_nAddr);
    }
    pnt.drawText(m_nAddrStart, pos_y, strTxt);

    strTxt = "";

    switch(wp->GetType()) {
        case WPTYPE_BYTE: {
            pnt.setFont(m_Font);
            uint16_t addrStart = wp->m_nAddr;
            for(int i=0; i< wp->m_nSize; i++) {
                strTxt += ::ByteToOctString(m_pDebugger->GetBoard()->GetByteIndirect(addrStart + i)) + " ";
            }

            break;
        }
        case WPTYPE_WORD: {
            pnt.setFont(m_Font);
            uint16_t addrStart = wp->m_nAddr;
            for(int i=0; i< wp->m_nSize; i+=2) {
                strTxt += ::WordToOctString(m_pDebugger->GetBoard()->GetWordIndirect(addrStart + i)) + " ";
            }
            break;
        }
        case WPTYPE_STRING: {
//            pnt.setFont(m_Font);
//            pnt.setPen(g_crBPColorHighLighting[BPCOLOR_ADDR]);
//            uint16_t addrStart = wp->m_nAddr;
//            for(int i=0; i< wp->m_nSize; i+=2) {
//                strTxt = ::WordToOctString(m_pDebugger->GetBoard()->GetWordIndirect(addrStart + i)) + " ";
//            }
            break;
        }
        default: {
                break;
        }
    }
       pnt.setPen(g_crBPColorHighLighting[WPCOLOR_VALUE]);
       pnt.drawText(m_nValuesStart, pos_y, strTxt);
}


void CWatchpointView::paintEvent(QPaintEvent *event)
{
    int pos_y = winHeaderHight + m_nlineHeight;
    CString strTxt;
    int nLines = numRowsVisible() - 1;

    m_pScrollBar->setMaximum(max(0, m_pWatchpointTable.count() - numRowsVisible()));
    QDockWidget::paintEvent(event);

    QPainter  pnt(this);
    int nIndex = 0;

    if(!m_pDebugger)
        return;

    pnt.setPen(g_crBPColorHighLighting[WPCOLOR_TITLE]);

    pnt.setFont(m_Font);
    pnt.drawText(m_nAddrStart, winHeaderHight, "WatchPoint");

    pnt.setFont(m_Font);
    pnt.drawText(m_nValuesStart, winHeaderHight, "Value");

    m_Font.setBold(false);
    pnt.setFont(m_Font);

    if (m_nStartIndex > (m_pWatchpointTable.count() - numRowsVisible())) {
        m_nStartIndex = (m_pWatchpointTable.count() - numRowsVisible());
        if (m_nStartIndex < 0)
            m_nStartIndex = 0;
    }

    CWatchPointList::const_iterator i = m_pWatchpointTable.cbegin();
    CWatchPointList::const_iterator last = m_pWatchpointTable.cend();
    for(; i != last; i++, nIndex++) {
        if(nIndex > (nLines + m_nStartIndex))
            break;
        if(nIndex < m_nStartIndex)
            continue;
        DrawWatchpointLine(nIndex - m_nStartIndex + 1, i.value(), pnt);
    }
}

CWatchPoint *CWatchpointView::GetWatchpointByPos(const QPoint &pos)
{
    int idx = (pos.y() - winHeaderHight) / m_nlineHeight + m_nStartIndex;

    QList<uint16_t> addrs = m_pWatchpointTable.keys();

    if (idx < addrs.size())
        return m_pWatchpointTable.value(addrs[idx]);
    else
        return nullptr;
}


void CWatchpointView::OnShowContextMenu(const QPoint &pos)
{
   QMenu contextMenu(tr("Watchpoint View menu"), this);

   CWatchPoint *wp = GetWatchpointByPos(pos);
   bool isNotEmpty = m_pWatchpointTable.count() > 0;

   QAction actDel("Remove", this);
   if(wp != nullptr) {
       connect(&actDel, &QAction::triggered, this, [=](){ OnDeleteWatchpoint(wp); });
       contextMenu.addAction(&actDel);
   }
   QAction actEdit("Edit", this);
   if(wp != nullptr) {
       connect(&actEdit, &QAction::triggered, this, [=](){ OnEditWatchpoint(wp); });
       contextMenu.addAction(&actEdit);
   }
   QAction actAdd("Add", this);
   connect(&actAdd, &QAction::triggered, this, &CWatchpointView::OnAddWatchpoint);
   contextMenu.addAction(&actAdd);
   contextMenu.addSeparator();

   QAction actRemoveAll("Remove All", this);
   connect(&actRemoveAll, &QAction::triggered, this, &CWatchpointView::OnRemoveAllWatchpoints);
   actRemoveAll.setEnabled(isNotEmpty);
   contextMenu.addAction(&actRemoveAll);


   contextMenu.exec(mapToGlobal(pos));
}

void CWatchpointView::OnDeleteWatchpoint(CWatchPoint *wp)
{
    m_pWatchpointTable.remove(wp->m_nAddr);
    emit UpdateDisasmView();
    emit UpdateMemoryView();
    Update();
}

void CWatchpointView::OnEditWatchpoint(CWatchPoint *wp)
{
    CWatchPoint tmp_wp = *wp;
    CWatchpointEdit m_Dialog(&tmp_wp, m_pDebugger, this);

    if(m_Dialog.exec()) {
        RemoveWatchpoint(wp->m_nAddr, wp->m_nSize, wp->m_nType);
        AddWatchpoint(tmp_wp.m_nAddr, tmp_wp.m_nSize, tmp_wp.m_nType);
    }

    update();
}

void CWatchpointView::OnAddWatchpoint()
{
    CWatchPoint tmp_wp(0);
    CWatchpointEdit m_Dialog(&tmp_wp, m_pDebugger, this);

    if(m_Dialog.exec()) {
        RemoveWatchpoint(tmp_wp.m_nAddr, tmp_wp.m_nSize, tmp_wp.m_nType);
        AddWatchpoint(tmp_wp.m_nAddr, tmp_wp.m_nSize, tmp_wp.m_nType);
    }
    update();
}

void CWatchpointView::RemoveAllWatchpoints()
{
    for (CWatchPoint *i : qAsConst(m_pWatchpointTable)) {
        delete i;
    }

    m_pWatchpointTable.clear();

    update();
}

void CWatchpointView::OnRemoveAllWatchpoints()
{
    RemoveAllWatchpoints();
}

void CWatchpointView::OnEditFinished()
{
    if(m_pEditingWP == nullptr)
        return;

    uint16_t addr = m_pNumberEdit->getMisc();
    uint16_t val = m_pNumberEdit->getValue();
    switch(m_pEditingWP->m_nType) {
        case WPTYPE_BYTE: {
            m_pDebugger->GetBoard()->SetByteIndirect(addr, val);
            break;
        }
        case WPTYPE_WORD: {
            m_pDebugger->GetBoard()->SetWordIndirect(addr, val);
            break;
        }
        default:
            break;
    }

    repaint();
    emit UpdateDisasmView();
    emit UpdateMemoryView();

}

void CWatchpointView::keyPressEvent(QKeyEvent *event)
{
    QDockWidget::keyPressEvent(event);
}

void CWatchpointView::mousePressEvent(QMouseEvent *event)
{
    Qt::MouseButtons m_Buttons = event->buttons();
    if (m_Buttons & Qt::MouseButton::LeftButton) {
        if(event->pos().y() > winHeaderHight) {
            if(event->pos().x() < m_nValuesStart) {
                CWatchPoint *wp = GetWatchpointByPos(event->pos());
                if(wp) {
                    wp->ShowSymbol(!wp->m_bShowSymbolName);
                    update();
                }
            }
        }
    }
    m_pNumberEdit->hide();
}

void CWatchpointView::mouseDoubleClickEvent(QMouseEvent *event)
{
    Qt::MouseButtons m_Buttons = event->buttons();
    if (m_Buttons & Qt::MouseButton::LeftButton) {
        if(event->pos().y() >= winHeaderHight && event->pos().x() > m_nValuesStart) {
            CWatchPoint *wp = GetWatchpointByPos(event->pos());
            if(wp) {
                CString strTxt;
                uint16_t addr;

                switch(wp->m_nType) {
                    case WPTYPE_WORD: {
                        int offset = (event->pos().x()-m_nValuesStart) / m_nWordWidth;

                        if(offset >= wp->m_nSize/2) {
                            return;
                        }

                        int line = (event->pos().y() - winHeaderHight) / m_nlineHeight;
                        addr = wp->m_nAddr + offset * 2;
                        strTxt = ::WordToOctString(m_pDebugger->GetBoard()->GetWordIndirect(addr));
                        m_pNumberEdit->setBase(8);
                        m_pNumberEdit->setWidth(m_nWordWidth);
                        m_pNumberEdit->move(m_nValuesStart + offset*m_nWordWidth-2, line * m_nlineHeight + winHeaderHight+2);
                        break;
                    }
                    case WPTYPE_BYTE: {
                        int m_nByteWidth = m_nWordWidth/2;
                        int offset = (event->pos().x()-m_nValuesStart) / m_nByteWidth;

                        if(offset >= wp->m_nSize) {
                            return;
                        }

                        int line = (event->pos().y() - winHeaderHight) / m_nlineHeight;
                        addr = wp->m_nAddr + offset;
                        strTxt = ::ByteToOctString(m_pDebugger->GetBoard()->GetByteIndirect(addr));
                        m_pNumberEdit->setBase(-8);
                        m_pNumberEdit->setWidth(m_nByteWidth);
                        m_pNumberEdit->move(m_nValuesStart + offset * m_nByteWidth-2, line * m_nlineHeight + winHeaderHight+2);
                        break;
                    }
                    case WPTYPE_STRING:
                    case WPTYPE_POINTER: {
                        return;
                    }
                    default: {
                        return;
                    }
                }
                m_pEditingWP = wp;
                m_pNumberEdit->setMisc(addr);
                m_pNumberEdit->setText(strTxt.trimmed());
                m_pNumberEdit->show();
                m_pNumberEdit->setFocus();
                m_pNumberEdit->selectAll();
            }
        }
    }
}

void CWatchpointView::wheelEvent(QWheelEvent *event)
{
    QPoint degrees = event->angleDelta() / 8;

    if(degrees.y() == 0) return;

    if(degrees.y() > 0) {
        if (m_nStartIndex) {
            m_nStartIndex--;
        }
    } else {
        if (m_nStartIndex < (m_pWatchpointTable.count() - numRowsVisible())) {
            m_nStartIndex++;
        }
    }

    m_pScrollBar->setValue(m_nStartIndex);

    repaint();
}
