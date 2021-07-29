#include "MainWindow.h"
#include "ui_mainwindow.h"

#include "BKMessageBox.h"
#include "Board_10_FDD.h"
#include "Board_11.h"
#include "Board_11M.h"
#include "Board_11M_FDD.h"
#include "Board_11_FDD.h"
#include "Board_EXT32.h"
#include "Board_MSTD.h"
#include "BKAYVolPan.h"

#include <QFileDialog>
#include <QResizeEvent>
#include <QDateTime>
#include <QToolButton>
#include <QFile>

extern CMainFrame *g_pMainFrame;
QObject           *g_pBKView;


CMainFrame::CMainFrame(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

//    or get more specific
//        statusBar()->setStyleSheet(
//            "QStatusBar::item { border: none; } "
//            "QStatusBar QLabel { border: 1px solid gray; border-radius: 3px; }"
//            );

       statusBar()->setStyleSheet(
           "QStatusBar::item { border: none; } "
           "QStatusBar QLabel { border: 1px solid gray; border-radius: 3px;}"
            );

        g_pMainFrame = this;

        m_nStartTick = GetTickCount();

        m_pSB_StatusLine = new QLabel("");
        m_pSB_RusLat = new QLabel("ЛАТ");
        m_pSB_ZaglStr = new QLabel("СТР");
        m_pSB_AR2  = new QLabel("");         // тут выводится АР2
        m_pSB_SU  = new QLabel("");;          // тут выводится СУ
        m_pSB_Common  = new QLabel("");      // тут выводится всякая общая инфа
        m_pSB_CAPS  = new QLabel("");
        m_pSB_NUM  = new QLabel("");
        m_pSB_SCR  = new QLabel("");

        statusBar()->addWidget(m_pSB_RusLat);
        m_pSB_RusLat->setMinimumSize(25,0);
        statusBar()->addWidget(m_pSB_ZaglStr);
        m_pSB_ZaglStr->setMinimumSize(25,0);
        statusBar()->addWidget(m_pSB_AR2);
        m_pSB_AR2->setMinimumSize(25,0);
        statusBar()->addWidget(m_pSB_SU);
        m_pSB_SU->setMinimumSize(25,0);
        statusBar()->addWidget(m_pSB_Common);
        m_pSB_Common->setMinimumSize(325,0);
        statusBar()->addWidget(m_pSB_CAPS);
        m_pSB_CAPS->setMinimumSize(35,0);
        statusBar()->addWidget(m_pSB_NUM);
        m_pSB_NUM->setMinimumSize(25,0);
        statusBar()->addWidget(m_pSB_SCR);
        m_pSB_SCR->setMinimumSize(25,0);
        statusBar()->addWidget(m_pSB_StatusLine,1); //
        m_pSB_StatusLine->setMinimumSize(60,0);
        m_pSB_StatusLine->setStyleSheet("QLabel { border: 1px solid blue; border-radius: 3px;}");

        CreateMenu();
        InitWindows();
}

void CMainFrame::InitWindows()
{
    CString strIniFileName;
    strIniFileName.LoadString(IDS_INI_FILENAME);
    g_Config.InitConfig(strIniFileName);
    g_Config.VerifyRoms(); // проверим наличие, но продолжим выполнение при отсутствии чего-либо

    m_pSound = new CBkSound();

    if (!m_pSound)
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
    }

    if ((m_pScreen = new CScreen(g_Config.m_nScreenRenderType)) == nullptr)
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_NOCSCREENCREATE);
            TRACE0("Не удалось создать класс CScreen\n");
    }
    // Temporary init for Debugger. Must be initialized from Debugger windows
    m_pDebugger = new CDebugger();

    m_pScreen->OnCreate();

    g_pBKView = m_pBKView = new CBKView(this, m_pScreen);
    m_pBKView->setMinimumSize(640,480);
    setCentralWidget(m_pBKView);
    m_pBKView->setFocusPolicy(Qt::FocusPolicy::StrongFocus);

    m_paneBKVKBDView = new CBKVKBDView(0, QString("Keyboard"), this);
    addDockWidget(Qt::RightDockWidgetArea, m_paneBKVKBDView);

    m_paneRegistryDumpViewCPU = new CRegDumpViewCPU();
    addDockWidget(Qt::RightDockWidgetArea, m_paneRegistryDumpViewCPU);
    m_paneRegistryDumpViewCPU->AttachDebugger(m_pDebugger);
    m_paneRegistryDumpViewCPU->setWindowTitle("Registers");

    m_paneDisassembleView = new CDisasmView();
    addDockWidget(Qt::RightDockWidgetArea, m_paneDisassembleView);
    m_paneDisassembleView->AttachDebugger(m_pDebugger);
    m_paneDisassembleView->setWindowTitle("Disassembly view");

    m_paneMemoryDumpView = new CMemDumpView();
    addDockWidget(Qt::LeftDockWidgetArea, m_paneMemoryDumpView);
    m_paneMemoryDumpView->AttachDebugger(m_pDebugger);
    m_paneMemoryDumpView->setFloating(true);
    m_paneMemoryDumpView->hide();
    m_paneMemoryDumpView->setWindowTitle("Memory dump");

    QObject::connect(this, &CMainFrame::PostMessage, this, &CMainFrame::ReceiveMessage);
    QObject::connect(this, &CMainFrame::SendMessage, this, &CMainFrame::ReceiveMessage);


    OnStartPlatform();

    repaintToolBars();

}

CMainFrame::~CMainFrame()
{
    delete m_pSound;
    delete ui;
}

void CMainFrame::repaintToolBars()
{
    QList<QToolBar *> toolbars = findChildren<QToolBar *>("");
    foreach (QToolBar *tb, toolbars) {
        UpdateActions(tb->actions());
    }

}

void CMainFrame::ToggleStatusBar()
{
    statusBar()->setHidden(!statusBar()->isHidden());
}

bool CMainFrame::event(QEvent *event)
{
//    if (event->type() == QEvent::User + 0) {
//        <MyCustomEvent *>(event);
//        // custom event handling here
//        return true;
//    }

    return QMainWindow::event(event);
}


#if 0
LRESULT CMainFrame::OnToolbarReset(WPARAM wp, LPARAM)
{
    // Настройки всех тулбаров: высота
    UpdateToolbarSize();
    // Сброс тулбаров через меню Настройки тулбаров
    ResetToolbar(UINT(wp));
    return S_OK;
}

void CMainFrame::ResetToolbar(UINT uiToolBarId)
{
    if (uiToolBarId == GetTBID(UIMENUS::SOUND_MENU))
    {
        CSliderButton btnSlider(ID_OPTIONS_SOUND_VOLUME, -1, TBS_TRANSPARENTBKGND);
        btnSlider.SetRange(0, 0xffff);
//      if (g_Config.m_bBigButtons) {
//          // Поменяем высоту контрола Слайдера на лету
//          // CRect rectok, wind;
//      }
        m_wndToolBarSound.ReplaceButton(ID_OPTIONS_SOUND_VOLUME, btnSlider);
        btnSlider.SetValue(g_Config.m_nSoundVolume);
//		if (m_pSound)
//		{
//			m_pSound->SoundGen_SetVolume(g_Config.m_nSoundVolume);
//		}
    }
    else if (uiToolBarId == GetTBID(UIMENUS::MAIN_MENU))
    {
        // создаём кнопки с выпадающими меню. 4 привода - 4 разных кнопки с 4мя разными-одинаковыми меню
        CMenu menuA;
        VERIFY(menuA.LoadMenu(IDR_LOADDRIVE_MENU_A));
        CMenu *pSubMenu = menuA.GetSubMenu(0);
        CMFCToolBarMenuButton btnBackA(ID_FILE_LOADDRIVE_A, pSubMenu->GetSafeHmenu(), -1);
        m_wndToolBar.ReplaceButton(ID_FILE_LOADDRIVE_A, btnBackA);
        CMenu menuB;
        VERIFY(menuB.LoadMenu(IDR_LOADDRIVE_MENU_B));
        pSubMenu = menuB.GetSubMenu(0);
        CMFCToolBarMenuButton btnBackB(ID_FILE_LOADDRIVE_B, pSubMenu->GetSafeHmenu(), -1);
        m_wndToolBar.ReplaceButton(ID_FILE_LOADDRIVE_B, btnBackB);
        CMenu menuC;
        VERIFY(menuC.LoadMenu(IDR_LOADDRIVE_MENU_C));
        pSubMenu = menuC.GetSubMenu(0);
        CMFCToolBarMenuButton btnBackC(ID_FILE_LOADDRIVE_C, pSubMenu->GetSafeHmenu(), -1);
        m_wndToolBar.ReplaceButton(ID_FILE_LOADDRIVE_C, btnBackC);
        CMenu menuD;
        VERIFY(menuD.LoadMenu(IDR_LOADDRIVE_MENU_D));
        pSubMenu = menuD.GetSubMenu(0);
        CMFCToolBarMenuButton btnBackD(ID_FILE_LOADDRIVE_D, pSubMenu->GetSafeHmenu(), -1);
        m_wndToolBar.ReplaceButton(ID_FILE_LOADDRIVE_D, btnBackD);
        // И тут-же обновим иконки дисков
        UpdateToolbarDriveIcons();
    }
}

#endif

// наглядно отобразим, что и в каком дисководе находится
void CMainFrame::UpdateToolbarDriveIcons()
{
    ChangeImageIcon(ID_FILE_LOADDRIVE_A, FDD_DRIVE::A);
    ChangeImageIcon(ID_FILE_LOADDRIVE_B, FDD_DRIVE::B);
    ChangeImageIcon(ID_FILE_LOADDRIVE_C, FDD_DRIVE::C);
    ChangeImageIcon(ID_FILE_LOADDRIVE_D, FDD_DRIVE::D);
}

#if 0

// Эта же функция может быть вызвана при изменении размеров иконок кнопкой
void CMainFrame::UpdateToolbarSize()
{
    if (g_Config.m_bBigButtons)
    {
        m_wndToolBar.SetSizes(CSize(28, 28), CSize(24, 23));
        m_wndToolBarSound.SetSizes(CSize(28, 28), CSize(24, 23));
        m_wndToolBarDebug.SetSizes(CSize(28, 28), CSize(24, 23));
        m_wndToolBarVCapt.SetSizes(CSize(28, 28), CSize(24, 23));
    }
    else
    {
        m_wndToolBar.SetSizes(CSize(20, 20), CSize(16, 15));
        m_wndToolBarSound.SetSizes(CSize(20, 20), CSize(16, 15));
        m_wndToolBarDebug.SetSizes(CSize(20, 20), CSize(16, 15));
        m_wndToolBarVCapt.SetSizes(CSize(20, 20), CSize(16, 15));
    }
}

#endif

void CMainFrame::OnLVolumeSlider(int value)
{
    if (m_pVolumeSlider)
    {
        g_Config.m_nSoundVolume = m_pVolumeSlider->value();
    }

    if (m_pSound)
    {
        m_pSound->SoundGen_SetVolume(g_Config.m_nSoundVolume);
    }
}

#if 0
bool CMainFrame::CreateDockingWindows()
{
    // Создать окно дампа регистров
    CString strWndName;
    BOOL bNameValid = strWndName.LoadString(IDS_REGDUMP_CPU_WND);
    ASSERT(bNameValid);

    if (!m_paneRegistryDumpViewCPU.Create(strWndName, this, CRect(0, 0, 280, 200), true, ID_VIEW_REGDUMP_CPU, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_MFDOCKREGDUMPVIEWERROR, MB_OK);
        TRACE1("Не удалось создать окно \"%s\"\n", strWndName);
        return false; // не удалось создать
    }

    bNameValid = strWndName.LoadString(IDS_REGDUMP_FDD_WND);
    ASSERT(bNameValid);

    if (!m_paneRegistryDumpViewFDD.Create(strWndName, this, CRect(0, 0, 280, 200), true, ID_VIEW_REGDUMP_FDD, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_MFDOCKREGDUMPVIEWERROR, MB_OK);
        TRACE1("Не удалось создать окно \"%s\"\n", strWndName);
        return false; // не удалось создать
    }

    // Создать окно управления лентами
    bNameValid = strWndName.LoadString(IDS_TAPECTRL_WND);
    ASSERT(bNameValid);

    if (!m_paneTapeCtrlView.Create(strWndName, this, CRect(0, 0, 280, 50), true, ID_VIEW_TAPECTRLWND, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI))
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_MFDOCKTAPECTRLVIEWERROR, MB_OK);
        TRACE1("Не удалось создать окно \"%s\"\n", strWndName);
        return false; // не удалось создать
    }

    // Создать окно отладка
    bNameValid = strWndName.LoadString(IDS_DEBUG_WND);
    ASSERT(bNameValid);

    if (!m_paneDisassembleView.Create(strWndName, this, CRect(0, 0, 350, 200), true, ID_VIEW_DEBUGWND, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_MFDOCKDEBUGVIEWERROR, MB_OK);
        TRACE1("Не удалось создать окно \"%s\"\n", strWndName);
        return false; // не удалось создать
    }

    bNameValid = strWndName.LoadString(IDS_MEMDUMP_WND);
    ASSERT(bNameValid);

    if (!m_paneMemoryDumpView.Create(strWndName, this, CRect(0, 0, 350, 200), true, ID_VIEW_MEMDUMP, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI))
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_MFDOCKMEMDUMPVIEWERROR, MB_OK);
        TRACE1("Не удалось создать окно \"%s\"\n", strWndName);
        return false; // не удалось создать
    }

    // Создать окно осциллографа
    bNameValid = strWndName.LoadString(IDS_OSCILLATOR_WND);
    ASSERT(bNameValid);

    if (!m_paneOscillatorView.Create(strWndName, this, CRect(0, 0, 400, 200), true, ID_VIEW_OSCILLATOR, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_MFDOCKOSCVIEWERROR, MB_OK);
        TRACE1("Не удалось создать окно \"%s\"\n", strWndName);
        return false; // не удалось создать
    }

    // Создать окно виртуальной клавиатуры
    bNameValid = strWndName.LoadString(IDS_VKBD_WND);

    if (!m_paneBKVKBDView.Create(strWndName, this, CRect(0, 0, 700, 200), true, ID_VIEW_VKBD, WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI))
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_MFDOCKTAPECTRLVIEWERROR, MB_OK);
        TRACE1("Не удалось создать окно \"%s\"\n", strWndName);
        return false; // не удалось создать
    }

    SetDockingWindowIcons(theApp.m_bHiColorIcons);
    return true;
}

void CMainFrame::SetDockingWindowIcons(bool bHiColorIcons)
{
    auto hIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_REGDUMPCPU_WND_HC : IDI_REGDUMPCPU_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_paneRegistryDumpViewCPU.SetIcon(hIcon, FALSE);
    hIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_REGDUMPFDD_WND_HC : IDI_REGDUMPFDD_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_paneRegistryDumpViewFDD.SetIcon(hIcon, FALSE);
    hIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_MEMDUMP_WND_HC : IDI_MEMDUMP_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_paneMemoryDumpView.SetIcon(hIcon, FALSE);
    hIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_DEBUG_WND_HC : IDI_DEBUG_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_paneDisassembleView.SetIcon(hIcon, FALSE);
    hIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_TAPE_WND_HC : IDI_TAPE_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_paneTapeCtrlView.SetIcon(hIcon, FALSE);
    hIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OSC_WND_HC : IDI_OSC_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_paneOscillatorView.SetIcon(hIcon, FALSE);
    hIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_VKBD_WND_HC : IDI_VKBD_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
    m_paneBKVKBDView.SetIcon(hIcon, FALSE);
}

// диагностика CMainFrame

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
    CFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext &dc) const
{
    CFrameWndEx::Dump(dc);
}
#endif // _DEBUG


// обработчики сообщений CMainFrame

void CMainFrame::OnViewCustomize()
{
    auto pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* сканировать меню*/);

    if (pDlgCust)
    {
        pDlgCust->EnableUserDefinedToolbars();
        pDlgCust->Create();
    }
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp, LPARAM lp)
{
    LRESULT lres = CFrameWndEx::OnToolbarCreateNew(wp, lp);

    if (lres)
    {
        auto pUserToolbar = reinterpret_cast<CMFCToolBar *>(lres);
        ASSERT_VALID(pUserToolbar);
        CString strCustomize;
        BOOL bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
        ASSERT(bNameValid);
        pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
    }

    return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
    CWaitCursor wait;
    theApp.m_nAppLook = id;

    switch (theApp.m_nAppLook)
    {
        case ID_VIEW_APPLOOK_WIN_2000:
            CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
            break;

        case ID_VIEW_APPLOOK_OFF_XP:
            CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
            break;

        case ID_VIEW_APPLOOK_WIN_XP:
            CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
            CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
            break;

        case ID_VIEW_APPLOOK_OFF_2003:
            CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
            CDockingManager::SetDockingMode(DT_SMART);
            break;

        case ID_VIEW_APPLOOK_VS_2005:
            CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
            CDockingManager::SetDockingMode(DT_SMART);
            break;

        case ID_VIEW_APPLOOK_VS_2008:
            CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2008));
            CDockingManager::SetDockingMode(DT_SMART);
            break;

        case ID_VIEW_APPLOOK_WINDOWS_7:
            CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows7));
            CDockingManager::SetDockingMode(DT_SMART);
            break;

        default:
            switch (theApp.m_nAppLook)
            {
                case ID_VIEW_APPLOOK_OFF_2007_BLUE:
                    CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
                    break;

                case ID_VIEW_APPLOOK_OFF_2007_BLACK:
                    CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
                    break;

                case ID_VIEW_APPLOOK_OFF_2007_SILVER:
                    CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
                    break;

                case ID_VIEW_APPLOOK_OFF_2007_AQUA:
                    CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
                    break;
            }

            CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
            CDockingManager::SetDockingMode(DT_SMART);
    }

    RedrawWindow(nullptr, nullptr, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);
    theApp.WriteInt(_T("ApplicationLook"), theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(QAction *act)
{
    act->setChecked(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
    CFrameWndEx::OnSettingChange(uFlags, lpszSection);
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd *pParentWnd, CCreateContext *pContext)
{
    // базовый класс не работает
    if (!CFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
    {
        return FALSE;
    }

    // включить кнопку настройки для всех пользовательских панелей инструментов
    CString strCustomize;
    BOOL bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
    ASSERT(bNameValid);

    for (int i = 0; i < iMaxUserToolbars; ++i)
    {
        CMFCToolBar *pUserToolbar = GetUserToolBarByIndex(i);

        if (pUserToolbar != nullptr)
        {
            pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
        }
    }

    // единственно возможный вариант проверки на 2 копии.
    // недостаток - второй экземпляр полностью запускается, полностью отображается на экране.
    // после передачи параметров - закрывается. Как сделать, чтобы не мелькало - непонятно.
    // т.к. в общем-то параметры передавать надо не здесь. А вот обрабатывать - здесь.
    // может как-то удастся перенести всё это в БКАпп, и оттуда передавать сюда командную строку.
    if (!m_mtInstance.Lock(0))
    {
        // Если существует другой экземпляр эмулятора
        // Найдём другое окно
        ::PostMessage(HWND_BROADCAST, m_nInterAppGlobalMsg, QUESTION_PRIME_HWND, reinterpret_cast<LPARAM>(GetSafeHwnd()));
        Sleep(500); // Закроем приложение (в PreTranslateMessage)
    }

    return TRUE;
}
# endif

void CMainFrame::OnStartPlatform()
{
    // вызывается после создания CBKView и во время его первой инициализации
    // в функции CBKView::OnInitialUpdate()
    InitKbdStatus();

    if (ProcessFile(true)) // вот здесь только создаётся борда бкашки
    {
//		return S_OK;
    }
    else
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_MFBKBOARDERROR, MB_OK);
//		DestroyWindow(); // не создалось - ничего не можем сделать. выходим.
//		return S_FALSE;
    }
}

void CMainFrame::InitKbdStatus()
{
    m_paneBKVKBDView->SetKeyboardStatus(STATUS_FIELD::KBD_XLAT, false);
//    m_paneBKVKBDView->SetKeyboardStatus(STATUS_FIELD::KBD_CAPS, !(GetKeyState(VK_CAPITAL) & 1)); // зафиксируем текущее состояние капслока
    m_paneBKVKBDView->SetKeyboardStatus(STATUS_FIELD::KBD_AR2, false);
    m_paneBKVKBDView->SetKeyboardStatus(STATUS_FIELD::KBD_SU, false);
}

#if 0
LRESULT CMainFrame::OnDrawBKScreen(WPARAM, LPARAM)
{
    register auto view = DYNAMIC_DOWNCAST(CBKView, GetActiveView());
#if (BK_USE_CONVEYORTHREAD)

    if (view)
    {
        view->DrawScreen();
    }

#else

    if (view && view->IsWindowVisible())
    {
        view->Invalidate(FALSE);
    }

#endif
    return S_OK;
}

LRESULT CMainFrame::OnDebugDrawScreen(WPARAM wParam, LPARAM)
{
    // чтобы видеть как происходит движение луча в процессе прорисовки экрана
    // эти строки нужно закомментировать.
    // а иногда наоборот, очень неудобно когда экран по частям перерисовывается.
    // тогда эти строки надо раскомментировать
    // !!!TODO: может сделать чекбокс, с выбором способа отладочной перерисовки
    m_pScreen->SetRegister(m_pBoard->m_reg177664);
    m_pScreen->PrepareScreenRGB32(m_pBoard->GetMainMemory() + DWORD_PTR(wParam));
    register auto view = GetActiveView();

    if (view)
    {
        view->Invalidate(FALSE);
    }

    return S_OK;
}

LRESULT CMainFrame::OnDrawOscilloscope(WPARAM, LPARAM)
{
    if (m_paneOscillatorView.IsVisible())
    {
        m_paneOscillatorView.Invalidate(FALSE);
    }

    return S_OK;
}

void CMainFrame::InitRegistry()
{
    static initRegRaram regParam[5] =
    {
        // Register BinFile
        { _T("BKEmulator.BinFile"), _T("binary file"), _T("/b \"%1\""), IDR_MAINFRAME, IDS_FILEEXT_BINARY, true },
        // Register MemFile
        { _T("BKEmulator.MemFile"), _T("memory state"), _T("/m \"%1\""), IDI_MEMORY, IDS_FILEEXT_MEMORYSTATE, true },
        // Register TapeFle
        { _T("BKEmulator.TapeFile"), _T("tape file"), _T("/t \"%1\""), IDI_TAPE, IDS_FILEEXT_TAPE, true },
        // Register ScriptFile
        { _T("BKEmulator.ScriptFile"), _T("script file"), _T("/s \"%1\""), IDI_SCRIPT, IDS_FILEEXT_SCRIPT, true },
        // Register ROMFile
        { _T("BKEmulator.RomFile"), _T("ROM file"), _T(""), IDI_ROM, IDS_FILEEXT_ROM, false }
    };

    for (auto &i : regParam)
    {
        RegisterDefaultIcon(theApp.m_strProgramName, &i);

        if (i.bShell)
        {
            RegisterShellCommand(theApp.m_strProgramName, &i);
        }
    }
}

void CMainFrame::RegisterDefaultIcon(CString &strName, initRegRaram *regParam)
{
    CString strTitle(MAKEINTRESOURCE(IDS_EMUL_TITLE));
    CString strIcon = IntToString(regParam->iconID * -1, 10);
    CString strExt(MAKEINTRESOURCE(regParam->nExtStrID));
    HKEY hKey;
    LSTATUS hr = ::RegCreateKey(HKEY_CLASSES_ROOT, strExt, &hKey);
    hr = ::RegSetValue(hKey, nullptr, REG_SZ, regParam->strSection, 0);
    hr = ::RegCloseKey(hKey);
    hr = ::RegCreateKey(HKEY_CLASSES_ROOT, regParam->strSection, &hKey);
    hr = ::RegSetValue(hKey, nullptr, REG_SZ, strTitle + _T(" ") + regParam->strDescription, 0);
    hr = ::RegCloseKey(hKey);
    hr = ::RegCreateKey(HKEY_CLASSES_ROOT, regParam->strSection + _T("\\DefaultIcon"), &hKey);
    hr = ::RegSetValue(hKey, nullptr, REG_SZ, _T("\"") + g_Config.GetConfCurrPath() + strName + _T("\",") + strIcon, 0);
    hr = ::RegCloseKey(hKey);
}

void CMainFrame::RegisterShellCommand(CString &strName, initRegRaram *regParam)
{
    HKEY hKey;
    LSTATUS hr = ::RegCreateKey(HKEY_CLASSES_ROOT, regParam->strSection + _T("\\Shell"), &hKey);
    hr = ::RegCloseKey(hKey);
    hr = ::RegCreateKey(HKEY_CLASSES_ROOT, regParam->strSection + _T("\\Shell\\Open"), &hKey);
    hr = ::RegCloseKey(hKey);
    hr = ::RegCreateKey(HKEY_CLASSES_ROOT, regParam->strSection + _T("\\Shell\\Open\\Command"), &hKey);
    hr = ::RegSetValue(hKey, nullptr, REG_SZ, _T("\"") + g_Config.GetConfCurrPath() + strName + _T("\" ") + regParam->strArguments, 0);
    hr = ::RegCloseKey(hKey);
}

BOOL CMainFrame::PreTranslateMessage(MSG *pMsg)
{
    if (pMsg->message == m_nInterAppGlobalMsg)
    {
        HWND hwnd = GetSafeHwnd();

        // обработка сообщения, которым обмениваются между собой копии приложения
        switch (pMsg->wParam)
        {
            case QUESTION_PRIME_HWND:
            {
                // основная прога обрабатывает запрос копии
                // если другая копия проги спрашивает эту копию, надо ей ответить, и послать
                // хэндл окна этой копии
                auto copy_hwnd = reinterpret_cast<HWND>(pMsg->lParam);

                // поскольку мы разослали сообщение брудкастом, то и сами себе его получили, поэтому
                // если copy_hwnd == hwnd, то получили сообщение сами от себя, нафиг оно нам надо?
                if (copy_hwnd != hwnd)
                {
                    // копия проги не должна самой себе посылать ответ
                    ::PostMessage(copy_hwnd, m_nInterAppGlobalMsg, ANSWER_PRIME_HWND, reinterpret_cast<LPARAM>(hwnd));
                }

                return TRUE;
            }

            case ANSWER_PRIME_HWND:
            {
                // копия обрабатывает ответ от основной проги
                // тут мы получили ответ от первой копии проги, получили хэндл окна
                CString strCommands = GetCommandLine();
                // теперь наша задача, отправить параметры командной строки основной копии проги.
                //
                // Fill the COPYDATA structure
                //
                TCHAR pInstanceCommandPrompt[1024];
                memcpy(pInstanceCommandPrompt, strCommands.GetBuffer(sizeof(pInstanceCommandPrompt)), sizeof(pInstanceCommandPrompt));
                COPYDATASTRUCT cd;
                cd.dwData = 2;
                cd.cbData = sizeof(pInstanceCommandPrompt);
                cd.lpData = reinterpret_cast<PVOID>(pInstanceCommandPrompt);
                // вызываем функцию, передающую данные в &cd основной копии проги
                ::SendMessage(reinterpret_cast<HWND>(pMsg->lParam), WM_COPYDATA, reinterpret_cast<WPARAM>(hwnd), reinterpret_cast<LPARAM>(&cd));
                strCommands.ReleaseBuffer();
                DestroyWindow(); // всё, делать больше нечего. киляемся
                return TRUE;
            }
        }
    }

    if (pMsg->message == WM_SYSKEYDOWN && pMsg->wParam == VK_MENU)
    {
        // Если нажали любой Alt
        return FALSE;   // будем обрабатывать сами
    }

    if (pMsg->message == WM_SYSKEYUP && pMsg->wParam == VK_MENU)
    {
        // Если отжали любой Alt
        return FALSE;   // будем обрабатывать сами
    }

    return CFrameWndEx::PreTranslateMessage(pMsg);
}


BOOL CMainFrame::OnCopyData(CWnd *pWnd, COPYDATASTRUCT *pCopyDataStruct)
{
    if (pCopyDataStruct->dwData == 2)
    {
        CString strLine(reinterpret_cast<LPCTSTR>(pCopyDataStruct->lpData), pCopyDataStruct->cbData);
        bool bRes = false;

        if (ParseCommandLineParameters(strLine))
        {
            bRes = ProcessFile(false);
        }

        SetForegroundWindow();
        return bRes;
    }

    return CFrameWnd::OnCopyData(pWnd, pCopyDataStruct);
}


bool CMainFrame::ParseCommandLineParameters(CString strCommands)
{
    ClearProcessingFiles();
    int nFindPos = strCommands.Find(_T('/'));

    if (nFindPos == -1)
    {
        return true; // ключей нету, разбирать нечего, просто выходим.
    }

    // если ключи есть, оставляем только их.
    strCommands = strCommands.Right(strCommands.GetLength() - max(0, nFindPos));
    int nStrLen; // текущая длина оставшейся неразобранной строки

    while (nStrLen = strCommands.GetLength())
    {
        nFindPos = strCommands.Find(_T('/'), 1); // ищем, есть ли ещё ключи

        if (nFindPos == -1) // если нету
        {
            nFindPos = nStrLen; // то всё что есть - параметр данного ключа
        }

        CString strParam = strCommands.Left(nFindPos); // выделяем параметр в отдельную строку
        strCommands = strCommands.Right(nStrLen - nFindPos); // и отсекаем их от обрабатываемой строки.
        strParam.Trim(); // уберём пробелы, если есть
        TCHAR command = toupper(strParam[1]); // получим значение ключа
        strParam = strParam.Right(strParam.GetLength() - 2); // выделим аргументы
        strParam.Trim(); // снова уберём пробелы. теперь они точно есть
        strParam.Trim(_T('\"')); // а ещё кавычки

        switch (command)
        {
            case _T('B'): // Binary file
            {
                m_strBinFileName = ::GetFileName(strParam);
                CString strFilePath = ::GetFilePath(strParam);

                if (!strFilePath.IsEmpty())
                {
                    g_Config.m_strBinPath = strFilePath;
                }

                m_strScriptFileName = _T("autorun\\monitor_load.bkscript");
                m_Script.SetArgument(::GetFileTitle(m_strBinFileName));
            }
            break;

            case _T('M'): // Memory state file
            {
                m_strMemFileName = ::GetFileName(strParam);
                CString strFilePath = ::GetFilePath(strParam);

                if (!strFilePath.IsEmpty())
                {
                    g_Config.m_strMemPath = strFilePath;
                }
            }
            break;

            case _T('T'): // BK tape file
            {
                m_strTapeFileName = ::GetFileName(strParam);
                CString strFilePath = ::GetFilePath(strParam);

                if (!strFilePath.IsEmpty())
                {
                    g_Config.m_strTapePath = strFilePath;
                }
            }
            break;

            case _T('S'): // Initial Script file
            {
                m_strScriptFileName = ::GetFileName(strParam);
                CString strFilePath = ::GetFilePath(strParam);

                if (!strFilePath.IsEmpty())
                {
                    g_Config.m_strScriptsPath = strFilePath;
                }
            }
            break;

            case _T('?'):
            case _T('H'):
            {
                CString strTitle(MAKEINTRESOURCE(IDS_EMUL_TITLE));
                CString strPrompt(MAKEINTRESOURCE(IDS_COMMAND_PROMPT_INFO));
                g_BKMsgBox.Show(strTitle + strPrompt, MB_OK);
                return true;
            }

            case _T('W'):
                // не знаю что такое, так и было
                break;
        }
    }

    return true;
}
#endif

bool CMainFrame::StartPlayTape(const CString &strPath)
{
    CString strExt = ::GetFileExt(strPath);
    TAPE_FILE_INFO tfi;
    memset(&tfi, 255, sizeof(TAPE_FILE_INFO));

    if (!strExt.CompareNoCase(CString(MAKEINTRESOURCE(IDS_FILEEXT_WAVE))))
    {
        m_tape.LoadWaveFile(strPath);
    }
    else if (!strExt.CompareNoCase(CString(MAKEINTRESOURCE(IDS_FILEEXT_TAPE))))
    {
        m_tape.LoadMSFFile(strPath);
    }
    else if (!strExt.CompareNoCase(CString(MAKEINTRESOURCE(IDS_FILEEXT_BINARY))))
    {
        m_tape.LoadBinFile(strPath, &tfi);
    }
    else
    {
        return false;
    }

    m_tape.GetWaveFile(&tfi); // вычисляем и заполняем внутренние переменные
    m_tape.SetWaveLoaded(true);
    UpdateTapeDlgControls();
//	m_paneTapeCtrlView.StartPlayTape();
    return true;
}


void CMainFrame::UpdateTapeDlgControls()
{
//	m_paneTapeCtrlView.UpdateTapeControls(g_Config.m_bEmulateLoadTape, g_Config.m_bEmulateSaveTape);
}

void CMainFrame::closeEvent(QCloseEvent *event)
{
    OnClose();
    QMainWindow::closeEvent(event);
//    event->accept();
}

void CMainFrame::OnClose() // OnClose()
{
    // тут надо занести в конфиг разные переменные и параметры опций, которые надо сохранить
    g_Config.m_nCPUFrequency = m_pBoard->GetCPUSpeed();
//    g_Config.m_nDisasmAddr = m_pDebugger->GetCurrentAddress();
//	g_Config.m_nDumpAddr = m_paneMemoryDumpView.GetDumpAddress();
    StopAll();
    CheckDebugMemmap(); // если карта памяти была открыта - закроем
    QMainWindow::close();
}

//void CMainFrame::OnTimer(UINT_PTR nIDEvent)
//{
//	switch (nIDEvent)
//	{
//		case BKTIMER_UI_REFRESH:
//			OnMainLoop();
//			break;

//		case BKTIMER_UI_TIME:
//			OnMainLoopTime();
//			break;
//	}
//}

/////////////////////////////////////////////////////////////////////////////
// по идее, вызывается каждые 20 миллисекунд, обновляет информацию в
// пользовательском интерфейсе
void CMainFrame::OnMainLoop()
{
    if (m_pBoard)   // работаем только если есть матплата
    {
        // карту памяти тоже тут обновляем
        if (m_bBKMemViewOpen)
        {
//			m_pBKMemView->DrawTab();
        }

        // Парсим скрипт, если он есть
        m_Script.RunScript();
        m_pBoard->FrameParam();  // применяем новые параметры

        // если проц остановлен, то информацию не обновлять, т.к. она принудительно обновляется при пошаговой отладке.
        // а если работает - то обновлять
        if (!m_pBoard->IsCPUBreaked())
        {
            // это имеет смысл только в информативных целях, иначе сильно проседает производительность
            // на слабых конфигурациях. причём эта штука должна быть именно здесь.
            if (g_Config.m_nRegistersDumpInterval && !m_pBKView->IsFullScreenMode())
            {
                if (--m_nRegsDumpCounter <= 0)
                {
                    m_nRegsDumpCounter = g_Config.m_nRegistersDumpInterval;
                    SetDebugCtrlsState();
                }
            }
        }
    }
}

void CMainFrame::OnMainLoopTime()
{
    register DWORD nTmpTick = GetTickCount();
    nTmpTick = (nTmpTick - m_nStartTick) / 1000;
    // таймер аптайма. поточнее, но из-за того, что OnMainLoopTime() вызывается не точно через 1000мс,
    // и сюда попадаем иногда больше чем через секунду, то при выводе иногда счётчик перескакивает
    // через секунду. Но тем не менее, тут показывается реальное время работы, а не счётчик попаданий,
    // как раньше.
    register DWORD seconds = nTmpTick % 60;
    register DWORD minutes = (nTmpTick / 60) % 60;
    register DWORD hours = nTmpTick / (60 * 60);

// вот эта штука возвращает фокус программе в полоэкранном режиме, после нажатий на разного рода
// управляющие клавиши, которые не обрабатываются эмулятором и приводят к потере фокуса эмулятором
//  if (m_pScreen->IsFullScreenMode()) // если мы в полноэкранном режиме
//  {
// #ifdef TARGET_WINXP
//      // для D3D9 таких проблем нету
//      SetFocusToBK(); // будем принудительно передавать фокус Экрану
// #else
//
//      // если в полноэкранном режиме и рендер не D3D, то только тогда
//      if (g_Config.m_nScreenRenderType != CONF_SCREEN_RENDER_D3D)
//      {
//          SetFocusToBK(); // будем принудительно передавать фокус Экрану
//      }
//
//      // Тут возникла проблема с полноэкранным режимом D3D11.
//      // если мы передаём фокус CBKView - вываливаемся из режима. Потому что
//      // в полноэкранном режиме фокус должен быть у CScreen, как только он будет потерян
//      // тут же выпадаем из полноэкранного режима
// #endif
//  }

    if (g_Config.m_bShowPerformance) // если в оконном и включена опция показа информации
    {
        // Показываем производительность в строке состояния
        CString strPerformance;
        strPerformance.Format(_T("Scr: %d x %d, FPS: %i, CPU: %d Hz, Uptime %02i:%02i:%02i"), m_nScreen_X, m_nScreen_Y, m_pBKView->GetFPS(), m_pBoard->GetCPUFreq(), hours, minutes, seconds);
//        m_wndStatusBar.SetPaneText(static_cast<int>(STATUS_FIELD::INFO), strPerformance.GetString());
        m_pSB_Common->setText(strPerformance);
    }
}

#if 0
LRESULT CMainFrame::OnDropFile(WPARAM wParam, LPARAM lParam)
{
    auto strPath = reinterpret_cast<CString *>(lParam);
    ClearProcessingFiles();
    CString strFilePath = ::GetFilePath(*strPath);
    CString strFileExt = ::GetFileExt(*strPath);
    CString strBinExt(MAKEINTRESOURCE(IDS_FILEEXT_BINARY));
    CString strMemExt(MAKEINTRESOURCE(IDS_FILEEXT_MEMORYSTATE));
    CString strTapExt(MAKEINTRESOURCE(IDS_FILEEXT_TAPE));
    CString strWavExt(MAKEINTRESOURCE(IDS_FILEEXT_WAVE));
    CString strScptExt(MAKEINTRESOURCE(IDS_FILEEXT_SCRIPT));

    if (!strFileExt.CompareNoCase(strBinExt))  // Binary file
    {
        m_strBinFileName = ::GetFileName(*strPath);

        if (!strFilePath.IsEmpty())
        {
            g_Config.m_strBinPath = strFilePath;
        }

        m_strScriptFileName = _T("autorun\\monitor_load.bkscript");
        m_Script.SetArgument(::GetFileTitle(m_strBinFileName));
    }
    else if (!strFileExt.CompareNoCase(strMemExt))  // Memory state file
    {
        m_strMemFileName = ::GetFileName(*strPath);

        if (!strFilePath.IsEmpty())
        {
            g_Config.m_strMemPath = strFilePath;
        }
    }
    else if (strFileExt.CompareNoCase(strTapExt) == 0
             || strFileExt.CompareNoCase(strWavExt) == 0)  // BK tape file
    {
        m_strTapeFileName = ::GetFileName(*strPath);

        if (!strFilePath.IsEmpty())
        {
            g_Config.m_strTapePath = strFilePath;
        }
    }
    else if (!strFileExt.CompareNoCase(strScptExt))  // Initial Script file
    {
        m_strScriptFileName = ::GetFileName(*strPath);

        if (!strFilePath.IsEmpty())
        {
            g_Config.m_strScriptsPath = strFilePath;
        }
    }
    else
    {
        ASSERT(false);
        return S_FALSE;
    }

    ProcessFile(false); // здесь если надо создаётся новая борда бкашки, а если нет - остаётся текущая
    return S_OK;
}

#endif

void CMainFrame::ClearProcessingFiles()
{
    m_strBinFileName.Empty();
    m_strMemFileName.Empty();
    m_strTapeFileName.Empty();
    m_strScriptFileName.Empty();
}

#if 0
LRESULT CMainFrame::OnOutKeyboardStatus(WPARAM wParam, LPARAM lParam)
{
    auto pane = static_cast<int>(wParam);
    auto str = reinterpret_cast<CString *>(lParam);
    m_wndStatusBar.SetPaneText(pane, *str);
    return S_OK;
}
#endif

void CMainFrame::SetupConfiguration(CONF_BKMODEL nConf)
{
    if (ConfigurationConstructor(nConf))
    {
        SetFocusToBK();
        repaintToolBars();
    }
    else
    {
        // надо вообще закрыть прогу. нечего тут делать.
        g_BKMsgBox.Show(IDS_BK_ERROR_MFBKBOARDERROR, MB_OK);
//        theApp.ExitInstance();
        exit(1);
    }
}

// Вход: bCreate = true - вызывается из OnCreate, нужно создавать новую конфигурацию
//              = false - вызывается из OnDrop, нужно использовать существующую конфигурацию
bool CMainFrame::ProcessFile(bool bCreate)
{
    m_Script.SetScriptPath(g_Config.m_strScriptsPath, m_strScriptFileName, /*m_paneBKVKBDView.GetXLatStatus()*/ true);
    bool bRes = false;

    // Инициализация БК чипа
    if (!m_strMemFileName.IsEmpty()) // если задан .msf файл, то пробуем загрузить его
    {
        // Если не можем загрузить msf файл
        if (!(bRes = LoadMemoryState(g_Config.m_strMemPath + m_strMemFileName)))
        {
            bRes = ConfigurationConstructor(CONF_BKMODEL::BK_0010_01);    // Создаём конфигурацию по умолчанию
        }
    }
    else
    {
        // иначе, смотрим, что там.

        // если вызвали из ДрагнДропа или
        // если вызвали из оболочки виндовс по ассоциации - то
        // Если задан bin файл
        if (!m_strBinFileName.IsEmpty())
        {
            CString tmp = g_Config.m_strBinPath;
            bRes = ConfigurationConstructor(CONF_BKMODEL::BK_0010_01);    // Создаём конфигурацию по умолчанию
            g_Config.m_strBinPath = tmp; // восстанавливаем свой путь к бин файлу
        }
        else
        {
            if (bCreate) // если вызвали из onCreate - то создаём заданную модель
            {
                bRes = ConfigurationConstructor(g_Config.GetBKModelNumber());  // Создаём заданную конфигурацию
            }

            // иначе вообще ничего не создаём, т.е. дропнули что-то, что не требует пересоздания конфигурации
        }
    }

    if (!bCreate || bRes) // если конфигурация текущая, или была создана новая.
    {
        // Если задан файл ленты, запускаем его играться
        if (!m_strTapeFileName.IsEmpty())
        {
            StartPlayTape(g_Config.m_strTapePath + m_strTapeFileName);
        }
    }

    SetFocusToBK();
    return bRes;
}

void CMainFrame::InitEmulator()
{
    CString str;
    str.LoadString(g_mstrConfigBKModelParameters[static_cast<int>(g_Config.GetBKModelNumber())].nIDBKModelName);
//    UpdateFrameTitleForDocument(str);

    switch (g_Config.m_nVKBDType)
    {
        default:
        case 0:
            m_paneBKVKBDView->SetKeyboardView(IDB_BITMAP_SOFT);
            break;

        case 1:
            m_paneBKVKBDView->SetKeyboardView(IDB_BITMAP_PLEN);
            break;
    }

//    auto pSlider = DYNAMIC_DOWNCAST(CSliderButton, m_wndToolBarSound.GetButton(m_wndToolBarSound.CommandToIndex(ID_OPTIONS_SOUND_VOLUME)));

    if (m_pVolumeSlider)
    {
        m_pVolumeSlider->setValue(g_Config.m_nSoundVolume);
    }

    m_pSound->SoundGen_SetVolume(g_Config.m_nSoundVolume); // продублируем для надёжности
    // инициализация текущих настроек
    m_speaker.EnableSound(g_Config.m_bSpeaker);
    m_speaker.SetFilter(g_Config.m_bSpeakerFilter);
    m_speaker.SetDCOffsetCorrect(g_Config.m_bSpeakerDCOffset);
    m_covox.EnableSound(g_Config.m_bCovox);
    m_covox.SetFilter(g_Config.m_bCovoxFilter);
    m_covox.SetDCOffsetCorrect(g_Config.m_bCovoxDCOffset);
    m_covox.SetStereo(g_Config.m_bStereoCovox);
    m_menestrel.EnableSound(g_Config.m_bMenestrel);
    m_menestrel.SetFilter(g_Config.m_bMenestrelFilter);
    m_menestrel.SetDCOffsetCorrect(g_Config.m_bMenestrelDCOffset);
    m_menestrel.SetStereo(true); // всегда стерео
    m_ay8910.EnableSound(g_Config.m_bAY8910);
    m_ay8910.SetFilter(g_Config.m_bAY8910Filter);
    m_ay8910.SetDCOffsetCorrect(g_Config.m_bAY8910DCOffset);
    m_ay8910.SetStereo(true); // всегда стерео
    m_tape.SetWaveParam(g_Config.m_nSoundSampleRate, BUFFER_CHANNELS);
    // параметры экрана
    m_pScreen->SetSmoothing(g_Config.m_bSmoothing);
    m_pScreen->SetColorMode(g_Config.m_bColorMode);
    m_pScreen->SetAdaptMode(g_Config.m_bAdaptBWMode);
    m_pScreen->SetLuminoforeEmuMode(g_Config.m_bLuminoforeEmulMode);
    g_Config.m_bFullscreenMode ? m_pBKView->SetFullScreenMode() : m_pBKView->SetWindowMode();
    // Настройка панели управления записью
//    m_paneTapeCtrlView.InitParams(&m_tape);
    UpdateTapeDlgControls();
    // наглядно отобразим, что и в каком дисководе находится
    UpdateToolbarDriveIcons();
    InitKbdStatus(); // переинициализируем состояния клавиатуры
    m_pDebugger->SetCurrentAddress(g_Config.m_nDisasmAddr); // Обновим окно дизассемблера
//    m_paneMemoryDumpView.SetDumpAddress(g_Config.m_nDumpAddr); // Обновим окно дампа памяти
//    UpdateData(FALSE);
}

// выносим это в отдельную процедуру, т.к. есть вероятность неудачного прочтения MSF,
// при этом надо восстановить всё как было раньше
void CMainFrame::AttachObjects()
{
//    auto vw = DYNAMIC_DOWNCAST(CBKView, GetActiveView());

//    if (vw)
//    {
//        vw->ReCreateSCR(); // пересоздаём экран с новыми параметрами.
//    }

    int nMtc = m_pSound->ReInit(); // пересоздаём звук с новыми параметрами, на выходе - длина медиафрейма в сэмплах
    m_speaker.ReInit(); // ещё надо переинициализирвоать устройства, там
    m_speaker.ConfigureTapeBuffer(nMtc);// переопределяем буферы в зависимости от текущей частоты дискретизации
    m_covox.ReInit();   // есть вещи, зависящие от частоты дискретизации,
    m_menestrel.ReInit();   //
    m_ay8910.ReInit();  // которая теперь величина переменная. Но требует перезапуска конфигурации.
//    m_paneOscillatorView.ReCreateOSC(); // пересоздаём осциллограф с новыми параметрами
//    m_paneOscillatorView.SetBuffer(nMtc); // при необходимости откорректируем размер приёмного буфера.

    if (m_pBoard)
    {
        m_pBoard->AttachWindow(this);  // цепляем к MotherBoard этот класс
        // порядок имеет значение. сперва нужно делать обязательно AttachWindow(this)
        // и только затем m_pBoard->SetMTC(). И эта функция обязательна, там звуковой буфер вычисляется
        // и выделяется
        m_pBoard->SetMTC(nMtc); // и здесь ещё. тройная работа получается.
        // Присоединяем к новосозданному чипу устройства
        m_pBoard->AttachSound(m_pSound);
        m_pBoard->AttachSpeaker(&m_speaker);
        m_pBoard->AttachMenestrel(&m_menestrel);
        m_pBoard->AttachCovox(&m_covox);
        m_pBoard->AttachAY8910(&m_ay8910);

        // если в ини файле задана частота, то применим её, вместо частоты по умолчанию.
        if (g_Config.m_nCPUFrequency)
        {
            m_pBoard->SetCPUFreq(g_Config.m_nCPUFrequency);
        }

        // Цепляем к новому чипу отладчик, т.е. наоборот, к отладчику чип
        m_pDebugger->AttachBoard(m_pBoard);
//        m_paneRegistryDumpViewCPU.SetFreqParam();
        // Цепляем обработчик скриптов
        m_Script.AttachBoard(m_pBoard);
    }
}

bool CMainFrame::ConfigurationConstructor(CONF_BKMODEL nConf, bool bStart)
{
	bool bReopenMemMap = false;

	if (m_pBoard)
	{
		// Если конфигурация уже существует, удалим её
		StopAll();  // сперва всё остановим
		bReopenMemMap = CheckDebugMemmap(); // флаг переоткрытия карты памяти
		// перед сохранением настройки флагов заберём из диалога
		g_Config.m_nCPUFrequency = m_pBoard->GetCPUSpeed();
		g_Config.SaveConfig();
		SAFE_DELETE(m_pBoard);   // удалим конфигурацию
	}

	g_Config.SetBKModelNumber(nConf);

	// создадим новую конфигурацию
	switch (g_Config.m_BKBoardModel)
	{
		case MSF_CONF::BK1001:
			m_pBoard = new CMotherBoard;
			break;

		case MSF_CONF::BK1001_MSTD:
			m_pBoard = new CMotherBoard_MSTD;
			break;

		case MSF_CONF::BK1001_EXT32:
			m_pBoard = new CMotherBoard_EXT32;
			break;

		case MSF_CONF::BK1001_FDD:
			m_pBoard = new CMotherBoard_10_FDD;
			break;

		case MSF_CONF::BK11:
			m_pBoard = new CMotherBoard_11;
			break;

		case MSF_CONF::BK11_FDD:
			m_pBoard = new CMotherBoard_11_FDD;
			break;

		case MSF_CONF::BK11M:
			m_pBoard = new CMotherBoard_11M;
			break;

		case MSF_CONF::BK11M_FDD:
			m_pBoard = new CMotherBoard_11M_FDD;
			break;

		default:
			ASSERT(false);
			return false;
	}

	if (!m_pBoard)
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
		return false;
	}

	m_pBoard->SetFDDType(g_Config.m_BKFDDModel);
	g_Config.LoadConfig(false); // Читаем из ини файла параметры
	AttachObjects();    // пересоздадим и присоединим необходимые устройства.

	if (!m_pBoard->InitBoard(g_Config.m_nCPURunAddr))
	{
		// если ресет не удался - значит не удалось проинициализировать
		// память - значит не удалось загрузить какие-то дампы прошивок -
		// значит дальше работать невозможно.
		SAFE_DELETE(m_pBoard);
		return false;
	}

	InitEmulator();     // переинициализируем модель

//    if (bReopenMemMap)      // если надо
//    {
//        OnDebugMemmap();    // заново откроем карту памяти
//    }

	if (bStart)
	{
		m_pBoard->StartTimerThread();
		StartTimer();
		// Запускаем CPU
		m_pBoard->RunCPU();

		// если не установлен флаг остановки после создания
		if (g_Config.m_bPauseCPUAfterStart)
		{
			m_pBoard->BreakCPU();
		}
	}

	SetFocusToBK();
	return true;
}
// упрощённый вариант функции для загрузки конфигурации
bool CMainFrame::ConfigurationConstructor_LoadConf(CONF_BKMODEL nConf)
{
	g_Config.SetBKModelNumber(nConf);

	// создадим новую конфигурацию
	switch (g_Config.m_BKBoardModel)
	{
		case MSF_CONF::BK1001:
			m_pBoard = new CMotherBoard;
			break;

		case MSF_CONF::BK1001_MSTD:
			m_pBoard = new CMotherBoard_MSTD;
			break;

		case MSF_CONF::BK1001_EXT32:
			m_pBoard = new CMotherBoard_EXT32;
			break;

		case MSF_CONF::BK1001_FDD:
			m_pBoard = new CMotherBoard_10_FDD;
			break;

		case MSF_CONF::BK11:
			m_pBoard = new CMotherBoard_11;
			break;

		case MSF_CONF::BK11_FDD:
			m_pBoard = new CMotherBoard_11_FDD;
			break;

		case MSF_CONF::BK11M:
			m_pBoard = new CMotherBoard_11M;
			break;

		case MSF_CONF::BK11M_FDD:
			m_pBoard = new CMotherBoard_11M_FDD;
			break;

		default:
			ASSERT(false);
			return false;
	}

	if (!m_pBoard)
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
		return false;
	}

	m_pBoard->SetFDDType(g_Config.m_BKFDDModel);
	// присоединим устройства, чтобы хоть что-то было для выполнения ResetHot
	AttachObjects();

	if (!m_pBoard->InitBoard(g_Config.m_nCPURunAddr))
	{
		// если ресет не удался - значит не удалось проинициализировать
		// память - значит не удалось загрузить какие-то дампы прошивок -
		// значит дальше работать невозможно.
		SAFE_DELETE(m_pBoard);
		return false;
	}

	SetFocusToBK();
	return true;
}

#if 0
LRESULT CMainFrame::OnResetKbdManager(WPARAM, LPARAM)
{
    InitKbdStatus();
    auto vw = DYNAMIC_DOWNCAST(CBKView, GetActiveView());

    if (vw)
    {
        vw->ClearKPRS();
    }

    m_pBoard->m_reg177660 &= ~0200;  // ещё удалим флаг готовности нового кода
    m_pBoard->m_reg177716in |= 0100;  // ещё сбросим флаг нажатия клавиши
    SetFocusToBK();
    return S_OK;
}
#endif

bool CMainFrame::LoadMemoryState(const CString &strPath)
{
	CMSFManager msf;
	bool bRet = false;

	if (!msf.OpenFile(strPath, true))
	{
		return bRet;
	}

	if (msf.GetType() == MSF_STATE_ID && msf.GetVersion() >= MSF_VERSION_MINIMAL)
	{
		StopAll();
		bool bReopenMemMap = CheckDebugMemmap(); // флаг переоткрытия карты памяти

		// временно выгрузим все образы дискет и винчестеров.
		if (m_pBoard->GetFDDType() != BK_DEV_MPI::NONE)
		{
			m_pBoard->GetFDD()->DetachDrives();
		}

		// Сохраняем старую конфигурацию
		CMotherBoard *pOldBoard = m_pBoard;
		CONF_BKMODEL nOldConf = g_Config.GetBKModelNumber();
		m_pBoard = nullptr;

		if (ConfigurationConstructor_LoadConf(msf.GetConfiguration()))
		{
			if (m_pBoard->RestoreState(msf, nullptr))
			{
				SAFE_DELETE(pOldBoard);
				bRet = true;
			}
			else
			{
				// не удалось восстановить состояние, надо вернуть старую конфигурацию.
				g_BKMsgBox.Show(IDS_ERRMSF_WRONG, MB_OK);
				SAFE_DELETE(m_pBoard);
				m_pBoard = pOldBoard;
				g_Config.SetBKModelNumber(nOldConf);
				g_Config.LoadConfig();      // восстановим из ини файла параметры
			}

			// приаттачим все образы дискет и винчестеров.
			if (m_pBoard->GetFDDType() != BK_DEV_MPI::NONE)
			{
				m_pBoard->GetFDD()->AttachDrives();
			}

			AttachObjects();            // переприсоединим устройства, уже с такой, какой надо конфигурацией
			InitEmulator();             // переинициализируем модель
		}
		else
		{
			// Неподдерживаемая конфигурация, или ошибка при создании
			g_BKMsgBox.Show(IDS_ERRMSF_WRONG, MB_OK);
			m_pBoard = pOldBoard;
			g_Config.SetBKModelNumber(nOldConf);
			AttachObjects();
		}

		if (bReopenMemMap && !m_bBKMemViewOpen)
		{
//			OnDebugMemmap();
		}

		StartAll();
	}
	else
	{
		g_BKMsgBox.Show(IDS_ERRMSF_OLD, MB_OK);
	}

	return bRet;
}


bool CMainFrame::SaveMemoryState(const CString &strPath)
{
	if (m_pBoard)
	{
		CMSFManager msf;
		msf.SetConfiguration(g_Config.GetBKModelNumber());

		if (!msf.OpenFile(strPath, false))
		{
			return false;
		}

		StopTimer();
		m_pBoard->StopCPU(false);
		m_pBoard->RestoreState(msf, m_pScreen->GetScreenshot());
		m_pBoard->RunCPU(false);
		StartTimer();
	}

	return true;
}

/* оставим код на память.
bool CMainFrame::MakeScreenShot()
{
    bool bRet = false;
    // это не совсем честно. на самом деле BK_SCREEN_WIDTH х BK_SCREEN_HEIGHT - это viewport экрана
    // а оригинальный размер формируемой текстуры экрана - 512х256
    // просто такой скриншот выглядит не очень красиво, слишком мелкий и сплющенный по высоте.
    HBITMAP hBitmap = (HBITMAP)CopyImage(m_pScreen->GetScreenshot(), IMAGE_BITMAP, BK_SCREEN_WIDTH, BK_SCREEN_HEIGHT, 0);

    CString strName;
    strName.Format(_T("screenshot_%d.bmp"), g_Config.m_nScreenshotNumber++);
    CString szFileName = g_Config.m_strScreenShotsPath + strName;

    CFile File;
    BITMAPINFO *pbmi;
    BITMAPFILEHEADER bfh;
    BITMAP bmp;

    // Получение параметров рисунка
    if (!GetObject(hBitmap, sizeof(bmp), &bmp))
        return false;

    // Количество битов под пиксель
    uint16_t wClrBits = (uint16_t)(bmp.bmPlanes*bmp.bmBitsPixel);
    if (wClrBits == 1);
    else if (wClrBits <= 4)
        wClrBits = 4;
    else if (wClrBits <= 8)
        wClrBits = 8;
    else if (wClrBits <= 16)
        wClrBits = 16;
    else if (wClrBits <= 24)
        wClrBits = 24;
    else if (wClrBits <= 32)
        wClrBits = 32;

    // Выделение памяти для BITMAPINFO
    if (wClrBits != 24)
    {
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * (1<<wClrBits));
    }
    else
    {
        pbmi = (PBITMAPINFO)LocalAlloc(LPTR, sizeof(BITMAPINFOHEADER));
    }
    // Заполнение BITMAPINFO
    pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    pbmi->bmiHeader.biHeight = bmp.bmHeight;
    pbmi->bmiHeader.biWidth = bmp.bmWidth;
    pbmi->bmiHeader.biPlanes = bmp.bmPlanes;
    pbmi->bmiHeader.biBitCount = bmp.bmBitsPixel;
    if (wClrBits < 24)
        pbmi->bmiHeader.biClrUsed = (1<<wClrBits);
    pbmi->bmiHeader.biCompression = BI_RGB;

    // ..количество памяти, необходимое для таблицы цветов
    pbmi->bmiHeader.biSizeImage = ((pbmi->bmiHeader.biWidth * wClrBits + 31) & ~31) / 8 * pbmi->bmiHeader.biHeight;
    pbmi->bmiHeader.biClrImportant = 0;

    // Получение памяти под таблицу цветов и массив индексов
    LPBYTE lpData = new uint8_t[pbmi->bmiHeader.biSizeImage];
    if (lpData)
    {
        // Копирование таблицы цветов в выделенную область памяти
        HDC hDC = GetDC()->GetSafeHdc();
        HDC hTempDC = CreateCompatibleDC(hDC);

        SelectObject(hTempDC, hBitmap);
        if (GetDIBits(hTempDC, hBitmap, 0, (uint16_t)bmp.bmHeight, lpData, pbmi, DIB_RGB_COLORS))
        {
            // Создание файла
            _tmkdir(g_Config.m_strScreenShotsPath);
            if (File.Open(szFileName, CFile::modeCreate | CFile::shareExclusive | CFile::modeWrite))
            {
                // Заполняем структуру информации о файле
                bfh.bfType = 0x4d42;
                // Смещение данных рисунка от начала файла
                bfh.bfOffBits = (DWORD)(sizeof(BITMAPFILEHEADER) + pbmi->bmiHeader.biSize +
                    pbmi->bmiHeader.biClrUsed * sizeof(RGBQUAD));
                bfh.bfSize = bfh.bfOffBits + pbmi->bmiHeader.biSizeImage;
                bfh.bfReserved1 = 0;
                bfh.bfReserved2 = 0;

                // Запись данных в файл, ошибки записи проигнорируем. ну и их нафиг, скриншот же.
                // ..BITMAPFILEHEADER
                File.Write(&bfh, sizeof(bfh));
                // ..BITMAPINFOHEADER и RGBQUAD
                File.Write(&pbmi->bmiHeader, sizeof(BITMAPINFOHEADER) + pbmi->bmiHeader.biClrUsed * sizeof(RGBQUAD));
                // ..массив цветов и индексов
                File.Write(lpData, pbmi->bmiHeader.biSizeImage);
                // Запись закончена
                File.Close();
                bRet = true;
            }
        }
        DeleteDC(hTempDC);
        delete lpData;
    }
    DeleteObject(hBitmap);
    return bRet;
}
*/

bool CMainFrame::MakeScreenShot()
{
    bool bRet = false;
#if 0
    QPixmap hBitmap;

    if (g_Config.m_bOrigScreenshotSize)
    {
        hBitmap = m_pScreen->GetScreenshot();
    }
    else
    {
        // это не совсем честно. на самом деле BK_SCREEN_WIDTH х BK_SCREEN_HEIGHT - это viewport экрана
        // а оригинальный размер формируемой текстуры экрана - 512х256
        // просто такой скриншот выглядит не очень красиво, слишком мелкий и сплющенный по высоте.
        hBitmap = (HBITMAP)CopyImage(m_pScreen->GetScreenshot(), IMAGE_BITMAP, CScreen::BK_SCREEN_WIDTH, CScreen::BK_SCREEN_HEIGHT, LR_COPYDELETEORG);
    }

    if (hBitmap)
    {
        CString strName;
        strName.Format(_T("screenshot_%d.png"), g_Config.m_nScreenshotNumber++);
        CString szFileName = g_Config.m_strScreenShotsPath + strName;

        if (OpenClipboard())
        {
            HANDLE hRet = SetClipboardData(CF_BITMAP, hBitmap);
#ifdef _DEBUG

            if (!hRet) // нужно для отладки, когда что-то идёт не так
            {
                GetLastErrorOut(_T("SetClipboardData"));
            }

#endif
            CloseClipboard();
        }

        _tmkdir(g_Config.m_strScreenShotsPath);
        CImage image; // оказывается, в MFC есть вот такая херота.
        image.Attach(hBitmap, CImage::DIBOR_DEFAULT);

        if (SUCCEEDED(image.Save(szFileName, Gdiplus::ImageFormatPNG))) // и как всё оказывается просто.
        {
            bRet = true;
        }

        DeleteObject(hBitmap);
    }
    else
    {
        g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
    }
#endif
    return bRet;
}

bool CMainFrame::CheckDebugMemmap()
{
	bool bRet = m_bBKMemViewOpen;

    if (bRet)   // если была открыта карта памяти
    {
        m_bBKMemViewOpen = false;
//        m_pBKMemView->GetWindowRect(m_rectMemMap); // а пока закроем
//        m_pBKMemView->DestroyWindow();
    }

    return bRet;    // скажем, что после её надо будет переоткрыть
}

void CMainFrame::SetDebugCtrlsState()
{
    if (m_pBoard)
    {
        m_paneRegistryDumpViewCPU->DisplayRegDump();
//        m_paneRegistryDumpViewFDD.DisplayRegDump();
//        // тормозит, если много строк на экране.
        m_paneMemoryDumpView->DisplayMemDump();
    }
}

inline static QIcon makeIcon(int i, QPixmap &pm)
{
    return QIcon(pm.copy(i * 16, 0, 16, 16));
}

void CMainFrame::ChangeImageIcon(UINT nBtnID, FDD_DRIVE eDrive)
{
    if (m_pBoard)
    {
        QPixmap tbFDDImg(":toolBar/FDD");
        int nDrive = static_cast<int>(eDrive) & 3;

        int index = (m_pBoard->GetFDD()->IsAttached(eDrive)) ? nDrive : nDrive + 4;

        m_ToolbarFDDButton[nDrive]->setIcon(makeIcon(index, tbFDDImg));

//        int iImage = (m_pBoard->GetFDD()->IsAttached(eDrive)) ?
//                     GetCmdMgr()->GetCmdImage(ID_FILE_LOADEDDRIVE) :
//                     GetCmdMgr()->GetCmdImage(ID_FILE_EMPTYDRIVE);
//        int nIndex = m_wndToolBar.CommandToIndex(nBtnID);
//        m_wndToolBar.GetButton(nIndex)->SetImage(iImage);
//        m_wndToolBar.InvalidateButton(nIndex); // Перерисовываем только нужную кнопку
////      m_wndToolBar.Invalidate(FALSE); // перерисовываем весь тулбар, а то какие-то непонятные явления в Вин8 случаются:
//        // не желает перерисовываться кнопка.
    }
}


void CMainFrame::LoadFileHDDImage(UINT nBtnID, HDD_MODE eMode)
{
    CString strFilterIMG(MAKEINTRESOURCE(IDS_FILEFILTER_BKIMG));
//    CLoadImgDlg dlg(true, nullptr, nullptr,
//                    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER,
//                    strFilterIMG, m_pScreen->GetBackgroundWindow());
//    dlg.GetOFN().lpstrInitialDir = g_Config.m_strIMGPath; // диалог всегда будем начинать с домашней директории образов.



    CString str = QFileDialog::getOpenFileName(this,"Load File Image", g_Config.m_strIMGPath, "*.*");
    if (!str.isNull())
    {
//        CString str = dlg.GetPathName();
        // тут надо примонтировать str в заданный привод nDrive
        g_Config.SetDriveImgName(eMode, str);

//        if (m_pBoard)
//        {
//            m_pBoard->GetFDD()->AttachImage(eDrive, str);
//        }
    }

    // nBtnID - ид кнопки, нужно заменить картинку, на картинку со значком.
//    ChangeImageIcon(nBtnID, eDrive);
//    SetFocusToBK();
    SetupConfiguration(g_Config.GetBKModelNumber());
}

void CMainFrame::LoadFileImage(UINT nBtnID, FDD_DRIVE eDrive)
{
    CString strFilterIMG(MAKEINTRESOURCE(IDS_FILEFILTER_BKIMG));
//    CLoadImgDlg dlg(true, nullptr, nullptr,
//                    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER,
//                    strFilterIMG, m_pScreen->GetBackgroundWindow());
//    dlg.GetOFN().lpstrInitialDir = g_Config.m_strIMGPath; // диалог всегда будем начинать с домашней директории образов.



    CString str = QFileDialog::getOpenFileName(this,"Load File Image", g_Config.m_strIMGPath, "*.*");
    if (!str.isNull())
    {
//        CString str = dlg.GetPathName();
        // тут надо примонтировать str в заданный привод nDrive
        g_Config.SetDriveImgName(eDrive, str);

        if (m_pBoard)
        {
            m_pBoard->GetFDD()->AttachImage(eDrive, str);
        }
    }

    // nBtnID - ид кнопки, нужно заменить картинку, на картинку со значком.
    ChangeImageIcon(nBtnID, eDrive);
    SetFocusToBK();
}

struct BinFileHdr {
    uint16_t start;
    uint16_t len;
};

void CMainFrame::LoadBinFile()
{
    CString str = QFileDialog::getOpenFileName(this,"Load File Image", g_Config.m_strIMGPath, "*.bin *.BIN");

    if (!str.isNull())
    {

        CFile binFile;
        BinFileHdr hdr;

        binFile.Open(str, CFile::modeRead);
        binFile.Read(&hdr, 4);

        uint8_t *mem = new uint8_t[hdr.len];
        uint8_t *pmem = mem;

        binFile.Read(mem, hdr.len);

        if (m_pBoard)
        {
            bool isRunning = m_pBoard->IsCPUBreaked();
            m_pBoard->StopCPU();
            for(int i=0; i<hdr.len; i++) {
                m_pBoard->SetByte(hdr.start+i, *pmem++);
            }
            m_pBoard->SetRON(CCPU::REGISTER::PC, hdr.start);
            m_pBoard->RunCPU();
            if(isRunning)
                m_pBoard->BreakCPU();

        }
        delete[] mem;

        // Load symbols from .lst if exist
        CString path, name, ext;
        splitpath(str, path, name, ext);

        if(!m_pDebugger->m_SymTable.LoadSymbolsSTB(QDir(path).filePath(name + ".STB")))
           if(!m_pDebugger->m_SymTable.LoadSymbolsSTB(QDir(path).filePath(name + ".stb")))
              if(!m_pDebugger->m_SymTable.LoadSymbols(QDir(path).filePath(name + ".lst")))
                 if(!m_pDebugger->m_SymTable.LoadSymbols(QDir(path).filePath(name + ".LST"))) {}

        m_paneDisassembleView->repaint();
    }

    SetFocusToBK();
}

void CMainFrame::LoadSymbols()
{
    CString str = QFileDialog::getOpenFileName(this,"Load Symbols from ", g_Config.m_strIMGPath, "*.lst *.LST *.stb *.STB");

    if(!str.isNull()) {
        if(GetFileExt(str).toLower() == "stb") {
            m_pDebugger->m_SymTable.LoadSymbolsSTB(str);
        } else if(GetFileExt(str).toLower() == "lst") {
            m_pDebugger->m_SymTable.LoadSymbols(str);
        }
    }

}

void CMainFrame::OnSaveDisasm()
{
    CString str = QFileDialog::getSaveFileName(this,"Save disassembled code", g_Config.m_strIMGPath, "*.asm");

    if (!str.isNull())
    {

        CString line;
        QFile asmFile(str);
        if (asmFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            u_int16_t addr = 01000;
            while(addr < 05000) {
                addr += m_pDebugger->DissassembleAddr(addr, line, 0);
                line += '\n';
                asmFile.write(line.toLocal8Bit().data());
            }
            asmFile.close();
        }
    }

}

void CMainFrame::StopAll()
{
	StopTimer();

	if (m_pBoard)
	{
		m_pBoard->StopCPU(); // остановка CPU - там прекращается обработка команд и поток работает вхолостую
		m_pBoard->StopTimerThread(); // остановка и завершение потока.
	}
}

void CMainFrame::StartAll()
{
	if (m_pBoard)
	{
		m_pBoard->StartTimerThread();
		m_pBoard->RunCPU();
	}

	StartTimer();
}

void CMainFrame::SetFocusToBK()
{
     m_pBKView->setFocus();
//    auto vw = GetActiveView();

//    if (vw)
//    {
//        vw->SetFocus();
//    }
//    else
//    {
//        TRACE("Nope Active Views. Focus set to main Window!\n");
//        AfxGetMainWnd()->SetFocus();
//    }
}


void CMainFrame::SetFocusToDebug()
{
    m_paneDisassembleView->setFocus();
}

void CMainFrame::OnCpuBreak()
{
    if (!m_paneDisassembleView->isHidden())
    {
        if (m_pBoard)
        {
            register uint16_t pc = m_pBoard->GetRON(CCPU::REGISTER::PC);
            // прорисовываем окно дизассемблера
            m_pDebugger->UpdateCurrentAddress(pc);
        }
    }

    m_Action_DebugStop->setIcon(m_Action_DebugStop_Start);

   if (!m_paneMemoryDumpView->isHidden()) {
        if (m_pBoard) {
            m_paneMemoryDumpView->repaint();
        }
    }


    SetDebugCtrlsState();
    SetFocusToDebug();
}


void CMainFrame::OnFileLoadstate()
{
    CString strFilterMSF(MAKEINTRESOURCE(IDS_FILEFILTER_MSF));
//    CLoadMemoryDlg dlg(true, nullptr, nullptr,
//                       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR | OFN_EXPLORER,
//                       strFilterMSF, m_pScreen->GetBackgroundWindow());
//    dlg.GetOFN().lpstrInitialDir = g_Config.m_strMemPath;

//    if (dlg.DoModal() == IDOK)
//    {
//        LoadMemoryState(dlg.GetPathName());
//    }

    CString str = QFileDialog::getOpenFileName(this,"Load Emulator State", g_Config.m_strMemPath, "*.*");
    if (!str.isNull()) {
        LoadMemoryState(str);
    }

    SetFocusToBK();
}

void CMainFrame::OnFileSavestate()
{
    CString strFilterMSF(MAKEINTRESOURCE(IDS_FILEFILTER_MSF));
//    CFileDialog dlg(false, _T("msf"), nullptr,
//                    OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
//                    strFilterMSF, m_pScreen->GetBackgroundWindow());
//    dlg.GetOFN().lpstrInitialDir = g_Config.m_strMemPath;

//    if (dlg.DoModal() == IDOK)
//    {
//        if (SaveMemoryState(dlg.GetPathName()))
//        {
//        }
//    }

//    g_Config.m_strMemPath = ::GetFilePath(dlg.GetPathName());

    CString str = QFileDialog::getSaveFileName(this,"Save Emulator State", g_Config.m_strMemPath, "*.*");
    if (!str.isNull()) {
        if (SaveMemoryState(str))
        {
        }
//        g_Config.m_strMemPath = ::GetFilePath(dlg.GetPathName());
    }

    SetFocusToBK();
}

void CMainFrame::OnFileLoadtape()
{
    CString strFilterTape(MAKEINTRESOURCE(IDS_FILEFILTER_TAPE_LOAD));
    CString strTapeExt(MAKEINTRESOURCE(IDS_FILEEXT_TAPE));
//    CLoadTapeDlg dlg(true, strTapeExt, nullptr,
//                     OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR,
//                     strFilterTape, m_pScreen->GetBackgroundWindow());
//    dlg.GetOFN().lpstrInitialDir = g_Config.m_strTapePath;

//    if (dlg.DoModal() == IDOK)
//    {
//        StartPlayTape(dlg.GetPathName());
//    }

    CString str = QFileDialog::getOpenFileName(this,"Save Emulator State", g_Config.m_strMemPath, "*.*");
    if (!str.isNull()) {
        StartPlayTape(str);
    }
    SetFocusToBK();
}


void CMainFrame::OnUpdateFileLoadtape(QAction *act)
{
//    act->setEnabled(!g_Config.m_bEmulateLoadTape);
    act->setEnabled(!g_Config.m_bEmulateLoadTape);
}

void CMainFrame::OnFileScreenshot()
{
    MakeScreenShot();
}


void CMainFrame::OnCpuLongReset()
{
	m_bLongResetPress = true;
	OnCpuResetCpu();
}

void CMainFrame::OnCpuSuResetCpu()
{
    bool bSU = m_paneBKVKBDView->GetSUStatus();
    m_paneBKVKBDView->SetSUStatus(true);
    OnCpuResetCpu();
    m_paneBKVKBDView->SetSUStatus(bSU);
}

void CMainFrame::OnCpuResetCpu()
{
	if (!m_pBoard)
	{
		return;
	}

	if (m_pBoard->IsCPURun()) // защита от множественного вызова функции.
	{
		m_pBoard->StopCPU();
		BK_DEV_MPI fdd_model = m_pBoard->GetFDDType();

		// если контроллер А16М и сделали длинный ресет или контроллер СМК512 - делаем перезапуск
		// с адреса, который задаётся контроллером.
		if (
		    ((fdd_model == BK_DEV_MPI::A16M) && m_bLongResetPress) ||
		    (fdd_model == BK_DEV_MPI::SMK512)
		)
		{
			m_pBoard->SetAltProMode(ALTPRO_A16M_START_MODE);
			m_pBoard->SetAltProCode(0);
			m_pBoard->ResetCold(0);
		}
		else
		{
			// если у нас БК11, то можно реализовать СУ/ресет - перезапуск по 100000
            if ((m_pBoard->GetBoardModel() != BK_DEV_MPI::BK0010) && m_paneBKVKBDView->GetSUStatus())
			{
				m_pBoard->ResetCold(040000);
			}
			else
			{
				m_pBoard->ResetCold(0);
			}

			// СУ/ресет не реализовывается на контроллере СМК512 потому, что всегда подменяется
			// содержимое 177716 по чтению наложением ПЗУ.
			// А на А16М при коротком ресете и на обычных КНГМД СУ/ресет работает
		}

		m_bLongResetPress = false;
		InitKbdStatus(); // переинициализируем состояния клавиатуры
		// Запускаем CPU
		m_pBoard->RunCPU();
	}

	// если установлен флаг остановки после создания, приостановим выполнение
	if (g_Config.m_bPauseCPUAfterStart)
	{
		m_pBoard->BreakCPU();
	}
}

void CMainFrame::OnCpuRunbk001001()
{
	SetupConfiguration(CONF_BKMODEL::BK_0010_01);
}

void CMainFrame::OnCpuRunbk001001Focal()
{
	SetupConfiguration(CONF_BKMODEL::BK_0010_01_MSTD);
}

void CMainFrame::OnCpuRunbk00100132k()
{
	SetupConfiguration(CONF_BKMODEL::BK_0010_01_EXT32RAM);
}

void CMainFrame::OnCpuRunbk001001Fdd()
{
	SetupConfiguration(CONF_BKMODEL::BK_0010_01_FDD);
}

void CMainFrame::OnCpuRunbk001001Fdd16k()
{
	SetupConfiguration(CONF_BKMODEL::BK_0010_01_A16M);
}

void CMainFrame::OnCpuRunbk001001FddSmk512()
{
	SetupConfiguration(CONF_BKMODEL::BK_0010_01_SMK512);
}

void CMainFrame::OnCpuRunbk001001FddSamara()
{
	SetupConfiguration(CONF_BKMODEL::BK_0010_01_SAMARA);
}

void CMainFrame::OnCpuRunbk0011()
{
	SetupConfiguration(CONF_BKMODEL::BK_0011);
}

void CMainFrame::OnCpuRunbk0011Fdd()
{
	SetupConfiguration(CONF_BKMODEL::BK_0011_FDD);
}

void CMainFrame::OnCpuRunbk0011FddA16m()
{
	SetupConfiguration(CONF_BKMODEL::BK_0011_A16M);
}

void CMainFrame::OnCpuRunbk0011FddSmk512()
{
	SetupConfiguration(CONF_BKMODEL::BK_0011_SMK512);
}

void CMainFrame::OnCpuRunbk0011FddSamara()
{
	SetupConfiguration(CONF_BKMODEL::BK_0011_SAMARA);
}

void CMainFrame::OnCpuRunbk0011m()
{
	SetupConfiguration(CONF_BKMODEL::BK_0011M);
}

void CMainFrame::OnCpuRunbk0011mFDD()
{
	SetupConfiguration(CONF_BKMODEL::BK_0011M_FDD);
}

void CMainFrame::OnCpuRunbk0011mFddA16m()
{
	SetupConfiguration(CONF_BKMODEL::BK_0011M_A16M);
}

void CMainFrame::OnCpuRunbk0011mFddSmk512()
{
	SetupConfiguration(CONF_BKMODEL::BK_0011M_SMK512);
}

void CMainFrame::OnCpuRunbk0011mFddSamara()
{
	SetupConfiguration(CONF_BKMODEL::BK_0011M_SAMARA);
}

void CMainFrame::OnUpdateCpuRunbk001001(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0010_01);
}

void CMainFrame::OnUpdateCpuRunbk001001Focal(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0010_01_MSTD);
}

void CMainFrame::OnUpdateCpuRunbk00100132k(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0010_01_EXT32RAM);
}

void CMainFrame::OnUpdateCpuRunbk001001Fdd(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0010_01_FDD);
}

void CMainFrame::OnUpdateCpuRunbk001001Fdd16k(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0010_01_A16M);
}

void CMainFrame::OnUpdateCpuRunbk001001FddSmk512(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0010_01_SMK512);
}

void CMainFrame::OnUpdateCpuRunbk001001FddSamara(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0010_01_SAMARA);
}

void CMainFrame::OnUpdateCpuRunbk0011(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0011);
}

void CMainFrame::OnUpdateCpuRunbk0011Fdd(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0011_FDD);
}


void CMainFrame::OnUpdateCpuRunbk0011FddA16m(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0011_A16M);
}

void CMainFrame::OnUpdateCpuRunbk0011FddSmk512(QAction *act)
{
    // на БК11 СМК не работает, т.к. использует п/п ПЗУ БК11М
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0011_SMK512);
}

void CMainFrame::OnUpdateCpuRunbk0011FddSamara(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0011_SAMARA);
}

void CMainFrame::OnUpdateCpuRunbk0011m(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0011M);
}

void CMainFrame::OnUpdateCpuRunbk0011mFDD(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0011M_FDD);
}

void CMainFrame::OnUpdateCpuRunbk0011mFddA16m(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0011M_A16M);
}

void CMainFrame::OnUpdateCpuRunbk0011mFddSmk512(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0011M_SMK512);
}

void CMainFrame::OnUpdateCpuRunbk0011mFddSamara(QAction *act)
{
    act->setChecked(g_Config.m_BKConfigModelNumber == CONF_BKMODEL::BK_0011M_SAMARA);
}

void CMainFrame::OnCpuAccelerate()
{
    if (m_pBoard)
    {
        m_pBoard->AccelerateCPU();
//        m_paneRegistryDumpViewCPU.UpdateFreq();
    }
}

void CMainFrame::OnUpdateCpuAccelerate(QAction *act)
{
    act->setEnabled((m_pBoard) ? m_pBoard->CanAccelerate() : FALSE);
}

void CMainFrame::OnCpuSlowdown()
{
    if (m_pBoard)
    {
        m_pBoard->SlowdownCPU();
//        m_paneRegistryDumpViewCPU.UpdateFreq();
    }
}

void CMainFrame::OnUpdateCpuSlowdown(QAction *act)
{
    act->setEnabled((m_pBoard) ? m_pBoard->CanSlowDown() : FALSE);
}

void CMainFrame::OnCpuNormalspeed()
{
    if (m_pBoard)
    {
        m_pBoard->NormalizeCPU();
//        m_paneRegistryDumpViewCPU.UpdateFreq();
    }
}



void CMainFrame::OnOptionsEnableSpeaker()
{
	g_Config.m_bSpeaker = !m_speaker.IsSoundEnabled();
	m_speaker.EnableSound(g_Config.m_bSpeaker);
}

void CMainFrame::OnUpdateOptionsEnableSpeaker(QAction *act)
{
    act->setChecked(m_speaker.IsSoundEnabled());
}

void CMainFrame::OnOptionsEnableCovox()
{
	g_Config.m_bCovox = !m_covox.IsSoundEnabled();
	m_covox.EnableSound(g_Config.m_bCovox);

	if (g_Config.m_bCovox)
	{
		// выключим AY
		g_Config.m_bAY8910 = false;
		m_ay8910.EnableSound(g_Config.m_bAY8910);
		// выключим менестрель
		g_Config.m_bMenestrel = false;
		m_menestrel.EnableSound(g_Config.m_bMenestrel);
	}
}

void CMainFrame::OnUpdateOptionsEnableCovox(QAction *act)
{
    act->setChecked(m_covox.IsSoundEnabled());
}

void CMainFrame::OnOptionsStereoCovox()
{
	g_Config.m_bStereoCovox = !m_covox.IsStereo();
	m_covox.SetStereo(g_Config.m_bStereoCovox);
}

void CMainFrame::OnUpdateOptionsStereoCovox(QAction *act)
{
    act->setChecked(m_covox.IsStereo());
}

void CMainFrame::OnOptionsEnableMenestrel()
{
	g_Config.m_bMenestrel = !m_menestrel.IsSoundEnabled();
	m_menestrel.EnableSound(g_Config.m_bMenestrel);

	if (g_Config.m_bMenestrel)
	{
		// выключим ковокс
		g_Config.m_bCovox = false;
		m_covox.EnableSound(g_Config.m_bCovox);
		// выключим AY
		g_Config.m_bAY8910 = false;
		m_ay8910.EnableSound(g_Config.m_bAY8910);
	}
}

void CMainFrame::OnUpdateOptionsEnableMenestrel(QAction *act)
{
	act->setChecked(m_menestrel.IsSoundEnabled());
}

void CMainFrame::OnOptionsEnableAy8910()
{
	g_Config.m_bAY8910 = !m_ay8910.IsSoundEnabled();
	m_ay8910.EnableSound(g_Config.m_bAY8910);

	if (g_Config.m_bAY8910)
	{
		// выключим ковокс
		g_Config.m_bCovox = false;
		m_covox.EnableSound(g_Config.m_bCovox);
		// выключим менестрель
		g_Config.m_bMenestrel = false;
		m_menestrel.EnableSound(g_Config.m_bMenestrel);
	}
}

void CMainFrame::OnUpdateOptionsEnableAy8910(QAction *act)
{
    act->setChecked(m_ay8910.IsSoundEnabled());
}

void CMainFrame::OnOptionsSpeakerFilter()
{
	g_Config.m_bSpeakerFilter = !m_speaker.IsFilter();
	m_speaker.SetFilter(g_Config.m_bSpeakerFilter);
}

void CMainFrame::OnUpdateOptionsSpeakerFilter(QAction *act)
{
    act->setChecked(m_speaker.IsFilter());
}

void CMainFrame::OnOptionsCovoxFilter()
{
	g_Config.m_bCovoxFilter = !m_covox.IsFilter();
	m_covox.SetFilter(g_Config.m_bCovoxFilter);
}

void CMainFrame::OnUpdateOptionsCovoxFilter(QAction *act)
{
    act->setChecked(m_covox.IsFilter());
}

void CMainFrame::OnOptionsMenestrelFilter()
{
	g_Config.m_bMenestrelFilter = !m_menestrel.IsFilter();
	m_menestrel.SetFilter(g_Config.m_bMenestrelFilter);
}

void CMainFrame::OnUpdateOptionsMenestrelFilter(QAction *act)
{
	act->setChecked(m_menestrel.IsFilter());
}

void CMainFrame::OnOptionsAy8910Filter()
{
    g_Config.m_bAY8910Filter = !m_ay8910.IsFilter();
    m_ay8910.SetFilter(g_Config.m_bAY8910Filter);
}

void CMainFrame::OnUpdateOptionsAy8910Filter(QAction *act)
{
	act->setChecked(m_ay8910.IsFilter());
}


void CMainFrame::OnOptionsSpeakerDcoffset()
{
	g_Config.m_bSpeakerDCOffset = !m_speaker.IsDCOffsetCorrect();
	m_speaker.SetDCOffsetCorrect(g_Config.m_bSpeakerDCOffset);
}


void CMainFrame::OnUpdateOptionsSpeakerDcoffset(QAction *act)
{
    act->setChecked(m_speaker.IsDCOffsetCorrect());
}


void CMainFrame::OnOptionsCovoxDcoffset()
{
	g_Config.m_bCovoxDCOffset = !m_covox.IsDCOffsetCorrect();
	m_covox.SetDCOffsetCorrect(g_Config.m_bCovoxDCOffset);
}


void CMainFrame::OnUpdateOptionsCovoxDcoffset(QAction *act)
{
    act->setChecked(m_covox.IsDCOffsetCorrect());
}


void CMainFrame::OnOptionsMenestrelDcoffset()
{
	g_Config.m_bMenestrelDCOffset = !m_menestrel.IsDCOffsetCorrect();
	m_menestrel.SetDCOffsetCorrect(g_Config.m_bMenestrelDCOffset);
}


void CMainFrame::OnUpdateOptionsMenestrelDcoffset(QAction *act)
{
    act->setChecked(m_menestrel.IsDCOffsetCorrect());
}


void CMainFrame::OnOptionsAy8910Dcoffset()
{
	g_Config.m_bAY8910DCOffset = !m_ay8910.IsDCOffsetCorrect();
	m_ay8910.SetDCOffsetCorrect(g_Config.m_bAY8910DCOffset);
}


void CMainFrame::OnUpdateOptionsAy8910Dcoffset(QAction *act)
{
    act->setChecked(m_ay8910.IsDCOffsetCorrect());
}

void CMainFrame::OnOptionsEmulateBkkeyboard()
{
    g_Config.m_bBKKeyboard = !g_Config.m_bBKKeyboard;
}

void CMainFrame::OnUpdateOptionsEmulateBkkeyboard(QAction *act)
{
    act->setChecked(g_Config.m_bBKKeyboard);
}

void CMainFrame::OnOptionsEnableJoystick()
{
    g_Config.m_bJoystick = !g_Config.m_bJoystick;

    if (g_Config.m_bJoystick) // если включаем джойстик
    {
        g_Config.m_bICLBlock = false; // блок нагрузок выключаем
    }
}

void CMainFrame::OnUpdateOptionsEnableJoystick(QAction *act)
{
    act->setChecked(g_Config.m_bJoystick);
}

void CMainFrame::OnDebugEnableIclblock()
{
    g_Config.m_bICLBlock = !g_Config.m_bICLBlock;

    if (g_Config.m_bICLBlock) // если включаем блок нагрузок
    {
        g_Config.m_bJoystick = false; // джойстик выключаем
    }
}

void CMainFrame::OnUpdateDebugEnableIclblock(QAction *act)
{
    act->setChecked(g_Config.m_bICLBlock);
}

void CMainFrame::OnOptionsEmulateFddio()
{
    g_Config.m_bEmulateFDDIO = !g_Config.m_bEmulateFDDIO;
}

void CMainFrame::OnUpdateOptionsEmulateFddio(QAction *act)
{
    act->setChecked(g_Config.m_bEmulateFDDIO);
}

void CMainFrame::OnOptionsUseSavesdirectory()
{
    g_Config.m_bSavesDefault = !g_Config.m_bSavesDefault;
}

void CMainFrame::OnUpdateOptionsUseSavesdirectory(QAction *act)
{
    act->setChecked(g_Config.m_bSavesDefault);
    act->setEnabled(g_Config.m_bEmulateSaveTape);
}

void CMainFrame::OnOptionsEmulateTapeLoading()
{
    g_Config.m_bEmulateLoadTape = !g_Config.m_bEmulateLoadTape;
    UpdateTapeDlgControls();
}

void CMainFrame::OnUpdateOptionsEmulateTapeLoading(QAction *act)
{
    act->setChecked(g_Config.m_bEmulateLoadTape);
}

void CMainFrame::OnOptionsEmulateTapeSaving()
{
    g_Config.m_bEmulateSaveTape = !g_Config.m_bEmulateSaveTape;
    UpdateTapeDlgControls();
}

void CMainFrame::OnUpdateOptionsEmulateTapeSaving(QAction *act)
{
    act->setChecked(g_Config.m_bEmulateSaveTape);
}

#if 0

void CMainFrame::OnOptionsTapemanager()
{
	auto pdlg = new CTapeManagerDlg(m_pScreen->GetBackgroundWindow());

	if (pdlg)
	{
		pdlg->DoModal();
		delete pdlg;
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

	SetFocusToBK();
}

void CMainFrame::OnAppSettings()
{
	auto pSettingsDlg = new CSettingsDlg;

	if (pSettingsDlg)
	{
		if (m_pBoard)
		{
			m_pBoard->StopCPU(); // остановка CPU, чтобы ничего не попортить
		}

		// возвращаются три своих кастомных значения
		INT_PTR res = pSettingsDlg->DoModal();
		delete pSettingsDlg;

		if (res != NO_CHANGES)
		{
			// обновим данные конфигов и параметров и пользовательский интерфейс
			// в соответствии с новыми данными
			InitEmulator();
			// в этой функции не всё обязательно обновлять в этом месте
			// но чтобы не дублировать функции, будем вызывать её. Всё равно ничего страшного
			// не случится.

			// ещё нужно обновить значение частоты, а то оно старым перебивается.
			if (g_Config.m_nCPUFrequency)
			{
				m_pBoard->SetCPUFreq(g_Config.m_nCPUFrequency);
			}
		}

		if (res == CHANGES_NEEDREBOOT) // если нужен перезапуск
		{
			// перезапускаем конфигурацию.
			CONF_BKMODEL n = g_Config.GetBKModelNumber();
			ConfigurationConstructor(n); // конфиг сохраняем там.
		}
		else
		{
			// если не нужен перезапуск или вообще отмена - просто возобновляем работу
			if (m_pBoard)
			{
				m_pBoard->RunCPU();
			}
		}

		SetFocusToBK();
	}
}

void CMainFrame::OnPaletteEdit()
{
	auto pPaletteDlg = new CBKPaletteDlg;

	if (pPaletteDlg)
	{
		INT_PTR res = pPaletteDlg->DoModal();
		delete pPaletteDlg;

		if (res == IDOK)
		{
			m_pScreen->InitColorTables();
		}
	}
}

void CMainFrame::OnOptionsJoyedit()
{
	auto pJoyEditDlg = new CJoyEditDlg;

	if (pJoyEditDlg)
	{
		INT_PTR res = pJoyEditDlg->DoModal();
		delete pJoyEditDlg;
	}
}
#endif

void CMainFrame::OnSettAyvolpan()
{
    CBKAYVolPan pAYVolPanDlg;

    auto res = pAYVolPanDlg.exec();
}

void CMainFrame::OnDebugBreak()
{
    if (m_pBoard)
    {
        if (m_pBoard->IsCPUBreaked())
        {
            m_pBoard->UnbreakCPU(CMotherBoard::ADDRESS_NONE);
            SetFocusToBK();
            m_Action_DebugStop->setIcon(m_Action_DebugStop_Stop);
        }
        else
        {
            m_pBoard->BreakCPU();
            SetFocusToDebug();
            m_Action_DebugStop->setIcon(m_Action_DebugStop_Start);
        }
    }

    if (!m_paneMemoryDumpView->isHidden()) {
        m_paneMemoryDumpView->repaint();
    }

}

void CMainFrame::OnUpdateDebugBreak(QAction *act)
{
    if (m_pBoard)
    {
//        int nIndex = m_wndToolBarDebug.CommandToIndex(ID_DEBUG_STARTBREAK);
//        CMFCToolBarButton *pBtn = m_wndToolBarDebug.GetButton(nIndex);
//        CString strMenu;
//        int iImage;

//        if (m_pBoard->IsCPUBreaked())
//        {
//            strMenu.LoadString(IDS_MENU_DEBUG_CONTINUE);
//            iImage = GetCmdMgr()->GetCmdImage(ID_DEBUG_START);
//        }
//        else
//        {
//            strMenu.LoadString(IDS_MENU_DEBUG_BREAK);
//            iImage = GetCmdMgr()->GetCmdImage(ID_DEBUG_BREAK);
//        }

//        pBtn->SetImage(iImage);
//        pCmdUI->SetText(strMenu);
//        m_wndToolBarDebug.UpdateData();
//        m_wndToolBarDebug.RedrawWindow();
    }
}

void CMainFrame::OnDebugStepinto()
{
    if (m_pBoard)
    {
        m_pBoard->RunInto();
    }
}

void CMainFrame::OnUpdateDebugStepinto(QAction *act)
{
    act->setEnabled((m_pBoard) ? m_pBoard->IsCPUBreaked() : FALSE);
}

void CMainFrame::OnDebugStepover()
{
    if (m_pBoard)
    {
        m_pBoard->RunOver();
    }
}

void CMainFrame::OnUpdateDebugStepover(QAction *act)
{
    act->setEnabled((m_pBoard) ? m_pBoard->IsCPUBreaked() : FALSE);
}

void CMainFrame::OnDebugStepout()
{
    if (m_pBoard)
    {
        m_pBoard->RunOut();
    }
}

void CMainFrame::OnUpdateDebugStepout(QAction *act)
{
    act->setEnabled((m_pBoard) ? m_pBoard->IsCPUBreaked() : FALSE);
}

#ifdef ENABLE_BACKTRACE
void CMainFrame::OnDebugStepBack()
{
    if(m_pBoard)
    {
        m_pBoard->StepBack();
    }
    OnCpuBreak();
    m_paneRegistryDumpViewCPU->DisplayRegDump();
}

void CMainFrame::OnUpdateDebugStepback(QAction *act)
{
    act->setEnabled((m_pBoard) ? m_pBoard->IsCPUBreaked() : FALSE);
}
#endif

void CMainFrame::OnDebugRuntocursor()
{
    if (m_pBoard)
    {
        m_pBoard->RunToAddr(m_pDebugger->GetCursorAddress());
    }
}

void CMainFrame::OnUpdateDebugRuntocursor(QAction *act)
{
    act->setEnabled((m_pBoard) ? m_pBoard->IsCPUBreaked() : FALSE);
}

void CMainFrame::OnDebugBreakpoint()
{
    if (!m_pDebugger->SetSimpleBreakpoint())
    {
        m_pDebugger->RemoveBreakpoint();
    }

    m_paneDisassembleView->repaint();
}

#if 0
void CMainFrame::OnDebugMemmap()
{
	if (!m_pBoard)
	{
		return;
	}

	/*
	TODO доделать:
	1. синхронизацию. Нельзя удалять, пока работает DrawCurrentTab и нельзя его запускать когда объект удаляется.
	Если перед удалением объекта останавливать поток TimerThreadFunc, то конфликтов не будет.
	Удаление никогда не будет работать параллельно с DrawCurrentTab
	2. подумать насчёт переделки в DockingTab. хоть размеры и не способствуют, может будет проще управлять
	этой штукой, если она будет DockingTab с запрещённым докингом.
	*/
	m_bBKMemViewOpen = false;

	if (m_pBKMemView) // если раньше окно уже было открыто, а мы снова пробуем открыть
	{
		m_pBKMemView->DestroyWindow(); // старое окно удалим
	}

	// и пойдём создавать новое окно
	auto pBKMemVw = new CBKMEMDlg(m_pBoard->GetBoardModel(), m_pBoard->GetFDDType(),
	                              m_pBoard->GetMainMemory(), m_pBoard->GetAddMemory(), this); // обязательно создавать динамически.

	if (pBKMemVw)
	{
		if (pBKMemVw->Create(IDD_BKMEM_MAP_DLG, this))
		{
			if (m_rectMemMap != CRect(0, 0, 0, 0))
			{
				pBKMemVw->MoveWindow(m_rectMemMap, false);
			}

			pBKMemVw->ShowWindow(SW_SHOW);
			m_pBKMemView = pBKMemVw;
			m_bBKMemViewOpen = true;
		}
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}
}

// эта функция нужна только для того, чтобы m_pBKMemView присвоить nullptr
// когда мы закрываем карту памяти кнопкой крестик в правом верхнем углу
LRESULT CMainFrame::OnMemMapClose(WPARAM, LPARAM)
{
	m_bBKMemViewOpen = false; // при закрытии крестиком ещё и эту переменную надо разблокировать
	m_pBKMemView = nullptr;
	return S_OK;
}

LRESULT CMainFrame::OnMemDumpUpdate(WPARAM, LPARAM)
{
	m_paneMemoryDumpView.DisplayMemDump();
	return S_OK;
}

void CMainFrame::OnDebugDumpregsInterval(UINT id)
{
	switch (id)
	{
		default:
		case ID_DEBUG_DUMPREGS_INTERVAL_0:
			g_Config.m_nRegistersDumpInterval = 0;
			break;

		case ID_DEBUG_DUMPREGS_INTERVAL_1:
			g_Config.m_nRegistersDumpInterval = 1;
			break;

		case ID_DEBUG_DUMPREGS_INTERVAL_2:
			g_Config.m_nRegistersDumpInterval = 2;
			break;

		case ID_DEBUG_DUMPREGS_INTERVAL_3:
			g_Config.m_nRegistersDumpInterval = 3;
			break;

		case ID_DEBUG_DUMPREGS_INTERVAL_4:
			g_Config.m_nRegistersDumpInterval = 4;
			break;

		case ID_DEBUG_DUMPREGS_INTERVAL_5:
			g_Config.m_nRegistersDumpInterval = 5;
			break;

		case ID_DEBUG_DUMPREGS_INTERVAL_10:
			g_Config.m_nRegistersDumpInterval = 10;
			break;

		case ID_DEBUG_DUMPREGS_INTERVAL_15:
			g_Config.m_nRegistersDumpInterval = 15;
			break;

		case ID_DEBUG_DUMPREGS_INTERVAL_20:
			g_Config.m_nRegistersDumpInterval = 20;
			break;

		case ID_DEBUG_DUMPREGS_INTERVAL_25:
			g_Config.m_nRegistersDumpInterval = 25;
			break;

		case ID_DEBUG_DUMPREGS_INTERVAL_50:
			g_Config.m_nRegistersDumpInterval = 50;
			break;
	}
}

void CMainFrame::OnUpdateDebugDumpregsInterval(QAction *act)
{
    switch (g_Config.m_nRegistersDumpInterval)
    {
        default:
            g_Config.m_nRegistersDumpInterval = 0;

        // тут break не нужен! Но и стоять это должно строго перед case 0:
        case 0:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_0);
            break;

        case 1:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_1);
            break;

        case 2:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_2);
            break;

        case 3:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_3);
            break;

        case 4:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_4);
            break;

        case 5:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_5);
            break;

        case 10:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_10);
            break;

        case 15:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_15);
            break;

        case 20:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_20);
            break;

        case 25:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_25);
            break;

        case 50:
            act->setChecked(pCmdUI->m_nID == ID_DEBUG_DUMPREGS_INTERVAL_50);
            break;
    }
}

#endif

void CMainFrame::OnDebugDialogAskForBreak()
{
    if (m_pBoard)
    {
        g_Config.m_bAskForBreak = !g_Config.m_bAskForBreak;
    }
}

void CMainFrame::OnUpdateDebugDialogAskForBreak(QAction *act)
{
    if (m_pBoard)
    {
        act->setChecked(g_Config.m_bAskForBreak);
    }
}

void CMainFrame::OnDebugPauseCpuAfterStart()
{
    g_Config.m_bPauseCPUAfterStart = !g_Config.m_bPauseCPUAfterStart;
}

void CMainFrame::OnUpdateDebugPauseCpuAfterStart(QAction *act)
{
    act->setChecked(g_Config.m_bPauseCPUAfterStart);
}

void CMainFrame::OnOptionsShowPerformanceOnStatusbar()
{
    g_Config.m_bShowPerformance = !g_Config.m_bShowPerformance;

    if (!g_Config.m_bShowPerformance)
    {
//        m_wndStatusBar.SetPaneText(static_cast<int>(STATUS_FIELD::INFO), _T(""));
        m_pSB_Common->setText("");
    }
}

void CMainFrame::OnUpdateOptionsShowPerformanceOnStatusbar(QAction *act)
{
    act->setChecked(g_Config.m_bShowPerformance);
}

#if 0
void CMainFrame::OnVkbdtypeKeys(UINT id)
{
    switch (id)
    {
        default:
        case ID_VKBDTYPE_KEYS:
            g_Config.m_nVKBDType = 0;
            m_paneBKVKBDView.SetKeyboardView(IDB_BITMAP_SOFT);
            break;

        case ID_VKBDTYPE_MEMBRANE:
            g_Config.m_nVKBDType = 1;
            m_paneBKVKBDView.SetKeyboardView(IDB_BITMAP_PLEN);
            break;
    }
}

void CMainFrame::OnUpdateVkbdtypeKeys(QAction *act)
{
    switch (g_Config.m_nVKBDType)
    {
        default:
            g_Config.m_nVKBDType = 0; // тут break не нужен! Но и стоять это должно строго перед case 0:

        case 0:
            act->setChecked(pCmdUI->m_nID == ID_VKBDTYPE_KEYS);
            break;

        case 1:
            act->setChecked(pCmdUI->m_nID == ID_VKBDTYPE_MEMBRANE);
            break;
    }
}

void CMainFrame::OnViewFullscreenmode()
{
    if (m_pBoard)
    {
        m_pBoard->StopCPU(false);

        if (m_pScreen->IsFullScreenMode())
        {
            m_pScreen->SetWindowMode();
        }
        else
        {
            m_pScreen->SetFullScreenMode();
        }

        // вышеприведённые функции в процессе работы меняют флаг g_Config.m_bFullscreenMode
        g_Config.m_bFullscreenMode = m_pScreen->IsFullScreenMode(); // поэтому его надо восстановить
        m_pBoard->RunCPU(false);
        theApp.GetMainWnd()->ShowWindow(SW_RESTORE); // после выхода из полноэкранного режима окно надо полностью перерисовать
        // а то после вызова диалогов в полноэкранном режиме, при выходе из полноэкранного режима не перерисовывается меню и тулбар
    }
}
#endif

#if 0
void CMainFrame::OnViewSmoothing()
{
    g_Config.m_bSmoothing = !m_pScreen->IsSmoothing();
    m_pScreen->SetSmoothing(g_Config.m_bSmoothing);
}

void CMainFrame::OnUpdateViewSmoothing(QAction *act)
{
    act->setChecked(m_pScreen->IsSmoothing());
}

void CMainFrame::OnViewColormode()
{
    g_Config.m_bColorMode = !m_pScreen->IsColorMode();
    m_pScreen->SetColorMode(g_Config.m_bColorMode);
}

void CMainFrame::OnUpdateViewColormode(QAction *act)
{
    act->setChecked(m_pScreen->IsColorMode());
}

void CMainFrame::OnViewAdaptivebwmode()
{
    g_Config.m_bAdaptBWMode = !m_pScreen->IsAdaptMode();
    m_pScreen->SetAdaptMode(g_Config.m_bAdaptBWMode);
}

void CMainFrame::OnUpdateViewAdaptivebwmode(QAction *act)
{
    act->setEnabled(!m_pScreen->IsColorMode());
    act->setChecked(m_pScreen->IsAdaptMode());
}

void CMainFrame::OnViewLuminoforemode()
{
    g_Config.m_bLuminoforeEmulMode = !m_pScreen->GetLuminoforeEmuMode();
    m_pScreen->SetLuminoforeEmuMode(g_Config.m_bLuminoforeEmulMode);
}

void CMainFrame::OnUpdateViewLuminoforemode(QAction *act)
{
    act->setChecked(m_pScreen->GetLuminoforeEmuMode());
}
#endif

#if 0
void CMainFrame::OnToolLaunch(UINT id)
{
    LaunchTool(id - ID_TOOL_MENU_0);
}
#endif

/*
// !!!
Нужно реализовать:
1. динамическое выпадающее меню. добавлять пункт - "Открыть в БКДЕ" с передачей имени образа в качестве параметра
2. Придумать если возможно - редактирование образа в БКДЕ, и в это время не должно быть конфликтов с эмулятором.
Либо нужно отмонтировать образ на время работы, а при закрытии БКДЕ как-то давать эмулятору знать, что образ
можно назад монтировать.
Либо останавливать эмулятор и не давать к нему доступа пока не будет закрыт БКДЕ.
Оба способа непонятно как сделать.
*/
void CMainFrame::OnFileLoadDrive(UINT id)
{
    FDD_DRIVE nDrive = FDD_DRIVE::NONE;
    HDD_MODE nMode;

    switch (id)
    {
        case ID_FILE_LOADDRIVE_A:
            nDrive = FDD_DRIVE::A;
            break;

        case ID_FILE_LOADDRIVE_B:
            nDrive = FDD_DRIVE::B;
            break;

        case ID_FILE_LOADDRIVE_C:
            nDrive = FDD_DRIVE::C;
            break;

        case ID_FILE_LOADDRIVE_D:
            nDrive = FDD_DRIVE::D;
            break;

        case ID_FILE_LOADHDD_MASTER:
            nMode = HDD_MODE::MASTER;
            break;

        case ID_FILE_LOADHDD_SLAVE:
            nMode = HDD_MODE::SLAVE;
            break;

        default:
            return;
    }

    if (id < ID_FILE_LOADHDD_MASTER)
        LoadFileImage(id, nDrive);
    else
        LoadFileHDDImage(id, nMode);
}

void CMainFrame::OnUpdateFileLoadDrive(QAction *act, UINT id)
{
    FDD_DRIVE nDrive = FDD_DRIVE::NONE;
    switch (id)
    {
        case ID_FILE_LOADDRIVE_A:
            nDrive = FDD_DRIVE::A;
            break;

        case ID_FILE_LOADDRIVE_B:
            nDrive = FDD_DRIVE::B;
            break;

        case ID_FILE_LOADDRIVE_C:
            nDrive = FDD_DRIVE::C;
            break;

        case ID_FILE_LOADDRIVE_D:
            nDrive = FDD_DRIVE::D;
            break;

        default:
            return;
    }
    bool isEnabled = m_pBoard ? m_pBoard->GetFDD()->GetDriveState(nDrive) : false;
    act->setEnabled(isEnabled);
    QToolBar * tb = (QToolBar *)act->parent();
    QToolButton *btn = (QToolButton *)tb->widgetForAction(act);
    btn->setEnabled(isEnabled);
}

void CMainFrame::OnFileUnmount(UINT id)
{
    FDD_DRIVE nDrive = FDD_DRIVE::NONE;
    UINT nIconID = id;

    switch (id)
    {
        case ID_FILE_UMOUNT_A:
            nDrive = FDD_DRIVE::A;
            nIconID = ID_FILE_LOADDRIVE_A;
            break;

        case ID_FILE_UMOUNT_B:
            nDrive = FDD_DRIVE::B;
            nIconID = ID_FILE_LOADDRIVE_B;
            break;

        case ID_FILE_UMOUNT_C:
            nDrive = FDD_DRIVE::C;
            nIconID = ID_FILE_LOADDRIVE_C;
            break;

        case ID_FILE_UMOUNT_D:
            nDrive = FDD_DRIVE::D;
            nIconID = ID_FILE_LOADDRIVE_D;
            break;

        default:
            return;
    }

    if (m_pBoard)
    {
        m_pBoard->GetFDD()->DetachImage(nDrive);
        ChangeImageIcon(nIconID, nDrive);
    }

    g_Config.SetDriveImgName(nDrive, g_strEmptyUnit);
}

///*
//Как оказалось, единственно возможный способ заменить текст в выпадающем меню -
//перехватить событие, и подменять строку при вызове меню.
//*/
//void CMainFrame::OnShowFddPopupMenu()
//{
//    if (pMenuPopup == nullptr)
//    {
//        return TRUE;
//    }

//    auto pParentButton = pMenuPopup->GetParentButton();

//    if (pParentButton == nullptr)
//    {
//        return TRUE;
//    }

//    FDD_DRIVE eDrive = FDD_DRIVE::NONE;

//    switch (pParentButton->m_nID)
//    {
//        case ID_FILE_LOADDRIVE_A:
//            eDrive = FDD_DRIVE::A;
//            break;

//        case ID_FILE_LOADDRIVE_B:
//            eDrive = FDD_DRIVE::B;
//            break;

//        case ID_FILE_LOADDRIVE_C:
//            eDrive = FDD_DRIVE::C;
//            break;

//        case ID_FILE_LOADDRIVE_D:
//            eDrive = FDD_DRIVE::D;
//            break;
//    }

//    if (m_pBoard && eDrive != FDD_DRIVE::NONE)
//    {
//        int nDrive = g_Config.GetDriveNum(eDrive);
//        // заменяемое значение пункта меню
//        CString strn = (m_pBoard->GetFDD()->IsAttached(eDrive)) ?
//                       g_Config.m_strFDDrives[nDrive] : g_strEmptyUnit;
//        auto item = pMenuPopup->GetMenuItem(pMenuPopup->GetMenuItemCount() - 1);
//        item->m_strText = strn;
//    }

//    return CFrameWndEx::OnShowPopupMenu(pMenuPopup);
//}

CPoint CMainFrame::m_aScreenSizes[SCREENSIZE_NUMBER] =
{
    { -1, -1 }, // 0
    { 256, 192 }, // 1
    { 324, 243 }, // 2
    { 432, 324 }, // 3
    { 512, 384 }, // 4
    { 576, 432 }, // 5
    { 768, 576 }, // 6
    { 1024, 768 } // 7
};

void CMainFrame::OnSetScreenSize(UINT id)
{
#if 0
    switch (id)
    {
        default:
        case ID_VIEW_SCREENSIZE_CUSTOM:
            m_nScreenSize = SCREENSIZE_CUSTOM;
            m_nScreen_X = m_nScreen_CustomX ? m_nScreen_CustomX : m_aScreenSizes[4].x;
            m_nScreen_Y = m_nScreen_CustomY ? m_nScreen_CustomY : m_aScreenSizes[4].y;
            break;

        case ID_VIEW_SCREENSIZE_256X192: // x0.5
            m_nScreenSize = SCREENSIZE_256X192;
            break;

        case ID_VIEW_SCREENSIZE_324X243:
            m_nScreenSize = SCREENSIZE_324X243;
            break;

        case ID_VIEW_SCREENSIZE_432X324:
            m_nScreenSize = SCREENSIZE_432X324;
            break;

        case ID_VIEW_SCREENSIZE_512X384: // x1
            m_nScreenSize = SCREENSIZE_512X384;
            break;

        case ID_VIEW_SCREENSIZE_576X432:
            m_nScreenSize = SCREENSIZE_576X432;
            break;

        case ID_VIEW_SCREENSIZE_768X576: // x1.5
            m_nScreenSize = SCREENSIZE_768X576;
            break;

        case ID_VIEW_SCREENSIZE_1024X768: // x2
            m_nScreenSize = SCREENSIZE_1024X768;
            break;
    }

    if (m_nScreenSize)
    {
        m_nScreen_X = m_aScreenSizes[m_nScreenSize].x;
        m_nScreen_Y = m_aScreenSizes[m_nScreenSize].y;
    }

    CRect rectView; // размеры области CBKView
    CRect rectMain; // размеры главного окна
    GetActiveView()->GetClientRect(&rectView);
    GetWindowRect(rectMain);
    int offsetX = rectView.Width() - m_nScreen_X; // дельта X;
    int offsetY = rectView.Height() - m_nScreen_Y; // дельта Y;
    int newsizeX = rectMain.Width() - offsetX;
    int newsizeY = rectMain.Height() - offsetY;
    SetWindowPos(nullptr, rectMain.left, rectMain.top, newsizeX, newsizeY, SWP_SHOWWINDOW | SWP_NOZORDER);
#endif
}

void CMainFrame::resizeEvent(QResizeEvent* event)
{
   QMainWindow::resizeEvent(event);
   // Your code here.
   QSize size = event->size();
   OnScreenSizeChanged(size.width(), size.height());
}

void CMainFrame::OnScreenSizeChanged(uint width, uint height)
{
    m_nScreen_X = width;
    m_nScreen_Y = height;
    bool bFound = false;

    for (int i = SCREENSIZE_256X192; i < SCREENSIZE_NUMBER; ++i)
    {
        if (m_nScreen_X == m_aScreenSizes[i].x && m_nScreen_Y == m_aScreenSizes[i].y)
        {
            m_nScreenSize = static_cast<ScreenSizeNumber>(i);
            bFound = true;
            break;
        }
    }

    if (!bFound)
    {
        m_nScreen_CustomX = m_nScreen_X;
        m_nScreen_CustomY = m_nScreen_Y;
        m_nScreenSize = SCREENSIZE_CUSTOM;
    }

}

void CMainFrame::OnUpdateSetScreenSize(QAction *act, UINT id)
{
/*
    switch (m_nScreenSize)
    {
        default:
            m_nScreenSize = SCREENSIZE_CUSTOM; // тут break не нужен!

        case SCREENSIZE_CUSTOM:
            act->setChecked(pCmdUI->m_nID == ID_VIEW_SCREENSIZE_CUSTOM);
            break;

        case SCREENSIZE_256X192:
            act->setChecked(pCmdUI->m_nID == ID_VIEW_SCREENSIZE_256X192);
            break;

        case SCREENSIZE_324X243:
            act->setChecked(pCmdUI->m_nID == ID_VIEW_SCREENSIZE_324X243);
            break;

        case SCREENSIZE_432X324:
            act->setChecked(pCmdUI->m_nID == ID_VIEW_SCREENSIZE_432X324);
            break;

        case SCREENSIZE_512X384:
            act->setChecked(pCmdUI->m_nID == ID_VIEW_SCREENSIZE_512X384);
            break;

        case SCREENSIZE_576X432:
            act->setChecked(pCmdUI->m_nID == ID_VIEW_SCREENSIZE_576X432);
            break;

        case SCREENSIZE_768X576:
            act->setChecked(pCmdUI->m_nID == ID_VIEW_SCREENSIZE_768X576);
            break;

        case SCREENSIZE_1024X768:
            act->setChecked(pCmdUI->m_nID == ID_VIEW_SCREENSIZE_1024X768);
            break;
    }
    */
}

CString CMainFrame::MakeUniqueName()
{
//#pragma warning(disable:4996)
    // сделаем уникальное имя файла, чтоб не заморачиваться - возьмём тек. время
//    time_t      now = time(nullptr);
//    struct tm   tstruct;
//    constexpr auto nBufSize = 80;
//    TCHAR       buf[nBufSize];
//    tstruct = *localtime(&now);

//    wcsftime(buf, nBufSize, _T("_%Y%m%d-%H%M%S"), &tstruct);
//    return CString(buf);

    return QDateTime(QDateTime::currentDateTime()).toString("yyyyMMdd-hh-mm-ss");

}

void CMainFrame::OnOptionsLogAy8910()
{
    if (m_ay8910.isLogEnabled())
    {
        // выключим лог
        m_ay8910.log_done();
    }
    else
    {
        // включим лог
        m_ay8910.log_start(MakeUniqueName());
    }
}


void CMainFrame::OnUpdateOptionsLogAy8910(QAction *act)
{
    act->setChecked(m_ay8910.isLogEnabled());
}


void CMainFrame::OnVideoCaptureStart()
{
    CString str = MakeUniqueName();
    m_pBKView->SetCaptureStatus(true, str);
    m_pSound->SetCaptureStatus(true, str);
}

void CMainFrame::OnUpdateVideoCaptureStart(QAction *act)
{
//    act->setEnabled(m_bFoundFFMPEG && !m_pScreen->IsCapture());
}

void CMainFrame::OnVideoCaptureStop()
{
    CString str = _T("");
    m_pBKView->SetCaptureStatus(false, str);
    m_pSound->SetCaptureStatus(false, str);
}

void CMainFrame::OnUpdateVideoCaptureStop(QAction *act)
{
//    act->setEnabled(m_bFoundFFMPEG && m_pScreen->IsCapture());
}

void CMainFrame::OnVkbdtypeKeys(UINT id)
{
    switch (id)
    {
        default:
        case IDB_BITMAP_SOFT:
            g_Config.m_nVKBDType = IDB_BITMAP_SOFT;
            m_paneBKVKBDView->SetKeyboardView(IDB_BITMAP_SOFT);
            break;

        case IDB_BITMAP_PLEN:
            g_Config.m_nVKBDType = IDB_BITMAP_SOFT;
            m_paneBKVKBDView->SetKeyboardView(IDB_BITMAP_PLEN);
            break;
    }
}

void CMainFrame::OnUpdateVkbdtypeKeys(QAction *act, UINT id)
{

      act->setChecked(id == g_Config.m_nVKBDType);
//    switch (g_Config.m_nVKBDType)
//    {
//        default:
//            g_Config.m_nVKBDType = 0; // тут break не нужен! Но и стоять это должно строго перед case 0:

//        case 0:
//            act->setChecked(pCmdUI->m_nID == ID_VKBDTYPE_KEYS);
//            break;

//        case 1:
//            act->setChecked(pCmdUI->m_nID == ID_VKBDTYPE_MEMBRANE);
//            break;
//    }
}

void CMainFrame::OnViewFullscreenmode()
{
#if 0
    if (m_pBoard)
    {
        m_pBoard->StopCPU(false);

        if (m_pScreen->IsFullScreenMode())
        {
            m_pScreen->SetWindowMode();
        }
        else
        {
            m_pScreen->SetFullScreenMode();
        }

        // вышеприведённые функции в процессе работы меняют флаг g_Config.m_bFullscreenMode
        g_Config.m_bFullscreenMode = m_pScreen->IsFullScreenMode(); // поэтому его надо восстановить
        m_pBoard->RunCPU(false);
        theApp.GetMainWnd()->ShowWindow(SW_RESTORE); // после выхода из полноэкранного режима окно надо полностью перерисовать
        // а то после вызова диалогов в полноэкранном режиме, при выходе из полноэкранного режима не перерисовывается меню и тулбар
    }
#endif
}

void CMainFrame::OnViewSmoothing()
{
    g_Config.m_bSmoothing = !m_pScreen->IsSmoothing();
    m_pScreen->SetSmoothing(g_Config.m_bSmoothing);
    m_pBKView->SetSmoothing(g_Config.m_bSmoothing);
}

void CMainFrame::OnUpdateViewSmoothing(QAction *act)
{
    act->setChecked(m_pScreen->IsSmoothing());
}

void CMainFrame::OnViewColormode()
{
    g_Config.m_bColorMode = !m_pScreen->IsColorMode();
    m_pScreen->SetColorMode(g_Config.m_bColorMode);
}

void CMainFrame::OnUpdateViewColormode(QAction *act)
{
    act->setChecked(m_pScreen->IsColorMode());
}

void CMainFrame::OnViewAdaptivebwmode()
{
    g_Config.m_bAdaptBWMode = !m_pScreen->IsAdaptMode();
    m_pScreen->SetAdaptMode(g_Config.m_bAdaptBWMode);
}

void CMainFrame::OnUpdateViewAdaptivebwmode(QAction *act)
{
    act->setEnabled(!m_pScreen->IsColorMode());
    act->setChecked(m_pScreen->IsAdaptMode());
}

void CMainFrame::OnViewLuminoforemode()
{
    g_Config.m_bLuminoforeEmulMode = !m_pScreen->GetLuminoforeEmuMode();
    m_pScreen->SetLuminoforeEmuMode(g_Config.m_bLuminoforeEmulMode);
}

void CMainFrame::OnUpdateViewLuminoforemode(QAction *act)
{
    act->setChecked(m_pScreen->GetLuminoforeEmuMode());
}

void CMainFrame::OnDebugDrawScreen(uint param)
{
    GetScreen()->ChangeBuffer(m_pBoard->GetMainMemory()+param, 16384);
    GetScreen()->ReDrawScreen();
}


void CMainFrame::ReceiveMessage(uint msgCode, uint param)
{
    switch(msgCode) {
        case WM_CPU_DEBUGBREAK:
            OnCpuBreak();
            break;
        case WM_SCR_DEBUGDRAW:
            OnDebugDrawScreen(param);
            break;
        case WM_OSC_DRAW:
            (void)param;
            break;
        case WM_SCR_DRAW:
            break;
        case WM_RESET_KBD_MANAGER:
            break;
        default:
            break;
    }
}

