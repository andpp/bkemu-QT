// BKSoundDevice.cpp: implementation of the CBKSoundDevice class.
//


#include "pch.h"
#include "Config.h"
#include "BKSoundDevice.h"
#include "BKMessageBox.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <utility> // там есть #include <algorithm>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction

CBKSoundDevice::CBKSoundDevice()
	: m_bEnableSound(true)
	, m_bFilter(true)
	, m_bStereo(false)
	, m_nBufferPosL(0)
	, m_nBufferPosR(0)
	, m_dAvgL(0.0)
	, m_dAvgR(0.0)
	, m_nLeftBufPos(0)
	, m_nRightBufPos(0)
	, m_dLeftAcc(0.0)
	, m_dRightAcc(0.0)
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

	for (int i = 0; i < FIR_LENGTH; ++i)
	{
		m_LeftBuf[i] = m_RightBuf[i] = 0.0;
	}

	m_pH = new double[FIR_LENGTH];

	if (m_pH)
	{
		ReInit();
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
}

void CBKSoundDevice::ReInit()
{}

void CBKSoundDevice::Reset()
{}

void CBKSoundDevice::SetSample(uint16_t inVal)
{(void)inVal;}

void CBKSoundDevice::GetSample(register SAMPLE_INT &sampleL, register SAMPLE_INT &sampleR)
{(void)sampleL; (void)sampleR;}


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

	if (t < 0.0)
	{
		t = -t;
	}

	if (t > double(MAX_SAMPLE) / FLOAT_BASE)
	{
		return sample + fAvg;
	}
	else
	{
		return sample - fAvg;
	}
}

// на входе - формируемый массив и его размер
// dFs1 - частота среза фильтров НЧ или ВЧ
// dFs2 - верхняя частота среза фильтров ПЗ или ПП
// nFilterType - тип фильтра
// nN - минимум 3. защиты от дурака нет. слишком малое значение вызовет выход за границы массива
void CBKSoundDevice::CalcFIR(double *pH, int nN, double dFs1, double dFs2, FIR_FILTER nFilterType)
{
	FIR_WINDOW nWindowType = FIR_WINDOW::BLACKMAN_HARRIS;
	// Расчёт импульсной характеристики фильтра
	double dFc1 = dFs1 / g_Config.m_nSoundSampleRate;
	double dWc1 = 2.0 * M_PI * dFc1;
	int n = nN / 2;

	// ФНЧ
	for (int i = 0; i <= n; ++i)
	{
		if (i == 0)
		{
			pH[n] = dWc1;
		}
		else
		{
			pH[n + i] = pH[n - i] = sinl(dWc1 * i) / (i);
		}
	}

	// весовые коэффициенты, и заодно сумму подсчитаем
	double dSUM = 0.0;

	for (int i = 0; i < nN; ++i)
	{
		pH[i] *= getWindow(i, nN, nWindowType);
		dSUM += pH[i];
	}

	// Нормализация импульсной характеристики
	for (int i = 0; i < nN; ++i)
	{
		pH[i] /= dSUM; // сумма коэффициентов равна 1
		// TRACE(_T("%.10f\n"), pH[i]);
	}

	if (nFilterType == FIR_FILTER::HIGHPASS)
	{
		for (int i = 0; i < nN; ++i)
		{
			pH[i] = -pH[i];
		}

		pH[n] += 1.0;
	}
	else if ((nFilterType == FIR_FILTER::BANDPASS) || (nFilterType == FIR_FILTER::BANDSTOP))
	{
		auto pHf = new double[nN];

		if (pHf)
		{
			double dFc2 = dFs2 / g_Config.m_nSoundSampleRate;
			double dWc2 = 2.0 * M_PI * dFc2;

			for (int i = 0; i <= n; ++i)
			{
				if (i == 0)
				{
					pHf[n] = dWc2;
				}
				else
				{
					pHf[n + i] = pHf[n - i] = sinl(dWc2 * i) / (i);
				}
			}

			// весовые коэффициенты, и заодно сумму подсчитаем
			double dSUM = 0.0;

			for (int i = 0; i < nN; ++i)
			{
				pHf[i] *= getWindow(i, nN, nWindowType);
				dSUM += pHf[i];
			}

			// Нормализация импульсной характеристики
			for (int i = 0; i < nN; ++i)
			{
				pHf[i] /= dSUM; // сумма коэффициентов равна 1
			}

			// инвертируем и объединяем с ФНЧ
			for (int i = 0; i < nN; ++i)
			{
				pH[i] -= pHf[i];
			}

			pH[n] += 1.0;

			if (nFilterType == FIR_FILTER::BANDPASS)
			{
				for (int i = 0; i < nN; ++i)
				{
					pH[i] = -pH[i];
				}

				pH[n] += 1.0;
			}

			delete[] pHf;
		}
		else
		{
			g_BKMsgBox.Show(IDS_BK_ERROR_NOTENMEMR, MB_OK);
		}
	}
}


// вход:
// i - текущая позиция
// n - порядок фильтра
double CBKSoundDevice::getWindow(int i, int n, FIR_WINDOW window)
{
	if (window == FIR_WINDOW::BARTLETT)
	{
		// устраняем нулевые значения
		long double a = i - (n - 1) / 2.0;

		if (a < 0)
		{
			a = -a;
		}

		return 2.0 / n * (n / 2.0 - a);
	}
	else if (window == FIR_WINDOW::HANNING)
	{
		// устраняем нулевые значения
		return 0.5 - 0.5 * cosl(M_PI / n * (1.0 + 2.0 * i));
	}

	if (window == FIR_WINDOW::BLACKMAN)
	{
		// устраняем нулевые значения
		long double a = M_PI / n * (1.0 + 2.0 * i);
		return 0.5 * (1.0 - 0.16 - cosl(a) + 0.16 * cosl(2.0 * a));
	}
	else
	{
		long double a = 2.0 * M_PI * i / (n - 1);

		if (window == FIR_WINDOW::HAMMING)
		{
			return 0.54 - 0.46 * cosl(a);
		}
		else if (window == FIR_WINDOW::BLACKMAN_HARRIS)
		{
			return 0.35875 - 0.48829 * cosl(a) + 0.14128 * cosl(2.0 * a) - 0.01168 * cosl(3.0 * a);
		}
		else if (window == FIR_WINDOW::BLACKMAN_NUTTAL)
		{
			return 0.35819 - 0.4891775 * cosl(a) + 0.1365995 * cosl(2.0 * a) - 0.0106411 * cosl(3.0 * a);
		}
		else if (window == FIR_WINDOW::NUTTAL)
		{
			return 0.355768 - 0.487396 * cosl(a) + 0.144232 * cosl(2.0 * a) - 0.012604 * cosl(3.0 * a);
		}
	}

	return 1.0;
}
/*На входе:
sample - новый, поступающий сэмпл.
pBuf - кольцевой буфер сэмплов, длиной FIR_LENGTH
nBufPos - позиция в буфере, pBuf, куда помещать сэмпл.
*/
double CBKSoundDevice::FIRFilter(register double sample, register double *pBuf, register int &nBufPos)
{
	// поместим новый сэмпл в буфер.
	pBuf[nBufPos] = sample;

	if (++nBufPos >= FIR_LENGTH)
	{
		nBufPos = 0;
	}

	if (m_bFilter)
	{
		// теперь nBufPos указывает на начало, откуда брать сэмплы для фильтрации
		register double dAcc = 0.0;
		register int j = nBufPos;

		for (register int i = 0; i < FIR_LENGTH; ++i)
		{
			dAcc += m_pH[i] * pBuf[j];

			if (++j >= FIR_LENGTH)
			{
				j = 0;
			}
		}

		return dAcc;
	}
	else
	{
		return sample;
	}
}


