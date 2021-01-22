// BKSoundDevice.h: interface for the CBKSoundDevice class.
//


#pragma once

#include "BKSound_Defines.h"

constexpr auto DCOFFSET_BUFFER_MASK = 0xff; // степень двойки минус 1
constexpr auto DCOFFSET_BUFFER_LEN = (DCOFFSET_BUFFER_MASK + 1); // степень двойки

// длина ких фильтра. нечётная
constexpr auto FIR_LENGTH = 127;

class CBKSoundDevice
{
		enum class FIR_WINDOW
		{
			RECTANGULAR = 0,    // Rectangular window function
			BARTLETT,           // Bartlett (triangular) window
			HANNING,            // Hanning window
			HAMMING,            // Hamming window
			BLACKMAN,           // Blackman window
			BLACKMAN_HARRIS,    // Blackman-Harris window
			BLACKMAN_NUTTAL,    // Blackman-Nuttal window
			NUTTAL              // Nuttal window
		};

	protected:
		enum class FIR_FILTER
		{
			NONE = 0,
			LOWPASS,    // ФНЧ
			HIGHPASS,   // ФВЧ
			BANDSTOP,   // ФПЗ
			BANDPASS    // ФПП
		};
		bool            m_bEnableSound;     // флаг разрешения звука
		bool            m_bFilter;          // флаг включения фильтрации звука
		bool            m_bStereo;

		// переменные для вычисления смещения DC.
		int             m_nBufferPosL, m_nBufferPosR;
		double          m_dAvgL, m_dAvgR;
		double         *m_pdBufferL;
		double         *m_pdBufferR;

		double          *m_pH;              // коэффициенты КИХ

		double          m_LeftBuf[FIR_LENGTH];  // буферы для фильтра
		double          m_RightBuf[FIR_LENGTH]; // в перспективе - динамические
		int             m_nLeftBufPos;      // позиции в буферах фильтра
		int             m_nRightBufPos;

		// накопители значения сэмпла, значимо только последнее на момент выборки значение
		double          m_dLeftAcc, m_dRightAcc;

		double          DCOffset(register double sample, register double &fAvg, register double *pBuf, register int &nBufPos);
		void            CalcFIR(double *pH, int nN, double dFs1, double dFs2, FIR_FILTER nFilterType);
		double          getWindow(int i, int n, FIR_WINDOW window);
		double          FIRFilter(register double sample, register double *pBuf, register int &nBufPos);

	public:
		CBKSoundDevice();
		virtual ~CBKSoundDevice();
		virtual void    ReInit();
		virtual void    Reset();

		inline void     EnableSound(bool bEnable)
		{
			m_bEnableSound = bEnable;

			if (!bEnable)
			{
				m_dLeftAcc = m_dRightAcc = 0.0;
			}
		}
		inline bool     IsSoundEnabled()
		{
			return m_bEnableSound;
		}

		inline void     SetFilter(bool bEnable)
		{
			m_bFilter = bEnable;
		}
		inline bool     GetFilter()
		{
			return m_bFilter;
		}
		inline void         SetStereo(bool bEnable)
		{
			m_bStereo = bEnable;
		};
		inline bool         IsStereo()
		{
			return m_bStereo;
		};
		virtual void    SetSample(uint16_t inVal);
		virtual void    GetSample(register SAMPLE_INT &sampleL, register SAMPLE_INT &sampleR);

};
