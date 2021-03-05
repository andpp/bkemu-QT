#pragma once

#include "resource.h"       // основные символы
#include "Config.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// Диалоговое окно CBKPaletteDlg


class CBKPaletteDlg : public CDialogEx
{
		DECLARE_DYNAMIC(CBKPaletteDlg)

		CMFCColorButton m_btnPalMonoBK;
		CMFCColorButton m_btnPalMonoWH;

		CMFCColorButton m_arPalMonoAdapt[4];
		CMFCColorButton m_arPalColor[16][4];


	public:
		CBKPaletteDlg(CWnd *pParent = nullptr);   // стандартный конструктор
		virtual ~CBKPaletteDlg() override;

// Данные диалогового окна
		enum { IDD = IDD_BKPALETTE_DLG };

	protected:
		COLORREF BGRtoColorref(DWORD bgr);
		DWORD ColorrefToBGR(COLORREF col);

	protected:
		virtual void DoDataExchange(CDataExchange *pDX) override;    // поддержка DDX/DDV
		virtual BOOL OnInitDialog() override;
		virtual void OnOK() override;
		afx_msg void OnBnClickedButtonResetMonopal();
		afx_msg void OnBnClickedButtonResetMonoadaptPal();
		afx_msg void OnBnClickedButtonResetColorpal();
		afx_msg void OnCbnSelchangeSettComboColormono();
		DECLARE_MESSAGE_MAP()
};
