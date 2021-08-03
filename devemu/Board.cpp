// Chip.cpp: implementation of the CMotherBoard class.
//


#include "pch.h"
#include "Ini.h"
#include "Board.h"
#include "BKMessageBox.h"

#include <QApplication>
#include <QFileDialog>
#include <time.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CMotherBoard::CMotherBoard(BK_DEV_MPI model)
	: m_pParent(nullptr)   // Указатель на окно, в которое посылаются оповещающие сообщения
	, m_pSpeaker(nullptr)
	, m_pCovox(nullptr)
	, m_pMenestrel(nullptr)
	, m_pAY8910(nullptr)
	, m_pDebugger(nullptr)
	, m_bBreaked(false)
	, m_bRunning(false)
	, m_bKillTimerEvent(false)
	, m_nLowBound(1)
	, m_nHighBound(100000000)
	, m_nCPUFreq_prev(0)
	  // переменные,задающие конкретную модель
	, m_BoardModel(model)
	, m_nStartAddr(0100000)
	, m_nBKPortsIOArea(BK_PURE_PORTSIO_AREA)
{
	m_sTV.init();
	SetCPUBaseFreq(CPU_SPEED_BK10); // частота задаётся этой константой
	ZeroMemory(m_MemoryMap, sizeof(m_MemoryMap));
	m_cpu.AttachBoard(this);
	m_fdd.AttachBoard(this);
	m_fdd.init_A16M_10(&m_ConfBKModel, ALTPRO_A16M_STD10_MODE);
	// Инициализация модуля памяти
	m_pMemory = new uint8_t[0200000];

	if (!m_pMemory)
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

	InitMemoryValues(0200000);
}

CMotherBoard::~CMotherBoard()
{
	SAFE_DELETE_ARRAY(m_sTV.pSoundBuffer);
	SAFE_DELETE_ARRAY(m_pMemory);
}

MSF_CONF CMotherBoard::GetConfiguration()
{
	return MSF_CONF::BK1001;
}

BK_DEV_MPI CMotherBoard::GetBoardModel()
{
	return m_BoardModel;
}

void CMotherBoard::AttachWindow(CMainFrame *pParent)
{
	m_pParent = pParent;
}

void CMotherBoard::AttachSound(CBkSound *pSnd)
{
	m_pSound = pSnd;
}

void CMotherBoard::AttachSpeaker(CSpeaker *pDevice)
{
	m_pSpeaker = pDevice;
}

void CMotherBoard::AttachCovox(CCovox *pDevice)
{
	m_pCovox = pDevice;
}

void CMotherBoard::AttachMenestrel(CMenestrel *pDevice)
{
	m_pMenestrel = pDevice;
}

void CMotherBoard::AttachAY8910(CEMU2149 *pDevice)
{
	m_pAY8910 = pDevice;
}

void CMotherBoard::AttachDebugger(CDebugger *pDevice)
{
	m_pDebugger = pDevice;
}

void CMotherBoard::InitMemoryValues(int nMemSize)
{
	uint16_t val = 0;
	uint8_t flag = 0;
	auto pPtr = reinterpret_cast<uint16_t *>(m_pMemory);

	for (int i = 0; i < nMemSize / 2; i++, flag--)
	{
		pPtr[i] = val;
		val = ~val;

		if (flag == 192)
		{
			val = ~val;
			flag = 0;
		}
	}
}

void CMotherBoard::OnReset()
{
	if (m_pParent)
	{
		m_pParent->GetScreen()->SetPalette(0);
		m_pParent->GetScreen()->SetRegister(m_reg177664);
	}

	m_sTV.clear();
	m_nCPUFreq_prev = 0; // принудительно заставим применить изменения параметров фрейма
	FrameParam();
}

/*
В функциях GetByte, GetWord, SetByte, SetWord доступ к регистрам 0177700..0177712
не обрабатывается, т.к. эти функции должны вызываться только из CPU, и доступ к
регистрам обрабатывается там.
*/

void CMotherBoard::GetByte(uint16_t addr, uint8_t *pValue)
{
	*pValue = GetByte(addr);
}

uint8_t CMotherBoard::GetByte(uint16_t addr)
{
	register int nTC = 0;
	return GetByteT(addr, nTC);
}

uint8_t CMotherBoard::GetByteT(uint16_t addr, int &nTC)
{
	register int nBank = (addr >> 12) & 0x0f;
	register uint8_t v;

	// Сперва проверим, на системные регистры
	if (nBank == 15)
	{
		register BK_DEV_MPI nBKFddType = GetFDDType();

		if (addr >= m_nBKPortsIOArea)
		{
			switch (nBKFddType)
			{
				case BK_DEV_MPI::SMK512:
					switch (GetAltProMode())
					{
						case ALTPRO_SMK_SYS_MODE:
						case ALTPRO_SMK_ALL_MODE:
							// в режиме SYS и ALL в этом диапазоне можно читать
							goto gblb1;

						default:
							if (OnGetSystemRegister(addr, &v, true))
							{
								nTC += REG_TIMING_CORR_VALUE;
								return v;
							}
					}

					// иначе, в этом диапазоне действуют обычные правила
					break;

				case BK_DEV_MPI::A16M:
					switch (GetAltProMode())
					{
						case ALTPRO_A16M_START_MODE:
							// в режиме Start в этом диапазоне можно читать
gblb1:
							nTC += m_MemoryMap[nBank].nTimingCorrection;

							if (OnGetSystemRegister(addr, &v, true))
							{
								return v | m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007777)];
							}
							else
							{
								return m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007777)];
							}

						case ALTPRO_A16M_BASIC_MODE:

							// ещё одно исключение. в режиме бейсика m_nBKPortsIOArea == BK_PURE_PORTSIO_AREA целых 0600 байтов ещё
							if ((addr < BK_PURE_PORTSIO_AREA) && (GetAltProExtCode() & 010)) // если бейсик подключён
							{
								goto gblb1;
							}
					}

				// тут break; не нужен!!!
				// иначе, как в обычном режиме, смотрим порты
				default:

					// в обычном режиме тут только регистры могут быть
					if (OnGetSystemRegister(addr, &v, true))
					{
						nTC += REG_TIMING_CORR_VALUE;
						return v;
					}
					else
					{
						throw CExceptionHalt(addr, _T("Can't read this address."));
					}

					break;
			}
		}
		else
		{
			// по адресам 170000-177000 для SMK если режим не SYS - всегда ОЗУ
			if (nBKFddType == BK_DEV_MPI::SMK512 && GetAltProMode() != ALTPRO_A16M_START_MODE)
			{
				nTC += m_MemoryMap[nBank].nTimingCorrection;
				return m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007777)];
			}
		}
	}

	// теперь проверим на чтение из несуществующего банка
	if (m_MemoryMap[nBank].bReadable)
	{
		nTC += m_MemoryMap[nBank].nTimingCorrection;
		// если читать можно, заодно и ремап сделаем, может не понадобится переопределять везде эту функцию
		return m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007777)];
	}
	else
	{
		throw CExceptionHalt(addr, _T("Can't read this address."));
	}
}


void CMotherBoard::GetWord(uint16_t addr, uint16_t *pValue)
{
	*pValue = GetWord(addr);
}

uint16_t CMotherBoard::GetWord(uint16_t addr)
{
	int nTC = 0;
	return GetWordT(addr, nTC);
}

uint16_t CMotherBoard::GetWordT(uint16_t addr, int &nTC)
{
	register int nBank = (addr >> 12) & 0x0f;
	register uint16_t v;

	// Сперва проверим, на системные регистры
	if (nBank == 15)
	{
		register BK_DEV_MPI nBKFddType = GetFDDType();

		if (addr >= m_nBKPortsIOArea)
		{
			switch (nBKFddType)
			{
				case BK_DEV_MPI::SMK512:
					switch (GetAltProMode())
					{
						case ALTPRO_SMK_SYS_MODE:
						case ALTPRO_SMK_ALL_MODE:
							// в режиме SYS и ALL в этом диапазоне можно читать
							goto gwlb1;

						default:
							if (OnGetSystemRegister(addr, &v, false))
							{
								nTC += REG_TIMING_CORR_VALUE;
								return v;
							}
					}

					// иначе, в этом диапазоне действуют обычные правила
					break;

				case BK_DEV_MPI::A16M:
					switch (GetAltProMode())
					{
						case ALTPRO_A16M_START_MODE:
							// в режиме Start в этом диапазоне можно читать
gwlb1:
							nTC += m_MemoryMap[nBank].nTimingCorrection;

							if (OnGetSystemRegister(addr, &v, false))
							{
								return v | *(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007776)];
							}
							else
							{
								return *(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007776)];
							}

						case ALTPRO_A16M_BASIC_MODE:

							// ещё одно исключение. в режиме бейсика m_nBKPortsIOArea == BK_PURE_PORTSIO_AREA целых 0600 байтов ещё
							if ((addr < BK_PURE_PORTSIO_AREA) && (GetAltProExtCode() & 010)) // если бейсик подключён
							{
								goto gwlb1;
							}
					}

				// тут break; не нужен!!!
				// иначе, как в обычном режиме, смотрим порты
				default:

					// в обычном режиме тут только регистры могут быть
					if (OnGetSystemRegister(addr, &v, false))
					{
						nTC += REG_TIMING_CORR_VALUE;
						return v;
					}
					else
					{
						throw CExceptionHalt(addr, _T("Can't read this address."));
					}

					break;
			}
		}
		else
		{
			// по адресам 170000-177000 для SMK если режим не SYS - всегда ОЗУ
			if (nBKFddType == BK_DEV_MPI::SMK512 && GetAltProMode() != ALTPRO_A16M_START_MODE)
			{
				nTC += m_MemoryMap[nBank].nTimingCorrection;
				return *(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007776)];
			}
		}
	}

	// теперь проверим на чтение из несуществующего банка
	if (m_MemoryMap[nBank].bReadable)
	{
		nTC += m_MemoryMap[nBank].nTimingCorrection;
		// если читать можно, заодно и ремап сделаем, может не понадобится переопределять везде эту функцию
		return *(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007776)];
	}
	else
	{
		throw CExceptionHalt(addr, _T("Can't read this address."));
	}
}


void CMotherBoard::SetByte(uint16_t addr, uint8_t value)
{
	int nTC = 0;
	SetByteT(addr, value, nTC);
}

void CMotherBoard::SetByteT(uint16_t addr, uint8_t value, int &nTC)
{
	register int nBank = (addr >> 12) & 0x0f;

	// Сперва проверим, на системные регистры
	if (nBank == 15)
	{
		register BK_DEV_MPI nBKFddType = GetFDDType();

		if (addr >= m_nBKPortsIOArea)
		{
			switch (nBKFddType)
			{
				case BK_DEV_MPI::SMK512:
					switch (GetAltProMode())
					{
						case ALTPRO_SMK_HLT11_MODE:
						case ALTPRO_SMK_HLT10_MODE:
							// case ALTPRO_SMK_OZU10_MODE:
							// в режиме Hlt11 и Hlt10 в этом диапазоне можно писать, а для старой версии, ещё и в режиме ОЗУ10
							goto sblb1;

						default:
							if (OnSetSystemRegister(addr, value, true))
							{
								nTC += REG_TIMING_CORR_VALUE;
								return;
							}
					}

					// иначе, в этом диапазоне действуют обычные правила
					break;

				case BK_DEV_MPI::A16M:

					// в режиме Hlt11 в этом диапазоне можно писать
					if (GetAltProMode() == ALTPRO_A16M_HLT11_MODE)
					{
sblb1:
						OnSetSystemRegister(addr, value, true);
						m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 07777)] = value;
						nTC += m_MemoryMap[nBank].nTimingCorrection;
						return;
					}

				// иначе, как в обычном режиме, смотрим порты
				default:

					// в обычном режиме тут только регистры могут быть
					if (OnSetSystemRegister(addr, value, true))
					{
						nTC += REG_TIMING_CORR_VALUE;
						return;
					}
					else
					{
						throw CExceptionHalt(addr, _T("Can't write this address."));
					}

					break;
			}
		}
		else
		{
			if (nBKFddType == BK_DEV_MPI::SMK512 && GetAltProMode() != ALTPRO_A16M_START_MODE)
			{
				m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 07777)] = value;
				nTC += m_MemoryMap[nBank].nTimingCorrection;
				return;
			}
		}
	}

	if (m_MemoryMap[nBank].bWritable)
	{
		m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 07777)] = value;
		nTC += m_MemoryMap[nBank].nTimingCorrection;
	}
	else
	{
		throw CExceptionHalt(addr, _T("Can't write this address."));
	}
}


void CMotherBoard::SetWord(uint16_t addr, uint16_t value)
{
	int nTC = 0;
	SetWordT(addr, value, nTC);
}

void CMotherBoard::SetWordT(uint16_t addr, uint16_t value, int &nTC)
{
	register int nBank = (addr >> 12) & 0x0f;

	// Сперва проверим, на системные регистры
	if (nBank == 15)
	{
		register BK_DEV_MPI nBKFddType = GetFDDType();

		if (addr >= m_nBKPortsIOArea)
		{
			switch (nBKFddType)
			{
				case BK_DEV_MPI::SMK512:
					switch (GetAltProMode())
					{
						case ALTPRO_SMK_HLT11_MODE:
						case ALTPRO_SMK_HLT10_MODE:
							// case ALTPRO_SMK_OZU10_MODE:
							// в режиме Hlt11 и Hlt10 в этом диапазоне можно писать, а для старой версии, ещё и в режиме ОЗУ10
							goto swlb1;

						default:
							if (OnSetSystemRegister(addr, value, false))
							{
								nTC += REG_TIMING_CORR_VALUE;
								return;
							}
					}

					// иначе, в этом диапазоне действуют обычные правила
					break;

				case BK_DEV_MPI::A16M:

					// в режиме Hlt11 в этом диапазоне можно писать
					if (GetAltProMode() == ALTPRO_A16M_HLT11_MODE)
					{
swlb1:
						OnSetSystemRegister(addr, value, false);
						*(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 07776)] = value;
						nTC += m_MemoryMap[nBank].nTimingCorrection;
						return;
					}

				// иначе, как в обычном режиме, смотрим порты
				default:

					// в обычном режиме тут только регистры могут быть
					if (OnSetSystemRegister(addr, value, false))
					{
						nTC += REG_TIMING_CORR_VALUE;
						return;
					}
					else
					{
						throw CExceptionHalt(addr, _T("Can't write this address."));
					}

					break;
			}
		}
		else
		{
			if (nBKFddType == BK_DEV_MPI::SMK512 && GetAltProMode() != ALTPRO_A16M_START_MODE)
			{
				*(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 07776)] = value;
				nTC += m_MemoryMap[nBank].nTimingCorrection;
				return;
			}
		}
	}

	if (m_MemoryMap[nBank].bWritable)
	{
		*(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 07776)] = value;
		nTC += m_MemoryMap[nBank].nTimingCorrection;
	}
	else
	{
		throw CExceptionHalt(addr, _T("Can't write this address."));
	}
}


/*
GetByteIndirect,GetWordIndirect,SetByteIndirect,SetWordIndirect
этими функциями пользуются многие вспомогательные модули для того чтобы читать данные
напрямую из памяти, и как правило чтобы читать/писать что-то из/в регистров, а т.к.
внутренние системные регистры переехали в CPU, здесь делается перехват их адресов, чтобы
получить/записать значения из/в регистров в CPU
и да. тут не делаются проверки на невозможность, читать/писать в память, ибо эти функции
не CPU. и в экранную память вывод тоже не делается.
*/

uint8_t CMotherBoard::GetByteIndirect(uint16_t addr)
{
	register int nBank = (addr >> 12) & 0x0f;
	register uint8_t v;

	if (OnGetSystemRegister(addr, &v, true))
	{
		return (GetAltProMode() == ALTPRO_A16M_START_MODE) ? (v | m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007777)]) : v;
	}

	if ((0177700 <= addr) && (addr < 0177714))
	{
		register uint16_t w = m_cpu.GetSysRegs(addr);

		if (GetAltProMode() == ALTPRO_A16M_START_MODE)
		{
			w |= m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007777)];
		}

		return (addr & 1) ? HIBYTE(w) : LOBYTE(w);
	}

	return m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007777)];
}


uint16_t CMotherBoard::GetWordIndirect(uint16_t addr)
{
	register int nBank = (addr >> 12) & 0x0f;
	register uint16_t v;

	if (OnGetSystemRegister(addr, &v, false))
	{
		return (GetAltProMode() == ALTPRO_A16M_START_MODE) ? (v | *(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007776)]) : v;
	}

	if ((0177700 <= addr) && (addr < 0177714))
	{
		register uint16_t w = m_cpu.GetSysRegs(addr);

		if (GetAltProMode() == ALTPRO_A16M_START_MODE)
		{
			w |= *(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007776)];
		}

		return w;
	}

	return *(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007776)];
}


void CMotherBoard::SetByteIndirect(uint16_t addr, uint8_t value)
{
	register int nBank = (addr >> 12) & 0x0f;

	if (OnSetSystemRegister(addr, value, true))
	{
		if (GetAltProMode() == ALTPRO_A16M_HLT11_MODE || (GetAltProMode() == ALTPRO_SMK_HLT10_MODE && GetFDDType() == BK_DEV_MPI::SMK512))
		{
			m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007777)] = value;
		}

		return;
	}

	if ((0177700 <= addr) && (addr < 0177714))
	{
		m_cpu.SetSysRegsInternal(addr, value);

		if (GetAltProMode() == ALTPRO_A16M_HLT11_MODE || (GetAltProMode() == ALTPRO_SMK_HLT10_MODE && GetFDDType() == BK_DEV_MPI::SMK512))
		{
			m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007777)] = value;
		}
	}
	else
	{
		m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007777)] = value;
	}
}


void CMotherBoard::SetWordIndirect(uint16_t addr, uint16_t value)
{
	register int nBank = (addr >> 12) & 0x0f;

	if (OnSetSystemRegister(addr, value, false))
	{
		if (GetAltProMode() == ALTPRO_A16M_HLT11_MODE || (GetAltProMode() == ALTPRO_SMK_HLT10_MODE && GetFDDType() == BK_DEV_MPI::SMK512))
		{
			*(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007776)] = value;
		}

		return;
	}

	if ((0177700 <= addr) && (addr < 0177714))
	{
		m_cpu.SetSysRegsInternal(addr, value);

		if (GetAltProMode() == ALTPRO_A16M_HLT11_MODE || (GetAltProMode() == ALTPRO_SMK_HLT10_MODE && GetFDDType() == BK_DEV_MPI::SMK512))
		{
			*(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007776)] = value;
		}
	}
	else
	{
		*(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 007776)] = value;
	}
}


uint16_t CMotherBoard::GetRON(CCPU::REGISTER reg)
{
	return m_cpu.GetRON(reg);
}

void CMotherBoard::SetRON(CCPU::REGISTER reg, uint16_t value)
{
	m_cpu.SetRON(reg, value);
}

// это ресет оборудования, выполняемый по команде RESET. вызывается из CPU
void CMotherBoard::ResetDevices()
{
	m_reg177660 = 0100; // команда RESET запрещает прерывания от клавиатуры. проверено.
	m_reg177714in = 0;
	m_reg177714out = 0;
	m_reg177716out_tap = 0200;
	m_reg177716in |= 0300;

	// если у нас есть сопр. по команде RESET он тоже должен инициализироваться (в правильной схеме подключения)
	if (m_pAY8910)
	{
		m_pAY8910->Reset();
	}

	// инициализация ОЗУ и CPU
	Reset(); // заставляет выполниться OnReset
}


bool CMotherBoard::InitBoard(uint16_t addrStart)
{
	if (addrStart == 0)
	{
		addrStart = m_nStartAddr;    // Используем адрес по умолчанию
	}
	else
	{
		m_nStartAddr = addrStart;    // иначе используем адрес из конфигурации
	}

	// инициализация регистров
	m_reg177716in = addrStart & 0177400;
	m_reg177662out = 047400;
	m_reg177664 = 0330; // начальное значение. на самом деле - оно случайное
	m_reg177662in = 0;
	m_reg177716out_mem = 0;

	// инициализация карты памяти
	if (!InitMemoryModules())
	{
		return false;
	}

	if (m_pSpeaker)
	{
		m_pSpeaker->Reset();
	}

	if (m_pCovox)
	{
		m_pCovox->Reset();
	}

	m_cpu.InitCPU();

	if (m_pParent)
	{
        emit m_pParent->SendMessage(WM_RESET_KBD_MANAGER); // почистим индикацию управляющих клавиш в статусбаре
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
// на входе - маска, по которой сбрасываем биты адреса.
void CMotherBoard::ResetCold(uint16_t addrMask)
{
	m_reg177662out = 047400;    // регистр инициализируется
	//m_reg177664 = 0330;   ресет не должен затрагивать этот регистр
	m_reg177662in = 0;
	m_reg177716in = m_nStartAddr & ~addrMask; // формируем адрес запуска
	m_cpu.InitCPU(); // здесь делается чтение адреса запуска из регистра 177716in
	m_reg177716in = m_nStartAddr & 0177400; // и восстановим стандартное значение адреса запуска, без сброшенных битов

	if (m_pParent)
	{
        emit m_pParent->SendMessage(WM_RESET_KBD_MANAGER); // почистим индикацию управляющих клавиш в статусбаре
	}
}


void CMotherBoard::StopInterrupt()
{
	// нажали на кнопку стоп
	m_cpu.SetIRQ1();
}

void CMotherBoard::UnStopInterrupt()
{
	// отжали кнопку стоп
	m_cpu.UnsetIRQ1();
}


void CMotherBoard::KeyboardInterrupt(uint16_t interrupt)
{
	if (!(m_reg177660 & 0100))
	{
		// если прерывания разрешены, делаем прерывание
		m_cpu.InterruptVIRQ(interrupt);
	}
}

int CMotherBoard::CalcStep()
{
	int code = 0;
    Qt::KeyboardModifiers key = QApplication::queryKeyboardModifiers();

//	if (::GetAsyncKeyState(VK_CONTROL))
    if (key & Qt::ControlModifier)
	{
		code |= (1 << 1);
	}

//	if (::GetAsyncKeyState(VK_MENU))
    if (key & Qt::MetaModifier)
	{
		code |= (1 << 2);
	}

//	if (::GetAsyncKeyState(VK_SHIFT))
    if (key & Qt::ShiftModifier)
	{
		code |= (1 << 0);
	}

	int step;

	switch (code)
	{
		default:
		case 0: // ничего не нажато
			step = 1000;
			break;

		case 1: // shift
			step = 1;
			break;

		case 2: // ctrl
			step = 10;
			break;

		case 3: // crtl+shift
			step = 100;
			break;

		case 4: // alt
			step = 1000000;
			break;

		case 5: // alt+shift
			step = 10000;
			break;

		case 6: // ctrl+alt
			step = 100000;
			break;

		case 7: // ctrl+alt+shift
			step = 0; // мегакомбо
			break;
	}

	return step;
}

void CMotherBoard::AccelerateCPU()
{
	if (m_nCPUFreq < m_nHighBound)
	{
		register int n = CalcStep(); // если мегакомбо, для увеличения - увеличиваем в 2 раза

		if (n)
		{
			m_nCPUFreq += n;
		}
		else
		{
			m_nCPUFreq *= 2;
		}

		if (m_nCPUFreq > m_nHighBound)
		{
			m_nCPUFreq = m_nHighBound;
		}
	}
}

void CMotherBoard::SlowdownCPU()
{
	if (m_nCPUFreq > m_nLowBound)
	{
		register int n = CalcStep(); // если мегакомбо, для уменьшения - уменьшаем в 2 раза

		if (n)
		{
			m_nCPUFreq -= n;
		}
		else
		{
			m_nCPUFreq /= 2;
		}

		if (m_nCPUFreq < m_nLowBound)
		{
			m_nCPUFreq = m_nLowBound;
		}
	}
}


void CMotherBoard::NormalizeCPU()
{
	m_nCPUFreq = m_nBaseCPUFreq;
}

bool CMotherBoard::CanAccelerate()
{
	return !(m_nCPUFreq >= m_nHighBound);
}

bool CMotherBoard::CanSlowDown()
{
	return !(m_nCPUFreq <= m_nLowBound);
}

int CMotherBoard::GetLowBound()
{
	return m_nLowBound;
}

int CMotherBoard::GetHighBound()
{
	return m_nHighBound;
}

void CMotherBoard::SetCPUBaseFreq(int frq) // установка базовой частоты
{
	m_nCPUFreq = m_nBaseCPUFreq = frq;
}

void CMotherBoard::SetCPUFreq(int frq) // установка текущей частоты
{
	if (frq < GetLowBound())
	{
		frq = GetLowBound();
	}
	else if (frq > GetHighBound())
	{
		frq = GetHighBound();
	}

	m_nCPUFreq = frq;
}

int CMotherBoard::GetCPUFreq() // выдача текущей частоты
{
	return m_nCPUFreq;
}

int CMotherBoard::GetCPUSpeed() // выдача текущей частоты для конфигурации
{
	return (m_nCPUFreq == m_nBaseCPUFreq) ? 0 : m_nCPUFreq;
}

CFDDController *CMotherBoard::GetFDD()
{
	return &m_fdd;
}

void CMotherBoard::SetFDDType(BK_DEV_MPI model, bool bInit /*= true*/)
{
    (void)bInit;
	m_fdd.SetFDDType(model);    // не забывать! bool bInit удалять нельзя, оно в других местах нужно.
}

BK_DEV_MPI CMotherBoard::GetFDDType()
{
	return m_fdd.GetFDDType();
}

uint16_t CMotherBoard::GetAltProMode()
{
	return LOWORD(m_ConfBKModel.nAltProMode);
}

void CMotherBoard::SetAltProMode(uint16_t w)
{
	m_ConfBKModel.nAltProMode = w;
	MemoryManager();
}

uint16_t CMotherBoard::GetAltProCode()
{
	return LOWORD(m_ConfBKModel.nAltProMemBank);
}

void CMotherBoard::SetAltProCode(uint16_t w)
{
	m_ConfBKModel.nAltProMemBank = w;
	MemoryManager();
}

uint16_t CMotherBoard::GetAltProExtCode()
{
	return LOWORD(m_ConfBKModel.nExtCodes);
}

void CMotherBoard::Set177716RegTap(uint16_t w)
{
	register uint16_t mask = 0360;
	m_reg177716out_tap = (~mask & m_reg177716out_tap) | (mask & w);
	m_pSpeaker->SetData(m_reg177716out_tap);
	// пока не решится проблема ложных срабатываний, это лучше не использовать
	/*
	if (m_pParent->GetTapePtr()->IsWaveLoaded())
	{
	    if (m_reg177716out_tap & 0200)
	    {
	        m_pParent->GetTapePtr()->StopPlay();
	    }
	    else
	    {
	        m_pParent->GetTapePtr()->StartPlay();
	    }
	}
	// */
}

uint8_t *CMotherBoard::GetMainMemory()
{
	return m_pMemory;
}

uint8_t *CMotherBoard::GetAddMemory()
{
	return nullptr;
}

/*
Чтение из системных регистров
вход: addr - адрес регистра (177660, 177716 и т.п.)
      pDst - адрес в массиве, куда сохраняется новое значение
      bByteOperation - флаг операции true - байтовая, false - словная
*/
bool CMotherBoard::OnGetSystemRegister(uint16_t addr, void *pDst, bool bByteOperation)
{
	auto pDst_W = reinterpret_cast<uint16_t *>(pDst);
	auto pDst_B = reinterpret_cast<uint8_t *>(pDst);
	register bool bRet = true;
	register uint16_t v = 0;

	switch (addr & 0177776)
	{
		case 0177660:
			v = m_reg177660;
			break;

		case 0177662:
			// если читаем из регистра данных клавиатуры,
			// сбросим бит готовности из регистра состояния клавиатуры
			m_reg177660 &= ~0200;
			v = m_reg177662in;
//          {
//              TCHAR ch = BKToUNICODE_Char(v);
//              TRACE1("board readed char %c\n", ch);
//          }
			break;

		case 0177664:
			v = m_reg177664;
			break;

		case 0177714:
			v = m_reg177714in;
			break;

		case 0177716:
			v = m_reg177716in;
			// В БК 2й разряд SEL1 фиксирует любую запись в этот регистр, взводя триггер D9.1 на неограниченное время, сбрасывается который любым чтением этого регистра.
			m_reg177716in &= ~4;
			break;

		default:
			bRet = false;
	}

	if (bByteOperation)
	{
		*pDst_B = (addr & 1) ? HIBYTE(v) : LOBYTE(v);
	}
	else
	{
		*pDst_W = v;
	}

	return bRet;
}

/*
Запись данных в системные регистры БК0010
вход: addr - адрес регистра(177660, 177716 и т.п.)
      src - записываемое значение.
      bByteOperation - флаг операции true - байтовая, false - словная
*/
bool CMotherBoard::OnSetSystemRegister(uint16_t addr, uint16_t src, bool bByteOperation)
{
	register uint16_t mask; // это какие биты в регистре используются

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
			(0200)бит 7 -- готовность: "1" -- в регистре данных клавиатуры(177662) готов код нажатой клавиши.
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
			/*
			177662
			Регистр данных клавиатуры.
			    биты 0-6: код клавиши. Доступ только по чтению.
			(040000)бит 14: разрешение прерывания по(внешнему) таймеру(50 Гц),
			    "0" -- прерывание разрешено,
			    "1" -- таймер отключён.
			    Доступен только по записи.
			    Бит 14 на БК0010 не документирован, и таймер работал только на некоторых экземплярах, можно считать, что его нет
			*/
			return false; // на бк10 запись запрещена. это был один из способов определения бк10 или бк11

		case 0177664:
			/*
			177664
			Регистр скроллинга. Доступен по записи и чтению.
			    биты 0-7: смещение скроллинга, строк. Начальное значение -- 0330.
			(01000)бит 9: сокращённый режим экрана, "0" -- сокращённый(1/4 экрана, старшие адреса),
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
			Системный регистр. Внешний регистр 1(ВР1, SEL1) процессора ВМ1, регистр начального пуска.
			как и 177714 состоит из двух регистров, раздельных по чтению и по записи
			По чтению:
			(004)бит 2: признак записи в системный регистр. Устанавливается в "1" при
			    любой записи в регистр, сбрасывается в "0" после чтения из регистра.
			(020)бит 4: данные с ТЛГ-линии, "0" - логический 0, "1" - логическая 1
			(040)бит 5: данные с магнитофона, "0" - логический 0, "1" - логическая 1
			(100)бит 6: индикатор нажатия клавиши, установлен в "0" если нажата клавиша клавиатуры, "1" если нет нажатых клавиш.
			(200)бит 7: сигнал готовности с ТЛГ-линии, "0" - логический 0, "1" - логическая 1
			        Однако по заводской документации на БК0010 этот бит - константа "1", указывающая на отсутствие
			        в системе команд расширенной арифметики
			    биты 8-15: адрес начального пуска, 100000(БК-0010), младший байт при этом игнорируется
			    биты 0,1,3, не используются, "0".
			По записи:
			(020)бит 4: данные для передачи на ТЛГ-линию, "0" - логический 0, "1" - логическая 1
			(040)бит 5: данные на магнитофон(либо сигнал готовности на ТЛГ-линию - этого в тех документации нету.). Начальное состояние "0".
			(100)бит 6: данные на магнитофон и на пьезодинамик. Начальное состояние "0".
			(200)бит 7: включение двигателя магнитофона, "1" -- стоп, "0" -- пуск. Начальное состояние "1".
			    биты 0,1,3, не используются, "0".
			*/
			if (bByteOperation)
			{
				src &= 0377; // работаем с младшим байтом

				if (addr & 1)
				{
					src <<= 8; // работаем со старшим байтом
				}
			}

			Set177716RegTap(src);
			// В БК 2й разряд SEL1 фиксирует любую запись в этот регистр, взводя триггер D9.1 на неограниченное время, сбрасывается который любым чтением этого регистра.
			m_reg177716in |= 4;
			return true;
	}

	return false;
}


void CMotherBoard::RunOver()
{
	// Run one command with go over command
	register uint16_t pc = GetRON(CCPU::REGISTER::PC);
	register uint16_t instr = GetWordIndirect(pc);
	register uint16_t NextAddr = ADDRESS_NONE;

	if (m_pDebugger->IsInstructionOver(instr))
	{
		RunToAddr(pc + m_pDebugger->CalcInstructionLength(instr));
	}
    else if ((NextAddr = m_pDebugger->CalcNextAddr(pc)) != (uint16_t)ADDRESS_NONE)
	{
		RunToAddr(NextAddr);
	}
	else
	{
		RunInto();
	}
}


void CMotherBoard::RunToAddr(uint16_t addr)
{
	// Run all commands to address
	UnbreakCPU(addr);
}

void CMotherBoard::RunInto()
{
	// Run one command with go into command
	UnbreakCPU(GO_INTO);
}
#ifdef ENABLE_BACKTRACE
void CMotherBoard::StepBack()
{
    // Allow step back only if CPU is not running
    if(IsCPUBreaked())
        m_cpu.BT_StepBack();
}

void CMotherBoard::StepForward()
{
    // Allow step back only if CPU is not running
    if(IsCPUBreaked())
        m_cpu.BT_StepForward();
}


uint16_t CMotherBoard::GetPrevPC()
{
    return m_cpu.BT_GetPrevPC();
}
#endif

void CMotherBoard::RunOut()
{
	// Run all commands to function end
	CDebugger::InitOutMode();
	UnbreakCPU(GO_OUT);
}


void CMotherBoard::BreakCPU()
{
	m_bBreaked = true;      // включаем отладочную приостановку
	CDebugger::InitOutMode();

	if (m_pParent)
	{
        emit m_pParent->PostMessage(WM_CPU_DEBUGBREAK, 0);
	}
}

void CMotherBoard::UnbreakCPU(int nGoto)
{
	m_sTV.nGotoAddress = nGoto;
	m_bBreaked = false;     // отменяем отладочную приостановку
}


inline bool CMotherBoard::IsCPUBreaked()
{
	return m_bBreaked;
}

void CMotherBoard::RunCPU(bool bUnbreak)
{
	if (bUnbreak)
	{
		UnbreakCPU(ADDRESS_NONE);
	}

	// Set running flag
	m_bRunning = true;
}


void CMotherBoard::StopCPU(bool bUnbreak)
{
	m_bRunning = false;

	if (bUnbreak)
	{
		UnbreakCPU(ADDRESS_NONE); // отменяем отладочную приостановку, для случая, когда мы останавливаем
		// проц чтобы изменить конфигурацию, загрузить состояние или выйти из программы
	}

	Sleep(40); // !!!костыль, т.к. нету никакой синхронизации, после останова надо убедиться, что цикл в потоке
	// воспринял новое значение флага и не выполняет инструкций
}


inline bool CMotherBoard::IsCPURun()
{
	return m_bRunning;
}


//////////////////////////////////////////////////////////////////////
// перехват разных подпрограмм монитора, для их эмуляции
bool CMotherBoard::Interception()
{
	switch (GetRON(CCPU::REGISTER::PC) & 0177776)
	{
		case 0116256:
			return EmulateSaveTape();

		case 0116640:
			return EmulateLoadTape();
	}

	return false; // если ничего не выполнилось
}

int CMotherBoard::GetScreenPage()
{
	return 1;    // для БК10 всегда 1
}

// выход: true - ПЗУ успешно прочитано
//       false - ПЗУ не прочитано или не задано
bool CMotherBoard::LoadRomModule(int iniRomNameIndex, int bank)
{
	CString strName = g_Config.GetIniObj()->GetValueString(IDS_INI_SECTIONNAME_ROMMODULES, iniRomNameIndex, g_strEmptyUnit);

	if (strName == g_strEmptyUnit) // если там пусто
	{
		return false; // Там ПЗУ не задано, но это не ошибка
	}

    CString strPath = QDir(g_Config.m_strROMPath).filePath(strName);
	CFile file;

	if (file.Open(strPath, CFile::modeRead))
	{
		register auto len = static_cast<UINT>(file.GetLength());

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

	return false;
}


bool CMotherBoard::InitMemoryModules()
{
	m_ConfBKModel.nROMPresent = 0;

	if (LoadRomModule(IDS_INI_BK10_RE2_017_MONITOR, BRD_10_MON10_BNK))
	{
		m_ConfBKModel.nROMPresent |= (3 << BRD_10_MON10_BNK);
	}

	if (LoadRomModule(IDS_INI_BK10_RE2_106_BASIC1, BRD_10_BASIC10_1_BNK))
	{
		m_ConfBKModel.nROMPresent |= (3 << BRD_10_BASIC10_1_BNK);
	}

	if (LoadRomModule(IDS_INI_BK10_RE2_107_BASIC2, BRD_10_BASIC10_2_BNK))
	{
		m_ConfBKModel.nROMPresent |= (3 << BRD_10_BASIC10_2_BNK);
	}

	if (LoadRomModule(IDS_INI_BK10_RE2_108_BASIC3, BRD_10_REGISTERS_BNK))
	{
		m_ConfBKModel.nROMPresent |= (3 << BRD_10_REGISTERS_BNK);
	}

	// и проинициализируем карту памяти
	MemoryManager();
	return true;
}

void CMotherBoard::MemoryManager()
{
	for (int i = 0; i <= 017; ++i)
	{
		if (i < BRD_10_MON10_BNK)
		{
			m_MemoryMap[i].bReadable = true;
		}
		else
		{
			if (m_ConfBKModel.nROMPresent & (1 << i))
			{
				m_MemoryMap[i].bReadable = true;
			}
			else
			{
				m_MemoryMap[i].bReadable = false;
			}
		}

		m_MemoryMap[i].bWritable = (i < BRD_10_MON10_BNK) ? true : false;
		m_MemoryMap[i].nBank = i;
		m_MemoryMap[i].nOffset = i << 12;
		m_MemoryMap[i].nTimingCorrection = (i < BRD_10_MON10_BNK) ? RAM_TIMING_CORR_VALUE_D : ROM_TIMING_CORR_VALUE;
	}
}


bool CMotherBoard::RestoreState(CMSFManager &msf, QImage *hScreenshot)
{
	if (RestorePreview(msf, hScreenshot))
	{
		if (RestoreConfig(msf))
		{
			if (RestoreRegisters(msf))
			{
				if (RestoreMemoryMap(msf)) // необходимо делать строго перед работой с памятью.
				{
					if (RestoreMemory(msf))
					{
						return true;
					}
				}
			}
		}
	}

	return false;
}

bool CMotherBoard::RestoreConfig(CMSFManager &msf)
{
	if (msf.IsLoad())
	{
		msf.GetBlockConfig();
		MSF_FRAMEDATA framedata;

		if (msf.GetBlockFrameData(&framedata))
		{
			m_cpu.SetTimerSpeedInternal(framedata.nTimerSpeed, framedata.nTimerDiv);
			m_sTV.nVideoAddress = framedata.nVideoAddress & 0xffff; // видео адрес, младшие 6 бит - счётчик строк внутри строки
			m_sTV.bHgate = !!framedata.bHgate; // флаг отсчёта служебных видеоциклов в строке
			m_sTV.bVgate = !!framedata.bVgate; // флаг отсчёта служебных строк
			m_sTV.nVGateCounter = framedata.nVGateCounter; // дополнительный счётчик служебных строк
			m_sTV.nLineCounter = framedata.nLineCounter & 0377; // счётчик видео строк
			m_sTV.nCPUTicks = framedata.nCPUTicks;
			m_sTV.fMediaTicks = framedata.fMediaTicks;
			m_sTV.fMemoryTicks = framedata.fMemoryTicks;
			m_sTV.fFDDTicks = framedata.fFDDTicks;

			// вот это обязательно нужно сделать.
			// а то переинициализация фрейма делается опять не в то время, когда надо
			// с ещё старыми параметрами, а потом, с новыми уже не делается.
			m_nCPUFreq_prev = 0;
			FrameParam();
		}
		else
		{
			m_sTV.clear();
		}

		return true; // если конфиг не прочёлся, значит этого блока просто нет. ничего страшного
	}
	else
	{
		bool bRet = msf.SetBlockConfig();
		MSF_FRAMEDATA framedata;
		m_cpu.GetTimerSpeedInternal(framedata.nTimerSpeed, framedata.nTimerDiv);
		framedata.nVideoAddress = m_sTV.nVideoAddress; // видео адрес, младшие 6 бит - счётчик строк внутри строки
		framedata.bHgate = m_sTV.bHgate ? 1 : 0; // флаг отсчёта служебных видеоциклов в строке
		framedata.bVgate = m_sTV.bVgate ? 1 : 0; // флаг отсчёта служебных строк
		framedata.nVGateCounter = m_sTV.nVGateCounter; // дополнительный счётчик служебных строк
		framedata.nLineCounter = m_sTV.nLineCounter; // счётчик видео строк
		framedata.nCPUTicks = m_sTV.nCPUTicks;
		framedata.fMediaTicks = m_sTV.fMediaTicks;
		framedata.fMemoryTicks = m_sTV.fMemoryTicks;
		framedata.fFDDTicks = m_sTV.fFDDTicks;
		return  bRet && msf.SetBlockFrameData(&framedata);
	}
}


bool CMotherBoard::RestoreRegisters(CMSFManager &msf)
{
	MSF_CPU_REGISTERS cpu_reg;
	MSF_PORT_REGS port_regs;

	if (msf.IsLoad())
	{
		if (!msf.GetBlockCPURegisters(&cpu_reg))
		{
			return false;
		}

		SetRON(CCPU::REGISTER::R0, cpu_reg.r0);
		SetRON(CCPU::REGISTER::R1, cpu_reg.r1);
		SetRON(CCPU::REGISTER::R2, cpu_reg.r2);
		SetRON(CCPU::REGISTER::R3, cpu_reg.r3);
		SetRON(CCPU::REGISTER::R4, cpu_reg.r4);
		SetRON(CCPU::REGISTER::R5, cpu_reg.r5);
		SetRON(CCPU::REGISTER::SP, cpu_reg.sp);
		SetRON(CCPU::REGISTER::PC, cpu_reg.pc);
		SetPSW(cpu_reg.psw);

		if (msf.GetVersion() >= MSF_VERSION_MINIMAL)
		{
			if (!msf.GetBlockPortRegs(&port_regs))
			{
				return false;
			}

			m_reg177660 = port_regs.p0177660;
			m_reg177662in = port_regs.p0177662_in;
			m_reg177662out = port_regs.p0177662_out;
			m_reg177664 = port_regs.p0177664;
			m_reg177714in = port_regs.p0177714_in;
			m_reg177714out = port_regs.p0177714_out;
			m_reg177716in = port_regs.p0177716_in;
			m_reg177716out_tap = port_regs.p0177716_out_tap;
			m_reg177716out_mem = port_regs.p0177716_out_mem;
			m_cpu.SetSysRegsInternal(0177700, port_regs.p0177700);
			m_cpu.SetSysRegsInternal(0177702, port_regs.p0177702);
			m_cpu.SetSysRegsInternal(0177704, port_regs.p0177704);
			m_cpu.SetSysRegsInternal(0177706, port_regs.p0177706);
			m_cpu.SetSysRegsInternal(0177710, port_regs.p0177710);
			m_cpu.SetSysRegsInternal(0177712, port_regs.p0177712);
		}
		else
		{
			return false;
		}
	}
	else
	{
		cpu_reg.r0 = GetRON(CCPU::REGISTER::R0);
		cpu_reg.r1 = GetRON(CCPU::REGISTER::R1);
		cpu_reg.r2 = GetRON(CCPU::REGISTER::R2);
		cpu_reg.r3 = GetRON(CCPU::REGISTER::R3);
		cpu_reg.r4 = GetRON(CCPU::REGISTER::R4);
		cpu_reg.r5 = GetRON(CCPU::REGISTER::R5);
		cpu_reg.sp = GetRON(CCPU::REGISTER::SP);
		cpu_reg.pc = GetRON(CCPU::REGISTER::PC);
		cpu_reg.psw = GetPSW();

		if (!msf.SetBlockCPURegisters(&cpu_reg))
		{
			return false;
		}

		port_regs.p0177660 = m_reg177660;
		port_regs.p0177662_in = m_reg177662in;
		port_regs.p0177662_out = m_reg177662out;
		port_regs.p0177664 = m_reg177664;
		port_regs.p0177714_in = m_reg177714in;
		port_regs.p0177714_out = m_reg177714out;
		port_regs.p0177716_in = m_reg177716in;
		port_regs.p0177716_out_tap = m_reg177716out_tap;
		port_regs.p0177716_out_mem = m_reg177716out_mem;
		port_regs.p0177700 = m_cpu.GetSysRegs(0177700);
		port_regs.p0177702 = m_cpu.GetSysRegs(0177702);
		port_regs.p0177704 = m_cpu.GetSysRegs(0177704);
		port_regs.p0177706 = m_cpu.GetSysRegs(0177706);
		port_regs.p0177710 = m_cpu.GetSysRegs(0177710);
		port_regs.p0177712 = m_cpu.GetSysRegs(0177712);

		if (!msf.SetBlockPortRegs(&port_regs))
		{
			return false;
		}
	}

	return true;
}


bool CMotherBoard::RestoreMemory(CMSFManager &msf)
{
	if (msf.IsLoad())
	{
		return msf.GetBlockBaseMemory(m_pMemory);
	}
	else
	{
		return msf.SetBlockBaseMemory(m_pMemory);
	}
}

bool CMotherBoard::RestoreMemoryMap(CMSFManager &msf)
{
	if (msf.IsLoad())
	{
		return msf.GetBlockMemMap(m_MemoryMap, &m_ConfBKModel);
	}
	else
	{
		return msf.SetBlockMemMap(m_MemoryMap, &m_ConfBKModel);
	}
}


bool CMotherBoard::RestorePreview(CMSFManager &msf, QImage *hScreenshot)
{
	bool bRet = true;

	if (msf.IsSave())
	{
		ASSERT(hScreenshot);

        if (hScreenshot)
        {
//			auto hBm = (HBITMAP)CopyImage(hScreenshot, IMAGE_BITMAP, 256, 256, LR_CREATEDIBSECTION | LR_COPYDELETEORG);

//			if (hBm)
//			{
//				bRet = msf.SetBlockPreview(hBm);
//				DeleteObject(hBm);
//			}

//			DeleteObject(hScreenshot);
        }
        delete hScreenshot;
	}

	return bRet;
}



void CMotherBoard::StopTimerThread()
{
//	MSG msg;

	if (!m_bKillTimerEvent)
	{
		m_bKillTimerEvent = true;
		StopCPU();
//		BOOL bRet;

		while (m_bKillTimerEvent) // ждём завершения потока
		{
//			// берём на себя трансляцию сообщений, чтобы UI не зависало
//			// этот костыль уже не нужен. Но на всякий случай, пусть останется, он безвреден.
//			if (bRet = GetMessage(&msg, m_pParent->GetSafeHwnd(), 0, 0))
//			{
//				if (bRet != -1)
//				{
//					TranslateMessage(&msg);
//					DispatchMessage(&msg);
//				}
//				else
//				{
//					break;
//				}
//			}

			Sleep(10);
		}

		// поток остановился, и можно продолжать остальные действия по остановке
	}
}

bool CMotherBoard::StartTimerThread()
{
	m_TimerThread = std::thread(&CMotherBoard::TimerThreadFunc, this);

	if (m_TimerThread.joinable())
	{
		m_TimerThread.detach();
		return true;
	}

	g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	return false;
}


void CMotherBoard::FrameParam()
{
	if (m_nCPUFreq_prev != m_nCPUFreq) // если частота не менялась, пересчёта не делать
	{
		m_nCPUFreq_prev = m_nCPUFreq;
		register auto fCPUfreq = double(m_nCPUFreq);
		m_sTV.fCpuTickTime = 1.0 / fCPUfreq; // длительность одного такта в секундах
		m_sTV.fMemory_Mod = fCPUfreq / 750000.0;      // коэффициент пересчёта ТЧ проца в количество обращений к видеопамяти за фрейм
		m_sTV.fFDD_Mod = fCPUfreq / 15625.0;  // 15625 = 5 оборотов в сек, в одном обороте 3125 слов,
		// норма 15625.0, больше - быстрее вращение, меньше - медленнее
		m_sTV.fMedia_Mod = fCPUfreq / double(g_Config.m_nSoundSampleRate); // коэффициент пересчёта тактов процессора в медиа такты
		// означает - через сколько тактов процессора надо вставлять 1 медиа такт, т.к. число не целое, то
		// получается приблизительно.
        // Insert delays for 80% of speed
        m_sTV.nBoard_Mod = (long long)800000000ll * (long long)m_sTV.nBoardTicksMax/(long long)m_nCPUFreq;
	}
}


void CMotherBoard::TimerThreadFunc()
{
	uint16_t nPreviousPC = ADDRESS_NONE;    // предыдущее значение регистра РС
	// типы nPreviousPC и m_sTV.nGotoAddress не должны совпадать, иначе будет всегда срабатывать условие отладочного
	// останова, даже если нам этого не надо

    struct timespec timeCurrent;
    struct timespec timeBoard;

    // Initialize board clock so we could synchronize it with PC's system clock
    clock_gettime(CLOCK_REALTIME, &timeBoard);
    timeBoard.tv_nsec += m_sTV.nBoard_Mod;
    if (timeBoard.tv_nsec >= 1000000000) {
          timeBoard.tv_nsec -= 1000000000;
          timeBoard.tv_sec++;
    }

    m_sTV.nBoardTicks = 0;

	do
	{
		// тут для отладочных целей приостанавливаем цикл.
		while (IsCPUBreaked()) // пока процессор приостановлен - ждём
		{
			if (!IsCPURun())    // если решили всё остановить,
			{
				break;          // прекращаем ждать
			}

			DrawDebugScreen();  // но продолжаем обновлять экран
			Sleep(20);          // со стандартной частотой примерно 50Гц
		}

		if (IsCPURun()) // если процессор работает, выполняем эмуляцию
		{
			// Выполняем набор инструкций

			// Если время пришло, выполняем текущую инструкцию
			if (--m_sTV.nCPUTicks <= 0)
			{
				try
				{
					// Выполняем одну инструкцию, возвращаем время выполнения одной инструкции в тактах.
					m_sTV.nCPUTicks = m_cpu.TranslateInstruction();
					// Сохраняем текущее значение PC для отладки
					nPreviousPC = m_cpu.GetRON(CCPU::REGISTER::PC);
#ifdef ENABLE_TRACE
                    if(m_pDebugger->IsTraceEnabled()) {
                        if(m_cpu.GetTraceFlags() == CCPU::TRACE_FLAGS::isJump) {
                            m_pDebugger->m_SymTable.AddSymbolIfNotExist(nPreviousPC, "j_" + CString::number(nPreviousPC, 8));
                        } else if(m_cpu.GetTraceFlags() == CCPU::TRACE_FLAGS::isCall) {
                            m_pDebugger->m_SymTable.AddSymbolIfNotExist(nPreviousPC, "c_" + CString::number(nPreviousPC, 8));
                        } else if(m_cpu.GetTraceFlags() == CCPU::TRACE_FLAGS::isLoop) {
                            m_pDebugger->m_SymTable.AddSymbolIfNotExist(nPreviousPC, "l_" + CString::number(nPreviousPC, 8));
                        }
                    }
#endif
					Interception();  // перехват разных подпрограмм монитора, для их эмуляции
				}
				catch (CExceptionHalt &halt)
				{
					// BK Violation exception. Например запись в ПЗУ или чтение из несуществующей памяти
					if (g_Config.m_bAskForBreak)
					{
						BreakCPU();  // Останавливаем CPU для отладки
						CString strMessage;
                        strMessage.Format(IDS_ERRMSG_ROM, halt.m_addr, nPreviousPC, halt.m_info.toLocal8Bit().data());
                        int answer = g_BKMsgBox.Show(strMessage, MB_ABORTRETRYIGNORE | MB_DEFBUTTON2 | MB_ICONSTOP);

						switch (answer)
						{
							case IDIGNORE:  // если выбрали "игнорировать"
								g_Config.m_bAskForBreak = false; // то снимаем галку, и больше не вызываем диалог

							// и выполним всё, что должно выполняться для "повтор"
							case IDRETRY: // если выбрали "повтор" - то просто продолжить выполнение и посмотреть, что будет дальше
								UnbreakCPU(ADDRESS_NONE); // обратно запускаем CPU
								m_cpu.ReplyError();  // Делаем прер. по вектору 4(halt) в следующем цикле
								break;

							case IDABORT: // если выбрали "прервать" - остаёмся в отладочном останове
							default:
								break;
						}
					}
					else
					{
						m_sTV.nCPUTicks = 64;
#ifdef ENABLE_BACKTRACE
                        m_cpu.BT_Push();
#endif
						m_cpu.ReplyError();  // Делаем прер. по вектору 4(halt) в следующем цикле
					}

					nPreviousPC = ADDRESS_NONE;
				}
				catch (...)
				{
					// Любое другое исключение. Неизвестное исключение или ошибка доступа к памяти WINDOWS.
					// Эта ошибка может быть неизвестной ошибкой BK или ошибкой эмулятора
					CString strMessage(MAKEINTRESOURCE(IDS_ERRMSG_INTERNAL));
					g_BKMsgBox.Show(strMessage, MB_OK | MB_ICONSTOP);
					BreakCPU();
					StopCPU(false);
					break; // Прекращаем выполнять инструкции
				}

				if (
				    m_pDebugger->GetDebugPCBreak(nPreviousPC) || // Спросим отладчик на счёт условий остановки. Если ВОИСТИНУ останов
				    (m_sTV.nGotoAddress == nPreviousPC) || // Если останов на адресе где стоит отладочный курсор
				    (m_sTV.nGotoAddress == GO_INTO) || // Отладочный останов если только одиночный шаг
                    (m_sTV.nGotoAddress == GO_OUT && CDebugger::IsInstructionOut(m_cpu.GetCurrentInstruction())) || // Отладочный останов если команда выхода из п/п
                    (m_cpu.GetInterruptFlag() & g_Config.m_nSysBreakConfig)  // Stop on HW interrupt if enabled
				)
				{
					BreakCPU();
				}
			}

			if (--m_sTV.fMemoryTicks <= 0.0)
			{
				do
				{
					Make_One_Screen_Cycle();  // тут выполняются циклы экрана
					m_sTV.fMemoryTicks += m_sTV.fMemory_Mod;
				}
				while (m_sTV.fMemoryTicks < 1.0);
			}

			m_pSpeaker->RCFilterLF(m_sTV.fCpuTickTime); // эмуляция конденсатора на выходе линейного входа.

			if (--m_sTV.fMediaTicks <= 0.0)
			{
				do
				{
					MediaTick();  // тут делается звучание всех устройств и обработка прочих устройств
					m_sTV.fMediaTicks += m_sTV.fMedia_Mod;
				}
				while (m_sTV.fMediaTicks < 1.0);
			}

			if (--m_sTV.fFDDTicks <= 0.0)
			{
				do
				{
					m_fdd.Periodic();     // Вращаем диск на одно слово на дорожке
					m_sTV.fFDDTicks += m_sTV.fFDD_Mod;
				}
				while (m_sTV.fFDDTicks < 1.0);
			}

            if (--m_sTV.nBoardTicks <= 0 ) {
                // clock_nanosleep() doesn't work correctly
                // Using simple busy wait
//                do {
//                    clock_gettime(CLOCK_REALTIME, &timeCurrent);
//                } while(timeBoard.tv_sec > timeCurrent.tv_sec || timeBoard.tv_nsec > timeCurrent.tv_nsec);

                m_sTV.nBoardTicks = m_sTV.nBoardTicksMax;

                // Calculate board clock after SLEEP_COUNT cycles
                timeBoard.tv_nsec += m_sTV.nBoard_Mod;
                if (timeBoard.tv_nsec >= 1000000000) {
                    timeBoard.tv_nsec -= 1000000000;
                    timeBoard.tv_sec++;
                }
            }
        }
		else
		{
			Sleep(10);
		}

	}
	while (!m_bKillTimerEvent);       // пока не придёт событие остановки

	m_bKillTimerEvent = false;
}

void CMotherBoard::SetMTC(int mtc)
{
	// переменные для медиа системы
	m_sTV.nMediaTicksPerFrame = mtc; // длина буфера в сэмплах
	SAFE_DELETE_ARRAY(m_sTV.pSoundBuffer);
	m_sTV.pSoundBuffer = new SAMPLE_INT[m_sTV.nMediaTicksPerFrame * BUFFER_CHANNELS];

	if (!m_sTV.pSoundBuffer)
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}
}

void CMotherBoard::MediaTick()
{
	ASSERT(m_pSpeaker);

	// обработка медиатактов
	if (m_sTV.nMediaTickCount == 0)
	{
		// Получаем сохранённые звуковые данные, это чтобы слышать звучание с кассеты
		m_pParent->GetTapePtr()->PlayWaveGetBuffer(m_sTV.pSoundBuffer, m_sTV.nMediaTicksPerFrame); // Берём данные с ленты -> m_pSoundBuffer
		// Посылаем данные с ленты в 177716 <- m_sTV.pSoundBuffer
		m_pSpeaker->ReceiveTapeBuffer(m_sTV.pSoundBuffer, m_sTV.nMediaTicksPerFrame);
		m_sTV.nBufPos = 0;
	}

	// осуществляем приём данных с магнитофона, так он будет работать даже если спикер выключен
	if (m_pSpeaker->GetTapeSample())
	{
		m_reg177716in |= 040;
	}
	else
	{
		m_reg177716in &= ~040;
	}

	register SAMPLE_INT fL = 0.0, fR = 0.0;
	m_pSound->SoundGen_ResetSample(m_sTV.pSoundBuffer[m_sTV.nBufPos], m_sTV.pSoundBuffer[m_sTV.nBufPos + 1]); // задаём начальные уровни для микширования

	// Берём звуки со спикера BK
	if (m_pSpeaker->IsSoundEnabled())
	{
		m_pSpeaker->GetSample(fL, fR);
		m_pSound->SoundGen_MixSample(fL, fR); // микшируем сэмплы внутри m_pSound
	}

	// Берём данные с ковокса
	if (m_pCovox->IsSoundEnabled())
	{
		m_pCovox->GetSample(fL, fR);
		m_pSound->SoundGen_MixSample(fL, fR);
	}

	if (m_pMenestrel->IsSoundEnabled())
	{
		m_pMenestrel->GetSample(fL, fR);
		m_pSound->SoundGen_MixSample(fL, fR);
	}

	// Берём данные с ay8910
	if (m_pAY8910->IsSoundEnabled())
	{
		m_pAY8910->GetSample(fL, fR);
		m_pSound->SoundGen_MixSample(fL, fR);
	}

	m_pSound->SoundGen_FeedDAC_Mixer(fL, fR); // получаем сюда смикшированное, а там - осуществляем звучание
	m_sTV.pSoundBuffer[m_sTV.nBufPos++] = fL; // это нужно в основном для осциллографа
	m_sTV.pSoundBuffer[m_sTV.nBufPos++] = fR; // и для экспорта вывода в WAV(TAP)

	if (++m_sTV.nMediaTickCount >= m_sTV.nMediaTicksPerFrame)
	{
		m_sTV.nMediaTickCount = 0;
		// копируем содержимое буфера осциллографу
//		m_pParent->GetOscillatorViewPtr()->GetobjPtr()->FillBuffer(m_sTV.pSoundBuffer);
		// Отображаем копию буфера на осциллографе в фоне
        emit m_pParent->PostMessage(WM_OSC_DRAW);
		// Отправляем их на ленту <- m_sTV.pSoundBuffer
		m_pParent->GetTapePtr()->RecordWaveGetBuffer(m_sTV.pSoundBuffer, m_sTV.nMediaTicksPerFrame);
	}
}


// это вариант точного алгоритма из VP1-037
void CMotherBoard::Make_One_Screen_Cycle()
{
	register uint16_t dww = m_sTV.nVideoAddress & 076; // счётчик слов внутри строки
	register uint16_t dwa = m_sTV.nVideoAddress & 037700; // адрес строки

	if (!(m_sTV.bVgate || m_sTV.bHgate))
	{
		m_pParent->GetScreen()->SetExtendedMode(!(m_reg177664 & 01000));
                register uint32_t nScrAddr = ((GetScreenPage())) << 14;
		register uint8_t *nScr = m_pMemory + nScrAddr + m_sTV.nVideoAddress;
		m_pParent->GetScreen()->PrepareScreenLineWordRGB32(m_sTV.nLineCounter, dww, *reinterpret_cast<uint16_t *>(nScr));
	}

	dww += 2; // переходим к следующему слову

	if (m_sTV.bHgate) // считаем служебное поле в строке
	{
		if (dww == 040) // отсчитали 16. служебных слов
		{
			dww = 0;
			m_sTV.bHgate = false;
		}
	}
	else if (dww == 0100)
	{
		// если отсчитали 32. слова, нужно теперь считать служебные
		dww = 0;
		m_sTV.bHgate = true;
		register int dwl = (m_sTV.nLineCounter + 1) & 0377; // Счётчик строк

		if (m_sTV.bVgate) // считаем служебные
		{
			if (--m_sTV.nVGateCounter <= 0) // отсчитали 64 служебные строки
			{
				dwl = 0;  // снова начнём считать информационные
				m_sTV.bVgate = false;
			}

			// когда считаем служебные строки, выбираем из них 353..350
			if ((m_sTV.nVGateCounter & 0374) == 050)    // if (050 <= m_nVGateCounter && m_nVGateCounter <= 053)
			{
				// если строки 351, 350
				if (!(m_sTV.nVGateCounter & 2))         // if (050 == m_nVGateCounter || m_nVGateCounter == 051)
				{
					// применяем новое смещение
					dwa = (m_reg177664 & 0377) << 6; // VA[13:6] <= RA[7:0];
				}

				// при этом на новую строку не переходим. в одной крутимся.
			}
			else
			{
				// переход на следующую строку
				dwa += 0100; // VA[13:6] <= VA[13:6] + 8'b00000001;
			}
		}
		else
		{
			if (dwl == 0)
			{
				// если отсчитали 256 информационных строк, нужно теперь считать служебные.
				m_sTV.bVgate = true;
				m_sTV.nVGateCounter = 64;

				if (!(m_reg177662out & 040000)) // если бит 14 установлен, таймер не работает.
				{
					m_cpu.TickIRQ2();

					if (m_pAY8910)
					{
						// для лога дампа регистров AY
						m_pAY8910->log_timerTick();
					}
				}

                emit m_pParent->SendMessage(WM_SCR_DRAW);
			}

			// переход на следующую строку
			dwa += 0100; // VA[13:6] <= VA[13:6] + 8'b00000001;
		}

		m_sTV.nLineCounter = dwl;
	}

	m_sTV.nVideoAddress = (dwa | dww) & 037776;
}


// специально для отладки, рисуем экран старым методом.
// при этом новый метод не отключается и поэтому возможны разного рода казусы.
void CMotherBoard::DrawDebugScreen()
{
	if (m_pParent)
	{
		register int nScrAddr = GetScreenPage() << 14;
		m_pParent->PostMessage(WM_SCR_DEBUGDRAW, WPARAM(nScrAddr));
	}
}

//constexpr auto BK_NAMELENGTH = 16;   // Максимальная длина имени файла на БК - 16 байтов
constexpr auto BK_NAMELENGTH = 10;   // Стандартная длина имени файла на БК 0010 - 10 байтов
constexpr auto BK_EMT36BP = 0320;
constexpr auto BK_EMT36BP_ERRADDR = 0301;
constexpr auto BK_EMT36BP_CRCADDR = 0312;
constexpr auto BK_EMT36BP_ADDRESS = 2;
constexpr auto BK_EMT36BP_LENGTH = 4;
constexpr auto BK_EMT36BP_NAME = 6;
constexpr auto BK_EMT36BP_FOUND_ADDRESS = BK_EMT36BP_NAME + BK_NAMELENGTH;
constexpr auto BK_EMT36BP_FOUND_LENGTH = BK_EMT36BP_FOUND_ADDRESS + 2;
constexpr auto BK_EMT36BP_FOUND_NAME = BK_EMT36BP_FOUND_LENGTH + 2;

// для БК10.
bool CMotherBoard::EmulateLoadTape()
{
	CString strBinExt;
	strBinExt.LoadString(IDS_FILEEXT_BINARY);
	/*
	 * Известные косяки: 1) для бейсиковских бин файлов удаляет расширение .bin, как исправить
	 * непонятно, потому что никак не узнать, что мы загружаем именно бейсиковский бин файл,
	 * а не какой-либо другой. То же самое и с любыми другими файлами, по задумке авторов,
	 * имеющими расширение .bin, оно как правило удаляется.
	 * 2) при загрузке файла с автозапуском, автозапуск делается из ячейки на слово выше, чем в реальности.
	 * Это может быть критично только для очень хитрых программ.
	 **/
	bool bFileSelect = false; // что делать после диалога выбора
	bool bCancelSelectFlag = false; // флаг для усложнения алгоритма
	bool bError = false;
	bool bIsDrop = false;

	// если включена эмуляция и по этому адресу действительно ПЗУ монитора БК10
	if (g_Config.m_bEmulateLoadTape && ((GetWord(0116206) == 04767) && (GetWord(0116210) == 0426)))
	{
		uint16_t fileAddr = 0;
		uint16_t fileSize = 0;
		uint16_t abp = BK_EMT36BP;
		CString strFileName;
		// Внутренняя загрузка на БК
		uint8_t bkName[BK_NAMELENGTH] = { 0 };  // Максимальная длина имени файла на БК - BK_NAMELENGTH байтов
		uint8_t bkFoundName[BK_NAMELENGTH] = { 0 };

		if (m_pParent->GetStrBinFileName()->IsEmpty())
		{
			// если загружаем не через драг-н-дроп, то действуем как обычно
			abp = GetRON(CCPU::REGISTER::R1);       // получим адрес блока параметров из R1 (BK emt 36)
			fileAddr = GetWord(abp + BK_EMT36BP_ADDRESS);    // второе слово - адрес загрузки/сохранения
			fileSize = GetWord(abp + BK_EMT36BP_LENGTH);    // третье слово - длина файла (для загрузки может быть 0)

			// Подбираем 16 байтовое имя файла из блока параметров
			for (uint16_t c = 0; c < BK_NAMELENGTH; ++c)
			{
				bkName[c] = GetByte(abp + BK_EMT36BP_NAME + c);
			}

			strFileName = BKToUNICODE(bkName, BK_NAMELENGTH); // тут надо перекодировать  имя файла из кои8 в unicode
			strFileName.Trim(); // удаляем пробелы в конце файла, а в середине - оставляем

//			if (!strFileName.IsEmpty()) // если имя файла не пустое
//			{
//				strFileName += strBinExt; // добавляем стандартное расширение для бин файлов,
//				// чтобы не рушить логику следующих проверок
//			}
		}
		else
		{
			// если загружаем через драг-н-дроп, то берём имя оттуда
			strFileName = *m_pParent->GetStrBinFileName();
			m_pParent->GetStrBinFileName()->Empty();
			bIsDrop = true;
		}

		if (strFileName.SpanExcluding(_T(" ")) == _T(""))
		{
			// Если имя пустое - то покажем диалог выбора файла.
			bFileSelect = false;
l_SelectFile:
			// Запомним текущую директорию
            CString strCurDir = ::GetCurrentDirectory();
			::SetCurrentDirectory(g_Config.m_strBinPath);
			CString strFilterBin(MAKEINTRESOURCE(IDS_FILEFILTER_BIN));
//            QFileDialog dlg(nullptr, "", g_Config.m_strBinPath, strFilterBin);
//			                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
//			                strFilterBin, m_pParent->GetScreen()->GetBackgroundWindow());
			// Зададим начальной директорией директорию с Bin файлами
//			dlg.GetOFN().lpstrInitialDir = g_Config.m_strBinPath;

//            int res = dlg.exec();

            strFileName = FileDialogCaller().getOpenFileName(nullptr, "", g_Config.m_strBinPath, strFilterBin);

            if (strFileName.size() == 0)
			{
				// Если нажали Отмену, установим ошибку во втором байте блока параметров
				SetByte(BK_EMT36BP_ERRADDR, 4);
				bError = true; // случилась ошибка
                bCancelSelectFlag = true;
			}
			else
			{
                CString strBkFileName = ::GetFileName(strFileName);
//                g_Config.m_strBinPath = ::GetFilePath(dlg.selectedFiles()[0]);
//				strFileName = dlg.GetPathName(); // вот выбранный файл
//                strFileName = ::GetFilePath(dlg.selectedFiles().first()); // вот выбранный файл
                // имя файла надо бы как-то поместить в 0352..0372 иначе некоторые глюки наблюдаются
                UNICODEtoBK(strBkFileName, bkFoundName, BK_NAMELENGTH, true); // вот из этого массива будем потом помещать

				if (bFileSelect)
				{
					// тут надо проверить тот ли файл нам подсовывают.
                    CString strFound = ::GetFileName(strFileName);
					CString strFindEx = BKToUNICODE(bkName, BK_NAMELENGTH); // с расширением
					CString strFind = ::GetFileTitle(strFindEx); // без расширения

					if (!bIsDrop) // только если не дроп. там не с чем сравнивать
					{
						if (strFind.CompareNoCase(strFound) != 0 && strFindEx.CompareNoCase(strFound) != 0)
						{
							int result = g_BKMsgBox.Show(IDS_BK_ERROR_WRONGFILE, MB_ICONWARNING | MB_YESNO | MB_DEFBUTTON2);

							if (result == IDYES)
							{
								goto l_SelectFile;
							}
							else
							{
								SetByte(BK_EMT36BP_ERRADDR, 4);
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
//			CFileStatus fs; // получим информацию о файле - таким образом проверяется существует он или нет
			SetSafeName(strFileName); // перекодируем небезопасные символы на безопасные

			// сейчас узнаем, нужно ли нам добавлять расширение .bin, или наоборот, удалять
			if (::GetFileExt(strFileName).CompareNoCase(strBinExt) == 0)
			{
				// у файла уже есть расширение бин
                if (!QFileInfo::exists(strCurrentPath + strFileName)) // если нету файла с расширением.
				{
					CString str = ::GetFileTitle(strFileName); // удаляем расширение

                    if (QFileInfo::exists(strCurrentPath + str)) // если есть файл без расширения
					{
						strFileName = str; // оставим файл без расширения
					}
					else
					{
						// нет файла ни с расширением, ни без расширения
						SetByte(BK_EMT36BP_ERRADDR, 1);
						bError = true;
					}
				}
			}
			else
			{
				// у файла нету расширения бин
                if (!QFileInfo::exists(QDir(strCurrentPath).filePath(strFileName))) // если нету файла без расширения
				{
					CString str = strFileName + strBinExt; // добавляем стандартное расширение для бин файлов.

                    if (QFileInfo::exists(QDir(strCurrentPath).filePath(str)))  // если есть файл с расширением
					{
						strFileName = str;
					}
					else
					{
						// нет файла ни с расширением, ни без расширения
						SetByte(BK_EMT36BP_ERRADDR, 1);
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
			uint16_t loadAddr = 01000;
			uint16_t loadLen = 0;
			uint16_t loadcrc = 0;

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

				if (!bError)
				{
					SetWord(abp + BK_EMT36BP_FOUND_ADDRESS, readAddr);
					SetWord(abp + BK_EMT36BP_FOUND_LENGTH, readSize);

					if (bkFoundName[0])
					{
						// копируем прочитанное имя файла
						for (uint16_t i = 0; i < BK_NAMELENGTH; ++i)
						{
							SetByte(abp + BK_EMT36BP_FOUND_NAME + i, bkFoundName[i]);
						}
					}
					else
					{
						// копируем прочитанное имя файла
						for (uint16_t i = 0; i < BK_NAMELENGTH; ++i)
						{
							SetByte(abp + BK_EMT36BP_FOUND_NAME + i, bkName[i]);
						}
					}

					if (fileAddr == 0)
					{
						fileAddr = readAddr;
					}

//                  if (fileAddr < 01000)
//                  {
//                      // если файл с автозапуском, на всякий случай остановим скрипт
//                      // если он выполнялся
//                      m_pParent->GetScriptRunnerPtr()->StopScript();
//                  }
					SetWord(0264, fileAddr); loadAddr = fileAddr;
					SetWord(0266, readSize); loadLen = readSize;
					DWORD cs = 0; // подсчитаем контрольную сумму

					// Загрузка по адресу fileAddr
					for (int i = 0; i < readSize; ++i)
					{
						uint8_t val;
						file.Read(&val, sizeof(val));
						SetByte(fileAddr++, val);
						cs += uint16_t(val);

						if (cs & 0xffff0000)
						{
							cs++;
							cs &= 0xffff;
						}
					}

					uint16_t crc;

					if (bIsCRC && file.Read(&crc, sizeof(crc)) == sizeof(uint16_t))
					{
						if (crc != LOWORD(cs))
						{
							SetByte(BK_EMT36BP_ERRADDR, 2);
							cs = crc;
						}
					}

					// а иначе, мы не знаем какая должна быть КС. поэтому считаем, что файл априори верный
					file.Close();
					// Заполняем системные ячейки, как это делает emt 36
					loadcrc = LOWORD(cs);
					SetWord(BK_EMT36BP_CRCADDR, loadcrc); // сохраним контрольную сумму
				}
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
						SetByte(BK_EMT36BP_ERRADDR, 0);
						bFileSelect = true; // включим проверку на неподходящее имя.
						goto l_SelectFile;

					// если отмена - просто выходим с заданным кодом ошибки
					case IDYES:
						// если хотим остановиться - зададим останов.
						BreakCPU();
						SetByte(BK_EMT36BP_ERRADDR, 4);
						break;
				}
			}

			if (loadAddr < 0750)
			{
				// Помещаем в R1 последний адрес, куда производилось чтение, как в emt 36
				SetRON(CCPU::REGISTER::R0, 0);
				SetRON(CCPU::REGISTER::R1, loadAddr + loadLen);
				SetRON(CCPU::REGISTER::R2, 0);
				SetRON(CCPU::REGISTER::R3, 0177716);
				SetRON(CCPU::REGISTER::R5, 040);
				SetRON(CCPU::REGISTER::PC, 0117374); // выходим туда.
			}
			else
			{
				SetRON(CCPU::REGISTER::R0, loadcrc);
				SetRON(CCPU::REGISTER::R0, 0314);
				SetRON(CCPU::REGISTER::R3, 0177716);
				SetRON(CCPU::REGISTER::R4, 0);
				// Помещаем в R5 последний адрес, куда производилось чтение, как в emt 36
				SetRON(CCPU::REGISTER::R5, loadAddr + loadLen);
				SetRON(CCPU::REGISTER::PC, 0116710); // выходим туда.
			}
		}
		else
		{
			SetRON(CCPU::REGISTER::PC, 0116214); // выходим туда.
		}

		// Refresh keyboard
		m_pParent->SendMessage(WM_RESET_KBD_MANAGER); // и почистим индикацию управляющих клавиш в статусбаре
		return true; // сэмулировали
	}

	return false; // не эмулируем
}

bool CMotherBoard::EmulateSaveTape()
{
	CString strBinExt;
	strBinExt.LoadString(IDS_FILEEXT_BINARY);
	bool bError = false;

	// если включена эмуляция и по этому адресу действительно ПЗУ монитора БК10
	if (g_Config.m_bEmulateSaveTape && ((GetWord(0116170) == 04767) && (GetWord(0116172) == 062)))
	{
		// получим адрес блока параметров из R1 (BK emt 36)
		uint16_t abp = GetRON(CCPU::REGISTER::R1);
		uint16_t fileAddr = GetWord(abp + BK_EMT36BP_ADDRESS);  // второе слово - адрес загрузки/сохранения
		uint16_t fileSize = GetWord(abp + BK_EMT36BP_LENGTH);  // третье слово - длина файла (для загрузки может быть 0)

		if (fileSize)
		{
			uint8_t bkName[BK_NAMELENGTH];   // Максимальная длина имени файла на БК - 16 байтов

			// Подбираем 16 байтовое имя файла из блока параметров
			for (uint16_t c = 0; c < BK_NAMELENGTH; ++c)
			{
				bkName[c] = GetByte(abp + BK_EMT36BP_NAME + c);
			}

			CString strFileName = BKToUNICODE(bkName, BK_NAMELENGTH); // тут надо перекодировать  имя файла из кои8 в unicode
			strFileName.Trim(); // удаляем пробелы в конце файла, а в середине - оставляем

			// Если имя пустое
			if (strFileName.SpanExcluding(_T(" ")) == _T(""))
			{
				// Покажем диалог сохранения
//				CFileDialog dlg(FALSE, nullptr, nullptr,
//				                OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER,
//				                nullptr, m_pParent->GetScreen()->GetBackgroundWindow());
//				dlg.GetOFN().lpstrInitialDir = g_Config.m_strBinPath;

                strFileName = FileDialogCaller().getOpenFileName(nullptr, "", g_Config.m_strBinPath, nullptr);

                if (strFileName.size() > 0)
				{
//					g_Config.m_strBinPath = ::GetFilePath(dlg.GetPathName());
//					// Получим имя
//					strFileName = dlg.GetPathName();
					UNICODEtoBK(strFileName, bkName, BK_NAMELENGTH, true);
				}
				else
				{
					// Если отмена - установим флаг ошибки
					SetByte(BK_EMT36BP_ERRADDR, 4);
					bError = true;
				}
			}
			else
			{
				// Если имя не пустое
				SetSafeName(strFileName);
                strFileName = QDir(g_Config.m_bSavesDefault ? g_Config.m_strSavesPath : g_Config.m_strBinPath). // подставляем соответствующий путь
                              filePath(strFileName + strBinExt); // добавляем стандартное расширение для бин файлов.
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

				DWORD cs = 0; // подсчитаем контрольную сумму

				for (int i = 0; i < fileSize; ++i)
				{
					uint8_t val = GetByte(fileAddr++);
					file.Write(&val, sizeof(val));
					cs += uint16_t(val);

					if (cs & 0xffff0000)
					{
						cs++;
						cs &= 0xffff;
					}
				}

				if (g_Config.m_bUseLongBinFormat)
				{
					file.Write(&cs, sizeof(uint16_t)); // контрольная сумма
				}

				file.Close();
				SetWord(BK_EMT36BP_CRCADDR, LOWORD(cs)); // сохраним контрольную сумму на своё место
			}
		}

		SetRON(CCPU::REGISTER::PC, 0116402); // выходим туда.
		// Refresh keyboard
		m_pParent->SendMessage(WM_RESET_KBD_MANAGER); // и почистим индикацию управляющих клавиш в статусбаре
		return true; // сэмулировали
	}

	return false; // не эмулируем
}

