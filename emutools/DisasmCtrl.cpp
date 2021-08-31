#include "DisasmCtrl.h"
#include <QKeyEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QApplication>
#include "Board.h"


CDisasmCtrl::CDisasmCtrl() : QWidget()
  , m_pDebugger(nullptr)
  , m_Font("Monospace")
{
    setMinimumSize(540, 280);
    m_Font.setStyleHint(QFont::TypeWriter);
//    m_Font.setPointSize(11);
    m_Font.setPixelSize(14);
    m_LineLayout.RecalculatePositions(true);

    QFontMetricsF fm(m_Font);
    m_nlineHeight = fm.height();  // font height


    m_DblClickTimer.setSingleShot(true);
    connect(&m_DblClickTimer, &QTimer::timeout, this, &CDisasmCtrl::DblClickTimeout);
}

void CDisasmCtrl::AttachDebugger(CDebugger *pDebugger)
{
    m_pDebugger = pDebugger;
}

void CDisasmCtrl::keyPressEvent(QKeyEvent *event)
{
    event->setAccepted(false);
}

void CDisasmCtrl::paintEvent(QPaintEvent* event)
{
    QPainter  painter(this);
    int nIndex;
    (void)event;
    uint16_t pc;
    int pcLine = -1;

    if(!m_pDebugger)
        return;

    painter.setFont(m_Font);
//    m_nlineHeight = QFontMetrics(painter.font()).height();  // font height

    for(nIndex=0; nIndex < numRowsVisible(); nIndex++) {
      if(m_pDebugger->DrawDebuggerLine(nIndex, lineOffset, painter, m_LineLayout)) {
          pcLine = nIndex;
      }
    }

    if (!m_pDebugger->IsCPUBreaked())
        return;

    painter.setPen(RGB(0x66, 0, 0));
    pc = m_pDebugger->GetRegister(static_cast<CCPU::REGISTER>(7));
    uint16_t nextAddr = m_pDebugger->CalcNextAddr(pc);
    int nextLine = m_pDebugger->GetLineByAddress(nextAddr);

    if(pcLine >= 0) {
        // PC on screem
        if( nextLine == DBG_RES_BEFORE_TOP ) {
            // Draw up to top
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS, m_nlineHeight * pcLine, m_LineLayout.DBG_LINE_NEXTLINE_POS, 0);
        } else if (nextLine == DBG_RES_AFTER_BOTTOM) {
            // Draw down to bottom
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS, m_nlineHeight * (pcLine + 1), m_LineLayout.DBG_LINE_NEXTLINE_POS, height());
        } else {
            int offset = nextLine > pcLine ? 1 : 0;
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS, m_nlineHeight * (pcLine + offset),             m_LineLayout.DBG_LINE_NEXTLINE_POS, m_nlineHeight * nextLine + m_nlineHeight/2 - 1);
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS, m_nlineHeight * nextLine + m_nlineHeight/2 -1, m_LineLayout.DBG_LINE_ADR_START-2,  m_nlineHeight * nextLine + m_nlineHeight/2 - 1);
        }
    } else if(nextLine >= 0) {
        // PC out of screen
        if (pc < nextAddr) {
            // Draw down from top
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS, 0, m_LineLayout.DBG_LINE_NEXTLINE_POS, m_nlineHeight * nextLine + m_nlineHeight/2 -1);
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS, m_nlineHeight * nextLine + m_nlineHeight/2 -1, m_LineLayout.DBG_LINE_ADR_START-2,  m_nlineHeight * nextLine + m_nlineHeight/2 - 1);
        } else {
            // Draw from bottom
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS, height(), m_LineLayout.DBG_LINE_NEXTLINE_POS, m_nlineHeight * nextLine + m_nlineHeight/2 -1);
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS, m_nlineHeight * nextLine + m_nlineHeight/2 -1, m_LineLayout.DBG_LINE_ADR_START-2,  m_nlineHeight * nextLine + m_nlineHeight/2 - 1);
        }
    } else if(m_pDebugger->GetLineByAddress(pc) != nextLine) {
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS, 0, m_LineLayout.DBG_LINE_NEXTLINE_POS, height());
    }

#ifdef ENABLE_BACKTRACE
    painter.setPen(RGB(0xC0, 0xC0, 0xB0));
    nextAddr = m_pDebugger->GetBoard()->BTGetPrevPC();
    nextLine = m_pDebugger->GetLineByAddress(nextAddr);

    if(pcLine >= 0) {
        // PC on screem
        if( nextLine == DBG_RES_BEFORE_TOP ) {
            // Draw up to top
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS+2, m_nlineHeight * pcLine, m_LineLayout.DBG_LINE_NEXTLINE_POS+2, 0);
        } else if (nextLine == DBG_RES_AFTER_BOTTOM) {
            // Draw down to bottom
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS+2, m_nlineHeight * (pcLine + 1), m_LineLayout.DBG_LINE_NEXTLINE_POS+2, height());
        } else {
            int offset = nextLine > pcLine ? 1 : 0;
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS+2, m_nlineHeight * (pcLine + offset),             m_LineLayout.DBG_LINE_NEXTLINE_POS+2, m_nlineHeight * nextLine + m_nlineHeight/2 - 1);
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS+2, m_nlineHeight * nextLine + m_nlineHeight/2 -1, m_LineLayout.DBG_LINE_ADR_START-2,  m_nlineHeight * nextLine + m_nlineHeight/2 - 1);
        }
    } else if(nextLine >= 0) {
        // PC out of screen
        if (pc < nextAddr) {
            // Draw down from top
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS+2, 0, m_LineLayout.DBG_LINE_NEXTLINE_POS+2, m_nlineHeight * nextLine + m_nlineHeight/2 -1);
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS+2, m_nlineHeight * nextLine + m_nlineHeight/2 -1, m_LineLayout.DBG_LINE_ADR_START-2,  m_nlineHeight * nextLine + m_nlineHeight/2 - 1);
        } else {
            // Draw from bottom
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS+2, height(), m_LineLayout.DBG_LINE_NEXTLINE_POS+2, m_nlineHeight * nextLine + m_nlineHeight/2 -1);
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS+2, m_nlineHeight * nextLine + m_nlineHeight/2 -1, m_LineLayout.DBG_LINE_ADR_START-2,  m_nlineHeight * nextLine + m_nlineHeight/2 - 1);
        }
    } else if(m_pDebugger->GetLineByAddress(pc) != nextLine) {
            painter.drawLine(m_LineLayout.DBG_LINE_NEXTLINE_POS+2, 0, m_LineLayout.DBG_LINE_NEXTLINE_POS+2, height());
    }
#endif

}

void CDisasmCtrl::mousePressEvent(QMouseEvent *event)
{
    m_DblClickButtons = event->buttons();
    if ((m_DblClickButtons & Qt::MouseButton::LeftButton) ||
            (m_DblClickButtons & Qt::MouseButton::RightButton)) {
        emit HideAddrEdit();
        emit HideLabelEdit();
        m_bIsDblClick = false;
        m_DblClickPos = event->pos();
        volatile int i = QApplication::doubleClickInterval();
        m_DblClickTimer.start(i+10);
//        m_DblClickTimer.start(QApplication::doubleClickInterval()+10);

//        QPoint m_lastPos = event->pos();
//        if(m_lastPos.x() <= m_LineLayout.DBG_LINE_ADR_START ) {
//            int ln = m_lastPos.y() / m_nlineHeight;
//            emit DisasmCheckBp(ln, event->button() == Qt::MouseButton::RightButton);
//        }

    }
}

// Process for single mouse clich after timeout
void CDisasmCtrl::DblClickTimeout()
{
    if(m_bIsDblClick)
        return;

    if(m_DblClickPos.x() <= m_LineLayout.DBG_LINE_ADR_START ) {
        int ln = m_DblClickPos.y() / m_nlineHeight;
        emit DisasmCheckBp(ln, m_DblClickButtons & Qt::MouseButton::RightButton);
    }
}

void CDisasmCtrl::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_bIsDblClick = true;
    QPoint m_lastPos = event->pos();

    if (event->button() == Qt::MouseButton::LeftButton) {
        if(m_lastPos.x() >= m_LineLayout.DBG_LINE_ADR_START && m_lastPos.x() <= m_LineLayout.DBG_LINE_LBL_START) {
            emit ShowAddrEdit();
        } else if(m_lastPos.x() <= m_LineLayout.DBG_LINE_ADR_START ) {
            int ln = m_lastPos.y() / m_nlineHeight;
            emit DisasmDelBP(ln);
        }
    } else if (event->button() == Qt::MouseButton::RightButton) {
        if(m_lastPos.x() >= m_LineLayout.DBG_LINE_ADR_START && m_lastPos.x() <= m_LineLayout.DBG_LINE_INS_START) {
            int ln = m_lastPos.y() / m_nlineHeight;
            emit ShowLabelEdit(ln);
        }
    }
}


void CDisasmCtrl::wheelEvent(QWheelEvent *event)
{
    QPoint degrees = event->angleDelta() / 8;

    if(degrees.y() == 0) return;

    if(degrees.y() > 0) {
        emit DisasmStepUp();
    } else {
        emit DisasmStepDn();
    }
}
