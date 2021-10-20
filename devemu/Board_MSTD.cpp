// Board_MSTD.cpp: implementation of the CMotherBoard_MSTD class.
//


#include "pch.h"
#include "Board_MSTD.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction


CMotherBoard_MSTD::CMotherBoard_MSTD()
    = default;

CMotherBoard_MSTD::~CMotherBoard_MSTD()
    = default;


MSF_CONF CMotherBoard_MSTD::GetConfiguration()
{
	return MSF_CONF::BK1001_MSTD;
}

bool CMotherBoard_MSTD::InitMemoryModules()
{
	m_ConfBKModel.nROMPresent = 0;

	if (LoadRomModule(IDS_INI_BK10_RE2_017_MONITOR, BRD_10_MON10_BNK))
	{
		m_ConfBKModel.nROMPresent |= (3 << BRD_10_MON10_BNK);
	}

	if (LoadRomModule(IDS_INI_BK10_RE2_018_FOCAL, BRD_10_FOCAL_BNK))
	{
		m_ConfBKModel.nROMPresent |= (3 << BRD_10_FOCAL_BNK);
	}

	if (LoadRomModule(IDS_INI_BK10_OPT_MSTD, BRD_10_CUSTOM_BNK))
	{
		m_ConfBKModel.nROMPresent |= (3 << BRD_10_CUSTOM_BNK);
	}

	if (LoadRomModule(IDS_INI_BK10_RE2_019_MSTD, BRD_10_TESTS_BNK))
	{
		m_ConfBKModel.nROMPresent |= (3 << BRD_10_TESTS_BNK);
	}

	// и проинициализируем карту памяти
	MemoryManager();
	return true;
}


