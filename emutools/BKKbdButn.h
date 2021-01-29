#pragma once
//#include <atlimage.h>
#include "pch.h"
#include <QGraphicsView>

enum class BKKeyType : int
{
	RESERVED,
	REGULAR,
	LSHIFT,
	RSHIFT,
	CTRL,
	ALT,
	ZAGL,
	STR,
	STOP,
	ENDARRAY
};

struct BKKey
{
	BKKeyType nType;
	int x1;
	int y1;
	int x2;
	int y2;
	uint8_t nScanCode;
	uint8_t nInterrupt;
	uint8_t nUniqueNum;
};


class CBKKbdButn : public QWidget
{
	protected:
		HWND    m_hwndParent; // хэндл родительского окна
        QWidget   *m_cwndParent; // указатель на родительское окно

        QImage  m_Img;
        QImage  m_Img_p;   // Для битмапа-нажиматора
        QPainter  m_ImgScr;

		int     m_cx;
		int     m_cy;
		int     m_imgW;
		int     m_imgH;
		int     m_nIdx;

		int     m_nAR2Index;
		int     m_nSUIndex;
		int     m_nLShiftIndex;
		int     m_nRShiftIndex;
		int     m_nArraySize;

		// Клавиатура
		bool    m_bAR2Pressed;     // флаг нажатия АР2
		bool    m_bSUPressed;      // флаг нажатия СУ
		bool    m_bShiftPressed;   // флаг нажатия шифта
		bool    m_bRShiftPressed;  // флаг нажатия правого шифта
		bool    m_bZaglPressed;    // true - загл (капслок выключен), false - стр (капслок включён)
		bool    m_bXlatMode;       // true - рус, false - лат

		bool    m_bControlKeyPressed;
		bool    m_bRegularKeyPressed;

		BKKey  *m_pBKKeyboardArray;

		static const BKKey m_ButnKbdKeys[];
		static const BKKey m_PlenKbdKeys[];

	public:
		CBKKbdButn(UINT nID = 0);
		virtual ~CBKKbdButn() override;
//		virtual BOOL DestroyWindow() override;
		void AdjustLayout();
		void SetID(UINT nID);
		inline int GetWidth()
		{
			return m_imgW;
		}
		inline int GetHeihgt()
		{
			return m_imgH;
		}

		inline void SetAR2Status(bool bB)
		{
			m_bAR2Pressed = bB;
		}
		inline bool GetAR2Status()
		{
			return m_bAR2Pressed;
		}
		inline void SetShiftStatus(bool bB)
		{
			m_bShiftPressed = bB;
			// правый шифт не трогаем, чтобы не нажимались они одновременно
		}
		inline bool GetShiftStatus()
		{
			return m_bShiftPressed || m_bRShiftPressed;
		}
		inline void SetSUStatus(bool bB)
		{
			m_bSUPressed = bB;
		}
		inline bool GetSUStatus()
		{
			return m_bSUPressed;
		}
		inline void SetCapitalStatus(bool bB)
		{
			m_bZaglPressed = bB;
		}
		inline bool GetCapitalStatus()
		{
			return m_bZaglPressed;
		}
		inline void SetXLatStatus(bool bB)
		{
			m_bXlatMode = bB;
		}
		inline bool GetXLatStatus()
		{
			return m_bXlatMode;
		}

		uint8_t GetUniqueKeyNum(uint8_t nScancode);

	protected:
//		virtual BOOL PreCreateWindow(CREATESTRUCT &cs) override;
		int GetKeyIndex(int x, int y);
		int GetKeyIndexById(BKKeyType nType);
		int GetArraySize();
		void _FocusPressedkey(int nIdx);
		uint8_t TranslateScanCode(uint8_t nScanCode);
		void ControlKeysUp();
		void ClearObj();

        void paintEvent(QPaintEvent* event) override;

//		afx_msg LRESULT OnRealKeyDown(WPARAM wParam, LPARAM lParam);
//		afx_msg LRESULT OnRealKeyUp(WPARAM wParam, LPARAM lParam);
//		afx_msg void OnPaint();
//		afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
//		afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
//		afx_msg BOOL OnEraseBkgnd(CDC *pDC);
//		afx_msg void OnSize(UINT nType, int cx, int cy);
//		DECLARE_MESSAGE_MAP()
};

