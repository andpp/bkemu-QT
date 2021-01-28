// BKMessageBox.h: interface for the CBKMessageBox class.
//

#pragma once
//#include "resource.h"
#include "pch.h"
#include <QMessageBox>

enum: int {
    IDNOBUTTON  = 0x000000,
    IDYES       = 0x000001,
    IDNO        = 0x000002,
    IDCANCEL    = 0x000004,
    IDIGNORE    = 0x000008,
    IDRETRY     = 0x000010,
    IDABORT     = 0x000020,
    IDOK        = 0x000040
};

#define MB_YESNOCANCEL (IDYES|IDNO|IDCANCEL)
#define MB_DEFBUTTON2  IDNOBUTTON
#define MB_ABORTRETRYIGNORE (IDABORT | IDRETRY | IDIGNORE)
#define MB_OK IDOK

#define MB_ICONSTOP    0x10000
#define MB_ICONWARNING 0x20000


class CBKMessageBox
{
		//const static CString m_strCaption;
        //HWND m_hwnd;
	public:
        CBKMessageBox()
		{
            //m_hwnd = AfxGetMainWnd()->GetSafeHwnd();

		};
		virtual ~CBKMessageBox()
		= default;
        int Show(const CString &strText, UINT nType = MB_OK, UINT nIDHelp = 0);
        int Show(const LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0);
		int Show(UINT strID, UINT nType = MB_OK, UINT nIDHelp = 0);
};

extern CBKMessageBox g_BKMsgBox;
