//#include <QVBoxLayout>
#include <QVBoxLayout>
//#include <QToolBar>
#include <QResizeEvent>


#include "MemDumpDlg.h"
#include "Debugger.h"
#include "MainWindow.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// массив цветов для подсветки синтаксиса
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
CMemDumpDlg::CMemDumpDlg(QWidget *parent) : QWidget(parent)
  , m_pDebugger(nullptr)
  , m_nBase(10)
  , m_Font("Monospace")
  , m_nDumpAddress(0200)
{
    m_pNumberEdit = new CNumberEdit(8, this);
    m_pNumberEdit->hide();
    m_Font.setStyleHint(QFont::TypeWriter);
    m_pNumberEdit->setFont(m_Font);

//    m_nTimer.setSingleShot(true);
//    connect(&m_nTimer, &QTimer::timeout, this, &CRegDumpCPUCtrl::changeBase);
    QObject::connect(m_pNumberEdit, &CNumberEdit::AddressUpdated, this, &CMemDumpDlg::onEditFinished);

    QPainter pnt(this);
    pnt.setFont(m_Font);
    QFontMetricsF fm = pnt.fontMetrics();
    m_nlineHeight = fm.height();  // font height

    m_nOctWidth =  int(fm.horizontalAdvance("0000000  ") + 1);
    m_nASCIIWidth = int(fm.horizontalAdvance("01234567 ") + 1)*2;

    m_nAddrStart = 5;
    m_nDumpStart = m_nAddrStart + m_nOctWidth;
    m_nASCIIStart = m_nDumpStart + 8 * m_nOctWidth + 8;
    setMinimumSize(m_nASCIIStart + m_nASCIIWidth, m_nlineHeight * 8);
}

CMemDumpDlg::~CMemDumpDlg()
{
}

//void CMemDumpDlg::resizeEvent(QResizeEvent *event)
//{
//    QSize size = event->size();
//    size.setHeight(size.height() - 10);
//}

void CMemDumpDlg::AttachDebugger(CDebugger *pDebugger)
{
    m_pDebugger = pDebugger;
}

void CMemDumpDlg::paintEvent(QPaintEvent* event)
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
    int dumpAddrOffset = 0;

    while(pos_y < height()) {
      strTxt = ::WordToOctString(m_nDumpAddress + dumpAddrOffset);
      pnt.setPen(g_crMemColorHighLighting[MEMCOLOR_TITLE]);
      pnt.drawText(m_nAddrStart, pos_y, strTxt);
      strData = "";
      strTxt = "";

      pnt.setPen(g_crMemColorHighLighting[MEMCOLOR_LEFT_VAL]);
      for(int i=0; i<8; i++) {
          uint16_t val = m_pDebugger->GetDebugMemDumpWord(m_nDumpAddress + dumpAddrOffset + i*2);
          ::WordToOctString(val, strData);
          pnt.drawText(m_nDumpStart + m_nOctWidth*i, pos_y, strData);
          strTxt += GetMemDumpByteAsANSI(LOBYTE(val));
          strTxt += GetMemDumpByteAsANSI(HIBYTE(val));
          if(i == 3) {
              strTxtFirst = strTxt;
              strTxt = "";
              pnt.setPen(g_crMemColorHighLighting[MEMCOLOR_RIGHT_VAL]);
          }
      }
      pnt.setPen(g_crMemColorHighLighting[MEMCOLOR_LEFT_CHAR]);
      pnt.drawText(m_nASCIIStart, pos_y, strTxtFirst);
      pnt.setPen(g_crMemColorHighLighting[MEMCOLOR_RIGHT_CHAR]);
      pnt.drawText(m_nASCIIStart + m_nASCIIWidth/2, pos_y, strTxt);
      pos_y += m_nlineHeight;
      dumpAddrOffset += 16;

    }

}

void CMemDumpDlg::keyPressEvent(QKeyEvent *event)
{

}

void CMemDumpDlg::mousePressEvent(QMouseEvent *event)
{

}

void CMemDumpDlg::mouseDoubleClickEvent(QMouseEvent *event)
{

}

void CMemDumpDlg::wheelEvent(QWheelEvent *event)
{
    QPoint degrees = event->angleDelta() / 8;

    if(degrees.y() == 0) return;

    if(degrees.y() > 0) {
        m_nDumpAddress -= 16;
    } else {
        m_nDumpAddress += 16;
    }

    repaint();

}

void CMemDumpDlg::onEditFinished()
{

}

QChar CMemDumpDlg::GetMemDumpByteAsANSI(uint8_t byte)
{
    QChar ansi;

    if (byte < ' ')
    {
        ansi = QChar(' ');
    }
    else if (byte >= 0x80)
    {
        ansi = koi8tbl[byte - 0x80];
    }
    else
    {
        ansi = QChar(byte);
    }

    return ansi;
}
