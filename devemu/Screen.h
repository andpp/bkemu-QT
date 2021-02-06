// Screen.h: Файл заголовка
//

#pragma once
#include "LockVarType.h"
#include "Screen_Shared.h"
#include "Screen_Sizes.h"
#include "Config.h"

#include <QWidget>

#include <mutex>

#if (BK_USE_CONVEYORTHREAD)
#include <queue>
#include <thread>
#endif

#define afx_msg

// CScreen
// вывод в лог времени фреймов
#define DBG_OUT_SCREENFRAMELENGTH 0

class CScreen: QObject
{
    Q_OBJECT

//		BKSCREENHANDLE      m_pscrSharedFunc;
//		HMODULE             m_hModule;
//		using GETBKSCR = BKSCREENHANDLE(WINAPI *)();

		union BCOLOR
		{
			uint32_t dw;
			uint8_t bb[sizeof(uint32_t)];
		};

		static const int    BK_SCREEN_WIDTH, BK_SCREEN_HEIGHT;
		// 16kPage
		uint8_t            *m_pBuffer;
		size_t              m_nBufSize;

		bool                m_bReverseScreen; // флаг перевёрнутого экрана.
		/* DrawDIB в отличие всех остальных рисует вверх ногами.
		или наоборот, все остальные в отличие от DrawDIB рисуют вверх ногами. Поэтому для определения
		направления введён этот флаг. */

	protected:
        QWindow *m_pwndParent;       // указатель на родительское окно (View)
        int                 m_nFrame;           // счётчик фреймов
		int                 m_nCurFPS;          // текущее значение FPS
		std::mutex          m_mutFPS;
		uint8_t             m_nOfs;             // смещение экрана. Используется только в отладочном методе прорисовки экрана
		int                 m_nPaletteNum_m256; // номер палитры умноженный на 256 для быстроты
		bool                m_bSmoothing;       // флаг использования сглаживания при увеличении картинки
		bool                m_bColorMode;       // флаг работы в цветном режиме
		bool                m_bAdapt;           // флаг работы в ч/б адаптивном режиме
		bool                m_bExtended;        // флаг расширенного режима.
		bool                m_bLuminoforeEmul;  // флаг режима плавного затухания люминофора
		LockVarType         m_lockChangeMode;   // флаг, пока переключаются режимы экрана,чтобы в это время не рисовался экран
		LockVarType         m_lockPrepare;
		LockVarType         m_lockBusy;

		uint32_t           *m_pColTable32;
		uint32_t           *m_pMonoTable32;
		uint32_t           *m_pTexBits;     // указатель на выделенную память текстуры. здесь с ним ничего делать не нужно.
		uint32_t            m_nBitBufferSize;
		int                 m_nCurrentScr;  // текущий номер экрана из массива экранов

		ScrsParams          m_scrParam;
#if (_DEBUG && DBG_OUT_SCREENFRAMELENGTH)
		DWORD m_nTickCounter;
		FILE *dbgFile;
#endif // _DEBUG

#if (BK_USE_CONVEYORTHREAD)
		volatile int        m_nFreeScreens; // количество свободных экранов
		std::thread         m_DrawThread;
		volatile bool       m_bThreadTerminate;   // флаг завершения потока
		std::queue<int>     m_vScrns;       // очередь экранов
		std::mutex          m_mutThread;    // мутекс потока. залочен пока поток живой
		std::mutex          m_mutScrBuf;    // мутекс для синхронизации

		bool                StartDrawThread();
		void                StopDrawThread();
		void                DrawThreadFunc();
#endif

		// мышь марсианка
		int                 m_nPointX;
		int                 m_nPointY;
		uint16_t			m_MouseValue; // новые значения параметров, выдаваемых мышью
		bool				m_bMouseMove;
		bool				m_bMouseOutEna; // флаг разрешения выдачи данных мышой
		uint16_t			m_nMouseEnaStrobe; // строб формирования разрешения выдачи



	public:
        CScreen(CONF_SCREEN_RENDER nRenderType);
        CScreen(CONF_SCREEN_RENDER nRenderType, uint8_t *buffer, size_t size); // 16kpage
        virtual ~CScreen() override;

        uint32_t * GetTexBits() { return m_pTexBits; }

		uint16_t			GetMouseStatus();
		void				SetMouseStrobe(uint16_t data);

		void                ChangeBuffer(uint8_t *buffer, size_t size) // 16kpage
		{
			m_pBuffer = buffer;
			m_nBufSize = size;
		}
        void                InitVars(CONF_SCREEN_RENDER nRenderType);
		void                ClearObjects();

#if (BK_USE_CONVEYORTHREAD)
		void                ReDrawScreen() // 16kpage
		{
			PrepareScreenRGB32(m_pBuffer);
			DrawScreen(m_pTexBits);
		};
		void                PrepareFrame();
		void                _DrawScreen(bool bCheckFPS = false);
		void                DrawScreen(UINT32 *pTexBits, bool bCheckFPS = false);
#else
		void                ReDrawScreen() // 16kpage
		{
			PrepareScreenRGB32(m_pBuffer);
//			DrawScreen();
		};
//		void                DrawScreen(bool bCheckFPS = false);
#endif

		void                PrepareScreenLineByteRGB32(int nLineNum, int nByteNum, uint8_t b);
		void                PrepareScreenLineWordRGB32(int nLineNum, int nByteNum, uint16_t w);
#ifdef _DEBUG
		void                PrepareScreenLineByte_Debug(int nLineNum, int nByteNum);
#endif
		void                PrepareScreenRGB32(uint8_t *ScreenBuffer);
		void                AdjustLayout();
		// Screen own methods
		void                SetRegister(uint16_t w)
		{
			SetOffset(LOBYTE(w));
			SetExtendedMode((w & 01000) == 0);
		}
		inline void         SetOffset(uint8_t nOfs)
		{
			m_nOfs = nOfs;
		}
		inline uint8_t      GetOffset()
		{
			return m_nOfs;
		}
		inline void         SetExtendedMode(bool bFlag)
		{
			m_bExtended = bFlag;
		}
		inline bool         GetExtendedMode()
		{
			return m_bExtended;
		}
		inline void         SetLuminoforeEmuMode(bool bFlag)
		{
			m_bLuminoforeEmul = bFlag;
		}
		inline bool         GetLuminoforeEmuMode()
		{
			return m_bLuminoforeEmul;
		}
//		inline int          GetFPS()
//		{
//			return m_nCurFPS;
//		}
		inline int          GetPalette()
		{
			return (m_nPaletteNum_m256 >> 8);
		}
		void                SetPalette(int palette)
		{
			m_nPaletteNum_m256 = (palette & 0xf) << 8;
		}

		void                SetAdaptMode(bool bFlag)
		{
			m_bAdapt = bFlag;
			InitColorTables();
		}
		inline bool         IsAdaptMode()
		{
			return m_bAdapt;
		}

		void                SetSmoothing(bool bSmoothing);
		inline bool         IsSmoothing()
		{
			return m_bSmoothing;
		}
		void                SetColorMode(bool bColorMode = true);
		inline bool         IsColorMode()
		{
			return m_bColorMode;
		}

//		// захват видео
//		void                SetCaptureStatus(bool bCapture, const CString &strUniq);
//		bool                IsCapture()
//		{
//			return m_bCaptureProcessed;
//		}

//		void                RestoreFS();
//		bool                SetFullScreenMode();
//		bool                SetWindowMode();
//		bool                IsFullScreenMode();

//        inline QWidget        *GetBackgroundWindow()
//		{
//            return nullptr; // this
//		}
//		HBITMAP             GetScreenshot();

		bool                InitColorTables();

	protected:

//		virtual BOOL PreCreateWindow(CREATESTRUCT &cs) override;
//		virtual BOOL PreTranslateMessage(MSG *pMsg) override;

public:
        int OnCreate(/*LPCREATESTRUCT lpCreateStruct*/);
        void OnSize(UINT nType, int cx, int cy);
        BOOL OnEraseBkgnd(CDC *pDC);
        void OnDestroy();
//        void OnTimer(uint * nIDEvent);
        void timerEvent(QTimerEvent *event) override;

        void OnMouseMove(UINT nFlags, CPoint point);
        void OnLButtonDown(UINT nFlags, CPoint point);
        void OnLButtonUp(UINT nFlags, CPoint point);
        void OnRButtonDown(UINT nFlags, CPoint point);
        void OnRButtonUp(UINT nFlags, CPoint point);

//		DECLARE_MESSAGE_MAP()
};


