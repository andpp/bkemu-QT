#include "pch.h"
#include "BKMessageBox.h"
//#include "resource.h"

//const CString CBKMessageBox::m_strCaption = _T("BKEmu сообщает");

int CBKMessageBox::Show(CString strText, UINT nType, UINT nIDHelp)
{
    CString strCaption(MAKEINTRESOURCE(IDS_MSGBOX_CAPTION));
//	return MessageBox(m_hwnd, strText.GetString(), strCaption.GetString(), nType | MB_SETFOREGROUND);
//    QMessageBox::StandardButton reply;
     return QMessageBox::question(NULL, strCaption, strText, nType);
}

int CBKMessageBox::Show(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
{
	// return AfxMessageBox(lpszText, nType | MB_SETFOREGROUND, nIDHelp);
    CString strCaption(MAKEINTRESOURCE(IDS_MSGBOX_CAPTION));
//	return MessageBox(m_hwnd, lpszText, strCaption.GetString(), nType | MB_SETFOREGROUND);
    return QMessageBox::question(NULL, strCaption, CString(lpszText), nType);
}

int CBKMessageBox::Show(UINT strID, UINT nType, UINT nIDHelp)
{
	CString strMessage;
	BOOL bValid = strMessage.LoadString(strID);

	if (!bValid)
	{
        const char *strf = MAKEINTRESOURCE(IDS_MSGBOX_ERRMSG);
        strMessage = CString::asprintf(strf, strID);
	}

    return Show(strMessage, nType, nIDHelp);
}


CBKMessageBox g_BKMsgBox;

