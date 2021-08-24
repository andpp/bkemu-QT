// SprView.cpp: файл реализации
//
#if 0

#include "pch.h"
#include "SprView.h"


// CSprView

IMPLEMENT_DYNAMIC(CSprView, CScrollView)

CSprView::CSprView()
	: m_cx(0)
	, m_cy(0)
{
	m_pScreen = new CSprScr();
}

CSprView::~CSprView()
{
	if (m_pScreen)
	{
		delete m_pScreen;
	}
}

BEGIN_MESSAGE_MAP(CSprView, CScrollView)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_DESTROY()
END_MESSAGE_MAP()



// обработчики сообщений CSprView

void CSprView::OnDestroy()
{
	if (m_pScreen)
	{
		m_pScreen->DestroyWindow();
	}

	CScrollView::OnDestroy();
}

int CSprView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CScrollView::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}

	SetScrollSizes(MM_TEXT, CSize(m_cx, m_cy));
	EnableScrollBarCtrl(SB_HORZ, FALSE);

	if (m_pScreen)
	{
		if (!m_pScreen->Create(nullptr, _T("BKScreen"), WS_VISIBLE | WS_CHILD, CRect(0, 0, m_cx, m_cy), this, 0))
		{
			TRACE0("Не удалось создать экран\n");
			return -1; // не удалось создать
		}
	}

	return 0;
}

BOOL CSprView::PreCreateWindow(CREATESTRUCT &cs)
{
	cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	m_cx = cs.cx;
	m_cy = cs.cy;
	return CScrollView::PreCreateWindow(cs);
}

void CSprView::OnSize(UINT nType, int cx, int cy)
{
	CScrollView::OnSize(nType, cx, cy);

	// корректируем размеры картинки, чтобы они были кратны байту,
	// т.к. размеры окна мы можем менять попиксельно, а размеры картинки - только побайтно
	if (m_pScreen)
	{
		int nScale = m_pScreen->GetScale();
		m_cx = cx;
		m_cy = cy;
		int s_cx = m_cx / nScale;
		s_cx &= ~7;

		if (s_cx == 0)
		{
			s_cx = 8;
			m_cx = s_cx * nScale;
		}

		int nTotalPixels = static_cast<int>(m_pScreen->GetBufferSize()) * 8;
		int s_cy = nTotalPixels / s_cx;

		while (nTotalPixels > s_cx * s_cy)
		{
			s_cy++;
		}

		m_pScreen->AdjustLayout(s_cx * nScale, s_cy * nScale, GetScrollPos(SB_VERT));
		SetScrollSizes(MM_TEXT, CSize(s_cx * nScale, s_cy * nScale));
	}

	EnableScrollBarCtrl(SB_HORZ, FALSE);
}

void CSprView::OnDraw(CDC *pDC)
{
	if (m_pScreen)
	{
		m_pScreen->DrawScreen();
	}
}


#endif
