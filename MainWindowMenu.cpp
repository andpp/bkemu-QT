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

#include <QWidgetAction>

inline static QIcon makeIcon(int i, QPixmap &pm)
{
    return QIcon(pm.copy(i * 16, 0, 16, 16));
}

using updateFuncRef = void (CMainFrame::*)(QAction *);
using updateFuncArg1Ref = void (CMainFrame::*)(QAction *, UINT);

struct UpdateFunc
{
    UpdateFunc()                           : fn(nullptr), fnArg1(nullptr), arg(0) {}
    UpdateFunc(updateFuncRef f)            : fn(f),       fnArg1(nullptr), arg(0) {};
    UpdateFunc(updateFuncArg1Ref f, int a) : fn(nullptr), fnArg1(f),       arg(a) {};
    updateFuncRef fn;
    updateFuncArg1Ref fnArg1;
    UINT arg;

};
Q_DECLARE_METATYPE(UpdateFunc);

void CMainFrame::OnMenuAboutToShow()
{
    QMenu *menu = static_cast<QMenu *>(sender());
    if (menu == nullptr)
        return;

    UpdateActions(menu->actions());
}

void CMainFrame::OnToolbarActionTriggered()
{
    QToolBar *menu = static_cast<QToolBar *>(sender());
    if (menu == nullptr)
        return;

    UpdateActions(menu->actions());
}

void CMainFrame::UpdateActions(QList<QAction *>menu)
{
    foreach (QAction *action, menu) {
        if (action->isSeparator()) {
            // Nothing to update
        } else if (action->menu()) {
            // This action is associated with a submenu, iterating it recursively...
            UpdateActions(action->menu()->actions());
        } else {
           UpdateFunc func = qvariant_cast<UpdateFunc>(action->data());
           if (func.fn) {
                (this->*func.fn)(action);
           } else if (func.fnArg1){
                (this->*func.fnArg1)(action, func.arg);
           }
        }
    }
}

/*
Как оказалось, единственно возможный способ заменить текст в выпадающем меню -
перехватить событие, и подменять строку при вызове меню.
*/
void CMainFrame::OnShowFddPopupMenu()
{
    QMenu *menu = static_cast<QMenu *>(sender());
    if (menu == nullptr)
        return;

    QList<QAction *>acts = menu->actions();
    UpdateFunc func = qvariant_cast<UpdateFunc>(acts[0]->data());
    UINT id = func.arg;

    FDD_DRIVE eDrive = FDD_DRIVE::NONE;

    switch (id)
    {
        case ID_FILE_LOADDRIVE_A:
            eDrive = FDD_DRIVE::A;
            break;

        case ID_FILE_LOADDRIVE_B:
            eDrive = FDD_DRIVE::B;
            break;

        case ID_FILE_LOADDRIVE_C:
            eDrive = FDD_DRIVE::C;
            break;

        case ID_FILE_LOADDRIVE_D:
            eDrive = FDD_DRIVE::D;
            break;

        default:
            return;
    }

    if (m_pBoard && eDrive != FDD_DRIVE::NONE)
    {
        int nDrive = g_Config.GetDriveNum(eDrive);
        if (m_pBoard->GetFDD()->IsAttached(eDrive)) {
            acts[0]->setText(tr("Unload Drive ") + CString('A' + id) + ":");
            dynamic_cast<QLabel *>(dynamic_cast<QWidgetAction *>(acts[1])->defaultWidget())->setText(g_Config.m_strFDDrives[nDrive]);
            disconnect(acts[0],&QAction::triggered, this, nullptr);
            connect(acts[0],&QAction::triggered, this, [=](){ CMainFrame::OnFileUnmount(id); });
        } else {
            acts[0]->setText(tr("Load Drive ") + CString('A' + id) + ":");
            dynamic_cast<QLabel *>(dynamic_cast<QWidgetAction *>(acts[1])->defaultWidget())->setText(g_strEmptyUnit);
            disconnect(acts[0],&QAction::triggered, this, nullptr);
            connect(acts[0],&QAction::triggered, this, [=](){ CMainFrame::OnFileLoadDrive(id); });
        }
    }
}


void CMainFrame::CreateMenu()
{
    QMenu *menu;
    QMenu *menu1;
    QAction *act;

    QPixmap tbMainImg(":toolBar/main");
    QPixmap tbMenu1Img(":toolBar/menu1");
    QPixmap tbDbgImg(":toolBar/dbg");
    QPixmap tbSndImg(":toolBar/snd");
    QPixmap tbFDDImg(":toolBar/FDD");
    QToolBar *tb;
    QVariant UpdateAction;

    menuBar()->clear();
    QList<QToolBar *> allToolBars = findChildren<QToolBar *>();
    foreach(QToolBar *_tb, allToolBars) {
        // This does not delete the tool bar.
        removeToolBar(_tb);
        _tb->clear();
    }

    tb = addToolBar(tr("Main"));
    connect(tb, &QToolBar::actionTriggered, this, &CMainFrame::OnToolbarActionTriggered);

//    POPUP "&Файл"
    menu = menuBar()->addMenu(tr("Файл"));
    connect(menu, &QMenu::aboutToShow, this, &CMainFrame::OnMenuAboutToShow);

//    MENUITEM "&Загрузить состояние...",     ID_FILE_LOADSTATE
    act = new QAction(makeIcon(0, tbMainImg), QString(tr("Загрузить состояние...")), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnFileLoadstate);
    menu->addAction(act);
    tb->addAction(act);


//    MENUITEM "&Сохранить состояние...",     ID_FILE_SAVESTATE
    act = new QAction(makeIcon(1, tbMainImg), QString(tr("Сохранить состояние...")), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnFileSavestate);
    menu->addAction(act);
    tb->addAction(act);

//    MENUITEM "Загрузить &ленту...",         ID_FILE_LOADTAPE
    act = new QAction(makeIcon(2, tbMainImg), QString(tr("Загрузить &ленту...")), this);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateFileLoadtape)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnFileLoadtape);
    menu->addAction(act);
    tb->addAction(act);

//    MENUITEM "С&криншот",                   ID_FILE_SCREENSHOT
    QAction *aScrshot = act = new QAction(makeIcon(16, tbMainImg), QString(tr("Скриншот")), this);
//    connect(act, &QAction::triggered, this, &CMainFrame::OnFileScreenshot);
    menu->addAction(act);

//    MENUITEM SEPARATOR
    menu->addSeparator();

//        MENUITEM "&Печать...",                  ID_FILE_CUSTOM_PRINT
    QAction * aPrint = act = new QAction(makeIcon(15, tbMainImg), QString(tr("Печать...")), this);
//    connect(act, &QAction::triggered, this, &CMainFrame::OnFile);
    menu->addAction(act);

//        MENUITEM "Нас&тройка печати...",        ID_FILE_PRINT_SETUP
    act = new QAction(QString(tr("Настройка печати...")), this);
//    connect(act, &QAction::triggered, this, &CMainFrame::OnFileScreenshot);
    menu->addAction(act);

    menu1 = menu->addMenu(tr("Язык"));

        act = new QAction(QString(tr("English")));
        UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateFileSetLanguage, LANG_EN)); act->setData(UpdateAction);
        connect(act, &QAction::triggered, this, [=]() {OnFileSetLanguage(LANG_EN);});
        act->setCheckable(true);
        menu1->addAction(act);

        act = new QAction(QString(tr("Russian")));
        UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateFileSetLanguage, LANG_RU)); act->setData(UpdateAction);
        connect(act, &QAction::triggered, this, [=]() {OnFileSetLanguage(LANG_RU);});
        act->setCheckable(true);
        menu1->addAction(act);

//        MENUITEM SEPARATOR
   menu->addSeparator();

//        MENUITEM "В&ыход",                      ID_APP_EXIT
    act = new QAction(QString(tr("Выход")), this);
    connect(act, &QAction::triggered, this, &CMainFrame::close);
    menu->addAction(act);

    // ===================================================================================

//    POPUP "&Конфигурация"
    menu = menuBar()->addMenu(tr("Конфигурация"));
    connect(menu, &QMenu::aboutToShow, this, &CMainFrame::OnMenuAboutToShow);
    tb->addSeparator();

//        MENUITEM "&Рестарт БК",                 ID_CPU_RESETCPU
    act = new QAction(makeIcon(3, tbMainImg), QString("&Рестарт БК"), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuResetCpu);
    menu->addAction(act);
    tb->addAction(act);

//        MENUITEM "СУ+Рестарт БК",               ID_CPU_SURESETCPU
    act = new QAction(QString(tr("СУ+Рестарт БК")), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuSuResetCpu);
    menu->addAction(act);

//        MENUITEM "&Длинный рестарт БК",         ID_CPU_LONGRESET
    act = new QAction(QString(tr("Длинный рестарт БК")), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuLongReset);
    menu->addAction(act);

    act = new QAction(QString(tr("Power cycle БК")), this);
    connect(act, &QAction::triggered, this, [=]{ SetupConfiguration(g_Config.GetBKModelNumber()); });
    menu->addAction(act);

//        MENUITEM SEPARATOR
    menu->addSeparator();

    QActionGroup *ag = new QActionGroup(this);

//        MENUITEM "Старт БК0010-01",             ID_CPU_RUNBK001001
    act = new QAction(QString(tr("Старт БК0010-01")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk001001)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + блок Фокал-МСТД", ID_CPU_RUNBK001001_FOCAL
    act = new QAction(QString(tr("Старт БК0010-01 + блок Фокал-МСТД")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk001001Focal)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001Focal);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + доп. 32кб ОЗУ", ID_CPU_RUNBK001001_32K
    act = new QAction(QString(tr("Старт БК0010-01 + доп. 32кб ОЗУ")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk00100132k)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk00100132k);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + стандартный КНГМД", ID_CPU_RUNBK001001_FDD
    act = new QAction(QString(tr("Старт БК0010-01 + стандартный КНГМД")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk001001Fdd)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001Fdd);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + контроллер A16M", ID_CPU_RUNBK001001_FDD16K
    act = new QAction(QString(tr("Старт БК0010-01 + контроллер A16M")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk001001Fdd16k)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001Fdd16k);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + контроллер СМК-512", ID_CPU_RUNBK001001_FDD_SMK512
    act = new QAction(QString(tr("Старт БК0010-01 + контроллер СМК-512")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk001001FddSmk512)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001FddSmk512);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0010-01 + контроллер Samara", ID_CPU_RUNBK001001_FDD_SAMARA
    act = new QAction(QString(tr("Старт БК0010-01 + контроллер Samara")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk001001FddSamara)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001FddSamara);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011 + МСТД",         ID_CPU_RUNBK0011
    act = new QAction(QString(tr("Старт БК0011 + МСТД")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk0011)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011 + стандартный КНГМД", ID_CPU_RUNBK0011_FDD
    act = new QAction(QString(tr("Старт БК0011 + стандартный КНГМД")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk0011Fdd)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011Fdd);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011 + контроллер А16М", ID_CPU_RUNBK0011_FDD_A16M
    act = new QAction(QString(tr("Старт БК0011 + контроллер А16М")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk0011FddA16m)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011FddA16m);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011 + контроллер СМК-512", ID_CPU_RUNBK0011_FDD_SMK512
    act = new QAction(QString(tr("Старт БК0011 + контроллер СМК-512")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk0011FddSmk512)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011FddSmk512);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011 + контроллер Samara", ID_CPU_RUNBK0011_FDD_SAMARA
    act = new QAction(QString(tr("Старт БК0011 + контроллер Samara")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk0011FddSamara)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011FddSamara);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011М + МСТД",        ID_CPU_RUNBK0011M
    act = new QAction(QString(tr("Старт БК0011М + МСТД")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk0011m)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011m);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011М + стандартный КНГМД", ID_CPU_RUNBK0011M_FDD
    act = new QAction(QString(tr("Старт БК0011М + стандартный КНГМД")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk0011mFDD)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011mFDD);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011М + контроллер A16M", ID_CPU_RUNBK0011M_FDD_A16M
    act = new QAction(QString(tr("Старт БК0011М + контроллер A16M")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk0011mFddA16m)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011mFddA16m);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011М + контроллер СМК-512", ID_CPU_RUNBK0011M_FDD_SMK512
    act = new QAction(QString(tr("Старт БК0011М + контроллер СМК-512")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk0011mFddSmk512)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011mFddSmk512);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM "Старт БК0011М + контроллер Samara", ID_CPU_RUNBK0011M_FDD_SAMARA
    act = new QAction(QString(tr("Старт БК0011М + контроллер Samara")), this);
    act->setCheckable(true);
    UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateCpuRunbk0011mFddSamara)); act->setData(UpdateAction);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk0011mFddSamara);
    ag->addAction(act);
    menu->addAction(act);

//        MENUITEM SEPARATOR
    menu->addSeparator();

//        MENUITEM "&Ускорить",                   ID_CPU_ACCELERATE
    act = new QAction(makeIcon(4, tbMainImg), QString(tr("Ускорить")), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuAccelerate);
    menu->addAction(act);
    tb->addAction(act);

//        MENUITEM "&Замедлить",                  ID_CPU_SLOWDOWN
    act = new QAction(makeIcon(5, tbMainImg), QString(tr("Замедлить")), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuSlowdown);
    menu->addAction(act);
    tb->addAction(act);

//        MENUITEM "&Стандартная скорость",       ID_CPU_NORMALSPEED
    act = new QAction(makeIcon(6, tbMainImg), QString(tr("Стандартная скорость.")), this);
    connect(act, &QAction::triggered, this, &CMainFrame::OnCpuNormalspeed);
    menu->addAction(act);
    tb->addAction(act);

    // ==============================================================================================

//    POPUP "&Вид"
    menu = menuBar()->addMenu(tr("Вид"));
    connect(menu, &QMenu::aboutToShow, this, &CMainFrame::OnMenuAboutToShow);
    tb->addSeparator();

//        MENUITEM "&Строка состояния",           ID_VIEW_STATUS_BAR
     act = new QAction(QString(tr("&Строка состояния")), this);
     act->setCheckable(true);
     act->setChecked(!statusBar()->isHidden());
     connect(act, &QAction::triggered, this, &CMainFrame::ToggleStatusBar);
     menu->addAction(act);

//     POPUP "Виртуальная &клавиатура"
     menu1 = menu->addMenu(tr("Виртуальная клавиатура"));
//     connect(menu1, &QMenu::aboutToShow, this, &CMainFrame::OnMenuAboutToShow);

         ag = new QActionGroup(this);

    //         MENUITEM "&Кнопочная",                  ID_VKBDTYPE_KEYS
//         m_pActions_Keybd_type[IDB_BITMAP_SOFT] =
         act = new QAction(QString(tr("Кнопочная")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateVkbdtypeKeys, IDB_BITMAP_SOFT)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, [=](){ m_paneBKVKBDView->SetKeyboardView(IDB_BITMAP_SOFT); m_paneBKVKBDView->show(); } );
         ag->addAction(act);
         menu1->addAction(act);

    //         MENUITEM "&Плёночная",                  ID_VKBDTYPE_MEMBRANE
//         m_pActions_Keybd_type[IDB_BITMAP_PLEN] =
         act = new QAction(QString(tr("Плёночная")), this);
         act->setCheckable(true);
         connect(act, &QAction::triggered, this, [=](){ m_paneBKVKBDView->SetKeyboardView(IDB_BITMAP_PLEN); m_paneBKVKBDView->show(); } );
         ag->addAction(act);
         menu1->addAction(act);
#if 0
//         POPUP "Установить размер экрана"
     menu1 = menu->addMenu(tr("Установить размер экрана"));
             ag = new QActionGroup(this);

//             MENUITEM "256 x 192 (x0.5)",            ID_VIEW_SCREENSIZE_256X192
//             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_256X192)] =
             act = new QAction(QString("256 x 192 (x0.5)"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "324 x 243",                   ID_VIEW_SCREENSIZE_324X243
//             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_324X243)] =
             act = new QAction(QString("324 x 243"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "432 x 324",                   ID_VIEW_SCREENSIZE_432X324
//             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_432X324)] =
             act = new QAction(QString("432 x 324"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "512 х 384 (x1)",              ID_VIEW_SCREENSIZE_512X384
//             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_512X384)] =
             act = new QAction(QString("512 х 384 (x1)"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);


//             MENUITEM "576 x 432",                   ID_VIEW_SCREENSIZE_576X432
//             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_576X432)] =
             act = new QAction(QString("576 x 432"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "768 x 576 (x1.5)",            ID_VIEW_SCREENSIZE_768X576
//             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_768X576)] =
             act = new QAction(QString("768 x 576 (x1.5)"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "1024 x 768 (x2)",             ID_VIEW_SCREENSIZE_1024X768
//             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_1024X768)] =
             act = new QAction(QString("1024 x 768 (x2)"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);

//             MENUITEM "Свой",                        ID_VIEW_SCREENSIZE_CUSTOM
//             m_pActions_ScreenSize[static_cast<int>(ScreenSizeNumber::SCREENSIZE_CUSTOM)] =
             act = new QAction(QString("Свой"), this);
             act->setCheckable(true);
             connect(act, &QAction::triggered, this, &CMainFrame::OnCpuRunbk001001);
             ag->addAction(act);
             menu1->addAction(act);
#endif
         //        MENUITEM SEPARATOR
         menu->addSeparator();

//             MENUITEM "Сг&лаживание",                ID_VIEW_SMOOTHING
//         m_Action_ViewSmoothing =
         act = new QAction(makeIcon(10, tbMainImg), QString(tr("Сглаживание")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateViewSmoothing)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnViewSmoothing);
         menu->addAction(act);
         tb->addAction(act);

//             MENUITEM "Во весь &экран",              ID_VIEW_FULLSCREENMODE
//         m_Action_ViewSmoothing =
         act = new QAction(makeIcon(7, tbMainImg), QString(tr("Во весь экран")), this);
         act->setCheckable(true);
         connect(act, &QAction::triggered, this, &CMainFrame::OnViewFullscreenmode);
         menu->addAction(act);
         tb->addAction(act);

//             MENUITEM "&Цветной режим",              ID_VIEW_COLORMODE
//         m_Action_ViewColormode =
         act = new QAction(makeIcon(8, tbMainImg), QString(tr("Цветной режим")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateViewColormode)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnViewColormode);
         menu->addAction(act);
         tb->addAction(act);

//             MENUITEM "&Адаптивный Ч/Б режим",       ID_VIEW_ADAPTIVEBWMODE
//         m_Action_ViewAdaptivebwmode =
         act = new QAction(makeIcon(9, tbMainImg), QString(tr("Адаптивный Ч/Б режим")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateViewAdaptivebwmode)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnViewAdaptivebwmode);
         menu->addAction(act);
         tb->addAction(act);

//             MENUITEM "&Эмуляция затухания люминофора", ID_VIEW_LUMINOFOREMODE
//         m_Action_ViewLuminoforemode =
         act = new QAction(QString(tr("Эмуляция затухания люминофора")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateViewLuminoforemode)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnViewLuminoforemode);
         menu->addAction(act);

         tb->addSeparator();
//         act = new QAction(makeIcon(11, tbMainImg), QString("&Load Drive A:"), this);
//         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateFileLoadDrive, 0)); act->setData(UpdateAction);
//         connect(act,&QAction::triggered, this, [=](){ CMainFrame::OnFileLoadDrive(0); });
//         tb->addAction(act);


         for(int i=0; i<4; i++) {
             QToolButton *FDDButton;
             QMenu *FddDropDownMenu;
             QWidgetAction *wact;
             QLabel *lact;

             FDDButton = m_ToolbarFDDButton[i] = new QToolButton(this);
                 FDDButton->setIcon(makeIcon(4 + i, tbFDDImg));
                 FDDButton->setPopupMode(QToolButton::InstantPopup);
                 FddDropDownMenu = new QMenu(FDDButton);
                 connect(FddDropDownMenu, &QMenu::aboutToShow, this, &CMainFrame::OnShowFddPopupMenu);

                 act = new QAction(FddDropDownMenu);
                 UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateFileLoadDrive, i)); act->setData(UpdateAction);
                 FddDropDownMenu->addAction(act);

                 wact = new QWidgetAction(FddDropDownMenu);
                 lact = new QLabel("", FddDropDownMenu);
                 lact->setAlignment(Qt::AlignCenter);
                 lact->setMinimumHeight(FddDropDownMenu->height()-1);
                 wact->setDefaultWidget(lact);
                 FddDropDownMenu->addAction(wact);

                 FDDButton->setMenu(FddDropDownMenu);
                 act = tb->addWidget(FDDButton);
                 UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateFileLoadDrive, i)); act->setData(UpdateAction);
         }

         act = new QAction(makeIcon(20, tbMainImg), QString(tr("Load HDD Master")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateFileLoadDrive, 4)); act->setData(UpdateAction);
         connect(act,&QAction::triggered, this, [=](){ CMainFrame::OnFileLoadDrive(4); });
         tb->addAction(act);

         act = new QAction(makeIcon(20, tbMainImg), QString(tr("Load HDD Slave")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateFileLoadDrive, 5)); act->setData(UpdateAction);
         connect(act,&QAction::triggered, this, [=](){ CMainFrame::OnFileLoadDrive(5); });
         tb->addAction(act);

         tb->addSeparator();

        act = new QAction(makeIcon(19, tbMainImg), QString(tr("Load Bin")), this);
        connect(act,&QAction::triggered, this, &CMainFrame::OnLoadBinFile);
        tb->addAction(act);


         tb->addSeparator();
         tb->addAction(aScrshot);
         tb->addAction(aPrint);

         tb = addToolBar(tr("Debug"));
         connect(tb, &QToolBar::actionTriggered, this, &CMainFrame::OnToolbarActionTriggered);


         m_Action_DebugStop_Stop = makeIcon(0, tbDbgImg);
         m_Action_DebugStop_Start = makeIcon(0, tbMenu1Img);

         menu = menuBar()->addMenu(tr("Отладка"));
         connect(menu, &QMenu::aboutToShow, this, &CMainFrame::OnMenuAboutToShow);

         m_Action_DebugStop = act = new QAction(m_Action_DebugStop_Stop, QString(tr("Стоп")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateDebugBreak)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugBreak);
         tb->addAction(act);
         menu->addAction(act);

         menu->addSeparator();

         act = new QAction(makeIcon(1, tbDbgImg), QString(tr("Шаг с заходом")), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugStepinto);
         tb->addAction(act);
         menu->addAction(act);

         act = new QAction(makeIcon(2, tbDbgImg), QString(tr("Шаг с обходом")), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugStepover);
         tb->addAction(act);
         menu->addAction(act);

         act = new QAction(makeIcon(3, tbDbgImg), QString(tr("Шаг с выходом")), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugStepout);
         tb->addAction(act);
         menu->addAction(act);

         act = new QAction(makeIcon(4, tbDbgImg), QString(tr("Выполнить до с&троки")), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugRuntocursor);
         tb->addAction(act);
         menu->addAction(act);

         act = new QAction(makeIcon(6, tbDbgImg), QString(tr("Карта памяти")), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugMemmap);
//         tb->addAction(act);
         menu->addAction(act);


         act = new QAction(QString(tr("Stop on System Interrupt")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateBreakOnSysInterrupt)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnBreakOnSysInterrupt);
         menu->addAction(act);

         act = new QAction(QString(tr("Stop on User Interrupt")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateBreakOnUserInterrupt)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnBreakOnUserInterrupt);
         menu->addAction(act);

         menu->addSeparator();

    //     act = new QAction(makeIcon(5, tbDbgImg), QString("Set Breakpoint"), this);
    //     connect(act, &QAction::triggered, this, &CMainFrame::OnCpuResetCpu);
    //     tb->addAction(act);

         menu = menuBar()->addMenu(tr("Инструменты"));
         connect(menu, &QMenu::aboutToShow, this, &CMainFrame::OnMenuAboutToShow);

         act = new QAction(QString(tr("Дамп регистров")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnMenuUpdateRegistryDumpView)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, [=](){ m_paneRegistryDumpViewCPU->Toggle(); } );
         menu->addAction(act);

         act = new QAction(QString(tr("Дамп памяти")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnMenuUpdateMemDumpView)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, [=](){ m_paneMemoryDumpView->Toggle(); } );
         menu->addAction(act);

         act = new QAction(QString(tr("Дизассемблер")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnMenuUpdateDisasmView)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, [=](){ m_paneDisassembleView->Toggle(); } );
         menu->addAction(act);

         act = new QAction(QString(tr("Дамп стека")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnMenuUpdateStackView)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, [=](){ m_paneStackView->Toggle(); } );
         menu->addAction(act);

         act = new QAction(QString(tr("Breakpoints")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnMenuUpdateBreakPointView)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, [=](){ m_paneBreakPointView->Toggle(); } );
         menu->addAction(act);

         act = new QAction(QString(tr("Symbol Table")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnMenuUpdateSymbolTableView)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, [=](){ m_paneSymbolTableView->Toggle(); } );
         menu->addAction(act);

         act = new QAction(QString(tr("Watchpoint Table")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnMenuUpdateWatchpointView)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, [=](){ m_paneWatchPointView->Toggle(); } );
         menu->addAction(act);

         menu->addSeparator();

         act = new QAction(QString(tr("Save Disassembled Area")), this);
         connect(act,&QAction::triggered, this, &CMainFrame::OnSaveDisasm);
         menu->addAction(act);

         menu1 = menu->addMenu(tr("Load tools"));

             act = new QAction(QString(tr("Load Symbol Table")), this);
             connect(act,&QAction::triggered, this, &CMainFrame::OnLoadSymbolTable);
             menu1->addAction(act);

             act = new QAction(QString(tr("Load Breakpoints")), this);
             connect(act,&QAction::triggered, this, &CMainFrame::OnLoadBreakpoints);
             menu1->addAction(act);

             act = new QAction(QString(tr("Load Watchpoints")), this);
             connect(act,&QAction::triggered, this, &CMainFrame::OnLoadWatchpoints);
             menu1->addAction(act);

             act = new QAction(QString(tr("Load Memory Region")), this);
             connect(act,&QAction::triggered, this, &CMainFrame::OnLoadMemoryRegion);
             menu1->addAction(act);


         menu1 = menu->addMenu(tr("Save tools"));

             act = new QAction(QString(tr("Save Symbol Table")), this);
             connect(act,&QAction::triggered, this, &CMainFrame::OnSaveSymbolTable);
             menu1->addAction(act);


             act = new QAction(QString(tr("Save Breakpoints")), this);
             connect(act,&QAction::triggered, this, &CMainFrame::OnSaveBreakpoints);
             menu1->addAction(act);


             act = new QAction(QString(tr("Save Watchpoints")), this);
             connect(act,&QAction::triggered, this, &CMainFrame::OnSaveWatchpoints);
             menu1->addAction(act);


             act = new QAction(QString(tr("Save Memory Region")), this);
             connect(act,&QAction::triggered, this, &CMainFrame::OnSaveMemoryRegion);
             menu1->addAction(act);


         act = new QAction(QString(tr("Run Lua Script")), this);
         connect(act,&QAction::triggered, this, &CMainFrame::OnRunLuaScript);
         menu->addAction(act);

//         POPUP "&Опции"
         menu = menuBar()->addMenu(tr("Опции"));
         connect(menu, &QMenu::aboutToShow, this, &CMainFrame::OnMenuAboutToShow);

         tb = addToolBar(tr("Options"));
         connect(tb, &QToolBar::actionTriggered, this, &CMainFrame::OnToolbarActionTriggered);


//             MENUITEM "&Настройки эмулятора",        ID_APP_SETTINGS
         act = new QAction(QString(tr("Настройки эмулятора")), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugRuntocursor);
         menu->addAction(act);

//             MENUITEM "&Редактор Палитр",            ID_OPTIONS_PALETTE
         act = new QAction(QString(tr("Редактор Палитр")), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugRuntocursor);
         menu->addAction(act);

//             MENUITEM "Редактор Джойстика",          ID_OPTIONS_JOYEDIT
         act = new QAction(QString(tr("Редактор Джойстика")), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnDebugRuntocursor);
         menu->addAction(act);

//             MENUITEM SEPARATOR
         menu->addSeparator();

//             MENUITEM "Включить Speaker",           ID_OPTIONS_ENABLE_SPEAKER
         act = new QAction(makeIcon(0,tbSndImg), QString(tr("Включить &Speaker")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsEnableSpeaker)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsEnableSpeaker);
         menu->addAction(act);
         tb->addAction(act);

//             MENUITEM "Включить Covox",             ID_OPTIONS_ENABLE_COVOX
         act = new QAction(makeIcon(1,tbSndImg), QString(tr("Включить &Covox")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsEnableCovox)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsEnableCovox);
         act->setCheckable(true);
         menu->addAction(act);
         tb->addAction(act);

//             MENUITEM "Стерео Covox",               ID_OPTIONS_STEREO_COVOX
         act = new QAction(makeIcon(4,tbSndImg), QString(tr("Сте&рео Covox")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsStereoCovox)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsStereoCovox);
         act->setCheckable(true);
         menu->addAction(act);

//             MENUITEM "Включить AY8910",            ID_OPTIONS_ENABLE_AY8910
         act = new QAction(makeIcon(2,tbSndImg), QString(tr("Включить &AY8910")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsEnableAy8910)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsEnableAy8910);
         act->setCheckable(true);
         menu->addAction(act);
         tb->addAction(act);

 //             MENUITEM "Включить Менестрель",            ID_OPTIONS_ENABLE_AY8910
          act = new QAction(makeIcon(3,tbSndImg), QString(tr("Включить Менестрель")), this);
          UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsEnableMenestrel)); act->setData(UpdateAction);
          connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsEnableMenestrel);
          act->setCheckable(true);
          menu->addAction(act);
          tb->addAction(act);

          m_pVolumeSlider = new QSlider(Qt::Horizontal,this);
          m_pVolumeSlider->setToolTip(tr("Громкость"));
          m_pVolumeSlider->setRange(0, 65535);
          m_pVolumeSlider->setMaximumWidth(100);
          connect(m_pVolumeSlider, &QSlider::valueChanged, this, &CMainFrame::OnLVolumeSlider);
          act = tb->addWidget(m_pVolumeSlider);

//             MENUITEM tr("Дамп регистров A&Y8910"),      ID_OPTIONS_LOG_AY8910
         act = new QAction(makeIcon(10,tbSndImg), QString(tr("Дамп регистров A&Y8910")), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsLogAy8910);
         menu->addAction(act);

//             MENUITEM "Параметры AY8910",            ID_OPTIONS_AYVOLPAN
         act = new QAction(makeIcon(9,tbSndImg), QString(tr("Параметры AY8910")), this);
         connect(act, &QAction::triggered, this, &CMainFrame::OnSettAyvolpan);
         menu->addAction(act);

//             POPUP "Фильтры"
         menu1 = menu->addMenu(tr("Фильтры"));
//         connect(menu1, &QMenu::aboutToShow, this, &CMainFrame::OnMenuAboutToShow);

//                 MENUITEM "Speaker",                     ID_OPTIONS_SPEAKER_FILTER
         act = new QAction(makeIcon(5,tbSndImg), QString(tr("Speaker")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsSpeakerFilter)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsSpeakerFilter);
         menu1->addAction(act);

//                 MENUITEM "Covox",                       ID_OPTIONS_COVOX_FILTER
         act = new QAction(makeIcon(6,tbSndImg), QString(tr("Covox")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsCovoxFilter)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsCovoxFilter);
         menu1->addAction(act);

//                 MENUITEM "AY8910",                      ID_OPTIONS_AY8910_FILTER
         act = new QAction(makeIcon(7,tbSndImg), QString(tr("AY8910")), this);
         act->setCheckable(true);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsAy8910Filter)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsAy8910Filter);
         menu1->addAction(act);

 //                 MENUITEM "Menestrel",                      ID_OPTIONS_AY8910_FILTER
          act = new QAction(makeIcon(8,tbSndImg), QString(tr("Menestrel")), this);
          act->setCheckable(true);
          UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsMenestrelFilter)); act->setData(UpdateAction);
          connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsMenestrelFilter);
          menu1->addAction(act);


//             MENUITEM SEPARATOR
         menu->addSeparator();

//             MENUITEM "Эмулировать клавиатуру БК",  ID_OPTIONS_EMULATE_BKKEYBOARD
         act = new QAction(QString(tr("Эмулировать &клавиатуру БК")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsEmulateBkkeyboard)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsEmulateBkkeyboard);
         act->setCheckable(true);
         menu->addAction(act);

//             MENUITEM "Включить &джойстик",          ID_OPTIONS_ENABLE_JOYSTICK
         act = new QAction(QString(tr("Включить джойстик")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsEnableJoystick)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsEnableJoystick);
         act->setCheckable(true);
         menu->addAction(act);

//             MENUITEM "&Эмулировать ввод-вывод дисковода", ID_OPTIONS_EMULATE_FDDIO
         act = new QAction(QString(tr("Эмулировать ввод-вывод дисковода")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsEmulateFddio)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsEmulateFddio);
         act->setCheckable(true);
         menu->addAction(act);

//             MENUITEM "&Исп. папку Saves по умолчанию (BASIC или Focal)", ID_OPTIONS_USE_SAVESDIRECTORY
         act = new QAction(QString(tr("Исп. папку Saves по умолчанию (BASIC или Focal)")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsUseSavesdirectory)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsUseSavesdirectory);
         act->setCheckable(true);
         menu->addAction(act);

//             MENUITEM SEPARATOR
         menu->addSeparator();

//             MENUITEM "Эмулировать загрузку &ленты", ID_OPTIONS_EMULATE_TAPE_LOADING
         act = new QAction(QString(tr("Эмулировать загрузку ленты")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsEmulateTapeLoading)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsEmulateTapeLoading);
         act->setCheckable(true);
         menu->addAction(act);

//             MENUITEM "Эмулировать &сохранение на ленту", ID_OPTIONS_EMULATE_TAPE_SAVING
         act = new QAction(QString(tr("Эмулировать сохранение на ленту")), this);
         UpdateAction.setValue(UpdateFunc(&CMainFrame::OnUpdateOptionsEmulateTapeSaving)); act->setData(UpdateAction);
         connect(act, &QAction::triggered, this, &CMainFrame::OnOptionsEmulateTapeSaving);
         act->setCheckable(true);
         menu->addAction(act);

         addToolBar(&m_paneTapeCtrlView);

//             MENUITEM SEPARATOR
//             MENUITEM "&Менеджер лент БК...",        ID_OPTIONS_TAPEMANAGER
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
    MENUITEM "&Исп. папку Saves по умолчанию (BASIC или Focal)", ID_OPTIONS_USE_SAVESDIRECTORY
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
    MENUITEM "&Диалог при ошибке доступа к памяти", ID_DEBUG_DIALOG_ASK_FOR_BREAK
    MENUITEM "Ост&анов CPU после запуска эмулятора", ID_DEBUG_PAUSE_CPU_AFTER_START
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
