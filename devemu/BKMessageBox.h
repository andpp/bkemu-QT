// BKMessageBox.h: interface for the CBKMessageBox class.
//

#pragma once
//#include "resource.h"
#include "pch.h"
#include <QMessageBox>

#define MB_YESNOCANCEL (QMessageBox::Yes|QMessageBox::No|QMessageBox::Cancel)
#define MB_DEFBUTTON2  (QMessageBox::Default)
#define MB_ABORTRETRYIGNORE (QMessageBox::Abort|QMessageBox::Retry|QMessageBox::Ignore)
#define MB_OK QMessageBox::Ok

#define IDIGNORE QMessageBox::Ignore
#define IDRETRY QMessageBox::Retry
#define IDABORT QMessageBox::Abort
#define IDNO QMessageBox::No
#define IDYES QMessageBox::Yes

#define MB_ICONSTOP QMessageBox::Icon::Critical
#define MB_ICONWARNING QMessageBox::Icon::Warning


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
		int Show(CString strText, UINT nType = MB_OK, UINT nIDHelp = 0);
		int Show(LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0);
		int Show(UINT strID, UINT nType = MB_OK, UINT nIDHelp = 0);
};

extern CBKMessageBox g_BKMsgBox;
