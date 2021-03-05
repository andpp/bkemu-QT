// Board_11M.cpp: implementation of the CMotherBoard_11M class.
//


#include "pch.h"
#include "Board_11M.h"
#include "BKMessageBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction


CMotherBoard_11M::CMotherBoard_11M(BK_DEV_MPI model)
	: CMotherBoard(model)
	, m_bBlockStop(false)
{
	m_nStartAddr = 0140000;
	m_nBKPortsIOArea = BK_PURE_PORTSIO_AREA;
	m_fdd.init_A16M_11M(&m_ConfBKModel, ALTPRO_A16M_STD11_MODE);
	SetCPUBaseFreq(CPU_SPEED_BK11); // частота задаётся этой константой
	SAFE_DELETE_ARRAY(m_pMemory);
	// пересоздадим массив памяти.
	m_pMemory = new uint8_t[0700000];

	if (!m_pMemory)
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

	InitMemoryValues(0700000);
	m_reg177716out_mem = 0;
}

CMotherBoard_11M::~CMotherBoard_11M()
{}

MSF_CONF CMotherBoard_11M::GetConfiguration()
{
	return MSF_CONF::BK11M;
}


void CMotherBoard_11M::SetBlockStop(bool status)
{
	m_bBlockStop = status;
}

bool CMotherBoard_11M::GetBlockStop()
{
	return m_bBlockStop;
}

void CMotherBoard_11M::StopInterrupt()
{
	// нажали на кнопку стоп
	if (!m_bBlockStop) // если кнопка СТОП не заблокирована
	{
		m_cpu.SetIRQ1();
	}
}

void CMotherBoard_11M::UnStopInterrupt()
{
	// отжали кнопку стоп
	if (!m_bBlockStop) // если кнопка СТОП не заблокирована
	{
		m_cpu.UnsetIRQ1();
	}
}


int CMotherBoard_11M::GetScreenPage()
{
	return ((m_reg177662out & 0100000) ? 6 : 5);
}

void CMotherBoard_11M::OnReset()
{
	CMotherBoard::OnReset();
	ChangePalette(); // эта функция используется и при начальной инициализации, и при выполнении
	// команды RESET, что не совсем правильно. при RESET регистр палитр не затрагивается.
}


void CMotherBoard_11M::Set177716RegMem(uint16_t w)
{
	register uint16_t mask = 077433;
	m_reg177716out_mem = (m_reg177716out_mem & ~mask) | (w & mask);
	MemoryManager();
}

void CMotherBoard_11M::Set177716RegTap(uint16_t w)
{
	register uint16_t mask = 010344;
	m_reg177716out_tap = (m_reg177716out_tap & ~mask) | (w & mask);
	m_pSpeaker->SetData(m_reg177716out_tap);
	SetBlockStop(!!(m_reg177716out_tap & 010000)); // управляем блокировкой кнопки стоп.
}

/*
Запись данных в системные регистры БК0011М
    вход: num - адрес регистра (177660, 177716 и т.п.)
          src - записываемое значение.
          bByteOperation - флаг операции true - байтовая, false - словная
*/
bool CMotherBoard_11M::OnSetSystemRegister(uint16_t addr, uint16_t src, bool bByteOperation)
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
			(0100000)бит 15: переключение буферов экрана 0 - №5, 1 - №6
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
			(004)бит 2: данные на магнитофон. Начальное состояние "0".
			(040)бит 5: данные на магнитофон. Начальное состояние "0".
			(100)бит 6: данные на магнитофон и на пьезодинамик. Начальное состояние "0".
			(200)бит 7: включение двигателя магнитофона, "1" -- стоп, "0" -- пуск. Начальное состояние "1".
			(010000)бит 12: разрешение прерывания по клавише СТОП, 0 - разрешено, 1 - запрещено
			    биты 0,1,3,4 не используются, "0".
			    бит 11==1
			(001)бит 0: подключение страницы №8 пзу в окно №1
			(002)бит 1: подключение страницы №9 пзу в окно №1
			(010)бит 3: подключение страницы №10 пзу в окно №1
			(020)бит 4: подключение страницы №11 пзу в окно №1
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

			if (src & 04000)
			{
				Set177716RegMem(src);
			}
			else
			{
				Set177716RegTap(src);
			}

			// В БК 2й разряд SEL1 фиксирует любую запись в этот регистр, взводя триггер D9.1 на неограниченное время, сбрасывается который любым чтением этого регистра.
			m_reg177716in |= 4;
			return true;
	}

	return false;
}

bool CMotherBoard_11M::LoadRomModule11(int iniRomNameIndex, int bank)
{
	CString strName = g_Config.GetIniObj()->GetValueString(IDS_INI_SECTIONNAME_ROMMODULES, iniRomNameIndex, g_strEmptyUnit);
	// здесь делается жёсткая зависимость от номера банка. При смене структуры данных тут тоже всё надо будет переделывать.
	int n = (bank - BRD_11_BASIC2_BNK) / 2; // номер бита в битовой маске

	if (strName == g_strEmptyUnit) // если там пусто
	{
		m_ConfBKModel.nROMPresent &= ~(1 << n); // сбрасываем бит
		return true; // Там ПЗУ не предусмотрено, но это не ошибка
	}
	else
	{
		m_ConfBKModel.nROMPresent |= (1 << n); // устанавливаем бит бит
	}

    CString strPath = QDir(g_Config.m_strROMPath).filePath(strName);
	CFile file;

	if (file.Open(strPath, CFile::modeRead))
	{
		register auto len = (UINT)file.GetLength();

		if (len > 020000) // размер ПЗУ не должен быть больше 8кб
		{
			len = 020000;
		}

		UINT readed = file.Read(&m_pMemory[bank << 12], len);
		file.Close();

		if (readed == len)
		{
			return true;
		}
	}
	else
	{
		CString strError;
		strError.LoadString(IDS_ERROR_CANTOPENFILE);
		g_BKMsgBox.Show(strError + _T('\'') + strPath + _T('\''), MB_OK | MB_ICONSTOP);
	}

	m_ConfBKModel.nROMPresent &= ~(1 << n); // сбрасываем бит, потому что не смогли правильно прочитать ПЗУ
	return false;
}

bool CMotherBoard_11M::InitMemoryModules()
{
	m_ConfBKModel.nROMPresent = 0;
	LoadRomModule11(IDS_INI_BK11M_RE2_328_BASIC2, BRD_11_BASIC2_BNK);
	LoadRomModule11(IDS_INI_BK11M_RE2_329_BASIC3, BRD_11_BASIC3_BNK);
	LoadRomModule11(IDS_INI_BK11M_RE2_327_BASIC1, BRD_11_BASIC1_BNK);
	LoadRomModule11(IDS_INI_BK11M_RE2_325_EXT, BRD_11_EXT_BNK);
	bool bBos = LoadRomModule(IDS_INI_BK11M_RE2_324_BOS, BRD_11_BOS_BNK);
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
	bool bTst = LoadRomModule(IDS_INI_BK11M_RE2_330_MSTD, BRD_11_TST_BNK);

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

void CMotherBoard_11M::InitMemoryValues(int nMemSize)
{
	uint16_t val = 0;
	auto pPtr = reinterpret_cast<uint16_t *>(m_pMemory);
	int n = 8;
	int flag = 8;

	for (int i = 0; i < nMemSize / 2; ++i)
	{
		pPtr[i] = val;

		if (--n <= 0)
		{
			n = 8;

			if (--flag > 0)
			{
				val = ~val;
			}
			else
			{
				flag = 8;
			}
		}
	}
}

void CMotherBoard_11M::MemoryManager()
{
	static const int PageNums[8]    = {1, 5, 2, 3, 4, 7, 0, 6}; // перекодировка БКшной кодировки номеров страниц в нормальную.
	register uint16_t mem = m_reg177716out_mem;
	register int nRAMPageWnd0 = (mem >> 12) & 7; // номер страницы ОЗУ в окне 0
	register int nRAMPageWnd1 = (mem >> 8) & 7; // номер страницы ОЗУ в окне 1
	register int nROMinWnd1 = 0; // номер страницы ПЗУ в окне 1, если 0 - нет там ПЗУ, там ОЗУ

	// стр 8 пзу
	if (mem & 1)
	{
		nROMinWnd1 = 8;
	}
	// стр 9 пзу
	else if (mem & 2)
	{
		nROMinWnd1 = 9;
	}
	// стр 10 пзу
	else if (mem & 010)
	{
		nROMinWnd1 = 10;
	}
	// стр 11 пзу
	else if (mem & 020)
	{
		nROMinWnd1 = 11;
	}

	// теперь надо в соответствии с этим поправить карту памяти
	// сперва разберёмся с окном 0, там может быть только ОЗУ, в том числе и экран
	for (int i = 04, j = 0; i <= 07; ++i, ++j)
	{
		m_MemoryMap[i].nBank = (PageNums[nRAMPageWnd0] << 2) + j;
		m_MemoryMap[i].nOffset = m_MemoryMap[i].nBank << 12;
		m_MemoryMap[i].nTimingCorrection = RAM_TIMING_CORR_VALUE_D;
	}

	// теперь разберёмся с окном 1, там может быть как пзу, так и ОЗУ, а так же может быть пусто, для страниц 10 и 11
	if (nROMinWnd1)
	{
		int n = (nROMinWnd1 - 8) << 1;

		for (int i = 010, j = 0; i <= 013; ++i, ++j)
		{
			m_MemoryMap[i].bReadable = !!(m_ConfBKModel.nROMPresent & (1 << (n + (j >> 1))));
			m_MemoryMap[i].bWritable = false;
			m_MemoryMap[i].nBank = (nROMinWnd1 << 2) + j;
			m_MemoryMap[i].nOffset = m_MemoryMap[i].nBank << 12;
			m_MemoryMap[i].nTimingCorrection = ROM_TIMING_CORR_VALUE;
		}
	}
	else
	{
		for (int i = 010, j = 0; i <= 013; ++i, ++j)
		{
			m_MemoryMap[i].bReadable = true;
			m_MemoryMap[i].bWritable = true;
			m_MemoryMap[i].nBank = (PageNums[nRAMPageWnd1] << 2) + j;
			m_MemoryMap[i].nOffset = m_MemoryMap[i].nBank << 12;
			m_MemoryMap[i].nTimingCorrection = RAM_TIMING_CORR_VALUE_D;
		}
	}
}

void CMotherBoard_11M::ChangePalette()
{
	if (m_pParent)
	{
		m_pParent->GetScreen()->SetPalette(m_reg177662out >> 8);
	}
}

// тут возможно надо будет сделать перехват магнитофона
bool CMotherBoard_11M::Interception()
{
	if (CMotherBoard::Interception())
	{
		return true;
	}

	return false;
}

bool CMotherBoard_11M::RestoreState(CMSFManager &msf, QImage *hScreenshot)
{
	if (RestorePreview(msf, hScreenshot))
	{
		if (RestoreConfig(msf))
		{
			if (RestoreRegisters(msf))
			{
				if (RestoreMemoryMap(msf))
				{
					if (RestoreMemory(msf))
					{
						if (msf.IsLoad())
						{
							ChangePalette();
						}

						return true;
					}
				}
			}
		}
	}

	return false;
}

bool CMotherBoard_11M::RestoreMemory(CMSFManager &msf)
{
	if (msf.IsLoad())
	{
		m_fdd.SetFDDType(g_Config.m_BKFDDModel);

		if (msf.GetBlockBaseMemory11M(m_pMemory))
		{
			return true;
		}
	}
	else
	{
		if (msf.SetBlockBaseMemory11M(m_pMemory))
		{
			return true;
		}
	}

	return false;
}

