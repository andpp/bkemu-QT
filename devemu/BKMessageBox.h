// BKMessageBox.h: interface for the CBKMessageBox class.
//

#pragma once
//#include "resource.h"
#include "pch.h"
#include <QMessageBox>
#include <QApplication>

enum: int {
    IDNOBUTTON  = QMessageBox::NoButton,
    IDYES       = QMessageBox::Yes,
    IDNO        = QMessageBox::No,
    IDCANCEL    = QMessageBox::Cancel,
    IDIGNORE    = QMessageBox::Ignore,
    IDRETRY     = QMessageBox::Retry,
    IDABORT     = QMessageBox::Abort,
    IDOK        = QMessageBox::Ok
};

#define MB_YESNOCANCEL (IDYES|IDNO|IDCANCEL)
#define MB_YESNO (IDYES|IDNO)
#define MB_DEFBUTTON2  IDNOBUTTON
#define MB_ABORTRETRYIGNORE (IDABORT | IDRETRY | IDIGNORE)
#define MB_OK IDOK

#define MB_ICONSTOP    0x02 //0x10000
#define MB_ICONWARNING 0x04 //0x20000


class CBKMessageBox : public QObject
{
    Q_OBJECT
		//const static CString m_strCaption;
        //HWND m_hwnd;
	public:
        CBKMessageBox(QObject* parent = 0);
        int Show(const CString &strText, UINT nType = MB_OK, UINT nIDHelp = 0);
        int Show(const LPCTSTR lpszText, UINT nType = MB_OK, UINT nIDHelp = 0);
		int Show(UINT strID, UINT nType = MB_OK, UINT nIDHelp = 0);

     public:
        Q_INVOKABLE int Show_(const CString &strText, const uint nType, const uint nIDHelp);
};

extern CBKMessageBox g_BKMsgBox;
