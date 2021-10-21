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
    = default;


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
bool CMotherBoard_11::SetSystemRegister(uint16_t addr, uint16_t src, bool bByteOperation)
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

bool CMotherBoard_11::Interception()
{
	if (CMotherBoard::Interception())
	{
		return true;
	}

	switch (GetRON(CCPU::REGISTER::PC) & 0177776)
	{
		case 0144264:
			return EmulateSaveTape11();

		case 0145006:
			return EmulateLoadTape11();
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

// !!! на БК11 алгоритм более сложен. для чтения/записи используется окно1, и туда по очереди подключаются
// нужные страницы, т.е. BK_EMT36BP11_USRPG как такового нету
// поэтому, чтобы не усложнять себе жизнь, сделаем всё сами.

constexpr auto BK_NAMELENGTH = 16;   // Максимальная длина имени файла на БК - 16 байтов
constexpr auto BK_BMB11 = 040134;
constexpr auto BK_BMB10_ADDRESS = 2;
constexpr auto BK_BMB10_LENGTH = 4;
constexpr auto BK_BMB10_NAME = 6;
constexpr auto BK_BMB11_PAGES = BK_BMB10_NAME + BK_NAMELENGTH;
constexpr auto BK_BMB11_FOUND_ADDRESS = BK_BMB11_PAGES + 2;
constexpr auto BK_BMB11_FOUND_LENGTH = BK_BMB11_FOUND_ADDRESS + 2;
constexpr auto BK_BMB11_FOUND_NAME = BK_BMB11_FOUND_LENGTH + 2;
constexpr auto BK_BMB11_ERRADDR = 052;
constexpr auto BK_BMB11_RNOFLAG = 040213;
constexpr auto BK_BMB11_FICTFLAG = 040214;
constexpr auto BK_BMB11_SYSPG = 043046;
constexpr auto BK_BMB11_CRCADDR = 040222;
constexpr auto BK_BMB11_FICTADDR = 040222;

bool CMotherBoard_11::EmulateLoadTape11()
{
	if (g_Config.m_bEmulateLoadTape && ((GetWord(0143752) == 016706) && (GetWord(0143754) == 074240)))
	{
		bool bFileSelect = false; // что делать после диалога выбора
		bool bCancelSelectFlag = false; // флаг для усложнения алгоритма
		bool bError = false;
		bool bIsDrop = false;
		CString strBinExt(MAKEINTRESOURCE(IDS_FILEEXT_BINARY));
		uint16_t fileAddr = 0;
		uint16_t abp = BK_BMB11;
		// получим код для подключения страниц
		uint16_t nPageCode = GetWord(064) & 0204;
		int nWnd0 = GetByte(abp + BK_BMB11_PAGES) & 7;
		int nWnd1 = GetByte(abp + BK_BMB11_PAGES + 1) & 7;
		nPageCode |= (m_arPageCodes[nWnd0] << 12) | (m_arPageCodes[nWnd1] << 8) | 04000;
		uint16_t nSysPageCode = GetWord(BK_BMB11_SYSPG);
		CString strFileName;
		// Внутренняя загрузка на БК
		uint8_t bkName[BK_NAMELENGTH] = { 0 };  // Максимальная длина имени файла на БК - BK_NAMELENGTH байтов
		uint8_t bkFoundName[BK_NAMELENGTH] = { 0 };

		if (!m_pParent->isBinFileNameSet())
		{
			// если загружаем не через драг-н-дроп, то действуем как обычно
			fileAddr = GetWord(abp + BK_BMB10_ADDRESS);   // второе слово - адрес загрузки/сохранения

			// Подбираем 16 байтовое имя файла из блока параметров
			for (uint16_t c = 0; c < BK_NAMELENGTH; ++c)
			{
				bkName[c] = GetByte(abp + BK_BMB10_NAME + c);
			}

			strFileName = BKToUNICODE(bkName, BK_NAMELENGTH); // тут надо перекодировать  имя файла из кои8 в unicode
			strFileName.Trim(); // удаляем пробелы в конце файла, а в середине - оставляем

			if (!strFileName.IsEmpty()) // если имя файла не пустое
			{
				strFileName += strBinExt; // добавляем стандартное расширение для бин файлов,
				// чтобы не рушить логику следующих проверок
			}
		}
		else
		{
			// если загружаем через драг-н-дроп, то берём имя оттуда
			strFileName = m_pParent->GetStrBinFileName();
			bIsDrop = true;
		}

		if (strFileName.SpanExcluding(_T(" ")).IsEmpty())
		{
			// Если имя пустое - то покажем диалог выбора файла.
			bFileSelect = false;
l_SelectFile:
            // Запомним текущую директорию
            CString strCurDir = GetCurrentDirectory();
//			::GetCurrentDirectory(1024, strCurDir.GetBufferSetLength(1024));
//			strCurDir.ReleaseBuffer();
            ::SetCurrentDirectory(g_Config.m_strBinPath);
            CString strFilterBin(MAKEINTRESOURCE(IDS_FILEFILTER_BIN));
//			CFileDialog dlg(TRUE, nullptr, nullptr,
//			                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
//			                strFilterBin, m_pParent->GetScreen()->GetBackgroundWindow());
//			// Зададим начальной директорией директорию с Bin файлами
//			dlg.GetOFN().lpstrInitialDir = g_Config.m_strBinPath.GetString();

            strFileName = FileDialogCaller().getOpenFileName(nullptr, "", g_Config.m_strBinPath, strFilterBin);

            if (strFileName.size() == 0)
			{
				// Если нажали Отмену, установим ошибку во втором байте блока параметров
				bError = true; // случилась ошибка
			}
			else
			{
                g_Config.m_strBinPath = ::GetFilePath(strFileName);
//				strFileName = dlg.GetPathName(); // вот выбранный файл
                // имя файла надо бы как-то поместить в 0352..0372 иначе некоторые глюки наблюдаются
                CString fileTitle = ::GetFileTitle(strFileName);
                UNICODEtoBK(fileTitle, bkFoundName, BK_NAMELENGTH, true); // вот из этого массива будем потом помещать

				if (bFileSelect)
				{
					// тут надо проверить тот ли файл нам подсовывают.
                    CString strFound = ::GetFileTitle(strFileName);
					CString strFindEx = BKToUNICODE(bkName, BK_NAMELENGTH); // с расширением
					CString strFind = ::GetFileTitle(strFindEx); // без расширения

					if (!bIsDrop) // только если не дроп. там не с чем сравнивать
					{
						if (strFind.CollateNoCase(strFound) != 0 && strFindEx.CollateNoCase(strFound) != 0)
						{
							int result = g_BKMsgBox.Show(IDS_BK_ERROR_WRONGFILE, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);

							if (result == IDYES)
							{
								goto l_SelectFile;
							}
							else
							{
								bError = true;
								bCancelSelectFlag = true; // не будем ничего делать, сразу выйдем.
							}
						}
					}
				}
			}

			// восстановим текущую директорию
			::SetCurrentDirectory(strCurDir);
		}
		else    // Если имя не пустое
		{
			// If Saves Default flag is set loading from User directory
			// Else Load from Binary files directory
			CString strCurrentPath = g_Config.m_bSavesDefault ? g_Config.m_strSavesPath : g_Config.m_strBinPath;
			CFileStatus fs; // получим информацию о файле - таким образом проверяется существует он или нет
			SetSafeName(strFileName); // перекодируем небезопасные символы на безопасные

			// сейчас узнаем, нужно ли нам добавлять расширение .bin, или наоборот, удалять
			if (::GetFileExt(strFileName).CollateNoCase(strBinExt) == 0)
			{
				// у файла уже есть расширение бин
                if (!CFile::GetStatus(QDir(strCurrentPath).filePath(strFileName), fs)) // если нету файла с расширением.
				{
					CString str = ::GetFileTitle(strFileName); // удаляем расширение

                    if (CFile::GetStatus(QDir(strCurrentPath).filePath(str), fs)) // если есть файл без расширения
					{
						strFileName = str; // оставим файл без расширения
					}
					else
					{
						// нет файла ни с расширением, ни без расширения
						SetByte(BK_BMB11_ERRADDR, 1);
						bError = true;
					}
				}
			}
			else
			{
				// у файла нету расширения бин
				if (!CFile::GetStatus(QDir(strCurrentPath).filePath(strFileName), fs)) // если нету файла без расширения
				{
					CString str = strFileName + strBinExt; // добавляем стандартное расширение для бин файлов.

					if (CFile::GetStatus(QDir(strCurrentPath).filePath(str), fs))  // если есть файл с расширением
					{
						strFileName = str;
					}
					else
					{
						// нет файла ни с расширением, ни без расширения
						SetByte(BK_BMB11_ERRADDR, 1);
						bError = true;
					}
				}
			}

			strFileName = QDir(strCurrentPath).filePath(strFileName);
		}

		if (!bCancelSelectFlag)
		{
			CFile file;
			uint16_t readAddr = 0;
			uint16_t readSize = 0;

			// Загрузим файл, если ошибок не было
			if (!bError && file.Open(strFileName, CFile::modeRead))
			{
				file.Read(&readAddr, sizeof(readAddr));   // Первое слово в файле - адрес загрузки
				file.Read(&readSize, sizeof(readSize));   // Второе слово в файле - длина
				// сплошь и рядом встречаются .bin файлы. у которых во втором слове указана длина
				// меньше, чем длина файла - 4. Это другой формат бин, у которого в начале указывается
				// адрес, длина, имя файла[16], массив[длина], КС - контрольная сумма в конце
				uint16_t filesz = (file.GetLength() < 65536) ? static_cast<uint16_t>(file.GetLength()) : 65535;
				bool bIsCRC = false;

				if (readSize == filesz - 4)
				{
					bIsCRC = false;
				}
				else if (readSize == filesz - 6)
				{
					bIsCRC = true;
				}
				else if (readSize == filesz - 22)
				{
					bIsCRC = true;
					file.Read(bkFoundName, BK_NAMELENGTH); // прочитаем оригинальное имя файла
				}
				else
				{
					// всё равно загрузим. Пусть не бин
                    file.Seek(0, CFile::begin);
					readAddr = 0;
					readSize = filesz;
				}

				SetWord(abp + BK_BMB11_FOUND_ADDRESS, readAddr);
				SetWord(abp + BK_BMB11_FOUND_LENGTH, readSize);

				if (bkFoundName[0])
				{
					// копируем прочитанное имя файла
					for (uint16_t i = 0; i < BK_NAMELENGTH; ++i)
					{
						SetByte(abp + BK_BMB11_FOUND_NAME + i, bkFoundName[i]);
					}
				}
				else
				{
					// копируем прочитанное имя файла
					for (uint16_t i = 0; i < BK_NAMELENGTH; ++i)
					{
						SetByte(abp + BK_BMB11_FOUND_NAME + i, bkName[i]);
					}
				}

				if (fileAddr == 0)
				{
					fileAddr = readAddr;
				}

				bool bReadNameOnly = !!GetByte(BK_BMB11_RNOFLAG); //прочитать только имя файла
				bool bFictive = !!GetByte(BK_BMB11_FICTFLAG); //фиктивное чтение

				if (!bReadNameOnly)
				{
					SetWord(0177716, nPageCode); // подключаем нужные страницы ОЗУ
					DWORD cs = 0; // подсчитаем контрольную сумму

					// Загрузка по адресу fileAddr
					for (int i = 0; i < readSize; ++i)
					{
						uint8_t val;
						file.Read(&val, sizeof(val));

						if (bFictive)
						{
							SetByte(BK_BMB11_FICTADDR, val);
						}
						else
						{
							SetByte(fileAddr++, val);
						}

						cs += uint16_t(val);

						if (cs & 0xffff0000)
						{
							cs++;
							cs &= 0xffff;
						}
					}

					SetWord(0177716, nSysPageCode); // подключаем системные страницы
					uint16_t crc;

					if (bIsCRC && file.Read(&crc, sizeof(crc)) == sizeof(uint16_t))
					{
						if (crc != LOWORD(cs))
						{
							SetByte(BK_BMB11_ERRADDR, 2);
							cs = crc;
						}
					}

					// а иначе, мы не знаем какая должна быть КС. поэтому считаем, что файл априори верный
					file.Close();
					// Заполняем системные ячейки, как это делает emt 36
					uint16_t loadcrc = LOWORD(cs);
					SetWord(BK_BMB11_CRCADDR, loadcrc); // сохраним контрольную сумму
				}

				SetRON(CCPU::REGISTER::PC, 0144026); // выходим туда.
			}
			else
			{
				// При ошибке покажем сообщение
				CString strError;
				strError.Format(IDS_CANT_OPEN_FILE_S, strFileName);
				int result = g_BKMsgBox.Show(strError, MB_ICONWARNING | MB_YESNOCANCEL | MB_DEFBUTTON2);

				switch (result)
				{
					case IDNO:
						// если не хотим останавливаться, то пойдём на диалог, и поищем файл в другом месте.
						bError = false;
						SetByte(BK_BMB11_ERRADDR, 0);
						bFileSelect = true; // включим проверку на неподходящее имя.
						goto l_SelectFile;

					// если отмена - просто выходим с заданным кодом ошибки
					case IDYES:
						// если хотим остановиться - зададим останов.
						BreakCPU();
						SetByte(BK_BMB11_ERRADDR, 4);
						break;
				}

				SetRON(CCPU::REGISTER::PC, 0143752); // выходим на обработку ошибок.
			}
		}
		else
		{
			SetRON(CCPU::REGISTER::PC, 0143752); // выходим на обработку ошибок.
		}

		// Refresh keyboard
		m_pParent->SendMessage(WM_RESET_KBD_MANAGER); // и почистим индикацию управляющих клавиш в статусбаре
		return true; // сэмулировал
	}

	return false;
}


bool CMotherBoard_11::EmulateSaveTape11()
{
	if (g_Config.m_bEmulateSaveTape && ((GetWord(0143752) == 016706) && (GetWord(0143754) == 074240)))
	{
		bool bError = false;
		uint16_t abp = BK_BMB11; // вот тут блок параметров
		// получим код для подключения страниц
		uint16_t nPageCode = GetWord(064) & 0204;
		int nWnd0 = GetByte(abp + BK_BMB11_PAGES) & 7;
		int nWnd1 = GetByte(abp + BK_BMB11_PAGES + 1) & 7;
		nPageCode |= (m_arPageCodes[nWnd0] << 12) | (m_arPageCodes[nWnd1] << 8) | 04000;
		uint16_t nSysPageCode = GetWord(BK_BMB11_SYSPG);
		// получим адрес блока параметров из R1 (BK emt 36)
		uint16_t fileAddr = GetWord(abp + BK_BMB10_ADDRESS);  // второе слово - адрес загрузки/сохранения
		uint16_t fileSize = GetWord(abp + BK_BMB10_LENGTH);  // третье слово - длина файла (для загрузки может быть 0)
		uint16_t cs = GetWord(BK_BMB11_CRCADDR);// заберём подсчитанную КС

		if (fileSize)
		{
			uint8_t bkName[BK_NAMELENGTH];   // Максимальная длина имени файла на БК - 16 байтов

			// Подбираем 16 байтовое имя файла из блока параметров
			for (uint16_t c = 0; c < BK_NAMELENGTH; ++c)
			{
				bkName[c] = GetByte(abp + BK_BMB10_NAME + c);
			}

			CString strFileName = BKToUNICODE(bkName, BK_NAMELENGTH); // тут надо перекодировать  имя файла из кои8 в unicode
			strFileName.Trim(); // удаляем пробелы в конце файла, а в середине - оставляем

			// Если имя пустое
			if (strFileName.SpanExcluding(_T(" ")).IsEmpty())
			{
				// Покажем диалог сохранения
//				CFileDialog dlg(FALSE, nullptr, nullptr,
//				                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
//				                nullptr, m_pParent->GetScreen()->GetBackgroundWindow());
//				dlg.GetOFN().lpstrInitialDir = g_Config.m_strBinPath.GetString();

				strFileName = FileDialogCaller().getSaveFileName(nullptr, "", g_Config.m_strBinPath, nullptr);

				if (strFileName.size() > 0)
				{
					g_Config.m_strBinPath = ::GetFilePath(strFileName);
					// Получим имя
//					strFileName = dlg.GetPathName();
					UNICODEtoBK(strFileName, bkName, BK_NAMELENGTH, true);
				}
				else
				{
					// Если отмена - установим флаг ошибки
					bError = true;
				}
			}
			else
			{
				CString strBinExt(MAKEINTRESOURCE(IDS_FILEEXT_BINARY));
				// Если имя не пустое
				SetSafeName(strFileName);
				strFileName = (g_Config.m_bSavesDefault ? g_Config.m_strSavesPath : g_Config.m_strBinPath) // подставляем соответствующий путь
				              + strFileName + strBinExt; // добавляем стандартное расширение для бин файлов.
			}

			CFile file;

			// Save file array if no errors
			if (!bError && (file.Open(strFileName, CFile::modeCreate | CFile::modeWrite)))
			{
				// Записываем заголовок бин файла
				file.Write(&fileAddr, sizeof(fileAddr)); // слово адреса
				file.Write(&fileSize, sizeof(fileSize)); // слово длины

				if (g_Config.m_bUseLongBinFormat)
				{
					file.Write(bkName, BK_NAMELENGTH); // имя файла
				}

				SetWord(0177716, nPageCode); // подключаем нужные страницы ОЗУ

				for (int i = 0; i < fileSize; ++i)
				{
					uint8_t val = GetByte(fileAddr++);
					file.Write(&val, sizeof(val));
				}

				SetWord(0177716, nSysPageCode); // подключаем системные страницы

				if (g_Config.m_bUseLongBinFormat)
				{
					file.Write(&cs, sizeof(uint16_t)); // контрольная сумма
				}

				file.Close();
			}
		}

		if (bError)
		{
			SetRON(CCPU::REGISTER::PC, 0143752); // выходим на обработку ошибок.
		}
		else
		{
			SetRON(CCPU::REGISTER::PC, 0144026); // выходим туда.
		}

		// Refresh keyboard
		m_pParent->SendMessage(WM_RESET_KBD_MANAGER); // и почистим индикацию управляющих клавиш в статусбаре
		return true; // сэмулировали
	}

	return false;
}

