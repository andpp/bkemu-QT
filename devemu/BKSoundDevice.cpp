// BKSoundDevice.cpp: implementation of the CBKSoundDevice class.
//


#include "pch.h"
#include "Config.h"
#include "BKSoundDevice.h"

// #include <cmath>
// #if defined TARGET_WINXP
// #define _USE_MATH_DEFINES
// #include <math.h>
// #else
// #include <corecrt_math_defines.h>
// #endif
// #include <utility> // там есть #include <algorithm>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CBKSoundDevice::CBKSoundDevice()
	: m_bEnableSound(true)
	, m_bFilter(true)
	, m_bDCOffset(false)
	, m_bStereo(false)
	, m_nBufferPosL(0)
	, m_nBufferPosR(0)
	, m_dAvgL(0.0)
	, m_dAvgR(0.0)
	, m_pH(nullptr)
	, m_pLeftBuf(nullptr)
	, m_pRightBuf(nullptr)
	, m_nFirLength(FIR_LENGTH)
	, m_nLeftBufPos(0)
	, m_nRightBufPos(0)
	, m_dLeftAcc(0.0)
	, m_dRightAcc(0.0)
	, m_RCFVal(1.0)
{
	m_pdBufferL = new double[DCOFFSET_BUFFER_LEN];
	m_pdBufferR = new double[DCOFFSET_BUFFER_LEN];

	if (m_pdBufferL && m_pdBufferR)
	{
		for (int i = 0; i < DCOFFSET_BUFFER_LEN; ++i)
		{
			m_pdBufferL[i] = m_pdBufferR[i] = 0.0;
		}
	}
	else
	{
		g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
	}
}

CBKSoundDevice::~CBKSoundDevice()
{
	SAFE_DELETE_ARRAY(m_pdBufferL);
	SAFE_DELETE_ARRAY(m_pdBufferR);
	SAFE_DELETE_ARRAY(m_pH);
	SAFE_DELETE_ARRAY(m_pLeftBuf);
	SAFE_DELETE_ARRAY(m_pRightBuf);
}

void CBKSoundDevice::ReInit()
{}

void CBKSoundDevice::Reset()
{}

void CBKSoundDevice::SetData(uint16_t inVal)
{}

void CBKSoundDevice::GetSample(register SAMPLE_INT &sampleL, register SAMPLE_INT &sampleR)
{}


// автоматическое выравнивание уровня, очень полезно для ковокса, для AY - так себе,
double CBKSoundDevice::DCOffset(register double sample, register double &fAvg, register double *pBuf, register int &nBufPos)
{
	/*
	идея такая:
	avg0 = (buf[0] + buf[1] + ... + buf[buf_len-1]) / buf_len

	avg1 = avg0 - buf[buf_len-1] / buf_len + sample / buf_len

	не нужно сдвигать содержимое буфера, выкидывая последнее значение и добавляя в начало текущее.
	и не нужно каждый раз суммировать всё содержимое буфера, для расчёта среднего значения.
	имеем указатель на позицию в буфере, прочитанное значение из буфера - это выкидываемый последний сэмпл
	а занесение нового значения - запись текущего сэмпла в буфер на заданную позицию.
	*/
	fAvg += (sample - pBuf[nBufPos]) / double(DCOFFSET_BUFFER_LEN);
	pBuf[nBufPos++] = sample;
	nBufPos &= DCOFFSET_BUFFER_MASK;
	// устраняем клиппинг, может появляться из-за резкой смены знака амплитуды
	register double t = sample - fAvg;
	constexpr double b = double(MAX_SAMPLE) / FLOAT_BASE;

	if (t > b)
	{
		return b;
	}

	if (t < -b)
	{
		return -b;
	}

	return t;
}

/*На входе:
sample - новый, поступающий сэмпл.
pBuf - кольцевой буфер сэмплов, длиной m_nFirLength
nBufPos - позиция в буфере, pBuf, куда помещать сэмпл.
*/
double CBKSoundDevice::FIRFilter(register double sample, register double *pBuf, register int &nBufPos)
{
	// поместим новый сэмпл в буфер.
	pBuf[nBufPos] = sample;

	if (++nBufPos >= m_nFirLength)
	{
		nBufPos = 0;
	}

	if (m_bFilter)
	{
		// теперь nBufPos указывает на начало, откуда брать сэмплы для фильтрации
		register double dAcc = 0.0;
		register int j = nBufPos;

		for (register int i = 0; i < m_nFirLength; ++i)
		{
			dAcc += m_pH[i] * pBuf[j];

			if (++j >= m_nFirLength)
			{
				j = 0;
			}
		}

		// ограничение амплитуды
		constexpr double b = double(MAX_SAMPLE) / FLOAT_BASE;

		if (dAcc > b)
		{
			dAcc = b;
		}
		else if (dAcc < -b)
		{
			dAcc = -b;
		}

		return dAcc;
	}

	return sample;
}

bool CBKSoundDevice::CreateFIRBuffers(int nLen)
{
	int nLen1 = nLen + 1;
	SAFE_DELETE_ARRAY(m_pH);
	m_pH = new double[nLen1];
	SAFE_DELETE_ARRAY(m_pLeftBuf);
	m_pLeftBuf = new double[nLen1];
	SAFE_DELETE_ARRAY(m_pRightBuf);
	m_pRightBuf = new double[nLen1];

	if (m_pH && m_pLeftBuf && m_pRightBuf)
	{
		memset(m_pH, 0, nLen1 * sizeof(double));
		memset(m_pLeftBuf, 0, nLen1 * sizeof(double));
		memset(m_pRightBuf, 0, nLen1 * sizeof(double));
		m_nFirLength = nLen;
		return true;
	}

	return false;
}


// почти работает.
// в реальности там получается разное сопротивление для разных уровней,
// но и так работает почти как в оригинале.
void CBKSoundDevice::RCFilter(sRCFparam &p, const double fAcc, const double fTime)
{
	if (fAcc > p.fUi_prev)
	{
		p.fUi_prev = fAcc;
		p.fminvol = RCFilterCalc(p);	// от этого уровня начинаем
		p.fmaxvol = fAcc;				// до этого уровня постараемся дойти
		p.bRCProc = true;				// надо заряжать

		if (p.fmaxvol < p.fminvol)  // так не бывает, но всё же, если конденсатор был заряжен сильнее, чем сейчас уровень
		{
			std::swap(p.fmaxvol, p.fminvol); // то надо разряжать
			p.bRCProc = false;
		}

		p.fdeltavol = p.fmaxvol - p.fminvol; // дельта - величина, насколько подскочило напряжение относительно заряда конденсатора
		p.ft = fTime;
	}
	else if (fAcc < p.fUi_prev)
	{
		p.fUi_prev = fAcc;
		p.fmaxvol = RCFilterCalc(p);	// от этого уровня начинаем
		p.fminvol = fAcc;				// к этому уровню постараемся дойти
		p.bRCProc = false;				// надо разряжать

		if (p.fmaxvol < p.fminvol)  // если конденсатор был заряжен меньше, чем сейчас уровень
		{
			std::swap(p.fmaxvol, p.fminvol); // то надо заряжать
			p.bRCProc = true;
		}

		p.fdeltavol = p.fmaxvol - p.fminvol;  // дельта - величина, насколько изменилось напряжение относительно заряда конденсатора
		p.ft = fTime;
	}
	else
	{
		p.ft += fTime;  // если напряжение держится одного уровня - просто продолжаем процесс
	}

}

SAMPLE_INT CBKSoundDevice::RCFilterCalc(sRCFparam &p)
{
	register double v = p.fdeltavol * exp(-(p.ft / m_RCFVal));

	if (p.bRCProc)
	{
		// зарядка
		// return m_fminvol + m_fdeltavol * (1 - exp(-(m_ft / (6.8e-9 * 8200))));
		// ниже - эта же функция с раскрытыми скобками, на одно действие меньше.
		return p.fmaxvol - v;
	}

	// разрядка
	return p.fminvol + v;
}

