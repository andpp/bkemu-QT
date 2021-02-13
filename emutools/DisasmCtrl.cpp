#include "DisasmCtrl.h"
#include <QKeyEvent>
#include <QPaintEvent>
#include <QMouseEvent>

CDisasmCtrl::CDisasmCtrl() : QWidget()
  , m_pDebugger(nullptr)
  , m_nlineHeight(1)
{
    setMinimumSize(540, 480);
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

    if(!m_pDebugger)
        return;

    m_nlineHeight = QFontMetrics(painter.font()).height();  // font height


    for(nIndex=0; nIndex < height()/m_nlineHeight; nIndex++)
      m_pDebugger->DrawDebuggerLine(nIndex, painter);
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
