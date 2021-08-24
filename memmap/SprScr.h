#pragma once
#if 0
#include <vfw.h>

class CSprScr : public CWnd
{
        DECLARE_DYNAMIC(CSprScr)

		uint8_t *m_pBuffer;
		size_t  m_nBufSize;

		int             m_cx, m_cy;     // размеры окна viewporta, с учётом масштабирования
		int             m_orig_cx, m_orig_cy;   // размеры Картинки без учёта масштабирования
		int             m_nScale;       // текущее масштабирование
		int             m_PaletteNum;

		bool            m_bColorMode;   // флаг цветного режима
		bool            m_bAdapt;       // флаг адаптивного ЧБ режима

		bool            m_bBusy;        // флаг, что занято выводом на экран
		bool            m_bChangeMode;  // флаг, что занято изменением режимов, как правило - размера

		uint32_t *m_pColTable32;
		uint32_t *m_pMonoTable32;
		uint32_t *m_bits;

		HWND    m_hwndScreen;  // Screen View window handle
        HDRAWDIB m_hdd;
		HBITMAP m_hbmp;
        BITMAPINFO m_bmpinfo;

	public:
		CSprScr();
		virtual ~CSprScr() override;
		void DrawScreen();
		void ReDrawScreen();
		void AdjustLayout(int cx, int cy, int ysp);

		void SetParameters(uint8_t *buffer, size_t size, int scale)
		{
			m_pBuffer = buffer;
			m_nBufSize = size;
			SetScale(scale);
		}

		inline size_t GetBufferSize()
		{
			return m_nBufSize;
		}

		void SetScale(int scale);

		inline int GetScale()
		{
			return m_nScale;
		}

		void SetPalette(int palette);

		inline int GetPalette()
		{
			return m_PaletteNum;
		}

		void SetAdaptMode(bool bFlag);

		inline bool IsAdaptMode()
		{
			return m_bAdapt;
		}

		void SetColorMode(bool bColorMode = true);

		inline bool IsColorMode()
		{
			return m_bColorMode;
		}

	protected:
		void ScreenView_Init();
		void ScreenView_Done();
		void InitColorTables();
		void PrepareScreenRGB32();
		void CalcOrigSize();
        virtual BOOL PreCreateWindow(CREATESTRUCT &cs) override;
        afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
        afx_msg void OnDestroy();
        afx_msg BOOL OnEraseBkgnd(CDC *pDC);
        DECLARE_MESSAGE_MAP()
};

#endif
