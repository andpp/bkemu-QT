//#include <QVBoxLayout>
#include <QVBoxLayout>
//#include <QToolBar>
#include <QResizeEvent>


#include "MemDumpDlg.h"
#include "Debugger.h"
#include "MainWindow.h"
#include "Config.h"


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
    MEMCOLOR_LEFT_VAL_HL,
    MEMCOLOR_RIGHT_VAL_HL,
    MEMCOLOR_LEFT_CHAR_HL,
    MEMCOLOR_RIGHT_CHAR_HL,
    MEMCOLOR_NUM_COLS    // количество цветов
};
const COLORREF g_crMemColorHighLighting[] =
{
    RGB(0, 0, 0),       // MEMCOLOR_TITLE
    RGB(0xff, 0x66, 0), // MEMCOLOR_LEFT_VAL
    RGB(0, 0x66, 0xcc), // MEMCOLOR_RIGHT_VAL
    RGB(0x66, 0, 0xcc), // MEMCOLOR_LEFT_CHAR
    RGB(0x60, 0x66, 0xff), // MEMCOLOR_RIGHT_CHAR
    RGB(0xFF, 0, 0),       // MEMCOLOR_TITLE
    RGB(0xff, 0xFF, 0), // MEMCOLOR_LEFT_VAL
    RGB(0, 0xFF, 0xcc), // MEMCOLOR_RIGHT_VAL
    RGB(0xFF, 0, 0xcc), // MEMCOLOR_LEFT_CHAR
    RGB(0x60, 0xFF, 0xff), // MEMCOLOR_RIGHT_CHAR
};
CMemDumpDlg::CMemDumpDlg(QWidget *parent) : QWidget(parent)
  , m_pDebugger(nullptr)
  , m_nBase(8)
  , m_Font(g_FontMonospace)
  , m_nDisplayMode(DUMP_DISPLAY_MODE::DD_WORD_VIEW)
  , m_nDumpAddress(g_Config.m_nDumpAddr)
{
    m_pNumberEdit = new CNumberEdit(8, this);
    m_pNumberEdit->hide();
    m_Font.setStyleHint(QFont::TypeWriter);
    m_pNumberEdit->setFont(m_Font);

    m_nTimer.setSingleShot(true);
    connect(&m_nTimer, &QTimer::timeout, this, &CMemDumpDlg::changeDisplayMode);
    QObject::connect(m_pNumberEdit, &CNumberEdit::AddressUpdated, this, &CMemDumpDlg::onEditFinished);

    QFontMetricsF fm(m_Font);
    m_nlineHeight = fm.height();  // font height

    m_nOctWidth =  int(fm.horizontalAdvance("0000000") + 4);
    m_nASCIIWidth = int(fm.horizontalAdvance("01234567 ") + 1)*2;

    m_nAddrStart = 5;
    m_nDumpStart = m_nAddrStart + m_nOctWidth;
    m_nASCIIStart = m_nDumpStart + 8 * m_nOctWidth + 8;
    setMinimumSize(m_nASCIIStart + m_nASCIIWidth, m_nlineHeight * 8);
    memset(m_Mem, 0 , sizeof(m_Mem));
}

CMemDumpDlg::~CMemDumpDlg()
{
    g_Config.m_nDumpAddr = m_nDumpAddress;
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
    (void)event;

    CString strTxt;
    CString strTxtFirst;
    CString strData;

    int pos_y = m_nlineHeight;
    int dumpAddrOffset = 0;
    int colorOffset;

    while(pos_y < height()) {
      strTxt = ::WordToOctString(m_nDumpAddress + dumpAddrOffset);
      pnt.setPen(g_crMemColorHighLighting[MEMCOLOR_TITLE]);
      pnt.drawText(m_nAddrStart, pos_y, strTxt);
      strData = "";
      strTxt = "";

      for(int i=0; i<8; i++) {
          uint16_t addr = m_nDumpAddress + dumpAddrOffset + i*2;
          uint16_t val = m_pDebugger->GetDebugMemDumpWord(addr);
          bool changed = *(uint16_t *)(m_Mem + addr) == val;
          colorOffset = 0;
          if(changed) {
            *(uint16_t *)(m_Mem + addr) = val; // Update new value
            colorOffset = 4;
          }
          if( i <= 3) {
              pnt.setPen(g_crMemColorHighLighting[MEMCOLOR_LEFT_VAL + colorOffset]);
          } else {
              pnt.setPen(g_crMemColorHighLighting[MEMCOLOR_RIGHT_VAL + colorOffset]);
          }

          if (m_nDisplayMode == DUMP_DISPLAY_MODE::DD_WORD_VIEW) {
              ::WordToOctString(val, strData);
              pnt.drawText(m_nDumpStart + m_nOctWidth*i, pos_y, strData);
          } else {
              ::ByteToOctString(LOBYTE(val), strData);
              pnt.drawText(m_nDumpStart + m_nOctWidth*i, pos_y, strData);
              ::ByteToOctString(HIBYTE(val), strData);
              pnt.drawText(m_nDumpStart + m_nOctWidth*i + m_nOctWidth/2, pos_y, strData);
          }
          strTxt += GetMemDumpByteAsANSI(LOBYTE(val));
          strTxt += GetMemDumpByteAsANSI(HIBYTE(val));
          if(i == 3) {
              strTxtFirst = strTxt;
              strTxt = "";
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
    auto key = event->key();

    switch (key) {
        case Qt::Key_PageDown:
                m_nDumpAddress += 16 * (height()/m_nlineHeight);
                break;
        case Qt::Key_PageUp:
                m_nDumpAddress -= 16 * (height()/m_nlineHeight);
                break;
        case Qt::Key_Down:
                m_nDumpAddress += 16;
                break;
        case Qt::Key_Up:
                m_nDumpAddress -= 16;
                break;
        default:
             return;
    }

    repaint();
    event->accept();
}

void CMemDumpDlg::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::MouseButton::LeftButton) {
        setFocus();
    } else if (event->button() == Qt::MouseButton::RightButton) {
        QPoint mPos = event->pos();
        if (mPos.x() >= m_nDumpStart && mPos.x() <= m_nASCIIStart) {
//            m_nTimer.start(QApplication::doubleClickInterval()+10);
            changeDisplayMode();
        }
    }
    m_pNumberEdit->hide();

}

void CMemDumpDlg::mouseDoubleClickEvent(QMouseEvent *event)
{
    m_nTimer.stop();
    if (event->button() == Qt::MouseButton::LeftButton) {
//        if (m_bReadOnly)
//            return;
        QPoint mPos = event->pos();
        if(mPos.x() <= m_nDumpStart) {
            CString strTxt;
            m_nEditingMode = EDITING_MODE::EM_ADDRESS;
            int line = mPos.y()/m_nlineHeight;
            m_nEditedAddress = line;
            ::WordToOctString(m_nDumpAddress + line * 16, strTxt);
            m_pNumberEdit->setBase(8);
            m_pNumberEdit->setWidth(m_nOctWidth+4);
            m_pNumberEdit->move(m_nAddrStart-7, line * m_nlineHeight + 4);
            m_pNumberEdit->setText(strTxt);
            m_pNumberEdit->show();
            m_pNumberEdit->setFocus();
        } else
        if(mPos.x() <= m_nASCIIStart) {
            m_nEditingMode = EDITING_MODE::EM_DATA;
            CString strTxt;
            int line = mPos.y()/m_nlineHeight;
            int offset = (mPos.x()-m_nDumpStart) / m_nOctWidth;
            m_nEditedAddress = m_nDumpAddress + line * 16 + offset * 2;
            if( m_nDisplayMode == DUMP_DISPLAY_MODE::DD_WORD_VIEW) {
                strTxt = QStringLiteral("%1").arg(m_pDebugger->GetDebugMemDumpWord(m_nEditedAddress), 7, 8);
                m_pNumberEdit->setWidth(m_nOctWidth + 4);
                m_pNumberEdit->move( m_nDumpStart + m_nOctWidth *  offset - 7, m_nlineHeight * line + 4);
                m_pNumberEdit->setBase(m_nBase);
            } else {
                if ((mPos.x()-m_nDumpStart) % m_nOctWidth >= m_nOctWidth /2) {
                   m_nEditedAddress += 1;
                }

                strTxt = QStringLiteral("%1").arg(m_pDebugger->GetDebugMemDumpByte(m_nEditedAddress), 3, 8);
                m_pNumberEdit->setWidth(m_nOctWidth/2 + 4);
                m_pNumberEdit->move( m_nDumpStart + m_nOctWidth * offset + ((m_nEditedAddress & 1) ? m_nOctWidth/2 : 0)  - 7, m_nlineHeight * line + 4);
                m_pNumberEdit->setBase(-m_nBase);
            }
            m_pNumberEdit->setText(strTxt.trimmed());
            m_pNumberEdit->selectAll();
            m_pNumberEdit->show();
            m_pNumberEdit->setFocus();
        }
    }

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
    switch (m_nEditingMode) {
        case EDITING_MODE::EM_ADDRESS:
            {
                m_nDumpAddress = m_pNumberEdit->getValue() - m_nEditedAddress * 16;
                break;
            }
        case EDITING_MODE::EM_DATA:
            {
                if( m_nDisplayMode == DUMP_DISPLAY_MODE::DD_WORD_VIEW) {
                    m_pDebugger->GetBoard()->SetWord(m_nEditedAddress, m_pNumberEdit->getValue());
                } else {
                    m_pDebugger->GetBoard()->SetByte(m_nEditedAddress, m_pNumberEdit->getValue());
                }
                break;
            }
        case EDITING_MODE::EM_ASCII:
            break;

    }
    repaint();

}

QChar CMemDumpDlg::GetMemDumpByteAsANSI(uint8_t byte)
{
    QChar ansi;

    if (byte < ' ')
    {
        ansi = QChar('.');
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
