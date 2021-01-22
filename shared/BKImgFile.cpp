#if 0
#include "pch.h"
#include "BKImgFile.h"
#pragma warning(disable:4996)

uint8_t CBKImgFile::m_nCylinders = 81;
uint8_t CBKImgFile::m_nHeads = 2;
uint8_t CBKImgFile::m_nSectors = 10;

constexpr auto BLOCK_SIZE = 512;

CBKImgFile::CBKImgFile()
	: m_f(nullptr)
	, m_h(INVALID_HANDLE_VALUE)
{
}

CBKImgFile::CBKImgFile(const std::wstring strName, const bool bWrite)
	: m_f(nullptr)
	, m_h(INVALID_HANDLE_VALUE)
{
	Open(strName, bWrite);
}


CBKImgFile::~CBKImgFile()
{
	Close();
}

bool CBKImgFile::Open(const std::wstring strName, const bool bWrite)
{
	bool bRet = false;
	bool bNeedFDRaw = false;
	bool bFloppy = false;
	bool bFDRaw = false;
	DWORD dwRet;

	if (strName.length() >= 4)
	{
		std::wstring st = strName.substr(0, 4);

		if (st == L"\\\\.\\")
		{
			bFloppy = true;
			st = strName.substr(4, 5);

			if (st == L"fdraw")
			{
				bNeedFDRaw = true;
				HANDLE hFD = CreateFile(L"\\\\.\\fdrawcmd", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

				if (hFD != INVALID_HANDLE_VALUE)
				{
					DWORD dwVersion = 0;
					DeviceIoControl(hFD, IOCTL_FDRAWCMD_GET_VERSION, nullptr, 0, &dwVersion, sizeof(dwVersion), &dwRet, nullptr);
					CloseHandle(hFD);
					bFDRaw = true;

					if (!dwVersion)
					{
						// ("fdrawcmd.sys не установлен, смотрите: http://simonowen.com/fdrawcmd/\n");
						bFDRaw = false;
					}
					else if (HIWORD(dwVersion) != HIWORD(FDRAWCMD_VERSION))
					{
						// ("Установленный fdrawcmd.sys не совместим с этой программой.\n");
						bFDRaw = false;
					}
				}
			}
		}
	}

	if (bFloppy)
	{
		m_h = CreateFile(strName.c_str(), GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);

		if (m_h != INVALID_HANDLE_VALUE)
		{
			if (bNeedFDRaw && bFDRaw)
			{
				int DISK_DATARATE = FD_RATE_250K;          // 2 is 250 kbit/sec
				DeviceIoControl(m_h, IOCTL_FD_SET_DATA_RATE, &DISK_DATARATE, sizeof(DISK_DATARATE), nullptr, 0, &dwRet, nullptr);
				DeviceIoControl(m_h, IOCTL_FD_RESET, nullptr, 0, nullptr, 0, &dwRet, nullptr);
			}
			else
			{
				return false;
			}

			m_strName = strName;
			bRet = true;
		}
	}
	else
	{
		std::wstring strMode = bWrite ? L"r+b" : L"rb";
		m_f = _wfopen(strName.c_str(), strMode.c_str());

		if (m_f)
		{
			m_strName = strName;
			bRet = true;
		}
	}

	return bRet;
}

void CBKImgFile::Close()
{
	if (m_f)
	{
		fclose(m_f);
		m_f = nullptr;
	}

	if (m_h != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_h);
		m_h = INVALID_HANDLE_VALUE;
	}
}


CBKImgFile::CHS CBKImgFile::ConvertLBA(UINT lba)
{
	CHS ret;
	// превратить смещение в байтах в позицию в c:h:s;
	// поскольку формат строго фиксирован: c=80 h=2 s=10, а перемещения предполагаются только по границам секторов
	UINT t = static_cast<UINT>(m_nHeads) * static_cast<UINT>(m_nSectors);
	ret.c = lba / t;
	t = lba % t;
	ret.h = static_cast<uint8_t>(t / static_cast<UINT>(m_nSectors));
	ret.s = static_cast<uint8_t>(t % static_cast<UINT>(m_nSectors)) + 1;
	return ret;
}

UINT CBKImgFile::ConvertCHS(uint8_t c, uint8_t h, uint8_t s)
{
	UINT lba = (static_cast<UINT>(c) * static_cast<UINT>(m_nHeads) + static_cast<UINT>(h)) * static_cast<UINT>(m_nSectors) + static_cast<UINT>(s) - 1;
	return lba;
}

bool CBKImgFile::IOOperation(DWORD cmd, FD_READ_WRITE_PARAMS *rwp, void *buffer, UINT numSectors)
{
	UINT nSectorSize = 128 << rwp->size;
	auto pDataBuf = reinterpret_cast<uint8_t *>(buffer);
	DWORD dwRet;
	UINT cyl = rwp->cyl;
	BOOL b = DeviceIoControl(m_h, IOCTL_FDCMD_SEEK, &cyl, sizeof(cyl), nullptr, 0, &dwRet, nullptr);

	for (UINT n = 0; n < numSectors; ++n)
	{
		rwp->eot = rwp->sector + 1;
		rwp->phead = rwp->head;

		// посекторно читаем, потому что fdraw не умеет блочно с переходом на следующую дорожку
		if (!DeviceIoControl(m_h, cmd, rwp, sizeof(FD_READ_WRITE_PARAMS), pDataBuf, nSectorSize, &dwRet, nullptr))
		{
			return false;
		}

		pDataBuf += nSectorSize;

		if (++rwp->sector > m_nSectors)
		{
			rwp->sector = 1;

			if (++rwp->head >= m_nHeads)
			{
				rwp->head = 0;

				if (++rwp->cyl > m_nCylinders) // допускаем дорожки 0..81
				{
					return false;
				}
				else
				{
					cyl = rwp->cyl;
					b = DeviceIoControl(m_h, IOCTL_FDCMD_SEEK, &cyl, sizeof(cyl), nullptr, 0, &dwRet, nullptr);
				}
			}
		}
	}

	return true;
}

bool CBKImgFile::ReadCHS(void *buffer, uint8_t cyl, uint8_t head, uint8_t sector, UINT numSectors)
{
	if (m_f)
	{
		UINT lba = ConvertCHS(cyl, head, sector) * BLOCK_SIZE;

		if (0 == fseek(m_f, lba, SEEK_SET))
		{
			lba = numSectors * BLOCK_SIZE;
			return (lba == fread(buffer, 1, lba, m_f));
		}
		else
		{
			return false;
		}
	}
	else if (m_h != INVALID_HANDLE_VALUE)
	{
		FD_READ_WRITE_PARAMS rwp = { FD_OPTION_MFM, head, cyl, head, sector, 2, sector, 0x0a, 0xff };
		return IOOperation(IOCTL_FDCMD_READ_DATA, &rwp, buffer, numSectors);
	}

	return false;
}

bool CBKImgFile::WriteCHS(void *buffer, uint8_t cyl, uint8_t head, uint8_t sector, UINT numSectors)
{
	if (m_f)
	{
		UINT lba = ConvertCHS(cyl, head, sector) * BLOCK_SIZE;

		if (0 == fseek(m_f, lba, SEEK_SET))
		{
			lba = numSectors * BLOCK_SIZE;
			return (lba == fwrite(buffer, 1, lba, m_f));
		}
		else
		{
			return false;
		}
	}
	else if (m_h != INVALID_HANDLE_VALUE)
	{
		FD_READ_WRITE_PARAMS rwp = { FD_OPTION_MFM, head, cyl, head, sector, 2, sector, 0x0a, 0xff };
		return IOOperation(IOCTL_FDCMD_WRITE_DATA, &rwp, buffer, numSectors);
	}

	return false;
}

bool CBKImgFile::ReadLBA(void *buffer, UINT lba, UINT numSectors)
{
	if (m_f)
	{
		if (0 == fseek(m_f, lba * BLOCK_SIZE, SEEK_SET))
		{
			UINT size = numSectors * BLOCK_SIZE;
			return (size == fread(buffer, 1, size, m_f));
		}
		else
		{
			return false;
		}
	}
	else if (m_h != INVALID_HANDLE_VALUE)
	{
		CHS chs = ConvertLBA(lba);
		FD_READ_WRITE_PARAMS rwp = { FD_OPTION_MFM, chs.h, chs.c, chs.h, chs.s, 2, chs.s, 0x0a, 0xff };
		return IOOperation(IOCTL_FDCMD_READ_DATA, &rwp, buffer, numSectors);
	}

	return false;
}




bool CBKImgFile::WriteLBA(void *buffer, UINT lba, UINT numSectors)
{
	if (m_f)
	{
		if (0 == fseek(m_f, lba * BLOCK_SIZE, SEEK_SET))
		{
			UINT size = numSectors * BLOCK_SIZE;
			return (size == fwrite(buffer, 1, size, m_f));
		}
		else
		{
			return false;
		}
	}
	else if (m_h != INVALID_HANDLE_VALUE)
	{
		CHS chs = ConvertLBA(lba);
		FD_READ_WRITE_PARAMS rwp = { FD_OPTION_MFM, chs.h, chs.c, chs.h, chs.s, 2, chs.s, 0x0a, 0xff };
		return IOOperation(IOCTL_FDCMD_WRITE_DATA, &rwp, buffer, numSectors);
	}

	return false;
}

long CBKImgFile::GetFileSize()
{
	if (m_f)
	{
		struct _stat stat_buf;
		int rc = _wstat(m_strName.c_str(), &stat_buf);
		return rc == 0 ? stat_buf.st_size : -1;
	}
	else if (m_h != INVALID_HANDLE_VALUE)
	{
		return static_cast<long>(m_nCylinders) * static_cast<long>(m_nSectors) * static_cast<long>(m_nHeads) * BLOCK_SIZE;
	}

	return 0;
}

bool CBKImgFile::SeekTo00()
{
	if (m_f)
	{
		return (0 == fseek(m_f, 0, SEEK_SET));
	}
	else if (m_h != INVALID_HANDLE_VALUE)
	{
		DWORD dwRet;
		int cyl = 0;
		return !!DeviceIoControl(m_h, IOCTL_FDCMD_SEEK, &cyl, sizeof(cyl), nullptr, 0, &dwRet, nullptr);
	}

	return false;
}

bool CBKImgFile::IsFileOpen()
{
	if (m_f)
	{
		return true;
	}

	if (m_h != INVALID_HANDLE_VALUE)
	{
		return true;
	}

	return false;
}


#endif
