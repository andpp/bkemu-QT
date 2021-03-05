// Speaker.h: interface for the CSpeaker class.
//


#pragma once

#include "BKSoundDevice.h"

class CSpeaker : public CBKSoundDevice
{
		static const SAMPLE_INT m_dSpeakerValues[8];

	protected:
		enum            {DEFAULT_SIZE = 2 * 1024 * 1014};
		long long       m_tickCount;

		int             m_ips;
		SAMPLE_INT     *m_pRecieveTapeSamples; // массив данных получаемых с ленты, из которых узнается, что там прочлось
		SAMPLE_INT      m_nAverage;

	public:

		CSpeaker();
		virtual ~CSpeaker() override;
		virtual void    ReInit() override;
		virtual void    Reset() override;

		void            ConfigureTapeBuffer(int ips);

		void            ReceiveTapeBuffer(void *pBuff, int nSampleLen);
		bool            GetTapeSample();

		virtual void    SetData(uint16_t inVal) override;
		virtual void    GetSample(register SAMPLE_INT &sampleL, register SAMPLE_INT &sampleR) override;
};
