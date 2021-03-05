// Board_11.cpp: implementation of the CMotherBoard_11 class.
//


#include "pch.h"
#include "Board_11.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction


CMotherBoard_11::CMotherBoard_11(BK_DEV_MPI model)
	: CMotherBoard_11M(model)
{
}

CMotherBoard_11::~CMotherBoard_11()
{}


MSF_CONF CMotherBoard_11::GetConfiguration()
{
	return MSF_CONF::BK11;
}

void CMotherBoard_11::Set177716RegMem(uint16_t w)
{
	register uint16_t mask = 077437;
	m_reg177716out_mem = (m_reg177716out_mem & ~mask) | (w & mask);
	MemoryManager();
}

void CMotherBoard_11::Set177716RegTap(uint16_t w)
{
	register uint16_t mask = 0340;
	m_reg177716out_tap = (m_reg177716out_tap & ~mask) | (w & mask);
	m_pSpeaker->SetData(m_reg177716out_tap);
}

int CMotherBoard_11::GetScreenPage()
{
	return ((m_reg177716out_mem & 04) ? 6 : 5);
}

/*
Запись данных в системные регистры БК0011
    вход: num - адрес регистра (177660, 177716 и т.п.)
          src - записываемое значение.
          bByteOperation - флаг операции true - байтовая, false - словная
*/
bool CMotherBoard_11::OnSetSystemRegister(uint16_t addr, uint16_t src, bool bByteOperation)
{
	uint16_t mask = 0177777; // это какие биты в регистре используются

	switch (addr & 0177776)
	{
		case 0177660:
			/*
			177660
			Регистр состояния клавиатуры.
			(0100)бит 6 -- маскирование прерывание от клавиатуры,
			    "0" -- разрешено прерывание по вектору 060 либо 0274.
			    Прерывание вызывается при появлении "1" в бите 7. Начальное состояние: "0".
			    Доступен по чтению и по записи
			(0200)бит 7 -- готовность: "1" -- в регистре данных клавиатуры (177662) готов код нажатой клавиши.
			    Устанавливается при нажатии на клавишу, сбрасывается при чтении регистра данных клавиатуры.
			    Начальное состояние: "1".
			    Доступен только по чтению.
			другие биты: "0". Доступны только по чтению.
			*/
			mask = 0100;

			if (bByteOperation)
			{
				src &= 0377;

				if (addr & 1)
				{
					src = (src << 8) | (m_reg177660 & 0377); // работаем со старшим байтом, младший оставляем неизменным
				}
				else
				{
					src = src | (m_reg177660 & 0177400); // работаем с младшим байтом, старший оставляем неизменным
				}
			}

			// сбрасываем используемые биты и устанавливаем их значения из слова, которое записываем.
			// остальные биты - которые не используются - остаются нетронутыми.
			m_reg177660 = (m_reg177660 & ~mask) | (src & mask);
			return true;

		case 0177662:
			mask = 0147400;

			/*
			177662
			Регистр данных клавиатуры.
			    биты 0-6: код клавиши. Доступ только по чтению.
			биты 08-11: палитра
			(040000)бит 14: разрешение прерывания по (внешнему) таймеру (50 Гц),
			    "0" -- прерывание разрешено,
			    "1" -- таймер отключён.
			    Доступен только по записи.
			(0100000)бит 15: разрешение прерывания по клавише СТОП, 0 - разрешено, 1 - запрещено
			*/
			if (bByteOperation)
			{
				src &= 0377; // работаем с младшим байтом

				if (addr & 1)
				{
					src <<= 8; // работаем со старшим байтом
				}
			}

			// сбрасываем используемые биты и устанавливаем их значения из слова, которое записываем.
			// остальные биты - которые не используются - остаются нетронутыми.
			m_reg177662out = (m_reg177662out & ~mask) | (src & mask);
			ChangePalette();
			SetBlockStop(!!(m_reg177662out & 0100000)); // управляем блокировкой кнопки стоп.
			return true;

		case 0177664:
			/*
			177664
			Регистр скроллинга. Доступен по записи и чтению.
			    биты 0-7: смещение скроллинга, строк. Начальное значение -- 0330.
			(01000)бит 9: сокращённый режим экрана, "0" -- сокращённый (1/4 экрана, старшие адреса),
			    "1" -- полный экран 256 строк.
			*/
			mask = 01377;

			if (bByteOperation)
			{
				src &= 0377;
				/*
				оказалось, что в этот регистр не работает байтовая запись.
				чтение байта работает, но записывается всегда слово.
				а при байтовой операции, старший байт просто теряется.
				*/
			}

			m_reg177664 = (m_reg177664 & ~mask) | (src & mask);
			return true;

		case 0177714:

			/*177714
			    Регистр параллельного программируемого порта ввода вывода - два регистра, входной по чтению и выходной по записи.
			    из выходного нельзя ничего прочитать т.к., читается оно из входного,
			    во входной невозможно ничего записать, т.к. записывается оно в выходной.
			*/
			if (bByteOperation)
			{
				src &= 0377; // работаем с младшим байтом

				if (addr & 1)
				{
					src <<= 8; // работаем со старшим байтом
				}
			}

			if (m_pAY8910)
			{
				if (bByteOperation)
				{
					m_pAY8910->synth_write_data(LOBYTE(src));
				}
				else
				{
					m_pAY8910->synth_write_address(src);
				}
			}

			if (m_pCovox)
			{
				m_pCovox->SetData(src);
			}

			if (m_pMenestrel)
			{
				m_pMenestrel->SetData(src);
			}

			m_reg177714out = src;

			if (g_Config.m_bICLBlock) // если включён блок нагрузок
			{
				// данные записанные в выходной порт передаются во входной
				m_reg177714in = src;
			}
			else if (g_Config.m_bMouseMars)
			{
				m_pParent->GetScreen()->SetMouseStrobe(src);
				m_reg177714in = m_pParent->GetScreen()->GetMouseStatus();
			}

			return true;

		case 0177716:

			/*
			177716
			Системный регистр. Внешний регистр 1 (ВР1, SEL1) процессора ВМ1, регистр начального пуска.
			как и 177714 состоит из двух регистров, раздельных по чтению и по записи
			По чтению:
			(004)бит 2: признак записи в системный регистр. Устанавливается в "1" при
			любой записи в регистр, сбрасывается в "0" по окончании операции чтения из регистра.
			(040)бит 5: данные с магнитофона, "0" - логический 0, "1" - логическая 1
			(100)бит 6: индикатор нажатия клавиши, установлен в "0" если нажата клавиша клавиатуры, "1" если нет нажатых клавиш.
			(200)бит 7: константа "1", указывающая на отсутствие в системе команд расширенной арифметики
			биты 8-15: адрес начального пуска, 140000 (БК-0011), младший байт при этом игнорируется
			остальные биты не используются, "0".
			По записи:
			    бит 11==0
			полностью игнорируется
			    бит 11==1
			(001)бит 0: подключение страницы №8 пзу в окно №1
			(002)бит 1: подключение страницы №9 пзу в окно №1
			(004)бит 2: переключение буферов экрана 0 - №5, 1 - №6
			(010)бит 3: подключение страницы №10 пзу в окно №1
			(020)бит 4: подключение страницы №11 пзу в окно №1
			(040)бит 5: данные на магнитофон. Начальное состояние "0".
			(100)бит 6: данные на магнитофон и на пьезодинамик. Начальное состояние "0".
			(200)бит 7: включение двигателя магнитофона, "1" -- стоп, "0" -- пуск. Начальное состояние "1".
			    биты 8-10: номер страницы озу в окно №1
			    биты 12-14: номер страницы озу в окно №0
			*/
			if (bByteOperation)
			{
				src &= 0377; // работаем с младшим байтом

				if (addr & 1)
				{
					src <<= 8; // работаем со старшим байтом
				}
			}

			if (src & 04000) // запись в регистр производится только при бит11 == 1, иначе - просто игнорируется
			{
				Set177716RegMem(src);
				Set177716RegTap(src);
			}

			// В БК 2й разряд SEL1 фиксирует любую запись в этот регистр, взводя триггер D9.1 на неограниченное время, сбрасывается который любым чтением этого регистра.
			m_reg177716in |= 4;
			return true;
	}

	return false;
}


bool CMotherBoard_11::InitMemoryModules()
{
	m_ConfBKModel.nROMPresent = 0;
	LoadRomModule11(IDS_INI_BK11_RE2_199_BASIC2, BRD_11_BASIC2_BNK);
	LoadRomModule11(IDS_INI_BK11_RE2_200_BASIC3, BRD_11_BASIC3_BNK);
	LoadRomModule11(IDS_INI_BK11_RE2_198_BASIC1, BRD_11_BASIC1_BNK);
	LoadRomModule11(IDS_INI_BK11_RE2_202_EXT, BRD_11_EXT_BNK);
	bool bBos = LoadRomModule(IDS_INI_BK11_RE2_201_BOS, BRD_11_BOS_BNK);
	// Подгружаем опциональные страницы 012 и 013.
	LoadRomModule11(IDS_INI_BK11_RE_OPT_PG12_1, BRD_11_PG12_1_BNK);
	LoadRomModule11(IDS_INI_BK11_RE_OPT_PG12_2, BRD_11_PG12_2_BNK);
	LoadRomModule11(IDS_INI_BK11_RE_OPT_PG13_1, BRD_11_PG13_1_BNK);
	LoadRomModule11(IDS_INI_BK11_RE_OPT_PG13_2, BRD_11_PG13_2_BNK);

	// и проинициализируем карту памяти
	for (int i = 0; i < 020; ++i)
	{
		m_MemoryMap[i].bReadable = true;
		m_MemoryMap[i].bWritable = true;
		m_MemoryMap[i].nBank = i;
		m_MemoryMap[i].nOffset = i << 12;
		m_MemoryMap[i].nTimingCorrection = RAM_TIMING_CORR_VALUE_D;
	}

	// по адресу 140000 - основное ПЗУ БОС
	for (int i = 014, bnk = BRD_11_BOS_BNK; i <= 015; ++i, ++bnk)
	{
		m_MemoryMap[i].bReadable = bBos;
		m_MemoryMap[i].bWritable = false;
		m_MemoryMap[i].nBank = bnk;
		m_MemoryMap[i].nOffset = bnk << 12;
		m_MemoryMap[i].nTimingCorrection = ROM_TIMING_CORR_VALUE;
	}

	// по адресу 160000 - по умолчанию - модуль МСТД
	bool bTst = LoadRomModule(IDS_INI_BK11_RE2_203_MSTD, BRD_11_TST_BNK);

	for (int i = 016, bnk = BRD_11_TST_BNK; i <= 017; ++i, ++bnk)
	{
		m_MemoryMap[i].bReadable = bTst;
		m_MemoryMap[i].bWritable = false;
		m_MemoryMap[i].nBank = bnk;
		m_MemoryMap[i].nOffset = bnk << 12;
		m_MemoryMap[i].nTimingCorrection = ROM_TIMING_CORR_VALUE;
	}

	MemoryManager();
	return true;
}

