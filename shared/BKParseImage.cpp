#if 0
#include "pch.h"

#include "BKDRT11Header.h"
#include "BKParseImage.h"

//#pragma warning(disable:4996)

CBKParseImage::CBKParseImage()
{}

CBKParseImage::~CBKParseImage()
{}

const char *CBKParseImage::strID_Andos = "ANDOS ";
const char *CBKParseImage::strID_DXDOS = "DX-DOS";
const char *CBKParseImage::strID_FAT12 = "FAT12   ";
const char *CBKParseImage::strID_Aodos = " AO-DOS ";
const char *CBKParseImage::strID_Nord  = "NORD ";
const char *CBKParseImage::strID_Nord_Voland = "BY VOLAND"; // мультизагрузочные диски - тоже будем опознавать как норд
const char *CBKParseImage::strID_RT11  = "?BOOT-";
const char *CBKParseImage::strID_RT11Q = "?QUBOOT-";

/* определение типа образа, на выходе номер, соответствующий определённой ОС
    0 - образ не опознан, -1 - ошибка
*/
PARSE_RESULT CBKParseImage::ParseImage(const std::wstring &fname, unsigned long nBaseOffset)
{
	union
	{
		uint8_t pSector[BLOCK_SIZE];
		uint16_t wSector[BLOCK_SIZE / 2];
	};
	CBKImgFile BKFloppyImgFile;
	PARSE_RESULT ret;
	bool bAC1 = false; // условие андос 1 (метка диска)
	bool bAC2 = false; // условие андос 2 (параметры диска)

	if (!BKFloppyImgFile.Open(fname.c_str(), false))
	{
		ret.imageOSType = IMAGE_TYPE::ERROR_NOIMAGE;
		return ret;
	}

	BKFloppyImgFile.SeekTo00();
	ret.strName = fname;
	ret.nBaseOffset = nBaseOffset;
	ret.nImageSize = BKFloppyImgFile.GetFileSize(); // получим размер файла
	nBaseOffset /= BLOCK_SIZE; // смещение в LBA

	if (BKFloppyImgFile.ReadLBA(pSector, nBaseOffset + 0, 1))
	{
		// Получим признак системного диска
		if (wSector[0] == 0240 /*NOP*/)
			if (wSector[1] != 5 /*RESET*/)
			{
				ret.bImageBootable = true;
			}

		// проверим тут, а то потом, прямо в условии присваивания не работают из-за оптимизации проверки условий
		bAC1 = (memcmp(strID_Andos, reinterpret_cast<char *>(pSector + 04), strlen(strID_Andos)) == 0);
		//      секторов в кластере                   число файлов в корне                          медиадескриптор                   число секторов в одной фат
		bAC2 = ((pSector[015] == 4) && (*(reinterpret_cast<uint16_t *>(pSector + 021)) == 112) && (pSector[025] == 0371) && (*(reinterpret_cast<uint16_t *>(pSector + 026)) == 2));

		// Узнаем формат образа
		if (wSector[0400 / 2] == 0123456) // если микродос
		{
			// определяем с определённой долей вероятности систему аодос
			if (pSector[4] == 032 && (wSector[6 / 2] == 256 || wSector[6 / 2] == 512))
			{
				// либо расширенный формат
				ret.imageOSType = IMAGE_TYPE::AODOS;
			}
			else if (memcmp(strID_Aodos, reinterpret_cast<char *>(pSector + 4), strlen(strID_Aodos)) == 0)
			{
				// либо по метке диска
				// аодос 1.77 так не распознаётся. его будем считать просто микродосом. т.к. там нету директорий
				ret.imageOSType = IMAGE_TYPE::AODOS;
			}
			else if (memcmp(strID_Nord, reinterpret_cast<char *>(pSector + 4), strlen(strID_Nord)) == 0)
			{
				// предполагаем, что это метка для системы
				ret.imageOSType = IMAGE_TYPE::NORD;
			}
			else if (memcmp(strID_Nord_Voland, reinterpret_cast<char *>(pSector + 0xa0), strlen(strID_Nord_Voland)) == 0)
			{
				// мультизагрузочные диски - тоже будем опознавать как норд
				ret.imageOSType = IMAGE_TYPE::NORD;
			}
			/* обнаружилась нехорошая привычка AODOS и возможно NORD мимикрировать под MKDOS,
			так что проверка на него - последней, при этом, т.к. MKDOSу похрен на ячейки 04..012
			то иногда, если там были признаки аодоса, мкдосный диск принимается за аодосный.
			но это лучше, чем наоборот, ибо в первом случае просто в имени директории будет глючный символ,
			а во втором - вообще все директории пропадают.
			*/
			else if (wSector[0402 / 2] == 051414)
			{
				ret.imageOSType = IMAGE_TYPE::MKDOS;
			}
			// тут надо найти способ как определить прочую экзотику
			else
			{
				int nRet = AnalyseMicrodos(&BKFloppyImgFile, nBaseOffset, pSector);

				switch (nRet)
				{
					case 0:
						ret.imageOSType = IMAGE_TYPE::MIKRODOS;
						break;

					case 1:
						ret.imageOSType = IMAGE_TYPE::AODOS;
						break;

					case 2:
						ret.imageOSType = IMAGE_TYPE::NORD;
						break;

					default:
						ret.imageOSType = IMAGE_TYPE::ERROR_NOIMAGE;
						break;
				}
			}
		}
		// проверка на формат FAT12, если надпись "FAT12"
		// однако, в некоторых андосах, нету идентификатора фс, зато есть метка андос,
		// но бывает, что и метки нету, зато параметры BPB специфические
		else if ( // если хоть что-то выполняется, то скорее всего это FAT12, но возможны ложные срабатывания из-за мусора в этой области
		    bAC1 || bAC2 ||
		    (memcmp(strID_FAT12, reinterpret_cast<char *>(pSector + 066), strlen(strID_FAT12)) == 0)
		)
		{
			uint8_t nMediaDescriptor = pSector[025];
			int nBootSize = *(reinterpret_cast<uint16_t *>(pSector + 016)) * (*(reinterpret_cast<uint16_t *>(pSector + 013)));
			uint8_t b[BLOCK_SIZE];
			BKFloppyImgFile.ReadLBA(b, nBaseOffset + nBootSize / BLOCK_SIZE, 1);

			if (b[1] == 0xff && b[0] == nMediaDescriptor) // если медиадескриптор в BPB и фат совпадают, то это точно FAT12
			{
				if (bAC1 || bAC2) // если одно из этих условий - то это скорее всего ANDOS
				{
					ret.imageOSType = IMAGE_TYPE::ANDOS;
				}
				else if (memcmp(strID_DXDOS, reinterpret_cast<char *>(pSector + 04), strlen(strID_DXDOS)) == 0) // если метка диска DX-DOS, то это скорее всего DX-DOS, других вариантов пока нет.
				{
					ret.imageOSType = IMAGE_TYPE::DXDOS;
				}
				else
				{
					ret.imageOSType = IMAGE_TYPE::MSDOS; // тут только MS DOS остался
				}
			}
			else
			{
				ret.imageOSType = IMAGE_TYPE::UNKNOWN;
			}
		}
		// проверка на предположительно возможно НС-ДОС
		else if (wSector[2 / 2] == 012700 && wSector[4 / 2] == 0404 && wSector[6 / 2] == 0104012)
		{
			// нс-дос никак себя не выделяет среди других ос, поэтому будем её определять по коду.
			ret.imageOSType = IMAGE_TYPE::NCDOS;
		}
		else
		{
			// дальше сложные проверки
			// проверим, а не рт11 ли у нас
			int t = IsRT11_old(&BKFloppyImgFile, nBaseOffset, pSector);

			switch (t)
			{
				case 0:
				{
					t = IsRT11(&BKFloppyImgFile, nBaseOffset, pSector);

					switch (t)
					{
						case 0:
						{
							// нет, не рт11
							// проверим, а не ксидос ли у нас
							t = IsCSIDOS3(&BKFloppyImgFile, nBaseOffset, pSector);

							switch (t)
							{
								case 0:
									ret.imageOSType = IMAGE_TYPE::UNKNOWN;
									break;

								case 1:
									ret.imageOSType = IMAGE_TYPE::CSIDOS3;
									break;

								default:
									ret.imageOSType = IMAGE_TYPE::ERROR_NOIMAGE;
							}

							break;
						}

						case 1:
						{
							ret.imageOSType = IMAGE_TYPE::RT11;
							break;
						}

						default:
							ret.imageOSType = IMAGE_TYPE::ERROR_NOIMAGE;
					}

					break;
				}

				case 1:
				{
					ret.imageOSType = IMAGE_TYPE::RT11;
					break;
				}

				default:
					ret.imageOSType = IMAGE_TYPE::ERROR_NOIMAGE;
			}
		}
	}
	else
	{
		ret.imageOSType = IMAGE_TYPE::ERROR_NOIMAGE;
	}

	BKFloppyImgFile.Close();
	return ret;
}

/*
анализатор каталога микродос, для уточнения формата.
2 - Nord
1 - Aodos
0 - Microdos
-1 - ошибка чтения файла
*/
int CBKParseImage::AnalyseMicrodos(CBKImgFile *f, unsigned long nBaseOffsetLBA, uint8_t *buf)
{
#pragma pack(push)
#pragma pack(1)
	struct MikrodosFileRecord
	{
		uint8_t dir_dir;    // статус записи: бит 7 - бэд блок, остальные значения - неизвестно
		uint8_t dir_num;    // статус записи: значения неизвестны
		uint8_t name[14];   // имя файла, если начинается с 0177 - каталог. и значит это не микродос
		uint16_t start_block;// начальный блок
		uint16_t len_blk;   // длина в блоках, если len_blk == 0 то это тоже с большой долей вероятности каталог
		uint16_t address;   // стартовый адрес
		uint16_t length;    // длина, или остаток длины от длины в блоках, если размер файла > 64кб
		MikrodosFileRecord()
		{
			memset(this, 0, sizeof(MikrodosFileRecord));
		}
		MikrodosFileRecord &operator = (const MikrodosFileRecord &src)
		{
			memcpy(this, &src, sizeof(MikrodosFileRecord));
			return *this;
		}
		MikrodosFileRecord &operator = (const MikrodosFileRecord *src)
		{
			memcpy(this, src, sizeof(MikrodosFileRecord));
			return *this;
		}
	};
#pragma pack(pop)
	int nRet = -1;
	constexpr int nCatSize = 10; // размер каталога - одна сторона нулевой дорожки.
	auto pCatBuffer = new uint8_t[nCatSize * BLOCK_SIZE];

	if (pCatBuffer)
	{
		// перемещаемся к нулевому сектору, + 0 - для наглядности
		if (f->ReadLBA(pCatBuffer, nBaseOffsetLBA + 0, nCatSize))
		{
			int nRecNum = *(reinterpret_cast<uint16_t *>(&pCatBuffer[030])); // читаем общее кол-во файлов.
			auto pDiskCat = reinterpret_cast<MikrodosFileRecord *>(pCatBuffer + 0500); // каталог диска
			int nMKDirFlag = 0;
			int nAODirFlag = 0;

			// сканируем каталог и ищем там директории
			for (int i = 0; i < nRecNum; ++i)
			{
				if (pDiskCat[i].name[0] == 0177)
				{
					nMKDirFlag++;
				}
				else if (pDiskCat[i].name[0] != 0 && pDiskCat[i].len_blk == 0)
				{
					nAODirFlag++;
				}
			}

			if (nMKDirFlag && nAODirFlag)
			{
				nRet = 0;
			}
			else if (nMKDirFlag)
			{
				nRet = 2;
			}
			else if (nAODirFlag)
			{
				nRet = 1;
			}
			else
			{
				nRet = 0;
			}
		}

		delete[]pCatBuffer;
	}

	return nRet;
}



// сравнение двух символов, без учёта регистра, русских букв не сравниваем, т.к. пока не нужно
// выход: true - равны
//      false - не равны
bool CBKParseImage::charcompare(uint8_t a, uint8_t b)
{
	if ('a' <= a && a <= 'z')
	{
		a -= ' '; // делаем из маленькой буквы большую
	}

	if ('a' <= b && b <= 'z')
	{
		b -= ' '; // делаем из маленькой буквы большую
	}

	if (a == b)
	{
		return true;
	}

	return false;
}

bool CBKParseImage::substrfind(uint8_t *bufs, size_t len_bufs, uint8_t *buff, size_t len_buff)
{
	size_t b_pos = 0; // позиция в буфере.
	size_t s_pos = 0; // позиция в строке, которую ищем
	size_t t_pos;
	bool bOk = false;

	while (b_pos < len_bufs)
	{
		// пока есть где искать
		// ищем совпадение с первым символом
		if (charcompare(bufs[b_pos++], buff[s_pos]))
		{
			// нашли. теперь проверим на совпадения с остальными символами
			t_pos = b_pos;
			s_pos++;    // первый символ совпал, начинаем проверять со второго
			bOk = true; // флаг выхода из цикла поиска на совпадение строки

			while (s_pos < len_buff)
			{
				// пока строка не кончится
				if (!charcompare(buff[s_pos++], bufs[t_pos++]))
				{
					// если символ не совпал
					bOk = false; // то не ОК
					break;
				}
			}

			b_pos = t_pos;
			s_pos = 0; // вернёмся на начало строки

			if (bOk)
			{
				// если таки вся строка совпала
				break; // незачем дальше искать
			}
		}
	}

	// на выходе отсюда bOk == true если строка нашлась, иначе - false
	return bOk;
}

/*
Проверим, а не rt11 ли это старым методом. Поскольку непонятно как узнать, что это у нас УКНЦшный АДОС.
выход:
1 - rt11
0 - не rt11
-1 - ошибка чтения файла
*/
int CBKParseImage::IsRT11_old(CBKImgFile *f, unsigned long nBaseOffsetLBA, uint8_t *buf)
{
	int nRet = 0;

	// перемещаемся к нулевому сектору, + 0 - для наглядности

	if (!f->ReadLBA(buf, nBaseOffsetLBA + 0, 1))
	{
		return -1;
	}

	// надо найти строку strID_RT11 где-то в нулевом секторе.
	size_t s_len = strlen(strID_RT11);
	size_t b_len = BLOCK_SIZE - s_len;
	bool bOk = substrfind(buf, b_len, (uint8_t *)strID_RT11, s_len);

	if (bOk)
	{
		nRet = 1;
	}
	else
	{
		// если не нашли строку первого вида, поищем второго
		s_len = strlen(strID_RT11Q);
		b_len = BLOCK_SIZE - s_len;
		bOk = substrfind(buf, b_len, (uint8_t *) strID_RT11Q, s_len);

		if (bOk)
		{
			nRet = 1;
		}
	}

	return nRet;
}

/*
Проверим, а не rt11 ли это. выход:
1 - rt11
0 - не rt11
-1 - ошибка чтения файла
*/
int CBKParseImage::IsRT11(CBKImgFile *f, unsigned long nBaseOffsetLBA, uint8_t *buf)
{
	int nRet = 0;
	auto pwBuf = reinterpret_cast<uint16_t *>(buf);
	uint8_t bufsh[BLOCK_SIZE];
	// проверку будем осуществлять методом эмуляции чтения каталога.
	const unsigned int nImgSize = f->GetFileSize() / BLOCK_SIZE; // размер образа в блоках.

	// перемещаемся к первому сектору
	if (!f->ReadLBA(buf, nBaseOffsetLBA + 1, 1))
	{
		return -1;
	}

	unsigned int nBeginBlock = pwBuf[0724 / 2]; // получим блок, с которого начинается сектор.
	unsigned int nCurrentSegment = 1;   // номер текущего сегмента
	unsigned int nTotalSegments = 0;    // всего возможных сегментов в каталоге
	unsigned int nUsedSegments = -1;    // число занятых сегментов, не должно совпадать с nCount
	unsigned int nAddBytes = 0;
	unsigned int nCount = 0;            // счётчик пройденных сегментов

	// !!! фикс для АДОС
	if ((nBeginBlock == 0) || (nBeginBlock > 255))
	{
		nBeginBlock = 6;
	}

	if (nBeginBlock < nImgSize)
	{
		while (nCurrentSegment > 0)
		{
			int offs = ((nCurrentSegment - 1) * 2 + nBeginBlock);

			if (!f->ReadLBA(bufsh, nBaseOffsetLBA + offs, 1)) // перемещаемся к началу текущего сегмента каталога и читаем
			{
				return -1;
			}

			auto CurrentSegmentHeader = reinterpret_cast<RT11SegmentHdr *>(bufsh);

			if (nTotalSegments == 0)
			{
				nTotalSegments = CurrentSegmentHeader->segments_num; // если в первый раз, запомним число сегментов.

				if (nTotalSegments > 31) // если число сегментов больше 31, то не RT-11
				{
					break;
				}

				nUsedSegments = CurrentSegmentHeader->used_segments;

				// если в первом сегменте счётчик использованных сегментов больше общего числа сегментов
				if (nUsedSegments > nTotalSegments)
				{
					break;
				}

				nAddBytes = CurrentSegmentHeader->filerecord_add_bytes;

				if (nAddBytes > 252) // если число доп слов больше размера сегмента - то это не RT-11
				{
					// т.к. в сегменте обязательно должен быть маркер конца сегмента, то в нём должны умещаться как минимум 2 записи.
					// поэтому 72*7 делим пополам
					break;
				}
			}
			else
			{
				// если не в первый - сравним старое кол-во с новым, если не равно - значит не RT-11
				if (nTotalSegments != CurrentSegmentHeader->segments_num)
				{
					break;
				}

				if (nAddBytes != CurrentSegmentHeader->filerecord_add_bytes)
				{
					break;
				}
			}

			// если начальный блок в сегменте выходит за пределы образа - то это не RT-11
			if (CurrentSegmentHeader->file_block > nImgSize)
			{
				break;
			}

			nCurrentSegment = CurrentSegmentHeader->next_segment;

			if (nCurrentSegment > 31) // если номер следующего сегмента больше 31, то не RT-11
			{
				break;
			}

			if (nCurrentSegment > nTotalSegments) // если номер следующего сегмента больше общего кол-ва сегментов, то не RT-11
			{
				break;
			}

			nCount++; // чтобы не зациклиться, считаем пройденные сегменты

			if (nCount > nTotalSegments) // если их было больше, чем общее число сегментов - то это не RT-11
			{
				break;
			}
		}

		// когда выходим из цикла
		// если соблюдаются следующие условия, то очень вероятно, что это всё-таки RT-11
		// третье условие - избыточно
		if ((nCurrentSegment == 0) && (nCount == nUsedSegments) && (nCount <= nTotalSegments))
		{
			nRet = 1;
		}
	}

	return nRet;
}

/*
Проверим, а не ксидос ли это. выход:
1 - ксидос
0 - не ксидос
-1 - ошибка чтения
*/
int CBKParseImage::IsCSIDOS3(CBKImgFile *f, unsigned long nBaseOffsetLBA, uint8_t *buf)
{
	auto pwBuf = reinterpret_cast<uint16_t *>(buf);
	int nRet = -1;

	// проверим, а не ксидос ли у нас
	if (f->ReadLBA(buf, nBaseOffsetLBA + 2, 1))
	{
		if ((pwBuf[8 / 2] == 0123123) || (pwBuf[8 / 2] == 0123124))
		{
			// если ксидос или особый ксидос
			if ((pwBuf[4 / 2] == 0123123) && (pwBuf[6 / 2] == 0123123)) // если ксидос3
			{
				nRet = 1;
			}
			else
			{
				nRet = 0; // про ксидос других версий ничего неизвестно
			}
		}
		else
		{
			nRet = 0; // вообще не ксидос
		}
	}

	return nRet;
}

#endif
