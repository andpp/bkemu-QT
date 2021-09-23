#pragma once

#include "TapeControlDlg.h"
#include <QToolBar>

// CTapeCtrlView

class CTapeCtrlView : public QToolBar
{
     Q_OBJECT
//		DECLARE_DYNAMIC(CTapeCtrlView)
//    QAction *m_pActSave;
//    QAction *m_pActPlay;
//    QAction *m_pActStop;
//    QAction *m_pActAutobegin;
//    QAction *m_pActAutoend;

	protected:
        CTapeControlDlg m_TapeCtrlDlg;

	public:
		CTapeCtrlView();
		virtual ~CTapeCtrlView() override;
//		virtual void AdjustLayout() override;

        void UpdateToolbar();

		void UpdateTapeControls(bool bEmuLoad = false, bool bEmuSave = false);
		void StartPlayTape();
		void InitParams(CTape *pTape);

	protected:
//		afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
//		afx_msg void OnSize(UINT nType, int cx, int cy);
//		afx_msg void OnPaint();
//		DECLARE_MESSAGE_MAP()
};
