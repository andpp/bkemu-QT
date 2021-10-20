#pragma once
#include "BKImgFile.h"

enum class IMAGE_TYPE : int
{
	ERROR_NOIMAGE = -1,
	UNKNOWN = 0,
	ANDOS,
	MKDOS,
	AODOS,
	NORD,
	MIKRODOS,
	CSIDOS3,
	RT11,
	NCDOS,
	DXDOS,
	MSDOS
};

// размер блока по умолчанию. == размер сектора
constexpr auto BLOCK_SIZE = 512;


struct PARSE_RESULT
{
	unsigned long   nImageSize;
	unsigned long   nBaseOffset;
	IMAGE_TYPE      imageOSType;
	bool            bImageBootable;
	std::wstring    strName;

	PARSE_RESULT()
	{
		clear();
	}

	~PARSE_RESULT()
	{
		this->strName.erase();
	}

	void clear()
	{
		this->nBaseOffset = this->nImageSize = 0;
		this->imageOSType = IMAGE_TYPE::UNKNOWN;
		this->bImageBootable = false;
		this->strName.erase();
	}

	PARSE_RESULT(const PARSE_RESULT *src)
	{
		this->nImageSize = src->nImageSize;
		this->nBaseOffset = src->nBaseOffset;
		this->imageOSType = src->imageOSType;
		this->bImageBootable = src->bImageBootable;
		this->strName = src->strName;
	}

	PARSE_RESULT &operator = (const PARSE_RESULT &src)
	    = default;
};

class CBKParseImage
{
		static const char *strID_Andos;
		static const char *strID_DXDOS;
		static const char *strID_FAT12;
		static const char *strID_Aodos;
		static const char *strID_Nord;
		static const char *strID_Nord_Voland;
		static const char *strID_RT11;
		static const char *strID_RT11Q;
	public:
		CBKParseImage();
		~CBKParseImage();
		PARSE_RESULT    ParseImage(const std::wstring &fname, unsigned long nBaseOffset);

	protected:
		bool            charcompare(uint8_t a, uint8_t b);
		bool            substrfind(uint8_t *bufs, size_t len_bufs, uint8_t *buff, size_t len_buff);
		int             IsCSIDOS3(CBKImgFile *f, unsigned long nBaseOffsetLBA, uint8_t *buf);
		int             IsRT11_old(CBKImgFile *f, unsigned long nBaseOffsetLBA, uint8_t *buf);
		int             IsRT11(CBKImgFile *f, unsigned long nBaseOffsetLBA, uint8_t *buf);
		int             AnalyseMicrodos(CBKImgFile *f, unsigned long nBaseOffsetLBA, uint8_t *buf);

};

