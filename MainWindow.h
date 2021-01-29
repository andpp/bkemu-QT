#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include <QLabel>
#include "pch.h"

#include "BKView.h"
//#include "RegDumpViewCPU.h"
//#include "RegDumpViewFDD.h"
//#include "MemDumpView.h"
//#include "DisasmView.h"
//#include "TapeCtrlView.h"
//#include "OscillatorView.h"
#include "BKVKBDView.h"

//#include "DropTarget.h"
#include "ScriptRunner.h"
#include "BKSound.h"
#include "Speaker.h"
#include "Covox.h"
#include "emu2149.h"
#include "Tape.h"
#include "Config.h"
#include "Board.h"
#include "Debugger.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum: int {
    ID_FILE_LOADDRIVE_A = 0,
    ID_FILE_LOADDRIVE_B,
    ID_FILE_LOADDRIVE_C,
    ID_FILE_LOADDRIVE_D
};

enum: int {
    ID_FILE_UMOUNT_A = 0,
    ID_FILE_UMOUNT_B,
    ID_FILE_UMOUNT_C,
    ID_FILE_UMOUNT_D
};

class CMainFrame : public QMainWindow
{
    Q_OBJECT

public:
    CMainFrame(QWidget *parent = nullptr);

private:
    Ui::MainWindow *ui;

public:
//    CRegDumpViewCPU     m_paneRegistryDumpViewCPU;  // панель дампа регистров CPU
//    CRegDumpViewFDD     m_paneRegistryDumpViewFDD;  // панель дампа регистров FDD
//    CMemDumpView        m_paneMemoryDumpView;       // панель дампа памяти
//    CDisasmView         m_paneDisassembleView;      // панель отладчика
//    CTapeCtrlView       m_paneTapeCtrlView;         // панель управления записью
//    COscillatorlView    m_paneOscillatorView;       // панель осциллографа
    CBKVKBDView         *m_paneBKVKBDView;           // панель виртуальной клавиатуры

    CBKView            *m_pBKView;

//    CMutex              m_mtInstance;           // мутекс, предназначенный для запуска только одной копии программы (пока не функционирует как надо)
    UINT                m_nInterAppGlobalMsg;   // индекс юзерского сообщения, которое будет зарегистрировано

//    CBKMEMDlg          *m_pBKMemView;           // объект отображения карты памяти
    bool                m_bBKMemViewOpen;       // флаг, что m_pBKMemView достоверен
    CRect               m_rectMemMap;           // координаты окна карты памяти
    // Эмулятор
    CMotherBoard       *m_pBoard;               // объект материнской платы БК
    CBkSound           *m_pSound;               // модуль звуковой подсистемы
    CDebugger          *m_pDebugger;
    CScreen            *m_pScreen;              // модуль подсистемы вывода на экран

    CSpeaker            m_speaker;              // объект пищалка
    CCovox              m_covox;                // объект ковокс
    CEMU2149            m_ay8910;               // объект сопроцессор Ay8910-3
    CTape               m_tape;                 // объект обработчика кассет

    CScriptRunner       m_Script;               // объект обработчика скриптов
//    CDropTarget         m_dropTarget;           // объект поддержки драг-н-дропа (вроде даже работает)
    // счётчики
    bool                m_bBeginPeriod;
    DWORD               m_nStartTick;
    int                 m_nRegsDumpCounter;     // счётчик. через сколько 20мс интервалов обновлять на экране дамп регистров
    bool                m_bLongResetPress;      // специально для А16М, вводим длинный ресет

    // имена файлов, получаемых через ДнД или командную строку
    CString             m_strBinFileName;
    CString             m_strMemFileName;
    CString             m_strTapeFileName;
    CString             m_strScriptFileName;
    // размеры экрана
    enum ScreenSizeNumber : int
    {
        SCREENSIZE_CUSTOM = 0,
        SCREENSIZE_256X192,
        SCREENSIZE_324X243,
        SCREENSIZE_432X324,
        SCREENSIZE_512X384,
        SCREENSIZE_576X432,
        SCREENSIZE_768X576,
        SCREENSIZE_1024X768,
        SCREENSIZE_NUMBER
    };
    ScreenSizeNumber    m_nScreenSize;
    int                 m_nScreen_X, m_nScreen_Y, m_nScreen_CustomX, m_nScreen_CustomY;
    static CPoint       m_aScreenSizes[SCREENSIZE_NUMBER];

    bool                m_bFoundFFMPEG;

    void                InitWindows();

protected:
    // функции инициализации эмулятора
    void                InitKbdStatus();
    void                InitEmulator();
    void                InitRegistry();

//    void                RegisterDefaultIcon(CString &strName, initRegRaram *regParam);
//    void                RegisterShellCommand(CString &strName, initRegRaram *regParam);

    bool                ParseCommandLineParameters(CString strCommands);
    void                SetupConfiguration(CONF_BKMODEL nConf = CONF_BKMODEL::BK_0010_01);
    bool                ConfigurationConstructor(CONF_BKMODEL nConf, bool bStart = true);
    bool                ConfigurationConstructor_LoadConf(CONF_BKMODEL nConf);
    void                AttachObjects();

    void                UpdateTapeDlgControls();   // изменение состояния кнопок Запись и Стоп в панели управления записью
    void                UpdateToolbarDriveIcons();
    void                UpdateToolbarSize();
    void                ResetToolbar(UINT uiToolBarId);

    bool                LoadMemoryState(const CString &strPath);
    bool                SaveMemoryState(const CString &strPath);

    void                SetDebugCtrlsState();

    bool                MakeScreenShot();

    bool                StartPlayTape(const CString &strPath);
    void                ClearProcessingFiles();
    bool                ProcessFile(bool bCreate = false);
    // работа с иконками загрузки/выгрузки образов на панели инструментов
    void                LoadFileImage(UINT nBtnID, FDD_DRIVE eDrive);
    void                ChangeImageIcon(UINT nBtnID, FDD_DRIVE eDrive);

    inline void         StartTimer()
    {
        BKTIMER_UI_REFRESH = startTimer(20);  // запустить таймер для OnMainLoop
        BKTIMER_UI_TIME = startTimer(1000);   // запустить таймер для OnMainLoopTimer
    }
    inline void         StopTimer()
    {
        killTimer(BKTIMER_UI_REFRESH);      // остановить таймер для OnMainLoop
        killTimer(BKTIMER_UI_TIME);         // остановить таймер для OnMainLoopTimer
    }

    inline void timerEvent(QTimerEvent *event) override {
        if (event->timerId() == BKTIMER_UI_REFRESH) OnMainLoop();
        else if (event->timerId() == BKTIMER_UI_TIME) OnMainLoopTime();
    }

    void                OnMainLoop();       // функция вызова по таймеру
    void                OnMainLoopTime();   // функция вызова по таймеру
    void                StopAll();          // остановить всё - процессор, BKTIMER_UI_REFRESH
    void                StartAll();         // запустить всё - процессор, BKTIMER_UI_REFRESH
    bool                CheckDebugMemmap();
    CString             MakeUniqueName();
    UINT                GetTBID(UINT);
// Переопределение
public:
//    virtual BOOL        LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd *pParentWnd = nullptr, CCreateContext *pContext = nullptr) override;
//    virtual BOOL        OnShowPopupMenu(CMFCPopupMenu *pMenuPopup) override;
//    virtual BOOL        PreTranslateMessage(MSG *pMsg) override;

// Реализация
public:
//    CMainFrame();
    virtual ~CMainFrame() override;
#ifdef _DEBUG
    virtual void        AssertValid() const override;
    virtual void        Dump(CDumpContext &dc) const override;
#endif
    void                SetFocusToBK();
    void                SetFocusToDebug();

    void                SendMessage(uint msgCode) { (void)msgCode;}
    void                PostMessage(uint msgCode, uint param = 0) { (void)msgCode; (void)param;}

    void                SetScreen(CScreen *scr, CDebugger *dbg)
    {
        m_pScreen = scr;   // задание объекта экранной подсистемы. Он создаётся в другом месте
        m_pDebugger = dbg;
    }
    // выдача указателей на объекты всем желающим
    inline CBkSound     *GetBKSoundPtr()
    {
        return m_pSound;
    }
    inline CSpeaker     *GetSpeakerPtr()
    {
        return &m_speaker;
    }
    inline CCovox       *GetCovoxPtr()
    {
        return &m_covox;
    }
    inline CEMU2149     *GetAY8910Ptr()
    {
        return &m_ay8910;
    }
    inline CTape        *GetTapePtr()
    {
        return &m_tape;
    }
//    inline CScriptRunner *GetScriptRunnerPtr()
//    {
//        return &m_Script;
//    }
//    inline COscillatorlView *GetOscillatorViewPtr()
//    {
//        return &m_paneOscillatorView;
//    }

    inline CBKVKBDView *GetBKVKBDViewPtr()
    {
        return m_paneBKVKBDView;
    }

    inline CMotherBoard *GetBoard()
    {
        return m_pBoard;
    }
    inline CScreen *GetScreen()
    {
        return m_pScreen;
    }
    inline CString *GetStrBinFileName()
    {
        return &m_strBinFileName;
    }
public:
    void  setStatusLine(const CString &str) { m_pSB_StatusLine->setText(str); }

private:
    QLabel *m_pSB_StatusLine;  // индикатор строки состояния
    QLabel *m_pSB_RusLat;      // тут выводится ЛАТ/РУС
    QLabel *m_pSB_ZaglStr;     // тут выводится ЗАГЛ/СТР
    QLabel *m_pSB_AR2;         // тут выводится АР2
    QLabel *m_pSB_SU;          // тут выводится СУ
    QLabel *m_pSB_Common;      // тут выводится всякая общая инфа
    QLabel *m_pSB_CAPS;
    QLabel *m_pSB_NUM;
    QLabel *m_pSB_SCR;

    void CreateMenu();

    void resizeEvent(QResizeEvent*) override;
    void closeEvent(QCloseEvent *event) override;

protected:
    // Созданные функции схемы сообщений
            bool CreateDockingWindows();
            void SetDockingWindowIcons(bool bHiColorIcons);
            void OnMemMapClose();      // событие передаваемое из объекта карты памяти, говорящее, что оно закрывается, и не надо больше его вызывать
            void OnMemDumpUpdate();
            void OnDropFile();
            void OnToolbarCreateNew();
            void OnToolbarReset();
            void OnScreenSizeChanged(uint width, uint height);
            void OnResetKbdManager();
            void OnCpuBreak();
            void OnOutKeyboardStatus();
            void OnStartPlatform();

            void OnDebugDrawScreen();
            void OnDrawBKScreen();
            void OnDrawOscilloscope();

//            int OnCreate(LPCREATESTRUCT lpCreateStruct);
            void OnViewCustomize();
            void OnApplicationLook(UINT id);
//            void OnUpdateApplicationLook(CCmdUI *pCmdUI);
            void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
//            void OnLVolumeSlider(NMHDR *pNMHDR, LRESULT *pResult);

            void OnClose();
//            BOOL OnCopyData(CWnd *pWnd, COPYDATASTRUCT *pCopyDataStruct);
//            void OnTimer(UINT_PTR nIDEvent);

    // меню Файл
            void OnFileLoadstate();
            void OnFileSavestate();
            void OnFileLoadtape();
//            void OnUpdateFileLoadtape(CCmdUI *pCmdUI);
            void OnFileScreenshot();
    // меню Конфигурация
            void OnCpuResetCpu();
            void OnCpuSuResetCpu();
            void OnCpuLongReset();
            void OnCpuRunbk001001();
//            void OnUpdateCpuRunbk001001(CCmdUI *pCmdUI);
            void OnCpuRunbk001001Focal();
//            void OnUpdateCpuRunbk001001Focal(CCmdUI *pCmdUI);
            void OnCpuRunbk00100132k();
//            void OnUpdateCpuRunbk00100132k(CCmdUI *pCmdUI);
            void OnCpuRunbk001001Fdd();
//            void OnUpdateCpuRunbk001001Fdd(CCmdUI *pCmdUI);
            void OnCpuRunbk001001Fdd16k();
//            void OnUpdateCpuRunbk001001Fdd16k(CCmdUI *pCmdUI);
            void OnCpuRunbk001001FddSmk512();
//            void OnUpdateCpuRunbk001001FddSmk512(CCmdUI *pCmdUI);
            void OnCpuRunbk001001FddSamara();
//            void OnUpdateCpuRunbk001001FddSamara(CCmdUI *pCmdUI);
            void OnCpuRunbk0011();
//            void OnUpdateCpuRunbk0011(CCmdUI *pCmdUI);
            void OnCpuRunbk0011Fdd();
//            void OnUpdateCpuRunbk0011Fdd(CCmdUI *pCmdUI);
            void OnCpuRunbk0011FddA16m();
//            void OnUpdateCpuRunbk0011FddA16m(CCmdUI *pCmdUI);
            void OnCpuRunbk0011FddSmk512();
//            void OnUpdateCpuRunbk0011FddSmk512(CCmdUI *pCmdUI);
            void OnCpuRunbk0011FddSamara();
//            void OnUpdateCpuRunbk0011FddSamara(CCmdUI *pCmdUI);
            void OnCpuRunbk0011m();
//            void OnUpdateCpuRunbk0011m(CCmdUI *pCmdUI);
            void OnCpuRunbk0011mFDD();
//            void OnUpdateCpuRunbk0011mFDD(CCmdUI *pCmdUI);
            void OnCpuRunbk0011mFddA16m();
//            void OnUpdateCpuRunbk0011mFddA16m(CCmdUI *pCmdUI);
            void OnCpuRunbk0011mFddSmk512();
//            void OnUpdateCpuRunbk0011mFddSmk512(CCmdUI *pCmdUI);
            void OnCpuRunbk0011mFddSamara();
//            void OnUpdateCpuRunbk0011mFddSamara(CCmdUI *pCmdUI);
            void OnCpuAccelerate();
//            void OnUpdateCpuAccelerate(CCmdUI *pCmdUI);
            void OnCpuSlowdown();

            //            void OnUpdateCpuSlowdown(CCmdUI *pCmdUI);
            void OnCpuNormalspeed();
    // меню Опции
            void OnOptionsEnableSpeaker();
//            void OnUpdateOptionsEnableSpeaker(CCmdUI *pCmdUI);
            void OnOptionsEnableCovox();
//            void OnUpdateOptionsEnableCovox(CCmdUI *pCmdUI);
            void OnOptionsStereoCovox();
//            void OnUpdateOptionsStereoCovox(CCmdUI *pCmdUI);
            void OnOptionsEnableAy8910();
//            void OnUpdateOptionsEnableAy8910(CCmdUI *pCmdUI);
            void OnOptionsSpeakerFilter();
//            void OnUpdateOptionsSpeakerFilter(CCmdUI *pCmdUI);
            void OnOptionsCovoxFilter();
//            void OnUpdateOptionsCovoxFilter(CCmdUI *pCmdUI);
            void OnOptionsAy8910Filter();
//            void OnUpdateOptionsAy8910Filter(CCmdUI *pCmdUI);
            void OnOptionsLogAy8910();
//            void OnUpdateOptionsLogAy8910(CCmdUI *pCmdUI);
            void OnOptionsEmulateBkkeyboard();
//            void OnUpdateOptionsEmulateBkkeyboard(CCmdUI *pCmdUI);
            void OnOptionsEnableJoystick();
//            void OnUpdateOptionsEnableJoystick(CCmdUI *pCmdUI);
            void OnOptionsEmulateFddio();
//            void OnUpdateOptionsEmulateFddio(CCmdUI *pCmdUI);
            void OnOptionsUseSavesdirectory();
//            void OnUpdateOptionsUseSavesdirectory(CCmdUI *pCmdUI);
            void OnOptionsEmulateTapeLoading();
//            void OnUpdateOptionsEmulateTapeLoading(CCmdUI *pCmdUI);
            void OnOptionsEmulateTapeSaving();
//            void OnUpdateOptionsEmulateTapeSaving(CCmdUI *pCmdUI);
            void OnOptionsTapemanager();
            void OnAppSettings();
            void OnPaletteEdit();
            void OnOptionsJoyedit();
            void OnSettAyvolpan();
    // меню Отладка
            void OnDebugBreak();
//            void OnUpdateDebugBreak(CCmdUI *pCmdUI);
            void OnDebugStepinto();
//            void OnUpdateDebugStepinto(CCmdUI *pCmdUI);
            void OnDebugStepover();
//            void OnUpdateDebugStepover(CCmdUI *pCmdUI);
            void OnDebugStepout();
//            void OnUpdateDebugStepout(CCmdUI *pCmdUI);
            void OnDebugRuntocursor();
//            void OnUpdateDebugRuntocursor(CCmdUI *pCmdUI);
            void OnDebugBreakpoint();
            void OnDebugMemmap();
            void OnDebugDumpregsInterval(UINT id);
//            void OnUpdateDebugDumpregsInterval(CCmdUI *pCmdUI);
            void OnDebugDialogAskForBreak();
//            void OnUpdateDebugDialogAskForBreak(CCmdUI *pCmdUI);
            void OnDebugPauseCpuAfterStart();
//            void OnUpdateDebugPauseCpuAfterStart(CCmdUI *pCmdUI);
            void OnDebugEnableIclblock();
//            void OnUpdateDebugEnableIclblock(CCmdUI *pCmdUI);
    // меню Вид
            void OnOptionsShowPerformanceOnStatusbar();
//            void OnUpdateOptionsShowPerformanceOnStatusbar(CCmdUI *pCmdUI);
            void OnVkbdtypeKeys(UINT id);
//            void OnUpdateVkbdtypeKeys(CCmdUI *pCmdUI);
            void OnViewSmoothing();
//            void OnUpdateViewSmoothing(CCmdUI *pCmdUI);
            void OnViewFullscreenmode();
            void OnViewColormode();
//            void OnUpdateViewColormode(CCmdUI *pCmdUI);
            void OnViewAdaptivebwmode();
//            void OnUpdateViewAdaptivebwmode(CCmdUI *pCmdUI);
            void OnViewLuminoforemode();
//            void OnUpdateViewLuminoforemode(CCmdUI *pCmdUI);
            void OnSetScreenSize(UINT id);
//            void OnUpdateSetScreenSize(CCmdUI *pCmdUI);
    // меню Инструменты
            void OnToolLaunch(UINT id);
    // тулбар для работы с дискетами и их меню
            void OnFileLoadDrive(UINT id);
//            void OnUpdateFileLoadDrive(CCmdUI *pCmdUI);
            void OnFileUnmount(UINT id);
    // Захват видео
            void OnVideoCaptureStart();
//            void OnUpdateVideoCaptureStart(CCmdUI *pCmdUI);
            void OnVideoCaptureStop();
//            void OnUpdateVideoCaptureStop(CCmdUI *pCmdUI);
};
#endif // MAINWINDOW_H
