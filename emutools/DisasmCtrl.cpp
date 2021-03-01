#include "DisasmCtrl.h"
#include <QKeyEvent>
#include <QPaintEvent>
#include <QMouseEvent>

CDisasmCtrl::CDisasmCtrl() : QWidget()
  , m_pDebugger(nullptr)
  , m_nlineHeight(1)
  , m_Font("Monospace")
{
    setMinimumSize(540, 480);
    m_Font.setStyleHint(QFont::TypeWriter);
//    m_Font.setPointSize(11);
    m_Font.setPixelSize(14);
}

void CDisasmCtrl::AttachDebugger(CDebugger *pDebugger)
{
    m_pDebugger = pDebugger;
}

void CDisasmCtrl::keyPressEvent(QKeyEvent *event)
{

}

void CDisasmCtrl::paintEvent(QPaintEvent* event)
{
    QPainter  painter(this);
    uint nIndex;
    (void)event;
    uint16_t pc;
    int pcLine = -1;

    if(!m_pDebugger)
        return;

    painter.setFont(m_Font);
    m_nlineHeight = QFontMetrics(painter.font()).height();  // font height

    for(nIndex=0; nIndex < height()/m_nlineHeight; nIndex++) {
      if(m_pDebugger->DrawDebuggerLine(nIndex, painter)) {
          pcLine = nIndex;
      }
    }

    if(pcLine >= 0) {
        painter.setPen(RGB(0x66, 0, 0));
        pc = m_pDebugger->GetRegister(7);
        uint16_t nextAddr = m_pDebugger->CalcNextAddr(pc);
        uint firstAddr = m_pDebugger->GetCursorAddress();
        uint lastAddr = m_pDebugger->GetBottomAddress();
        if( nextAddr < firstAddr ) {
            // Draw UP
            painter.drawLine(DBG_LINE_NEXTLINE_POS, m_nlineHeight * pcLine, DBG_LINE_NEXTLINE_POS, 0);
        } else if (nextAddr > lastAddr) {
            // Draw Down
            painter.drawLine(DBG_LINE_NEXTLINE_POS, m_nlineHeight * (pcLine + 1), DBG_LINE_NEXTLINE_POS, height());
        } else {
            uint nextLine = 0;
            uint offset;
            while (m_pDebugger->GetLineAddress(nextLine) < nextAddr)
                nextLine++;
            if (nextLine > pcLine) {
                offset = 1;
            } else {
                offset = 0;
            }
            painter.drawLine(DBG_LINE_NEXTLINE_POS, m_nlineHeight * (pcLine + offset),             DBG_LINE_NEXTLINE_POS, m_nlineHeight * nextLine + m_nlineHeight/2 - 1);
            painter.drawLine(DBG_LINE_NEXTLINE_POS, m_nlineHeight * nextLine + m_nlineHeight/2 -1, DBG_LINE_ADR_START-2,  m_nlineHeight * nextLine + m_nlineHeight/2 - 1);
        }
    }

}

void CDisasmCtrl::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        emit HideAddrEdit();
        QPoint m_lastPos = event->pos();
        if(m_lastPos.x() <= DBG_LINE_ADR_START ) {
            int ln = m_lastPos.y() / m_nlineHeight;
            emit DisasmCheckBp(ln);
        }
    }
}

void CDisasmCtrl::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        QPoint m_lastPos = event->pos();
        if(m_lastPos.x() >= DBG_LINE_ADR_START && m_lastPos.x() <= DBG_LINE_INS_START) {
            int ln = m_lastPos.y() / m_nlineHeight;
            emit ShowAddrEdit(m_lastPos);
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
