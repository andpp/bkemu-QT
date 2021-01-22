/*  This file is part of BKBTL.
    BKBTL is free software: you can redistribute it and/or modify it under the terms
of the GNU Lesser General Public License as published by the Free Software Foundation,
either version 3 of the License, or (at your option) any later version.
    BKBTL is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public License along with
BKBTL. If not, see <http://www.gnu.org/licenses/>. */


// FDDController.h: interface for the CFDDController class.
//


#pragma once

#include "Device.h"
#include "HDD.h"
#include "Config.h"

#include <mutex>

#define A16M_ROM_10 16
#define A16M_A0_10 18
#define A16M_A2_10 20

#define A16M_ROM_11M 50
#define A16M_A0_11M 52
#define A16M_A2_11M 54

//////////////////////////////////////////////////////////////////////
// CFloppy

#define FLOPPY_FSM_IDLE         0

#define FLOPPY_CMD_SELECT_DRIVE_A       01      // выбор накопителя 0
#define FLOPPY_CMD_SELECT_DRIVE_B       02      // выбор накопителя 1
#define FLOPPY_CMD_SELECT_DRIVE_C       04      // выбор накопителя 2
#define FLOPPY_CMD_SELECT_DRIVE_D       010     // выбор накопителя 3
#define FLOPPY_CMD_ENGINESTART          020     // включение электродвигателя
#define FLOPPY_CMD_SIDEUP               040     // выбор стороны, 0 - верхняя
#define FLOPPY_CMD_DIR                  0100    // направление шага
#define FLOPPY_CMD_STEP                 0200    // шаг
#define FLOPPY_CMD_READDATA             0400    // признак "начало чтения"
#define FLOPPY_CMD_WRITEMARKER          01000   // признак "запись маркера"
#define FLOPPY_CMD_PRECORRECTION        02000   // включение схемы прекоррекции

// dir == 0 to center (towards trk0)
// dir == 1 from center (towards trk80)

#define FLOPPY_STATUS_TRACK0            01      // признак "дорожка 0"
#define FLOPPY_STATUS_RDY               02      // признак готовности накопителя
#define FLOPPY_STATUS_WRITEPROTECT      04      // запрет записи
#define FLOPPY_STATUS_MOREDATA          0200    // признак, что данные готовы или требование новых данных
#define FLOPPY_STATUS_CHECKSUMOK        040000  // для чтения - признак записи ЦКС
#define FLOPPY_STATUS_INDEXMARK         0100000 // датчик индексного отверстия

#define FLOPPY_RAWTRACKSIZE             6250
#define FLOPPY_RAWMARKERSIZE            (FLOPPY_RAWTRACKSIZE / 2)
#define FLOPPY_INDEXLENGTH              32

#define ALTPRO_A16M_START_MODE 0160
#define ALTPRO_A16M_STD10_MODE 060
#define ALTPRO_A16M_OZU10_MODE 0120
#define ALTPRO_A16M_BASIC_MODE 020
#define ALTPRO_A16M_STD11_MODE 0140
#define ALTPRO_A16M_OZU11_MODE 040
#define ALTPRO_A16M_OZUZZ_MODE 0100
#define ALTPRO_A16M_HLT11_MODE 0

#define ALTPRO_SMK_SYS_MODE   0160
#define ALTPRO_SMK_STD10_MODE 060
#define ALTPRO_SMK_OZU10_MODE 0120
#define ALTPRO_SMK_ALL_MODE   020
#define ALTPRO_SMK_STD11_MODE 0140
#define ALTPRO_SMK_OZU11_MODE 040
#define ALTPRO_SMK_HLT10_MODE 0100
#define ALTPRO_SMK_HLT11_MODE 0

#define ALTPRO_BEGIN_STROB_TRIGGER 6
#define ALTPRO_MODE_MASK 0160
#define ALTPRO_CODE_MASK 02015

struct CFloppyDrive
{
		CFile fFile;
		CString strFileName;                // имя файла образа, который примонтирован в привод, чтобы повторно не перепримонтировывать
		bool bPresent;                      // Флаг физического присутствия привода, для разных контроллеров
		bool okReadOnly;                    // Флаг защиты от записи
		int datatrack;                      // Номер трека данных в массиве data
		int dataside;                       // Сторона диска данных в массиве data
		int dataptr;                        // Смещение данных внутри data - позиция заголовка
		uint8_t data[FLOPPY_RAWTRACKSIZE];     // Raw track image for the current track
		uint8_t marker[FLOPPY_RAWMARKERSIZE];  // Позиции маркеров

	public:
		CFloppyDrive();
		void Reset();
};

class CMotherBoard;

class CFDDController : public CDevice
{
#ifdef _DEBUG
// дебажные переменные, нужные для подсчёта и вывода в отладочную консоль число оборотов в секунду.
// правда результат получается какой-то абстрактный
		DWORD m_msec; // счётчики миллисекунд, прошедшие с момента старта системы
		int m_nFrame;
#endif
		enum
		{
			FDD_NOERROR = 0,
			FDD_DISK_PROTECTED,         // 1
			FDD_SECT_HEADER_ERROR,      // 2
			FDD_0_TRACK_ERROR,          // 3
			FDD_POSITIONING_ERROR,      // 4
			FDD_SECTOR_NOT_FOUND,       // 5
			FDD_NO_DISK,                // 6
			FDD_STOP,                   // 7
			FDD_ADDR_MARKER_NOT_FOUND,  // 010
			FDD_DATA_MARKER_NOT_FOUND,  // 011
			FDD_BAD_FORMAT              // 012
		};


#pragma pack(push)
#pragma pack(1)
		struct TABLE_EMFDD
		{
			uint16_t    CSRW;               // 00 копия по записи регистра состояния КНГМД
			uint16_t    CURTRK;             // 02 адрес текущей дорожки (адрес одного из последующих байтов)
			uint8_t     TRKTAB[4];          // 04 таблица текущих дорожек
			uint16_t    TDOWN;              // 10 задержка опускания головки
			uint16_t    TSTEP;              // 12 задержка перехода с дорожки на дорожку
			uint8_t     TRKCOR;             // 14 дорожка начала предкомпенсации
			uint8_t     BRETRY;             // 15 число попыток повтора при ошибке
			uint8_t     FLAGS;              // 16 рабочая ячейка драйвера
			uint8_t     FILLB;              // 17 код заполнения при форматировании
			uint16_t    FLGPTR;             // 20 указатель на байт признаков (один из следующих байтов)
			uint8_t     FLGTAB[4];          // 22 байты признаков
			uint16_t    ADDR;               // 26 адрес начала массива данных в ОЗУ (обязательно чётный)
			int16_t     WCNT;               // 30 количество слов для пересылки
			uint8_t     SIDE;               // 32 номер стороны диска
			uint8_t     TRK;                // 33 номер дорожки
			uint8_t     UNIT;               // 34 номер привода
			uint8_t     SECTOR;             // 35 номер сектора
			uint16_t    WRTVAR;             // 36 значение, записываемое при форматировании
			uint16_t    MARKER;             // 40 буфер маркера при записи
			uint16_t    FREE;               // 42 длина пустого остатка сектора
			uint16_t    INTIME;             // 44 счётчик длительности индекса
			uint16_t    BUF4;               // 46 буфер для сохранения вектора 4
			uint16_t    BUFSP;              // 50 буфер для сохранения SP
			uint16_t    BUFPSW;             // 52 буфер для сохранения PSW
			uint8_t     CRETRY;             // 54 счётчик повторов при ошибке
			uint8_t     TURNS;              // 55 число оборотов диска при поиске сектора
			uint8_t     SECRET;             // 56 число повторных попыток поиска сектора
			uint8_t     ERRNUM;             // 57 буфер для номера ошибки
			uint16_t    MAXSEC;             // 60 число секторов на дорожке
			uint16_t    HOLTIN;             // 62 время задержки после индекса до первого сектора
			uint16_t    SECLEN;             // 64 длина сектора в словах (следить за значением при записи, иначе портится разметка диска)
		};
#pragma pack(pop)

	protected:
		CFloppyDrive    m_drivedata[static_cast<int>(FDD_DRIVE::NUM_FDD)];

		FDD_DRIVE       m_drive;       // Номер привода: от 0 до 3; -1 если не выбран
		CFloppyDrive   *m_pDrive;      // Текущий привод; nullptr если не выбран
		int             m_track;       // Номер дорожки: от 0 до 79 (в реальных дисководах возможно до 81, в особых случаях даже до 83)
		int             m_side;        // Сторона диска: 0 или 1
		uint32_t        m_status;      // См. определения FLOPPY_STATUS_XXX (флаги, принимаемые от контроллера дисковода)
		uint32_t        m_flags;       // См. определения FLOPPY_CMD_XXX (флаги, передаваемые контроллеру дисковода)
		uint16_t        m_datareg;     // Регистр данных режима чтения
		uint16_t        m_writereg;    // Регистр данных режима записи
		bool            m_writeflag;   // В регистре данных режима записи есть данные
		bool            m_writemarker; // Запись маркера в m_marker
		uint16_t        m_shiftreg;    // Регистр сдвига режима записи
		bool            m_shiftflag;   // В регистре сдвига режима записи есть данные
		bool            m_shiftmarker; // Запись маркера в m_marker
		bool            m_bWriteMode;  // true = режим записи, false = режим чтения
		bool            m_bSearchSync; // Подрежим чтения: true = поиск синхро последовательности, false = просто чтение
		bool            m_bSearchSyncTrigger; // триггер для отлова перехода из 1 в 0 бита FLOPPY_CMD_READDATA
		bool            m_bCRCCalc;     // true = CRC в процессе подсчёта
		bool            m_bTrackChanged;    // true = m_data было изменено - надо сохранить его в файл

		BK_DEV_MPI     m_FDDModel;
		bool            m_bA16M_Trigger;
		uint16_t        m_nAltproMode; // код режима работы контроллера Альтпро
		uint16_t        m_nAltproMemBank; // номер банка памяти контроллера Альтпро

		CATA_IDE        m_ATA_IDE;
		std::mutex      m_mutPeriodicBusy;

	public:
		CFDDController();
		virtual ~CFDDController() override;

		// Virtual method called after reset command
		virtual void        OnReset() override;
		void                InitVariables();
		// Methods for Set/Get byte/word
		virtual void        GetByte(uint16_t addr, uint8_t *pValue) override;
		virtual void        GetWord(uint16_t addr, uint16_t *pValue) override;
		virtual void        SetByte(uint16_t addr, uint8_t value) override;
		virtual void        SetWord(uint16_t addr, uint16_t value) override;
		bool                AttachImage(FDD_DRIVE eDrive, CString &sFileName);
		void                DetachImage(FDD_DRIVE eDrive);
		void                ReadDrivesPath();
		void                DetachDrives();
		void                EmulateFDD(CMotherBoard *pBoard);
		bool                IsAttached(FDD_DRIVE eDrive);
		bool                IsReadOnly(FDD_DRIVE eDrive);
		bool                GetDriveState(FDD_DRIVE eDrive);
		void                init_A16M_10(ConfBKModel_t *mmodl, const int v);
		void                init_A16M_11M(ConfBKModel_t *mmodl, const int v);
		bool                Change_AltPro_Mode(ConfBKModel_t *mmodl, const uint16_t w);
		void                A16M_MemManager_10(BKMEMBank_t *mmap, ConfBKModel_t *mmodl);
		void                A16M_MemManager_11M(BKMEMBank_t *mmap, ConfBKModel_t *mmodl);
		void                SMK512_MemManager_10(BKMEMBank_t *mmap, ConfBKModel_t *mmodl);
		void                SMK512_MemManager_11M(BKMEMBank_t *mmap, ConfBKModel_t *mmodl);

		void                SetFDDType(BK_DEV_MPI model);

		BK_DEV_MPI        GetFDDType();
		void                InitHDD();
		bool                WriteHDDRegisters(uint16_t num, uint16_t data);     // запись в регистры HDD
		bool                ReadHDDRegisters(uint16_t num, uint16_t &data);     // чтение из регистров HDD
		uint16_t            ReadDebugHDDRegisters(int nDrive, HDD_REGISTER num, bool bReadMode);   // отладочное чтение из регистров HDD

		bool                IsEngineOn();
		uint16_t            GetData();              // Чтение порта 177132 - данные
		uint16_t            GetState();             // Чтение порта 177130 - состояние устройства
		void                WriteData(uint16_t Data);   // Запись в порт 177132 - данные
		void                SetCommand(uint16_t cmd);   // Запись в порт 177130 - команды

		uint16_t            GetDataDebug();          // Получить значение порта 177132 - данные для отладки
		uint16_t            GetStateDebug();         // Получить значение порта 177130 - состояние устройства для отладки
		uint16_t            GetWriteDataDebug();     // Получить значение порта 177132 - переданные данные для отладки
		uint16_t            GetCmdDebug();    // Получить значение порта 177130 - переданные команды для отладки

		void                Periodic(); // Вращение диска; вызывается каждые 64 мкс - 15625 раз в секунду. Вызывается из основного цикла

	private:
		inline void         A16M_SetMemBank(BKMEMBank_t *mmap, int nBnk, int BnkNum);
		inline void         A16M_SetMemSegment(BKMEMBank_t *mmap, int nBnk, int BnkNum);
		inline void         A16M_SetRomBank(BKMEMBank_t *mmap, int nBnk, int BnkNum);

		void                PrepareTrack();
		void                FlushChanges();  // Если текущая дорожка была изменена, сохраним её
		static void         EncodeTrackData(const register uint8_t *pSrc, register uint8_t *data, register uint8_t *marker, register uint16_t track, register uint16_t side);
		static bool         DecodeTrackData(const register uint8_t *pRaw, register uint8_t *pDest);

};


