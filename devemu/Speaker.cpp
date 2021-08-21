// Speaker.cpp: implementation of the CSpeaker class.
//


#include "pch.h"
#include "Config.h"
#include "Speaker.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CSpeaker::CSpeaker()
	: m_pRecieveTapeSamples(nullptr)
	, m_nAverage(0.0)
	, m_tickCount(0)
	, m_ips(0)
{
	SetFCFilterValue(6.8e-9 * 8200);
	if (CreateFIRBuffers(FIR_LENGTH))
	{
		ReInit();
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}
}

CSpeaker::~CSpeaker()
{
	SAFE_DELETE_ARRAY(m_pRecieveTapeSamples);
}

void CSpeaker::ReInit()
{
	// нормализация частоты среза: w = fs / (Fd/2)
	// как на самом деле должно быть, в libdspl-2.0 не описано, но эта форумула
	// на слух даёт примерно подходящий результат
	double w0 = 2 * 8500.0 / double(g_Config.m_nSoundSampleRate);
	double w1 = 0.0;
    fir_linphase(m_nFirLength, w0, w1, FIR_FILTER::LOWPASS,
	                       FIR_WINDOW::BLACKMAN_HARRIS, true, 0.0, m_pH);
}

void CSpeaker::Reset()
{
	m_tickCount = 0;
}

void CSpeaker::ConfigureTapeBuffer(int ips)
{
	m_ips = ips;
	SAFE_DELETE_ARRAY(m_pRecieveTapeSamples);
	m_pRecieveTapeSamples = new SAMPLE_INT[m_ips];

	if (m_pRecieveTapeSamples)
	{
		ZeroMemory(m_pRecieveTapeSamples, m_ips * SAMPLE_INT_SIZE);
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}
}

/*
Вход: pBuff - указатель на общий буфер, данные там типа SAMPLE_INT
      nSampleLen - длина буфера pBuff в cэмплах
*/
void CSpeaker::ReceiveTapeBuffer(void *pBuff, int nSampleLen)
{
	ASSERT(m_ips == nSampleLen); // m_ips == должно быть размер в сэмплах!
	Reset(); // обнуляем TickCounter,
	register auto inBuf = reinterpret_cast<SAMPLE_INT *>(pBuff);
	register SAMPLE_INT avg = 0.0; // инт не хватает, случается переполнение

	// Заполняем из входного буфера внутренний. из стерео делаем моно.
	for (register int n = 0; n < m_ips; ++n)
	{
		register SAMPLE_INT l = *inBuf++;
		register SAMPLE_INT r = *inBuf++;
		register SAMPLE_INT t = (l + r) / 2.0; // микшируем

		if (!m_bDCOffset)
		{
			t = DCOffset(t, m_dAvgL, m_pdBufferL, m_nBufferPosL); // на выходе m_nBufferPosL указывает на следующую позицию.
		}

		m_pRecieveTapeSamples[n] = t;
		avg += t;
	}

	m_nAverage = avg / SAMPLE_INT(m_ips);
}


bool CSpeaker::GetTapeSample()
{
	return (m_pRecieveTapeSamples[m_tickCount++] > m_nAverage);
}

/*
 Спикер использует только верхний полупериод сэмпла
 Посмотрев амплитуды звуков БК и эмулятора, у меня закрадывается подозрение,
 что нужно использовать преобразование ШИМ->АМ или как-то иначе эмулировать
 поведение конденсатора.
 Значения амплитуд неточные, измерены на глаз.
*/
const SAMPLE_INT CSpeaker::m_dSpeakerValues[8] =
{
	SAMPLE_INT(MIN_SAMPLE) * 0.00 / FLOAT_BASE, // 0 0   0%
	SAMPLE_INT(MAX_SAMPLE) * 0.15 / FLOAT_BASE, // 1 4   15%
	SAMPLE_INT(MAX_SAMPLE) * 0.30 / FLOAT_BASE, // 2 40  30%
	SAMPLE_INT(MAX_SAMPLE) * 0.40 / FLOAT_BASE, // 3 44  40%
	SAMPLE_INT(MAX_SAMPLE) * 0.70 / FLOAT_BASE, // 4 100 70%
	SAMPLE_INT(MAX_SAMPLE) * 0.80 / FLOAT_BASE, // 5 104 80%
	SAMPLE_INT(MAX_SAMPLE) * 0.94 / FLOAT_BASE, // 6 140 94%
	SAMPLE_INT(MAX_SAMPLE) * 1.00 / FLOAT_BASE  // 7 144 100%
};

void CSpeaker::SetData(uint16_t inVal)
{
	if (m_bEnableSound)
	{
		register int w = inVal & 0144; // получим маску звуковых битов

		if (w & 04) // 2й бит переместим в 4й
		{
			w |= 020;
		}

		w >>= 4; // сдвинем на 4 бита вправо. в результате получится число 0..7
		m_dLeftAcc = m_dSpeakerValues[w];
	}
	else
	{
		m_dLeftAcc = 0.0;
	}
}

void CSpeaker::GetSample(register SAMPLE_INT &sampleL, register SAMPLE_INT &sampleR)
{
	sampleL = m_bFilter ? RCFilterCalc(m_RCFL) : m_dLeftAcc;

	if (m_bDCOffset)
	{
		// если тут раскомментировать, не забыть убрать эту функцию из ReceiveTapeBuffer.
		// потому что они один и тот же буфер используют. и будут нехорошие глюки
		sampleL = DCOffset(sampleL, m_dAvgL, m_pdBufferL, m_nBufferPosL); // на выходе m_nBufferPosL указывает на следующую позицию.
	}

	// фильтр
	sampleL = FIRFilter(sampleL, m_pLeftBuf, m_nLeftBufPos);
	sampleR = sampleL;
}

