// Tape.cpp: implementation of the CTape class.
//
// !!!Остался один непонятный косяк. Wav файлы, которые записаны эмулятором
// на частотах 44100 и 96000 не воспринимаются эмулятором на частоте 48000
// даже преобразованный в tap такой wav не воспринимается. Как-то неправильно
// ресэмплинг делается. Там каждый 10й(11й) сэмпл дублируется /удаляется
// Wav файл, записанный на 48000 не воспринимается на частоте 44100, а на
// 96000 - воспринимается.

#include "pch.h"
#include "Tape.h"

//#include "resource.h"
#include "Config.h"
#include "MSFManager.h"
#include "BKMessageBox.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif


constexpr auto RECORD_BUFFER = (4 * 1024 * 1024);
constexpr auto RECORD_GROW = (2 * 1024 * 1024);
constexpr auto BAD_CODE = 65535;
constexpr auto RESET_CODE = 256;
constexpr auto PREFIX = 256;
constexpr auto CODE = 257;
constexpr auto BAD_LENGTH = -1;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction


CTape::CTape()
	: m_pWave(nullptr)
	, m_nWaveLength(0)
	, m_nPlayPos(0)
	, m_pRecord(nullptr)
	, m_nRecordPos(0)
	, m_nRecordLength(0)
	, m_bAutoBeginRecord(false)
	, m_bAutoEndRecord(false)
	, m_pBin(nullptr)
	, m_nPos(0)
	, m_nAverage(0)
	, m_nAvgLength(0)
	, m_dAvgLength(0.0)
	, m_bInverse(false)
	, m_nScanTableSize(0)
	, m_bRecord(false)
	, m_bPlay(false)
	, m_bWaveLoaded(false)
{
	SetWaveParam(DEFAULT_SOUND_SAMPLE_RATE, BUFFER_CHANNELS);

	// Initialize table pointers
	for (auto &y : m_pScanTable)
	{
		y = nullptr;
	}
}

CTape::~CTape()
{
	SAFE_DELETE_MEMORY(m_pWave);
	SAFE_DELETE_MEMORY(m_pRecord);
	SAFE_DELETE_ARRAY(m_pBin);
	// очищаем таблицы
	ClearTables();
}


void CTape::ClearTables()
{
	for (auto &y : m_pScanTable)
	{
		SAFE_DELETE_ARRAY(y);
	}
}


void CTape::SetWaveParam(int nWorkingSSR /*= DEFAULT_SOUND_SAMPLE_RATE*/, int nWorkingChn /*= BUFFER_CHANNELS*/)
{
	m_nWorkingSSR = nWorkingSSR;
	m_nWorkingChannels = nWorkingChn;
	m_nSampleBlockAlign = m_nWorkingChannels * SAMPLE_INT_SIZE;
}


bool CTape::LoadWaveFile(const CString &strPath)
{
	bool bRet = false;
	CFile waveFile;
	WAVEFORMATEX wfx;
	DataHeader dataHeader;

	if (waveFile.Open(strPath, CFile::modeRead)) // если файл открылся.
	{
		WaveHeader waveHeader;

		if (waveFile.Read(&waveHeader, sizeof(WaveHeader)) == sizeof(WaveHeader)) // если заголовок прочёлся
		{
			if (waveHeader.riffTag == RIFF_TAG && waveHeader.fmtTag == FMT_TAG) // если заголовок тот, что нам нужен
			{
				if (waveFile.Read(&wfx, waveHeader.fmtSize) == waveHeader.fmtSize) // если информация о формате прочиталась
				{
					wfx.cbSize = 0; // обнуляем поле доп. информации, мы всё равно не умеем её использовать

					if (wfx.wFormatTag == WAVE_FORMAT_PCM) // если формат WAV тот, что нам нужен
					{
						for (;;) // ищем нужный блок данных
						{
							if (waveFile.Read(&dataHeader, sizeof(DataHeader)) == sizeof(DataHeader)) // если заголовок прочёлся
							{
								if (dataHeader.dataTag == DATA_TAG) // если блок тот, что нам нужен
								{
									bRet = true; // всё, всё что нужно нашли
									break; // выходим
								}

								waveFile.Seek(dataHeader.dataSize, CFile::current); // переходим к следующему блоку.
							}
						}
					}
				}
			}
		}
	}

	if (bRet)
	{
		/*
		тут нужна конвертация из любого формата wav WAVE_FORMAT_PCM, в m_nWorkingSSR, 16 бит, 2 канала

		для WAVE_FORMAT_PCM данные могут быть только 8 или 16 бит,
		каналов 1 или 2 по стандарту, но никто не мешает сделать и больше
		частота дискретизации 8000, 11025, 22050, 44100, но никто не мешает сделать и больше
		*/
		{
			// определим количество сэмплов в исходном сигнале
			int nSrcSamples = dataHeader.dataSize / wfx.nBlockAlign;
			// определим количество сэмплов в результирующем сигнале
			auto nDstSamples = int(double(nSrcSamples) * double(m_nWorkingSSR) / double(wfx.nSamplesPerSec));

			// выделяем буфер под звук
			if (!AllocWaveBuffer(nDstSamples))
			{
				return false;
			}

			m_nControlWaveLength = m_nWaveLength;
			m_nWaveLength = 0; // длину надо обнулить
			auto inBuf = new uint8_t[wfx.nAvgBytesPerSec]; // входной буфер в байтах

			if (inBuf)
			{
				int converted;
				int nRest = dataHeader.dataSize;

				do
				{
					// сколько байтов можно прочитать
					int readed = nRest > int(wfx.nAvgBytesPerSec) ? int(wfx.nAvgBytesPerSec) : nRest;
					nRest -= readed; // сколько ещё осталось прочитать

					// оказалось, что чтение глючит, и при достижении конца файла возвращается не число
					// реально прочитанных байтов, а заданное максимально возможное число читаемых байтов
					// из-за чего пришлось вручную рассчитывать дозы
					if (waveFile.Read(inBuf, readed) != readed)
					{
						break; // если ошибка чтения - прервём цикл
					}

					int nDataLen = readed / wfx.nBlockAlign; // сколько сэмплов в буфере
					converted = ConvertSamples(wfx, inBuf, wfx.nAvgBytesPerSec, nDataLen);
				}
				while (nRest > 0);

				delete[] inBuf;
			}
			else
			{
				g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
			}
		}
		waveFile.Close();
		CalculateAverage();
	}

	return bRet;
}

/*
преобразование массива m_pWave из частоты nSrcSSR в nDstSSR
Цифровое. Не подходит для музыки.
Правильный ресемплинг с помощью БПФ превращает цифровой звук в
полное непотребство.
*/
void CTape::ResampleBuffer(int nSrcSSR, int nDstSSR)
{
	if (nSrcSSR == nDstSSR)
	{
		return;
	}

	SAMPLE_INT *pOldWave = m_pWave;
	int nOldWaveLen = m_nWaveLength;
	// новая длина в сэмплах
	int nNewLen = int(double(nOldWaveLen) * double(nDstSSR) / double(nSrcSSR)) + 1;
	m_pWave = nullptr;

	if (!AllocWaveBuffer(nNewLen)) // выделяем новый буфер
	{
		return;
	}

	m_nControlWaveLength = m_nWaveLength;
	m_nWaveLength = 0; // длину надо обнулить
	// формируем параметры входного wave
	WAVEFORMATEX wfx;
	ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nSamplesPerSec = nSrcSSR;
	wfx.wBitsPerSample = SAMPLE_INT_BPS;
	wfx.nChannels = m_nWorkingChannels;
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	int inBufLen = wfx.nAvgBytesPerSec;
	auto inBuf = new uint8_t[inBufLen]; // входной буфер в байтах

	if (!inBuf)
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

	SAMPLE_INT *pOldWavePtr = pOldWave; // указатель в буфере
	int nOldWaveLenByte = nOldWaveLen * wfx.nBlockAlign; // размер ваве в байтах

	while (nOldWaveLenByte > 0)
	{
		int readedBytes = nOldWaveLenByte > inBufLen ? inBufLen : nOldWaveLenByte; // сколько читать в байтах
		memcpy(inBuf, pOldWavePtr, readedBytes);
		nOldWaveLenByte -= inBufLen;
		pOldWavePtr += (readedBytes / SAMPLE_INT_SIZE);
		int nDataLen = readedBytes / wfx.nBlockAlign; // сколько сэмплов в буфере
		int converted = ConvertSamples(wfx, inBuf, inBufLen, nDataLen);
	}

	SAFE_DELETE_MEMORY(pOldWave);
}

/*
Конвертирование сэмплов из одной частоты в другую.
Вход: wfx_in - параметры входного wave.
inBuf - буфер кусочка входных данных
nBufSize - размер буфера в байтах
nDataLen - количество сэмплов в буфере
результирующий массив формируется в m_pWave
выход - количество обработанных сэмплов, должно быть равно nDataLen
*/
int CTape::ConvertSamples(WAVEFORMATEX wfx_in, void *inBuf, int nBufSize, int nDataLen)
{
	enum class RESAMPLE_OP : int {DNSAMPLE, REGULAR, UPSAMPLE};
	auto inBuf8 = reinterpret_cast<uint8_t *>(inBuf); // буфер для 8 битного звука
	auto inBuf16 = reinterpret_cast<SAMPLE_IO *>(inBuf); // буфер для 16 битного звука
	auto inBufFF = reinterpret_cast<SAMPLE_INT *>(inBuf); // буфер для внутреннего звука в плавающем формате
	int nDestBufferBasePos = m_nWaveLength; // начнём отсюда в буфере
	SAMPLE_INT sample;
	RESAMPLE_OP nOperation = RESAMPLE_OP::REGULAR;
	double dResCoeff = 1.0;

	if (m_nWorkingSSR != wfx_in.nSamplesPerSec)
	{
		dResCoeff = double(m_nWorkingSSR) / double(wfx_in.nSamplesPerSec); // коэффициент ресемплирования

		if (dResCoeff > 1.0)
		{
			nOperation = RESAMPLE_OP::UPSAMPLE;
		}
		else if (dResCoeff < 1.0)
		{
			nOperation = RESAMPLE_OP::DNSAMPLE;
		}
	}

	int pds = 0;

	// для каждого прочитавшегося сэмпла
	for (int nSourceSamplePos = 0; nSourceSamplePos < nDataLen; ++nSourceSamplePos)
	{
		// при необходимости делаем ресэмплинг
		auto nDestSamplePos = (nOperation == RESAMPLE_OP::REGULAR) ? nSourceSamplePos : int(double(nSourceSamplePos) * dResCoeff);

		// для каждого канала в сэмпле
		for (int channel = 0; channel < wfx_in.nChannels; ++channel)
		{
			// получаем сэмпл, и делаем его 16 битным
			switch (wfx_in.wBitsPerSample)
			{
				case 8:
					sample = (SAMPLE_INT(uint16_t(*inBuf8++) << 8) - FLOAT_BASE) / FLOAT_BASE; // unsigned 8 to signed 16
					break;

				case 16:
					sample = SAMPLE_INT(*inBuf16++) / FLOAT_BASE;
					break;

				case SAMPLE_INT_BPS:
					sample = *inBufFF++;
					break;

				// в WAVE_FORMAT_PCM может быть только 8 и 16 бит и 1 или 2 канала
				// в WAVE_FORMAT_EXTENSIBLE может быть ещё и 24 и 32-х битный звук и много каналов
				// вот как-то нафиг, неохота возиться с этим.
				default:
					sample = 0;
			}

			// если канал меньше максимально возможного
			if (channel < m_nWorkingChannels)
			{
				// при апсэмплинге размножаем сэмпл
				// при даунсэмплинге - не выполняем цикл, пока не придёт время,
				// т.е. пропускаем заданное количество сэмплов
				int nCurrDestSamplePos = pds;

				switch (nOperation)
				{
					case RESAMPLE_OP::DNSAMPLE:
						if (nCurrDestSamplePos == nDestSamplePos)
						{
							int pos = (nDestBufferBasePos + nCurrDestSamplePos) * m_nWorkingChannels + channel;

							if (pos < m_nControlWaveLength * m_nWorkingChannels)
							{
								// при аппроксимации самого последнего сэмпла, может получиться
								// что он выйдет за пределы массива.
								// как так получается, х.з.
								m_pWave[pos] = sample;
							}
						}

						break;

					case RESAMPLE_OP::REGULAR:
					{
						int pos = (nDestBufferBasePos + nDestSamplePos) * m_nWorkingChannels + channel;
						ASSERT(pos < m_nControlWaveLength * m_nWorkingChannels); // здесь такое не должно происходить, но на всякий случай
						m_pWave[pos] = sample;
					}
					break;

					case RESAMPLE_OP::UPSAMPLE:
						do
						{
							int pos = (nDestBufferBasePos + nCurrDestSamplePos) * m_nWorkingChannels + channel;

							if (pos < m_nControlWaveLength * m_nWorkingChannels)
							{
								// при аппроксимации самого последнего сэмпла, может получиться
								// что он выйдет за пределы массива.
								// как так получается, х.з.
								m_pWave[pos] = sample;
							}

							nCurrDestSamplePos++;
						}
						while (nCurrDestSamplePos <= nDestSamplePos);

						break;

					default:
						ASSERT(false);
						break;
				}
			}

			// все остальные каналы - игнорируем.
		}

		// если канал всего один, дублируем последний сэмпл на остальные возможные каналы (т.е. на второй).
		if (wfx_in.nChannels == 1)
		{
			int channel = 1;

			while (channel < m_nWorkingChannels)
			{
				int nCurrDestSamplePos = pds;

				switch (nOperation)
				{
					case RESAMPLE_OP::DNSAMPLE:
						if (nCurrDestSamplePos == nDestSamplePos)
						{
							int pos = (nDestBufferBasePos + nCurrDestSamplePos) * m_nWorkingChannels + channel;

							if (pos < m_nControlWaveLength * m_nWorkingChannels)
							{
								// при аппроксимации самого последнего сэмпла, может получиться
								// что он выйдет за пределы массива.
								// как так получается, х.з.
								m_pWave[pos] = sample;
							}
						}

						break;

					case RESAMPLE_OP::REGULAR:
					{
						int pos = (nDestBufferBasePos + nDestSamplePos) * m_nWorkingChannels + channel;
						ASSERT(pos < m_nControlWaveLength * m_nWorkingChannels); // здесь такое не должно происходить, но на всякий случай
						m_pWave[pos] = sample;
					}
					break;

					case RESAMPLE_OP::UPSAMPLE:
						do
						{
							int pos = (nDestBufferBasePos + nCurrDestSamplePos) * m_nWorkingChannels + channel;

							if (pos < m_nControlWaveLength * m_nWorkingChannels)
							{
								// при аппроксимации самого последнего сэмпла, может получиться
								// что он выйдет за пределы массива.
								// как так получается, х.з.
								m_pWave[pos] = sample;
							}

							nCurrDestSamplePos++;
						}
						while (nCurrDestSamplePos <= nDestSamplePos);

						break;

					default:
						ASSERT(false);
						break;
				}

				channel++;
			}
		}

		pds = nDestSamplePos + 1; // откуда начинать для следующего сэмпла
	}

	m_nWaveLength = nDestBufferBasePos + pds;

	if (m_nWaveLength > m_nControlWaveLength) // костыль
	{
		m_nWaveLength = m_nControlWaveLength;
	}

	return pds;
}



/*
Расчёт среднего уровня массива. Это будет среднее арифметическое.
Так считать дольше, но корректнее.
*/
void CTape::CalculateAverage()
{
	double avg = 0.0;

    for (register uint i = 0; i < m_nWaveLength; ++i)
	{
		avg += GetCurrentSampleMono(m_pWave, i);
	}

	m_nAverage = SAMPLE_INT(avg / m_nWaveLength);
}


bool CTape::LoadTmpFile(const CString &strPath)
{
	// внутри TMP файла - данные типа SAMPLE_INT с частотой 44100
	CFile tmpFile;

	if (!tmpFile.Open(strPath, CFile::modeRead))
	{
		return false;
	}

	auto ullLen = tmpFile.GetLength();

	if (ullLen > MAXINT32)
	{
		return false;
	}

	int nSampleSize = int(ullLen) / SAMPLE_INT_SIZE; // размер в сэмплах во всех каналах

	if (!AllocWaveBuffer(nSampleSize / BUFFER_CHANNELS))
	{
		return false;
	}

	auto pTmpBuf = new SAMPLE_INT[DEFAULT_SOUND_SAMPLE_RATE * BUFFER_CHANNELS];

	if (pTmpBuf)
	{
		m_nWaveLength = 0;
		// формируем параметры входного wave
		WAVEFORMATEX wfx;
		ZeroMemory(&wfx, sizeof(WAVEFORMATEX));
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nSamplesPerSec = DEFAULT_SOUND_SAMPLE_RATE;
		wfx.wBitsPerSample = SAMPLE_INT_BPS;
		wfx.nChannels = BUFFER_CHANNELS;
		wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels;
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		auto nRest = nSampleSize / BUFFER_CHANNELS; // сколько сэмплов преобразовать

		do
		{
			int nChunk = nRest > int(wfx.nSamplesPerSec) ? wfx.nSamplesPerSec : nRest; // сколько сэмплов возьмём
			nRest -= nChunk;
			tmpFile.Read(pTmpBuf, nChunk * SAMPLE_INT_BLOCKALIGN); // прочитаем

			if (wfx.nSamplesPerSec == m_nWorkingSSR)
			{
				memcpy(m_pWave + m_nWaveLength * BUFFER_CHANNELS, pTmpBuf, nChunk * SAMPLE_INT_BLOCKALIGN);
				m_nWaveLength += nChunk;
			}
			else
			{
				int converted = ConvertSamples(wfx, pTmpBuf, wfx.nAvgBytesPerSec, nChunk);
			}
		}
		while (nRest > 0);

		delete[] pTmpBuf;
	}

	tmpFile.Close();
	// Calculate average
	CalculateAverage();
	return true;
}

// такой недостаток - сохранение всегда с текущей частотой дискретизации.
// а нужно добавить опцию, с какой частотой сохранять.
// ибо не всегда нужно, цифровому ваве вполне достаточно 44100Гц.
bool CTape::SaveWaveFile(const CString &strPath)
{
	CFile waveFile;

	if (!waveFile.Open(strPath, CFile::modeCreate | CFile::modeReadWrite))
	{
		return false;
	}

	WAVEFORMATEX wfx;
	wfx.wFormatTag = WAVE_FORMAT_PCM;
	wfx.nChannels = m_nWorkingChannels;
	wfx.nSamplesPerSec = m_nWorkingSSR;
	wfx.wBitsPerSample = SAMPLE_IO_BPS;
	wfx.nBlockAlign = (wfx.wBitsPerSample >> 3) * wfx.nChannels; // количество байтов на сэмпл
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign; // сколько байтов в секунду проиграется
	wfx.cbSize = 0;
	DataHeader dataHeader;
	dataHeader.dataTag = DATA_TAG;
	dataHeader.dataSize = m_nWaveLength * wfx.nBlockAlign;
	WaveHeader waveHeader;
	waveHeader.riffTag = RIFF_TAG;
	waveHeader.size = sizeof(WaveHeader) + sizeof(WAVEFORMATEX) + sizeof(DataHeader) + dataHeader.dataSize;
	waveHeader.waveTag = WAVE_TAG;
	waveHeader.fmtTag = FMT_TAG;
	waveHeader.fmtSize = sizeof(WAVEFORMATEX);
	waveFile.Write(&waveHeader, sizeof(WaveHeader));
	waveFile.Write(&wfx, waveHeader.fmtSize);
	waveFile.Write(&dataHeader, sizeof(DataHeader));
	// сохраним ваве в формате 16 бит 2 канала, с заданной частотой дискретизации
	auto pTmpBuf = new SAMPLE_IO[wfx.nSamplesPerSec * wfx.nChannels];

	if (pTmpBuf)
	{
		int nRest = m_nWaveLength; // сколько сэмплов преобразовать
		int nWavePos = 0; // позиция в буфере m_pWave

		do
		{
			int nChunk = nRest > int(wfx.nSamplesPerSec) ? wfx.nSamplesPerSec : nRest; // сколько сэмплов возьмём
			nRest -= nChunk;

			for (int i = 0; i < nChunk; ++i)
			{
				for (int j = 0; j < wfx.nChannels; ++j)
				{
					pTmpBuf[i * wfx.nChannels + j] = SAMPLE_IO(m_pWave[nWavePos * wfx.nChannels + j] * FLOAT_BASE);
				}

				nWavePos++;
			}

			waveFile.Write(pTmpBuf, nChunk * wfx.nBlockAlign);
		}
		while (nRest > 0);

		delete[] pTmpBuf;
	}

	return true;
}

// упаковываем каждый сэмпл в битовый массив. 1 сэмпл - 1 бит
void CTape::PackBits(uint8_t *pPackBits, int nPackBitsLength)
{
	int pos = 0; // Текущая позиция упакованного байта
	uint8_t mask = 1; // Циклическая битовая маска
	ZeroMemory(pPackBits, nPackBitsLength);  // обнуляем массив, чтобы не выставлять вместе с 1 ещё и 0
	// преобразуем частоту дискретизации из текущей в 44100 (по умолчанию)
	ResampleBuffer(m_nWorkingSSR, DEFAULT_SOUND_SAMPLE_RATE);

	for (int i = 0; i < m_nWaveLength; ++i)
	{
		// Берём текущий сэмпл
		if (GetCurrentSampleMono(m_pWave, i) >= m_nAverage) // Если больше нуля, то выставляем 1, иначе пропускаем
		{
			pPackBits[pos] |= mask;
		}

		mask <<= 1; // сдвигаем маску на 1 влево

		if (!mask) // если маска опустела,
		{
			// то байт заполнился, надо дальше заново установить маску и переходить к следующему байту
			mask = 1;
			pos++;
		}
	}
}


void CTape::UnpackBits(uint8_t *pPackBits, int nPackBitsLength)
{
	// Выделяем память для распакованных данных, тут уже должно быть 2 канала
	AllocWaveBuffer(nPackBitsLength * 8);
	m_nWaveLength = 0;

	for (int i = 0; i < nPackBitsLength; ++i)
	{
		uint8_t mask = 1;

		// Разжимаем каждый байт в 8 сэмплов
		for (int b = 0; b < 8; ++b)
		{
			// из бита 1 делаем байт MAX_SAMPLE, из бита 0 делаем байт MIN_SAMPLE
			SAMPLE_INT sample = ((pPackBits[i] & mask) ? MAX_SAMPLE : MIN_SAMPLE) / FLOAT_BASE;
			SetCurrentSampleMono(m_pWave, m_nWaveLength, sample);
			mask <<= 1; // сдвигаем маску
			m_nWaveLength++;
		}
	}

	// преобразуем частоту дискретизации из 44100 (по умолчанию) в текущую
	ResampleBuffer(DEFAULT_SOUND_SAMPLE_RATE, m_nWorkingSSR);
}


int CTape::PackLZW_Fast(uint8_t *pPackBits, int nPackBitsLength, uint16_t *pPackLZW)
{
	/*
	pPackBits[0] = 'A';
	pPackBits[1] = 'B';
	pPackBits[2] = 'C';
	pPackBits[3] = 'A';
	pPackBits[4] = 'B';
	pPackBits[5] = 'C';
	pPackBits[6] = 'A';
	pPackBits[7] = 'B';
	pPackBits[8] = 'C';
	pPackBits[9] = 'A';
	pPackBits[10] = 'B';
	pPackBits[11] = 'C';
	pPackBits[12] = 'A';
	pPackBits[13] = 'B';
	pPackBits[14] = 'C';
	pPackBits[15] = 'A';
	pPackBits[16] = 'B';
	pPackBits[17] = 'C';
	pPackBits[18] = 'A';
	pPackBits[19] = 'B';
	pPackBits[20] = 'C';
	pPackBits[21] = 'A';
	pPackBits[22] = 'B';
	pPackBits[23] = 'C';
	pPackBits[24] = 'A';
	pPackBits[25] = 'B';
	pPackBits[26] = 'C';
	pPackBits[27] = 'A';
	pPackBits[28] = 'B';
	pPackBits[29] = 'C';
	*/
	// Инициализируем таблицы
	InitTables();
	int nPackLZWLength = 0;
	uint16_t last_code = BAD_CODE;
	int read_pos;

	for (read_pos = 0; read_pos < nPackBitsLength; ++read_pos)
	{
		int code = TableLookup_Fast(pPackBits, last_code, read_pos);  // Берём текущую последовательность

		if (code == BAD_CODE || read_pos == (nPackBitsLength - 1))
		{
			// Если нет в таблице, или уже конец
			pPackLZW[nPackLZWLength++] = last_code;  // Упаковка: последовательность - 1 байт
			AddWord_Fast(pPackBits, last_code, read_pos);  // Добавление: последовательность в таблицу
			last_code = pPackBits[read_pos];
		}
		else
		{
			last_code = code;
		}
	}

	pPackLZW[nPackLZWLength++] = pPackBits[read_pos - 1]; // Пакуем последний байт как есть
	return nPackLZWLength; // Возвращаем длину сжатого массива
}


void CTape::UnpackLZW_Fast(uint8_t *pPackBits, int nPackBitsLength, uint16_t *pPackLZW, int nPackLZWLength)
{
	// Инициализируем таблицы
	InitTables();
	// Очищаем будущий распакованный массив. я так понимаю, что nPackBitsLength - это размер будущих распакованных данных
	ZeroMemory(pPackBits, nPackBitsLength);
	nPackBitsLength = 0;
	int unpack_pos = 0;
	/*
	m_pPackLZW[0] = 'A';
	m_pPackLZW[1] = 'B';
	m_pPackLZW[2] = 'C';
	m_pPackLZW[3] = 257;
	m_pPackLZW[4] = 259;
	m_pPackLZW[5] = 258;
	m_pPackLZW[6] = 260;
	m_pPackLZW[7] = 263;
	m_pPackLZW[8] = 262;
	m_pPackLZW[9] = 265;
	m_pPackLZW[10] = 261;
	m_pPackLZW[11] = 267;
	*/
	uint16_t last_code = BAD_CODE;

	for (int read_pos = 0; read_pos < nPackLZWLength; ++read_pos)
	{
		if (pPackLZW[read_pos] < 256) // Если текущий код < 256, распакуем его как есть
		{
			pPackBits[nPackBitsLength++] = uint8_t(pPackLZW[read_pos]);
		}
		else
		{
			// Иначе создаём таблицу от текущей позиции до позиции этого кода
			while (unpack_pos < nPackBitsLength)
			{
				int code = TableLookup_Fast(pPackBits, last_code, unpack_pos);

				if (code == BAD_CODE)
				{
					// Если нет текущей последовательности в таблице, добавим её
					AddWord_Fast(pPackBits, last_code, unpack_pos);
					last_code = pPackBits[unpack_pos];
				}
				else
				{
					last_code = code;
				}

				unpack_pos++;
			}

			if (!UnpackWord(pPackBits, nPackBitsLength, pPackLZW[read_pos]))  // Пробуем распаковать текущий код
			{
				// Если нет в таблице (случай XиX)
				int first_pos = nPackBitsLength;
				UnpackWord(pPackBits, nPackBitsLength, last_code);  // Распаковываем последний известный код
				pPackBits[nPackBitsLength++] = TableLookup_Fast(pPackBits, BAD_CODE, first_pos);  // Распаковываем последний известный код первого байта
				AddWord_Fast(pPackBits, last_code, unpack_pos);  // Добавление: Xпоследовательность + X в таблицу
				last_code = BAD_CODE;
			}
		}
	}
}


void CTape::InitTables()
{
	for (auto &y : m_pScanTable)
	{
		// т.к. эту функцию вызываем много раз, то выделять память надо только в первый раз
		if (!y)
		{
			y = new uint16_t[258];

			if (!y)
			{
				g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
			}
		}

		// все остальные разы - только заполнять таблицы начальными данными
		for (int x = 0; x < 258; ++x)
		{
			y[x] = BAD_CODE;
		}
	}

	for (int y = 0; y < 256; ++y)
	{
		m_pScanTable[y][CODE] = y;
	}

	m_nScanTableSize = CODE;
}

int CTape::TableLookup_Fast(uint8_t *pPackBits, uint16_t prefix, int end_pos)
{
	if (prefix == BAD_CODE)
	{
		return pPackBits[end_pos];
	}

	return m_pScanTable[prefix][pPackBits[end_pos]];
}


void CTape::AddWord_Fast(uint8_t *pPackBits, uint16_t prefix, int end_pos)
{
	// если размер таблицы превысил все допустимые рамки
	if (m_nScanTableSize >= MAX_TABLE_SIZE)
	{
		// обнулим его, и начнём заполнять сначала
		InitTables();
		return;
	}

	m_pScanTable[prefix][pPackBits[end_pos]] = m_nScanTableSize;
	m_pScanTable[m_nScanTableSize][PREFIX] = prefix;
	m_pScanTable[m_nScanTableSize][CODE] = pPackBits[end_pos];
	m_nScanTableSize++;
}


bool CTape::UnpackWord(uint8_t *pPackBits, int &nPackBitsLength, uint16_t code)
{
	if (code == BAD_CODE)
	{
		return false;
	}

	if (code < 256)
	{
		pPackBits[nPackBitsLength++] = LOBYTE(code);
		return true;
	}

	if (!UnpackWord(pPackBits, nPackBitsLength, m_pScanTable[code][PREFIX]))
	{
		return false;
	}

	pPackBits[nPackBitsLength++] = (uint8_t)m_pScanTable[code][CODE];
	return true;
}


bool CTape::LoadMSFFile(const CString &strPath, bool bSilent)
{
	CMSFManager msf;

	if (!msf.OpenFile(strPath, true, bSilent))
	{
		return false;
	}

	DWORD nPackBitsLength, nPackLZWLength;

	if (!msf.GetBlockTape(nullptr, &nPackBitsLength, &nPackLZWLength))
	{
		return false;
	}

	bool bRet = false;
	auto pPackBits = new uint8_t[nPackBitsLength];
	auto pPackLZW = new uint16_t[nPackLZWLength];

	if (pPackBits && pPackLZW)
	{
		bRet = msf.GetBlockTape(reinterpret_cast<uint8_t *>(pPackLZW), &nPackBitsLength, &nPackLZWLength);

		if (bRet)
		{
			UnpackLZW_Fast(pPackBits, nPackBitsLength, pPackLZW, nPackLZWLength / 2);
			UnpackBits(pPackBits, nPackBitsLength);
			// Calculate average
			CalculateAverage();
		}

		// чистим память за собой
		SAFE_DELETE_ARRAY(pPackBits);
		SAFE_DELETE_ARRAY(pPackLZW);
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

	return bRet;
}


bool CTape::SaveMSFFile(const CString &strPath)
{
	CMSFManager msf;

	if (!msf.OpenFile(strPath, false))
	{
		return false;
	}

	bool bRet = false;
	// получим длину в битах из длины в сэмплах.
	int nPackBitsLength = (m_nWaveLength + 7) >> 3;
	auto pPackBits = new uint8_t[nPackBitsLength];
	auto pPackLZW = new uint16_t[nPackBitsLength];

	if (pPackBits && pPackLZW)
	{
		PackBits(pPackBits, nPackBitsLength); // пакуем биты один сэмпл умещается в один бит
		int nPackLZW = PackLZW_Fast(pPackBits, nPackBitsLength, pPackLZW) * 2;
		bRet = msf.SetBlockTape(reinterpret_cast<uint8_t *>(pPackLZW), nPackBitsLength, nPackLZW);
		// чистим память за собой
		SAFE_DELETE_ARRAY(pPackBits);
		SAFE_DELETE_ARRAY(pPackLZW);
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}

	return bRet;
}

bool CTape::GetWaveFile(TAPE_FILE_INFO *pTfi, bool bHeaderOnly)
{
	if (!m_pWave)
	{
		return false;
	}

	m_nPos = 0;     // начнём сначала
	m_bInverse = false;
	memset(pTfi, 255, sizeof(TAPE_FILE_INFO));
	int nLength = 0;

	// ищем настроечную последовательность в начале файла
	if (!FindTuning(1024, pTfi->start_tuning, nLength))
	{
		return false;
	}

	pTfi->synchro_start = m_nPos;

	// ищем маркер
	if (!FindMarker1())
	{
		return false;
	}

	// ищем настроечную последовательность в начале заголовка файла
	if (!FindTuning(7, pTfi->marker1, nLength))
	{
		return false;
	}

	pTfi->synchro_header = m_nPos;

	// ищем маркер
	if (!FindMarker())
	{
		return false;
	}

	pTfi->header = m_nPos;

	// читаем заголовок файла
	for (int i = 0; i < 20; ++i)
	{
		if (!ReadByte(((uint8_t *)&pTfi->address)[i]))
		{
			return false;
		}
	}

	// ищем настроечную последовательность в начале массива данных файла
	if (!FindTuning(7, pTfi->marker2, nLength))
	{
		return false;
	}

	pTfi->synchro_data = m_nPos;

	// ищем маркер
	if (!FindMarker())
	{
		return false;
	}

	if (bHeaderOnly)
	{
		return true;
	}

	SAFE_DELETE_ARRAY(m_pBin);
	m_pBin = new uint8_t[pTfi->length];

	if (!m_pBin)
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
		return false;
	}

	pTfi->data = m_nPos;

	// читаем данные
	for (int i = 0; i < pTfi->length; ++i)
	{
		if (!ReadByte(m_pBin[i]))
		{
			return false;
		}
	}

	// читаем контрольную сумму
	if (!ReadByte(((uint8_t *)&pTfi->crc)[0]))
	{
		return false;
	}

	if (!ReadByte(((uint8_t *)&pTfi->crc)[1]))
	{
		return false;
	}

	pTfi->synchro6 = m_nPos;

	// ищем финишную последовательность
	if (FindSyncro6())
	{
		FindTuning(128, pTfi->end_tuning, nLength);
		pTfi->end = pTfi->end_tuning + nLength;
	}
	else
	{
		// её может и не быть в нестандартных случаях
		pTfi->end = m_nWaveLength;
	}

	return true;
}

/*
 * Вход: nLength - ориентировочная длина искомой последовательности
 * Выход: wave_pos - начало найденной последовательности
 *        wave_length - длина найденной последовательности
 */
bool CTape::FindTuning(int nLength, DWORD &wave_pos, int &wave_length)
{
	m_dAvgLength = 0;
	bool bFound = false;
	int syncro_pos = BAD_LENGTH;
	int last_pos = m_nPos;
	int imp_num = 0;
	int begin_pos = m_nPos;
	int nLastLen = 0;

	for (;;)
	{
		last_pos = m_nPos; // позиция перед импульсом
		int len = CalcImpLength(m_pWave, m_nPos, m_nWaveLength); // считаем длину импульса в сэмплах

		if (len == BAD_LENGTH)  // если не получилось посчитать
		{
			m_nPos = last_pos;  // откатываемся на позицию перед подсчётом
			return false;       // и выходим
		}

		register int a = nLastLen - len; // вычисляем разницу в длине импульсов

		if (a < 0)
		{
			a = -a;     // берём модуль
		}

		if (a <= 3)     // если разница меньше разброса длительностей
		{
			// то это правильный импульс, будем с ним работать
			m_dAvgLength += double(len);    // прибавим к средней длительности, чтоб потом вычислить
			imp_num++;              // посчитаем этот импульс

			if (imp_num >= nLength && !bFound) // если количество правильных импульсов достигло ориентировочной длины, и ещё не зафиксировали
			{
				syncro_pos = begin_pos; // фиксируем начало синхропоследовательности
				bFound = true;      // отмечаем факт
			}
		}
		else if (!bFound)   // импульс неправильный и ещё начало не нашли
		{
			m_dAvgLength = 0.0; // то всё отменяем
			imp_num = 0;
			begin_pos = m_nPos; // сдвигаем предполагаемое начало синхропоследовательности
		}
		else    // импульс неправильный, а начало мы уже зафиксировали
		{
			break;  // выходим из цикла, наверно до маркера дошли
		}

		nLastLen = len; // текущую длину -> предыдущую длину
	}

	m_nPos = last_pos;  // откатываемся к предыдущему импульсу
	wave_pos = syncro_pos; // выдаём найденное начало синхропоследовательности
	wave_length = m_nPos - syncro_pos; // выдаём длительность найденной синхропоследовательности

	if (imp_num)        // если есть что считать
	{
		m_dAvgLength = m_dAvgLength / imp_num; // вычисляем среднюю длину с округлением в большую сторону
	}

	return true;
}

// разброс параметров относительно идеала
// тут главное не переборщить с допуском
constexpr double dDelta = 0.3;
constexpr double MinusDelta(double x)
{
	return x - x * dDelta;
}
constexpr double PlusDelta(double x)
{
	return x + x * dDelta;
}

// возвращаем длину импульса в средних импульсах
// 0 - очень короткий импульс (неправильный или ложный)
// 1 - "0" и синхроимпульс, их длительность == L (длина среднего импульса)
// 2 - "1", его длительность == 2L
// 4 - маркер, его длительность  == 4L
// 8 - очень длинный импульс (или финиш, или неправильный)
int CTape::DefineLength(int nLength)
{
	double l = double(nLength) / m_dAvgLength;

	if (l < MinusDelta(1.0))
	{
		return 0;
	}
	else if (l < PlusDelta(1.0))
	{
		return 1;
	}
	else if (l < PlusDelta(2.0)) // упрощённые интервалы, без разрывов между допусками.
	{
		return 2;
	}
	else if (l < PlusDelta(4.0))
	{
		return 4;
	}

	return 8;
}

// поиск первого маркера
bool CTape::FindMarker1()
{
	int size = 0;
    uint pos = m_nPos;

	for (;;)
	{
		int len_1 = 0;

		// Считаем количество единиц
		while (GetCurrentSampleMono(m_pWave, pos) >= m_nAverage)
		{
			if (pos >= m_nWaveLength)
			{
				return false;
			}

			len_1++;
			pos++;
		}

		double l = double(len_1) / m_dAvgLength;
#ifdef _DEBUG
		constexpr double low = MinusDelta(4.0) / 2;
		constexpr double high = PlusDelta(4.0) / 2;
#endif

		if (MinusDelta(4.0) / 2 <= l && l <= PlusDelta(4.0) / 2)
		{
			break;
		}

		int len_0 = 0;

		// Считаем количество нулей
		while (GetCurrentSampleMono(m_pWave, pos) < m_nAverage)
		{
			if (pos >= m_nWaveLength)
			{
				return false;
			}

			len_0++;
			pos++;
		}

		l = double(len_0) / m_dAvgLength;

		if (MinusDelta(4.0) / 2 <= l && l <= PlusDelta(4.0) / 2)
		{
			m_bInverse = true;
			break;
		}
	}

	m_nPos = pos;
	bool bBit;

	if (ReadBit(bBit))
	{
		return bBit;    // в правильном маркере бит - "1"
	}

	return false;
}

bool CTape::FindMarker()
{
	int size = 0;
	int nLength = 0;

	for (;;)
	{
		nLength = CalcImpLength(m_pWave, m_nPos, m_nWaveLength);

		if (nLength == BAD_LENGTH)
		{
			return false;
		}

		size = DefineLength(nLength);

		if (size >= 4)
		{
			break;
		}
	}

	bool bBit;

	if (ReadBit(bBit))
	{
		return bBit;    // в правильном маркере бит - "1"
	}

	return false;
}


bool CTape::FindSyncro6()
{
	int nLength = CalcImpLength(m_pWave, m_nPos, m_nWaveLength);

	if (nLength == BAD_LENGTH)
	{
		return false;
	}

	int size = DefineLength(nLength);
	return (size >= 6);
}


bool CTape::ReadBit(bool &bBit)
{
	int oldPos = m_nPos;
	int length0 = CalcImpLength(m_pWave, m_nPos, m_nWaveLength); // длина информационного импульса

	if (length0 == BAD_LENGTH)
	{
		m_nPos = oldPos;
		return false;
	}

	int length1 = CalcImpLength(m_pWave, m_nPos, m_nWaveLength); // длина синхроимпульса за инфоимпульсом

	if (length1 == BAD_LENGTH)
	{
		m_nPos = oldPos;
		return false;
	}

	int size0 = DefineLength(length0);
//  int size1 = DefineLength(length1); // не имеет значения

	if (size0 == 1)
	{
		bBit = false;
		return true;
	}
	else if (size0 == 2)
	{
		bBit = true;
		return true;
	}

	m_nPos = oldPos; // при неудаче откатываемся к началу бита
	return false;
}


bool CTape::ReadByte(uint8_t &byte)
{
	// int oldPos = m_nPos;
	uint8_t mask = 1;
	byte = 0;
	bool bBit;

	for (int i = 0; i < 8; ++i)
	{
		if (!ReadBit(bBit))
		{
			// m_nPos = oldPos; // при неудаче откатываемся к началу байта
			// это не очень полезно при анализе, где встретился плохой бит.
			return false;
		}

		if (bBit)
		{
			byte |= mask;
		}

		mask <<= 1;
	}

	return true;
}

// на входе: pWave - указатель на массив, pos - позиция в сэмплах
SAMPLE_INT CTape::GetCurrentSampleMono(register SAMPLE_INT *pWave, register int pos)
{
	register int n = pos * m_nWorkingChannels;
	register SAMPLE_INT s = 0.0;

    for (register uint chan = 0; chan < m_nWorkingChannels; ++chan)
	{
		// микшируем
		register SAMPLE_INT t = s * pWave[n];
		s += pWave[n];

		if (t > 0.0) // если множители были одного знака
		{
			if (pWave[n] > 0.0) // если оба положительные
			{
				s -= t;
			}
			else // если оба отрицательные
			{
				s += t;
			}
		}

		// если разного или 0, то просто сумма
		n++;
	}

	return s;
}

// на входе: pWave - указатель на массив, pos - позиция в сэмплах
void CTape::SetCurrentSampleMono(SAMPLE_INT *pWave, int pos, SAMPLE_INT sample)
{
    register uint n = pos * m_nWorkingChannels;

    for (register uint chan = 0; chan < m_nWorkingChannels; ++chan)
	{
		pWave[n++] = sample; // размножаем сэмпл по каналам
	}
}

//////////////////////////////////////////////////////////////////////
// Вход: pos - текущая позиция в сэмплах
//       nLength - длина wave в сэмплах, предел, дальше которого нельзя
// Выход: длина импульса в сэмплах
int CTape::CalcImpLength(SAMPLE_INT *pWave, int &pos, int nLength)
{
	if (pos >= nLength)
	{
		return BAD_LENGTH;
	}

	int len_1 = 0;
	int len_0 = 0;

	if (m_bInverse)
	{
		// Пропускаем инверсные импульсы
		while (GetCurrentSampleMono(pWave, pos) >= m_nAverage)
		{
			if (++pos >= nLength)
			{
				return BAD_LENGTH;
			}
		}

		// Считаем количество единиц
		while (GetCurrentSampleMono(pWave, pos) < m_nAverage)
		{
			len_1++;

			if (++pos >= nLength)
			{
				return BAD_LENGTH;
			}
		}

		// Считаем количество нулей
		while (GetCurrentSampleMono(pWave, pos) >= m_nAverage)
		{
			len_0++;

			if (++pos >= nLength)
			{
				return BAD_LENGTH;
			}
		}

		return len_1 + len_0;
	}
	else
	{
		// Пропускаем инверсные импульсы
		while (GetCurrentSampleMono(pWave, pos) < m_nAverage)
		{
			if (++pos >= nLength)
			{
				return BAD_LENGTH;
			}
		}

		// Считаем количество единиц
		while (GetCurrentSampleMono(pWave, pos) >= m_nAverage)
		{
			len_1++;

			if (++pos >= nLength)
			{
				return BAD_LENGTH;
			}
		}

		// Считаем количество нулей
		while (GetCurrentSampleMono(pWave, pos) < m_nAverage)
		{
			len_0++;

			if (++pos >= nLength)
			{
				return BAD_LENGTH;
			}
		}

		return len_1 + len_0;
	}
}


uint16_t CTape::CalcCRC(TAPE_FILE_INFO *pTfi)
{
	if (!m_pBin)
	{
		return false;
	}

	int crc = 0;

	for (int i = 0; i < pTfi->length; ++i)
	{
		crc += m_pBin[i];

		if (crc & 0xFFFF0000)
		{
			// если случился перенос в 17 разряд (т.е. бит С для word)
			crc &= 0x0000FFFF; // его обнулим
			crc++; // но прибавим к сумме
		}
	}

	return LOWORD(crc);
}


bool CTape::LoadBinFile(const CString &strPath, TAPE_FILE_INFO *pTfi)
{
	CFile binFile;

	if (!binFile.Open(strPath, CFile::modeRead))
	{
		return false;
	}

	// читаем первое слово - адрес загрузки
	if (binFile.Read(&pTfi->address, sizeof(uint16_t)) != sizeof(uint16_t))
	{
		return false;
	}

	// читаем второе слово, длину.
	if (binFile.Read(&pTfi->length, sizeof(uint16_t)) != sizeof(uint16_t))
	{
		return false;
	}

	// проверим настоящий бин файл или просто расширение совпало.
	ULONGLONG ulllen = binFile.GetLength(); // размер файла

	if (ulllen > 65535)
	{
		// слишком большой файл, незнай что делать, пока будем просто выходить
		return false;
	}

	auto len = uint16_t(ulllen);
	bool bIsCRC = false;

	if (pTfi->length == len - 4)
	{
		bIsCRC = false;
	}
	else if (pTfi->length == len - 6)
	{
		bIsCRC = true;
	}
	else if (pTfi->length == len - 22)
	{
		bIsCRC = true;
		// а если сложный, надо пропустить немного ненужных данных
		binFile.Seek(20, CFile::begin);
	}
	else
	{
		// return false;
		// не бин, но всё равно будем читать такой файл
		binFile.Seek(0, CFile::begin); // перемещаемся в начало
		pTfi->length = len;
		pTfi->address = 0;
	}

	SAFE_DELETE_ARRAY(m_pBin);
	m_pBin = new uint8_t[pTfi->length];

	if (!m_pBin)
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
		return false;
	}

	if (binFile.Read(m_pBin, pTfi->length) != pTfi->length)
	{
		return false;
	}

	// прочитаем контрольную сумму, если она там есть
	if (bIsCRC && binFile.Read(&pTfi->crc, sizeof(uint16_t)) == sizeof(uint16_t))
	{
		if (CalcCRC(pTfi) != pTfi->crc)
		{
			TRACE("CRC Mismatch!\n");
		}
	}
	else
	{
		pTfi->crc = CalcCRC(pTfi);
	}

	CString strName = ::GetFileName(strPath);
	CString strExt = ::GetFileExt(strPath);

	if (!strExt.CompareNoCase(CString(MAKEINTRESOURCE(IDS_FILEEXT_BINARY))))
	{
		strName = ::GetFileTitle(strPath);
	}

	// кроме всего этого, тут надо отлавливать бейсиковские файлы. у них свой формат имени.
	// 6 символов - имя (пустое место забивается пробелами), потом .COD, .BIN, .ASC
	// а потом конец забивается нулями, а не пробелами.
	// .ASC обрабатывать не будем. .ASC нормальные люди не используют
	strExt = ::GetFileExt(strName).MakeUpper();

	if (strExt == _T(".COD")) // если это бейсиковский .COD
	{
		memset(pTfi->name, 0, 16);
		strName = ::GetFileTitle(strName);
		// преобразуем юникодную строку в бкашную
		UNICODEtoBK(strName, pTfi->name, 6, true);
		pTfi->name[6] = '.';
		pTfi->name[7] = 'C';
		pTfi->name[8] = 'O';
		pTfi->name[9] = 'D';
	}
	else if (strExt == _T(".BIN")) // если это бейсиковский .BIN
	{
		memset(pTfi->name, 0, 16);
		strName = ::GetFileTitle(strName);
		// преобразуем юникодную строку в бкашную
		UNICODEtoBK(strName, pTfi->name, 6, true);
		pTfi->name[6] = '.';
		pTfi->name[7] = 'B';
		pTfi->name[8] = 'I';
		pTfi->name[9] = 'N';
	}
	else
	{
		// преобразуем юникодную строку в бкашную
		UNICODEtoBK(strName, pTfi->name, 16, true);
	}

	SAFE_DELETE_MEMORY(m_pWave); // m_pWave = 0
	m_nPos = 0; // тут будет формироваться длина

	if (!SetWaveFile(pTfi)) // рассчитываем размер массива
	{
		return false;
	}

	AllocWaveBuffer(m_nPos); // выделяем память под массив
	m_nPos = 0;

	if (!SetWaveFile(pTfi)) // и теперь генерируем файл по настоящему.
	{
		return false;
	}

	// посчитаем средний уровень проще. массив-то полностью цифровой
	// m_nAverage = AVG_SAMPLE;
	// посчитаем полноценно
	CalculateAverage();
	return true;
}


bool CTape::SaveBinFile(const CString &strPath, TAPE_FILE_INFO *pTfi)
{
	if (!m_pBin)
	{
		return false;
	}

	CFile binFile;

	if (!binFile.Open(strPath, CFile::modeCreate | CFile::modeWrite))
	{
		return false;
	}

	binFile.Write(&pTfi->address, sizeof(uint16_t));
	binFile.Write(&pTfi->length, sizeof(uint16_t));

	if (g_Config.m_bUseLongBinFormat)
	{
		binFile.Write(&pTfi->name, 16);
	}

	binFile.Write(m_pBin, pTfi->length);

	if (g_Config.m_bUseLongBinFormat)
	{
		binFile.Write(&pTfi->crc, sizeof(uint16_t));
	}

	return true;
}


bool CTape::SetWaveFile(TAPE_FILE_INFO *pTfi)
{
	m_nPos = 0;
	m_nAvgLength = 15;
	ASSERT(m_nAvgLength & 1);

	if (!SaveTuning(4096))
	{
		return false;
	}

	if (!SaveTuning(8))
	{
		return false;
	}

	for (int i = 0; i < 20; ++i)
	{
		if (!SaveByte(((uint8_t *)&pTfi->address)[i]))
		{
			return false;
		}
	}

	if (!SaveTuning(8))
	{
		return false;
	}

	for (int i = 0; i < pTfi->length; ++i)
	{
		if (!SaveByte(m_pBin[i]))
		{
			return false;
		}
	}

	pTfi->crc = CalcCRC(pTfi);

	if (!SaveByte(((uint8_t *)&pTfi->crc)[0]))
	{
		return false;
	}

	if (!SaveByte(((uint8_t *)&pTfi->crc)[1]))
	{
		return false;
	}

	if (!SaveSyncro6())
	{
		return false;
	}

	if (!SaveTuning(256))
	{
		return false;
	}

	return true;
}


bool CTape::SaveTuning(int nLength)
{
	for (int i = 0; i < nLength; ++i)
	{
		if (!SaveImp(1))
		{
			return false;
		}
	}

	// запись маркера
	if (!SaveImp(4))
	{
		return false;
	}

	if (!SaveBit(true))
	{
		return false;
	}

	return true;
}



bool CTape::SaveSyncro6()
{
	if (!SaveImp(12))
	{
		return false;
	}

	return true;
}


bool CTape::SaveImp(int size)
{
	if (m_pWave)
	{
		for (int i = 0; i < size * (m_nAvgLength / 2 + 1); ++i)
		{
			if (m_nPos >= m_nWaveLength)
			{
				return false;
			}

			SetCurrentSampleMono(m_pWave, m_nPos++, double(MAX_SAMPLE) / FLOAT_BASE);
		}

		for (int i = 0; i < size * (m_nAvgLength / 2); ++i)
		{
			if (m_nPos >= m_nWaveLength)
			{
				return false;
			}

			SetCurrentSampleMono(m_pWave, m_nPos++, double(MIN_SAMPLE) / FLOAT_BASE);
		}
	}
	else
	{
		m_nPos += size * m_nAvgLength;
	}

	return true;
}


bool CTape::SaveBit(bool bBit)
{
	if (bBit)
	{
		if (!SaveImp(2))
		{
			return false;
		}
	}
	else
	{
		if (!SaveImp(1))
		{
			return false;
		}
	}

	if (!SaveImp(1))
	{
		return false;
	}

	return true;
}


bool CTape::SaveByte(uint8_t byte)
{
	uint8_t mask = 1;

	for (int i = 0; i < 8; ++i)
	{
		if (!SaveBit(!!(byte & mask)))
		{
			return false;
		}

		mask <<= 1;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
// на входе, nLenInSamples - размер в сэмплах
bool CTape::AllocWaveBuffer(int nLenInSamples)
{
	if (m_pWave == nullptr || m_nWaveLength < nLenInSamples)
	{
		SAFE_DELETE_MEMORY(m_pWave);
		m_pWave = (SAMPLE_INT *)malloc(nLenInSamples * m_nSampleBlockAlign);

		if (!m_pWave)
		{
			m_nWaveLength = 0;
			return false;
		}
	}

	m_nWaveLength = nLenInSamples;
	m_nAverage = MIN_SAMPLE;
	memset(m_pWave, 0, nLenInSamples * m_nSampleBlockAlign);
	return true;
}


bool CTape::LoadBuffer(SAMPLE_INT *pBuff, int nLenInSamples)
{
	if (!AllocWaveBuffer(nLenInSamples))
	{
		return false;
	}

	int nSize = nLenInSamples * m_nSampleBlockAlign;
	memcpy(m_pWave, pBuff, nSize);
	CalculateAverage();
	return true;
}


/*
вход: pBuff - буфер, куда передаётся tape, формат данных в буфере - 16бит 2 канала
nBufSampleLen - размер буфера в сэмплах.
*/
void CTape::PlayWaveGetBuffer(SAMPLE_INT *pBuff, int nBufSampleLen)
{
	ZeroMemory(pBuff, nBufSampleLen * m_nSampleBlockAlign);

	if (m_bPlay)
	{
		// m_nWaveLength и m_nPlayPos - измеряются в сэмплах
		register int nLength = m_nWaveLength - m_nPlayPos;

		if (nLength > nBufSampleLen)    // если длина больше чем буфер,
		{
			nLength = nBufSampleLen;    // уменьшим до длины буфера
		}

		register int nByteLength = nLength * m_nSampleBlockAlign;
		memcpy(pBuff, m_pWave + m_nPlayPos * BUFFER_CHANNELS, nByteLength);
		m_nPlayPos += nLength;

		if (m_nPlayPos >= m_nWaveLength)
		{
			StopPlay();
			m_nPlayPos = 0;
			m_bWaveLoaded = false; // больше не воспроизводить
		}
	}
}

/*
вход: pBuff - буфер, откуда берётся tape, формат данных в буфере - 16 бит 2 канала
nBufSampleLen - размер буфера в сэмплах.
*/
void CTape::RecordWaveGetBuffer(SAMPLE_INT *pBuff, int nBufSampleLen)
{
	if (m_bRecord)
	{
		// посчитаем размер буфера в байтах
		register int nBufLen = nBufSampleLen * m_nSampleBlockAlign;

		// если место в буфере кончается, надо сделать новый буфер, побольше, и данные из старого скопировать в новый
		while (m_nRecordPos + nBufSampleLen >= m_nRecordLength)
		{
			m_nRecordLength += RECORD_GROW;
			m_pRecord = (SAMPLE_INT *)realloc(m_pRecord, m_nRecordLength * m_nSampleBlockAlign);
			ASSERT(m_pRecord);
		}

		memcpy(m_pRecord + m_nRecordPos * m_nWorkingChannels, pBuff, nBufLen);
		/*
		Ещё тут надо высчитывать m_fAverage, потому что функция FindRecordBegin использует это значение
		причём, среднее значение в пределах заданного буфера вполне подойдёт.
		*/
		auto pSrcPos = pBuff;
		SAMPLE_INT avg = 0.0;

		for (register int i = 0; i < nBufSampleLen; ++i)
		{
			avg += GetCurrentSampleMono(pSrcPos, i);
		}

		m_nAverage = SAMPLE_INT(avg / nBufSampleLen);

		if (FindRecordEnd(nBufSampleLen))
		{
			StopRecord();
		}

		if (FindRecordBegin(nBufSampleLen))  // Если нашли точку начала записи
		{
			m_nRecordPos += nBufSampleLen;
		}
	}
}

void CTape::StartRecord(bool bAutoBeginRecord, bool bAutoEndRecord)
{
	m_bAutoBeginRecord = bAutoBeginRecord;
	m_bAutoEndRecord = bAutoEndRecord;
	m_nAverage = MIN_SAMPLE;
	m_nRecordPos = 0;
	SAFE_DELETE_MEMORY(m_pRecord);
	const register auto nBufSize = RECORD_BUFFER * m_nSampleBlockAlign;
	m_pRecord = reinterpret_cast<SAMPLE_INT *>(malloc(nBufSize)); // создаём новый массив длиной RECORD_BUFFER сэмплов, каждый сэмпл - m_nWorkingChannels * SAMPLE_INT_SIZE
	ZeroMemory(m_pRecord, nBufSize);
	m_nRecordLength = RECORD_BUFFER; // длина массива в сэмплах
	m_bRecord = true;
}


void CTape::StopRecord()
{
	if (m_bRecord)
	{
		m_bRecord = false;
		SAFE_DELETE_MEMORY(m_pWave);
		m_pWave = m_pRecord;
		m_pRecord = nullptr;
		m_nWaveLength = m_nRecordPos;
		m_nRecordPos = 0;
		CalculateAverage();
	}
}


//////////////////////////////////////////////////////////////////////
// nSmplBufLen - длина буфера в сэмплах
bool CTape::FindRecordBegin(int nSmplBufLen)
{
	int nLastLen = BAD_LENGTH;

	if (m_bAutoBeginRecord)
	{
		m_nRecordPos = 0;
		register int imp_num = 0;

		for (;;)
		{
			register int len = CalcImpLength(m_pRecord, m_nRecordPos, nSmplBufLen);

			// тут len - в сэмплах
			if (len == BAD_LENGTH)
			{
				m_nRecordPos = 0;
				return false;
			}

			register int a = nLastLen - len;

			if (a < 0) // если новая длина будет больше старой, то получится отрицательное значение
			{
				a = -a; // а нам важен модуль а не величина
			}

			if (a <= 3)
			{
				if (++imp_num >= 50)
				{
					break;
				}
			}
			else
			{
				imp_num = 0;
			}

			nLastLen = len;
			// TRACE ("\nWave pos %06i, Start = %i, Len = %i, Imp_Num = %i", m_nPos, begin_pos, len, imp_num);
			// Sleep (1);
		}

		TRACE("AutoSearch: File Start Found!\n");
		m_nRecordPos = 0;
		m_bAutoBeginRecord = false; // признак, что найдено начало записи при автопоиске
	}

	return true;
}


bool CTape::FindRecordEnd(int buffLength)
{
	// автоматически ищем конец записи только если было автоматически найдено начало,
	// или вообще не был задан автопоиск начала.
	if (!m_bAutoBeginRecord && m_bAutoEndRecord)
	{
		register SAMPLE_INT last_sample = -1;
		register int last_length = 0;
		register int nEnd = m_nRecordPos + buffLength;

		for (register int i = m_nRecordPos; i < nEnd; ++i)
		{
			register SAMPLE_INT sample = GetCurrentSampleMono(m_pRecord, i);

			if (sample == last_sample)
			{
				last_length++;
			}
			else
			{
				last_sample = sample;
				last_length = 0;
			}

			if (last_length >= 500)
			{
				m_bAutoEndRecord = false;
				TRACE("AutoSearch: File End Found!\n");
				return true;
			}
		}
	}

	return false;
}
