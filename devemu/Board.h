// Board.h: interface for the CMotherBoard class.
//

#pragma once

//#include <afxtempl.h>

#include <QThread>
#include <QFileDialog>
#include <QApplication>

#include "Device.h"
#include "CPU.h"
#include "FDDController.h"
#include "Debugger.h"
#include "Screen.h"
#include "BKSound.h"
#include "Speaker.h"
#include "Covox.h"
#include "emu2149.h"
#include "Menestrel.h"
#include "MSFManager.h"
#include "ExceptionHalt.h"
#include "MainWindow.h"
#include "Config.h"

constexpr auto BRD_10_MON10_BNK = 8;
constexpr auto BRD_10_BASIC10_1_BNK = 10;
constexpr auto BRD_10_BASIC10_2_BNK = 12;
constexpr auto BRD_10_REGISTERS_BNK = 14;

#include <thread>

class CMainFrame;

class FileDialogCaller : public QObject {
  Q_OBJECT

public:
  FileDialogCaller(QObject* parent = 0) : QObject(parent) {
    // The helper object will live in the GUI thread
    moveToThread(qApp->thread());
  }

  // Add the rest of parameters as needed
  QString getSaveFileName(QWidget* parent, const QString& caption, const QString& dir,
                          const QString& filter) {
    QString fileName;

    if (QThread::currentThread() != qApp->thread()) { // no GUI thread
      QMetaObject::invokeMethod(this, "getSaveFileName_", Qt::BlockingQueuedConnection,
                                Q_RETURN_ARG(QString, fileName),
                                Q_ARG(QWidget*, parent),
                                Q_ARG(QString, caption),
                                Q_ARG(QString, dir),
                                Q_ARG(QString, filter));
    } else { // in GUI thread, direct call
      fileName = getSaveFileName_(parent, caption, dir, filter);
    }

    return fileName;
  }

  // Add the rest of parameters as needed
  QString getOpenFileName(QWidget* parent, const QString& caption, const QString& dir,
                          const QString& filter) {
    QString fileName;

    if (QThread::currentThread() != qApp->thread()) { // no GUI thread
      QMetaObject::invokeMethod(this, "getOpenFileName_", Qt::BlockingQueuedConnection,
                                Q_RETURN_ARG(QString, fileName),
                                Q_ARG(QWidget*, parent),
                                Q_ARG(QString, caption),
                                Q_ARG(QString, dir),
                                Q_ARG(QString, filter));
    } else { // in GUI thread, direct call
      fileName = getOpenFileName_(parent, caption, dir, filter);
    }

    return fileName;
  }


private:
  Q_INVOKABLE QString getSaveFileName_(QWidget* parent, const QString& caption, const QString& dir,
                          const QString& filter) {
    return QFileDialog::getSaveFileName(parent, caption, dir, filter);
  }
  Q_INVOKABLE QString getOpenFileName_(QWidget* parent, const QString& caption, const QString& dir,
                          const QString& filter) {
    return QFileDialog::getOpenFileName(parent, caption, dir, filter);
  }
};


class CMotherBoard : public CDevice
{
    Q_OBJECT

		friend class CCPU;
		friend class CFDDController;

	public:

		// регистры, публичные, т.к. лень на каждый регистр писать функцию, хотя и можно.
		uint16_t            m_reg177660;
		uint16_t            m_reg177662in;
		uint16_t            m_reg177662out;
		uint16_t            m_reg177664;
		uint16_t            m_reg177714in;
		uint16_t            m_reg177714out;
		uint16_t            m_reg177716in;
		uint16_t            m_reg177716out_tap;
		uint16_t            m_reg177716out_mem;
		enum                { ADDRESS_NONE = -1, GO_INTO = -2, GO_OUT = -3 };

	protected:
        CMainFrame         *m_pParent;          // указатель на родительский фрейм, куда всякие сообщения посылаются

        struct ThreadVars_t
		{
			int     nGotoAddress;       // адрес, на котором будет сделан отладочный останов

			int     nCPUTicks;          // текущий счётчик тактов процессора
			double  fCpuTickTime;

            int     nBoardTicksMax;     // How many CPU ticks between clock synchronization
            int     nBoardTicks;
            long    nBoard_Mod;         // nanoseconds between clock synchronization

			double  fMediaTicks;        // счётчик медиа тактов, происходящих за одну инструкцию
			double  fMedia_Mod;
			double  fMemoryTicks;
			double  fMemory_Mod;
			double  fFDDTicks;
			double  fFDD_Mod;

			// переменные для медиатактов
			int     nMediaTicksPerFrame; // количество медиатактов во фрейме (длина звукового буфера в сэмплах).
			int     nMediaTickCount;    // счётчик медиа тактов (их должно быть не более g_Config.m_nSoundSampleRate/CPU_FRAMES_PER_SECOND в текущем фрейме)
			int     nBufPos;            // позиция в звуковом буфере
			SAMPLE_INT *pSoundBuffer;   // звуковой буфер

			// переменные для эмуляции луча ЭЛТ
			uint16_t nVideoAddress;     // видео адрес, младшие 6 бит - счётчик строк внутри строки
			bool    bHgate;             // флаг отсчёта служебных видеоциклов в строке
			bool    bVgate;             // флаг отсчёта служебных строк
			int     nVGateCounter;      // дополнительный счётчик служебных строк
			int     nLineCounter;       // счётчик видео строк

			void init()
			{
				nGotoAddress = ADDRESS_NONE;
                nBoardTicksMax = 100;   // Synchronize clock every 100 CPU ticks
				clear();
				fCpuTickTime = 0.0;
				fMedia_Mod = 0.0;
				fMemory_Mod = 0.0;
				fFDD_Mod = 0.0;
				nMediaTicksPerFrame = 0;
				nMediaTickCount = 0;
				nBufPos = 0;
				pSoundBuffer = nullptr;
			}
			void clear()
			{
				nCPUTicks = 0;
				fMediaTicks = 0.0;
				fMemoryTicks = 0.0;
				fFDDTicks = 0.0;
				nVideoAddress = 0; // видео адрес, младшие 6 бит - счётчик строк внутри строки
				bHgate = false;
				bVgate = true;
				nVGateCounter = 64;
				nLineCounter = 0; // счётчик видео строк
			}
		};

		BK_DEV_MPI         m_BoardModel;       // тип модели БК 10 или 11 или 11М
		uint16_t            m_nBKPortsIOArea;   // адрес, выше которого нет памяти, и все адреса считаются регистрами и портами
		uint16_t            m_nStartAddr;       // адрес запуска материнской платы

		int                 m_nLowBound;        // нижняя граница, меньше которой не может быть частота
		int                 m_nHighBound;       // верхняя граница, больше которой не может быть частота

		int                 m_nCPUFreq;         // текущая частота работы.
		int                 m_nCPUFreq_prev;    // текущая частота работы.
		int                 m_nBaseCPUFreq;     // базовая частота работы, по которой вычисляется множитель

		ThreadVars_t        m_sTV;              // блок переменных, используемых в потоке

		CFDDController      m_fdd;              // контроллер 1801ВП1-128.
		CCPU                m_cpu;              // Процессор 1801ВМ1
		uint8_t            *m_pMemory;          // Основная память БК + дополнительная для контроллеров
		BKMEMBank_t         m_MemoryMap[16];    // карта памяти 64кбайтного адресного пространства, там помещается 16 банков по 4 кб.
		ConfBKModel_t       m_ConfBKModel;

		CBkSound           *m_pSound;           // указатель на модуль звуковой подсистемы
		CSpeaker           *m_pSpeaker;         // указатель на объект пищалка
		CCovox             *m_pCovox;           // указатель на объект ковокс
		CMenestrel         *m_pMenestrel;       // указатель на объект Менестрель
		CEMU2149           *m_pAY8910;          // указатель на объект сопроцессор Ay8910-3
		CDebugger          *m_pDebugger;        // указатель на отладчик

		volatile bool       m_bBreaked;         // флаг состояния - процессор приостановлен для отладки
		volatile bool       m_bRunning;         // флаг состояния - процессор работает/стоит

		void                MediaTick();

		// поток с точным таймером
		void                TimerThreadFunc();  // собственно функция
		std::thread         m_TimerThread;
		volatile bool       m_bKillTimerEvent;  // флаг для остановки потока.

		void                Make_One_Screen_Cycle();

		// Инициализация памяти
		virtual bool        InitMemoryModules();
		virtual void        InitMemoryValues(int nMemSize);
		virtual void        MemoryManager();
		bool                LoadRomModule(int iniRomNameIndex, int bank);   // загрузка нужного модуля ПЗУ по заданному адресу

		// Методы для загрузки и сохранения состояния эмулятора
		virtual bool        RestoreRegisters(CMSFManager &msf);
		virtual bool        RestoreMemory(CMSFManager &msf);
		virtual bool        RestoreMemoryMap(CMSFManager &msf);
        virtual bool        RestorePreview(CMSFManager &msf, QImage *hScreenshot);
		virtual bool        RestoreConfig(CMSFManager &msf);


		// Методы, эмулирующие поведение регистров
		virtual bool        OnSetSystemRegister(uint16_t addr, uint16_t src, bool bByteOperation = false);
		virtual bool        OnGetSystemRegister(uint16_t addr, void *pDst, bool bByteOperation = false);

		virtual bool        Interception(); // Вызывается после каждой команды, для перехвата функций

		bool                EmulateLoadTape();
		bool                EmulateSaveTape();

		virtual int         GetScreenPage(); // получение номера страницы экрана
		int                 CalcStep();

	public:
		CMotherBoard(BK_DEV_MPI model = BK_DEV_MPI::BK0010);
        virtual ~CMotherBoard() override;

		virtual MSF_CONF    GetConfiguration();
		BK_DEV_MPI         GetBoardModel();

		void                AttachWindow(CMainFrame *pParent);
		void                AttachSound(CBkSound *pSnd);
		void                AttachSpeaker(CSpeaker *pDevice);
		void                AttachCovox(CCovox *pDevice);
		void                AttachMenestrel(CMenestrel *pDevice);
		void                AttachAY8910(CEMU2149 *pDevice);
		void                AttachDebugger(CDebugger *pDevice);

		void                SetMTC(int mtc);

		void                StopTimerThread();
		bool                StartTimerThread();

        virtual bool        RestoreState(CMSFManager &msf, QImage *hScreenshot);

		// Виртуальные методы, вызываемые после команды reset
		virtual void        OnReset() override;

		// Методы Set/Get byte/word
		virtual uint8_t     GetByteT(uint16_t addr, int &nTC);
		virtual uint16_t    GetWordT(uint16_t addr, int &nTC);
		virtual void        SetByteT(uint16_t addr, uint8_t value, int &nTC);
		virtual void        SetWordT(uint16_t addr, uint16_t value, int &nTC);

		virtual uint8_t     GetByte(uint16_t addr);
		virtual uint16_t    GetWord(uint16_t addr);
		virtual void        GetByte(uint16_t addr, uint8_t *pValue) override;
		virtual void        GetWord(uint16_t addr, uint16_t *pValue) override;
		virtual void        SetByte(uint16_t addr, uint8_t value) override;
		virtual void        SetWord(uint16_t addr, uint16_t value) override;

		virtual uint8_t     GetByteIndirect(uint16_t addr);
		virtual uint16_t    GetWordIndirect(uint16_t addr);
		virtual void        SetByteIndirect(uint16_t addr, uint8_t value);
		virtual void        SetWordIndirect(uint16_t addr, uint16_t value);

		uint16_t            GetRON(CCPU::REGISTER reg);
		void                SetRON(CCPU::REGISTER reg, uint16_t value);
		inline uint16_t     GetPSW()
		{
			return m_cpu.GetPSW();
		}
		inline void         SetPSW(register uint16_t value)
		{
			m_cpu.SetPSW(value);
		}

		inline bool         GetPSWBit(PSW_BIT pos)
		{
			return m_cpu.GetPSWBit(pos);
		}
		inline void         SetPSWBit(PSW_BIT pos, bool val)
		{
			m_cpu.SetPSWBit(pos, val);
		}

		virtual void        StopInterrupt();
		virtual void        UnStopInterrupt();
		void                KeyboardInterrupt(uint16_t interrupt);

		void                FrameParam();

		void                ResetDevices();
		bool                InitBoard(uint16_t addrStart); // Эта функция используется единственный раз при начальной инициализации модели.
		void                ResetCold(uint16_t addrMask);
		void                RunInto();
		void                RunOver();
		void                RunOut();
		void                RunToAddr(uint16_t addr);
		void                RunCPU(bool bUnbreak = true); // запуск. по умолчанию сбрасывается отладочный приостанов
		void                StopCPU(bool bUnbreak = true); // остановка. по умолчанию сбрасывается отладочный приостанов
		inline bool         IsCPURun();
		void                BreakCPU();
		void                UnbreakCPU(int nGoto);
		inline bool         IsCPUBreaked();

		void                AccelerateCPU();
		void                SlowdownCPU();
		void                NormalizeCPU();
		bool                CanAccelerate();
		bool                CanSlowDown();
		int                 GetLowBound();
		int                 GetHighBound();
		void                SetCPUBaseFreq(int frq); // установка базовой частоты
		void                SetCPUFreq(int frq); // установка текущей частоты
		int                 GetCPUFreq(); // выдача текущей частоты
		int                 GetCPUSpeed();   // выдача текущей частоты для конфигурации

		// Передача данных экрану
		virtual void        ChangePalette() {}

		// Приём/передача данных fdd контроллеру
		CFDDController     *GetFDD();
		virtual void        SetFDDType(BK_DEV_MPI model, bool bInit = true);
		BK_DEV_MPI         GetFDDType();
		uint16_t            GetAltProMode();
		void                SetAltProMode(uint16_t w);
		uint16_t            GetAltProCode();
		void                SetAltProCode(uint16_t w);
		uint16_t            GetAltProExtCode();
		// прочие функции
        virtual void        Set177716RegMem(uint16_t w) { (void)w; }
		virtual void        Set177716RegTap(uint16_t w);

		// функции для карты памяти
		virtual uint8_t    *GetMainMemory();
		virtual uint8_t    *GetAddMemory();

		void                DrawDebugScreen();
};

/*
Организация массива памяти (делается один большой массив)
32кб: ОЗУ
000000--банк 0 ---страница 0 --- 0
010000--банк 1
020000--банк 2
030000--банк 3
040000--банк 4 ---страница 1 --- 16384
050000--банк 5
060000--банк 6
070000--банк 7
32кб: ПЗУ
100000--банк 8 ---страница 2 --- 32768 - монитор MONIT10.ROM
110000--банк 9
120000--банк 10------------------------- бейсик Basic10_1.rom
130000--банк 11
140000--банк 12---страница 3 --- 49152 - бейсик Basic10_2.rom
150000--банк 13
160000--банк 14------------------------- бейсик и регистры
170000--банк 15
*/
