
#pragma once

#include "BKSoundDevice.h"

class CEMU2149 : public CBKSoundDevice
{
		enum
		{
			CHAN_A = 0,
			CHAN_B = 1,
			CHAN_C = 2,
			SOUND_CHANNELS = 3
		};
#pragma pack(push)
#pragma pack(1) // всё должно быть именно так, несмотря на предупреждения всяких анализаторов. Я лучше знаю.
		struct AYREGS
		{
			uint16_t fA, fB, fC;   // 0,1; 2,3; 4,5;
			uint8_t noise, mix;    // 6; 7;
			uint8_t vA, vB, vC;    // 8; 9; 10;
			uint16_t envT;         // 11,12;
			uint8_t env;           // 13;
			uint8_t portA, portB;  // 14; 15;
		};

		union
		{
			struct AYREGS m_r; // обозначения регистров для более удобного доступа к их значениям.
			uint8_t m_reg[16]; // содержимое регистров
		};

#pragma pack(pop)

		static const int m_voltbl[2][32];
		double m_vols[32];

		int     m_nClock, m_nRate;
		uint32_t m_nBaseIncrement;
		uint32_t m_nBaseCount;

		/* m_nRate converter*/
		double  m_dRealStep;
		double  m_dPSGTime;
		double  m_dPSGStep;

		bool    m_bHQ;
		uint8_t m_nSynthReg;

		struct SoundChannel
		{
			int nCount;
			int nVolume;    // громкость звучания канала
			int nFreq;      // частота ноты канала
			bool bEnv;      // флаг, что включена огибающая на канале
			bool bToneToggle;
			bool bToneMask; // флаг звучания ноты
			bool bNoiseMask; // флаг звучания шума
			// указатели на переменные из конфига
			double *pVolume;	// громкость канала
			double *pPanL;		// панорамирование влево
			double *pPanR;		// панорамирование вправо
		};

		struct SoundChannel m_Channel[SOUND_CHANNELS];

		int     m_nEnvPtr;
		int     m_nDEnv;
		bool    m_bEnvLValue;
		bool    m_bEnvAttack;
		bool    m_bEnvHold;
		bool    m_bEnvAlt;
		int     m_nEnvFreq;
		int     m_nEnvCount;

		uint32_t m_nNoiseSeed;
		int     m_nNoiseCount;
		int     m_nNoiseFreq;

		/* I/O Ctrl*/
		uint32_t m_nAddr;

		double m_dMixL, m_dMixR;
		void calc();

		// -----------------------------
		// логирование регистров
		bool m_bLog; // флаг работы логирования
		bool m_bLogRegOut;
		FILE *m_pLOGFile;
		// -----------------------------



	public:
		CEMU2149();
		virtual ~CEMU2149() override;
		virtual void ReInit() override;
		virtual void Reset() override;

		virtual void GetSample(register SAMPLE_INT &sampleL, register SAMPLE_INT &sampleR) override;

		void    PSG_reset();
		void    PSG_init(int c, int r);
		void    PSG_set_quality(bool q);
		void    PSG_setVolumeMode(int type);
		void    PSG_writeReg(uint32_t reg, uint8_t val);
		void    PSG_writeIO(uint32_t nAddr, uint8_t val);
		uint8_t PSG_readReg(int reg);
		uint8_t PSG_readIO();
		void    PSG_calc(SAMPLE_INT &L, SAMPLE_INT &R);

		// Writing register address
		void synth_write_address(uint16_t word);

		// Writing data
		void synth_write_data(uint8_t byte);

		bool isLogEnabled()
		{
			return m_bLog;
		}
		// фиксируем тик таймера 50Гц
		void log_timerTick();
		// запускаем логирование
		void log_start(const CString &strUniq);
		// завершаем логирование
		void log_done();

};

