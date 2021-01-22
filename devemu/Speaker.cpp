// Speaker.cpp: implementation of the CSpeaker class.
//


#include "pch.h"
#include "Config.h"
#include "Speaker.h"
#include "BKMessageBox.h"
#include <math.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CSpeaker::CSpeaker()
	: CBKSoundDevice()
	, m_pRecieveTapeSamples(nullptr)
	, m_nAverage(0.0)
	, m_bRCProc(false)
	, m_tickCount(0)
	, m_ft(0.0)
	, m_fmaxvol(0.0)
	, m_fminvol(0.0)
	, m_fdeltavol(0.0)
	, m_fUi_prev(0.0)
	, m_ips(0)
{
}

CSpeaker::~CSpeaker()
{
	SAFE_DELETE_ARRAY(m_pRecieveTapeSamples);
}

void CSpeaker::ReInit()
{
	// CalcFIR(m_pH, FIR_LENGTH, 5400.0, 0.0, FILTER_TYPE::LOWPASS); // как-то не очень
	CalcFIR(m_pH, FIR_LENGTH, 8000.0, 0.0, FIR_FILTER::LOWPASS);
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
		t = DCOffset(t, m_dAvgL, m_pdBufferL, m_nBufferPosL); // на выходе m_nBufferPosL указывает на следующую позицию.
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

void CSpeaker::SetSample(uint16_t inVal)
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

// почти работает.
// в реальности там получается разное сопротивление для разных уровней,
// но и так работает почти как в оригинале.
void CSpeaker::RCSHIM(const double fTime)
{
	if (m_bFilter)
	{
		if (m_dLeftAcc > m_fUi_prev)
		{
			m_fUi_prev = m_dLeftAcc;
			m_fminvol = RCSHIMCalc();   // от этого уровня начинаем
			m_fmaxvol = m_dLeftAcc;     // до этого уровня постараемся дойти
			m_bRCProc = true;           // надо заряжать

			if (m_fmaxvol < m_fminvol)  // так не бывает, но всё же, если конденсатор был заряжен сильнее, чем сейчас уровень
			{
				std::swap(m_fmaxvol, m_fminvol); // то надо разряжать
				m_bRCProc = false;
			}

			m_fdeltavol = m_fmaxvol - m_fminvol; // дельта - величина, насколько подскочило напряжение относительно заряда конденсатора
			m_ft = fTime;
		}
		else if (m_dLeftAcc < m_fUi_prev)
		{
			m_fUi_prev = m_dLeftAcc;
			m_fmaxvol = RCSHIMCalc();   // от этого уровня начинаем
			m_fminvol = m_dLeftAcc;     // к этому уровню постараемся дойти
			m_bRCProc = false;          // надо разряжать

			if (m_fmaxvol < m_fminvol)  // если конденсатор был заряжен меньше, чем сейчас уровень
			{
				std::swap(m_fmaxvol, m_fminvol); // то надо заряжать
				m_bRCProc = true;
			}

			m_fdeltavol = m_fmaxvol - m_fminvol;  // дельта - величина, насколько изменилось напряжение относительно заряда конденсатора
			m_ft = fTime;
		}
		else
		{
			m_ft += fTime;  // если напряжение держится одного уровня - просто продолжаем процесс
		}
	}
}

SAMPLE_INT CSpeaker::RCSHIMCalc()
{
	if (m_bRCProc)
	{
		// зарядка
		// return m_fminvol + m_fdeltavol * (1 - exp(-(m_ft / (6.8e-9 * 8200))));
		// ниже - эта же функция с раскрытыми скобками, на одно действие меньше.
		return m_fmaxvol - m_fdeltavol * exp(-(m_ft / (6.8e-9 * 8200)));
	}
	else
	{
		// разрядка
		return m_fminvol + m_fdeltavol * exp(-(m_ft / (6.8e-9 * 8200)));
	}
}

void CSpeaker::GetSample(register SAMPLE_INT &sampleL, register SAMPLE_INT &sampleR)
{
	sampleL = m_bFilter ? RCSHIMCalc() : m_dLeftAcc;
// если тут раскомментировать, не забыть убрать эту функцию из ReceiveTapeBuffer.
// потому что они один и тот же буфер используют. и будут нехорошие глюки
//  sampleL = DCOffset(sampleL, m_dAvgL, m_pdBufferL, m_nBufferPosL); // на выходе m_nBufferPosL указывает на следующую позицию.
	// фильтр
	sampleL = FIRFilter(sampleL, m_LeftBuf, m_nLeftBufPos);
	sampleR = sampleL;
}

