#include "RegDumpCPUCtrl.h"
#include <QKeyEvent>
#include <QPaintEvent>
#include <QMouseEvent>
#include <QApplication>

#define RGB(r,g,b) ( ((DWORD)(BYTE)r)|((DWORD)((BYTE)g)<<8)|((DWORD)((BYTE)b)<<16) )
#define GetRValue(RGBColor) (BYTE) (RGBColor)
#define GetGValue(RGBColor) (BYTE) (((DWORD)RGBColor) >> 8)
#define GetBValue(RGBColor) (BYTE) (((DWORD)RGBColor) >> 16)

// массив цветов для подсветки синтаксиса
enum : int
{
    RLCOLOR_REG = 0,
    RLCOLOR_VAL_OCT,
    RLCOLOR_VAL_UINT,
    RLCOLOR_VAL_INT,
    RLCOLOR_VAL_HEX,
    RLCOLOR_NUM_COLS    // количество цветов
};
const COLORREF g_crRegColorHighLighting[] =
{
    RGB(0, 0, 0), // RLCOLOR_REG
    RGB(0xff, 0x66, 0), // RLCOLOR_VALOCT
    RGB(0, 0x66, 0xcc), // RLCOLOR_VALUINT
    RGB(0x66, 0, 0xcc), // RLCOLOR_VALINT
    RGB(0x60, 0x66, 0xff), // RLCOLOR_VALHEX
};

CRegDumpCPUCtrl::CRegDumpCPUCtrl(uint regID, CString &regName, uint regNameWidth, QWidget *parent) : QWidget(parent)
  , m_pDebugger(nullptr)
  , m_nValue(0)
  , m_nRegCode(regID)
  , m_nRegType(REG_TYPE_RON)
  , m_bReadOnly(false)
  , m_bIsTextValue(false)
  , m_nBase(10)
  , m_Font("Monospace")
{
    m_sRegName = regName;
    m_pNumberEdit = new CNumberEdit(8, this);
    m_pNumberEdit->hide();
    m_Font.setStyleHint(QFont::TypeWriter);
    m_pNumberEdit->setFont(m_Font);

    m_nTimer.setSingleShot(true);
    connect(&m_nTimer, &QTimer::timeout, this, &CRegDumpCPUCtrl::changeBase);
    QObject::connect(m_pNumberEdit, &CNumberEdit::AddressUpdated, this, &CRegDumpCPUCtrl::onEditFinished);

    QPainter pnt(this);
    pnt.setFont(m_Font);
    QFontMetricsF fm = pnt.fontMetrics();

    if(regNameWidth) {
        m_nNameWidth = regNameWidth;
    } else {
        m_nNameWidth =  int(fm.horizontalAdvance(regName + " ") + 1);
    }
    m_nOctWidth =  int(fm.horizontalAdvance("0000000  ") + 1);
    m_nMiscWidth = int(fm.horizontalAdvance("-32767  ") + 1);

    m_nNameStart = 5;
    m_nOctStart = m_nNameStart + m_nNameWidth;
    m_nMiscStart = m_nOctStart + m_nOctWidth;
    setMinimumSize(m_nMiscStart + m_nMiscWidth + 10, 24);


}

void CRegDumpCPUCtrl::AttachDebugger(CDebugger *pDebugger)
{
    m_pDebugger = pDebugger;
}

void CRegDumpCPUCtrl::onEditFinished()
{
    m_nValue = m_pNumberEdit->getValue();
    if(m_pDebugger) {
        switch(m_nRegType) {
            case REG_TYPE_RON:
                m_pDebugger->OnDebugModify_Regs(m_nRegCode, m_nValue);
                break;
            case REG_TYPE_SYS:
                m_pDebugger->OnDebugModify_Ports(m_nRegCode, m_nValue);
                break;
            case REG_TYPE_ALTPRO:
                break;
        }
    }
    repaint();
}

void CRegDumpCPUCtrl::keyPressEvent(QKeyEvent *event)
{
    switch(event->key()) {


    }
}


void CRegDumpCPUCtrl::paintEvent(QPaintEvent* event)
{
    QPainter  pnt(this);
    pnt.setFont(m_Font);
    QFontMetrics fm = pnt.fontMetrics();
    (void)event;

    CString strTxt;

//    m_nlineHeight = QFontMetrics(pnt.font()).height();  // font height
    m_nlineHeight = fm.height();  // font height

    pnt.setPen(g_crRegColorHighLighting[RLCOLOR_REG]);
    pnt.drawText(5, m_nlineHeight, m_sRegName.toLocal8Bit().data());
    pnt.setPen(g_crRegColorHighLighting[RLCOLOR_VAL_OCT]);
    if (m_bIsTextValue) {
        pnt.drawText(m_nOctStart, m_nlineHeight, m_sTextValue.toLocal8Bit().data());
    } else {
        ::WordToOctString(m_nValue, strTxt);
        pnt.drawText(m_nOctStart, m_nlineHeight, strTxt);
    }


    switch(m_nBase) {
        case 10:
            pnt.setPen(g_crRegColorHighLighting[RLCOLOR_VAL_UINT]);
            strTxt = QStringLiteral("%1").arg(m_nValue, 5, 10, QLatin1Char(' ')).toUpper();
            break;
        case -10:
            pnt.setPen(g_crRegColorHighLighting[RLCOLOR_VAL_INT]);
            strTxt = QStringLiteral("%1").arg((short)m_nValue, 6, 10, QLatin1Char(' ')).toUpper();
            break;
        case 16:
        default:
            pnt.setPen(g_crRegColorHighLighting[RLCOLOR_VAL_HEX]);
            strTxt = " " + QStringLiteral("%1").arg(m_nValue, 4, 16, QLatin1Char('0')).toUpper();
            break;
    }

    int width = fm.horizontalAdvance(strTxt);
    pnt.drawText(m_nMiscStart + m_nMiscWidth - width, m_nlineHeight, strTxt);

}

void CRegDumpCPUCtrl::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_pNumberEdit->hide();
        QPoint mPos = event->pos();
        if (mPos.x() >= m_nMiscStart) {
            m_nTimer.start(QApplication::doubleClickInterval()+10);
        }
    }
}

void CRegDumpCPUCtrl::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        m_nTimer.stop();
        if (m_bReadOnly)
            return;
        QPoint mPos = event->pos();
        if(mPos.x() >= m_nOctStart && mPos.x() < m_nMiscStart) {
            CString strTxt;
            ::WordToOctString(m_nValue, strTxt);
            m_pNumberEdit->setBase(8);
            m_pNumberEdit->setSize(m_nOctWidth+4, m_pNumberEdit->height());
            m_pNumberEdit->move(m_nOctStart-3, 3);
            m_pNumberEdit->setText(strTxt);
            m_pNumberEdit->show();
            m_pNumberEdit->setFocus();
        } else
        if(mPos.x() >= m_nMiscStart) {
            CString strTxt;
            switch (m_nBase) {
                case 10:
                    strTxt = QStringLiteral("%1").arg(m_nValue, 5, 10);
                    strTxt.Trim();
                    break;
                case -10:
                    strTxt = QStringLiteral("%1").arg((short)m_nValue, 6, 10);
                    strTxt.Trim();
                    break;
                case 16:
                default:
                    strTxt = QStringLiteral("%1").arg(m_nValue, 4, 16, QLatin1Char('0')).toUpper();
                break;
            }
            m_pNumberEdit->setBase(m_nBase);
            m_pNumberEdit->setSize(m_nMiscWidth+4, m_pNumberEdit->height());
            m_pNumberEdit->move(m_nMiscStart + ((m_nBase == 10) ? 0 : 1), 3);
            m_pNumberEdit->setText(strTxt);
            m_pNumberEdit->show();
            m_pNumberEdit->setFocus();
        }
    }
}
