#include "pch.h"
#include "BKMessageBox.h"
//#include "resource.h"

//const CString CBKMessageBox::m_strCaption = _T("BKEmu сообщает");

int CBKMessageBox::Show(const CString &strText, UINT nType, UINT nIDHelp)
{
    (void)nIDHelp;
    CString strCaption(MAKEINTRESOURCE(IDS_MSGBOX_CAPTION));
//	return MessageBox(m_hwnd, strText.GetString(), strCaption.GetString(), nType | MB_SETFOREGROUND);
//    QMessageBox::StandardButton reply;

    int btn1 = 0, btn2 = 0, btn3 = 0;

    if ((nType & MB_YESNOCANCEL) == MB_YESNOCANCEL) {
        btn1 = QMessageBox::Yes;
        btn2 = QMessageBox::No;
        btn3 = QMessageBox::Cancel;
    } else if ((nType & MB_ABORTRETRYIGNORE) == MB_ABORTRETRYIGNORE) {
        btn1 = QMessageBox::Abort;
        btn2 = QMessageBox::Retry;
        btn3 = QMessageBox::Ignore;
    } else if (nType == MB_OK) {
        btn1 = QMessageBox::Ok;
    }

    if ((nType & MB_ICONWARNING) == MB_ICONWARNING) {
        return QMessageBox::warning(NULL, strCaption, strText, btn1, btn2, btn3);
    }
    if ((nType & MB_ICONSTOP) == MB_ICONSTOP) {
        return QMessageBox::critical(NULL, strCaption, strText, btn1, btn2, btn3);
    }
    return QMessageBox::question(NULL, strCaption, strText, btn1, btn2, btn3);
}

int CBKMessageBox::Show(LPCTSTR lpszText, UINT nType, UINT nIDHelp)
{
	// return AfxMessageBox(lpszText, nType | MB_SETFOREGROUND, nIDHelp);
//	return MessageBox(m_hwnd, lpszText, strCaption.GetString(), nType | MB_SETFOREGROUND);
    return Show(CString(lpszText), nType, nIDHelp);
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

