#include "pch.h"
#include "BKKbdButn.h"
//#include "BK.h"
#include "Config.h"
#include <QStyle>
#include <QMouseEvent>
#include <QCoreApplication>

#include "KeyDefinitions.h"

extern QObject           *g_pBKView;

CBKKbdButn::CBKKbdButn(UINT nID, QWidget *parent)
	: m_cx(0)
	, m_cy(0)
	, m_imgW(0)
	, m_imgH(0)
	, m_nIdx(-1)
    , m_nKbdIdx(-1)
	, m_nArraySize(-1)
	, m_bAR2Pressed(false)
	, m_bSUPressed(false)
	, m_bShiftPressed(false)
	, m_bRShiftPressed(false)
	, m_bZaglPressed(true)
    , m_bXlatMode(false)
	, m_bControlKeyPressed(false)
	, m_bRegularKeyPressed(false)
	, m_pBKKeyboardArray(nullptr)
    , m_cwndParent(parent)
{
	SetID(nID);

//	m_ImgScr.Create(m_imgW, m_imgH, m_Img.GetBPP());
}

void CBKKbdButn::SetID(UINT nID)
{
//	UINT nID_p = nID; // ид битмапа нажатой клавиатуры

	switch (nID)
	{
		case IDB_BITMAP_SOFT:
			m_pBKKeyboardArray = (BKKey *)m_ButnKbdKeys;
            m_Img.load(":kbd/kbdSoft");
            m_Img_p.load(":kbd/kbdSoftPressed");
//			nID_p = IDB_BITMAP_SOFT_P;  // Битмап нажатия для скрипучей клавы
			break;

        case IDB_BITMAP_PLEN:
            m_pBKKeyboardArray = (BKKey *)m_PlenKbdKeys;
            m_Img.load(":kbd/kbdPlen");
            m_Img_p.load(":kbd/kbdPlenPressed");
//			nID_p = IDB_BITMAP_PLEN_P;  // Битмап нажатия для силиконовой клавы
            break;

		default:
			return;
	}

//	if (!m_Img.IsNull())
//	{
//		m_Img.Destroy();
//	}

//	if (!m_Img_p.IsNull())
//	{
//		m_Img_p.Destroy();
//	}

//	m_Img.LoadFromResource(AfxGetInstanceHandle(), nID);
//	m_Img_p.LoadFromResource(AfxGetInstanceHandle(), nID_p);
    m_imgW = m_Img.width();
    m_imgH = m_Img.height();
	m_nArraySize = GetArraySize();
	m_nAR2Index = GetKeyIndexById(BKKeyType::ALT);
	m_nSUIndex = GetKeyIndexById(BKKeyType::CTRL);
	m_nLShiftIndex = GetKeyIndexById(BKKeyType::LSHIFT);
	m_nRShiftIndex = GetKeyIndexById(BKKeyType::RSHIFT);

    m_cx = m_imgW;
    m_cy = m_imgH;

    AdjustLayout();
}

CBKKbdButn::~CBKKbdButn()
{
	ClearObj();
}

void CBKKbdButn::ClearObj()
{
//	if (!m_ImgScr.IsNull())
//	{
//		m_ImgScr.Destroy();
//	}

//	if (!m_Img.IsNull())
//	{
//		m_Img.Destroy();
//	}

//	if (!m_Img_p.IsNull())
//	{
//		m_Img_p.Destroy();
//	}
}

//BEGIN_MESSAGE_MAP(CBKKbdButn, CWnd)
//	ON_WM_PAINT()
//	ON_WM_LBUTTONDOWN()
//	ON_WM_LBUTTONUP()
//	ON_WM_ERASEBKGND()

//	ON_MESSAGE(WM_VKBD_DN_CALLBACK, &CBKKbdButn::OnRealKeyDown)
//	ON_MESSAGE(WM_VKBD_UP_CALLBACK, &CBKKbdButn::OnRealKeyUp)
//	ON_WM_SIZE()
//END_MESSAGE_MAP()


void CBKKbdButn::paintEvent(QPaintEvent* event)
{
    QPainter  m_ImgScr(this);
    m_ImgScr.drawImage(0,0, m_Img);

	if (m_nIdx >= 0) // если нажата левая кнопка, и там где надо
	{
		// нужно отрисовать нажатую кнопку
        _FocusPressedkey(m_nIdx, m_ImgScr);
	}

    if (m_nKbdIdx >= 0) { // если нажата клавиша
        // нужно отрисовать нажатую кнопку
        _FocusPressedkey(m_nKbdIdx, m_ImgScr);

    }

	if (m_bAR2Pressed)
	{
        _FocusPressedkey(m_nAR2Index, m_ImgScr);
	}

	if (m_bSUPressed)
	{
        _FocusPressedkey(m_nSUIndex, m_ImgScr);
	}

	if (m_bShiftPressed)
	{
        _FocusPressedkey(m_nLShiftIndex, m_ImgScr);
	}

	if (m_bRShiftPressed)
	{
        _FocusPressedkey(m_nRShiftIndex, m_ImgScr);
	}

	// и затем рисуем смасштабированно из буфера.
//	m_ImgScr.Draw(hdc, 0, 0, m_cx, m_cy, 0, 0, m_imgW, m_imgH);
}

void CBKKbdButn::AdjustLayout()
{
    if (m_cwndParent == nullptr)
	{
		return;
	}

    int tbHeight = m_cwndParent->style()->pixelMetric(QStyle::PM_TitleBarHeight);
    m_cwndParent->setFixedSize(m_imgW, m_imgH+tbHeight-3);

//    CRect rcScreen(0, 0, m_imgW, m_imgH);
//	CRect rcNewScreen = rcScreen;
//	CRect rcClient;
//	m_cwndParent->GetClientRect(&rcClient);

//	if (rcScreen.Width() * rcClient.Height() > rcClient.Width() * rcScreen.Height())
//	{
//		rcNewScreen.right = rcClient.Width();
//		rcNewScreen.bottom = rcScreen.bottom * rcClient.Width() / rcScreen.right;
//	}
//	else
//	{
//		rcNewScreen.bottom = rcClient.Height();
//		rcNewScreen.right = rcScreen.right * rcClient.Height() / rcScreen.bottom;
//	}

//	if (rcNewScreen.Width() > m_imgW)
//	{
//		rcNewScreen.right = m_imgW;
//	}

//	if (rcNewScreen.Height() > m_imgH)
//	{
//		rcNewScreen.bottom = m_imgH;
//	}

//	int x_offs = (rcClient.Width() - rcNewScreen.Width()) / 2;
//	int y_offs = (rcClient.Height() - rcNewScreen.Height()) / 2;
//	rcClient.right = rcClient.left + rcNewScreen.Width();
//	rcClient.bottom = rcClient.top + rcNewScreen.Height();
//	rcClient.OffsetRect(x_offs, y_offs);
//    SetWindowPos(nullptr, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER);
}

void CBKKbdButn::_FocusPressedkey(int nIdx, QPainter & painter)
{
	// нужно отрисовать нажатую кнопку
    BKKey *pKey = &m_pBKKeyboardArray[nIdx];
	// затем накладываем нажатую кнопку
//	m_Img_p.BitBlt(m_ImgScr.GetDC(), pKey->x1, pKey->y1, pKey->x2 - pKey->x1, pKey->y2 - pKey->y1,
//	               pKey->x1, pKey->y1, SRCCOPY);
//	m_ImgScr.ReleaseDC();
    painter.drawImage(pKey->x1, pKey->y1, m_Img_p,  pKey->x1, pKey->y1, pKey->x2 - pKey->x1, pKey->y2 - pKey->y1);
}


//BOOL CBKKbdButn::OnEraseBkgnd(CDC *pDC)
//{
//	return TRUE; // CWnd::OnEraseBkgnd(pDC);
//}

//void CBKKbdButn::OnSize(UINT nType, int cx, int cy)
//{
//	CWnd::OnSize(nType, cx, cy);

//	if ((cy > 0) && (cx > 0))
//	{
//		m_cx = cx;
//		m_cy = cy;
//	}
//}

//BOOL CBKKbdButn::DestroyWindow()
//{
//	// TODO: добавьте специализированный код или вызов базового класса
//	ClearObj();
//	return CWnd::DestroyWindow();
//}

//BOOL CBKKbdButn::PreCreateWindow(CREATESTRUCT &cs)
//{
//	m_hwndParent = cs.hwndParent;
//	m_cwndParent = FromHandle(m_hwndParent);
//	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
//	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_SAVEBITS,
//	                                   ::LoadCursor(nullptr, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), nullptr);
//	return CWnd::PreCreateWindow(cs);
//}
// предполагается реакция на нажатие/отжатие реальных кнопок на реальной клавиатуре
// но что-то никак не понятно, где ловить нажатия и как превращать их в индекс кнопки
//LRESULT CBKKbdButn::OnRealKeyDown(WPARAM wParam, LPARAM lParam)
//{
//	return LRESULT();
//}

//LRESULT CBKKbdButn::OnRealKeyUp(WPARAM wParam, LPARAM lParam)
//{
//	return LRESULT();
//}

void CBKKbdButn::mousePressEvent(QMouseEvent* event)
{
	// с учётом масштабирования, надо преобразовать полученные координаты
	// в координаты рисунка.
    auto x = int(double(event->x()) * double(m_imgW) / double(m_cx) + 0.5);
    auto y = int(double(event->y()) * double(m_imgH) / double(m_cy) + 0.5);
	m_nIdx = GetKeyIndex(x, y);

	if (m_nIdx >= 0)
	{
		BKKey *pKey = &m_pBKKeyboardArray[m_nIdx];

		if (pKey->nType != BKKeyType::RESERVED)
		{
//			SetCapture();
//			LPARAM lParam = 0;
//			auto mw = theApp.GetMainWnd();

			switch (pKey->nType)
			{
				case BKKeyType::ALT:
//					mw->SendMessageToDescendants(!m_bAR2Pressed ? WM_VKBD_DOWN : WM_VKBD_UP, static_cast<WPARAM>(pKey->nType), lParam, FALSE, FALSE);
//					break;
                __attribute__((fallthrough));

				case BKKeyType::CTRL:
//					mw->SendMessageToDescendants(!m_bSUPressed ? WM_VKBD_DOWN : WM_VKBD_UP, static_cast<WPARAM>(pKey->nType), lParam, FALSE, FALSE);
//					break;
                __attribute__((fallthrough));

				case BKKeyType::LSHIFT:
//					mw->SendMessageToDescendants(!m_bShiftPressed ? WM_VKBD_DOWN : WM_VKBD_UP, static_cast<WPARAM>(pKey->nType), lParam, FALSE, FALSE);
//					break;
                __attribute__((fallthrough));

				case BKKeyType::RSHIFT:
//					mw->SendMessageToDescendants(!m_bRShiftPressed ? WM_VKBD_DOWN : WM_VKBD_UP, static_cast<WPARAM>(pKey->nType), lParam, FALSE, FALSE);
//					break;
                __attribute__((fallthrough));

				case BKKeyType::ZAGL:
//					mw->SendMessageToDescendants(WM_VKBD_DOWN, static_cast<WPARAM>(pKey->nType), lParam, FALSE, FALSE);
//					break;
                __attribute__((fallthrough));

				case BKKeyType::STR:
//					mw->SendMessageToDescendants(WM_VKBD_DOWN, static_cast<WPARAM>(pKey->nType), lParam, FALSE, FALSE);
//					break;
                __attribute__((fallthrough));

                case BKKeyType::STOP:
                    ControlKeysUp();
//					mw->SendMessageToDescendants(WM_VKBD_DOWN, static_cast<WPARAM>(pKey->nType), lParam, FALSE, FALSE);
//                  break;
                __attribute__((fallthrough));

                case BKKeyType::REGULAR:
				{
					m_bRegularKeyPressed = true;
                    uint8_t nKeyCode = TranslateScanCode(pKey->nKeyCode);
                    //uint8_t nInterrupt = (m_bAR2Pressed) ? INTERRUPT_274 : pKey->nInterrupt;
                    //uint8_t nUnique = pKey->nUniqueNum;
                    //QKeyEvent *ev = new QKeyEvent(QEvent::KeyPress, nUnique, Qt::NoModifier);
                    //QCoreApplication::postEvent(g_pBKView, ev);
                    BKKeyEvent *ev = new BKKeyEvent(nKeyCode, pKey, true);
                    QCoreApplication::postEvent(g_pBKView, ev);
//					lParam = (static_cast<UINT>(nUnique) << 16) | (static_cast<UINT>(nInterrupt) << 8) | static_cast<UINT>(nScanCode);
//					mw->SendMessageToDescendants(WM_VKBD_DOWN, static_cast<WPARAM>(pKey->nType), lParam, FALSE, FALSE);
				}
				break;

			}

			// если хоть одна из этих клавиш нажата - то флаг нажатой клавиши установлен
			m_bControlKeyPressed = m_bAR2Pressed || m_bSUPressed || m_bShiftPressed || m_bRShiftPressed;
		}
	}

//	Invalidate(FALSE);
//	CWnd::OnLButtonDown(nFlags, point);
    repaint();
}

void CBKKbdButn::mouseReleaseEvent(QMouseEvent* event)
{
    (void)event;

	if (m_nIdx >= 0)
	{
		BKKey *pKey = &m_pBKKeyboardArray[m_nIdx];

		if (pKey->nType != BKKeyType::RESERVED)
		{
//			ReleaseCapture();

			if (m_bRegularKeyPressed)
			{
				m_bRegularKeyPressed = false;
                uint8_t nKeyCode = TranslateScanCode(pKey->nKeyCode);
                //uint8_t nInt = (m_bAR2Pressed) ? INTERRUPT_274 : pKey->nInterrupt;
                //uint8_t nUnique = pKey->nUniqueNum;
                //QKeyEvent *ev = new QKeyEvent(QEvent::KeyRelease, nUnique, Qt::NoModifier);
                //QCoreApplication::postEvent(g_pBKView, ev);
                BKKeyEvent *ev = new BKKeyEvent(nKeyCode, pKey, false);
                QCoreApplication::postEvent(g_pBKView, ev);
//				LPARAM lParam = (static_cast<UINT>(nUnique) << 16) | (static_cast<UINT>(nInt) << 8) | static_cast<UINT>(nScanCode);
//				theApp.GetMainWnd()->SendMessageToDescendants(WM_VKBD_UP, static_cast<WPARAM>(pKey->nType), lParam, FALSE, FALSE);

				if (m_bControlKeyPressed)
				{
					ControlKeysUp();
				}
			}
			else if (pKey->nType == BKKeyType::STOP)
			{
//				theApp.GetMainWnd()->SendMessageToDescendants(WM_VKBD_UP, static_cast<WPARAM>(pKey->nType), 0, FALSE, FALSE);
			}
		}

		m_nIdx = -1;
	}

//	Invalidate(FALSE);
//	CWnd::OnLButtonUp(nFlags, point);
    repaint();
}

void CBKKbdButn::ControlKeysUp()
{
//	auto mw = theApp.GetMainWnd();

	if (m_bAR2Pressed)
	{
//		mw->SendMessageToDescendants(WM_VKBD_UP, static_cast<WPARAM>(BKKeyType::ALT), 0, FALSE, FALSE);
	}

	if (m_bSUPressed)
	{
//		mw->SendMessageToDescendants(WM_VKBD_UP, static_cast<WPARAM>(BKKeyType::CTRL), 0, FALSE, FALSE);
	}

	if (m_bShiftPressed)
	{
//		mw->SendMessageToDescendants(WM_VKBD_UP, static_cast<WPARAM>(BKKeyType::LSHIFT), 0, FALSE, FALSE);
	}

	if (m_bRShiftPressed)
	{
//		mw->SendMessageToDescendants(WM_VKBD_UP, static_cast<WPARAM>(BKKeyType::RSHIFT), 0, FALSE, FALSE);
	}

	m_bControlKeyPressed = false;
//	Invalidate(FALSE);
    repaint();
}

uint8_t CBKKbdButn::TranslateScanCode(uint8_t nScanCode)
{
#if 0
	if (m_bSUPressed)
	{
		if ((0100 <= nScanCode) && (nScanCode <= 0137))
		{
			return nScanCode & 037;
		}
	}

	if (GetShiftStatus())
	{
		if ((054 <= nScanCode) && (nScanCode <= 047))
		{
			return nScanCode + 020;
		}
		else if ((060 <= nScanCode) && (nScanCode <= 073))
		{
			return nScanCode - 020;
		}
	}

	if ((0100 <= nScanCode) && (nScanCode <= 0137))
	{
		// если режим строчных букв - то их в строчные
		if (m_bXlatMode)
		{
			if (!(m_bZaglPressed ^ !GetShiftStatus()))
			{
				nScanCode += 040;
			}
		}
		else
		{
			if (!(m_bZaglPressed ^ GetShiftStatus()))
			{
				nScanCode += 040;
			}
		}
	}
#endif
	return nScanCode;
}

int CBKKbdButn::GetKeyIndex(int x, int y)
{
	ASSERT(m_pBKKeyboardArray);

	for (int i = 0; i < m_nArraySize; ++i)
	{
		if ((m_pBKKeyboardArray[i].x1 <= x) && (x <= m_pBKKeyboardArray[i].x2)
		        && (m_pBKKeyboardArray[i].y1 <= y) && (y <= m_pBKKeyboardArray[i].y2)
		   )
		{
			return i;
		}
	}

	return -1;
}

int CBKKbdButn::GetKeyIndex(uint32_t scanCode)
{
    ASSERT(m_pBKKeyboardArray);

    for (int i = 0; i < m_nArraySize; ++i)
    {
        if (m_pBKKeyboardArray[i].nScanCode == scanCode)
        {
            return i;
        }
    }

    return -1;
}


int CBKKbdButn::GetKeyIndexById(BKKeyType nType)
{
	ASSERT(m_pBKKeyboardArray);

	for (int i = 0; i < m_nArraySize; ++i)
	{
		if (m_pBKKeyboardArray[i].nType == nType)
		{
			return i;
		}
	}

	return -1;
}

int CBKKbdButn::GetArraySize()
{
	ASSERT(m_pBKKeyboardArray);
	int i = -1;

	if (m_pBKKeyboardArray)
	{
		while (m_pBKKeyboardArray[++i].nType != BKKeyType::ENDARRAY)
		{
			if (i == INT_MAX)
			{
				return -1;
			}
		}
	}

	return i;
}

uint8_t CBKKbdButn::GetUniqueKeyNum(uint8_t nScancode)
{
	ASSERT(m_pBKKeyboardArray);

	if (m_pBKKeyboardArray)
	{
		for (int i = 0; i < m_nArraySize; ++i)
		{
			if (m_pBKKeyboardArray[i].nType == BKKeyType::REGULAR)
			{
				if (m_pBKKeyboardArray[i].nScanCode == nScancode)
				{
					return m_pBKKeyboardArray[i].nUniqueNum;
				}
			}
		}
	}

	return 0;
}

BKKey *CBKKbdButn::GetBKKeyByScan(uint16_t nScancode)
{
    ASSERT(m_pBKKeyboardArray);

    if (m_pBKKeyboardArray)
    {
        for (int i = 0; i < m_nArraySize; ++i)
        {
//            if (m_pBKKeyboardArray[i].nType == BKKeyType::REGULAR)
            {
                if (m_pBKKeyboardArray[i].nScanCode == nScancode)
                {
                    return &m_pBKKeyboardArray[i];
                }
            }
        }
    }
    return nullptr;
}


const BKKey CBKKbdButn::m_ButnKbdKeys[] =
{
    { BKKeyType::REGULAR,   5,   5,  100,  68, 0001, 0274, 000,             KBDKEY_POVT },      // BKKEY_POVT       ПОВТ
    { BKKeyType::REGULAR, 102,   5,  199,  68, 0003, 0060, SCANCODE_ESC,    KBDKEY_KT },        // BKKEY_KT         КТ
    { BKKeyType::REGULAR, 201,   5,  297,  68, 0013, 0274, 000,             KBDKEY_DELRIGHT },  // BKKEY_RGTDEL     del rgt
    { BKKeyType::REGULAR, 299,   5,  396,  68, 0026, 0060, SCANCODE_DEL,    KBDKEY_SDV },       // BKKEY_SDVIG      del
    { BKKeyType::REGULAR, 398,   5,  494,  68, 0027, 0060, SCANCODE_INSERT, KBDKEY_RAZDV },     // BKKEY_RAZDVIG    ins
    { BKKeyType::REGULAR, 496,   5,  593,  68, 0002, 0274, 000,             KBDKEY_INDSU },     // BKKEY_INDSU      ИНДСУ
    { BKKeyType::REGULAR, 595,   5,  691,  68, 0004, 0274, 000,             KBDKEY_BLKRED },    // BKKEY_BLOKRED    БЛОКРЕД
    { BKKeyType::REGULAR, 693,   5,  789,  68, 0000, 0274, 000,             KBDKEY_SHAG },      // BKKEY_SHAG       ШАГ
    { BKKeyType::REGULAR, 792,   5,  904,  68, 0014, 0060, 000,             KBDKEY_SBR },       // BKKEY_SBR        СБР

    { BKKeyType::STOP,    906,   5, 1019,  68, 0000, 0000, SCANCODE_BREAK,  KBDKEY_STOP },      // СТОП
    { BKKeyType::LSHIFT,    5,  70,   68, 134, 0000, 0000, SCANCODE_LSHIFT, KBDKEY_DSHIFT },    // shift

    { BKKeyType::REGULAR,  70,  70,  134, 134, 0073, 0060, SCANCODE_PLUS,   KBDKEY_SEMI },  // ;
    { BKKeyType::REGULAR, 136,  70,  200, 134, 0061, 0060, SCANCODE_1,      KBDKEY_1 },     // 1
    { BKKeyType::REGULAR, 201,  70,  265, 134, 0062, 0060, SCANCODE_2,      KBDKEY_2 },     // 2
    { BKKeyType::REGULAR, 267,  70,  331, 134, 0063, 0060, SCANCODE_3,      KBDKEY_3 },     // 3
    { BKKeyType::REGULAR, 333,  70,  396, 134, 0064, 0060, SCANCODE_4,      KBDKEY_4 },     // 4
    { BKKeyType::REGULAR, 398,  70,  462, 134, 0065, 0060, SCANCODE_5,      KBDKEY_5 },     // 5
    { BKKeyType::REGULAR, 464,  70,  528, 134, 0066, 0060, SCANCODE_6,      KBDKEY_6 },     // 6
    { BKKeyType::REGULAR, 530,  70,  593, 134, 0067, 0060, SCANCODE_7,      KBDKEY_7 },     // 7
    { BKKeyType::REGULAR, 595,  70,  659, 134, 0070, 0060, SCANCODE_8,      KBDKEY_8 },     // 8
    { BKKeyType::REGULAR, 661,  70,  725, 134, 0071, 0060, SCANCODE_9,      KBDKEY_9 },     // 9
    { BKKeyType::REGULAR, 726,  70,  790, 134, 0060, 0060, SCANCODE_0,      KBDKEY_0 },     // 0
    { BKKeyType::REGULAR, 792,  70,  856, 134, 0055, 0060, SCANCODE_MINUS,  KBDKEY_MINUS }, // -
    { BKKeyType::REGULAR, 857,  70,  921, 134, 0072, 0060, SCANCODE_COLON,  KBDKEY_COLON }, // :
    { BKKeyType::REGULAR, 923,  70, 1019, 134, 0030, 0060, SCANCODE_BACSPACE, KBDKEY_BACKSPACE },   // BKKEY_ZAB      backspace
    { BKKeyType::REGULAR,   5, 136,  100, 200, 0011, 0274, SCANCODE_TAB,    KBDKEY_TAB },           // BKKEY_TAB      ТАБ
    { BKKeyType::REGULAR, 102, 136,  166, 200, 0112, 0060, SCANCODE_J,      KBDKEY_J },     // J
    { BKKeyType::REGULAR, 168, 136,  232, 200, 0103, 0060, SCANCODE_C,      KBDKEY_C },     // C
    { BKKeyType::REGULAR, 234, 136,  297, 200, 0125, 0060, SCANCODE_U,      KBDKEY_U },     // U
    { BKKeyType::REGULAR, 299, 136,  363, 200, 0113, 0060, SCANCODE_K,      KBDKEY_K },     // K
    { BKKeyType::REGULAR, 365, 136,  429, 200, 0105, 0060, SCANCODE_E,      KBDKEY_E },     // E
    { BKKeyType::REGULAR, 431, 136,  494, 200, 0116, 0060, SCANCODE_N,      KBDKEY_N },     // N
    { BKKeyType::REGULAR, 496, 136,  560, 200, 0107, 0060, SCANCODE_G,      KBDKEY_G },     // G
    { BKKeyType::REGULAR, 562, 136,  626, 200, 0133, 0060, SCANCODE_LEFTBR, KBDKEY_LEFTBR },    // [
    { BKKeyType::REGULAR, 628, 136,  691, 200, 0135, 0060, SCANCODE_RIGHTBR, KBDKEY_RIGHTBR },  // ]
    { BKKeyType::REGULAR, 693, 136,  757, 200, 0132, 0060, SCANCODE_Z,      KBDKEY_Z },     // Z
    { BKKeyType::REGULAR, 759, 136,  823, 200, 0110, 0060, SCANCODE_H,      KBDKEY_H },     // H
    { BKKeyType::REGULAR, 825, 136,  888, 200, 0137, 0060, SCANCODE_QUOTE,  KBDKEY_RIFHTFBR },  // } Ъ
    { BKKeyType::REGULAR, 890, 136,  954, 200, 0057, 0060, SCANCODE_QUESTION, KBDKEY_QUESTION }, // ? /
    { BKKeyType::REGULAR, 956, 136, 1019, 200, 0023, 0060, 000,             KBDKEY_BC },            // BKKEY_VS     ВС

//    { BKKeyType::CTRL,      5, 202,  117, 265, 0000, 0000, SCANCODE_LWIN,   KBDKEY_SU },    // СУ
    { BKKeyType::CTRL,      5, 202,  117, 265, 0000, 0000, SCANCODE_RALT,   KBDKEY_SU },    // СУ

    { BKKeyType::REGULAR, 119, 202,  183, 265, 0106, 0060, SCANCODE_F,      KBDKEY_F },     // F
    { BKKeyType::REGULAR, 184, 202,  248, 265, 0131, 0060, SCANCODE_Y,      KBDKEY_Y },     // Y
    { BKKeyType::REGULAR, 250, 202,  314, 265, 0127, 0060, SCANCODE_W,      KBDKEY_W },     // W
    { BKKeyType::REGULAR, 316, 202,  380, 265, 0101, 0060, SCANCODE_A,      KBDKEY_A },     // A
    { BKKeyType::REGULAR, 382, 202,  445, 264, 0120, 0060, SCANCODE_P,      KBDKEY_P },     // P
    { BKKeyType::REGULAR, 447, 202,  511, 265, 0122, 0060, SCANCODE_R,      KBDKEY_R },     // R
    { BKKeyType::REGULAR, 513, 202,  577, 265, 0117, 0060, SCANCODE_O,      KBDKEY_O },     // O
    { BKKeyType::REGULAR, 579, 202,  642, 265, 0114, 0060, SCANCODE_L,      KBDKEY_L },     // L
    { BKKeyType::REGULAR, 644, 202,  708, 265, 0104, 0060, SCANCODE_D,      KBDKEY_D },     // D
    { BKKeyType::REGULAR, 710, 202,  773, 265, 0126, 0060, SCANCODE_V,      KBDKEY_V },     // V
    { BKKeyType::REGULAR, 775, 202,  839, 265, 0134, 0060, SCANCODE_BACKSLASH, KBDKEY_BKSLASH }, // \ Э
    { BKKeyType::REGULAR, 841, 202,  905, 265, 0056, 0060, SCANCODE_GREAT,  KBDKEY_GREAT }, // > .
    { BKKeyType::REGULAR, 906, 202, 1019, 265, 0012, 0060, SCANCODE_ENTER,  KBDKEY_ENTER },         // BKKEY_ENTER  enter

    { BKKeyType::ZAGL,      5, 267,   85, 331, 0000, 0000, SCANCODE_CAPS,   KBDKEY_ZAGL },  // ЗАГЛ
    { BKKeyType::STR,      87, 267,  150, 331, 0000, 0000, SCANCODE_CAPS,   KBDKEY_STR },   // СТР

    { BKKeyType::REGULAR, 153, 267,  217, 331, 0121, 0060, SCANCODE_Q,      KBDKEY_Q },     // Q
    { BKKeyType::REGULAR, 218, 267,  282, 331, 0136, 0060, 000,             KBDKEY_CH },    // Ч
    { BKKeyType::REGULAR, 284, 267,  348, 331, 0123, 0060, SCANCODE_S,      KBDKEY_S },     // S
    { BKKeyType::REGULAR, 350, 267,  413, 331, 0115, 0060, SCANCODE_M,      KBDKEY_M },     // M
    { BKKeyType::REGULAR, 415, 267,  479, 331, 0111, 0060, SCANCODE_I,      KBDKEY_I },     // I
    { BKKeyType::REGULAR, 481, 267,  544, 331, 0124, 0060, SCANCODE_T,      KBDKEY_T },     // T
    { BKKeyType::REGULAR, 546, 267,  610, 331, 0130, 0060, SCANCODE_X,      KBDKEY_X },     // X
    { BKKeyType::REGULAR, 612, 267,  676, 331, 0102, 0060, SCANCODE_B,      KBDKEY_B },     // B
    { BKKeyType::REGULAR, 678, 267,  741, 331, 0100, 0060, 000,             KBDKEY_AT },    // @
    { BKKeyType::REGULAR, 743, 267,  823, 331, 0054, 0060, SCANCODE_LESS,   KBDKEY_LESS },  // < ,
    { BKKeyType::REGULAR,   5, 333,  134, 396, 0016, 0060, SCANCODE_LCTRL,  KBDKEY_RUS },           // BKKEY_RUS    РУС

    { BKKeyType::ALT,     136, 333,  216, 396, 0000, 0000, SCANCODE_LALT,   KBDKEY_AR2 },   // АР2

    { BKKeyType::REGULAR, 218, 333,  691, 396, 0040, 0060, SCANCODE_SPACE,  KBDKEY_SPACE },         // BKKEY_PROBEL     space
    { BKKeyType::REGULAR, 693, 333,  822, 396, 0017, 0060, SCANCODE_RCTRL,  KBDKEY_LAT },           // BKKEY_LAT        ЛАТ
    { BKKeyType::REGULAR, 824, 267,  888, 396, 0010, 0060, SCANCODE_LEFT,   KBDKEY_LEFT },          // BKKEY_L_ARROW    left
    { BKKeyType::REGULAR, 890, 267,  954, 331, 0032, 0060, SCANCODE_UP,     KBDKEY_UP },            // BKKEY_U_ARROW    up
    { BKKeyType::REGULAR, 890, 333,  954, 396, 0033, 0060, SCANCODE_DOWN,   KBDKEY_DOWN },          // BKKEY_D_ARROW    down
    { BKKeyType::REGULAR, 956, 267, 1019, 396, 0031, 0060, SCANCODE_RIGHT,  KBDKEY_RIGHT },         // BKKEY_R_ARROW    right

    { BKKeyType::ENDARRAY,  0,   0,    0,   0,    0,    0,  0, 0 }
};

const BKKey CBKKbdButn::m_PlenKbdKeys[] =
{
    { BKKeyType::ALT,        4,   4,   66,  68, 0000, 0000, SCANCODE_LALT,  KBDKEY_AR2 }, // НР
    { BKKeyType::CTRL,      68,   4,  130,  68, 0000, 0000, 000,            KBDKEY_SU }, // СУ
    { BKKeyType::STOP,     132,   4,  193,  68, 0000, 0000, 000,            KBDKEY_STOP }, // СТОП
    { BKKeyType::REGULAR,  195,   4,  256,  68, 0000, 0274, 000,            KBDKEY_SHAG }, // ШАГ
    { BKKeyType::REGULAR,  258,   4,  320,  68, 0002, 0274, 000,            KBDKEY_INDSU }, // ИНДСУ
    { BKKeyType::REGULAR,  322,   4,  384,  68, 0004, 0274, 000,            KBDKEY_BLKRED }, // БЛОКРЕД
    { BKKeyType::REGULAR,  386,   4,  448,  68, 0005, 0274, 000,            KBDKEY_GRAPH }, // ГРАФ
    { BKKeyType::REGULAR,  450,   4,  511,  68, 0006, 0274, 000,            KBDKEY_ZAP }, // ЗАП
    { BKKeyType::REGULAR,  513,   4,  574,  68, 0007, 0274, 000,            KBDKEY_STIR }, // СТИР
    { BKKeyType::REGULAR,  576,   4,  638,  68, 0015, 0060, 000,            KBDKEY_USTTAB }, // УСТТАБ
    { BKKeyType::REGULAR,  640,   4,  702,  68, 0020, 0060, 000,            KBDKEY_SBRTAB }, // СБРТАБ
    { BKKeyType::REGULAR,  704,   4,  766,  68, 0003, 0060, SCANCODE_ESC,   KBDKEY_KT }, // КТ
    { BKKeyType::REGULAR,  768,   4,  829,  68, 0023, 0060, 000, KBDKEY_BC }, // ВС
    { BKKeyType::REGULAR,  831,   4,  893,  68, 0014, 0060, 000, KBDKEY_SBR }, // СБР
    { BKKeyType::REGULAR,  896,   4,  956,  68, 0024, 0060, 000, KBDKEY_GT }, // ГТ
    { BKKeyType::REGULAR,  958,   4, 1020,  68, 0013, 0274, 000, KBDKEY_DELRIGHT }, // СБР пр
    { BKKeyType::REGULAR,    4,  70,   66, 134, 0073, 0060, SCANCODE_PLUS,  KBDKEY_SEMI }, // ;
    { BKKeyType::REGULAR,   68,  70,  130, 134, 0061, 0060, SCANCODE_1,     KBDKEY_1 }, // 1
    { BKKeyType::REGULAR,  132,  70,  193, 134, 0062, 0060, SCANCODE_2,     KBDKEY_2 }, // 2
    { BKKeyType::REGULAR,  195,  70,  256, 134, 0063, 0060, SCANCODE_3,     KBDKEY_3 }, // 3
    { BKKeyType::REGULAR,  258,  70,  320, 134, 0064, 0060, SCANCODE_4,     KBDKEY_4 }, // 4
    { BKKeyType::REGULAR,  322,  70,  384, 134, 0065, 0060, SCANCODE_5,     KBDKEY_5 }, // 5
    { BKKeyType::REGULAR,  386,  70,  448, 134, 0066, 0060, SCANCODE_6,     KBDKEY_6 }, // 6
    { BKKeyType::REGULAR,  450,  70,  511, 134, 0067, 0060, SCANCODE_7,     KBDKEY_7 }, // 7
    { BKKeyType::REGULAR,  513,  70,  574, 134, 0070, 0060, SCANCODE_8,     KBDKEY_8 }, // 8
    { BKKeyType::REGULAR,  576,  70,  638, 134, 0071, 0060, SCANCODE_9,     KBDKEY_9 }, // 9
    { BKKeyType::REGULAR,  640,  70,  702, 134, 0060, 0060, SCANCODE_0,     KBDKEY_0 }, // 0
    { BKKeyType::REGULAR,  704,  70,  766, 134, 0055, 0060, SCANCODE_MINUS, KBDKEY_MINUS }, // -
    { BKKeyType::REGULAR,  768,  70,  829, 134, 0034, 0060, 000, KBDKEY_LEFTUP }, // left-up
    { BKKeyType::REGULAR,  831,  70,  893, 134, 0032, 0060, SCANCODE_UP, KBDKEY_UP }, // up
    { BKKeyType::REGULAR,  896,  70,  956, 134, 0035, 0060, 000, KBDKEY_RIGHTUP }, // right-up
    { BKKeyType::REGULAR,  958,  70, 1020, 134, 0001, 0274, 000, KBDKEY_POVT }, // ПОВТ
    { BKKeyType::REGULAR,    4, 136,   66, 200, 0112, 0060, SCANCODE_J,     KBDKEY_J }, // J
    { BKKeyType::REGULAR,   68, 136,  130, 200, 0103, 0060, SCANCODE_C,     KBDKEY_C }, // C
    { BKKeyType::REGULAR,  132, 136,  193, 200, 0125, 0060, SCANCODE_U,     KBDKEY_U }, // U
    { BKKeyType::REGULAR,  195, 136,  256, 200, 0113, 0060, SCANCODE_K,     KBDKEY_K }, // K
    { BKKeyType::REGULAR,  258, 136,  320, 200, 0105, 0060, SCANCODE_E,     KBDKEY_E }, // E
    { BKKeyType::REGULAR,  322, 136,  384, 200, 0116, 0060, SCANCODE_N,     KBDKEY_N }, // N
    { BKKeyType::REGULAR,  386, 136,  448, 200, 0107, 0060, SCANCODE_G,     KBDKEY_G }, // G
    { BKKeyType::REGULAR,  450, 136,  511, 200, 0133, 0060, 000, KBDKEY_LEFTBR }, // [
    { BKKeyType::REGULAR,  513, 136,  574, 200, 0135, 0060, 000, KBDKEY_RIGHTBR }, // ]
    { BKKeyType::REGULAR,  576, 136,  638, 200, 0132, 0060, SCANCODE_Z,     KBDKEY_Z }, // Z
    { BKKeyType::REGULAR,  640, 136,  702, 200, 0110, 0060, SCANCODE_H,     KBDKEY_H }, // H
    { BKKeyType::REGULAR,  704, 136,  766, 200, 0072, 0060, 000, KBDKEY_COLON }, // :
    { BKKeyType::REGULAR,  768, 136,  829, 200, 0010, 0060, SCANCODE_LEFT, KBDKEY_LEFT }, // left
    { BKKeyType::REGULAR,  831, 136,  893, 200, 0022, 0060, 000, KBDKEY_TOP }, // top
    { BKKeyType::REGULAR,  896, 136,  956, 200, 0031, 0060, SCANCODE_RIGHT, KBDKEY_RIGHT }, // right
    { BKKeyType::RESERVED, 958, 136, 1020, 200, 0000, 0000, 000, KBDKEY_REZERV1 }, // Резерв
    { BKKeyType::REGULAR,    4, 202,   66, 265, 0106, 0060, SCANCODE_F,     KBDKEY_F }, // F
    { BKKeyType::REGULAR,   68, 202,  130, 265, 0131, 0060, SCANCODE_Y,     KBDKEY_Y }, // Y
    { BKKeyType::REGULAR,  132, 202,  193, 265, 0127, 0060, SCANCODE_W,     KBDKEY_W }, // W
    { BKKeyType::REGULAR,  195, 202,  256, 265, 0101, 0060, SCANCODE_A,     KBDKEY_A }, // A
    { BKKeyType::REGULAR,  258, 202,  320, 265, 0120, 0060, SCANCODE_P,     KBDKEY_P }, // P
    { BKKeyType::REGULAR,  322, 202,  384, 265, 0122, 0060, SCANCODE_R,     KBDKEY_R }, // R
    { BKKeyType::REGULAR,  386, 202,  448, 265, 0117, 0060, SCANCODE_O,     KBDKEY_O }, // O
    { BKKeyType::REGULAR,  450, 202,  511, 265, 0114, 0060, SCANCODE_L,     KBDKEY_L }, // L
    { BKKeyType::REGULAR,  513, 202,  574, 265, 0104, 0060, SCANCODE_D,     KBDKEY_D }, // D
    { BKKeyType::REGULAR,  576, 202,  638, 265, 0126, 0060, SCANCODE_V,     KBDKEY_V }, // V
    { BKKeyType::REGULAR,  640, 202,  702, 265, 0134, 0060, SCANCODE_BACKSLASH, KBDKEY_BKSLASH }, // \ Э
    { BKKeyType::REGULAR,  704, 202,  766, 265, 0056, 0060, SCANCODE_GREAT, KBDKEY_GREAT }, // > .
    { BKKeyType::REGULAR,  768, 202,  829, 265, 0037, 0060, 000, KBDKEY_LEFTDOWN }, // left-down
    { BKKeyType::REGULAR,  831, 202,  893, 265, 0033, 0060, 000, KBDKEY_DOWN }, // down
    { BKKeyType::REGULAR,  896, 202,  956, 265, 0036, 0060, 000, KBDKEY_RIGHTDOWN }, // right-down
    { BKKeyType::RESERVED, 958, 202, 1020, 265, 0000, 0000, 000, KBDKEY_REZERV2 }, // Резерв
    { BKKeyType::REGULAR,    4, 267,   66, 331, 0121, 0060, SCANCODE_Q,     KBDKEY_Q }, // Q
    { BKKeyType::REGULAR,   68, 267,  130, 331, 0136, 0060, 000, KBDKEY_CH }, // Ч
    { BKKeyType::REGULAR,  132, 267,  193, 331, 0123, 0060, SCANCODE_S,     KBDKEY_S }, // S
    { BKKeyType::REGULAR,  195, 267,  256, 331, 0115, 0060, SCANCODE_M,     KBDKEY_M }, // M
    { BKKeyType::REGULAR,  258, 267,  320, 331, 0111, 0060, SCANCODE_I,     KBDKEY_I }, // I
    { BKKeyType::REGULAR,  322, 267,  384, 331, 0124, 0060, SCANCODE_T,     KBDKEY_T }, // T
    { BKKeyType::REGULAR,  386, 267,  448, 331, 0130, 0060, SCANCODE_X,     KBDKEY_X }, // X
    { BKKeyType::REGULAR,  450, 267,  511, 331, 0102, 0060, SCANCODE_B,     KBDKEY_B }, // B
    { BKKeyType::REGULAR,  513, 267,  574, 331, 0100, 0060, SCANCODE_2,     KBDKEY_AT }, // @
    { BKKeyType::REGULAR,  576, 267,  638, 331, 0054, 0060, SCANCODE_LESS, KBDKEY_LESS }, // < ,
    { BKKeyType::REGULAR,  640, 267,  702, 331, 0057, 0060, SCANCODE_QUESTION, KBDKEY_QUESTION }, // ? /
    { BKKeyType::REGULAR,  704, 267,  766, 331, 0137, 0060, SCANCODE_BACSPACE, KBDKEY_BACKSPACE }, // ЗБ
    { BKKeyType::REGULAR,  768, 267,  829, 331, 0026, 0060, SCANCODE_DEL, KBDKEY_SDV }, // сдвижка
    { BKKeyType::REGULAR,  831, 267,  893, 331, 0030, 0060, SCANCODE_BACSPACE, KBDKEY_BACKSPACE }, // backspace
    { BKKeyType::REGULAR,  896, 267,  956, 331, 0027, 0060, SCANCODE_INSERT, KBDKEY_RAZDV }, // раздвижка
    { BKKeyType::RESERVED, 958, 267, 1020, 331, 0000, 0000, 000, KBDKEY_REZERV3 }, // Резерв
    { BKKeyType::LSHIFT,     4, 333,   66, 397, 0000, 0000, 000, KBDKEY_DSHIFT }, // ПР
    { BKKeyType::ZAGL,      68, 333,  130, 397, 0000, 0000, SCANCODE_CAPS,  KBDKEY_ZAGL }, // ЗАГЛ
    { BKKeyType::REGULAR,  132, 333,  256, 397, 0016, 0060, SCANCODE_LCTRL, KBDKEY_RUS }, // РУС
    { BKKeyType::REGULAR,  258, 333,  511, 397, 0040, 0060, SCANCODE_SPACE, KBDKEY_SPACE }, // space
    { BKKeyType::REGULAR,  513, 333,  638, 397, 0017, 0060, SCANCODE_RCTRL, KBDKEY_LAT }, // ЛАТ
    { BKKeyType::STR,      640, 333,  702, 397, 0000, 0000, SCANCODE_CAPS,  KBDKEY_STR }, // СТР
    { BKKeyType::RSHIFT,   704, 333,  766, 397, 0000, 0000, SCANCODE_LSHIFT, KBDKEY_DSHIFT }, // ПР-2
    { BKKeyType::REGULAR,  768, 333,  829, 397, 0011, 0274, SCANCODE_TAB, KBDKEY_TAB }, // ТАБ
    { BKKeyType::REGULAR,  831, 333,  956, 397, 0012, 0060, SCANCODE_ENTER, KBDKEY_ENTER }, // ВВОД
    { BKKeyType::RESERVED, 958, 333, 1020, 397, 0000, 0000, 000, KBDKEY_REZERV4 }, // Резерв

    { BKKeyType::ENDARRAY,   0,   0,    0,   0,    0,    0,  0,  0}
};


