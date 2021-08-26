#include "StackView.h"
#include "Debugger.h"

enum : int
{
    MEMCOLOR_TITLE = 0,
    MEMCOLOR_LEFT_VAL,
    MEMCOLOR_RIGHT_VAL,
    MEMCOLOR_LEFT_CHAR,
    MEMCOLOR_RIGHT_CHAR,
    MEMCOLOR_NUM_COLS    // количество цветов
};
const COLORREF g_crMemColorHighLighting[] =
{
    RGB(0, 0, 0),       // MEMCOLOR_TITLE
    RGB(0xff, 0x66, 0), // MEMCOLOR_LEFT_VAL
    RGB(0, 0x66, 0xcc), // MEMCOLOR_RIGHT_VAL
    RGB(0x66, 0, 0xcc), // MEMCOLOR_LEFT_CHAR
    RGB(0x60, 0x66, 0xff), // MEMCOLOR_RIGHT_CHAR
};

CStackView::CStackView(QWidget *pParent)
    : QDockWidget(pParent)
    , m_pDebugger(nullptr)
    , m_Font("Monospace")
{
    QFontMetricsF fm(m_Font);
    m_nlineHeight = fm.height();  // font height

    m_nOctWidth =  int(fm.horizontalAdvance("0000000  ") + 1);
    m_nASCIIWidth = int(fm.horizontalAdvance("01234567 ") + 1)*2;

    m_nAddrStart = 5;
    m_nDumpStart = m_nAddrStart + m_nOctWidth;
    m_nASCIIStart = m_nDumpStart + 8 * m_nOctWidth + 8;
    setMinimumSize(m_nASCIIStart + m_nASCIIWidth, m_nlineHeight * 8);

}

void CStackView::paintEvent(QPaintEvent* event)
{
    if(!m_pDebugger)
        return;

    QPainter  pnt(this);
    pnt.setFont(m_Font);
    QFontMetrics fm = pnt.fontMetrics();
    (void)event;

    CString strTxt;
    CString strTxtFirst;
    CString strData;

    int pos_y = m_nlineHeight;

    uint16_t nSP = m_pDebugger->GetRegister(CCPU::REGISTER::SP);
    int dumpAddrOffset = -3;
    int nLine = 0;

    while(pos_y < height()) {
        strTxt = ::WordToOctString(nSP);
        pnt.setPen(g_crMemColorHighLighting[MEMCOLOR_TITLE]);
        pnt.drawText(m_nAddrStart, pos_y, strTxt);
        strData = "";
        strTxt = "";

        pnt.setPen(g_crMemColorHighLighting[MEMCOLOR_LEFT_VAL]);
        uint16_t val = m_pDebugger->GetDebugMemDumpWord(nSP + (nLine + dumpAddrOffset) * 2);
        ::WordToOctString(val, strData);
        pnt.drawText(m_nDumpStart, pos_y, strData);
        pos_y += m_nlineHeight;
        nLine++;
    }
}

void CStackView::mousePressEvent(QMouseEvent *event)
{

}

void CStackView::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void CStackView::wheelEvent(QWheelEvent *event)
{

}
