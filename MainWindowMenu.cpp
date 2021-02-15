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

//BUTTON      ID_FILE_LOADSTATE
//BUTTON      ID_FILE_SAVESTATE
//BUTTON      ID_FILE_LOADTAPE
//SEPARATOR
//BUTTON      ID_CPU_RESETCPU
//BUTTON      ID_CPU_ACCELERATE
//BUTTON      ID_CPU_SLOWDOWN
//BUTTON      ID_CPU_NORMALSPEED
//SEPARATOR
//BUTTON      ID_VIEW_FULLSCREENMODE
//BUTTON      ID_VIEW_COLORMODE
//BUTTON      ID_VIEW_ADAPTIVEBWMODE
//BUTTON      ID_VIEW_SMOOTHING
//SEPARATOR
//BUTTON      ID_FILE_LOADDRIVE_A
//BUTTON      ID_FILE_LOADDRIVE_B
//BUTTON      ID_FILE_LOADDRIVE_C
//BUTTON      ID_FILE_LOADDRIVE_D
//SEPARATOR
//BUTTON      ID_FILE_CUSTOM_PRINT
//BUTTON      ID_FILE_SCREENSHOT
//BUTTON      ID_APP_SETTINGS

inline static QIcon makeIcon(int i, QPixmap &pm)
{
    return QIcon(pm.copy(i * 16, 0, 16, 16));
}


void CMainFrame::CreateMenu()
{
    QMenu *menu;
    QAction *act;

    QPixmap tbMainImg(":toolBar/main");
    QPixmap tbMenu1Img(":toolBar/menu1");
    QPixmap tbDbgImg(":toolBar/dbg");
    QToolBar *tb = addToolBar("Main");

    menuBar()->clear();
//    POPUP "&Файл"
    menu = menuBar()->addMenu(tr("&Файл"));

//    MENUITEM "&Загрузить состояние...",     ID_FILE_LOADSTATE
    act = new QAction(makeIcon(0, tbMainImg), QString("&Загрузить состояние..."), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnFileLoadstate);
    menu->addAction(act);
    tb->addAction(act);


//    MENUITEM "&Сохранить состояние...",     ID_FILE_SAVESTATE
    act = new QAction(makeIcon(1, tbMainImg), QString("&Сохранить состояние..."), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnFileSavestate);
    menu->addAction(act);
    tb->addAction(act);

//    MENUITEM "Загрузить &ленту...",         ID_FILE_LOADTAPE
    act = new QAction(makeIcon(2, tbMainImg), QString("&Загрузить &ленту..."), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnFileLoadtape);
    menu->addAction(act);
    tb->addAction(act);

//    MENUITEM "С&криншот",                   ID_FILE_SCREENSHOT
    QAction *aScrshot = act = new QAction(makeIcon(16, tbMainImg), QString("С&криншот"), this);
//    connect(act, &QAction::triggered, this, &CMainFrame::OnFileScreenshot);
    menu->addAction(act);

//    MENUITEM SEPARATOR
    menu->addSeparator();

//        MENUITEM "&Печать...",                  ID_FILE_CUSTOM_PRINT
    QAction * aPrint = act = new QAction(makeIcon(15, tbMainImg), QString("&Печать..."), this);
//    connect(act, &QAction::triggered, this, &CMainFrame::OnFile);
    menu->addAction(act);

//        MENUITEM "Нас&тройка печати...",        ID_FILE_PRINT_SETUP
    act = new QAction(QString("Нас&тройка печати..."), this);
//    connect(act, &QAction::triggered, this, &CMainFrame::OnFileScreenshot);
    menu->addAction(act);

//        MENUITEM SEPARATOR
    menu->addSeparator();

//        MENUITEM "В&ыход",                      ID_APP_EXIT
    act = new QAction(QString("В&ыход"), this);
    connect(act, &QAction::triggered, this, &CMainFrame::close);
    menu->addAction(act);

    // ===================================================================================

//    POPUP "&Конфигурация"
    menu = menuBar()->addMenu(tr("&Конфигурация"));
    tb->addSeparator();

//        MENUITEM "&Рестарт БК",                 ID_CPU_RESETCPU
    act = new QAction(makeIcon(3, tbMainImg), QString("&Рестарт БК"), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuResetCpu);
    menu->addAction(act);
    tb->addAction(act);

//        MENUITEM "СУ+Рестарт БК",               ID_CPU_SURESETCPU
    act = new QAction(QString("СУ+Рестарт БК"), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuSuResetCpu);
    menu->addAction(act);

//        MENUITEM "&Длинный рестарт БК",         ID_CPU_LONGRESET
    act = new QAction(QString("&Длинный рестарт БК"), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuLongReset);
    menu->addAction(act);

//        MENUITEM SEPARATOR
    menu->addSeparator();

    QActionGroup *ag = new QActionGroup(this);

//        MENUITEM "Старт БК0010-01",             ID_CPU_RUNBK001001
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0010_01)] = act = new QAction(QString("Старт БК0010-01"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + блок Фокал-МСТД", ID_CPU_RUNBK001001_FOCAL
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0010_01_MSTD)] = act = new QAction(QString("Старт БК0010-01 + блок Фокал-МСТД"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001Focal);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + доп. 32кб ОЗУ", ID_CPU_RUNBK001001_32K
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0010_01_EXT32RAM)] = act = new QAction(QString("Старт БК0010-01 + доп. 32кб ОЗУ"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk00100132k);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + стандартный КНГМД", ID_CPU_RUNBK001001_FDD
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0010_01_FDD)] = act = new QAction(QString("Старт БК0010-01 + стандартный КНГМД"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001Fdd);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + контроллер A16M", ID_CPU_RUNBK001001_FDD16K
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0010_01_A16M)] = act = new QAction(QString("Старт БК0010-01 + контроллер A16M"), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001Fdd16k);
    act->setCheckable(true);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + контроллер СМК-512", ID_CPU_RUNBK001001_FDD_SMK512
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0010_01_SMK512)] = act = new QAction(QString("Старт БК0010-01 + контроллер СМК-512"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001FddSmk512);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + контроллер Samara", ID_CPU_RUNBK001001_FDD_SAMARA
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0010_01_SAMARA)] = act = new QAction(QString("Старт БК0010-01 + контроллер Samara"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001FddSamara);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011 + МСТД",         ID_CPU_RUNBK0011
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0011)] = act = new QAction(QString("Старт БК0011 + МСТД"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011 + стандартный КНГМД", ID_CPU_RUNBK0011_FDD
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0011_FDD)] = act = new QAction(QString("Старт БК0011 + стандартный КНГМД"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011Fdd);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011 + контроллер А16М", ID_CPU_RUNBK0011_FDD_A16M
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0011_A16M)] = act = new QAction(QString("Старт БК0011 + контроллер А16М"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011FddA16m);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011 + контроллер СМК-512", ID_CPU_RUNBK0011_FDD_SMK512
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0011_SMK512)] = act = new QAction(QString("Старт БК0011 + контроллер СМК-512"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011FddSmk512);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011 + контроллер Samara", ID_CPU_RUNBK0011_FDD_SAMARA
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0011_SAMARA)] = act = new QAction(QString("Старт БК0011 + контроллер Samara"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011FddSamara);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011М + МСТД",        ID_CPU_RUNBK0011M
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0011M)] = act = new QAction(QString("Старт БК0011М + МСТД"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011m);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011М + стандартный КНГМД", ID_CPU_RUNBK0011M_FDD
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0011M_FDD)] = act = new QAction(QString("Старт БК0011М + стандартный КНГМД"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011mFDD);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011М + контроллер A16M", ID_CPU_RUNBK0011M_FDD_A16M
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0011M_A16M)] = act = new QAction(QString("Старт БК0011М + контроллер A16M"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011mFddA16m);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011М + контроллер СМК-512", ID_CPU_RUNBK0011M_FDD_SMK512
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0011M_SMK512)] = act = new QAction(QString("Старт БК0011М + контроллер СМК-512"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011mFddSmk512);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011М + контроллер Samara", ID_CPU_RUNBK0011M_FDD_SAMARA
    m_pActions_BKModel[static_cast<int>(CONF_BKMODEL::BK_0011M_SAMARA)] = act = new QAction(QString("Старт БК0011М + контроллер Samara"), this);
    act->setCheckable(true);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011mFddSamara);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM SEPARATOR
    menu->addSeparator();

//        MENUITEM "&Ускорить",                   ID_CPU_ACCELERATE
    act = new QAction(makeIcon(4, tbMainImg), QString("&Ускорить"), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuAccelerate);
    menu->addAction(act);
    tb->addAction(act);

//        MENUITEM "&Замедлить",                  ID_CPU_SLOWDOWN
    act = new QAction(makeIcon(5, tbMainImg), QString("&Замедлить"), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuSlowdown);
    menu->addAction(act);
    tb->addAction(act);

//        MENUITEM "&Стандартная скорость",       ID_CPU_NORMALSPEED
    act = new QAction(makeIcon(6, tbMainImg), QString("Стандартная скорость."), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuNormalspeed);
    menu->addAction(act);
    tb->addAction(act);

    // ==============================================================================================

//    POPUP "&Вид"
    menu = menuBar()->addMenu(tr("&Вид"));
    tb->addSeparator();

//        MENUITEM "&Строка состояния",           ID_VIEW_STATUS_BAR
     act = new QAction(QString("&Строка состояния"), this);
     act->setCheckable(true);
     act->setChecked(!statusBar()->isHidden());
     connect(act, &QAction::triggered, this, &CMainFrame::ToggleStatusBar);
     menu->addAction(act);

//     POPUP "Виртуальная &клавиатура"
     QMenu *menu1 = menu->addMenu(tr("Виртуальная &клавиатура"));
         ag = new QActionGroup(this);

    //         MENUITEM "&Кнопочная",                  ID_VKBDTYPE_KEYS
         m_pActions_Keybd_type[IDB_BITMAP_SOFT] = act = new QAction(QString("&Кнопочная"), this);
         act->setCheckable(true);
         connect(act, &QAction::triggered, this, [=](){ m_paneBKVKBDView->SetKeyboardView(IDB_BITMAP_SOFT); m_paneBKVKBDView->show(); } );
         ag->addAction(act);
         menu1->addAction(act);

    //         MENUITEM "&Плёночная",                  ID_VKBDTYPE_MEMBRANE
         m_pActions_Keybd_type[IDB_BITMAP_PLEN] = act = new QAction(QString("&Плёночная"), this);
         act->setCheckable(true);
         connect(act, &QAction::triggered, this, [=](){ m_paneBKVKBDView->SetKeyboardView(IDB_BITMAP_PLEN); m_paneBKVKBDView->show(); } );
         ag->addAction(act);
         menu1->addAction(act);

//         POPUP "Установить размер экрана"
     menu1 = menu->addMenu(tr("Установить размер экрана"));
             ag = new QActionGroup(this);

//             MENUITEM "256 x 192 (x0.5)",            ID_VIEW_SCREENSIZE_256X192
             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_256X192)] = act = new QAction(QString("256 x 192 (x0.5)"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "324 x 243",                   ID_VIEW_SCREENSIZE_324X243
             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_324X243)] = act = new QAction(QString("324 x 243"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "432 x 324",                   ID_VIEW_SCREENSIZE_432X324
             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_432X324)] = act = new QAction(QString("432 x 324"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "512 х 384 (x1)",              ID_VIEW_SCREENSIZE_512X384
             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_512X384)] = act = new QAction(QString("512 х 384 (x1)"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);


//             MENUITEM "576 x 432",                   ID_VIEW_SCREENSIZE_576X432
             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_576X432)] = act = new QAction(QString("576 x 432"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "768 x 576 (x1.5)",            ID_VIEW_SCREENSIZE_768X576
             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_768X576)] = act = new QAction(QString("768 x 576 (x1.5)"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "1024 x 768 (x2)",             ID_VIEW_SCREENSIZE_1024X768
             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_1024X768)] = act = new QAction(QString("1024 x 768 (x2)"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "Свой",                        ID_VIEW_SCREENSIZE_CUSTOM
             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_CUSTOM)] = act = new QAction(QString("Свой"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

         //        MENUITEM SEPARATOR
         menu->addSeparator();

//             MENUITEM "Сг&лаживание",                ID_VIEW_SMOOTHING
         m_Action_ViewSmoothing = act = new QAction(makeIcon(10, tbMainImg), QString("Сг&лаживание"), this);
         act->setCheckable(true);
         connect(act, &QAction::triggered, this, &CMainFrame::OnViewSmoothing);
         menu->addAction(act);
         tb->addAction(act);

//             MENUITEM "Во весь &экран",              ID_VIEW_FULLSCREENMODE
         m_Action_ViewSmoothing = act = new QAction(makeIcon(7, tbMainImg), QString("Во весь &экран"), this);
         act->setCheckable(true);
         connect(act, &QAction::triggered, this, &CMainFrame::OnViewFullscreenmode);
         menu->addAction(act);
         tb->addAction(act);

//             MENUITEM "&Цветной режим",              ID_VIEW_COLORMODE
         m_Action_ViewColormode = act = new QAction(makeIcon(8, tbMainImg), QString("&Цветной режим"), this);
         act->setCheckable(true);
         connect(act, &QAction::triggered, this, &CMainFrame::OnViewColormode);
         menu->addAction(act);
         tb->addAction(act);

//             MENUITEM "&Адаптивный Ч/Б режим",       ID_VIEW_ADAPTIVEBWMODE
         m_Action_ViewAdaptivebwmode = act = new QAction(makeIcon(9, tbMainImg), QString("&Адаптивный Ч/Б режим"), this);
         act->setCheckable(true);
         connect(act, &QAction::triggered, this, &CMainFrame::OnViewAdaptivebwmode);
         menu->addAction(act);
         tb->addAction(act);

//             MENUITEM "&Эмуляция затухания люминофора", ID_VIEW_LUMINOFOREMODE
         m_Action_ViewLuminoforemode = act = new QAction(QString("&Эмуляция затухания люминофора"), this);
         act->setCheckable(true);
         connect(act, &QAction::triggered, this, &CMainFrame::OnViewLuminoforemode);
         menu->addAction(act);

         tb->addSeparator();
         act = new QAction(makeIcon(11, tbMainImg), QString("&Load Drive A:"), this);
         connect(act,&QAction::triggered, this, [=](){ CMainFrame::OnFileLoadDrive(0); });
         tb->addAction(act);

         act = new QAction(makeIcon(12, tbMainImg), QString("&Load Drive B:"), this);
         connect(act,&QAction::triggered, this, [=](){ CMainFrame::OnFileLoadDrive(1); });
         tb->addAction(act);

         act = new QAction(makeIcon(13, tbMainImg), QString("&Load Drive C:"), this);
         connect(act,&QAction::triggered, this, [=](){ CMainFrame::OnFileLoadDrive(2); });
         tb->addAction(act);

         act = new QAction(makeIcon(14, tbMainImg), QString("&Load Drive D:"), this);
         connect(act,&QAction::triggered, this, [=](){ CMainFrame::OnFileLoadDrive(3); });
         tb->addAction(act);


        act = new QAction(makeIcon(19, tbMainImg), QString("&Load Bin"), this);
        connect(act,&QAction::triggered, this, &CMainFrame::LoadBinFile);
        tb->addAction(act);


         tb->addSeparator();
         tb->addAction(aScrshot);
         tb->addAction(aPrint);

         tb = addToolBar("Debug");

         m_Action_DebugStop_Stop = makeIcon(0, tbDbgImg);
         m_Action_DebugStop_Start = makeIcon(0, tbMenu1Img);

         m_Action_DebugStop = act = new QAction(m_Action_DebugStop_Stop, QString("Стоп"), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugBreak);
         tb->addAction(act);

         act = new QAction(makeIcon(1, tbDbgImg), QString("StepIn"), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugStepinto);
         tb->addAction(act);

         act = new QAction(makeIcon(2, tbDbgImg), QString("StepOver"), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugStepover);
         tb->addAction(act);

         act = new QAction(makeIcon(3, tbDbgImg), QString("StepOut"), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugStepout);
         tb->addAction(act);

//         act = new QAction(makeIcon(4, tbDbgImg), QString("Run"), this);
//         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugBreak);
//         tb->addAction(act);

    //     act = new QAction(makeIcon(5, tbDbgImg), QString("Set Breakpoint"), this);
    //     connect(act, &QAction::triggered, this, &CMainFrame::OnCpuResetCpu);
    //     tb->addAction(act);

}

#if 0

POPUP "&Файл"
BEGIN
    MENUITEM "&Загрузить состояние...",     ID_FILE_LOADSTATE
    MENUITEM "&Сохранить состояние...",     ID_FILE_SAVESTATE
    MENUITEM "Загрузить &ленту...",         ID_FILE_LOADTAPE
    MENUITEM "С&криншот",                   ID_FILE_SCREENSHOT
    MENUITEM SEPARATOR
    MENUITEM "&Печать...",                  ID_FILE_CUSTOM_PRINT
    MENUITEM "Нас&тройка печати...",        ID_FILE_PRINT_SETUP
    MENUITEM SEPARATOR
    MENUITEM "В&ыход",                      ID_APP_EXIT
END
POPUP "&Конфигурация"
BEGIN
    MENUITEM "&Рестарт БК",                 ID_CPU_RESETCPU
    MENUITEM "СУ+Рестарт БК",               ID_CPU_SURESETCPU
    MENUITEM "&Длинный рестарт БК",         ID_CPU_LONGRESET
    MENUITEM SEPARATOR
    MENUITEM "Старт БК0010-01",             ID_CPU_RUNBK001001
    MENUITEM "Старт БК0010-01 + блок Фокал-МСТД", ID_CPU_RUNBK001001_FOCAL
    MENUITEM "Старт БК0010-01 + доп. 32кб ОЗУ", ID_CPU_RUNBK001001_32K
    MENUITEM "Старт БК0010-01 + стандартный КНГМД", ID_CPU_RUNBK001001_FDD
    MENUITEM "Старт БК0010-01 + контроллер A16M", ID_CPU_RUNBK001001_FDD16K
    MENUITEM "Старт БК0010-01 + контроллер СМК-512", ID_CPU_RUNBK001001_FDD_SMK512
    MENUITEM "Старт БК0010-01 + контроллер Samara", ID_CPU_RUNBK001001_FDD_SAMARA
    MENUITEM "Старт БК0011 + МСТД",         ID_CPU_RUNBK0011
    MENUITEM "Старт БК0011 + стандартный КНГМД", ID_CPU_RUNBK0011_FDD
    MENUITEM "Старт БК0011 + контроллер А16М", ID_CPU_RUNBK0011_FDD_A16M
    MENUITEM "Старт БК0011 + контроллер СМК-512", ID_CPU_RUNBK0011_FDD_SMK512
    MENUITEM "Старт БК0011 + контроллер Samara", ID_CPU_RUNBK0011_FDD_SAMARA
    MENUITEM "Старт БК0011М + МСТД",        ID_CPU_RUNBK0011M
    MENUITEM "Старт БК0011М + стандартный КНГМД", ID_CPU_RUNBK0011M_FDD
    MENUITEM "Старт БК0011М + контроллер A16M", ID_CPU_RUNBK0011M_FDD_A16M
    MENUITEM "Старт БК0011М + контроллер СМК-512", ID_CPU_RUNBK0011M_FDD_SMK512
    MENUITEM "Старт БК0011М + контроллер Samara", ID_CPU_RUNBK0011M_FDD_SAMARA
    MENUITEM SEPARATOR
    MENUITEM "&Ускорить",                   ID_CPU_ACCELERATE
    MENUITEM "&Замедлить",                  ID_CPU_SLOWDOWN
    MENUITEM "&Стандартная скорость",       ID_CPU_NORMALSPEED
END
POPUP "&Опции"
BEGIN
    MENUITEM "&Настройки эмулятора",        ID_APP_SETTINGS
    MENUITEM "&Редактор Палитр",            ID_OPTIONS_PALETTE
    MENUITEM "Редактор Джойстика",          ID_OPTIONS_JOYEDIT
    MENUITEM SEPARATOR
    MENUITEM "Включить &Speaker",           ID_OPTIONS_ENABLE_SPEAKER
    MENUITEM "Включить &Covox",             ID_OPTIONS_ENABLE_COVOX
    MENUITEM "Сте&рео Covox",               ID_OPTIONS_STEREO_COVOX
    MENUITEM "Включить &AY8910",            ID_OPTIONS_ENABLE_AY8910
    MENUITEM "Дамп регистров A&Y8910",      ID_OPTIONS_LOG_AY8910
    MENUITEM "Параметры AY8910",            ID_OPTIONS_AYVOLPAN
    POPUP "Фильтры"
    BEGIN
        MENUITEM "Speaker",                     ID_OPTIONS_SPEAKER_FILTER
        MENUITEM "Covox",                       ID_OPTIONS_COVOX_FILTER
        MENUITEM "AY8910",                      ID_OPTIONS_AY8910_FILTER
    END
    MENUITEM SEPARATOR
    MENUITEM "Эмулировать &клавиатуру БК",  ID_OPTIONS_EMULATE_BKKEYBOARD
    MENUITEM "Включить &джойстик",          ID_OPTIONS_ENABLE_JOYSTICK
    MENUITEM "&Эмулировать ввод-вывод дисковода", ID_OPTIONS_EMULATE_FDDIO
    MENUITEM "&Исп. папку Saves по умолчанию (BASIC или Focal)", ID_OPTIONS_
USE_SAVESDIRECTORY
    MENUITEM SEPARATOR
    MENUITEM "Эмулировать загрузку &ленты", ID_OPTIONS_EMULATE_TAPE_LOADING
    MENUITEM "Эмулировать &сохранение на ленту", ID_OPTIONS_EMULATE_TAPE_SAV
ING
    MENUITEM SEPARATOR
    MENUITEM "&Менеджер лент БК...",        ID_OPTIONS_TAPEMANAGER
END
POPUP "От&ладка"
BEGIN
    MENUITEM "&Стоп",                       ID_DEBUG_STARTBREAK
    MENUITEM SEPARATOR
    MENUITEM "Шаг с &заходом",              ID_DEBUG_STEPINTO
    MENUITEM "Шаг с &обходом",              ID_DEBUG_STEPOVER
    MENUITEM "Шаг с &выходом",              ID_DEBUG_STEPOUT
    MENUITEM "Выполнить до с&троки",        ID_DEBUG_RUNTOCURSOR
    MENUITEM SEPARATOR
    MENUITEM "То&чка останова",             ID_DEBUG_BREAKPOINT
    MENUITEM "&Условие останова...",        ID_DEBUG_BREAKPOINTCONDITION
    MENUITEM "&Карта памяти...",            ID_DEBUG_MEMMAP
    MENUITEM SEPARATOR
    POPUP "Интервал обновления дампа регистров"
    BEGIN
        MENUITEM "Нет",                         ID_DEBUG_DUMPREGS_INTERVAL_0
        MENUITEM "1",                           ID_DEBUG_DUMPREGS_INTERVAL_1
        MENUITEM "2",                           ID_DEBUG_DUMPREGS_INTERVAL_2
        MENUITEM "3",                           ID_DEBUG_DUMPREGS_INTERVAL_3
        MENUITEM "4",                           ID_DEBUG_DUMPREGS_INTERVAL_4
        MENUITEM "5",                           ID_DEBUG_DUMPREGS_INTERVAL_5
        MENUITEM "10",                          ID_DEBUG_DUMPREGS_INTERVAL_10
        MENUITEM "15",                          ID_DEBUG_DUMPREGS_INTERVAL_15
        MENUITEM "20",                          ID_DEBUG_DUMPREGS_INTERVAL_20
        MENUITEM "25",                          ID_DEBUG_DUMPREGS_INTERVAL_25
        MENUITEM "50",                          ID_DEBUG_DUMPREGS_INTERVAL_50
    END
    MENUITEM "&Диалог при ошибке доступа к памяти", ID_DEBUG_DIALOG_ASK_FOR_
BREAK
    MENUITEM "Ост&анов CPU после запуска эмулятора", ID_DEBUG_PAUSE_CPU_AFTE
R_START
    MENUITEM SEPARATOR
    MENUITEM "&Блок нагрузок",              ID_DEBUG_ENABLE_ICLBLOCK
END
POPUP "&Вид"
BEGIN
    POPUP "&Панели инструментов и закрепляемые окна"
    BEGIN
        MENUITEM "<местозаполнитель>",          ID_VIEW_TOOLBAR
    END
    MENUITEM "&Строка состояния",           ID_VIEW_STATUS_BAR
    POPUP "&Вид приложения"
    BEGIN
        MENUITEM "Windows &2000",               ID_VIEW_APPLOOK_WIN_2000
        MENUITEM "Office &XP",                  ID_VIEW_APPLOOK_OFF_XP
        MENUITEM "&Windows XP",                 ID_VIEW_APPLOOK_WIN_XP
        MENUITEM "Office 200&3",                ID_VIEW_APPLOOK_OFF_2003
        MENUITEM "Visual Studio 200&5",         ID_VIEW_APPLOOK_VS_2005
        MENUITEM "Visual Studio 200&8",         ID_VIEW_APPLOOK_VS_2008
        POPUP "Office 200&7"
        BEGIN
            MENUITEM "&Голубой стиль",              ID_VIEW_APPLOOK_OFF_2007_BLUE
            MENUITEM "Ч&ёрный стиль",               ID_VIEW_APPLOOK_OFF_2007_BLACK
            MENUITEM "&Серебристый стиль",          ID_VIEW_APPLOOK_OFF_2007_SILVER
            MENUITEM "&Зеленовато-голубой стиль",   ID_VIEW_APPLOOK_OFF_2007_AQUA
        END
    END
    POPUP "Виртуальная &клавиатура"
    BEGIN
        MENUITEM "&Кнопочная",                  ID_VKBDTYPE_KEYS
        MENUITEM "&Плёночная",                  ID_VKBDTYPE_MEMBRANE
    END
    POPUP "Установить размер экрана"
    BEGIN
        MENUITEM "256 x 192 (x0.5)",            ID_VIEW_SCREENSIZE_256X192
        MENUITEM "324 x 243",                   ID_VIEW_SCREENSIZE_324X243
        MENUITEM "432 x 324",                   ID_VIEW_SCREENSIZE_432X324
        MENUITEM "512 х 384 (x1)",              ID_VIEW_SCREENSIZE_512X384
        MENUITEM "576 x 432",                   ID_VIEW_SCREENSIZE_576X432
        MENUITEM "768 x 576 (x1.5)",            ID_VIEW_SCREENSIZE_768X576
        MENUITEM "1024 x 768 (x2)",             ID_VIEW_SCREENSIZE_1024X768
        MENUITEM "Свой",                        ID_VIEW_SCREENSIZE_CUSTOM
    END
    MENUITEM SEPARATOR
    MENUITEM "Сг&лаживание",                ID_VIEW_SMOOTHING
    MENUITEM "Во весь &экран",              ID_VIEW_FULLSCREENMODE
    MENUITEM "&Цветной режим",              ID_VIEW_COLORMODE
    MENUITEM "&Адаптивный Ч/Б режим",       ID_VIEW_ADAPTIVEBWMODE
    MENUITEM "&Эмуляция затухания люминофора", ID_VIEW_LUMINOFOREMODE
END
POPUP "&Инструменты"
BEGIN
    MENUITEM SEPARATOR
END
POPUP "&Справка"
BEGIN
    MENUITEM "&О программе...",             ID_APP_ABOUT
END
END

#endif
