// Board_EXT32.cpp: implementation of the CMotherBoard_EXT32 class.
//


#include "pch.h"
#include "Board_EXT32.h"
#include "BKMessageBox.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction


CMotherBoard_EXT32::CMotherBoard_EXT32()
{
	m_nBKPortsIOArea = BK_PORTSIO_AREA;
	// Инициализация модуля памяти, 64 кб + 32 кб доп.
	SAFE_DELETE_ARRAY(m_pMemory);
	m_pMemory = new uint8_t [0300000];

	if (!m_pMemory)
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

	InitMemoryValues(0300000);
}

CMotherBoard_EXT32::~CMotherBoard_EXT32()
    = default;

MSF_CONF CMotherBoard_EXT32::GetConfiguration()
{
	return MSF_CONF::BK1001_EXT32;
}

void CMotherBoard_EXT32::OnReset()
{
	CMotherBoard::OnReset();
	SetMemoryPage(3);
	SetWordIndirect(0120000, 0); // хак, чтобы не зацикливаться в вечном стопе
}

bool CMotherBoard_EXT32::InitMemoryModules()
{
	m_ConfBKModel.nROMPresent = 0;

	if (LoadRomModule(IDS_INI_BK10_RE2_017_MONITOR, BRD_10_MON10_BNK))
	{
		m_ConfBKModel.nROMPresent |= (3 << BRD_10_MON10_BNK);
	}

	// и проинициализируем карту памяти
	MemoryManager();
	return true;
}

void CMotherBoard_EXT32::MemoryManager()
{
	for (int i = 0; i < 020; i++)
	{
		m_MemoryMap[i].bReadable = true;
		m_MemoryMap[i].bWritable = true;
		m_MemoryMap[i].nBank = i;
		m_MemoryMap[i].nOffset = i << 12;
		m_MemoryMap[i].nTimingCorrection = RAM_TIMING_CORR_VALUE_D;
	}

	// 8,9 - ПЗУ
	for (int i = BRD_10_MON10_BNK; i <= BRD_10_MON10_BNK + 1; i++)
	{
		if (m_ConfBKModel.nROMPresent & (1 << i))
		{
			m_MemoryMap[i].bReadable = true;
		}
		else
		{
			m_MemoryMap[i].bReadable = false;
		}

		m_MemoryMap[i].bWritable = false;
		m_MemoryMap[i].nTimingCorrection = ROM_TIMING_CORR_VALUE;
	}

	// 10, 11 банк - доп. озу.
	int ab = m_nPage * 2 + 020;

	for (int i = BRD_10_BASIC10_1_BNK; i <= BRD_10_BASIC10_1_BNK + 1; i++)
	{
		m_MemoryMap[i].nBank = ab;
		m_MemoryMap[i].nOffset = ab << 12;
		m_MemoryMap[i].nTimingCorrection = RAM_TIMING_CORR_VALUE_S;
	}

	// 12-15 банки - пусто, хотя туда можно было подрубать доп озу. кто придумал такую нерациональную схему, непонятно.
	for (int i = BRD_10_BASIC10_2_BNK; i <= BRD_10_BASIC10_2_BNK + 3; i++)
	{
		m_MemoryMap[i].bReadable = false;
		m_MemoryMap[i].bWritable = false;
	}
}


void CMotherBoard_EXT32::SetMemoryPage(int nPage)
{
	m_nPage = nPage & 3;
	// и диспетчер памяти, мы тут должны поменять в 10м и 11м банках нужные нам банки
	MemoryManager();
}


void CMotherBoard_EXT32::SetByteT(const uint16_t addr, uint8_t value, int &nTC)
{
	int nBank = (addr >> 12) & 0x0f;

	// Сперва проверим, на системные регистры
	if ((nBank == 15) && (addr >= m_nBKPortsIOArea))
	{
		if (SetSystemRegister(addr, value, true))
		{
			nTC += REG_TIMING_CORR_VALUE;
			return;
		}

		throw CExceptionHalt(addr, _T("Can't write this address."));
	}

	// monitor on 0100000 - 0120000
	if (010 <= nBank && nBank <= 011)
	{
		SetMemoryPage(value);
		return;
	}

	if (m_MemoryMap[nBank].bWritable)
	{
		nTC += m_MemoryMap[nBank].nTimingCorrection;
		m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 07777)] = value;
	}
	else
	{
		throw CExceptionHalt(addr, _T("Can't write this address."));
	}
}


void CMotherBoard_EXT32::SetWordT(const uint16_t addr, uint16_t value, int &nTC)
{
	const int nBank = (addr >> 12) & 0x0f;
	const uint16_t a = addr & 0177776;

	// Сперва проверим, на системные регистры
	if ((nBank == 15) && (a >= m_nBKPortsIOArea))
	{
		if (SetSystemRegister(a, value, false))
		{
			nTC += REG_TIMING_CORR_VALUE;
			return;
		}

		throw CExceptionHalt(addr, _T("Can't write this address."));
	}

	// monitor on 0100000 - 0120000
	if (010 <= nBank && nBank <= 011)
	{
		SetMemoryPage(value);
		return;
	}

	if (m_MemoryMap[nBank].bWritable)
	{
		nTC += m_MemoryMap[nBank].nTimingCorrection;
		*(uint16_t *)&m_pMemory[m_MemoryMap[nBank].nOffset + (addr & 07776)] = value;
	}
	else
	{
		throw CExceptionHalt(addr, _T("Can't write this address."));
	}
}


bool CMotherBoard_EXT32::RestoreState(CMSFManager &msf, QImage *hScreenshot)
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
							if (msf.GetBlockExt32Memory(&m_nPage, m_pMemory + 65536))
							{
								return true;
							}
						}
						else
						{
							if (msf.SetBlockExt32Memory(m_nPage, m_pMemory + 65536))
							{
								return true;
							}
						}
					}
				}
			}
		}
	}

	return false;
}

