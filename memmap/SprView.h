#pragma once
if 0
#include "SprScr.h"

// CSprView

class CSprView : public CScrollView
{
        DECLARE_DYNAMIC(CSprView)

		CSprScr *m_pScreen;
		int m_cx, m_cy; // размеры viewportа

	public:
		CSprView();
		virtual ~CSprView() override;
		// сквозные функции, просто передают параметры дальше
		inline void SetParameters(uint8_t *buffer, size_t size, int scale)
		{
			m_pScreen->SetParameters(buffer, size, scale);
		}

		inline void SetScale(int scale)
		{
			m_pScreen->SetScale(scale);
		}

		inline void SetPalette(int palette)
		{
			m_pScreen->SetPalette(palette);
		}

		inline int GetPalette()
		{
			return m_pScreen->GetPalette();
		}

		inline void SetAdaptMode(bool bFlag)
		{
			m_pScreen->SetAdaptMode(bFlag);
		}

		inline bool IsAdaptMode()
		{
			return m_pScreen->IsAdaptMode();
		}

		inline void SetColorMode(bool bColorMode = true)
		{
			m_pScreen->SetColorMode(bColorMode);
		}

		inline bool IsColorMode()
		{
			return m_pScreen->IsColorMode();
		}

	protected:
		virtual void OnDraw(CDC *pDC) override;
		virtual BOOL PreCreateWindow(CREATESTRUCT &cs) override;
		afx_msg void OnSize(UINT nType, int cx, int cy);
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnDestroy();
		DECLARE_MESSAGE_MAP()
};
#endif
