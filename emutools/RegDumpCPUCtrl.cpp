#include "RegDumpCPUCtrl.h"
#include <QKeyEvent>
#include <QPaintEvent>
#include <QMouseEvent>

CRegDumpCPUCtrl::CRegDumpCPUCtrl(uint regID, CString &regName, QWidget *parent) : QWidget(parent)
  , m_pDebugger(nullptr)
  , m_nValue(0)
  , m_nRegCode(regID)
  , m_bReadOnly(false)
  , m_bIsTextValue(false)
  , m_nBase(8)
{
    m_sRegName = regName;
    m_pNumberEdit = new CNumberEdit(8, this);
    m_pNumberEdit->hide();
    m_pNumberEdit->move(40-9,0);
    setMinimumSize(150, 24);
}

void CRegDumpCPUCtrl::AttachDebugger(CDebugger *pDebugger)
{
    m_pDebugger = pDebugger;
}

void CRegDumpCPUCtrl::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {


    }
}

void CRegDumpCPUCtrl::paintEvent(QPaintEvent* event)
{
    QPainter  pnt(this);
    (void)event;

    CString strTxt;

    m_nlineHeight = QFontMetrics(pnt.font()).height();  // font height

    pnt.drawText(5, m_nlineHeight, m_sRegName.toLocal8Bit().data());
    if (m_bIsTextValue) {
        pnt.drawText(40, m_nlineHeight, m_sTextValue.toLocal8Bit().data());
    } else {
        ::WordToOctString(m_nValue, strTxt);
        pnt.drawText(40, m_nlineHeight, strTxt);
    }
}

void CRegDumpCPUCtrl::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pNumberEdit->hide();
        QPoint m_lastPos = event->pos();
    }
}

void CRegDumpCPUCtrl::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        QPoint m_lastPos = event->pos();
//        if(m_lastPos.x() >= DBG_LINE_ADR_START && m_lastPos.x() <= DBG_LINE_INS_START) {
//            int ln = m_lastPos.y() / m_nlineHeight;
            CString strTxt;
            ::WordToOctString(m_nValue, strTxt);
            m_pNumberEdit->setText(strTxt);
            m_pNumberEdit->show();
            m_pNumberEdit->setFocus();
//        }
    }
}
