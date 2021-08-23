// TapeControlDlg.cpp : implementation file
//

#include "pch.h"
//#include "afxdialogex.h"
//#include "resource.h"
#include "TapeControlDlg.h"
#include "Config.h"

#include <QFileDialog>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTapeControlDlg dialog

CTapeControlDlg::CTapeControlDlg(QObject *pParent, CTape *pTape)
    : QObject((QObject *)pParent)
	, m_pTape(pTape)
	, m_bAutoBeginDetection(FALSE)
	, m_bAutoEnddetection(FALSE)
	, m_strTapePath(g_Config.m_strTapePath)
{
    InitParam();
}

//void CTapeControlDlg::DoDataExchange(CDataExchange *pDX)
//{
//	CDialogEx::DoDataExchange(pDX);
//	DDX_Check(pDX, IDC_CHECK_TC_AUTOBEGIN, m_bAutoBeginDetection);
//	DDX_Check(pDX, IDC_CHECK_TC_AUTOEND, m_bAutoEnddetection);
//}

//BEGIN_MESSAGE_MAP(CTapeControlDlg, CDialogEx)
//	ON_BN_CLICKED(IDC_CHECK_TC_RECORD, &CTapeControlDlg::OnTcRecord)
//	ON_BN_CLICKED(IDC_BUTTON_TC_STOP, &CTapeControlDlg::OnTcStop)
//	ON_BN_CLICKED(IDC_BUTTON_TC_PLAY, &CTapeControlDlg::OnTcPlay)
//	ON_BN_CLICKED(IDC_CHECK_TC_AUTOBEGIN, &CTapeControlDlg::OnBnClickedTcAutobegin)
//	ON_BN_CLICKED(IDC_CHECK_TC_AUTOEND, &CTapeControlDlg::OnBnClickedTcAutoend)
//	ON_WM_TIMER()
//	ON_WM_DESTROY()
//	ON_WM_SIZE()
//	ON_WM_GETMINMAXINFO()
//END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTapeControlDlg message handlers

//BOOL CTapeControlDlg::OnInitDialog()
//{
//	CDialogEx::OnInitDialog();
//	m_iconRecordActive = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_TCTRL_RECORD));
//	m_iconRecordPassive = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_TCTRL_RECORD_DARK));
//	m_iconRecordStop = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_TCTRL_STOP));
//	m_iconRecordStart = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_TCTRL_PLAY));
//	m_iconRecordPause = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_TCTRL_PAUSE));
//	(reinterpret_cast<CButton *>(GetDlgItem(IDC_CHECK_TC_RECORD)))->SetIcon(m_iconRecordPassive);
//	(reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_TC_STOP)))->SetIcon(m_iconRecordStop);
//	(reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_TC_PLAY)))->SetIcon(m_iconRecordStart);
//	InitParam();
//	SetTimer(BKTIMER_TAPECTRL, 300, nullptr);
//	m_Resizer.Init(this);
//	return TRUE; // return TRUE unless you set the focus to a control
//	// EXCEPTION: OCX Property Pages should return FALSE
//}


void CTapeControlDlg::InitParam()
{
	m_bAutoBeginDetection = g_Config.m_bTapeAutoBeginDetection;
	m_bAutoEnddetection = g_Config.m_bTapeAutoEnsDetection;
//	UpdateData(FALSE);
}

void CTapeControlDlg::CreateActions(QToolBar *tb)
{
    m_iconRecordStop    = QIcon(":tape/Stop");
    m_iconRecordStart   = QIcon(":tape/Play");
    m_iconRecordPause   = QIcon(":tape/Pause");
    m_iconRecordActive  = QIcon(":tape/Record");
    m_iconRecordPassive = QIcon(":tape/RecordDark");

    QAction *act;
    m_pActRecord = act = new QAction(m_iconRecordPassive, QString("Record"), this);
    connect(act, &QAction::triggered, this, &CTapeControlDlg::OnTcRecord);
    tb->addAction(act);

    m_pActPlay = act = new QAction(m_iconRecordStart, QString("Play"), this);
    connect(act, &QAction::triggered, this, &CTapeControlDlg::OnTcPlay);
    tb->addAction(act);

    m_pActStop = act = new QAction(m_iconRecordStop, QString("Stop"), this);
    connect(act, &QAction::triggered, this, &CTapeControlDlg::OnTcStop);
    tb->addAction(act);

    m_pActAutobegin = act = new QAction(QString("Autobegin"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CTapeControlDlg::OnBnClickedTcAutobegin);
    tb->addAction(act);

    m_pActAutoend = act = new QAction(QString("Autoend"), this);
    connect(act, &QAction::triggered, this, &CTapeControlDlg::OnBnClickedTcAutoend);
    act->setCheckable(true);
    tb->addAction(act);
    StartTimer();
}

// блокируем действия кнопок Enter и Esc
//void CTapeControlDlg::OnOK()
//{
//}


//void CTapeControlDlg::OnCancel()
//{
//}

//void CTapeControlDlg::OnSize(UINT nType, int cx, int cy)
//{
//	CDialogEx::OnSize(nType, cx, cy);
//	// Resize layout to current window size
//	m_Resizer.OnSize(cx, cy);
//}

void CTapeControlDlg::OnTcRecord()
{
//	UpdateData(TRUE);
	m_pTape->SetWaveLoaded(false);
//	auto pBtn = static_cast<CButton *>(GetDlgItem(IDC_CHECK_TC_RECORD));

//	if (IsDlgButtonChecked(IDC_CHECK_TC_RECORD))
    if (!m_bTcRecord)
	{
        m_pActRecord->setIcon(m_iconRecordActive);
//		pBtn->SetIcon(m_iconRecordActive);
		m_pTape->StartRecord(!!m_bAutoBeginDetection, !!m_bAutoEnddetection);
	}
	else
	{
//		pBtn->SetIcon(m_iconRecordPassive);
        m_pActRecord->setIcon(m_iconRecordPassive);

		m_pTape->StopRecord();
		ShowSaveDialog();
	}

    m_bTcRecord = !m_bTcRecord;

//	GetParent()->SetFocus();
}


void CTapeControlDlg::OnTcStop()
{
	if (m_pTape->IsWaveLoaded() && m_pTape->IsPlaying())
	{
		m_pTape->StopPlay();
		m_pTape->ResetPlayWavePos();
//		(reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_TC_PLAY)))->SetIcon(m_iconRecordStart);
        m_pActPlay->setIcon(m_iconRecordStart);
	}

//	auto pBtn = static_cast<CButton *>(GetDlgItem(IDC_CHECK_TC_RECORD));

    if(m_bTcRecord)
	{
        m_bTcRecord = false;
//		pBtn->SetCheck(FALSE);
	}

	if (m_pTape->IsRecording())
	{
//		pBtn->SetIcon(m_iconRecordPassive);
        m_pActRecord->setIcon(m_iconRecordPassive);
		m_pTape->StopRecord();
		ShowSaveDialog();
	}
}


void CTapeControlDlg::OnTcPlay()
{
	if (m_pTape->IsWaveLoaded())
	{
//		auto pBtn = static_cast<CButton *>(GetDlgItem(IDC_BUTTON_TC_PLAY));

		if (m_pTape->IsPlaying())
		{
			m_pTape->StopPlay();
//			pBtn->SetIcon(m_iconRecordStart);
            m_pActPlay->setIcon(m_iconRecordStart);
		}
		else
		{
			m_pTape->StartPlay();
//			pBtn->SetIcon(m_iconRecordPause);
            m_pActPlay->setIcon(m_iconRecordPause);
		}
	}
}

void CTapeControlDlg::OnTimer()
{
//	if (nIDEvent == BKTIMER_TAPECTRL)
    {
        if (m_pTape)
        {
//			auto pBtn = static_cast<CButton *>(GetDlgItem(IDC_CHECK_TC_RECORD));

//			if (pBtn->GetCheck() && !m_pTape->IsRecording())
            if (m_bTcRecord && !m_pTape->IsRecording())
            {
//				pBtn->SetCheck(FALSE);
                m_bTcRecord = false;
                OnTcRecord();
            }

            if (!m_pTape->IsWaveLoaded())
            {
//				(reinterpret_cast<CButton *>(GetDlgItem(IDC_BUTTON_TC_PLAY)))->SetIcon(m_iconRecordStart);
                m_pActPlay->setIcon(m_iconRecordStart);
//				GetDlgItem(IDC_BUTTON_TC_PLAY)->EnableWindow(FALSE);
                m_pActPlay->setEnabled(true);
            }
        }
    }
//	else
//	{
//		CDialogEx::OnTimer(nIDEvent);
//	}
}


void CTapeControlDlg::ShowSaveDialog()
{
	CString strFilterTape(MAKEINTRESOURCE(IDS_FILEFILTER_TAPE_SAVE));
	CString strWaveExt(MAKEINTRESOURCE(IDS_FILEEXT_WAVE));
	CString strTapeExt(MAKEINTRESOURCE(IDS_FILEEXT_TAPE));
	CString strBinExt(MAKEINTRESOURCE(IDS_FILEEXT_BINARY));
//	CFileDialog dlg(FALSE, strTapeExt, nullptr,
//	                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
//	                strFilterTape, this);
//	dlg.GetOFN().lpstrInitialDir = m_strTapePath;

//	if (dlg.DoModal() == IDOK)
    CString strFilePath = QFileDialog::getSaveFileName(nullptr,"Save Tape", g_Config.m_strTapePath, strFilterTape);

    if (!strFilePath.isNull())
    {
//		CString strFilePath = dlg.GetPathName();
		CString strExt = ::GetFileExt(strFilePath);

		if (!strExt.CompareNoCase(strWaveExt))
		{
			m_pTape->SaveWaveFile(strFilePath);
		}
		else if (!strExt.CompareNoCase(strBinExt))
		{
			TAPE_FILE_INFO tfi;
			memset(&tfi, 255, sizeof(TAPE_FILE_INFO));
			m_pTape->GetWaveFile(&tfi);
			m_pTape->SaveBinFile(strFilePath, &tfi);
		}
		else if (!strExt.CompareNoCase(strTapeExt))
		{
			m_pTape->SaveMSFFile(strFilePath);
		}
	}

//	m_strTapePath = ::GetFilePath(dlg.GetPathName());
}


void CTapeControlDlg::PressRecordButton()
{
//	SendDlgItemMessage(IDC_CHECK_TC_RECORD, BM_CLICK);
}

//void CTapeControlDlg::OnDestroy()
//{
//	if (m_iconRecordActive)
//	{
//		DestroyIcon(m_iconRecordActive);
//	}

//	if (m_iconRecordPassive)
//	{
//		DestroyIcon(m_iconRecordPassive);
//	}

//	if (m_iconRecordStop)
//	{
//		DestroyIcon(m_iconRecordStop);
//	}

//	if (m_iconRecordStart)
//	{
//		DestroyIcon(m_iconRecordStart);
//	}

//	if (m_iconRecordPause)
//	{
//		DestroyIcon(m_iconRecordPause);
//	}

//	CDialogEx::OnDestroy();
//}


//CSize CTapeControlDlg::GetMinSize()
//{
//	CSize size;
//	MINMAXINFO mmi;
//	m_Resizer.GetMinMaxInfo(&mmi);
//	size.cx = mmi.ptMinTrackSize.x;
//	size.cy = mmi.ptMinTrackSize.y;
//	return size;
//}


//void CTapeControlDlg::OnGetMinMaxInfo(MINMAXINFO *lpMMI)
//{
//	m_Resizer.GetMinMaxInfo(lpMMI);
//}


void CTapeControlDlg::OnBnClickedTcAutobegin()
{
//	UpdateData(TRUE);
	g_Config.m_bTapeAutoBeginDetection = !!m_bAutoBeginDetection;
}


void CTapeControlDlg::OnBnClickedTcAutoend()
{
//	UpdateData(TRUE);
	g_Config.m_bTapeAutoEnsDetection = !!m_bAutoEnddetection;
}


