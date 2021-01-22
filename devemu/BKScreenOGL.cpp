// BKScreenOGL.cpp: определяет экспортированные функции для приложения DLL.
//

#include "pch.h"
#include "BKScreenOGL.h"

//#pragma comment (lib,"opengl32.lib")
//#pragma comment (lib,"glu32.lib")

#include "Screen_Sizes.h"

//BKEXTERN_C
//{
//	BKSCRDLL_API BKSCREENHANDLE APIENTRY GetBKScreen()
//	{
//		return new CScreenOGL;
//	}


	const GLdouble CScreenOGL::m_cpTexcoords2[] =
	{
		0.0, 0.0, 0.9999, 0.0,
		0.9999, 0.9999, 0.0, 0.9999
	};

	const GLdouble CScreenOGL::m_cpVertices2[] =
	{
		0.0, 0.0, GLdouble(SCREEN_WIDTH), 0.0,
		GLdouble(SCREEN_WIDTH), GLdouble(SCREEN_HEIGHT), 0.0, GLdouble(SCREEN_HEIGHT)
	};

	const GLint CScreenOGL::m_cpIndices1[] = { 0, 1, 2, 3 };

	CScreenOGL::CScreenOGL() : CBKScreen_Shared()
//		, m_HDC(nullptr)
//		, hGLRC(nullptr)
		, m_bScrParamChanged(true)
		, m_bScrSizeChanged(true)
		, m_nTextureParam(GL_NEAREST)
	{
		m_rectWndVP = RECT();
		m_rectViewPort = RECT();
	}


	CScreenOGL::~CScreenOGL()
	{
	}


	void CScreenOGL::BKSS_OnSize(int cx, int cy)
	{
//		if (m_HDC && (cy > 0) && (cx > 0))
        if ((cy > 0) && (cx > 0))
        {
			m_rectWndVP.right = cx;
			m_rectWndVP.bottom = cy;
			m_rectViewPort = m_rectWndVP; // т.к. это работает только в оконном режиме.
			m_bScrSizeChanged = true;
		}
	}


	HRESULT CScreenOGL::BKSS_ScreenView_Init(ScrsParams & sspar, CWnd * pwndScr)
	{
		CBKScreen_Shared::BKSS_ScreenView_Init(sspar, pwndScr);
//		HRESULT hr = E_FAIL;
//		m_HDC = pwndScr->GetDC()->GetSafeHdc();

//		if (!m_HDC)
//		{
//			return E_FAIL;
//		}

		if (m_bFullScreen)
		{
			BKSS_SetFullScreenMode();
		}
		else
		{
			BKSS_SetWindowMode();
		}

		clear();
		return S_OK;
	}

	void CScreenOGL::BKSS_ScreenView_Done()
	{
//		if (wglGetCurrentContext())
//		{
//			wglMakeCurrent(nullptr, nullptr);
//		}

//		if (hGLRC)
//		{
//			wglDeleteContext(hGLRC);
//			hGLRC = nullptr;
//		}
	}

	void CScreenOGL::BKSS_DrawScreen(uint32_t *pBits)
	{
//		if (!wglMakeCurrent(m_HDC, hGLRC))
//		{
//			if (hGLRC) // каждый раз пересоздаём контекст
//			{
//				wglMakeCurrent(m_HDC, nullptr);
//				wglDeleteContext(hGLRC);
//				hGLRC = wglCreateContext(m_HDC);
//				wglMakeCurrent(m_HDC, hGLRC);
//			}

//			m_bScrParamChanged = true;
//		}

		if (m_bScrParamChanged)
		{
			set_screen_param();
		}

		if (m_bScrSizeChanged)
		{
			glViewport(m_rectViewPort.left, m_rectViewPort.top, m_rectViewPort.right, m_rectViewPort.bottom);
			m_bScrSizeChanged = false;
		}

		// Update Texture
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_nTextureWidth, m_nTextureHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, reinterpret_cast<GLvoid *>(pBits));
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_DOUBLE, 0, m_cpTexcoords2);
		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(2, GL_DOUBLE, 0, m_cpVertices2);
		glDrawElements(GL_QUADS, 4, GL_UNSIGNED_INT, m_cpIndices1);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glFlush();
//		SwapBuffers(m_HDC);
//		wglMakeCurrent(nullptr, nullptr);
	}

	bool CScreenOGL::BKSS_SetFullScreenMode()
	{
		bool bRet = false;

//		if (!m_bFullScreen)
//		{
//			// запомним состояние главного окна. На будущее, когда придумаем, как правильно
//			// работать в мультимониторных конфигурациях
//			m_mainPlacement.length = sizeof(WINDOWPLACEMENT);
//			GetWindowPlacement(m_pwndMain->GetSafeHwnd(), &m_mainPlacement);
//			// запомним состояние окна экрана в оконном режиме
//			m_windowedPlacement.length = sizeof(WINDOWPLACEMENT);
//			GetWindowPlacement(m_hwndScreen, &m_windowedPlacement);
//			// меняем стили с чилд на попап
//			SetWindowLongPtr(m_hwndScreen, GWL_STYLE, (GetWindowLongPtr(m_hwndScreen, GWL_STYLE) & ~WS_CHILD) | WS_POPUP);
//			SetParent(m_hwndScreen, nullptr); // отвязываем от родителя
//			// сворачиваем главное окно в иконку.
//			WINDOWPLACEMENT mnplacement = m_mainPlacement;
//			mnplacement.flags |= WPF_SETMINPOSITION;
//			mnplacement.showCmd = SW_SHOWMINIMIZED;
//			SetWindowPlacement(m_pwndMain->GetSafeHwnd(), &mnplacement);
//			// Но главное окно никуда не девается. А всё потому, что мы принудительно ему фокус передаём!
//			// Мало того, если принудительно скрыть главное окно
//			// эмулятор вообще останавливает работу. Так что такая фича не прокатывает.
//			// разворачивем экран на весь экран монитора, при этом он всегда разворачивается
//			// на основном мониторе
//			m_pwndScreen->SetWindowPos(&CWnd::wndTop, m_rectFullScreen.left, m_rectFullScreen.top, m_rectFullScreen.right, m_rectFullScreen.bottom, SWP_SHOWWINDOW);
//			m_bFullScreen = true;
//			bRet = true;
//		}

//		CreateContext();
//		m_rectViewPort = m_rectFSVP;
//		m_bScrSizeChanged = true;
		return bRet;
	}

	bool CScreenOGL::BKSS_SetWindowMode()
	{
//		if (m_bFullScreen)
//		{
//			// возвращаем привязку к родителю
//			SetParent(m_hwndScreen, m_pwndParent->GetSafeHwnd());
//			// возвращаем стиль чилд и убираем попап
//			SetWindowLongPtr(m_hwndScreen, GWL_STYLE, (GetWindowLongPtr(m_hwndScreen, GWL_STYLE) & ~WS_POPUP) | WS_CHILD);
//			// возвращаем положение экрана в оконном режиме
//			SetWindowPlacement(m_hwndScreen, &m_windowedPlacement);
//			// возвращаем положение главного окна
//			SetWindowPlacement(m_pwndMain->GetSafeHwnd(), &m_mainPlacement);
//			m_bFullScreen = false;
//		}

		CreateContext();
		m_rectViewPort = m_rectWndVP;
		m_bScrSizeChanged = true;
		return true;
	}

	void CScreenOGL::BKSS_SetColorMode()
	{
		m_bScrParamChanged = true;
	}

	void CScreenOGL::BKSS_SetSmoothing(bool bSmoothing)
	{
		m_nTextureParam = bSmoothing ? GL_LINEAR : GL_NEAREST;
		m_bScrParamChanged = true;
	}


/////////////////////////////////////////////////////////////////////////////
// GL helper functions
	bool CScreenOGL::SetWindowPixelFormat()
	{
//		static PIXELFORMATDESCRIPTOR pfd =
//		{
//			sizeof(PIXELFORMATDESCRIPTOR),  // size of this pfd
//			1,                              // version number
//			PFD_DRAW_TO_WINDOW |            // support window
//			PFD_SUPPORT_OPENGL |            // support OpenGL
//			PFD_DOUBLEBUFFER | PFD_DEPTH_DONTCARE,               // double buffered
//			PFD_TYPE_RGBA,                  // RGBA type
//			32,                             // 32-bit color depth
//			0, 0, 0, 0, 0, 0,               // color bits ignored
//			0,                              // no alpha buffer
//			0,                              // shift bit ignored
//			0,                              // no accumulation buffer
//			0, 0, 0, 0,                     // accum bits ignored
//			32,                             // 32-bit z-buffer
//			0,                              // no stencil buffer
//			0,                              // no auxiliary buffer
//			PFD_MAIN_PLANE,                 // main layer
//			0,                              // reserved
//			0, 0, 0                         // layer masks ignored
//		};
//		int pixelformat;

//		if ((pixelformat = ChoosePixelFormat(m_HDC, &pfd)) == 0)
//		{
//			AfxMessageBox(_T("Screen: ChoosePixelFormat failed"));
//			return false;
//		}

//		if (SetPixelFormat(m_HDC, pixelformat, &pfd) == FALSE)
//		{
//			AfxMessageBox(_T("Screen: SetPixelFormat failed"));
//			return false;
//		}

		return true;
	}

	void CScreenOGL::set_screen_param()
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
//		gluOrtho2D(0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
		glDisable(GL_DEPTH_TEST);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		// Set up the texture
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, m_nTextureParam);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, m_nTextureParam);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		// Enable textures
		glEnable(GL_TEXTURE_2D);
		m_bScrParamChanged = false;
	}

	void CScreenOGL::CreateContext()
	{
//		wglMakeCurrent(nullptr, nullptr);

		if (SetWindowPixelFormat())
		{
//			int n = ::GetPixelFormat(m_HDC);
//			::DescribePixelFormat(m_HDC, n, sizeof(m_pfd), &m_pfd);

//			if (hGLRC)
//			{
//				wglDeleteContext(hGLRC);
//			}

//			hGLRC = wglCreateContext(m_HDC);
//			wglMakeCurrent(m_HDC, hGLRC);
			set_screen_param();
//			wglMakeCurrent(nullptr, nullptr);
		}
	}


	void CScreenOGL::clear()
	{
		glClearColor(0.0, 0.0, 0.0, 0.0);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

//};
