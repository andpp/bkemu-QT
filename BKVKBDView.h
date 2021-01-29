#pragma once

#include "BKKbdButn.h"
//#include "resource.h"
#include "Config.h"
#include <QWidget>

// CBKVKBDView

class CBKVKBDView : public QWidget
{
//		DECLARE_DYNAMIC(CBKVKBDView)

	protected:
//		CRgn m_rgnRes;
//		CBrush m_br;
		CBKKbdButn *m_pKbdButn;
		UINT m_nViewID;

		static const char LatShiftDigitTable[10];
		static const char RusShiftDigitTable[10];
		static const char RusAlphaBetTable[26];
		static const char RusAlphaBetTableShift[26];

        QImage *kbdSoftPressed;


	public:
		CBKVKBDView(UINT nID = IDB_BITMAP_SOFT);
		virtual ~CBKVKBDView() override;
		int         SetKeyboardView(UINT nID);
		bool        TranslateKey(int key, bool bExtended, uint16_t *nScanCode, uint16_t *nInt);
		uint8_t     GetUniqueKeyNum(uint16_t nScanCode);
		inline void SetAR2Status(bool bB)
		{
			m_pKbdButn->SetAR2Status(bB);
		}
		inline bool GetAR2Status()
		{
			return m_pKbdButn->GetAR2Status();
		}
		inline void SetShiftStatus(bool bB)
		{
			m_pKbdButn->SetShiftStatus(bB);
		}
		inline bool GetShiftStatus()
		{
			return m_pKbdButn->GetShiftStatus();
		}
		inline void SetSUStatus(bool bB)
		{
			m_pKbdButn->SetSUStatus(bB);
		}
		inline bool GetSUStatus()
		{
			return m_pKbdButn->GetSUStatus();
		}
		inline void SetCapitalStatus(bool bB)
		{
			m_pKbdButn->SetCapitalStatus(bB);
		}
		inline bool GetCapitalStatus()
		{
			return m_pKbdButn->GetCapitalStatus();
		}
		inline void SetXLatStatus(bool bB)
		{
			m_pKbdButn->SetXLatStatus(bB);
		}
		inline bool GetXLatStatus()
		{
			return m_pKbdButn->GetXLatStatus();
		}

		void SetKeyboardStatus(STATUS_FIELD pane, bool set);
		bool GetKeyboardStatus(STATUS_FIELD pane);

		void OutKeyboardStatus(STATUS_FIELD pane);

	protected:
		int             CreateKeyboard();
		int             LatModeTranslation(int key);
		int             RusModeTranslation(int key);
//		virtual void    OnAfterFloat() override;

	protected:
    public:
        int OnCreate();
        void OnSize(UINT nType, int cx, int cy);
        void OnPaint();
//		bool OnEraseBkgnd(CDC *pDC);
        void OnDestroy();
};


