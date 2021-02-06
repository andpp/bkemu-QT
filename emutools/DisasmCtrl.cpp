#include "DisasmCtrl.h"
#include <QKeyEvent>
#include <QPaintEvent>

CDisasmCtrl::CDisasmCtrl() : QWidget()
  , m_pDebugger(nullptr)
{
    setMinimumSize(540, 480);
}

void CDisasmCtrl::AttachDebugger(CDebugger *pDebugger)
{
    m_pDebugger = pDebugger;
}

void CDisasmCtrl::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {


    }
}

void CDisasmCtrl::paintEvent(QPaintEvent* event)
{
    QPainter  painter(this);
    int nIndex;

    if(!m_pDebugger)
        return;

    int lineHeight = QFontMetrics(painter.font()).height();  // font height


    for(nIndex=0; nIndex < height()/lineHeight; nIndex++)
      m_pDebugger->DrawDebuggerLine(nIndex, painter);
}

