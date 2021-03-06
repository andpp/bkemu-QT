#pragma once

#include <QDialog>
#include <QSlider>
#include <QLabel>

#include "Config.h"

// Диалоговое окно CAYVolPan

class CBKAYVolPan : public QDialog
{
//		DECLARE_DYNAMIC(CBKAYVolPan)
    Q_OBJECT

	public:
        CBKAYVolPan(QWidget *pParent = nullptr);   // стандартный конструктор
		virtual ~CBKAYVolPan() override;

// Данные диалогового окна
#ifdef AFX_DESIGN_TIME
		enum { IDD = IDD_AYVOLPAN_DLG };
#endif

	protected:
        QSlider m_ctrPanA {Qt::Horizontal, this};
        QSlider m_ctrPanB {Qt::Horizontal, this};
        QSlider m_ctrPanC {Qt::Horizontal, this};
        QSlider m_ctrVolA {Qt::Vertical, this};
        QSlider m_ctrVolB {Qt::Vertical, this};
        QSlider m_ctrVolC {Qt::Vertical, this};

        QLabel m_DataLabel {"", this};
		// орингинальные значения, чтобы при отмене вернуть всё как было
		CConfig::AYVolPan_s m_orig;
		// тут будет храниться текущее значение
		CConfig::AYVolPan_s m_curr;
		int m_nVolA, m_nVolB, m_nVolC;		// значения в процентах, для наглядности
		int m_nPanAL, m_nPanBL, m_nPanCL;	// значения в процентах, для наглядности
		int m_nPanAR, m_nPanBR, m_nPanCR;	// значения в процентах, для наглядности
		void Save();

	protected:
//		virtual void DoDataExchange(CDataExchange *pDX) override;    // поддержка DDX/DDV
        BOOL OnInitDialog();
        void OnOK();
        void OnCancel();
        void OnHScroll(QSlider *pSlider);
        void OnVScroll(QSlider *pSlider);
        void UpdateData(bool b);
};
