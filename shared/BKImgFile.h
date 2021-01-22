#pragma once
//#include "fdrawcmd.h"
#include "pch.c"

class CBKImgFile
{
		static uint8_t  m_nCylinders;
		static uint8_t  m_nHeads;
		static uint8_t  m_nSectors;

		FILE           *m_f;        // если открываем образ - то это его файл
		HANDLE          m_h;        // если открываем устройство - то это его хэндл
		std::wstring    m_strName;  // имя образа или устройства

		struct CHS
		{
			uint8_t c, h, s;
			CHS() : c(0), h(0), s(0) {}
		};

		CHS             ConvertLBA(UINT lba); // LBA -> CHS
		UINT            ConvertCHS(uint8_t c, uint8_t h, uint8_t s);    // CHS -> LBA
		// операция ввода вывода, т.к. оказалось, что fdraw не умеет блочный обмен мультитрековый,
		// да и позиционирование нужно делать вручную
		bool            IOOperation(DWORD cmd, FD_READ_WRITE_PARAMS *rwp, void *buffer, UINT numSectors);

	public:
		CBKImgFile();
		CBKImgFile(const std::wstring strName, const bool bWrite);
		~CBKImgFile();
		bool            Open(const std::wstring strName, const bool bWrite);
		void            Close();

		/*
		buffer - куда читать/писать, о размере должен позаботиться пользователь
		cyl - номер дорожки
		head - номер головки
		sector - номер сектора
		numSectors - количество читаемых/писаемых секторов
		*/
		bool            ReadCHS(void *buffer, uint8_t cyl, uint8_t head, uint8_t sector, UINT numSectors);
		bool            WriteCHS(void *buffer, uint8_t cyl, uint8_t head, uint8_t sector, UINT numSectors);
		bool            ReadLBA(void *buffer, UINT lba, UINT numSectors);
		bool            WriteLBA(void *buffer, UINT lba, UINT numSectors);
		long            GetFileSize();
		bool            SeekTo00();
		bool            IsFileOpen();
};

