
// BKMEMDlg.cpp : файл реализации
//

#include "pch.h"
#include "Config.h"
#include "BKMEMDlg.h"
#include "BKMessageBox.h"
#include "Config.h"
#include "Screen.h"
#include "SprWnd.h"
#include "OpenGlView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//IMPLEMENT_DYNAMIC(CBKMEMDlg, CDialogEx)

// диалоговое окно CBKMEMDlg
CBKMEMDlg::CBKMEMDlg(BK_DEV_MPI nBKModel, BK_DEV_MPI nBKFDDModel, uint8_t *MainMem, uint8_t *AddMem, QWidget *pParent /*=nullptr*/)
    : QDialog(pParent)
	, m_Memory(MainMem)
	, m_MemoryADD(AddMem)
	, m_BKModel(nBKModel)
	, m_BKFDDmodel(nBKFDDModel)
	, m_nTabsCount(0)
	, m_nSelectedTab(0)
{
	ZeroMemory(m_Container, sizeof(m_Container));
	m_Screen[MM_FIRST_PAGE] = nullptr;
	m_Screen[MM_SECOND_PAGE] = nullptr;
    m_pTabWidget = new QTabWidget(this);
    m_pTabWidget->resize(610, 600);
    m_pTabWidget->move(5,5);
    resize(620, 640);

    OnInitDialog();
    StartTimer();
    connect(m_pTabWidget, &QTabWidget::currentChanged, this,  &CBKMEMDlg::OnTabChanged);
    setAttribute(Qt::WA_DeleteOnClose);
}

CBKMEMDlg::~CBKMEMDlg()
{
    StopTimer();
    OnDestroy();
    emit CloseWindow();
}

//void CBKMEMDlg::DoDataExchange(CDataExchange *pDX)
//{
//	CDialogEx::DoDataExchange(pDX);
//	DDX_Control(pDX, IDC_TAB_MM_MEMTABS, m_tab);
//}

//BEGIN_MESSAGE_MAP(CBKMEMDlg, CDialogEx)
//	ON_WM_QUERYDRAGICON()
//	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MM_MEMTABS, &CBKMEMDlg::OnTcnSelchangeTab1)
//	ON_BN_CLICKED(ID_BUTTON_MM_COLOR_MODE_P1, &CBKMEMDlg::OnBnClickedButtonMmColorModeP1)
//	ON_BN_CLICKED(ID_BUTTON_MM_BW_MODE_P1, &CBKMEMDlg::OnBnClickedButtonMmBwModeP1)
//	ON_BN_CLICKED(ID_BUTTON_MM_LOAD_P1, &CBKMEMDlg::OnBnClickedButtonMmLoadP1)
//	ON_BN_CLICKED(ID_BUTTON_MM_SAVE_P1, &CBKMEMDlg::OnBnClickedButtonMmSaveP1)
//	ON_BN_CLICKED(ID_BUTTON_MM_SPRITE_P1, &CBKMEMDlg::OnBnClickedButtonMmSpriteP1)
//	ON_BN_CLICKED(ID_BUTTON_MM_COLOR_MODE_P2, &CBKMEMDlg::OnBnClickedButtonMmColorModeP2)
//	ON_BN_CLICKED(ID_BUTTON_MM_BW_MODE_P2, &CBKMEMDlg::OnBnClickedButtonMmBwModeP2)
//	ON_BN_CLICKED(ID_BUTTON_MM_LOAD_P2, &CBKMEMDlg::OnBnClickedButtonMmLoadP2)
//	ON_BN_CLICKED(ID_BUTTON_MM_SAVE_P2, &CBKMEMDlg::OnBnClickedButtonMmSaveP2)
//	ON_BN_CLICKED(ID_BUTTON_MM_SPRITE_P2, &CBKMEMDlg::OnBnClickedButtonMmSpriteP2)
//	ON_WM_DESTROY()
//END_MESSAGE_MAP()

BOOL CBKMEMDlg::OnInitDialog()
{
//	CDialogEx::OnInitDialog();
    CreateScreen(MM_FIRST_PAGE, 0);
    CreateScreen(MM_SECOND_PAGE, 1);
//	// Подсказки
//	m_ToolTip.Create(this, WS_POPUP | TTS_NOPREFIX | TTS_USEVISUALSTYLE | TTS_ALWAYSTIP/* | TTS_BALLOON*/);
//	m_ToolTip.SetDelayTime(TTDT_AUTOPOP, 20000);
//	// *** you could change the default settings ***
//	m_ToolTip.SetMaxTipWidth(256);
//	// m_ToolTip.SetTipBkColor(RGB(120,120,120));
//	// m_ToolTip.SetTipTextColor(RGB(255, 0, 0));
//	//
//	m_ToolTip.AddTool(GetDlgItem(ID_BUTTON_MM_COLOR_MODE_P1), IDS_BUTTON_COLORMODE_STR);
//	m_ToolTip.AddTool(GetDlgItem(ID_BUTTON_MM_COLOR_MODE_P2), IDS_BUTTON_COLORMODE_STR);
//	m_ToolTip.AddTool(GetDlgItem(ID_BUTTON_MM_BW_MODE_P1), IDS_BUTTON_ADAPTIVEBWMODE_STR);
//	m_ToolTip.AddTool(GetDlgItem(ID_BUTTON_MM_BW_MODE_P2), IDS_BUTTON_ADAPTIVEBWMODE_STR);
//	m_ToolTip.AddTool(GetDlgItem(ID_BUTTON_MM_SPRITE_P1), IDS_BUTTON_SPRITE_STR);
//	m_ToolTip.AddTool(GetDlgItem(ID_BUTTON_MM_SPRITE_P2), IDS_BUTTON_SPRITE_STR);
//	m_ToolTip.AddTool(GetDlgItem(ID_BUTTON_MM_SAVE_P1), IDS_BUTTON_SAVE_STR);
//	m_ToolTip.AddTool(GetDlgItem(ID_BUTTON_MM_SAVE_P2), IDS_BUTTON_SAVE_STR);
//	m_ToolTip.AddTool(GetDlgItem(ID_BUTTON_MM_LOAD_P1), IDS_BUTTON_LOAD_STR);
//	m_ToolTip.AddTool(GetDlgItem(ID_BUTTON_MM_LOAD_P2), IDS_BUTTON_LOAD_STR);
//	// можно было так не извращаться, и оставить как было, но я слишком поздно
//	// заметил, что у новых иконок разрешение 72х72 точек на дюйм
//	// вместо 96х96 и и именно поэтому они глючно отображались, а не потому,
//	// на что я думал. Но, что сделано, то сделано, пусть теперь так остаётся
//	CBitmap m_bmparr;
//	m_bmparr.LoadBitmap(IDB_MM_BUTTONS);
//	CImageList m_imagelist;
//	m_imagelist.Create(16, 15, ILC_COLOR32 | ILC_MASK, 0, 0);
//	m_imagelist.Add(&m_bmparr, RGB(192, 192, 192));
//	reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_COLOR_MODE_P1))->SetIcon(m_imagelist.ExtractIcon(0));
//	reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_COLOR_MODE_P2))->SetIcon(m_imagelist.ExtractIcon(0));
//	reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_BW_MODE_P1))->SetIcon(m_imagelist.ExtractIcon(1));
//	reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_BW_MODE_P2))->SetIcon(m_imagelist.ExtractIcon(1));
//	reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_LOAD_P1))->SetIcon(m_imagelist.ExtractIcon(2));
//	reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_LOAD_P2))->SetIcon(m_imagelist.ExtractIcon(2));
//	reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_SAVE_P1))->SetIcon(m_imagelist.ExtractIcon(3));
//	reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_SAVE_P2))->SetIcon(m_imagelist.ExtractIcon(3));
//	reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_SPRITE_P1))->SetIcon(m_imagelist.ExtractIcon(4));
//	reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_SPRITE_P2))->SetIcon(m_imagelist.ExtractIcon(4));
//	m_imagelist.DeleteImageList();
//	m_bmparr.DeleteObject();

	// создаём вкладки
	switch (m_BKModel)
	{
		case BK_DEV_MPI::BK0010:
			CreateTabs_10();
			break;

		case BK_DEV_MPI::BK0011:
		case BK_DEV_MPI::BK0011M:
			CreateTabs_11M();
			break;

		default:
			ASSERT(false); // неопределённых значений не должно быть в принципе
	}

	switch (m_BKFDDmodel)
	{
		default:
		case BK_DEV_MPI::STD_FDD:
			break;

		case BK_DEV_MPI::A16M:
			AddTabsA16M();
			break;

		case BK_DEV_MPI::SMK512:
			AddTabsSMK512();
			break;
	}

	SelectTab(); // выберем какую-нибудь вкладку (по умолчанию - нулевую)
//	m_ToolTip.Activate(TRUE);
	return TRUE; // возврат значения TRUE, если фокус не передан элементу управления
}

void CBKMEMDlg::SelectTab()
{
	if (m_Container[MM_FIRST_PAGE][m_nSelectedTab].bExist)
	{
		SetTabParam(MM_FIRST_PAGE);
	}

	if (m_Container[MM_SECOND_PAGE][m_nSelectedTab].bExist)
	{
        m_Screen[MM_SECOND_PAGE]->show();
		SetTabParam(MM_SECOND_PAGE);
	}
	else
	{
        m_Screen[MM_SECOND_PAGE]->hide();
	}
}

void CBKMEMDlg::SetTabParam(int nPage)
{
	// переключим отображаемый буфер
    m_Screen[nPage]->GetScreen()->ChangeBuffer(m_Container[nPage][m_nSelectedTab].pBuffer, m_Container[nPage][m_nSelectedTab].nBufSize);
	// зададим режимы отображения
	SetColormode(nPage, m_Container[nPage][m_nSelectedTab].bColorMode);
	SetBWMode(nPage, m_Container[nPage][m_nSelectedTab].bBWAdaptMode);
}

void CBKMEMDlg::OnTabChanged(int nTab)
{
    m_nSelectedTab = nTab;

    if (m_nSelectedTab < 0)
    {
        m_nSelectedTab = 0;
    }

    SelectTab();
}

void CBKMEMDlg::CreateScreen(int nPage, int idUI)
{
	CString str;
	str.Format(_T("BKScreen_P%d"), nPage + 1);
	// если рендер D3D - то карту памяти выводим, используя DrawDIB

    CScreen *scr = new CScreen((g_Config.m_nScreenRenderType == CONF_SCREEN_RENDER::D3D ? CONF_SCREEN_RENDER::VFW : g_Config.m_nScreenRenderType));
    scr->OnCreate();
    m_Screen[nPage] = new COpenGlView(m_pTabWidget, scr);

	if (m_Screen[nPage])
	{
//		CRect rect;
//		GetDlgItem(idUI)->GetWindowRect(&rect);
//		ScreenToClient(&rect);
//		GetDlgItem(idUI)->DestroyWindow();

		// размеры - константа 512х256
//        if (!m_Screen[nPage]->Create(nullptr, str,
//		                             WS_VISIBLE | WS_CHILD,
//		                             CRect(rect.left, rect.top, rect.left + SCREEN_WIDTH, rect.top + SCREEN_HEIGHT),
//		                             &m_tab, 0)
//		   )
//		{
//			TRACE(_T("Не удалось создать экран %s\n"), str);
//			SAFE_DELETE(m_Screen[nPage]); // не удалось создать
//		}
        m_Screen[nPage]->move(5, (SCREEN_HEIGHT + 20) * nPage + 30);
        m_Screen[nPage]->resize(SCREEN_WIDTH, SCREEN_HEIGHT);
	}
	else
	{
		// не удалось создать
        TRACE(_T("Недостаточно памяти для создания экрана %s\n"), str.GetString());
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}
}

void CBKMEMDlg::CreateTabs_10()
{
	m_nTabsCount = 1;
	CString strtab;

	for (int i = 0; i < m_nTabsCount; ++i)
	{
		m_Container[MM_FIRST_PAGE][i].bExist = true;
		m_Container[MM_FIRST_PAGE][i].pBuffer = m_Memory + i * 0100000;
		m_Container[MM_FIRST_PAGE][i].nBufSize = 040000;
		m_Container[MM_FIRST_PAGE][i].bColorMode = true;
		m_Container[MM_FIRST_PAGE][i].bBWAdaptMode = false;
		m_Container[MM_SECOND_PAGE][i].bExist = true;
		m_Container[MM_SECOND_PAGE][i].pBuffer = m_Memory + i * 0100000 + 040000;
		m_Container[MM_SECOND_PAGE][i].nBufSize = 040000;
		m_Container[MM_SECOND_PAGE][i].bColorMode = true;
		m_Container[MM_SECOND_PAGE][i].bBWAdaptMode = false;
		strtab.Format(_T("BK10_pg%d%d"), i * 2, i * 2 + 1);
//		m_tab.InsertItem(i, strtab);
        m_pTabWidget->addTab(new QWidget(), strtab);
    }
}

void CBKMEMDlg::CreateTabs_11M()
{
	m_nTabsCount = 4;
	static int Pages[8] = { 0, 1, 2, 3, 5, 6, 4, 7 };
	CString strtab;

	for (int i = 0; i < m_nTabsCount; ++i)
	{
		int n0 = Pages[i * 2];
		int n1 = Pages[i * 2 + 1];
		m_Container[MM_FIRST_PAGE][i].bExist = true;
		m_Container[MM_FIRST_PAGE][i].pBuffer = m_Memory + (n0 << 14);
		m_Container[MM_FIRST_PAGE][i].nBufSize = 040000;
		m_Container[MM_FIRST_PAGE][i].bColorMode = true;
		m_Container[MM_FIRST_PAGE][i].bBWAdaptMode = false;
		m_Container[MM_SECOND_PAGE][i].bExist = true;
		m_Container[MM_SECOND_PAGE][i].pBuffer = m_Memory + (n1 << 14);
		m_Container[MM_SECOND_PAGE][i].nBufSize = 040000;
		m_Container[MM_SECOND_PAGE][i].bColorMode = true;
		m_Container[MM_SECOND_PAGE][i].bBWAdaptMode = false;
		strtab.Format(_T("BK11M_pg%d%d"), n0, n1);
        m_pTabWidget->addTab(new QWidget(), strtab);
	}
}

void CBKMEMDlg::AddTabsA16M()
{
	unsigned int i = m_nTabsCount++;
	CString strtab;
	m_Container[MM_FIRST_PAGE][i].bExist = true;
	m_Container[MM_FIRST_PAGE][i].pBuffer = m_MemoryADD;
	m_Container[MM_FIRST_PAGE][i].nBufSize = 040000;
	m_Container[MM_FIRST_PAGE][i].bColorMode = true;
	m_Container[MM_FIRST_PAGE][i].bBWAdaptMode = false;
	m_Container[MM_SECOND_PAGE][i].bExist = false;
    m_pTabWidget->addTab(new QWidget(), _T("A16M_pg0"));
}

void CBKMEMDlg::AddTabsSMK512()
{
	static const CString arPgNums[16] =
	{
		_T("0000"), _T("2000"), _T("0004"), _T("2004"),
		_T("0010"), _T("2010"), _T("0014"), _T("2014"),
		_T("0001"), _T("2001"), _T("0005"), _T("2005"),
		_T("0011"), _T("2011"), _T("0015"), _T("2015")
	};
	unsigned int n = m_nTabsCount;
	m_nTabsCount += 16;
	CString strtab;

	for (int i = n, j = 0; i < m_nTabsCount; ++i, ++j)
	{
		m_Container[MM_FIRST_PAGE][i].bExist = true;
		m_Container[MM_FIRST_PAGE][i].pBuffer = m_MemoryADD + j * 0100000;
		m_Container[MM_FIRST_PAGE][i].nBufSize = 040000;
		m_Container[MM_FIRST_PAGE][i].bColorMode = true;
		m_Container[MM_FIRST_PAGE][i].bBWAdaptMode = false;
		m_Container[MM_SECOND_PAGE][i].bExist = true;
		m_Container[MM_SECOND_PAGE][i].pBuffer = m_MemoryADD + j * 0100000 + 040000;
		m_Container[MM_SECOND_PAGE][i].nBufSize = 040000;
		m_Container[MM_SECOND_PAGE][i].bColorMode = true;
		m_Container[MM_SECOND_PAGE][i].bBWAdaptMode = false;
		strtab = _T("SMK_") + arPgNums[j];
        m_pTabWidget->addTab(new QWidget(), strtab);
    }
}

void CBKMEMDlg::DrawTab()
{
	if (!m_lockStop.IsLocked())
	{
		m_lockDraw.Lock();

		if (m_Container[MM_FIRST_PAGE][m_nSelectedTab].bExist)
		{
			m_Screen[MM_FIRST_PAGE]->ReDrawScreen();
		}

		if (m_Container[MM_SECOND_PAGE][m_nSelectedTab].bExist)
		{
			m_Screen[MM_SECOND_PAGE]->ReDrawScreen();
		}

		m_lockDraw.UnLock();
	}
}


//void CBKMEMDlg::OnCancel()
//{
//	CDialogEx::OnCancel();
//	DestroyWindow();
//}



void CBKMEMDlg::OnDestroy()
{
//	// посылаем в MainFrame сообщение о закрытии всего этого тут
//	GetParentFrame()->SendMessage(WM_MEMMAP_CLOSE, 0, 0);
//	CDialogEx::OnDestroy();
//	m_lockStop.Lock();

//	while (m_lockDraw.IsLocked())
//	{
//		Sleep(1); // тут может быть дедлок. если UI в m_pdlgBKMem->DrawTab() захочет сообщений;
//	}

//	m_tab.DeleteAllItems();
//	// DestroyWindow надо делать вручную
    CScreen *scr = m_Screen[MM_FIRST_PAGE]->GetScreen();
    SAFE_DELETE(m_Screen[MM_FIRST_PAGE]);
    SAFE_DELETE(scr);
    scr = m_Screen[MM_SECOND_PAGE]->GetScreen();
    SAFE_DELETE(m_Screen[MM_SECOND_PAGE]);
    SAFE_DELETE(scr);
    SAFE_DELETE(m_pTabWidget);
//	SAFE_DELETE(m_Screen[MM_FIRST_PAGE]);
//	SAFE_DELETE(m_Screen[MM_SECOND_PAGE]);
//	delete this; // самоудаляемся
	// и тут надо быть осторожным. Всё, что навыделяли - удалять вручную.
}

//BOOL CBKMEMDlg::PreTranslateMessage(MSG *pMsg)
//{
//	if (pMsg->message >= WM_MOUSEFIRST && pMsg->message <= WM_MOUSELAST)
//	{
//		m_ToolTip.RelayEvent(pMsg);
//	}

//	return CDialogEx::PreTranslateMessage(pMsg);
//}


void CBKMEMDlg::OnBnClickedButtonMmColorModeP1()
{
	ChangeColormode(MM_FIRST_PAGE);
}


void CBKMEMDlg::OnBnClickedButtonMmBwModeP1()
{
	ChangeBWMode(MM_FIRST_PAGE);
}


void CBKMEMDlg::OnBnClickedButtonMmLoadP1()
{
	LoadImg(MM_FIRST_PAGE);
}


void CBKMEMDlg::OnBnClickedButtonMmSaveP1()
{
	SaveImg(MM_FIRST_PAGE);
}


void CBKMEMDlg::OnBnClickedButtonMmSpriteP1()
{
	ViewSprite(MM_FIRST_PAGE);
}


void CBKMEMDlg::OnBnClickedButtonMmColorModeP2()
{
	ChangeColormode(MM_SECOND_PAGE);
}


void CBKMEMDlg::OnBnClickedButtonMmBwModeP2()
{
	ChangeBWMode(MM_SECOND_PAGE);
}


void CBKMEMDlg::OnBnClickedButtonMmLoadP2()
{
	LoadImg(MM_SECOND_PAGE);
}


void CBKMEMDlg::OnBnClickedButtonMmSaveP2()
{
	SaveImg(MM_SECOND_PAGE);
}


void CBKMEMDlg::OnBnClickedButtonMmSpriteP2()
{
	ViewSprite(MM_SECOND_PAGE);
}

void CBKMEMDlg::ChangeColormode(int nPage)
{
	bool b = !m_Container[nPage][m_nSelectedTab].bColorMode;
	m_Container[nPage][m_nSelectedTab].bColorMode = b;
	SetColormode(nPage, b);
}

void CBKMEMDlg::SetColormode(int nPage, bool bMode)
{
    m_Screen[nPage]->GetScreen()->SetColorMode(bMode);

	switch (nPage)
	{
		case MM_FIRST_PAGE:
//			reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_BW_MODE_P1))->EnableWindow(!bMode);
			break;

		case MM_SECOND_PAGE:
//			reinterpret_cast<CButton *>(GetDlgItem(ID_BUTTON_MM_BW_MODE_P2))->EnableWindow(!bMode);
			break;
	}
}

void CBKMEMDlg::ChangeBWMode(int nPage)
{
	bool b = !m_Container[nPage][m_nSelectedTab].bBWAdaptMode;
	m_Container[nPage][m_nSelectedTab].bBWAdaptMode = b;
	SetBWMode(nPage, b);
}

void CBKMEMDlg::SetBWMode(int nPage, bool bMode)
{
    m_Screen[nPage]->GetScreen()->SetAdaptMode(bMode);
}

void CBKMEMDlg::ViewSprite(int nPage)
{
#if 0
	auto pSprWnd = new CSprWnd(m_Container[nPage][m_nSelectedTab].pBuffer, m_Container[nPage][m_nSelectedTab].nBufSize); // обязательно создавать динамически.

	if (pSprWnd)
	{
		WNDCLASS wc;
		memset(&wc, 0, sizeof(wc));
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.hInstance = AfxGetInstanceHandle();
		wc.lpfnWndProc = ::DefWindowProc;
		wc.lpszClassName = _T("MySprClass");
		wc.hbrBackground = (HBRUSH)::GetStockObject(GRAY_BRUSH);
		AfxRegisterClass(&wc);
		CString str, str2;
		m_tab.GetWindowText(str);
		str2.Format(_T(" %d"), nPage);

		if (pSprWnd->Create(
		            _T("MySprClass"), // Имя класса виндовс
		            _T("Sprite View : ") + str + str2, // имя окна
		            WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME, // window styles
		            CRect(0, 0, 200, 200), // размер окна
		            GetParentOwner(), // родитель окна
		            nullptr,
		            WS_EX_TOOLWINDOW // | WS_EX_OVERLAPPEDWINDOW // extended window styles
		        ))
		{
			pSprWnd->ShowWindow(SW_SHOW);
		}
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

	m_Screen[nPage]->SetFocus();
#endif
}

void CBKMEMDlg::SaveImg(int nPage)
{
#if 0
	HBITMAP hBm = nullptr;
	uint32_t *pNewBits = nullptr;

	if (m_Container[nPage][m_nSelectedTab].bColorMode)
	{
		// сохранение страницы как изображения. совершенно особый подход.
		// сохранять будем как есть
		pNewBits = new uint32_t[256 * 256]; // новое битовое поле

		if (pNewBits)
		{
			int nbi = 0;

			// формируем битмап
			for (int y = 0; y < 256; ++y)
			{
				// алгоритм не оптимален, но нам нужна принципиальная работоспособность
				// оптимизируем потом
				for (int x = 0; x < 256; ++x)
				{
					// берём очередной бит.
					int b = y * 0100 + x / 4;
					int m = ((x % 4) * 2);
					uint8_t v = m_Container[nPage][m_nSelectedTab].pBuffer[b];
					int c = (v >> m) & 3;
					pNewBits[nbi++] = g_pColorPalettes[0][c];
				}
			}

			hBm = CreateBitmap(256, 256, 1, 32, pNewBits);
		}
		else
		{
			g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
		}
	}
	else
	{
		// создаём чёрно-белую картинку
		pNewBits = new uint32_t[512 * 256]; // новое битовое поле

		if (pNewBits)
		{
			int nbi = 0;

			// формируем битмап
			for (int y = 0; y < 256; ++y)
			{
				// алгоритм не оптимален, но нам нужна принципиальная работоспособность
				// оптимизируем потом
				for (int x = 0; x < 512; ++x)
				{
					// берём очередной бит.
					int b = y * 0100 + x / 8;
					int m = (x % 8);
					uint8_t v = m_Container[nPage][m_nSelectedTab].pBuffer[b];
					int c = (v >> m) & 1;
					pNewBits[nbi++] = g_pMonochromePalette[0][c];
				}
			}

			hBm = CreateBitmap(512, 256, 1, 32, pNewBits);
		}
		else
		{
			g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
		}
	}

	if (pNewBits)
	{
		CImage img;
		img.Attach(hBm);
		CString strFilter;
		CSimpleArray<GUID> aguidFileTypes;
		img.GetExporterFilterString(strFilter, aguidFileTypes);
		CFileDialog dlg(false, _T("png"), nullptr,
		                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
		                strFilter, this);

		if (dlg.DoModal() == IDOK)
		{
			DWORD nFilterSave = dlg.m_ofn.nFilterIndex;
			GUID guid = nFilterSave > 0 ? aguidFileTypes[nFilterSave - 1] : GUID(GUID_NULL);
			CString strFileName = dlg.GetPathName();

			if (dlg.GetFileExt().IsEmpty())
			{
				if (strFileName[strFileName.GetLength() - 1] == '.')
				{
					strFileName = strFileName.Left(strFileName.GetLength() - 1);
				}

				CString strExt;

				if (nFilterSave == 0)
				{
					strExt = _T(".jpg"); // default to JPEG
				}
				else
				{
					// Look up the first extension in the filters
					int nCount = (nFilterSave * 2) - 1;
					int nLeft = 0;

					while (nCount)
					{
						if (strFilter[nLeft++] == '|')
						{
							nCount--;
						}
					}

					ASSERT(nLeft < strFilter.GetLength());
					strExt = strFilter.Tokenize(_T(";|"), nLeft).MakeLower();
					strExt = ::PathFindExtension(strExt);
				}

				strFileName += strExt;
			}

			img.Save(strFileName, guid);
		}

		img.Detach();
		DeleteObject(hBm);
		delete[] pNewBits;
	}

	m_Screen[nPage]->SetFocus();
#endif
}

void CBKMEMDlg::LoadImg(int nPage)
{
#if 0
	CImage img;
	CString strImporters;
	CSimpleArray<GUID> aguidFileTypes;
	CString sf(MAKEINTRESOURCE(IDS_FILEFILTER_IMGLOAD));
	img.GetImporterFilterString(strImporters, aguidFileTypes, sf);
	CFileDialog dlg(true, _T("png"), nullptr,
	                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
	                strImporters, this);

	if (dlg.DoModal() == IDOK)
	{
		if (SUCCEEDED(img.Load(dlg.GetPathName())))
		{
			int w = img.GetWidth();
			int h = img.GetHeight();

			if (m_Container[nPage][m_nSelectedTab].bColorMode)
			{
				w = min(256, w); // если подсовывают большую картинку -
				h = min(256, h); // будем тупо обрезать её

				for (int y = 0; y < h; ++y)
				{
					// алгоритм не оптимален, но нам нужна принципиальная работоспособность
					// оптимизируем потом
					for (int x = 0; x < w; ++x)
					{
						// берём очередной пиксел.
						COLORREF c = img.GetPixel(x, y);
						int b = y * 0100 + x / 4; // байт
						int m = ((x % 4) * 2); // маска в нём
						// теперь определим, какой цвет у пикселя
						int bc = 0; // чёрный

						if ((c & 0xff0000) && !(c & 0x00f8f8)) // если в синем канале что-то есть, а в остальных почти нету
						{
							bc = 1; // то это синий цвет
						}
						else if ((c & 0x00ff00) && !(c & 0xf800f8)) // если в зелёном канале что-то есть, а в остальных почти нету
						{
							bc = 2; // то это зелёный цвет
						}
						else if ((c & 0x0000ff) && !(c & 0xf8f800)) // если в красном канале что-то есть, а в остальных почти нету
						{
							bc = 3; // то это красный цвет
						}

						// все остальные комбинации - чёрный цвет. нефиг тут.
						m_Container[nPage][m_nSelectedTab].pBuffer[b] &= ~(3 << m);
						m_Container[nPage][m_nSelectedTab].pBuffer[b] |= bc << m; // задаём цвет
					}
				}
			}
			else
			{
				w = min(512, w); // если подсовывают большую картинку -
				h = min(256, h); // будем тупо обрезать её

				for (int y = 0; y < h; ++y)
				{
					// алгоритм не оптимален, но нам нужна принципиальная работоспособность
					// оптимизируем потом
					for (int x = 0; x < w; ++x)
					{
						// берём очередной пиксел.
						COLORREF c = img.GetPixel(x, y);
						int b = y * 0100 + x / 8; // байт
						int m = (x % 8); // маска в нём
						// теперь определим, какой цвет у пикселя
						int bc = 0; // чёрный

						if (c & 0xf8f8f8) // хоть в одном канале есть что-то не совсем чёрное
						{
							bc = 1; // то это белый цвет
						}

						// все остальные комбинации - чёрный цвет. нефиг тут.
						m_Container[nPage][m_nSelectedTab].pBuffer[b] &= ~(1 << m);
						m_Container[nPage][m_nSelectedTab].pBuffer[b] |= bc << m; // задаём цвет
					}
				}
			}

			img.Destroy();
		}
	}

	m_Screen[nPage]->SetFocus();
#endif
}


/*
вот так эта вся херня выглядит.
CBKMEMDlg-----------------------------------------------+
| CTabCtrl----------------------------------------+     |
| | CScreen-------------------------------------+ | +-+ |
| | |                                           | | +-+ |
| | |                                           | | +-+ |
| | |                                           | | +-+ |
| | |                                           | | +-+ |
| | |                                           | | +-+ |
| | +-------------------------------------------+ |     |
| | CScreen-------------------------------------+ | +-+ |
| | |                                           | | +-+ |
| | |                                           | | +-+ |
| | |                                           | | +-+ |
| | |                                           | | +-+ |
| | |                                           | | +-+ |
| | +-------------------------------------------+ |     |
| +-----------------------------------------------+     |
+-------------------------------------------------------+
Справа от CTabCtrl - кнопки
*/

