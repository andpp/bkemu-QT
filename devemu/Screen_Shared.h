#pragma once
#include "SafeReleaseDefines.h"
#include "pch.h"

#include <QWindow>
#define CWnd QWidget

// условная переменная, чтобы использовать вывод на экран
// через очередь буферов в отдельном потоке.
// Применение улучшений не дало никаких, но замечены ухудшения в звуке.
#define BK_USE_CONVEYORTHREAD 0

//#ifdef BKSCRDLL_EXPORTS
//#define BKSCRDLL_API __declspec(dllexport)
//#else
//#define BKSCRDLL_API __declspec(dllimport)
//#endif

//#define BKSCRDLL_API

#ifdef __cplusplus
#define BKEXTERN_C     extern "C"
#else
#define BKEXTERN_C
#endif // __cplusplus

#if (BK_USE_CONVEYORTHREAD)
constexpr auto BK_SCRBUFFERS = 4; // количество экранных буферов, можно сделать очередь
#endif

// структура параметров экрана
struct ScrsParams
{
#if (BK_USE_CONVEYORTHREAD)
	uint32_t *pBits[BK_SCRBUFFERS]; // массив экранных буферов, можно сделать очередь
#endif
	int     nTxX;       // реальный размер текстуры по Х
	int     nTxY;       // реальный размер текстуры по Y
    RECT    rcFSDim;    // текущее разрешение экрана
	RECT    rcFSViewPort;    // размер viewporta в полноэкранном режиме

	ScrsParams()
		: nTxX(0)
		, nTxY(0)
		, rcFSDim(RECT())
		, rcFSViewPort(RECT())
	{
#if (BK_USE_CONVEYORTHREAD)

		for (auto &pBit : pBits)
		{
			pBit = nullptr;
		}

#endif
	}
};

/*BKEXTERN_C*/ class /*BKSCRDLL_API*/ CBKScreen_Shared
{
	protected:
		HWND                m_hwndScreen;       // хэндл окна оконного режима
		CWnd               *m_pwndMain;         // указатель на главное окно
		CWnd               *m_pwndParent;       // указатель на родительское окно (View)
		CWnd               *m_pwndScreen;       // указатель на окно оконного режима
		bool                m_bFullScreen;      // флаг работы в полноэкранном режиме
		int                 m_nTextureWidth;
		int                 m_nTextureHeight;
		RECT                m_rectFullScreen;   // размеры окна в полноэкранном режиме
		RECT                m_rectFSVP;         // размеры viewporta в полноэкранном режиме
		// left, top - координаты экрана, right, bottom - размеры, а не координаты!!!

		WINDOWPLACEMENT     m_windowedPlacement;
		WINDOWPLACEMENT     m_mainPlacement;

	public:
		virtual HRESULT     BKSS_ScreenView_Init(ScrsParams &sspar, CWnd *pwndScr)
		{
			// заполняем хэндлы и указатели на окна
			m_pwndScreen     = pwndScr;
//            m_pwndParent     = m_pwndScreen->pGetParent();
//			m_pwndMain       = m_pwndParent->GetParentOwner();
//			m_hwndScreen     = m_pwndScreen->GetSafeHwnd();
			m_nTextureWidth  = sspar.nTxX;
			m_nTextureHeight = sspar.nTxY;
			m_rectFullScreen = sspar.rcFSDim;
			m_rectFSVP       = sspar.rcFSViewPort;
			return S_OK;
		}
		virtual void        BKSS_ScreenView_Done() {}
        virtual void        BKSS_DrawScreen(uint32_t *pBits) {(void)pBits;}
		virtual void        BKSS_RestoreFullScreen() {}
		virtual bool        BKSS_SetFullScreenMode()
		{
			return false;
		}
		virtual bool        BKSS_SetWindowMode()
		{
			return false;
		}
		virtual CWnd       *BKSS_GetBackgroundWindow()
		{
			return nullptr;
		}

        virtual void        BKSS_SetSmoothing(bool bSmoothing) { (void)bSmoothing;}
		virtual void        BKSS_SetColorMode() {}
		virtual void        BKSS_OnEraseBackground() {}
        virtual void        BKSS_OnSize(int cx, int cy) {(void)cx; (void)cy; }

		CBKScreen_Shared()
			: m_hwndScreen(nullptr)
			, m_pwndMain(nullptr)
			, m_pwndParent(nullptr)
			, m_pwndScreen(nullptr)
			, m_bFullScreen(false)
			, m_nTextureWidth(0)
			, m_nTextureHeight(0)
		{}
        virtual ~CBKScreen_Shared()
          = default;

		virtual bool BKSS_GetReverseFlag()
		{
			return false;
		}

		virtual bool BKSS_IsFullScreenMode()
		{
			return m_bFullScreen;
		}
};

using BKSCREENHANDLE = CBKScreen_Shared *;

