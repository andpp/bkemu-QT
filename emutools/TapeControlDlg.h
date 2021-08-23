// TapeControlDlg.h : header file
//

#pragma once
//#include "afxcmn.h"
//#include "GFResizeDialogHelper.h"
#include "pch.h"

#include "Tape.h"
#include <QAction>
#include <QToolBar>
#include <QTimerEvent>

/////////////////////////////////////////////////////////////////////////////
// CTapeControlDlg dialog


class CTapeControlDlg : public QObject
{
     Q_OBJECT
//		GFResizeDialogHelper m_Resizer;
		CTape              *m_pTape;
//		HICON               m_iconRecordActive, m_iconRecordPassive, m_iconRecordStop;
//		HICON               m_iconRecordStart, m_iconRecordPause;
		BOOL                m_bAutoBeginDetection;
		BOOL                m_bAutoEnddetection;
		CString             m_strTapePath;

        QIcon m_iconRecordStop;
        QIcon m_iconRecordStart;
        QIcon m_iconRecordPause;
        QIcon m_iconRecordActive;
        QIcon m_iconRecordPassive;

        bool m_bTcRecord;
        bool m_bTcStop;

        int m_nTimer;

        inline void         StartTimer()
        {
            m_nTimer = startTimer(300);  // запустить таймер
        }

        inline void         StopTimer()
        {
            killTimer(m_nTimer);      // остановить таймер
        }

        inline void timerEvent(QTimerEvent *event) override {
            if (event->timerId() == m_nTimer) OnTimer();
        }

    protected:
        void                ShowSaveDialog();

		// Construction
	public:

        QAction *m_pActRecord;
        QAction *m_pActPlay;
        QAction *m_pActStop;
        QAction *m_pActAutobegin;
        QAction *m_pActAutoend;

        CTapeControlDlg(QObject *pParent = nullptr, CTape *pTape = nullptr);    // standard constructor
        ~CTapeControlDlg() { StopTimer(); }

        void CreateActions(QToolBar *tb);

		void                SetTape(CTape *pTape)
		{
			m_pTape = pTape;
		}
		CTape              *GetTape()
		{
			return m_pTape;
		}
		void                RunPlay()
		{
			OnTcPlay();
		}
		void                PressRecordButton();
		void                InitParam();
//		CSize               GetMinSize();
// Dialog Data
//		enum { IDD = IDD_TAPE_CONTROL };


// Overrides
		// ClassWizard generated virtual function overrides
	protected:
//		virtual void DoDataExchange(CDataExchange *pDX) override;    // DDX/DDV support


// Implementation
    protected:

    public:
		// Generated message map functions
//		virtual BOOL OnInitDialog() override;
//		virtual void OnOK() override;
//		virtual void OnCancel() override;
        void OnTcRecord();
        void OnTcStop();
        void OnTcPlay();
        void OnTimer();
//        void OnDestroy();
//        void OnSize(UINT nType, int cx, int cy);
//		void OnGetMinMaxInfo(MINMAXINFO *lpMMI);
        void OnBnClickedTcAutobegin();
        void OnBnClickedTcAutoend();
//		DECLARE_MESSAGE_MAP()
};
