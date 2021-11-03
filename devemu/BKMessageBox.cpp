#include "pch.h"
#include "BKMessageBox.h"
#include <QThread>
//#include "resource.h"

//const CString CBKMessageBox::m_strCaption = _T("BKEmu сообщает");

CBKMessageBox::CBKMessageBox(QObject* parent) : QObject(parent)
{
    //m_hwnd = AfxGetMainWnd()->GetSafeHwnd();
    qRegisterMetaType<CString>();
};


int CBKMessageBox::Show(const CString &strText, UINT nType, UINT nIDHelp)
{
    int res = 0;
    moveToThread(qApp->thread());

    if (QThread::currentThread() != qApp->thread()) { // no GUI thread
      QMetaObject::invokeMethod(this, "Show_", Qt::BlockingQueuedConnection,
                                Q_RETURN_ARG(int, res),
                                Q_ARG(const CString&, strText),
                                Q_ARG(uint, nType),
                                Q_ARG(uint, nIDHelp));
    } else { // in GUI thread, direct call
      res = Show_(strText, nType, nIDHelp);
    }

    return res;
}

int CBKMessageBox::Show(const CString &strTitle, const CString &strText, UINT nType, UINT nIDHelp)
{
    int res = 0;
    moveToThread(qApp->thread());

    if (QThread::currentThread() != qApp->thread()) { // no GUI thread
      QMetaObject::invokeMethod(this, "TShow_", Qt::BlockingQueuedConnection,
                                Q_RETURN_ARG(int, res),
                                Q_ARG(const CString&, strTitle),
                                Q_ARG(const CString&, strText),
                                Q_ARG(uint, nType),
                                Q_ARG(uint, nIDHelp));
    } else { // in GUI thread, direct call
      res = TShow_(strTitle, strText, nType, nIDHelp);
    }

    return res;
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

Q_INVOKABLE int CBKMessageBox::Show_(const CString &strText, const uint nType, const uint nIDHelp)
{
    CString strCaption(MAKEINTRESOURCE(IDS_MSGBOX_CAPTION));
    return ShowDialog(strCaption, strText, nType, nIDHelp);
}

Q_INVOKABLE int CBKMessageBox::TShow_(const CString &strTitle, const CString &strText, const uint nType, const uint nIDHelp)
{
    return ShowDialog(strTitle, strText, nType, nIDHelp);
}


int CBKMessageBox::ShowDialog(const CString &strTitle, const CString &strText, const uint nType, const uint nIDHelp)
{
    (void)nIDHelp;
    int res;

    int btn1 = 0, btn2 = 0, btn3 = 0;

    if ((nType & MB_YESNOCANCEL) != 0) {
        btn1 = nType & QMessageBox::Yes;
        btn2 = nType & QMessageBox::No;
        btn3 = nType & QMessageBox::Cancel;
    } else if ((nType & MB_ABORTRETRYIGNORE) != 0) {
        btn1 = nType & QMessageBox::Abort;
        btn2 = nType & QMessageBox::Retry;
        btn3 = nType & QMessageBox::Ignore;
    } else if (nType == MB_OK) {
        btn1 = QMessageBox::Ok;
    }

    if ((nType & MB_ICONWARNING) == MB_ICONWARNING) {
        res = QMessageBox::warning(NULL, strTitle, strText, btn1, btn2, btn3);
    } else if ((nType & MB_ICONSTOP) == MB_ICONSTOP) {
        res = QMessageBox::critical(NULL, strTitle, strText, btn1, btn2, btn3);
    } else {
        res = QMessageBox::question(NULL, strTitle, strText, btn1, btn2, btn3);
    }


    return res;
}

CBKMessageBox g_BKMsgBox;

