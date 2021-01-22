#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#pragma once

#include <QMainWindow>
#include "pch.h"

//#include "BKView.h"
//#include "RegDumpViewCPU.h"
//#include "RegDumpViewFDD.h"
//#include "MemDumpView.h"
//#include "DisasmView.h"
//#include "TapeCtrlView.h"
//#include "OscillatorView.h"
//#include "BKVKBDView.h"

//#include "DropTarget.h"
//#include "ScriptRunner.h"
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

#define WPARAM(a) a

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
//    CBKVKBDView         m_paneBKVKBDView;           // панель виртуальной клавиатуры

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

//    CScriptRunner       m_Script;               // объект обработчика скриптов
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

    bool                LoadMemoryState(CString &strPath);
    bool                SaveMemoryState(CString &strPath);

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

    void                OnMainLoop() {};       // функция вызова по таймеру
    void                OnMainLoopTime() {};   // функция вызова по таймеру
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

//    inline CBKVKBDView *GetBKVKBDViewPtr()
//    {
//        return &m_paneBKVKBDView;
//    }

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
};
#endif // MAINWINDOW_H
