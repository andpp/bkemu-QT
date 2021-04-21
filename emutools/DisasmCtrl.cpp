#include "DisasmCtrl.h"
#include <QKeyEvent>
#include <QPaintEvent>
#include <QMouseEvent>


CDisasmCtrl::CDisasmCtrl() : QWidget()
  , m_pDebugger(nullptr)
  , m_nlineHeight(1)
  , m_Font("Monospace")
{
    setMinimumSize(540, 280);
    m_Font.setStyleHint(QFont::TypeWriter);
//    m_Font.setPointSize(11);
    m_Font.setPixelSize(14);
    m_LineLayout.RecalculatePositions(true);
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
    m_nlineHeight = QFontMetrics(painter.font()).height();  // font height

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
}

void CDisasmCtrl::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        emit HideAddrEdit();
        emit HideLabelEdit();
        QPoint m_lastPos = event->pos();
        if(m_lastPos.x() <= m_LineLayout.DBG_LINE_ADR_START ) {
            int ln = m_lastPos.y() / m_nlineHeight;
            emit DisasmCheckBp(ln);
        }
    }
}

void CDisasmCtrl::mouseDoubleClickEvent(QMouseEvent *event)
{
    QPoint m_lastPos = event->pos();
    if (event->button() == Qt::MouseButton::LeftButton) {
        if(m_lastPos.x() >= m_LineLayout.DBG_LINE_ADR_START && m_lastPos.x() <= m_LineLayout.DBG_LINE_LBL_START) {
            emit ShowAddrEdit();
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
