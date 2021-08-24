#pragma once

#if 0
#include "SprView.h"

// CSprWnd

class CSprWnd : public CFrameWnd
{
		DECLARE_DYNAMIC(CSprWnd)

		uint8_t *m_pFileBuffer; // сюда будет читаться файл, из него будет формироваться текстура, чтобы вывести её DibDrawом
		size_t m_nFileLen;
		int m_cx, m_cy; // размеры viewportа
		int m_orig_cx, m_orig_cy;
		int m_nScale;

		int m_cxBorder;
		int m_cyBorder;
		int m_cxScroll;
		int m_cyScroll;
		int m_cyCaption;
		int m_cyToolbar;
		CSprView *m_pSprView;

		CToolBar m_wndToolBar;
		CComboBox m_comboBoxPalette;
		CComboBox m_comboBoxScale;
		CFont m_ComboboxFont;

	public:
		CSprWnd(uint8_t *buffer, size_t size);
		virtual ~CSprWnd() override;

		inline const size_t GetFileLen()
		{
			return m_nFileLen;
		}

	protected:
		void SetButtonState();

		afx_msg void OnViewColormode();
		afx_msg void OnViewBWMode();
		afx_msg void OnSetPalette();
		afx_msg void OnSetScale();
		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
		afx_msg void OnSize(UINT nType, int cx, int cy);
        DECLARE_MESSAGE_MAP()
};

#endif
