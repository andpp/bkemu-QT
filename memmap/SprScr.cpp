// ScreenDIB.cpp: файл реализации
//

#if 0

#include "pch.h"
#include "Config.h"
#include "SprScr.h"
#include "BKMessageBox.h"

// CSprScr

IMPLEMENT_DYNAMIC(CSprScr, CWnd)

CSprScr::CSprScr()
	: m_pBuffer(nullptr)
	, m_nBufSize(0)
	, m_PaletteNum(0)
	, m_nScale(1)
	, m_cx(0)
	, m_cy(0)
	, m_orig_cx(0)
	, m_orig_cy(0)
	, m_bColorMode(true)
	, m_bAdapt(false)
	, m_bBusy(false)
	, m_bChangeMode(false)
	, m_pColTable32(nullptr)
	, m_pMonoTable32(nullptr)
	, m_bits(nullptr)
	, m_hwndScreen(nullptr)   // Screen View window handle
	, m_hdd(nullptr)
	, m_hbmp(nullptr)
{
}

CSprScr::~CSprScr()
{
}

BEGIN_MESSAGE_MAP(CSprScr, CWnd)
	ON_WM_CREATE()
	ON_WM_ERASEBKGND()
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// обработчики сообщений CSprScr
void CSprScr::ScreenView_Init()
{
	m_hdd = DrawDibOpen();
	ASSERT(m_hwndScreen != nullptr);
	InitColorTables();

	if (m_hbmp)
	{
		DeleteObject(m_hbmp);
		m_hbmp = nullptr;
	}

	HDC hdc = ::GetDC(m_hwndScreen);
	m_bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bmpinfo.bmiHeader.biWidth = m_cx;
	m_bmpinfo.bmiHeader.biHeight = m_cy;
	m_bmpinfo.bmiHeader.biPlanes = 1;
	m_bmpinfo.bmiHeader.biBitCount = 32;
	m_bmpinfo.bmiHeader.biCompression = BI_RGB;
	m_bmpinfo.bmiHeader.biSizeImage = 0;
	m_bmpinfo.bmiHeader.biXPelsPerMeter = 0;
	m_bmpinfo.bmiHeader.biYPelsPerMeter = 0;
	m_bmpinfo.bmiHeader.biClrUsed = 0;
	m_bmpinfo.bmiHeader.biClrImportant = 0;
	m_hbmp = CreateDIBSection(hdc, &m_bmpinfo, DIB_RGB_COLORS, (void **) &m_bits, nullptr, 0);
	::ReleaseDC(m_hwndScreen, hdc);
}

void CSprScr::ScreenView_Done()
{
	SAFE_DELETE_ARRAY(m_pColTable32);
	SAFE_DELETE_ARRAY(m_pMonoTable32);
	SAFE_DELETE_OBJECT(m_hbmp);
	DrawDibClose(m_hdd);
}


void CSprScr::OnDestroy()
{
	ScreenView_Done();
	CWnd::OnDestroy();
}

int CSprScr::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	m_hwndScreen = GetSafeHwnd();
	ScreenView_Init();
	return 0;
}

BOOL CSprScr::OnEraseBkgnd(CDC *pDC)
{
	return TRUE;
	// return CWnd::OnEraseBkgnd(pDC);
}

BOOL CSprScr::PreCreateWindow(CREATESTRUCT &cs)
{
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_PARENTDC | CS_SAVEBITS,
	                                   ::LoadCursor(nullptr, IDC_ARROW), HBRUSH(COLOR_WINDOW + 1), nullptr);
	m_cx = cs.cx;
	m_cy = cs.cy;
	CalcOrigSize();
	return CWnd::PreCreateWindow(cs);
}

void CSprScr::CalcOrigSize()
{
	m_orig_cx = m_cx / m_nScale;
	m_orig_cy = m_cy / m_nScale;
}

void CSprScr::AdjustLayout(int cx, int cy, int ysp)
{
	m_cy = cy;
	m_cx = cx;
	MoveWindow(0, -ysp, m_cx, m_cy, FALSE);
	CalcOrigSize();
	PrepareScreenRGB32();
}

void CSprScr::SetColorMode(bool bColorMode)
{
	if (m_bColorMode != bColorMode)
	{
		m_bColorMode = bColorMode;
		ReDrawScreen();
	}
}

void CSprScr::SetAdaptMode(bool bFlag)
{
	if (m_bAdapt != bFlag)
	{
		m_bAdapt = bFlag;
		InitColorTables();
		ReDrawScreen();
	}
}

void CSprScr::SetScale(int scale)
{
	if (m_nScale != scale)
	{
		m_nScale = scale;
	}
}

void CSprScr::SetPalette(int palette)
{
	palette &= 0xf;

	if (m_PaletteNum != palette)
	{
		m_PaletteNum = palette;
		ReDrawScreen();
	}
}

void CSprScr::DrawScreen()
{
	if (m_bits == nullptr || m_bBusy || m_bChangeMode)
	{
		return;
	}

	HDC hdc = ::GetDC(m_hwndScreen);
	/*
	m_orig_cx, m_orig_cy - координаты оригинального размера
	m_cx, m_cy - смасштабированные координаты
	а само масштабирование делается средствами DrawDibDraw
	*/
	BOOL bRes = DrawDibDraw(m_hdd, hdc,
	                        0, 0, m_cx, m_cy,
	                        &m_bmpinfo.bmiHeader, m_bits,
	                        0, 0, m_orig_cx, m_orig_cy,
	                        0);
	::ReleaseDC(m_hwndScreen, hdc);
}

void CSprScr::ReDrawScreen()
{
	PrepareScreenRGB32();
	DrawScreen();
}

void CSprScr::PrepareScreenRGB32()
{
	if (m_orig_cx * m_orig_cy <= 0)
	{
		return;
	}

	m_bBusy = true;

	if (m_hbmp)
	{
		DeleteObject(m_hbmp);
		m_hbmp = nullptr;
	}

	HDC hdc = ::GetDC(m_hwndScreen);
	m_bmpinfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_bmpinfo.bmiHeader.biWidth = m_orig_cx;
	m_bmpinfo.bmiHeader.biHeight = m_orig_cy;
	m_hbmp = CreateDIBSection(hdc, &m_bmpinfo, DIB_RGB_COLORS, (void **) &m_bits, nullptr, 0);
	::ReleaseDC(m_hwndScreen, hdc);
	size_t i = 0;
	int nLineBytes = m_orig_cx / 8;

	for (int y = m_orig_cy - 1; y >= 0; y--)
	{
		uint32_t *pBits = m_bits + y * m_orig_cx;
		int x = 0;

		while (x < nLineBytes)
		{
			uint32_t *pPalette = ((m_bColorMode) ? m_pColTable32 : m_pMonoTable32) + (((m_PaletteNum << 8) + m_pBuffer[i++]) << 3);
			memcpy(pBits, pPalette, 8 * sizeof(uint32_t)); // копируем 1 байт
			pBits += 8;
			x++;

			if (i > m_nBufSize)
			{
				while (x < nLineBytes) // заполняем остаток строки чёрным цветом
				{
					ZeroMemory(pBits, 8 * sizeof(uint32_t));
					pBits += 8;
					x++;
				}

				break;
			}
		}
	}

	m_bBusy = false;
}

constexpr auto COLTABLE_SIZE = (16 * 256 * 8);
// 16 палитр, 256 вариантов значений байта, на каждый вариант - 8 uint32_tов,
void CSprScr::InitColorTables()
{
	m_bChangeMode = true;
	// Create color tables of all display modes for faster drawing
	SAFE_DELETE_ARRAY(m_pColTable32);
	SAFE_DELETE_ARRAY(m_pMonoTable32);
	m_pColTable32 = new uint32_t[COLTABLE_SIZE];
	m_pMonoTable32 = new uint32_t[COLTABLE_SIZE];

	if (m_pColTable32 && m_pMonoTable32)
	{
		for (int p = 0; p < 16; ++p) // для каждой из 16 палитр
		{
			// вот это делает одну палитру. а нам надо таких 16.
			for (int i = 0; i < 256; ++i) // для каждого значения байта (от 0 до 255) делаем маску, чтоб работать сразу с байтами и не заморачиваться с битами
			{
				uint32_t *pColBuff32 = &m_pColTable32[p * (256 * 8) + i * 8];
				uint32_t *pMonBuff32 = &m_pMonoTable32[p * (256 * 8) + i * 8];

				for (int n = 0; n < 4; ++n) // в каждом байте 4 двухбития, на каждые два бита, у нас отводится по 4 дворда
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
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

	m_bChangeMode = false;
}

#endif
