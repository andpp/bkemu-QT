// Screen.cpp: файл реализации
//
#include "pch.h"
#include "Screen.h"
#include "Config.h"

#undef BKSCRDLL_EXPORTS

//#include "BKScreenD2D\BKScreenD2D.h"
//#ifdef TARGET_WINXP
//#include "BKScreenD3D\BKScreenD3D.h"
//#else
//#include "BKScreenD3D11\BKScreenD3D.h"
//#endif
//#include "BKScreenDIB\BKScreenDIB.h"
//#include "BKScreenOGL\BKScreenOGL.h"

#include "BKScreenOGL.h"

#include "BKMessageBox.h"

constexpr int TEXTURE_WIDTH  = (SCREEN_WIDTH  *TEXTURE_MULTIPLER_X);
constexpr int TEXTURE_HEIGHT = (SCREEN_HEIGHT *TEXTURE_MULTIPLER_Y);

constexpr double BK_ASPECT_RATIO  = (4.0 / 3.0);

// CScreen

//IMPLEMENT_DYNAMIC(CScreen, CWnd)

CScreen::CScreen(CONF_SCREEN_RENDER nRenderType) : QObject()
    , m_pBuffer(nullptr)
	, m_nBufSize(0)
//	, m_pscrSharedFunc(nullptr)
//	, m_hModule(nullptr)
	, m_pColTable32(nullptr)
	, m_pMonoTable32(nullptr)
	, m_bSmoothing(false)
	, m_bColorMode(true)
	, m_bAdapt(false)
	, m_bExtended(false)
	, m_bLuminoforeEmul(false)
	, m_nOfs(0330)
	, m_nPaletteNum_m256(0)
	, m_nFrame(0)
	, m_nCurFPS(0)
	, m_pTexBits(nullptr)
	, m_nCurrentScr(0)
//	, m_hChildStd_IN_Rd(nullptr)
//	, m_hChildStd_IN_Wr(nullptr)

#if (BK_USE_CONVEYORTHREAD)
	, m_nFreeScreens(BK_SCRBUFFERS)
	, m_bThreadTerminate(false)
#endif

#if (_DEBUG && DBG_OUT_SCREENFRAMELENGTH)
	, m_nTickCounter(0)
	, dbgFile(nullptr)
#endif // _DEBUG

	// мышь марсианка
	, m_nPointX(0)
	, m_nPointY(0)
	, m_MouseValue(0)
	, m_bMouseMove(false)
	, m_bMouseOutEna(false)
	, m_nMouseEnaStrobe(0)
    , BK_SCREEN_WIDTH(1024)
    , BK_SCREEN_HEIGHT(static_cast<int>(BK_SCREEN_WIDTH / BK_ASPECT_RATIO))
{
	InitVars(nRenderType);
}

CScreen::CScreen(CONF_SCREEN_RENDER nRenderType, uint8_t *buffer, size_t size)
	: m_pBuffer(buffer)
	, m_nBufSize(size)
//	, m_pscrSharedFunc(nullptr)
//	, m_hModule(nullptr)
	, m_pColTable32(nullptr)
	, m_pMonoTable32(nullptr)
	, m_bSmoothing(false)
	, m_bColorMode(true)
	, m_bAdapt(false)
	, m_bExtended(false)
    , m_bReverseScreen(0)
    , m_bLuminoforeEmul(false)
	, m_nOfs(0330)
	, m_nPaletteNum_m256(0)
	, m_nFrame(0)
	, m_nCurFPS(0)
	, m_pTexBits(nullptr)
    , m_nCurrentScr(0)
//	, m_hChildStd_IN_Rd(nullptr)
//	, m_hChildStd_IN_Wr(nullptr)

#if (BK_USE_CONVEYORTHREAD)
	, m_nFreeScreens(BK_SCRBUFFERS)
	, m_bThreadTerminate(false)
#endif

#if (_DEBUG && DBG_OUT_SCREENFRAMELENGTH)
	, m_nTickCounter(0)
	, dbgFile(nullptr)
#endif // _DEBUG

	// мышь марсианка
	, m_nPointX(0)
	, m_nPointY(0)
	, m_MouseValue(0)
	, m_bMouseMove(false)
	, m_bMouseOutEna(false)
	, m_nMouseEnaStrobe(0)
    , BK_SCREEN_WIDTH(1024)
    , BK_SCREEN_HEIGHT(static_cast<int>(BK_SCREEN_WIDTH / BK_ASPECT_RATIO))
{
	InitVars(nRenderType);
}

//#ifdef _WIN64
//#define BK_OGLDLLNAME _T("BKScreenOGL_x64.dll")
//#define BK_D2DDLLNAME _T("BKScreenD2D_x64.dll")
//#define BK_DIBDLLNAME _T("BKScreenDIB_x64.dll")
//#define BK_D3DDLLNAME _T("BKScreenD3D_x64.dll")
//#else
//#define BK_OGLDLLNAME _T("BKScreenOGL.dll")
//#define BK_D2DDLLNAME _T("BKScreenD2D.dll")
//#define BK_DIBDLLNAME _T("BKScreenDIB.dll")
//#define BK_D3DDLLNAME _T("BKScreenD3D.dll")
//#endif

#define BK_SCRDLLFUNC "GetBKScreen"

void CScreen::InitVars(CONF_SCREEN_RENDER nRenderType)
{

//    m_pscrSharedFunc = new CScreenOGL(nullptr);

//	if (m_pscrSharedFunc)
//	{
//		m_bReverseScreen = m_pscrSharedFunc->BKSS_GetReverseFlag();
//	}
    m_bReverseScreen = 0;


#if (BK_USE_CONVEYORTHREAD)
	StartDrawThread();
#endif
}

CScreen::~CScreen()
{

#if (BK_USE_CONVEYORTHREAD)
	StopDrawThread();
#endif
	ClearObjects();
}


void CScreen::OnDestroy()
{
    killTimer(BKTIMER_SCREEN_FPS);
    killTimer(BKTIMER_MOUSE);

//	if (m_bCaptureProcessed)
//	{
//		CancelCapture();
//	}

#if (BK_USE_CONVEYORTHREAD)
	StopDrawThread();
#endif

	// перед выходом дождёмся прекращения выполнения рисования
	while (m_lockBusy.IsLocked())
	{
        Sleep(1);
	}

	// и заблокируем его во избежание потенциальных дедлоков
//	m_lockBusy.Lock();

//    if (m_pscrSharedFunc)
//	{
//        m_pscrSharedFunc->BKSS_ScreenView_Done();
//	}

//	m_lockBusy.UnLock();
//	CWnd::OnDestroy();
}

void CScreen::ClearObjects()
{
	SAFE_DELETE_ARRAY(m_pColTable32);
	SAFE_DELETE_ARRAY(m_pMonoTable32);
//	SAFE_DELETE(m_pscrSharedFunc);
	SAFE_DELETE_ARRAY(m_pTexBits);
#if (BK_USE_CONVEYORTHREAD)

	for (auto &pBit : m_scrParam.pBits)
	{
		SAFE_DELETE_ARRAY(pBit);
	}

#endif

//	if (m_hModule)
//	{
//		FreeLibrary(m_hModule);
//		m_hModule = nullptr;
//	}
}

// обработчики сообщений CScreen
//BOOL CScreen::PreCreateWindow(CREATESTRUCT &cs)
//{
//	m_pwndParent = FromHandle(cs.hwndParent);
//	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_OWNDC | CS_SAVEBITS,
//	                                   ::LoadCursor(nullptr, IDC_ARROW), HBRUSH(::GetStockObject(BLACK_BRUSH)), nullptr);
//	return CWnd::PreCreateWindow(cs);
//}

int CScreen::OnCreate(/*LPCREATESTRUCT lpCreateStruct*/)
{
//	if (CWnd::OnCreate(lpCreateStruct) == -1)
//	{
//		return -1;
//	}

//	if (m_pscrSharedFunc == nullptr)
//	{
//		g_BKMsgBox.Show(IDS_BK_ERROR_SCRDLLFUNCPTRERR, MB_OK);
//		return -1;
//	}

	if (!InitColorTables())
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_SCRCOLORTABLEERR, MB_OK);
		return -1;
	}

    BKTIMER_SCREEN_FPS = startTimer(1000, Qt::PreciseTimer);
    BKTIMER_MOUSE = startTimer(10, Qt::PreciseTimer);
	// заполняем параметры экрана.
	m_scrParam.nTxX = TEXTURE_WIDTH;
	m_scrParam.nTxY = TEXTURE_HEIGHT;
#if 0
	// рассчитаем размеры окна полноэкранного режима
	int wx, wy, dx, dy;
//	HDC dc = ::GetDC(nullptr);
	m_scrParam.rcFSDim.left = 0;
	m_scrParam.rcFSDim.top =  0;
    dx = m_scrParam.rcFSDim.right = width();   // ширина экрана
    dy = m_scrParam.rcFSDim.bottom = height();  // высота экрана
	wx = static_cast<int>(static_cast<double>(dy) * BK_ASPECT_RATIO);  // ширина экрана при высоте dy в пропорциях 4/3
	wy = static_cast<int>(static_cast<double>(dx) / BK_ASPECT_RATIO);  // высота экрана при ширине dx в пропорциях 4/3
//	::ReleaseDC(nullptr, dc);

	// рассчитаем размеры рисуемой картинки в полноэкранном режиме
	if (dx <= dy) // если монитор повёрнут на 90 градусов, или нестандартный - квадратный
	{
		if (dx < wx) // если не влазит по ширине, вписываем в ширину
		{
			wy = dy;
			wx = static_cast<int>(static_cast<double>(wy) * BK_ASPECT_RATIO);
		}
		else
		{
			// то вписываем картинку в высоту
			wx = dx;
			wy = static_cast<int>(static_cast<double>(wx) / BK_ASPECT_RATIO);
		}
	}
	else    // если монитор в обычном положении
	{
		if (dx < wx) // если не влазит по ширине, вписываем в ширину
		{
			wx = dx;
			wy = static_cast<int>(static_cast<double>(wx) / BK_ASPECT_RATIO);
		}
		else
		{
			// то вписываем картинку в высоту
			wy = dy;
			wx = static_cast<int>(static_cast<double>(wy) * BK_ASPECT_RATIO);
		}
	}

	m_scrParam.rcFSViewPort.left = (dx - wx) / 2;   // выравниваем по центру
	m_scrParam.rcFSViewPort.top = (dy - wy) / 2;
	m_scrParam.rcFSViewPort.right = wx;
	m_scrParam.rcFSViewPort.bottom = wy;
#if (_DEBUG && DBG_OUT_SCREENFRAMELENGTH)
	m_nTickCounter = GetTickCount();
	dbgFile = fopen("dbgScreen.txt", "wt");
#endif // _DEBUG
#endif
	// создаём и инициализируем экраны
	int nBitSize = m_scrParam.nTxX * m_scrParam.nTxY;
	m_nBitBufferSize = nBitSize * sizeof(uint32_t);
	m_pTexBits = new uint32_t[nBitSize];
#if (BK_USE_CONVEYORTHREAD)

	for (auto &pBit : m_scrParam.pBits)
	{
		SAFE_DELETE_ARRAY(pBit);
		pBit = new uint32_t[nBitSize];
	}

#endif

#if 0
	if (SUCCEEDED(m_pscrSharedFunc->BKSS_ScreenView_Init(m_scrParam, this)))
	{
		return 1;
	}
//	CString str;
//	str.Format(IDS_BK_ERROR_SCRDLLINITERR, m_strDllName);
//	g_BKMsgBox.Show(str, MB_OK);
    m_pscrSharedFunc->BKSS_ScreenView_Done();
    ClearObjects();
    return -1;
#endif
    return 1;
}

//void CScreen::OnSize(UINT nType, int cx, int cy)
//{
////	CWnd::OnSize(nType, cx, cy);
//    resize(cx, cy);
//	m_pscrSharedFunc->BKSS_OnSize(cx, cy);
//}


//BOOL CScreen::OnEraseBkgnd(CDC *pDC)
//{
//	return TRUE;
//}

//BOOL CScreen::PreTranslateMessage(MSG *pMsg)
//{
//	switch (pMsg->message)
//	{
//		// транслируем сообщения клавиатуры в CBKView.
//		// это крайне необходимо для рендера D3D11, без этого в полноэкранном режиме никак
//		// клавиатуру не обработать.
//		case WM_KEYDOWN:
//		case WM_KEYUP:
//		case WM_SYSKEYDOWN:
//		case WM_SYSKEYUP:
//			TRACE("CScreen translate kbd message to CBKView\n");
//			m_pwndParent->PostMessage(pMsg->message, pMsg->wParam, pMsg->lParam);
//			return TRUE;
//	}

//	return CWnd::PreTranslateMessage(pMsg);
//}

void CScreen::timerEvent(QTimerEvent *event)
{
    int nIDEvent = event->timerId();
    if(nIDEvent == BKTIMER_SCREEN_FPS) {
			m_mutFPS.lock();
			m_nCurFPS = m_nFrame;
			m_nFrame = 0;
			m_mutFPS.unlock();
    } else if (nIDEvent == BKTIMER_MOUSE) {
			if (!m_bMouseMove)
			{
				m_MouseValue &= ~017;
			}

			m_bMouseMove = false;
	}
}

#if (BK_USE_CONVEYORTHREAD)
void CScreen::_DrawScreen(bool bCheckFPS)
{
	// здесь перерисовывается текущее состояние экрана. он может быть и не до конца подготовленным,
	// там может быть и предыдущее содержимое
	DrawScreen(m_pTexBits, bCheckFPS);
	TRACE0("*** UI REDRAW ***\n");
}

void CScreen::DrawScreen(UINT32 *pTexBits, bool bCheckFPS)
{
	if (pTexBits == nullptr || m_lockChangeMode.IsLocked() || m_lockBusy.IsLocked())
	{
		TRACE0("*** Draw Skipped ***\n");
		return;
	}

	m_lockBusy.Lock();
	m_pscrSharedFunc->BKSS_DrawScreen(pTexBits);
	m_lockBusy.UnLock();
#if (_DEBUG && DBG_OUT_SCREENFRAMELENGTH)
	register DWORD nTmpTick = GetTickCount();
	register DWORD nFrame = nTmpTick - m_nTickCounter;
	m_nTickCounter = nTmpTick;
	fprintf(dbgFile, "%d ms\n", nFrame);
#endif // _DEBUG

	if (bCheckFPS)
	{
		m_mutFPS.lock();
		// Посчитаем FPS
		m_nFrame++;
		m_mutFPS.unlock();
	}
}

void CScreen::PrepareFrame()
{
	// 1. посылаем прорисовываться подготовленный экран m_pTexBits
	{
		std::lock_guard<std::mutex> locker(m_mutScrBuf);
		memcpy(m_scrParam.pBits[m_nCurrentScr], m_pTexBits, m_nBitBufferSize);
		m_vScrns.push(m_nCurrentScr);
		m_nFreeScreens--;
		// TRACE1("PUSH. screen: %d\n", m_nCurrentScr);
	}

	if (m_bCaptureFlag)
	{
		WriteToPipe();
	}

	// 2. ждём, пока появятся свободные экраны
	while (m_nFreeScreens <= 0)
	{
		TRACE0("wait free screens\n");
		SleepEx(0, TRUE);
	}

	// 3. назначаем следующий экран.
	if (++m_nCurrentScr >= BK_SCRBUFFERS)
	{
		m_nCurrentScr = 0;
	}
}

bool CScreen::StartDrawThread()
{
	m_DrawThread = std::thread(&CScreen::DrawThreadFunc, this);

	if (m_DrawThread.joinable())
	{
		m_DrawThread.detach();
		return true;
	}

	return false;
}

void CScreen::StopDrawThread()
{
	m_bThreadTerminate = true;
	std::lock_guard<std::mutex> lk(m_mutThread);
}

void CScreen::DrawThreadFunc()
{
	std::lock_guard<std::mutex> lk(m_mutThread);
	LARGE_INTEGER liDueTime;    // интервал таймера
// создаём waitable timer.
	HANDLE hWaitableTimer = CreateWaitableTimer(nullptr, FALSE, nullptr); // самосбрасывающийся

	if (hWaitableTimer == nullptr)
	{
		return;
	}

	liDueTime.QuadPart = -10000LL * 1000 / 50;
	// SetWaitableTimer(hWaitableTimer, &liDueTime, (1000 / 50), nullptr, nullptr, FALSE); // периодический

	do
	{
		SetWaitableTimer(hWaitableTimer, &liDueTime, 0, nullptr, nullptr, FALSE); // одноразовый, каждый раз переустанавливать надо
		{
			std::lock_guard<std::mutex> lk(m_mutScrBuf);

			// если в очереди что-то есть, достаём
			if (!m_vScrns.empty())
			{
				int nScr = m_vScrns.front();
				m_vScrns.pop();
				m_nFreeScreens++;
				// TRACE1("POP.  screen: %d\n", nScr);
				DrawScreen(m_scrParam.pBits[nScr], true);
			}
			else
			{
				TRACE0("Skip frame\n");
			}
		}
		// ждём срабатывания таймера.
		WaitForSingleObject(hWaitableTimer, INFINITE);
		// это вот ещё так можно, якобы ожидание можно остановить, когда события разные происходят
		// но проверять не будем, просто на всякий случай тут оставлю, вдруг ещё где-нибудь пригодится
		// MsgWaitForMultipleObjects(1, &hWaitableTimer, FALSE, INFINITE, QS_ALLEVENTS);
	}
	while (!m_bThreadTerminate);           // пока не придёт событие остановки

	CancelWaitableTimer(hWaitableTimer);
	CloseHandle(hWaitableTimer);
}
#else
//void CScreen::DrawScreen(bool bCheckFPS)
//{
//	if (m_pTexBits == nullptr || m_lockChangeMode.IsLocked() || m_lockBusy.IsLocked())
//	{
//		return;
//	}

//	m_lockBusy.Lock();
//	m_pscrSharedFunc->BKSS_DrawScreen(m_pTexBits);
//	m_lockBusy.UnLock();
//#if (_DEBUG && DBG_OUT_SCREENFRAMELENGTH)
//	register DWORD nTmpTick = GetTickCount();
//	register DWORD nFrame = nTmpTick - m_nTickCounter;
//	m_nTickCounter = nTmpTick;
//	fprintf(dbgFile, "%d ms\n", nFrame);
//#endif // _DEBUG

//	if (m_bCaptureFlag)
//	{
//		WriteToPipe();
//	}

//	if (bCheckFPS)
//	{
//		m_mutFPS.lock();
//		// Посчитаем FPS
//		m_nFrame++;
//		m_mutFPS.unlock();
//	}
//}
#endif


//void CScreen::RestoreFS()
//{
//	m_pscrSharedFunc->BKSS_RestoreFullScreen();
//}

//bool CScreen::SetFullScreenMode()
//{
//	while (m_lockBusy.IsLocked() || m_lockPrepare.IsLocked())
//	{
//        Sleep(1);    // если выполняется отрисовка, то подождём, пока процедуры не завершатся
//	}

//	m_lockChangeMode.Lock();
//	bool bRet = m_pscrSharedFunc->BKSS_SetFullScreenMode();
//	m_lockChangeMode.UnLock();
//	return bRet;
//}

//bool CScreen::SetWindowMode()
//{
//	while (m_lockBusy.IsLocked() || m_lockPrepare.IsLocked())
//	{
//        Sleep(1);    // если выполняется отрисовка, то подождём, пока процедуры не завершатся
//	}

//	m_lockChangeMode.Lock();
//	bool bRet = m_pscrSharedFunc->BKSS_SetWindowMode();
//	m_lockChangeMode.UnLock();
//	AdjustLayout();
//	return bRet;
//}

//bool CScreen::IsFullScreenMode()
//{
//	return m_pscrSharedFunc->BKSS_IsFullScreenMode();
//}

void CScreen::SetSmoothing(bool bSmoothing)
{
	m_bSmoothing = bSmoothing;
//	m_pscrSharedFunc->BKSS_SetSmoothing(m_bSmoothing);
}

void CScreen::SetColorMode(bool bColorMode)
{
	m_bColorMode = bColorMode;
//	m_pscrSharedFunc->BKSS_SetColorMode();
}


// этот алгоритм применяется только в режиме отладки, когда нужно принудительно перерисовывать весь экран
// и в карте памяти
void CScreen::PrepareScreenRGB32(uint8_t *ScreenBuffer)
{
//	TRACE0("*** DEBUG PREPARE ***\n");

	if (m_pTexBits == nullptr || m_lockChangeMode.IsLocked())
	{
		return;
	}

	m_lockPrepare.Lock();
	register uint8_t scroll = (m_nOfs - 0330) & 0377;
	register uint8_t *pVideo;
	register uint32_t *pBits, *pBitp, *pPalette;
	// Render to bitmap
	register uint32_t *pCurPalette = (m_bColorMode) ? m_pColTable32 : m_pMonoTable32;
	register int linesToShow = m_bExtended ? SCREEN_HEIGHT / 4 : SCREEN_HEIGHT;

	for (register int y = 0; y < linesToShow; ++y)
	{
		register uint8_t yy = (y + scroll) & 0377;
		// указатель на начало строки
		pBitp = pBits = m_pTexBits + ((m_bReverseScreen ? (255 - y) : y) * TEXTURE_WIDTH * TEXTURE_MULTIPLER_Y);
		pVideo = ScreenBuffer + (yy * 0100);

		for (register int x = 0; x < SCREEN_WIDTH / 8; ++x) // рисуем побайтно
		{
			pPalette = pCurPalette + ((m_nPaletteNum_m256 + *pVideo++) << 3);

			// один байт - это 8 бит
			for (register int i = 0; i < 8; ++i)
			{
#if (BK_USE_TEXTURE_MULTIPLER)

				// каждый бит надо повторить TEXTURE_MULTIPLER_X раз подряд.
				for (register int u = 0; u < TEXTURE_MULTIPLER_X; ++u)
				{
					*pBits++ = *pPalette; // один бит - один дворд.
				}

				pPalette++;
#else
				*pBits++ = *pPalette++; // один бит - один дворд.
#endif
			}
		}

#if (BK_USE_TEXTURE_MULTIPLER)

		// теперь pBits - указатель на начало новой строки, а pBitp - указатель на начало предыдущей строки
		// скопируем строку столько раз, сколько надо будет
		for (register int v = 1; v < TEXTURE_MULTIPLER_Y; ++v)
		{
			memcpy(pBits, pBitp, TEXTURE_WIDTH * sizeof(uint32_t));
			pBits += TEXTURE_WIDTH;
			pBitp += TEXTURE_WIDTH;
		}

#endif
	}

	if (m_bExtended)
	{
		if (!m_bReverseScreen)
		{
			// Для всех обнулять нужно область с 1/4 массива экрана до конца массива
			memset(m_pTexBits + (TEXTURE_WIDTH * TEXTURE_HEIGHT / 4), Color_Black, (TEXTURE_HEIGHT - TEXTURE_HEIGHT / 4) * TEXTURE_WIDTH * sizeof(uint32_t));
		}
		else
		{
			// Для DrawDIB обнулять нужно область с начала массива экрана до 3/4 массива экрана
			memset(m_pTexBits, Color_Black, (TEXTURE_HEIGHT - TEXTURE_HEIGHT / 4) * TEXTURE_WIDTH * sizeof(uint32_t));
		}
	}

	m_lockPrepare.UnLock();
}

// нельзя использовать constexpr, т.к. нужен именно макрос,
// т.к. нужно именно целочисленное умножение и деление в выражении
// и скобки нельзя
#define LUMINOFORE_COEFF 3 / 8
// #define  LUMINOFORE_COEFF 2 / 3

// Это старая функция, сейчас не используется.
// Но если что-то надо будет изменить, тренироваться будем на ней, а потом копипастить в PrepareScreenLineWordRGB32
// т.к. та функция - просто дважды повторённая эта.
// на входе у обеих функций:
// nLineNum - номер строки 0..255
// nByteNum - смещение байта/слова в строке относительно начала строки.
void CScreen::PrepareScreenLineByteRGB32(int nLineNum, int nByteNum, uint8_t b)
{
	if (m_pTexBits == nullptr || m_lockChangeMode.IsLocked())
	{
		return;
	}

	m_lockPrepare.Lock();
	register uint8_t nScreenLine = nLineNum & 0377;
	register uint8_t yy = 255 - nScreenLine;

	if (!m_bReverseScreen)
	{
		yy = nScreenLine;
	}

	register uint32_t *pBits = m_pTexBits + (yy * TEXTURE_MULTIPLER_Y * TEXTURE_WIDTH + nByteNum * 8 * TEXTURE_MULTIPLER_X);
#if (BK_USE_TEXTURE_MULTIPLER)
    register uint32_t *pBitp = pBits; // указатель на начало строки
#endif
	register uint32_t *pCurPalette = (m_bColorMode) ? m_pColTable32 : m_pMonoTable32;

	if (m_bExtended && nScreenLine >= (SCREEN_HEIGHT / 4))
	{
		// обнуляем
		if (m_bLuminoforeEmul)
		{
			for (register int i = 0; i < 8; ++i)
			{
				register auto c = reinterpret_cast<BCOLOR *>(pBits);
				// blue - 0, green - 1, red - 2, alpha - 3;
				c->bb[0] = LOBYTE(uint32_t(c->bb[0]) * LUMINOFORE_COEFF);
				c->bb[1] = LOBYTE(uint32_t(c->bb[1]) * LUMINOFORE_COEFF);
				c->bb[2] = LOBYTE(uint32_t(c->bb[2]) * LUMINOFORE_COEFF);
#if (BK_USE_TEXTURE_MULTIPLER)

				for (register int u = 1; u < TEXTURE_MULTIPLER_X; ++u)
				{
					*(pBits + 1) = *pBits++;
				}

#endif
				pBits++;
			}
		}
		else
		{
			constexpr auto nPitch = TEXTURE_MULTIPLER_X * 8 * sizeof(uint32_t);
			memset(pBits, 0, nPitch);
			pBits += nPitch;
		}
	}
	else
	{
		// копируем 1 байт
		register uint32_t *pPalette = pCurPalette + ((m_nPaletteNum_m256 + b) << 3);

		if (m_bLuminoforeEmul)
		{
			for (register int i = 0; i < 8; ++i)
			{
				// эмуляция затухания люминофора. Как-то не очень хорошо.
				// не удаётся подобрать вариант, чтобы и шлейф не мешал
				// и мерцания не было.
				// наверное люминофор гаснет нелинейно.
				register auto c = reinterpret_cast<BCOLOR *>(pBits);
				register auto p = reinterpret_cast<BCOLOR *>(pPalette);
				// blue - 0, green - 1, red - 2, alpha - 3;
				c->bb[0] = p->bb[0] ? p->bb[0] : LOBYTE(uint32_t(c->bb[0]) * LUMINOFORE_COEFF);
				c->bb[1] = p->bb[1] ? p->bb[1] : LOBYTE(uint32_t(c->bb[1]) * LUMINOFORE_COEFF);
				c->bb[2] = p->bb[2] ? p->bb[2] : LOBYTE(uint32_t(c->bb[2]) * LUMINOFORE_COEFF);
				c->bb[3] = p->bb[3];
#if (BK_USE_TEXTURE_MULTIPLER)

				for (register int u = 1; u < TEXTURE_MULTIPLER_X; ++u)
				{
					*(pBits + 1) = *pBits++;
				}

#endif
				pBits++;
				pPalette++;
			}
		}
		else
		{
			for (register int i = 0; i < 8; ++i)
			{
				for (register int u = 0; u < TEXTURE_MULTIPLER_X; ++u)
				{
					*pBits++ = *pPalette;
				}

				pPalette++;
			}
		}
	}

#if (BK_USE_TEXTURE_MULTIPLER)
	pBits = pBitp + TEXTURE_WIDTH;

	// размножим по вертикали
	for (register int v = 1; v < TEXTURE_MULTIPLER_Y; ++v)
	{
		memcpy(pBits, pBitp, 8 * TEXTURE_MULTIPLER_X * sizeof(uint32_t));
		pBits += TEXTURE_WIDTH;
		pBitp += TEXTURE_WIDTH;
	}

#endif
	m_lockPrepare.UnLock();
}

void CScreen::PrepareScreenLineWordRGB32(int nLineNum, int nByteNum, uint16_t w)
{
	// !много копипасты. если что-то менять, не забыть поменять в трёх местах
	if (m_pTexBits == nullptr || m_lockChangeMode.IsLocked())
	{
		return;
	}

	m_lockPrepare.Lock();
	// первый байт
	uint8_t b = LOBYTE(w);
	register uint8_t nScreenLine = nLineNum & 0377;
	register uint8_t yy = 255 - nScreenLine;

	if (!m_bReverseScreen)
	{
		yy = nScreenLine;
	}

	register uint32_t *pBits_A = m_pTexBits + (yy * TEXTURE_MULTIPLER_Y * TEXTURE_WIDTH + nByteNum * 8 * TEXTURE_MULTIPLER_X);
	register uint32_t *pBits = pBits_A;
	register uint32_t *pBitp = pBits_A; // указатель на начало строки
	register uint32_t *pCurPalette = (m_bColorMode) ? m_pColTable32 : m_pMonoTable32;

	if (m_bExtended && nScreenLine >= (SCREEN_HEIGHT / 4))
	{
		// обнуляем
		if (m_bLuminoforeEmul)
		{
			for (register int i = 0; i < 8; ++i)
			{
				register auto c = reinterpret_cast<BCOLOR *>(pBits);
				// blue - 0, green - 1, red - 2, alpha - 3;
				c->bb[0] = LOBYTE(uint32_t(c->bb[0]) * LUMINOFORE_COEFF);
				c->bb[1] = LOBYTE(uint32_t(c->bb[1]) * LUMINOFORE_COEFF);
				c->bb[2] = LOBYTE(uint32_t(c->bb[2]) * LUMINOFORE_COEFF);
#if (BK_USE_TEXTURE_MULTIPLER)

				for (register int u = 1; u < TEXTURE_MULTIPLER_X; ++u)
				{
					*(pBits + 1) = *pBits++;
				}

#endif
				pBits++;
			}
		}
		else
		{
			memset(pBits, 0, TEXTURE_MULTIPLER_X * 8 * sizeof(uint32_t));
			pBits += TEXTURE_MULTIPLER_X * 8 * sizeof(uint32_t);
		}
	}
	else
	{
		// копируем 1 байт
		register uint32_t *pPalette = pCurPalette + ((m_nPaletteNum_m256 + b) << 3);

		if (m_bLuminoforeEmul)
		{
			for (register int i = 0; i < 8; ++i)
			{
				// эмуляция затухания люминофора. Как-то не очень хорошо.
				// не удаётся подобрать вариант, чтобы и шлейф не мешал
				// и мерцания не было.
				// наверное люминофор гаснет нелинейно.
				register auto c = reinterpret_cast<BCOLOR *>(pBits);
				register auto p = reinterpret_cast<BCOLOR *>(pPalette);
				// blue - 0, green - 1, red - 2, alpha - 3;
				c->bb[0] = p->bb[0] ? p->bb[0] : LOBYTE(uint32_t(c->bb[0]) * LUMINOFORE_COEFF);
				c->bb[1] = p->bb[1] ? p->bb[1] : LOBYTE(uint32_t(c->bb[1]) * LUMINOFORE_COEFF);
				c->bb[2] = p->bb[2] ? p->bb[2] : LOBYTE(uint32_t(c->bb[2]) * LUMINOFORE_COEFF);
				c->bb[3] = p->bb[3];
#if (BK_USE_TEXTURE_MULTIPLER)

				for (register int u = 1; u < TEXTURE_MULTIPLER_X; ++u)
				{
					*(pBits + 1) = *pBits++;
				}

#endif
				pBits++;
				pPalette++;
			}
		}
		else
		{
			for (register int i = 0; i < 8; ++i)
			{
#if (BK_USE_TEXTURE_MULTIPLER)

				for (register int u = 0; u < TEXTURE_MULTIPLER_X; ++u)
				{
					*pBits++ = *pPalette;
				}

				pPalette++;
#else
				*pBits++ = *pPalette++;
#endif
			}
		}
	}

#if (BK_USE_TEXTURE_MULTIPLER)
	pBits = pBitp + TEXTURE_WIDTH;

	// размножим по вертикали
	for (register int v = 1; v < TEXTURE_MULTIPLER_Y; ++v)
	{
		memcpy(pBits, pBitp, 8 * TEXTURE_MULTIPLER_X * sizeof(uint32_t));
		pBits += TEXTURE_WIDTH;
		pBitp += TEXTURE_WIDTH;
	}

#endif
	// копипаста второй байт
	b = HIBYTE(w);
	pBits = pBitp = pBits_A + 8 * TEXTURE_MULTIPLER_X;

	if (m_bExtended && nScreenLine >= (SCREEN_HEIGHT / 4))
	{
		// обнуляем
		if (m_bLuminoforeEmul)
		{
			for (register int i = 0; i < 8; ++i)
			{
				register auto c = reinterpret_cast<BCOLOR *>(pBits);
				// blue - 0, green - 1, red - 2, alpha - 3;
				c->bb[0] = LOBYTE(uint32_t(c->bb[0]) * LUMINOFORE_COEFF);
				c->bb[1] = LOBYTE(uint32_t(c->bb[1]) * LUMINOFORE_COEFF);
				c->bb[2] = LOBYTE(uint32_t(c->bb[2]) * LUMINOFORE_COEFF);
#if (BK_USE_TEXTURE_MULTIPLER)

				for (register int u = 1; u < TEXTURE_MULTIPLER_X; ++u)
				{
					*(pBits + 1) = *pBits++;
				}

#endif
				pBits++;
			}
		}
		else
		{
			memset(pBits, 0, TEXTURE_MULTIPLER_X * 8 * sizeof(uint32_t));
			pBits += TEXTURE_MULTIPLER_X * 8 * sizeof(uint32_t);
		}
	}
	else
	{
		// копируем 1 байт
		register uint32_t *pPalette = pCurPalette + ((m_nPaletteNum_m256 + b) << 3);

		if (m_bLuminoforeEmul)
		{
			for (register int i = 0; i < 8; ++i)
			{
				// эмуляция затухания люминофора. Как-то не очень хорошо.
				// не удаётся подобрать вариант, чтобы и шлейф не мешал
				// и мерцания не было.
				// наверное люминофор гаснет нелинейно.
				register auto c = reinterpret_cast<BCOLOR *>(pBits);
				register auto p = reinterpret_cast<BCOLOR *>(pPalette);
				// blue - 0, green - 1, red - 2, alpha - 3;
				c->bb[0] = p->bb[0] ? p->bb[0] : LOBYTE(uint32_t(c->bb[0]) * LUMINOFORE_COEFF);
				c->bb[1] = p->bb[1] ? p->bb[1] : LOBYTE(uint32_t(c->bb[1]) * LUMINOFORE_COEFF);
				c->bb[2] = p->bb[2] ? p->bb[2] : LOBYTE(uint32_t(c->bb[2]) * LUMINOFORE_COEFF);
				c->bb[3] = p->bb[3];
#if (BK_USE_TEXTURE_MULTIPLER)

				for (register int u = 1; u < TEXTURE_MULTIPLER_X; ++u)
				{
					*(pBits + 1) = *pBits++;
				}

#endif
				pBits++;
				pPalette++;
			}
		}
		else
		{
			for (register int i = 0; i < 8; ++i)
			{
#if (BK_USE_TEXTURE_MULTIPLER)

				for (register int u = 0; u < TEXTURE_MULTIPLER_X; ++u)
				{
					*pBits++ = *pPalette;
				}

				pPalette++;
#else
				*pBits++ = *pPalette++;
#endif
			}
		}
	}

#if (BK_USE_TEXTURE_MULTIPLER)
	pBits = pBitp + TEXTURE_WIDTH;

	// размножим по вертикали
	for (register int v = 1; v < TEXTURE_MULTIPLER_Y; ++v)
	{
		memcpy(pBits, pBitp, 8 * TEXTURE_MULTIPLER_X * sizeof(uint32_t));
		pBits += TEXTURE_WIDTH;
		pBitp += TEXTURE_WIDTH;
	}

#endif
#ifdef _DEBUG
	//////////////////////////////////////////////////////////////////////////
	// Отладочный видимый ход луча
	// Нужно только при отладке экрана, включать только при необходимости.
	// в релизной версии принудительно отключим.
	// PrepareScreenLineByte_Debug(nLineNum, nByteNum);
	//////////////////////////////////////////////////////////////////////////
#endif
	m_lockPrepare.UnLock();
}

#ifdef _DEBUG
// отладочная трассировка луча.
// ставится красная точка на следующем месте, где будет луч.
void CScreen::PrepareScreenLineByte_Debug(int nLineNum, int nByteNum)
{
	nByteNum += 2;

	if (nByteNum >= 0100)
	{
		nByteNum = 0;
		nLineNum++;
		nLineNum &= 0377;
	}

	register uint8_t nScreenLine = nLineNum & 0377;
	register uint8_t yy = 255 - nScreenLine;

	if (!m_bReverseScreen)
	{
		yy = nScreenLine;
	}

	register uint32_t *pBits = m_pTexBits + (yy * TEXTURE_MULTIPLER_Y * TEXTURE_WIDTH + nByteNum * 8 * TEXTURE_MULTIPLER_X);
	register uint32_t *pBitp = pBits; // указатель на начало строки
	// копируем 1 байт (ставим красную точку)
	// красную не всегда видно, может что-то ещё надо будет придумать. может ксорить.
	register uint32_t *pPalette = m_pColTable32 + (3 * 8);

	for (register int i = 0; i < 2; ++i)
	{
#if (BK_USE_TEXTURE_MULTIPLER)

		for (register int u = 0; u < TEXTURE_MULTIPLER_X; ++u)
		{
			*pBits++ = *pPalette;
		}

		pPalette++;
#else
		*pBits++ = *pPalette++;
#endif
	}

#if (BK_USE_TEXTURE_MULTIPLER)
	pBits = pBitp + TEXTURE_WIDTH;

	// размножим по вертикали
	for (register int v = 1; v < TEXTURE_MULTIPLER_Y; ++v)
	{
		memcpy(pBits, pBitp, 8 * TEXTURE_MULTIPLER_X * sizeof(uint32_t));
		pBits += TEXTURE_WIDTH;
		pBitp += TEXTURE_WIDTH;
	}

#endif
}
#endif


constexpr auto COLTABLE_SIZE = (16 * 256 * 8);
// 16 палитр, 256 вариантов значений байта, на каждый вариант - 8 uint32_tов,

bool CScreen::InitColorTables()
{
	bool bRet = false;

	while (m_lockPrepare.IsLocked())   // если выполняется подготовка, то подождём, пока процедуры не завершатся
	{
        Sleep(1);
	}

	m_lockChangeMode.Lock();
	// Create color tables of all display modes for faster drawing
	SAFE_DELETE_ARRAY(m_pColTable32);
	SAFE_DELETE_ARRAY(m_pMonoTable32);
	m_pColTable32 = new uint32_t[COLTABLE_SIZE];
	m_pMonoTable32 = new uint32_t[COLTABLE_SIZE];

	if ((m_pColTable32 != nullptr) && (m_pMonoTable32 != nullptr))
	{
		for (int p = 0; p < 16; ++p)   // для каждой из 16 палитр
		{
			// вот это делает одну палитру. а нам надо таких 16.
			for (int i = 0; i < 256; ++i)   // для каждого значения байта (от 0 до 255) делаем маску, чтоб работать сразу с байтами и не заморачиваться с битами
			{
				uint32_t *pColBuff32 = &m_pColTable32[p * (256 * 8) + i * 8];
				uint32_t *pMonBuff32 = &m_pMonoTable32[p * (256 * 8) + i * 8];

				for (int n = 0; n < 4; ++n)   // в каждом байте 4 двухбития, на каждые два бита, у нас отводится по 4 дворда
				{
					int n2 = n * 2;

					switch ((i >> n2) & 3)
					{
						// Black
						case 0:
							pColBuff32[n2] = g_pColorPalettes[p][0];
							pMonBuff32[n2++] = (m_bAdapt) ? g_pAdaptMonochromePalette[0][0] : g_pMonochromePalette[0][0];
							pColBuff32[n2] = g_pColorPalettes[p][0];
							pMonBuff32[n2] = (m_bAdapt) ? g_pAdaptMonochromePalette[1][0] : g_pMonochromePalette[1][0];
							break;

						// Blue and Gray
						case 1:
							pColBuff32[n2] = g_pColorPalettes[p][1];
							pMonBuff32[n2++] = (m_bAdapt) ? g_pAdaptMonochromePalette[0][1] : g_pMonochromePalette[0][1];
							pColBuff32[n2] = g_pColorPalettes[p][1];
							pMonBuff32[n2] = (m_bAdapt) ? g_pAdaptMonochromePalette[1][1] : g_pMonochromePalette[1][1];
							break;

						// Green and Gray
						case 2:
							pColBuff32[n2] = g_pColorPalettes[p][2];
							pMonBuff32[n2++] = (m_bAdapt) ? g_pAdaptMonochromePalette[0][2] : g_pMonochromePalette[0][2];
							pColBuff32[n2] = g_pColorPalettes[p][2];
							pMonBuff32[n2] = (m_bAdapt) ? g_pAdaptMonochromePalette[1][2] : g_pMonochromePalette[1][2];
							break;

						// Red
						case 3:
							pColBuff32[n2] = g_pColorPalettes[p][3];
							pMonBuff32[n2++] = (m_bAdapt) ? g_pAdaptMonochromePalette[0][3] : g_pMonochromePalette[0][3];
							pColBuff32[n2] = g_pColorPalettes[p][3];
							pMonBuff32[n2] = (m_bAdapt) ? g_pAdaptMonochromePalette[1][3] : g_pMonochromePalette[1][3];
							break;
					}
				}
			} // end for(i)
		} // end for(p)

		bRet = true;
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

	m_lockChangeMode.UnLock();
	return bRet;
}

void CScreen::AdjustLayout()
{
	if (m_pwndParent == nullptr)
	{
		return;
	}

/*
	while (m_lockChangeMode.IsLocked() || m_lockBusy.IsLocked())
	{
		SleepEx(0, TRUE);    // если выполняется отрисовка, то подождём, пока процедуры не завершатся
	}

	m_lockChangeMode.Lock();
	CRect rcScreen(0, 0, BK_SCREEN_WIDTH, BK_SCREEN_HEIGHT);
	CRect rcNewScreen = rcScreen;
	CRect rcClient;
	m_pwndParent->GetClientRect(&rcClient);

	if (rcScreen.Width() * rcClient.Height() > rcClient.Width() * rcScreen.Height())
	{
		rcNewScreen.right = rcClient.Width();
		rcNewScreen.bottom = rcScreen.bottom * rcClient.Width() / rcScreen.right;
	}
	else
	{
		rcNewScreen.bottom = rcClient.Height();
		rcNewScreen.right = rcScreen.right * rcClient.Height() / rcScreen.bottom;
	}

	if (rcNewScreen.Width() > BK_SCREEN_WIDTH)
	{
		rcNewScreen.right = BK_SCREEN_WIDTH;
	}

	if (rcNewScreen.Height() > BK_SCREEN_HEIGHT)
	{
		rcNewScreen.bottom = BK_SCREEN_HEIGHT;
	}

	int x_offs = (rcClient.Width() - rcNewScreen.Width()) / 2;
	int y_offs = (rcClient.Height() - rcNewScreen.Height()) / 2;
	rcClient.right = rcClient.left + rcNewScreen.Width();
	rcClient.bottom = rcClient.top + rcNewScreen.Height();
	rcClient.OffsetRect(x_offs, y_offs);
	SetWindowPos(nullptr, rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), SWP_NOZORDER);
	m_lockChangeMode.UnLock();
 */
}

// мышь марсианка


uint16_t CScreen::GetMouseStatus()
{
	if (m_bMouseOutEna)
	{
		m_bMouseOutEna = false;
		return m_MouseValue;
	}

	return 0;
}

void CScreen::SetMouseStrobe(uint16_t data)
{
	if (!m_nMouseEnaStrobe && (data & 010))
	{
		m_bMouseOutEna = true;
	}
	else
	{
		m_bMouseOutEna = false;
	}

	m_nMouseEnaStrobe = (data & 010);
}

void CScreen::OnMouseMove(UINT nFlags, CPoint point)
{
	// определим направление движения
	int dx = m_nPointX - point.x;
	int dy = m_nPointY - point.y;
	m_nPointX = point.x;
	m_nPointY = point.y;

	m_MouseValue &= ~017;
	if (dx > 0)			// направление влево
	{
		m_MouseValue |= 010;   // LEFT
	}
	else if (dx < 0)	// направление вправо
	{
		m_MouseValue |= 002;	// RIGHT
	}
	// если == 0, то никуда не движемся

	if (dy > 0)			// направление вверх
	{
		m_MouseValue |= 001;   // UP
	}
	else if (dy < 0)	// направление вниз
	{
		m_MouseValue |= 004;   // DOWN
	}
	// если == 0, то никуда не движемся
	m_bMouseMove = !!(m_MouseValue & 017);

//	CWnd::OnMouseMove(nFlags, point);
}


void CScreen::OnLButtonDown(UINT nFlags, CPoint point)
{
	m_MouseValue |= 040;

//	CWnd::OnLButtonDown(nFlags, point);
}


void CScreen::OnLButtonUp(UINT nFlags, CPoint point)
{
	m_MouseValue &= ~040;

//	CWnd::OnLButtonUp(nFlags, point);
}


void CScreen::OnRButtonDown(UINT nFlags, CPoint point)
{
	m_MouseValue |= 0100;

//	CWnd::OnRButtonDown(nFlags, point);
}


void CScreen::OnRButtonUp(UINT nFlags, CPoint point)
{
	m_MouseValue &= ~0100;

//	CWnd::OnRButtonUp(nFlags, point);
}
